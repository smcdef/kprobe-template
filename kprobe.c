// SPDX-License-Identifier: GPL-3.0
/*
 * kprobe.c
 *
 * Here's a sample kernel module showing the use of return probes.
 */
#define pr_fmt(fmt) "kprobe: " fmt

#include "kprobe.h"

#ifdef KRETPROBE_TEST
/* kretprobe inode_permission entry */
KRETPROBE_ENTRY_HANDLER_DEFINE2(inode_permission, int *, mask_arg,
				struct inode *, inode, int, mask)
{
	if (!inode->i_rdev || inode->i_ino != 1033)
		return -1;

	*mask_arg = mask;

	return 0;
}

/* kretprobe inode_permission return */
KRETPROBE_RET_HANDLER_DEFINE(inode_permission, int *, mask, int, retval)
{
	pr_info("mask: 0x%x, retval: %d\n", *mask, retval);
	return 0;
}
#else
#ifndef PROBE_OFFSET_TEST
/* kprobe inode_permission */
KPROBE_HANDLER_DEFINE2(inode_permission,
		       struct inode *, inode, int, mask)
{
	if (!inode->i_rdev || inode->i_ino != 1033)
		return 0;

	return 0;
}
#else
/* kprobe inode_permission offset */
KPROBE_HANDLER_DEFINE_OFFSET(inode_permission, 0,
			     struct pt_regs *, regs)
{
	struct inode *inode = (void *)arg0(regs);

	if (!inode->i_rdev || inode->i_ino != 1033)
		return 0;

	return 0;
}
#endif /* PROBE_OFFSET_TEST */
#endif /* KRETPROBE_TEST */
