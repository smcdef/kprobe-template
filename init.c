// SPDX-License-Identifier: GPL-3.0
/*
 * init.c
 *
 * Here's the register of kprobes, kretprobes and tracepoints.
 */
#define pr_fmt(fmt) CONFIG_MODULE_NAME ": " fmt

#include "kprobe.h"

/* Defined in linker script */
extern struct kprobe * const __start_kprobe_template[];
extern struct kprobe * const __stop_kprobe_template[];

extern struct kretprobe * const __start_kretprobe_template[];
extern struct kretprobe * const __stop_kretprobe_template[];

extern struct tracepoint_entry * const __start_tracepoint_template[];
extern struct tracepoint_entry * const __stop_tracepoint_template[];

extern struct kprobe_initcall const * const __start_kprobe_initcall[];
extern struct kprobe_initcall const * const __stop_kprobe_initcall[];

static inline int num_tracepoint(void)
{
	return __stop_tracepoint_template - __start_tracepoint_template;
}

static inline bool is_tracepoint_lookup_finshed(int tp_initalized)
{
	return tp_initalized == num_tracepoint();
}

static void __init tracepoint_lookup(struct tracepoint *tp, void *priv)
{
	struct tracepoint_entry * const *tracepoint_ptr;
	int *tp_initalized = priv;

	if (is_tracepoint_lookup_finshed(*tp_initalized))
		return;

	for (tracepoint_ptr = __start_tracepoint_template;
	     tracepoint_ptr < __stop_tracepoint_template; tracepoint_ptr++) {
		struct tracepoint_entry *tracepoint = *tracepoint_ptr;

		if (tracepoint->tp || !tracepoint->name ||
		    strcmp(tp->name, tracepoint->name))
			continue;
		tracepoint->tp = tp;
		(*tp_initalized)++;
	}
}

static int __init kprobe_register_kprobes(void)
{
	int ret;
	struct kprobe * const *kprobe_ptr;

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

	return 0;

unregister_kprobes:
	while (--kprobe_ptr >= __start_kprobe_template) {
		struct kprobe *kprobe = *kprobe_ptr;

		pr_info("kprobe unregister at %pS\n", kprobe->addr);
		unregister_kprobe(kprobe);
	}

	return ret;
}

static void kprobe_unregister_kprobes(void)
{
	struct kprobe * const *kprobe_ptr = __stop_kprobe_template;

	/* Unregister kprobes */
	while (--kprobe_ptr >= __start_kprobe_template) {
		struct kprobe *kprobe = *kprobe_ptr;

		pr_info("kprobe unregister at %pS\n", kprobe->addr);
		unregister_kprobe(kprobe);
	}
}

static int __init kprobe_register_kretprobes(void)
{
	int ret;
	struct kretprobe * const *kretprobe_ptr;

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

	return ret;
}

static void kprobe_unregister_kretprobes(void)
{
	struct kretprobe * const *kretprobe_ptr = __stop_kretprobe_template;

	while (--kretprobe_ptr >= __start_kretprobe_template) {
		struct kretprobe *kretprobe = *kretprobe_ptr;
		int nmissed = kretprobe->nmissed;

		if (nmissed)
			pr_notice("kretprobe missed probing %d instances"
				  " of %pS\n", nmissed, kretprobe->kp.addr);

		pr_info("kretprobe unregister at %pS\n", kretprobe->kp.addr);
		unregister_kretprobe(kretprobe);
	}
}

static int __init kprobe_register_tracepoints(void)
{
	int ret;
	struct tracepoint_entry * const *tracepoint_ptr;

	for (tracepoint_ptr = __start_tracepoint_template;
	     tracepoint_ptr < __stop_tracepoint_template; tracepoint_ptr++) {
		struct tracepoint_entry *tracepoint = *tracepoint_ptr;

		ret = tracepoint_probe_register(tracepoint->tp, tracepoint->handler,
						tracepoint->priv);
		if (ret && ret != -EEXIST) {
			pr_err("tracepoint register fail at trace_%s returned %d\n",
			       tracepoint->name, ret);
			goto unregister_tracepoints;
		} else {
			pr_info("tracepoint register at trace_%s\n",
				tracepoint->name);
		}
	}

	return 0;
unregister_tracepoints:
	while (--tracepoint_ptr >= __start_tracepoint_template) {
		struct tracepoint_entry *tracepoint = *tracepoint_ptr;

		pr_info("tracepoint unregister at trace_%s\n",
			tracepoint->name);
		tracepoint_probe_unregister(tracepoint->tp, tracepoint->handler,
					    tracepoint->priv);
	}

	return ret;
}

static void kprobe_unregister_tracepoints(void)
{
	struct tracepoint_entry * const *tracepoint_ptr = __stop_tracepoint_template;

	while (--tracepoint_ptr >= __start_tracepoint_template) {
		struct tracepoint_entry *tracepoint = *tracepoint_ptr;

		pr_info("tracepoint unregister at trace_%s\n",
			tracepoint->name);
		tracepoint_probe_unregister(tracepoint->tp, tracepoint->handler,
					    tracepoint->priv);
	}
}

static int __init do_kprobe_initcalls(void)
{
	int ret = 0;
	struct kprobe_initcall const * const *initcall_p;

	for (initcall_p = __start_kprobe_initcall;
	     initcall_p < __stop_kprobe_initcall; initcall_p++) {
		struct kprobe_initcall const *initcall = *initcall_p;

		if (initcall->init) {
			ret = initcall->init();
			if (ret < 0)
				goto exit;
		}
	}

	return 0;
exit:
	while (--initcall_p >= __start_kprobe_initcall) {
		struct kprobe_initcall const *initcall = *initcall_p;

		if (initcall->exit)
			initcall->exit();
	}

	return ret;
}

static void do_kprobe_exitcalls(void)
{
	struct kprobe_initcall const * const *initcall_p = __stop_kprobe_initcall;

	while (--initcall_p >= __start_kprobe_initcall) {
		struct kprobe_initcall const *initcall = *initcall_p;

		if (initcall->exit)
			initcall->exit();
	}
}

static int __init kprobes_init(void)
{
	int ret;

	if (num_tracepoint()) {
		int tp_initalized = 0;

		/* Lookup for the tracepoint that we needed */
		for_each_kernel_tracepoint(tracepoint_lookup, &tp_initalized);
		if (!is_tracepoint_lookup_finshed(tp_initalized))
			return -ENODEV;
	}

	ret = do_kprobe_initcalls();
	if (ret < 0)
		return ret;

	ret = kprobe_register_kprobes();
	if (ret < 0)
		goto exit;

	ret = kprobe_register_kretprobes();
	if (ret < 0)
		goto unregister_kprobes;

	ret = kprobe_register_tracepoints();
	if (ret < 0)
		goto unregister_kretprobes;

	return 0;

unregister_kretprobes:
	kprobe_unregister_kretprobes();
unregister_kprobes:
	kprobe_unregister_kprobes();

	/* Make sure there is no caller left using the probe. */
	if (num_tracepoint())
		tracepoint_synchronize_unregister();
exit:
	do_kprobe_exitcalls();

	return ret;
}

static void __exit kprobes_exit(void)
{
	kprobe_unregister_tracepoints();
	kprobe_unregister_kretprobes();
	kprobe_unregister_kprobes();

	/* Make sure there is no caller left using the probe. */
	if (num_tracepoint())
		tracepoint_synchronize_unregister();

	do_kprobe_exitcalls();
}

module_init(kprobes_init);
module_exit(kprobes_exit);

MODULE_INFO(homepage, "https://github.com/smcdef/kprobe-template");
MODULE_VERSION(CONFIG_MODULE_VERSION);
MODULE_LICENSE(CONFIG_MODULE_LICENSE);
MODULE_AUTHOR(CONFIG_MODULE_AUTHOR);
MODULE_DESCRIPTION(CONFIG_MODULE_DESCRIPTION);
