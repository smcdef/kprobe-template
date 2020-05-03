// SPDX-License-Identifier: GPL-3.0
/*
 * init.c
 *
 * Here's the register of kprobes and kretprobes.
 */
#define pr_fmt(fmt) "kprobes: " fmt

#include "kprobe.h"

/*
 * Avoid compiler warning when there is only one
 * of kprobes or kretprobes.
 */
static struct kprobe * const __dummy_kprobe __used
	__attribute__((section(".__kprobe_dummy"))) = NULL;

static struct kretprobe * const __dummy_kretprobe __used
	__attribute__((section(".__kretprobe_dummy"))) = NULL;

/* Defined in linker script */
extern struct kprobe * const __start_kprobe_template[];
extern struct kprobe * const __stop_kprobe_template[];

extern struct kretprobe * const __start_kretprobe_template[];
extern struct kretprobe * const __stop_kretprobe_template[];

static int __init kprobes_init(void)
{
	int ret;
	struct kprobe * const *kprobe_ptr;
	struct kretprobe * const *kretprobe_ptr;

	for (kprobe_ptr = __start_kprobe_template;
	     kprobe_ptr < __stop_kprobe_template; kprobe_ptr++) {
		struct kprobe *kprobe = *kprobe_ptr;

		ret = register_kprobe(kprobe);
		if (ret < 0) {
			pr_err("kprobe register fail at %s+%x returned %d\n",
				kprobe->symbol_name, kprobe->offset, ret);
			goto unregister_kprobes;
		} else {
			pr_info("kprobe register at %pS\n", kprobe->addr);
		}
	}

	for (kretprobe_ptr = __start_kretprobe_template;
	     kretprobe_ptr < __stop_kretprobe_template; kretprobe_ptr++) {
		struct kretprobe *kretprobe = *kretprobe_ptr;

		ret = register_kretprobe(kretprobe);
		if (ret < 0) {
			pr_err("kretprobe register fail at %s+%x returned %d\n",
				kretprobe->kp.symbol_name, kretprobe->kp.offset,
				ret);
			goto unregister_kretprobes;
		} else {
			pr_info("kretprobe register at %pS\n",
				kretprobe->kp.addr);
		}
	}

	return 0;

unregister_kretprobes:
	while (--kretprobe_ptr >= __start_kretprobe_template) {
		struct kretprobe *kretprobe = *kretprobe_ptr;

		pr_info("kretprobe unregister at %pS\n", kretprobe->kp.addr);
		unregister_kretprobe(kretprobe);
	}

unregister_kprobes:
	while (--kprobe_ptr >= __start_kprobe_template) {
		struct kprobe *kprobe = *kprobe_ptr;

		pr_info("kprobe unregister at %pS\n", kprobe->addr);
		unregister_kprobe(kprobe);
	}

	/* Make sure there is no caller left using the probe. */
	synchronize_sched();

	return ret;
}

static void __exit kprobes_exit(void)
{
	struct kprobe * const *kprobe_ptr = __stop_kprobe_template;
	struct kretprobe * const *kretprobe_ptr = __stop_kretprobe_template;

	/* Unregister kretprobes */
	while (--kretprobe_ptr >= __start_kretprobe_template) {
		struct kretprobe *kretprobe = *kretprobe_ptr;
		int nmissed = kretprobe->nmissed;

		if (nmissed)
			pr_info("kretprobe missed probing %d instances"
				" of %pS\n", nmissed, kretprobe->kp.addr);

		pr_info("kretprobe unregister at %pS\n", kretprobe->kp.addr);
		unregister_kretprobe(kretprobe);
	}

	/* Unregister kprobes */
	while (--kprobe_ptr >= __start_kprobe_template) {
		struct kprobe *kprobe = *kprobe_ptr;

		pr_info("kprobe unregister at %pS\n", kprobe->addr);
		unregister_kprobe(kprobe);
	}

	/* Make sure there is no caller left using the probe. */
	synchronize_sched();
}

module_init(kprobes_init);
module_exit(kprobes_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Muchun Song <songmuchun@bytedance.com>");
MODULE_DESCRIPTION("Kprobe template for easy register kprobe/kretprobe");
