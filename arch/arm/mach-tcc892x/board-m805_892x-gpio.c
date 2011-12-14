/*
 * linux/arch/arm/mach-tcc892x/board-m805_892x-gpio.c
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
#include "board-m805_892x.h"
#include <asm/mach-types.h>

struct board_gpio_irq_config m805_892x_gpio_irqs[] = {
	{ -1, -1 },
};

#if (0)
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
	.gpio_base	= GPIO_PORTEXT4,
};

static struct pca953x_platform_data pca9539_data5 = {
	.gpio_base	= GPIO_PORTEXT5,
};
#endif

/* I2C core0 channel0 devices */
static struct i2c_board_info __initdata i2c_devices0[] = {
#if (0)
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
	{	// SV60 Power Board
		I2C_BOARD_INFO("pca9539", 0x76),
		.platform_data = &pca9539_data5,
	},
#endif
};

void __init m805_892x_init_gpio(void)
{
	if (!machine_is_m805_892x())
		return;

	i2c_register_board_info(0, i2c_devices0, ARRAY_SIZE(i2c_devices0));

	board_gpio_irqs = m805_892x_gpio_irqs;
	printk(KERN_INFO "M805_892X GPIO initialized\n");
	return;
}
