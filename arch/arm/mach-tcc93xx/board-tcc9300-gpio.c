/*
 * linux/arch/arm/mach-tcc93xx/board-tcc9300-gpio.c
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
#include "board-tcc9300.h"
#include <asm/mach-types.h>

static struct pca953x_platform_data pca9539_data1 = {
	.gpio_base	= GPIO_PORTEXT1,
};

static struct pca953x_platform_data pca9539_data2 = {
	.gpio_base	= GPIO_PORTEXT2,
};

static struct pca953x_platform_data pca9539_data3 = {
	.gpio_base	= GPIO_PORTEXT3,
};

static struct pca953x_platform_data pca9539_data4 = {
	.gpio_base	= GPIO_PORTEXT14,
};

/* I2C core1 channel 0 devices */
static struct i2c_board_info __initdata i2c_devices2[] = {
	{
		I2C_BOARD_INFO("pca9539", 0x74),
		.platform_data = &pca9539_data1,
	},
	{
		I2C_BOARD_INFO("pca9539", 0x77),
		.platform_data = &pca9539_data2,
	},
	{
		I2C_BOARD_INFO("pca9539", 0x75),
		.platform_data = &pca9539_data3,
	},
	{
		I2C_BOARD_INFO("pca9538", 0x70),
		.platform_data = &pca9539_data4,
	},	
};

int tcc9300_init_gpio(void)
{
	if (!machine_is_tcc9300())
		return 0;

	i2c_register_board_info(2, i2c_devices2, ARRAY_SIZE(i2c_devices2));

	printk(KERN_INFO "TCC9300 GPIO initialized\n");
	return 0;
}
