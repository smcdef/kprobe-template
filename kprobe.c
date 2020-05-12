// SPDX-License-Identifier: GPL-3.0
/*
 * kprobe.c
 *
 * Here's a sample kernel module showing the use of probes.
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "kprobe.h"

#define CREATE_PRINT_EVENT
#include "kprobe_print.h"

/* kretprobe inode_permission entry */
KRETPROBE_ENTRY_HANDLER_DEFINE2(inode_permission, int *, private,
				struct inode *, inode, int, mask)
{
	if (!inode->i_rdev || inode->i_ino != 1033)
		return -1;

	*private = mask;

	return 0;
}

/* kretprobe inode_permission return */
KRETPROBE_RET_HANDLER_DEFINE(inode_permission, int *, mask, int, retval)
{
	inode_permission_print(*mask, retval);
	return 0;
}

/* kprobe do_sys_open */
KPROBE_HANDLER_DEFINE4(do_sys_open,
		       int, dfd, const char __user *, filename,
		       int, flags, umode_t, mode)
{
	return 0;
}

/* kprobe __close_fd */
KPROBE_HANDLER_DEFINE2(__close_fd,
		       struct files_struct *, files, unsigned, fd)
{
	return 0;
}

/* tracepoint signal_generate */
TRACEPOINT_HANDLER_DEFINE(signal_generate,
			  int sig, struct siginfo *info,
			  struct task_struct *task, int group, int result)
{
	static const char *result_name[] = {
		"deliverd",
		"ignored",
		"already_pending",
		"overflow_fail",
		"lose_info",
	};

	signal_generate_print(sig, task, group, result_name[result]);
}
