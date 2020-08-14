/* SPDX-License-Identifier: GPL-2.0 */
#undef PRINT_EVENT_SYSTEM
#define PRINT_EVENT_SYSTEM kprobe_print

#if !defined(_KPROBE_PRINT_H) || defined(TRACE_HEADER_MULTI_READ)
#define _KPROBE_PRINT_H

#include "trace.h"


PRINT_EVENT_DEFINE(kmemleak,

	PE_PROTO(const void *ptr, const void *call_site),

	PE_ARGS(ptr, call_site),

	PE_STRUCT__entry(
		__field(	const void *,	ptr			)
		__field(	const void *,	call_site		)
	),

	PE_fast_assign(
		__entry->ptr		= ptr;
		__entry->call_site	= call_site;
	),

	PE_printk("kmemleak: 0x%px, caller: %pS", __entry->ptr, __entry->call_site)
);

#endif /* _KPROBE_PRINT_H */

/* This part must be outside protection */
#include "define_trace.h"
