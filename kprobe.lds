/*
 * The kprobes linker script.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
SECTIONS {
	. = ALIGN(8);
	.__kprobe_template : {
		__start_kprobe_template = .;
		KEEP(*(.__kprobe_template))
		__stop_kprobe_template = .;
		/*
		 * Avoid compiler warning when there is only
		 * kretprobes.
		 */
		KEEP(*(.__kprobe_dummy))
	}

	. = ALIGN(8);
	.__kretprobe_template : {
		__start_kretprobe_template = .;
		KEEP(*(.__kretprobe_template))
		__stop_kretprobe_template = .;
		/*
		 * Avoid compiler warning when there is only
		 * kprobes.
		 */
		KEEP(*(.__kretprobe_dummy))
	}
}
