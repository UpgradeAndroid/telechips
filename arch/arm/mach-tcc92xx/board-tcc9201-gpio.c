/*
 * linux/arch/arm/mach-tcc92x/board-tcc9201-gpio.c
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
#include <linux/io.h>
#include <linux/irq.h>
#include <asm/mach-types.h>
#include <mach/gpio.h>

#include <mach/bsp.h>

int gpio_to_irq(unsigned gpio)
{
	return -EINVAL;
}
EXPORT_SYMBOL(gpio_to_irq);

int tcc9201_init_gpio(void)
{
	if (!machine_is_tcc9201())
		return 0;

	printk(KERN_INFO "TCC9201 GPIO initialized\n");
	return 0;
}
postcore_initcall(tcc9201_init_gpio);
