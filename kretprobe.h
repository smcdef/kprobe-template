// SPDX-License-Identifier: GPL-3.0
/*
 * kretprobe.h
 *
 * Here's a sample kernel module showing the use of return probes.
 */
#ifndef __KRETPROBE_TEMPLATE_H
#define __KRETPROBE_TEMPLATE_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/kprobes.h>
#include <asm/irq_regs.h>

#if defined(CONFIG_X86_64)
#define SC_ARCH_REGS_TO_ARGS(x, ...)					\
	__MAP(x,__SC_ARGS						\
	      ,,regs->di,,regs->si,,regs->dx				\
	      ,,regs->r10,,regs->r8,,regs->r9)

#define arg0(regs)	(regs->di)
#define arg1(regs)	(regs->si)
#define arg2(regs)	(regs->dx)
#define arg3(regs)	(regs->r10)
#define arg4(regs)	(regs->r8)
#define arg5(regs)	(regs->r9)
#elif defined(CONFIG_ARM64)
#define SC_ARCH_REGS_TO_ARGS(x, ...)					\
	__MAP(x,__SC_ARGS						\
	      ,,regs->regs[0],,regs->regs[1],,regs->regs[2]		\
	      ,,regs->regs[3],,regs->regs[4],,regs->regs[5])

#define arg0(regs)	(regs->regs[0])
#define arg1(regs)	(regs->regs[1])
#define arg2(regs)	(regs->regs[2])
#define arg3(regs)	(regs->regs[3])
#define arg4(regs)	(regs->regs[4])
#define arg5(regs)	(regs->regs[5])
#else
#error "Unsupported architecture"
#endif

#define __KPROBE_ENTRY_HANDLER_DEFINE_COMM(name, type, off)		\
	extern type __kretprobe_private_size;				\
	static int name##_entry_handler(struct kretprobe_instance *ri,	\
					struct pt_regs *regs);		\
	static int name##_ret_handler(struct kretprobe_instance *ri,	\
				      struct pt_regs *regs);		\
	static struct kretprobe name##_kretprobe = {			\
		.kp.symbol_name	= #name,				\
		.kp.offset	= off,					\
		.handler	= name##_ret_handler,			\
		.entry_handler	= name##_entry_handler,			\
		.data_size	= sizeof(*__kretprobe_private_size),	\
		.maxactive	= 0,					\
	};								\
	static inline int __init name##_register(void)			\
	{								\
		int ret;						\
									\
		ret = register_kretprobe(&name##_kretprobe);		\
		if (ret < 0)						\
			pr_err("register kretprobe(%s+0x%x) fail"	\
			       " returned %d\n", #name, off, ret);	\
		else							\
			pr_info("planted return probe at %pS\n",	\
				name##_kretprobe.kp.addr);		\
		return ret;						\
	}								\
	module_init(name##_register);					\
									\
	static inline void __exit name##_unregister(void)		\
	{								\
		int nmissed = name##_kretprobe.nmissed;			\
									\
		if (nmissed)						\
			pr_info("missed probing %d instances of %pS\n",	\
				nmissed,				\
				name##_kretprobe.kp.addr);		\
		unregister_kretprobe(&name##_kretprobe);		\
		pr_info("kretprobe at %pS unregistered\n",		\
			name##_kretprobe.kp.addr);			\
	}								\
	module_exit(name##_unregister);					\
									\
	MODULE_LICENSE("GPL");						\
	MODULE_AUTHOR("Muchun Song <songmuchun@bytedance.com>");

#define __KPROBE_ENTRY_HANDLER_DEFINE_x(x, name, type, arg, ...)	\
	__KPROBE_ENTRY_HANDLER_DEFINE_COMM(name, type, 0)		\
	static inline int __se_##name##_entry_handler(type arg, 	\
			__MAP(x, __SC_LONG, __VA_ARGS__));		\
	static inline int __do_##name##_entry_handler(type arg, 	\
			__MAP(x, __SC_DECL, __VA_ARGS__));		\
	static int name##_entry_handler(struct kretprobe_instance *ri,	\
					struct pt_regs *regs)		\
	{								\
		return __se_##name##_entry_handler((type)ri->data,	\
				SC_ARCH_REGS_TO_ARGS(x, __VA_ARGS__));	\
	}								\
									\
	static inline int __se_##name##_entry_handler(type arg,		\
			__MAP(x, __SC_LONG, __VA_ARGS__))		\
	{								\
		int ret = __do_##name##_entry_handler(arg,		\
				__MAP(x, __SC_CAST, __VA_ARGS__));	\
		__MAP(x, __SC_TEST, __VA_ARGS__);			\
		return ret;						\
	}								\
	static inline int __do_##name##_entry_handler(type arg,		\
			__MAP(x, __SC_DECL, __VA_ARGS__))

#define __KPROBE_RET_HANDLER_DEFINE(func, ...)				\
	static inline int __se_##func##_ret_handler(__MAP(2, __SC_LONG,	\
			__VA_ARGS__));					\
	static inline int __do_##func##_ret_handler(__MAP(2, __SC_DECL,	\
			__VA_ARGS__));					\
	static int func##_ret_handler(struct kretprobe_instance *ri,	\
				      struct pt_regs *regs)		\
	{								\
		return __se_##func##_ret_handler((long)ri->data,	\
				(long)regs_return_value(regs));		\
	}								\
									\
	static inline int __se_##func##_ret_handler(__MAP(2, __SC_LONG,	\
			__VA_ARGS__))					\
	{								\
		int ret = __do_##func##_ret_handler(__MAP(2, __SC_CAST,	\
				__VA_ARGS__));				\
		__MAP(2, __SC_TEST, __VA_ARGS__);			\
		return ret;						\
	}								\
	static inline int __do_##func##_ret_handler(__MAP(2, __SC_DECL,	\
			__VA_ARGS__))

#define __KPROBE_ENTRY_HANDLER_DEFINE_OFFSET(func, offset, type, arg, ...) \
	__KPROBE_ENTRY_HANDLER_DEFINE_COMM(func, type, offset)		\
	static inline int __se_##func##_entry_handler(type arg, 	\
			__MAP(1, __SC_DECL, __VA_ARGS__));		\
	static inline int __do_##func##_entry_handler(type arg, 	\
			__MAP(1, __SC_DECL, __VA_ARGS__));		\
	static int func##_entry_handler(struct kretprobe_instance *ri,	\
					struct pt_regs *regs)		\
	{								\
		return __se_##func##_entry_handler((type)ri->data,	\
						   regs);		\
	}								\
									\
	static inline int __se_##func##_entry_handler(type arg,		\
			__MAP(1, __SC_DECL, __VA_ARGS__))		\
	{								\
		int ret = __do_##func##_entry_handler(arg,		\
				__MAP(1, __SC_CAST, __VA_ARGS__));	\
		__MAP(1, __SC_TEST, __VA_ARGS__);			\
		return ret;						\
	}								\
	static inline int __do_##func##_entry_handler(type arg,		\
			__MAP(1, __SC_DECL, __VA_ARGS__))

#define __KPROBE_ENTRY_HANDLER_DEFINE0(func, type, arg)			\
	__KPROBE_ENTRY_HANDLER_DEFINE_COMM(func, type)			\
	static inline int __do_##func##_entry_handler(type arg);	\
	static int func##_entry_handler(struct kretprobe_instance *ri,	\
					struct pt_regs *regs)		\
	{								\
		return __do_##func##_entry_handler((type)ri->data);	\
	}								\
									\
	static inline int __do_##func##_entry_handler(type arg)

/* The below is the user API for kernel module */
#define KPROBE_ENTRY_HANDLER_DEFINE0(func, type, arg)      \
	__KPROBE_ENTRY_HANDLER_DEFINE0(func, type, arg)
#define KPROBE_ENTRY_HANDLER_DEFINE1(func, type, arg, ...) \
	__KPROBE_ENTRY_HANDLER_DEFINE_x(1, func, type, arg, __VA_ARGS__)
#define KPROBE_ENTRY_HANDLER_DEFINE2(func, type, arg, ...) \
	__KPROBE_ENTRY_HANDLER_DEFINE_x(2, func, type, arg, __VA_ARGS__)
#define KPROBE_ENTRY_HANDLER_DEFINE3(func, type, arg, ...) \
	__KPROBE_ENTRY_HANDLER_DEFINE_x(3, func, type, arg, __VA_ARGS__)
#define KPROBE_ENTRY_HANDLER_DEFINE4(func, type, arg, ...) \
	__KPROBE_ENTRY_HANDLER_DEFINE_x(4, func, type, arg, __VA_ARGS__)
#define KPROBE_ENTRY_HANDLER_DEFINE5(func, type, arg, ...) \
	__KPROBE_ENTRY_HANDLER_DEFINE_x(5, func, type, arg, __VA_ARGS__)
#define KPROBE_ENTRY_HANDLER_DEFINE6(func, type, arg, ...) \
	__KPROBE_ENTRY_HANDLER_DEFINE_x(6, func, type, arg, __VA_ARGS__)

#define KPROBE_ENTRY_HANDLER_DEFINE_OFFSET(func, offset, type, arg, ...) \
	__KPROBE_ENTRY_HANDLER_DEFINE_OFFSET(func, offset, type, arg, __VA_ARGS__)

#define KPROBE_RET_HANDLER_DEFINE(func, ...) \
	__KPROBE_RET_HANDLER_DEFINE(func, __VA_ARGS__)

#endif /* __KRETPROBE_TEMPLATE_H */
