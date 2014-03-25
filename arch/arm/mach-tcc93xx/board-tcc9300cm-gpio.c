/*
 * linux/arch/arm/mach-tcc93xx/board-tcc9300cm-gpio.c
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
#include <linux/i2c/pca953x.h>
#include "board-tcc9300cm.h"
#include <asm/mach-types.h>

struct board_gpio_irq_config tcc9300_gpio_irqs[] = {
	{ -1, -1 },
};

static struct pca953x_platform_data pca9539_data1 = {
	.gpio_base	= GPIO_PORTEXT1,
};

static struct pca953x_platform_data pca9539_data2 = {
	.gpio_base	= GPIO_PORTEXT2,
};

static struct pca953x_platform_data pca9538_data3 = {
	.gpio_base	= GPIO_PORTEXT3,
};

/* I2C core0 channel 1 devices */
static struct i2c_board_info __initdata i2c_devices4[] = {
	{
		I2C_BOARD_INFO("pca9539", 0x77),
		.platform_data = &pca9539_data1,
	},
	{
		I2C_BOARD_INFO("pca9539", 0x74),
		.platform_data = &pca9539_data2,
	},
	{
		I2C_BOARD_INFO("pca9538", 0x70),
		.platform_data = &pca9538_data3,
	},
};

void __init tcc9300_init_gpio(void)
{
	if (!machine_is_tcc9300cm())
		return 0;

	i2c_register_board_info(4, i2c_devices4, ARRAY_SIZE(i2c_devices4));

	board_gpio_irqs = tcc9300_gpio_irqs;
	printk(KERN_INFO "TCC9300CM GPIO initialized\n");
	return 0;
}
