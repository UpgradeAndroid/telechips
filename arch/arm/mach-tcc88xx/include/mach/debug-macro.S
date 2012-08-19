/*
 * debug-macro.S
 *
 * Based on:    linux/arch/arm/kernel/debug.S by Ben Dooks
 * Author: <linux@telechips.com>
 * Created: February 10, 2009
 * Description: Debugging macro include header
 *
 * Copyright (C) 1994-1999 Russell King
 * Copyright (C) 2009 Telechips
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

		.macro	addruart,rp,rv
		mov	 \rp, #0xf0000000	@ physical base address
		orr	\rp, \rp, #0x00530000   @ debug port UART0
		orr	\rp, \rp, #0x1000	@ debug port UART0 0xf00531000
		mov	\rv, \rp
		.endm

		.macro	senduart,rd,rx
		strb	\rd, [\rx]
		.endm

		.macro	busyuart,rd,rx
		.endm

		.macro	waituart,rd,rx
1001:
		ldr \rd, [\rx, #0x14]
		tst \rd, #0x20

		beq 1001b
		.endm
