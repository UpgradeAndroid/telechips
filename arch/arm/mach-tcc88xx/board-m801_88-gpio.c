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
#include <mach/bsp.h>
#include <mach/irqs.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include "board-m801_88.h"
#include <asm/mach-types.h>

static struct board_gpio_irq_config m801_88_gpio_irqs[] = {
	/* Power Key */
	{
		.gpio = TCC_GPA(3),
		.irq = INT_TSD, /* is really INT_EI3 */
	/* Touchscreen IC pen down interrupt/GPIO */
	}, {
		.gpio = TCC_GPB(31),
		.irq = INT_EI2,
	}, {
		.gpio = -1,
		.irq = -1,
	},
};


void __init m801_88_init_gpio(void)
{
	volatile PGPIO pGPIO = (volatile PGPIO)tcc_p2v(HwGPIO_BASE);
	volatile PPIC pPIC = (volatile PPIC)tcc_p2v(HwVPIC_BASE);

	if (!machine_is_m801_88())
		return;

	BITCSET(pGPIO->EINTSEL0, HwEINTSEL0_EINT2_MASK, HwEINTSEL0_EINT2(SEL_GPIOB31));
	BITCSET(pGPIO->EINTSEL0, HwEINTSEL0_EINT3_MASK, HwEINTSEL0_EINT3(SEL_GPIOA3));

	pr_info("EI37SEL: %08x\n", pPIC->EI37SEL);
	pPIC->EI37SEL |= (1 << 3); /* 0 for TS demux, 1 for Ext. INT 3 */

	board_gpio_irqs = m801_88_gpio_irqs;
	printk(KERN_INFO "M801_88 GPIO initialized\n");
}
