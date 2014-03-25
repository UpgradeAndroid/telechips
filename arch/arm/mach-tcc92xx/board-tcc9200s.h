/*
 * arch/arm/mach-tcc92xx/board-tcc9200s.h
 *
 * Copyright (C) 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ARCH_ARM_MACH_TCC92XX_BOARD_TCC9200S_H
#define __ARCH_ARM_MACH_TCC92XX_BOARD_TCC9200S_H


// Nand
#define GPIO_NAND_RDY		TCC_GPB(31)
#define GPIO_NAND_WP		TCC_GPB(22)

// DxB
#define GPIO_DXB1_PD		TCC_GPE(3)
#define GPIO_DXB1_RST		TCC_GPE(11)
#define GPIO_DXB0_PD		TCC_GPE(5)
#define GPIO_DXB0_RST		TCC_GPD(8)
#define INT_DXB1_IRQ		TCC_GPA(11)

#endif
