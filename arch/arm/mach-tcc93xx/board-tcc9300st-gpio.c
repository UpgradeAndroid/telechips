/*
 * linux/arch/arm/mach-tcc93xx/board-tcc9300st-gpio.c
 *
 * Copyright (C) 2009, 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <mach/gpio.h>
#include "board-tcc9300st.h"
#include <asm/mach-types.h>

int tcc9300_init_gpio(void)
{
	if (!machine_is_tcc9300st())
		return 0;

	printk(KERN_INFO "TCC9300STB GPIO initialized\n");
	return 0;
}
