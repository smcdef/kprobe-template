// SPDX-License-Identifier: GPL-3.0
/*
 * Kprobe template
 */
#define pr_fmt(fmt) "kprobe: " fmt
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/percpu.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sizes.h>
#include <linux/syscalls.h>
#include <linux/tracepoint.h>
#include <linux/kprobes.h>
#include <linux/version.h>
#include <asm/irq_regs.h>

#if defined(CONFIG_X86_64)
#define SC_ARCH_REGS_TO_ARGS(x, ...)					\
	__MAP(x,__SC_ARGS						\
	      ,,regs->di,,regs->si,,regs->dx				\
	      ,,regs->r10,,regs->r8,,regs->r9)
#elif defined(CONFIG_ARM64)
#define SC_ARCH_REGS_TO_ARGS(x, ...)					\
	__MAP(x,__SC_ARGS						\
	      ,,regs->regs[0],,regs->regs[1],,regs->regs[2]		\
	      ,,regs->regs[3],,regs->regs[4],,regs->regs[5])
#else
#error "Unsupported architecture"
#endif

#define __KPROBE_HANDLER_DEFINE_COMM(name)				\
	static int name##_handler(struct kprobe *p,			\
				  struct pt_regs *regs);		\
	static struct kprobe name##_kprobe = {				\
		.symbol_name	= #name,				\
		.pre_handler	= name##_handler			\
	};								\
	static inline int __init name##_register(void)			\
	{								\
		int ret;						\
									\
		ret = register_kprobe(&name##_kprobe);			\
		if (ret < 0)						\
			pr_err("register kprobe(%s) fail returned %d\n",\
				#name, ret);				\
		return ret;						\
	}								\
	static inline void __exit name##_unregister(void)		\
	{								\
		unregister_kprobe(&name##_kprobe);			\
	}

#define __KPROBE_HANDLER_DEFINE_x(x, name, ...)				\
	__KPROBE_HANDLER_DEFINE_COMM(name)				\
	static inline int __se_##name##_handler(__MAP(x, __SC_LONG,	\
						      __VA_ARGS__));	\
	static inline int __do_##name##_handler(__MAP(x, __SC_DECL,	\
						      __VA_ARGS__));	\
	static int name##_handler(struct kprobe *p,			\
				  struct pt_regs *regs)			\
	{								\
		return __se_##name##_handler(SC_ARCH_REGS_TO_ARGS(x,	\
							__VA_ARGS__));	\
	}								\
									\
	static inline int __se_##name##_handler(__MAP(x, __SC_LONG,	\
							 __VA_ARGS__))	\
	{								\
		int ret = __do_##name##_handler(__MAP(x, __SC_CAST,	\
						      __VA_ARGS__));	\
		__MAP(x, __SC_TEST, __VA_ARGS__);			\
		return ret;						\
	}								\
	static inline int __do_##name##_handler(__MAP(x, __SC_DECL,	\
						      __VA_ARGS__))

#define KPROBE_HANDLER_DEFINE0(function)				\
	__KPROBE_HANDLER_DEFINE_COMM(name)				\
	static inline int __do_##name##_handler(void);			\
	static int name##_handler(struct kprobe *p,			\
				  struct pt_regs *regs)			\
	{								\
		return __do_##name##_handler();				\
	}								\
	static inline int __do_##name##_handler(void)

#define KPROBE_HANDLER_DEFINE1(function, ...) \
	__KPROBE_HANDLER_DEFINE_x(1, function, __VA_ARGS__)
#define KPROBE_HANDLER_DEFINE2(function, ...) \
	__KPROBE_HANDLER_DEFINE_x(2, function, __VA_ARGS__)
#define KPROBE_HANDLER_DEFINE3(function, ...) \
	__KPROBE_HANDLER_DEFINE_x(3, function, __VA_ARGS__)
#define KPROBE_HANDLER_DEFINE4(function, ...) \
	__KPROBE_HANDLER_DEFINE_x(4, function, __VA_ARGS__)
#define KPROBE_HANDLER_DEFINE5(function, ...) \
	__KPROBE_HANDLER_DEFINE_x(5, function, __VA_ARGS__)
#define KPROBE_HANDLER_DEFINE6(function, ...) \
	__KPROBE_HANDLER_DEFINE_x(6, function, __VA_ARGS__)

/* kprobe do_sys_open */
KPROBE_HANDLER_DEFINE4(do_sys_open,
		       int, dfd, const char __user *, filename,
		       int, flags, umode_t, mode)
{
	return 0;
}

static struct kprobe *kprobes[] = {
	&do_sys_open_kprobe,
};

static int __init kprobe_init(void)
{
	return register_kprobes(kprobes, ARRAY_SIZE(kprobes));
}

static void __exit kprobe_exit(void)
{
	unregister_kprobes(kprobes, ARRAY_SIZE(kprobes));
}

module_init(kprobe_init);
module_exit(kprobe_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Muchun Song <songmuchun@bytedance.com>");
