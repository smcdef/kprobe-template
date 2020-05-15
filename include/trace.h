/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _TRACE_EVENT_H
#define _TRACE_EVENT_H
#include <linux/kernel.h>
#include <linux/trace_seq.h>
#include <linux/sizes.h>
#include <linux/ring_buffer.h>
#include <linux/trace_events.h>

/*
 * The print entry - the most basic unit of tracing.
 */
struct print_event_entry {
	unsigned short	id;
};

struct print_event_class {
	unsigned short id;
	enum print_line_t (*format)(struct trace_seq *seq,
				    struct print_event_entry *entry);
	struct ring_buffer *buffer;
};

#define RB_BUFFER_SIZE	SZ_32K
#define PRINT_EVENT_DEFINE(name, proto, args, tstruct, assign, print)

#endif /* _TRACE_EVENT_H */
