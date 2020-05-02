// SPDX-License-Identifier: GPL-3.0
/*
 * kprobe.c
 *
 * Here's a sample kernel module showing the use of return probes.
 */
#define pr_fmt(fmt) "kprobes: " fmt

#include "kprobe.h"

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
	pr_info("mask: 0x%x, retval: %d\n", *mask, retval);
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
