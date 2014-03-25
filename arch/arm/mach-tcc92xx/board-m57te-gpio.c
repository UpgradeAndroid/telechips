/*
 * linux/arch/arm/mach-tcc92x/board-m57te-gpio.c
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
#include "board-m57te.h"

struct board_gpio_irq_config m57te_gpio_irqs[] = {
//	{ TCC_GPA(6), INT_EI8 },	/* SD/MMC slot 0 */
	{ TCC_GPA(10), INT_EI8 },	/* SD/MMC slot 1 */
	{ 0, 0 },
};

int m57te_init_gpio(void)
{
	if (!machine_is_m57te())
		return 0;

	writel(HwEINTSEL2_EINT8(SEL_GPIOA10)
	  /*     | HwEINTSEL2_EINT8(SEL_GPIOA6)*/,
	       &HwEINTSEL->EINTSEL2);

	board_gpio_irqs = m57te_gpio_irqs;

	printk(KERN_INFO "M57TE GPIO initialized\n");
	return 0;
}
postcore_initcall(m57te_init_gpio);
