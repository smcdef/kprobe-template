/* SPDX-License-Identifier: GPL-2.0 */
#undef PRINT_EVENT_SYSTEM
#define PRINT_EVENT_SYSTEM kprobe_print

#if !defined(_KPROBE_PRINT_H) || defined(TRACE_HEADER_MULTI_READ)
#define _KPROBE_PRINT_H

#include "trace.h"

PRINT_EVENT_DEFINE(signal_generate,

	PE_PROTO(int sig, struct task_struct *task,
		 int group, const char *result),

	PE_ARGS(sig, task, group, result),

	PE_STRUCT__entry(
		__array(	char,	from_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	from_pid			)
		__field(	int,	sig				)
		__array(	char,	to_comm,	TASK_COMM_LEN	)
		__field(	pid_t,	to_pid				)
		__field(	const char *,	group			)
		__field(	const char *,	result			)
	),

	PE_fast_assign(
		memcpy(__entry->from_comm, current->comm, TASK_COMM_LEN);
		__entry->from_pid	= current->pid;
		__entry->sig		= sig;
		memcpy(__entry->to_comm, task->comm, TASK_COMM_LEN);
		__entry->to_pid		= task->pid;
		__entry->group		= group ? "group" : "share";
		__entry->result		= result;
	),

	PE_printk("%s(%d) send signal(%d) to %s %s(%d) with %s",
		  __entry->from_comm, __entry->from_pid, __entry->sig,
		  __entry->group, __entry->to_comm, __entry->to_pid,
		  __entry->result)
);

PRINT_EVENT_DEFINE(inode_permission,

	PE_PROTO(int mask, int retval),

	PE_ARGS(mask, retval),

	PE_STRUCT__entry(
		__field(	int,	mask			)
		__field(	int,	retval			)
	),

	PE_fast_assign(
		__entry->mask	= mask;
		__entry->retval	= retval;
	),

	PE_printk("mask: 0x%x, retval: %d", __entry->mask, __entry->retval)
);

#endif /* _KPROBE_PRINT_H */

/* This part must be outside protection */
#include "define_trace.h"
