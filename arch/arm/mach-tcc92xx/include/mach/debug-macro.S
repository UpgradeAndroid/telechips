/* 
 * linux/include/asm-arm/arch-tcc8900/debug-macro.S
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

		.macro	addruart,rp,rv,tmp
		mov	 \rp, #0xf0000000	@ physical base address
		mov	 \rv, #0xf0000000	@ virtual base
		orr	\rp, \rp, #0x00530000   @ debug port UART0
		orr	\rp, \rp, #0x2000		@ debug port UART0 0xf00532000
		orr	\rv, \rv, #0x00530000   @ debug port UART0
		orr	\rv, \rv, #0x2000		@ debug port UART0 0xf00532000
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
