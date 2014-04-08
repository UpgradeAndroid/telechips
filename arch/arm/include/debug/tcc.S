/*
 * Early serial output macro for Telechips SOCs
 *
 * Copyright (C) 2014 Upgrade Android
 *
 * Ithamar R. Adema <ithamar@upgrade-android.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#define TCC_UART_DEBUG_PHYS_BASE 0xf0532000
#define TCC_UART_DEBUG_VIRT_BASE 0xf0532000

	.macro	addruart, rp, rv, tmp
	ldr	\rp, =TCC_UART_DEBUG_PHYS_BASE
	ldr	\rv, =TCC_UART_DEBUG_VIRT_BASE
	.endm

#define UART_SHIFT	2
#include <asm/hardware/debug-8250.S>
