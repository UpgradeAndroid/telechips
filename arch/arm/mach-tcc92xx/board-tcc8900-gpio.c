/*
 * linux/arch/arm/mach-tcc92x/board-tcc8900-gpio.c
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
#include "board-tcc8900.h"

struct board_gpio_irq_config tcc8900_gpio_irqs[] = {
#if defined(CONFIG_MMC_TCC_SDHC_CORE0)
	{ TCC_GPA(10), INT_EI8 },	/* SD/MMC slot 0 */
#endif	
#if defined(CONFIG_MMC_TCC_SDHC_CORE1)
	{ TCC_GPA(6), INT_EI9 },	/* SD/MMC slot 1 */
#endif	
	{ 0, 0 },
};

int tcc8900_init_gpio(void)
{
	unsigned int    temp_reg;

	if (!machine_is_tcc8900())
		return 0;


#if defined(CONFIG_MMC_TCC_SDHC_CORE0)
	temp_reg = readl(&HwEINTSEL->EINTSEL2);
	writel(HwEINTSEL2_EINT8(SEL_GPIOA10) |temp_reg,   &HwEINTSEL->EINTSEL2);
#endif

#if defined(CONFIG_MMC_TCC_SDHC_CORE1)
	temp_reg = readl(&HwEINTSEL->EINTSEL2);
	writel(HwEINTSEL2_EINT9(SEL_GPIOA6) |temp_reg,   &HwEINTSEL->EINTSEL2);	
#endif	

	board_gpio_irqs = tcc8900_gpio_irqs;

	printk(KERN_INFO "TCC8900 GPIO initialized\n");
	return 0;
}
postcore_initcall(tcc8900_init_gpio);
