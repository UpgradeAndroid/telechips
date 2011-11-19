/*
 * linux/arch/arm/mach-tcc92x/board-m801_88-gpio.c
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
#include <linux/i2c.h>
#include "board-m801_88.h"
#include <asm/mach-types.h>

struct board_gpio_irq_config m801_88_gpio_irqs[] = {
	{ -1, -1 },
};

/* I2C core0 channel0 devices */
static struct i2c_board_info __initdata i2c_devices0[] = {
};

void __init m801_88_init_gpio(void)
{
	if (!machine_is_m801_88())
		return;

	i2c_register_board_info(0, i2c_devices0, ARRAY_SIZE(i2c_devices0));

	board_gpio_irqs = m801_88_gpio_irqs;
	printk(KERN_INFO "M801_88 GPIO initialized\n");
	return;
}
