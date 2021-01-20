// SPDX-License-Identifier: GPL-3.0
/*
 * kprobe.c
 *
 * Here's a sample kernel module showing the use of probes.
 */
#define pr_fmt(fmt) CONFIG_MODULE_NAME ": " fmt

#include "kprobe.h"
#include <linux/sched.h>

static unsigned int rwsem_owner_pid;
module_param(rwsem_owner_pid, uint, 0);

static int wakeup_miss_task(void)
{
	struct task_struct *task;
	struct mm_struct *mm;
	struct rw_semaphore *sem;
	struct task_struct *(*find_task_sym)(pid_t vnr);
	int ret = -EINVAL;

	find_task_sym = (void *)kallsyms_lookup_name("find_task_by_vpid");
	if (!find_task_sym)
		return -ENODEV;

	rcu_read_lock();
	task = find_task_sym(rwsem_owner_pid);
	if (!task)
		goto out;

	mm = task->mm;
	if (!mm)
		goto out;

	sem = &mm->mmap_sem;
	if (atomic_long_read(&sem->count) != 0xfffffffeffffffff)
		goto out;

	atomic_long_inc(&sem->count);
	ret = 0;
out:
	rcu_read_unlock();

	return ret;
}
KPROBE_INITCALL(wakeup_miss_task, NULL);
