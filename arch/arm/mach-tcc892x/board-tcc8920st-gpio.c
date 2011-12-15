/*
 * linux/arch/arm/mach-tcc892x/board-tcc8920-gpio.c
 *
 * Copyright (C) 2011 Telechips, Inc.
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
#include <linux/i2c/pca953x.h>
#include "board-tcc8920st.h"
#include <asm/mach-types.h>

struct board_gpio_irq_config tcc8920_gpio_irqs[] = {
	{ -1, -1 },
};

/* I2C core0 channel0 devices */
static struct i2c_board_info __initdata i2c_devices0[] = {
};

void __init tcc8920_init_gpio(void)
{
	if (!machine_is_tcc8920st())
		return;

	i2c_register_board_info(0, i2c_devices0, ARRAY_SIZE(i2c_devices0));

	board_gpio_irqs = tcc8920_gpio_irqs;
	printk(KERN_INFO "TCC8920ST GPIO initialized\n");
	return;
}
