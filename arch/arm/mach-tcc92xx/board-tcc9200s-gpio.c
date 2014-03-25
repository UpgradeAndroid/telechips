/*
 * linux/arch/arm/mach-tcc92x/board-tcc9200s-gpio.c
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

struct board_gpio_irq_config tcc9200s_gpio_irqs[] = {
#if 0
	/* XXX: disable GPIO interrupt for now */
	{ TCC_GPA(2), INT_EI4 },
	{ TCC_GPA(3), INT_EI5 },
	{ TCC_GPA(12), INT_EI6 },
#endif
#if defined(CONFIG_MMC_TCC_SDHC_CORE0)
	{ TCC_GPA(10), INT_EI8 },	/* SD/MMC slot 0 */
#endif
#if defined(CONFIG_MMC_TCC_SDHC_CORE1)
	{ TCC_GPA(13), INT_EI9 },	/* SD/MMC slot 1 */
#endif	
	{ 0, 0 },
};

int tcc9200s_init_gpio(void)
{
	unsigned int	temp_reg;

	if (!machine_is_tcc9200s())
		return 0;

	/* Initialize GPIO external interrupts */
	writel(HwEINTSEL1_EINT4(SEL_GPIOA2)
	       | HwEINTSEL1_EINT5(SEL_GPIOA3)
	       | HwEINTSEL1_EINT6(SEL_GPIOA12),
	       &HwEINTSEL->EINTSEL1);

#if defined(CONFIG_MMC_TCC_SDHC_CORE0)
		temp_reg = readl(&HwEINTSEL->EINTSEL2);
		writel(HwEINTSEL2_EINT8(SEL_GPIOA10) |temp_reg,   &HwEINTSEL->EINTSEL2);
#endif
	
#if defined(CONFIG_MMC_TCC_SDHC_CORE1)
		temp_reg = readl(&HwEINTSEL->EINTSEL2);
		writel(HwEINTSEL2_EINT9(SEL_GPIOA13) |temp_reg,	 &HwEINTSEL->EINTSEL2); 
#endif	
	

	/* Set level-triggered mode */
	writel(readl(&HwPIC->MODE0)
	       | ((1 << INT_EI6) | (1 << INT_EI5) | (1 << INT_EI4)),
	       &HwPIC->MODE0);

	/* Set interrupt polarity to active low */
	writel(readl(&HwPIC->POL0)
	       | ((1 << INT_EI6) | (1 << INT_EI5) | (1 << INT_EI4)),
	       &HwPIC->POL0);

	board_gpio_irqs = tcc9200s_gpio_irqs;

	printk(KERN_INFO "TCC9200s GPIO initialized\n");
	return 0;
}
postcore_initcall(tcc9200s_init_gpio);
