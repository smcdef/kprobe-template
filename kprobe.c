// SPDX-License-Identifier: GPL-3.0
/*
 * kprobe.c
 *
 * Here's a sample kernel module showing the use of probes.
 */
#define pr_fmt(fmt) CONFIG_MODULE_NAME ": " fmt

#include <linux/timer.h>
#include <linux/stacktrace.h>
#include <linux/sizes.h>
#include <linux/version.h>
#include "kprobe.h"

#define CREATE_PRINT_EVENT
#include "kprobe_print.h"

#define SAMPLING_PERIOD_MS	(10 * 1000UL)

static unsigned int kmemcache_size = 1024;
module_param(kmemcache_size, uint, 0);

static const void *kmemleak_addr;
static const void *kmemleak_addr_last;
static const void *kmemleak_caller;

static unsigned long kmemleak_stack[64];
static unsigned int kmemleak_stack_nr;

static void print_kmemleak_stack(void)
{
	int i;

	for (i = 0; i < kmemleak_stack_nr; i++)
		pr_info("%*c%pS\n", 2, ' ', (void *)kmemleak_stack[i]);
}

static unsigned int store_stack_trace(int skip)
{
	struct stack_trace stack_trace;

	stack_trace.nr_entries = 0;
	stack_trace.max_entries = ARRAY_SIZE(kmemleak_stack);
	stack_trace.entries = kmemleak_stack;
	stack_trace.skip = skip;

	save_stack_trace(&stack_trace);

	/*
	 * Some daft arches put -1 at the end to indicate its a full trace.
	 *
	 * <rant> this is buggy anyway, since it takes a whole extra entry so a
	 * complete trace that maxes out the entries provided will be reported
	 * as incomplete, friggin useless </rant>.
	 */
	if (stack_trace.nr_entries != 0 &&
	    kmemleak_stack[stack_trace.nr_entries - 1] == ULONG_MAX)
		stack_trace.nr_entries--;

	return stack_trace.nr_entries;
}

static inline void kmem_alloc_comm(const char *ptr, size_t bytes_alloc,
				   unsigned long call_site)
{
	if (bytes_alloc != kmemcache_size ||
	    cmpxchg(&kmemleak_addr, NULL, ptr))
		return;

	kmemleak_stack_nr = store_stack_trace(1);
	kmemleak_caller = (void *)call_site;
}

static inline void kmem_free_comm(const char *ptr)
{
	if (!kmemleak_addr || kmemleak_addr != ptr)
		return;

	cmpxchg(&kmemleak_addr, ptr, NULL);
	kmemleak_addr_last = NULL;
}

TRACEPOINT_HANDLER_DEFINE(kmalloc, unsigned long call_site, const void *ptr,
			  size_t bytes_req, size_t bytes_alloc, gfp_t gfp_flags)
{
	kmem_alloc_comm(ptr, bytes_alloc, call_site);
}

TRACEPOINT_HANDLER_DEFINE(kmalloc_node, unsigned long call_site,
			  const void *ptr, size_t bytes_req, size_t bytes_alloc,
			  gfp_t gfp_flags, int node)
{
	kmem_alloc_comm(ptr, bytes_alloc, call_site);
}

TRACEPOINT_HANDLER_DEFINE(kmem_cache_alloc, unsigned long call_site,
			  const void *ptr, size_t bytes_req, size_t bytes_alloc,
			  gfp_t gfp_flags)
{
	kmem_alloc_comm(ptr, bytes_alloc, call_site);
}

TRACEPOINT_HANDLER_DEFINE(kmem_cache_alloc_node, unsigned long call_site,
			  const void *ptr, size_t bytes_req, size_t bytes_alloc,
			  gfp_t gfp_flags, int node)
{
	kmem_alloc_comm(ptr, bytes_alloc, call_site);
}

TRACEPOINT_HANDLER_DEFINE(kfree, unsigned long call_site, const void *ptr)
{
	kmem_free_comm(ptr);
}

TRACEPOINT_HANDLER_DEFINE(kmem_cache_free, unsigned long call_site,
			  const void *ptr)
{
	kmem_free_comm(ptr);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void kmemleak_timer_handler(unsigned long data)
#else
static void kmemleak_timer_handler(struct timer_list *timer)
#endif
{
	static unsigned int count;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	struct timer_list *timer = (struct timer_list *)data;
#endif

	if (kmemleak_addr) {
		kmemleak_print(kmemleak_addr, kmemleak_caller);

		if (++count % 10 == 0) {
			if (kmemleak_addr_last == kmemleak_addr) {
				pr_info("kmemleak: 0x%px\n", kmemleak_addr);
				print_kmemleak_stack();
			} else
				kmemleak_addr_last = kmemleak_addr;
		}
	}

	mod_timer(timer, jiffies + msecs_to_jiffies(SAMPLING_PERIOD_MS));
}

static struct timer_list kmemleak_timer;

static int __init kmemleak_kprobe_init(void)
{
	struct timer_list *timer = &kmemleak_timer;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	timer->flags = TIMER_IRQSAFE;
	setup_timer(timer, kmemleak_timer_handler, (unsigned long)timer);
#else
	timer_setup(timer, kmemleak_timer_handler, TIMER_IRQSAFE);
#endif
	timer->expires = jiffies + msecs_to_jiffies(SAMPLING_PERIOD_MS);
	add_timer(&kmemleak_timer);

	pr_info("Kmemleak: kmalloc-%d\n", kmemcache_size);

	return 0;
}

static void kmemleak_kprobe_exit(void)
{
	del_timer_sync(&kmemleak_timer);
}
KPROBE_INITCALL(kmemleak_kprobe_init, kmemleak_kprobe_exit);
