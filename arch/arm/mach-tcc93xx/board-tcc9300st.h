/*
 * arch/arm/mach-tcc93xx/board-tcc9300st.h
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

#ifndef __ARCH_ARM_MACH_TCC93XX_BOARD_TCC9300ST_H
#define __ARCH_ARM_MACH_TCC93XX_BOARD_TCC9300ST_H

#define GPIO_NAND_RDY		TCC_GPB(29)
#define GPIO_NAND_WP		TCC_GPB(15)

#define GPIO_PWR_KEY		TCC_GPE(29)
#define GPIO_LCD_BL		TCC_GPA(12)
#define GPIO_LCD_DISPLAY	TCC_GPE(1)


#define GPIO_LCD_ON		TCC_GPEXT1(0)
#define GPIO_CODEC_ON		TCC_GPEXT1(1)
#define GPIO_SD0_ON		TCC_GPF(17)
#define GPIO_SD1_ON		TCC_GPEXT1(3)
#define GPIO_SD2_ON		TCC_GPEXT1(4)
#define GPIO_EXT_CODEC_ON	TCC_GPEXT1(5)
#define GPIO_GPS_PWREN		TCC_GPEXT1(6)
#define GPIO_BT_ON	        TCC_GPEXT1(7)
#define GPIO_DXB_ON		TCC_GPEXT1(8)
#define GPIO_IPOD_ON		TCC_GPEXT1(9)
#define GPIO_CAS_ON		TCC_GPEXT1(10)
#define GPIO_FMTC_ON		TCC_GPEXT1(11)
#define GPIO_P-CAM-PWR_ON	TCC_GPEXT1(12)
#define GPIO_CAM1_ON		TCC_GPEXT1(13)
#define GPIO_CAM2_ON		TCC_GPEXT1(14)
#define GPIO_ATAPI_ON		TCC_GPEXT1(15)

#define GPIO_MUTE_CTL		TCC_GPEXT2(0)
#define GPIO_LVDSIVT_ON		TCC_GPEXT2(1)
#define GPIO_LVDS_LP_CTRL	TCC_GPEXT2(2)
#define GPIO_AUTH_RST		TCC_GPEXT2(3)
#define GPIO_BT_RST		TCC_GPEXT2(4)
#define GPIO_SDWF_RST		TCC_GPEXT2(5)
#define GPIO_CAS_RST		TCC_GPEXT2(6)
#define GPIO_CAS_GP		TCC_GPEXT2(6)
#define GPIO_DXB1_PD		TCC_GPEXT2(7)
#define GPIO_DXB2_PD		TCC_GPEXT2(7)
#define GPIO_PWR_CONTROL0	TCC_GPEXT2(8)
#define GPIO_PRW_CONTROL1	TCC_GPEXT2(9)
#define GPIO_HDD_RST		TCC_GPEXT2(10)
#define GPIO_OTG0_VBUS_EN	TCC_GPEXT2(11)
#define GPIO_OTG1_VBUS_EN	TCC_GPEXT2(12)
#define GPIO_HOST_VBUS_EN	TCC_GPEXT2(13)

#define GPIO_FMT_RST		TCC_GPEXT3(0)
#define GPIO_FMT_IRQ		TCC_GPEXT3(1)
#define GPIO_BT_WAKE		TCC_GPEXT3(2)
#define GPIO_BT_HWAKE		TCC_GPEXT3(3)
#define GPIO_PHY2_ON		TCC_GPEXT3(4)
#define GPIO_COMPASS_RST	TCC_GPEXT3(5)
#define GPIO_CAM_FL_EN		TCC_GPEXT3(6)
#define GPIO_CAM2_FL_EN		TCC_GPEXT3(7)
#define GPIO_CAM2_RST		TCC_GPEXT3(8)
#define GPIO_CAM2_PWDN	        TCC_GPEXT3(9)
#define GPIO_LCD_RESET		TCC_GPEXT3(10)
#define GPIO_TV_SLEEP		TCC_GPEXT3(11)
#define GPIO_ETH_ON		TCC_GPEXT3(12)
#define GPIO_ETH_RST		TCC_GPEXT3(13)
#define GPIO_SMART_AUX1		TCC_GPEXT3(14)
#define GPIO_SMART_AUX2		TCC_GPEXT3(15)

// DxB
#define GPIO_DXB1_SFRM		TCC_GPE(8)
#define GPIO_DXB1_SCLK		TCC_GPE(9)
#define GPIO_DXB1_SDI		TCC_GPE(10)
#define GPIO_DXB1_SDO		TCC_GPE(11)
#define GPIO_DXB1_RST		TCC_GPE(13)
#define INT_DXB1_IRQ		TCC_GPA(3)
#define GPIO_DXB1_PWDN		TCC_GPE(12)
#define GPIO_TSIF_ON		TCC_GPF(6)

#endif
