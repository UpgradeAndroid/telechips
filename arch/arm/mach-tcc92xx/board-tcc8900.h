/*
 * arch/arm/mach-tcc92xx/board-tcc8900.h
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

#ifndef __ARCH_ARM_MACH_TCC92XX_BOARD_TCC8900_H
#define __ARCH_ARM_MACH_TCC92XX_BOARD_TCC8900_H

// Nand
#define GPIO_NAND_RDY		TCC_GPB(28)
#define GPIO_NAND_WP		TCC_GPB(31)

#define GPIO_PWR_KEY		TCC_GPA(3)
#define GPIO_LCD_BL		TCC_GPA(7)
#define GPIO_LCD_DISPLAY	TCC_GPC(28)
#define GPIO_LCD_RESET		TCC_GPD(22)

#define GPIO_ATAPI_ON		TCC_GPEXT1(0)
#define GPIO_LCD_ON		TCC_GPEXT1(1)
#define GPIO_LVDSIVT_ON		TCC_GPEXT1(2)
#define GPIO_CAM_ON		TCC_GPEXT1(3)
#define GPIO_CODEC_ON		TCC_GPEXT1(4)
#define GPIO_FMTC_ON		TCC_GPEXT1(5)
#define GPIO_SD0_ON		TCC_GPEXT1(6)
#define GPIO_SD1_ON		TCC_GPEXT1(7)
#define GPIO_BT_ON		TCC_GPEXT1(8)
#define GPIO_CAS_ON		TCC_GPEXT1(9)
#define GPIO_CAN_ON		TCC_GPEXT1(10)
#define GPIO_ETH_ON		TCC_GPEXT1(11)
#define GPIO_DXB_ON		TCC_GPEXT1(12)
#define GPIO_IPOD_ON		TCC_GPEXT1(13)
#define GPIO_PWR_GP4		TCC_GPEXT1(14)
#define GPIO_LVDS_LP_CTRL	TCC_GPEXT1(15)

#define GPIO_ETH_RST		TCC_GPEXT2(0)
#define GPIO_DXB0_RST		TCC_GPEXT2(1)
#define GPIO_CAM_RST		TCC_GPEXT2(2)
#define GPIO_CAS_RST		TCC_GPEXT2(3)
#define GPIO_AUTH_RST		TCC_GPEXT2(4)
#define GPIO_FM_RST		TCC_GPEXT2(5)
#define GPIO_SATA_ON		TCC_GPEXT2(6)
#define GPIO_RTC_RST		TCC_GPEXT2(6)
#define GPIO_HDMI_ON		TCC_GPEXT2(7)
#define GPIO_BT_WAKE		TCC_GPEXT2(7)
#define GPIO_DXB0_IRQ		TCC_GPEXT2(8)
#define GPIO_BT_HWAKE		TCC_GPEXT2(9)
#define GPIO_FM_IRQ		TCC_GPEXT2(10)
#define GPIO_CP_READY		TCC_GPEXT2(11)
#define GPIO_DXB_GP0		TCC_GPEXT2(12)
#define GPIO_LCD_BL_EN		TCC_GPEXT2(13)
#define GPIO_CAM_FL_EN		TCC_GPEXT2(13)
#define GPIO_MUTE_CTL		TCC_GPEXT2(14)
#define GPIO_CAS_GP		TCC_GPEXT2(15)
#define GPIO_TV_SLEEP		TCC_GPEXT2(15)
#define GPIO_HDD_RST		TCC_GPEXT2(15)

#define GPIO_DVBUS_ON		TCC_GPEXT3(0)
#define GPIO_HVBUS_ON		TCC_GPEXT3(1)
#define GPIO_HDMI_LVDS_ON	TCC_GPEXT3(2)
#define GPIO_PWR_GP0		TCC_GPEXT3(3)
#define GPIO_PWR_GP1		TCC_GPEXT3(4)
#define GPIO_PWR_GP2		TCC_GPEXT3(5)
#define GPIO_PWR_GP3		TCC_GPEXT3(6)
#define GPIO_VCORE_CTL		TCC_GPEXT3(7)

// DxB
#define GPIO_DXB1_PD		TCC_GPE(3)
#define GPIO_DXB1_SFRM		TCC_GPE(8)
#define GPIO_DXB1_SCLK		TCC_GPE(9)
#define GPIO_DXB1_SDI		TCC_GPE(10)
#define GPIO_DXB1_RST		TCC_GPE(11)
#define INT_DXB1_IRQ		TCC_GPA(11)

#define GPIO_DXB0_PD		TCC_GPD(12)//TCC_GPD(10)
#define GPIO_DXB0_SFRM		TCC_GPC(31)//TCC_GPD(5)
#define GPIO_DXB0_SCLK		TCC_GPC(30)//TCC_GPD(6)
#define GPIO_DXB0_SDI		TCC_GPC(29)//TCC_GPD(7)
#define GPIO_DXB0_RST		TCC_GPE(2)//TCC_GPD(8)
#define INT_DXB0_IRQ		TCC_GPF(26)//TCC_GPD(9)

#define GPIO_DXB_UARTTX		TCC_GPE(4)
#define GPIO_DXB_UARTRX		TCC_GPE(5)

#endif
