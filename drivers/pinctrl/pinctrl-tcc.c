/*
 * Telechips SoCs pinctrl driver.
 *
 * Copyright (C) 2014 Upgrade Android
 *
 * Ithamar R. Adema <ithamar@upgrade-android.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "core.h"
#include "pinctrl-tcc.h"

#define GPA(x)	(PA_BASE + (x))
#define GPB(x)	(PB_BASE + (x))
#define GPC(x)	(PC_BASE + (x))
#define GPD(x)	(PD_BASE + (x))
#define GPE(x)	(PE_BASE + (x))
#define GPF(x)	(PF_BASE + (x))

static int tcc92xx_eint_sources[] = {
	GPA(0), GPA(1), GPA(2), GPA(3), GPA(4), GPA(5), GPA(6), GPA(7), GPA(8),
	GPA(9), GPA(10), GPA(11), GPA(12), GPA(13), GPA(14), GPA(15),
	GPD(5), GPD(6), GPD(7), GPD(8), GPD(9), GPD(10), GPD(13), GPD(14),
	GPD(15), GPD(16), GPD(17), GPD(18), GPD(19), GPD(20),
	GPF(23), GPF(24), GPF(25), GPF(26), GPF(27), GPF(20), GPF(17), GPF(13),
	GPF(10), GPF(8),
	GPC(28), GPC(29), GPC(30), GPC(31), GPC(9), GPC(13),
	GPB(28), GPB(29), GPB(30), GPB(31), GPB(8), GPB(12),
	GPE(4), GPE(5), GPE(24), GPE(25),
	/* TSWKU, TSSTOP, TSUPDN, -, -, PMKUP, USB_VBON, USB_VBOFF */
	-1,
};

static const struct tcc_desc_pin tcc92xx_pins[] = {
	TCC_PIN(TCC_PINCTRL_PIN_PA0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2c0")),		/* SCL0 */
	TCC_PIN(TCC_PINCTRL_PIN_PA1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2c0")),		/* SDA0 */
	TCC_PIN(TCC_PINCTRL_PIN_PA2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ckc")),		/* CLK_OUT0 */
	TCC_PIN(TCC_PINCTRL_PIN_PA3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ckc")),		/* CLK_OUT1 */
	TCC_PIN(TCC_PINCTRL_PIN_PA4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "wdt"),		/* WDTRST0 */
		TCC_FUNCTION(0x2, "tco")),		/* TCO0 */
	TCC_PIN(TCC_PINCTRL_PIN_PA5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ir"),		/* IRDI */
		TCC_FUNCTION(0x2, "tco")),		/* TCO1 */
	TCC_PIN(TCC_PINCTRL_PIN_PA6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "hdmi"),		/* HDMI_CECO */
		TCC_FUNCTION(0x2, "tco"),		/* TCO2 */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[19] */
	TCC_PIN(TCC_PINCTRL_PIN_PA7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "hdmi"),		/* HDMI_CECI */
		TCC_FUNCTION(0x2, "tco"),		/* TCO3 */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[20] */
	TCC_PIN(TCC_PINCTRL_PIN_PA8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2c1")),		/* SCL1 */
	TCC_PIN(TCC_PINCTRL_PIN_PA9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2c1")),		/* SDA1 */
	TCC_PIN(TCC_PINCTRL_PIN_PA10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cd0"),		/* CBCLK(0) */
		TCC_FUNCTION(0x2, "cd1")),		/* CBCLK(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PA11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cd0"),		/* CLRCK(0) */
		TCC_FUNCTION(0x2, "cd1")),		/* CLRCK(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PA12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cd0"),		/* CDATA(0) */
		TCC_FUNCTION(0x2, "cd1")),		/* CDATA(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PA13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ckc")),		/* EXTCLKI */
	TCC_PIN(TCC_PINCTRL_PIN_PA14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "hdmi"),		/* HDMI_HPD */
		TCC_FUNCTION(0x2, "tco")),		/* TCO4 */
	TCC_PIN(TCC_PINCTRL_PIN_PA15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "otg"),		/* UTM_DRVVBUS */
		TCC_FUNCTION(0x2, "tco")),		/* TCO5 */

	TCC_PIN(TCC_PINCTRL_PIN_PB0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD8 */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D0(5) */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D0(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PB1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD9 */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D1(5) */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D1(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PB2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD10 */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D2(5) */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D2(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PB3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD11 */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D3(5) */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D3(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PB4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD4 */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D4(5) */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D4(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PB5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD5 */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D5(5) */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D5(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PB6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD6 */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D6(5) */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D6(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PB7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD7 */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D7(5) */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D7(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PB8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD0 */
		TCC_FUNCTION(0x4, "gpsb1")),		/* SFRM(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PB9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD1 */
		TCC_FUNCTION(0x4, "gpsb1")),		/* SCLK(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PB10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD2 */
		TCC_FUNCTION(0x4, "gpsb1")),		/* SDI(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PB11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD3 */
		TCC_FUNCTION(0x4, "gpsb1")),		/* SDO(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PB12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD12 */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_CMD(5) */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_BUS(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PB13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXD13 */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_CLK(5) */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_CLK(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PB14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi")),		/* EDIXD14 */
	TCC_PIN(TCC_PINCTRL_PIN_PB15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi")),		/* EDIXD15 */
	TCC_PIN(TCC_PINCTRL_PIN_PB16,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi")),		/* EDIWEN0 */
	TCC_PIN(TCC_PINCTRL_PIN_PB17,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIWEN1 */
		TCC_FUNCTION(0x4, "gpsb0")),		/* SFRM(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PB18,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi")),		/* EDIOEN0 */
	TCC_PIN(TCC_PINCTRL_PIN_PB19,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIOEN1 */
		TCC_FUNCTION(0x4, "gpsb0")),		/* SCLK(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PB20,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi")),		/* EDIXA[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PB21,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXA[1] */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D4(6) */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D4(6) */
	TCC_PIN(TCC_PINCTRL_PIN_PB22,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIXA[2] */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D5(6) */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D5(6) */
	TCC_PIN(TCC_PINCTRL_PIN_PB23,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDICSN0 */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D6(6) */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D6(6) */
	TCC_PIN(TCC_PINCTRL_PIN_PB24,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDICSN1 */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D7(6) */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D7(6) */
	TCC_PIN(TCC_PINCTRL_PIN_PB25,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDICSN2 */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D0(6) */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D0(6) */
	TCC_PIN(TCC_PINCTRL_PIN_PB26,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDICSN3 */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D1(6) */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D1(6) */
	TCC_PIN(TCC_PINCTRL_PIN_PB27,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDICSN4 */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D2(6) */
		TCC_FUNCTION(0x3, "ms6"),		/* MS_D2(6) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[23] */
	TCC_PIN(TCC_PINCTRL_PIN_PB28,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIRDY0 */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D3(6) */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D3(6) */
	TCC_PIN(TCC_PINCTRL_PIN_PB29,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDIRDY1 */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_CMD(6) */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_BUS(6) */
	TCC_PIN(TCC_PINCTRL_PIN_PB30,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDICSN5 */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_CLK(6) */
		TCC_FUNCTION(0x3, "ms6"),		/* MS_CLK(6) */
		TCC_FUNCTION(0x4, "gpsb0"),		/* SDI(0) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[22] XXX duplicate "edi" func */
	TCC_PIN(TCC_PINCTRL_PIN_PB31,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDICSN6 */
		TCC_FUNCTION(0x4, "gpsb0"),		/* SDO(0) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[21] XXX duplicate "edi" func */

	TCC_PIN(TCC_PINCTRL_PIN_PC0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[0] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[0] */
		TCC_FUNCTION(0x3, "ts3"),		/* TSD5(3) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[0] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PC1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[1] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[1] */
		TCC_FUNCTION(0x3, "ts3"),		/* TSD6(3) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[1] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[1] */
	TCC_PIN(TCC_PINCTRL_PIN_PC2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[2] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[2] */
		TCC_FUNCTION(0x3, "ts3"),		/* TSD7(3) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[2] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[2] */
	TCC_PIN(TCC_PINCTRL_PIN_PC3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[3] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[3] */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[3] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[3] */
	TCC_PIN(TCC_PINCTRL_PIN_PC4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[4] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[4] */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[4] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[4] */
	TCC_PIN(TCC_PINCTRL_PIN_PC5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[5] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[5] */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[5] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[5] */
	TCC_PIN(TCC_PINCTRL_PIN_PC6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[6] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[6] */
		TCC_FUNCTION(0x3, "gpsb3"),		/* SDO(3) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[6] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[6] */
	TCC_PIN(TCC_PINCTRL_PIN_PC7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[7] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[7] */
		TCC_FUNCTION(0x3, "gpsb3"),		/* SDI(3) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[7] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[7] */
	TCC_PIN(TCC_PINCTRL_PIN_PC8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[8] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[8] */
		TCC_FUNCTION(0x3, "gpsb3"),		/* SCLK(3) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[8] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[8] */
	TCC_PIN(TCC_PINCTRL_PIN_PC9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[9] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[9] */
		TCC_FUNCTION(0x3, "gpsb3"),		/* SFRM(3) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[9] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[9] */
	TCC_PIN(TCC_PINCTRL_PIN_PC10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[10] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[10] */
		TCC_FUNCTION(0x3, "gpsb2"),		/* SDO(2) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[10] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[10] */
	TCC_PIN(TCC_PINCTRL_PIN_PC11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[11] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[11] */
		TCC_FUNCTION(0x3, "gpsb2"),		/* SDI(2) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[11] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[11] */
	TCC_PIN(TCC_PINCTRL_PIN_PC12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[9] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[12] */
		TCC_FUNCTION(0x3, "gpsb2"),		/* SCLK(2) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[12] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[12] */
	TCC_PIN(TCC_PINCTRL_PIN_PC13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[13] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[13] */
		TCC_FUNCTION(0x3, "gpsb2"),		/* SFRM(2) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[13] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[13] */
	TCC_PIN(TCC_PINCTRL_PIN_PC14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[14] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[14] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D7(0) */
		TCC_FUNCTION(0x4, "ms0"),		/* MS_D7(0) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[14] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[14] */
	TCC_PIN(TCC_PINCTRL_PIN_PC15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[15] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[15] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D6(0) */
		TCC_FUNCTION(0x4, "ms0"),		/* MS_D6(0) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[15] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[15] */
	TCC_PIN(TCC_PINCTRL_PIN_PC16,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[16] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[16] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D5(0) */
		TCC_FUNCTION(0x4, "ms0"),		/* MS_D5(0) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[16] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[16] */
	TCC_PIN(TCC_PINCTRL_PIN_PC17,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXD[17] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[17] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D4(0) */
		TCC_FUNCTION(0x4, "ms0"),		/* MS_D4(0) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LPD[17] */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[17] */
	TCC_PIN(TCC_PINCTRL_PIN_PC18,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[18] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D3(0) */
		TCC_FUNCTION(0x4, "ms0"),		/* MS_D3(0) */
		TCC_FUNCTION(0x5, "lcd_rgb1")),		/* LCD1_LPD[18] */
	TCC_PIN(TCC_PINCTRL_PIN_PC19,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[19] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D2(0) */
		TCC_FUNCTION(0x4, "ms0"),		/* MS_D2(0) */
		TCC_FUNCTION(0x5, "lcd_rgb1")),		/* LCD1_LPD[19] */
	TCC_PIN(TCC_PINCTRL_PIN_PC20,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[20] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D1(0) */
		TCC_FUNCTION(0x4, "ms0"),		/* MS_D1(0) */
		TCC_FUNCTION(0x5, "lcd_rgb1")),		/* LCD1_LPD[20] */
	TCC_PIN(TCC_PINCTRL_PIN_PC21,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[21] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D0(0) */
		TCC_FUNCTION(0x4, "ms0"),		/* MS_D0(0) */
		TCC_FUNCTION(0x5, "lcd_rgb1")),		/* LCD1_LPD[21] */
	TCC_PIN(TCC_PINCTRL_PIN_PC22,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[22] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_CLK(0) */
		TCC_FUNCTION(0x4, "ms0"),		/* MS_CLK(0) */
		TCC_FUNCTION(0x5, "lcd_rgb1")),		/* LCD1_LPD[22] */
	TCC_PIN(TCC_PINCTRL_PIN_PC23,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LPD[23] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_CMD(0) */
		TCC_FUNCTION(0x4, "ms0"),		/* MS_BUS(0) */
		TCC_FUNCTION(0x5, "lcd_rgb1")),		/* LCD1_LPD[23] */
	TCC_PIN(TCC_PINCTRL_PIN_PC24,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LWEN */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LDE */
		TCC_FUNCTION(0x3, "ts3"),		/* TSD4(3) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LDE */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[19] */
	TCC_PIN(TCC_PINCTRL_PIN_PC25,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LOEN */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LCK */
		TCC_FUNCTION(0x3, "ts3"),		/* TSD3(3) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LCK */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[18] */
	TCC_PIN(TCC_PINCTRL_PIN_PC26,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LXA[0] */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LHS */
		TCC_FUNCTION(0x3, "ts3"),		/* TSD2(3) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LHS */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[22] */
	TCC_PIN(TCC_PINCTRL_PIN_PC27,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LCSN0 */
		TCC_FUNCTION(0x2, "lcd_rgb0"),		/* LCD0_LVS */
		TCC_FUNCTION(0x3, "ts3"),		/* TSD1(3) */
		TCC_FUNCTION(0x5, "lcd_rgb1"),		/* LCD1_LVS */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[20] */
	TCC_PIN(TCC_PINCTRL_PIN_PC28,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd_cpu"),		/* LCSN1 */
		TCC_FUNCTION(0x2, "gpsb10"),		/* SDO(10) */
		TCC_FUNCTION(0x3, "ts3"),		/* TSVALID(3) */
		TCC_FUNCTION(0x6, "gpiof")),		/* GPIOF[21] */
	TCC_PIN(TCC_PINCTRL_PIN_PC29,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "gpsb10"),		/* SDI(10) */
		TCC_FUNCTION(0x3, "ts3")),		/* TSCLK(3) */
	TCC_PIN(TCC_PINCTRL_PIN_PC30,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* EXTLVS0(0) */
		TCC_FUNCTION(0x2, "gpsb10"),		/* SCLK(10) */
		TCC_FUNCTION(0x3, "ts3")),		/* TSD0(3) */
	TCC_PIN(TCC_PINCTRL_PIN_PC31,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd1"),		/* EXTLVS1(0) */
		TCC_FUNCTION(0x2, "gpsb10"),		/* SFRM(10) */
		TCC_FUNCTION(0x3, "ts3")),		/* TSSYNC(3) */

	TCC_PIN(TCC_PINCTRL_PIN_PD0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* BCLK(1) */
		TCC_FUNCTION(0x2, "i2s0")),		/* BCLK(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PD1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* LRCK(1) */
		TCC_FUNCTION(0x2, "i2s0")),		/* LRCK(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PD2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* MCLK(1) */
		TCC_FUNCTION(0x2, "i2s0")),		/* MCLK(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PD3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* DAO0(1) */
		TCC_FUNCTION(0x2, "i2s0")),		/* DAO0(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PD4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* DAI0(1) */
		TCC_FUNCTION(0x2, "i2s0")),		/* DAI0(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PD5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* DAO1(1) */
		TCC_FUNCTION(0x2, "gpsb11")),		/* SFRM(11) */
	TCC_PIN(TCC_PINCTRL_PIN_PD6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* DAI1(1) */
		TCC_FUNCTION(0x2, "gpsb11")),		/* SCLK(11) */
	TCC_PIN(TCC_PINCTRL_PIN_PD7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* DAO2(1) */
		TCC_FUNCTION(0x2, "gpsb11")),		/* SDI(11) */
	TCC_PIN(TCC_PINCTRL_PIN_PD8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* DAI2(1) */
		TCC_FUNCTION(0x2, "gpsb11")),		/* SDO(11) */
	TCC_PIN(TCC_PINCTRL_PIN_PD9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* DAO3(1) */
		TCC_FUNCTION(0x2, "gps6"),		/* SFRM(6) */
		TCC_FUNCTION(0x3, "ts2")),		/* TSD7(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* DAI3(1) */
		TCC_FUNCTION(0x2, "gps6"),		/* SCLK(6) */
		TCC_FUNCTION(0x3, "ts2")),		/* TSD6(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "spd1"),		/* SPD_TX(1) */
		TCC_FUNCTION(0x2, "gps6"),		/* SDI(6) */
		TCC_FUNCTION(0x3, "spd0")),		/* SPD_TX(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PD12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "spd1"),		/* SPD_RX(1) */
		TCC_FUNCTION(0x2, "gps6"),		/* SDO(6) */
		TCC_FUNCTION(0x3, "ts2")),		/* TSSYNC(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart4"),		/* UTXD(4) */
		TCC_FUNCTION(0x3, "ts2")),		/* TSD5(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart4"),		/* URXD(4) */
		TCC_FUNCTION(0x3, "ts2")),		/* TSD4(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart4"),		/* UCTS(4) */
		TCC_FUNCTION(0x2, "gpsb12"),		/* SFRM(12) */
		TCC_FUNCTION(0x3, "ts2")),		/* TSVALID(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD16,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart4"),		/* URTS(4) */
		TCC_FUNCTION(0x2, "gpsb12"),		/* SCLK(12) */
		TCC_FUNCTION(0x3, "ts2")),		/* TSCLK(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD17,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart5"),		/* UTXD(5) */
		TCC_FUNCTION(0x3, "ts2")),		/* TSD3(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD18,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart5"),		/* URXD(5) */
		TCC_FUNCTION(0x3, "ts2")),		/* TSD2(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD19,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart5"),		/* UCTS(5) */
		TCC_FUNCTION(0x2, "gpsb12"),		/* SDI(12) */
		TCC_FUNCTION(0x3, "ts2")),		/* TSD1(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD20,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart5"),		/* URTS(5) */
		TCC_FUNCTION(0x2, "gpsb12"),		/* SDO(12) */
		TCC_FUNCTION(0x3, "ts2")),		/* TSD0(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD21,
		TCC_FUNCTION(0x0, "gpio")),
	TCC_PIN(TCC_PINCTRL_PIN_PD22,
		TCC_FUNCTION(0x0, "gpio")),
	TCC_PIN(TCC_PINCTRL_PIN_PD23,
		TCC_FUNCTION(0x0, "gpio")),
	TCC_PIN(TCC_PINCTRL_PIN_PD24,
		TCC_FUNCTION(0x0, "gpio")),
	TCC_PIN(TCC_PINCTRL_PIN_PD25,
		TCC_FUNCTION(0x0, "gpio")),

	TCC_PIN(TCC_PINCTRL_PIN_PE0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart0")),		/* UTXD(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PE1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart0")),		/* URXD(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PE2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart0"),		/* UCTS(0) */
		TCC_FUNCTION(0x2, "gpsb5")),		/* SFRM(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PE3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart0"),		/* URTS(0) */
		TCC_FUNCTION(0x2, "gpsb5")),		/* SCLK(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PE4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart1")),		/* UTXD(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PE5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart1")),		/* URXD(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PE6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart1"),		/* UCTS(1) */
		TCC_FUNCTION(0x2, "gpsb5"),		/* SDI(5) */
		TCC_FUNCTION(0x3, "sd4"),		/* SD_CLK(4) */
		TCC_FUNCTION(0x4, "ms4")),		/* MS_CLK(4) */
	TCC_PIN(TCC_PINCTRL_PIN_PE7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart1"),		/* URTS(1) */
		TCC_FUNCTION(0x2, "gpsb5"),		/* SDO(5) */
		TCC_FUNCTION(0x3, "sd4"),		/* SD_CMD(4) */
		TCC_FUNCTION(0x4, "ms4")),		/* MS_BUS(4) */
	TCC_PIN(TCC_PINCTRL_PIN_PE8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart2"),		/* UTXD(2) */
		TCC_FUNCTION(0x2, "gpsb4"),		/* SFRM(4) */
		TCC_FUNCTION(0x3, "sd4"),		/* SD_D0(4) */
		TCC_FUNCTION(0x4, "ms4")),		/* MS_D0(4) */
	TCC_PIN(TCC_PINCTRL_PIN_PE9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart2"),		/* URXD(2) */
		TCC_FUNCTION(0x2, "gpsb4"),		/* SCLK(4) */
		TCC_FUNCTION(0x3, "sd4"),		/* SD_D1(4) */
		TCC_FUNCTION(0x4, "ms4")),		/* MS_D1(4) */
	TCC_PIN(TCC_PINCTRL_PIN_PE10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart2"),		/* UCTS(2) */
		TCC_FUNCTION(0x2, "gpsb4"),		/* SDI(4) */
		TCC_FUNCTION(0x3, "sd4"),		/* SD_D2(4) */
		TCC_FUNCTION(0x4, "ms4")),		/* MS_D2(4) */
	TCC_PIN(TCC_PINCTRL_PIN_PE11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart2"),		/* URTS(2) */
		TCC_FUNCTION(0x2, "gpsb4"),		/* SDO(4) */
		TCC_FUNCTION(0x3, "sd4"),		/* SD_D3(4) */
		TCC_FUNCTION(0x4, "ms4")),		/* MS_D3(4) */
	TCC_PIN(TCC_PINCTRL_PIN_PE12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CPD[0] */
		TCC_FUNCTION(0x2, "sd2"),		/* SD_D0(2) */
		TCC_FUNCTION(0x3, "ts1"),		/* TSD0(1) */
		TCC_FUNCTION(0x4, "ms2")),		/* MS_D0(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PE13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CPD[1] */
		TCC_FUNCTION(0x2, "sd2"),		/* SD_D1(2) */
		TCC_FUNCTION(0x3, "ts1"),		/* TSD1(1) */
		TCC_FUNCTION(0x4, "ms2")),		/* MS_D1(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PE14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CPD[2] */
		TCC_FUNCTION(0x2, "sd2"),		/* SD_D2(2) */
		TCC_FUNCTION(0x3, "ts1"),		/* TSD2(1) */
		TCC_FUNCTION(0x4, "ms2")),		/* MS_D2(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PE15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CPD[3] */
		TCC_FUNCTION(0x2, "sd2"),		/* SD_D3(2) */
		TCC_FUNCTION(0x3, "ts1"),		/* TSD3(1) */
		TCC_FUNCTION(0x4, "ms2")),		/* MS_D3(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PE16,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CPD[4] */
		TCC_FUNCTION(0x2, "sd2"),		/* SD_D4(2) */
		TCC_FUNCTION(0x3, "ts1"),		/* TSD4(1) */
		TCC_FUNCTION(0x4, "ms2")),		/* MS_D4(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PE17,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CPD[5] */
		TCC_FUNCTION(0x2, "sd2"),		/* SD_D5(2) */
		TCC_FUNCTION(0x3, "ts1"),		/* TSD5(1) */
		TCC_FUNCTION(0x4, "ms2")),		/* MS_D5(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PE18,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CPD[6] */
		TCC_FUNCTION(0x2, "sd2"),		/* SD_D6(2) */
		TCC_FUNCTION(0x3, "ts1"),		/* TSD6(1) */
		TCC_FUNCTION(0x4, "ms2")),		/* MS_D6(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PE19,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CPD[7] */
		TCC_FUNCTION(0x2, "sd2"),		/* SD_D7(2) */
		TCC_FUNCTION(0x3, "ts1"),		/* TSD7(1) */
		TCC_FUNCTION(0x4, "ms2")),		/* MS_D7(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PE20,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CCKI */
		TCC_FUNCTION(0x2, "sd2"),		/* SD_CLK(2) */
		TCC_FUNCTION(0x3, "ts1"),		/* TSCLK(1) */
		TCC_FUNCTION(0x4, "ms2")),		/* MS_CLK(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PE21,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CVS */
		TCC_FUNCTION(0x2, "sd2"),		/* SD_CMD(2) */
		TCC_FUNCTION(0x3, "ts1"),		/* TSSYNC(1) */
		TCC_FUNCTION(0x4, "ms2")),		/* MS_BUS(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PE22,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CHS */
		TCC_FUNCTION(0x3, "ts1")),		/* TSVALID(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PE23,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CCKO */
		TCC_FUNCTION(0x2, "cam")),		/* CFIELD XXX duplicate "cam" function */
	TCC_PIN(TCC_PINCTRL_PIN_PE24,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "adc")),		/* AIN[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PE25,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "adc")),		/* AIN[1] */
	TCC_PIN(TCC_PINCTRL_PIN_PE26,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "adc"),		/* AIN[2] */
		TCC_FUNCTION(0x2, "sd7"),		/* SD_CMD(7) */
		TCC_FUNCTION(0x3, "ms7")),		/* MS_BUS(7) */
	TCC_PIN(TCC_PINCTRL_PIN_PE27,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "adc"),		/* AIN[3] */
		TCC_FUNCTION(0x2, "sd7"),		/* SD_CLK(7) */
		TCC_FUNCTION(0x3, "ms7")),		/* MS_CLK(7) */
	TCC_PIN(TCC_PINCTRL_PIN_PE28,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "adc"),		/* TSC_YM */
		TCC_FUNCTION(0x2, "sd7"),		/* SD_D0(7) */
		TCC_FUNCTION(0x3, "ms7")),		/* MS_D0(7) */
	TCC_PIN(TCC_PINCTRL_PIN_PE29,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "adc"),		/* TSC_YP */
		TCC_FUNCTION(0x2, "sd7"),		/* SD_D1(7) */
		TCC_FUNCTION(0x3, "ms7")),		/* MS_D1(7) */
	TCC_PIN(TCC_PINCTRL_PIN_PE30,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "adc"),		/* TSC_XM */
		TCC_FUNCTION(0x2, "sd7"),		/* SD_D2(7) */
		TCC_FUNCTION(0x3, "ms7")),		/* MS_D2(7) */
	TCC_PIN(TCC_PINCTRL_PIN_PE31,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "adc"),		/* TSC_XP */
		TCC_FUNCTION(0x2, "sd7"),		/* SD_D3(7) */
		TCC_FUNCTION(0x3, "ms7")),		/* MS_D3(7) */

	TCC_PIN(TCC_PINCTRL_PIN_PF0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[0] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D0(3) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD0 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSD0(0) */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D0(3) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[3] */
	TCC_PIN(TCC_PINCTRL_PIN_PF1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[1] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D1(3) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD1 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSD1(0) */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D1(3) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[4] */
	TCC_PIN(TCC_PINCTRL_PIN_PF2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[2] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D2(3) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD2 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSD2(0) */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D2(3) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[5] */
	TCC_PIN(TCC_PINCTRL_PIN_PF3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[3] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D3(3) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD3 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSD3(0) */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D3(3) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[6] */
	TCC_PIN(TCC_PINCTRL_PIN_PF4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[4] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D4(3) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD4 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSD4(0) */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D4(3) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[7] */
	TCC_PIN(TCC_PINCTRL_PIN_PF5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[5] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D5(3) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD5 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSD5(0) */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D5(3) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[8] */
	TCC_PIN(TCC_PINCTRL_PIN_PF6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[6] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D6(3) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD6 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSD6(0) */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D6(3) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[9] */
	TCC_PIN(TCC_PINCTRL_PIN_PF7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[7] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D7(3) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD7 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSD7(0) */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D7(3) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[10] */
	TCC_PIN(TCC_PINCTRL_PIN_PF8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[8] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_CMD(3) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD8 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSVALID(0) */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_BUS(3) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[11] */
	TCC_PIN(TCC_PINCTRL_PIN_PF9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[9] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_CLK(3) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD9 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSCLK(0) */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_CLK(3) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[12] */
	TCC_PIN(TCC_PINCTRL_PIN_PF10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[10] */
		TCC_FUNCTION(0x2, "gpsb7"),		/* SDO(7) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD10 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSSYNC(0) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[13] */
	TCC_PIN(TCC_PINCTRL_PIN_PF11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[11] */
		TCC_FUNCTION(0x2, "gpsb7"),		/* SDI(7) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD11 */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[14] */
	TCC_PIN(TCC_PINCTRL_PIN_PF12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[12] */
		TCC_FUNCTION(0x2, "gpsb7"),		/* SCLK(7) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD12 */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[15] */
	TCC_PIN(TCC_PINCTRL_PIN_PF13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[13] */
		TCC_FUNCTION(0x2, "gpsb7"),		/* SFRM(7) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD13 */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[16] */
	TCC_PIN(TCC_PINCTRL_PIN_PF14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[14] */
		TCC_FUNCTION(0x2, "gpsb8"),		/* SDO(8) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD14 */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[17] */
	TCC_PIN(TCC_PINCTRL_PIN_PF15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[15] */
		TCC_FUNCTION(0x2, "gpsb8"),		/* SDI(8) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXD15 */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[18] */
	TCC_PIN(TCC_PINCTRL_PIN_PF16,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[16] */
		TCC_FUNCTION(0x2, "gpsb8"),		/* SCLK(8) */
		TCC_FUNCTION(0x3, "udma")),		/* HDDXA0 */
	TCC_PIN(TCC_PINCTRL_PIN_PF17,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[17] */
		TCC_FUNCTION(0x2, "gpsb8"),		/* SFRM(8) */
		TCC_FUNCTION(0x3, "udma")),		/* HDDXA1 */
	TCC_PIN(TCC_PINCTRL_PIN_PF18,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPRDN */
		TCC_FUNCTION(0x2, "sd1"),		/* SD_D3(1) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDXA2 */
		TCC_FUNCTION(0x4, "ms1")),		/* MS_D3(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PF19,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPWRN */
		TCC_FUNCTION(0x2, "sd1"),		/* SD_D2(1) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDCSN1 */
		TCC_FUNCTION(0x4, "ms1")),		/* MS_D2(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PF20,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPCSN0 */
		TCC_FUNCTION(0x2, "sd1"),		/* SD_D1(1) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDRDY */
		TCC_FUNCTION(0x4, "ms1")),		/* MS_D1(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PF21,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPCSN1 */
		TCC_FUNCTION(0x2, "sd1"),		/* SD_D0(1) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDCSN0 */
		TCC_FUNCTION(0x4, "ms1")),		/* MS_D0(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PF22,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXA */
		TCC_FUNCTION(0x2, "sd1"),		/* SD_CMD(1) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDAK */
		TCC_FUNCTION(0x4, "ms1")),		/* MS_BUS(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PF23,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPINT0 */
		TCC_FUNCTION(0x2, "sd1"),		/* SD_CLK(1) */
		TCC_FUNCTION(0x3, "udma"),		/* HDDRQ */
		TCC_FUNCTION(0x4, "ms1")),		/* MS_CLK(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PF24,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPINT1 */
		TCC_FUNCTION(0x2, "gps9"),		/* SDO(9) */
		TCC_FUNCTION(0x3, "udma")),		/* HDDIOW */
	TCC_PIN(TCC_PINCTRL_PIN_PF25,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "gps9"),		/* SDI(9) */
		TCC_FUNCTION(0x3, "udma")),		/* HDDIOR */
	TCC_PIN(TCC_PINCTRL_PIN_PF26,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd1"),		/* EXTLVS1(1) */
		TCC_FUNCTION(0x2, "gps9"),		/* SCLK(9) */
		TCC_FUNCTION(0x3, "can")),		/* CAN_RX */
	TCC_PIN(TCC_PINCTRL_PIN_PF27,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* EXTLVS0(1) */
		TCC_FUNCTION(0x2, "gps9"),		/* SFRM(9) */
		TCC_FUNCTION(0x3, "can")),		/* CAN_TX */
};

static const int tcc92xx_eints[] = {
	3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
};

static const int tcc92xx_bank_offsets[] = {
	0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140,
};

static const struct tcc_pinctrl_desc tcc92xx_pinctrl_data = {
	.pins = tcc92xx_pins,
	.npins = ARRAY_SIZE(tcc92xx_pins),

	.bank_offsets = tcc92xx_bank_offsets,
	.nbanks = ARRAY_SIZE(tcc92xx_bank_offsets),

	.neints = ARRAY_SIZE(tcc92xx_eints),
	.eint_irqs = tcc92xx_eints,
	.eint_sources = tcc92xx_eint_sources,
	.eintsel_offset = 0x184,
};

static int tcc88xx_eint_sources[] = {
	GPA(0), GPA(1), GPA(2), GPA(3), GPA(4), GPA(5), GPA(6),
	GPA(7), GPA(8), GPA(9), GPA(10), GPA(11), GPA(12),
	GPA(13), GPA(14), GPA(15), GPD(5), GPD(6), GPD(7),
	GPD(8), GPD(9), GPD(10), GPD(13), GPD(14), GPD(15),
	GPD(16), GPD(17), GPD(18), GPD(19), GPD(20),
	GPF(23), GPF(24), GPF(25), GPF(26), GPF(27),
	GPF(20), GPF(17), GPF(13), GPF(10), GPC(28),
	GPC(29), GPC(30), GPC(31), GPC(9), GPC(13),
	GPB(28), GPB(29), GPB(30), GPB(31), GPB(8),
	GPB(12), GPE(4), GPE(5), GPE(24), GPE(25),
	/* ADC_TS_WKU, ADC_STOP, ADC_UPDOWN, -, -,
		RTC_WAKEUP, USB0_VBON, USB0_VBOFF */
	-1
};

static const struct tcc_desc_pin tcc88xx_pins[] = {
	TCC_PIN(TCC_PINCTRL_PIN_PA0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2c0")),		/* SCL(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PA1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2c0")),		/* SDA(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PA2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "clkout")),		/* CLKOUT[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PA3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "clkout")),		/* CLKOUT[1] */
	TCC_PIN(TCC_PINCTRL_PIN_PA4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "usb"),		/* UTM1_VBUS */
		TCC_FUNCTION(0x2, "tco"),		/* TCO[0] */
		TCC_FUNCTION(0x3, "pdm"),		/* PDM[0] */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[18] */
	TCC_PIN(TCC_PINCTRL_PIN_PA5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "remocon"),		/* REM_IRDI */
		TCC_FUNCTION(0x2, "tco"),		/* TCO[1] */
		TCC_FUNCTION(0x3, "pdm")),		/* PDM[1] */
	TCC_PIN(TCC_PINCTRL_PIN_PA6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "tco"),		/* TCO[2] */
		TCC_FUNCTION(0x3, "pdm"),		/* PDM[2] */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[19] */
	TCC_PIN(TCC_PINCTRL_PIN_PA7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2c1"),		/* SCL(1) */
		TCC_FUNCTION(0x2, "tco"),		/* TCO[3] */
		TCC_FUNCTION(0x3, "pdm")),		/* PDM[3] */
	TCC_PIN(TCC_PINCTRL_PIN_PA8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2c1")),		/* SCA(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PA9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart1"),		/* UT_TXD(1) */
		TCC_FUNCTION(0x3, "sdpow")),		/* SD_POW[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PA10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart1"),		/* UT_RXD(1) */
		TCC_FUNCTION(0x2, "cd"),		/* CD_BCLKI */
		TCC_FUNCTION(0x3, "sdpow")),		/* SD_POW[1] */
	TCC_PIN(TCC_PINCTRL_PIN_PA11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart1"),		/* UT_CTS(1) */
		TCC_FUNCTION(0x2, "cd"),		/* CD_LRLKI */
		TCC_FUNCTION(0x3, "sdpow")),		/* SD_POW[2] */
	TCC_PIN(TCC_PINCTRL_PIN_PA12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart1"),		/* UT_RTX(1) */
		TCC_FUNCTION(0x2, "cd"),		/* CD_DI */
		TCC_FUNCTION(0x3, "sdpow")),		/* SD_POW[3] */
	TCC_PIN(TCC_PINCTRL_PIN_PA13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "extclki")),		/* EXTCLKI */
	TCC_PIN(TCC_PINCTRL_PIN_PA14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "tco"),		/* TCO[4] */
		TCC_FUNCTION(0x3, "pcie")),		/* PCIE_WAKE */
	TCC_PIN(TCC_PINCTRL_PIN_PA15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "usb"),		/* UTM_VBUS */
		TCC_FUNCTION(0x2, "tco")),		/* TCO[5] */

	TCC_PIN(TCC_PINCTRL_PIN_PB0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[0] */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D(5)[0] */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D(5)[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PB1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[1] */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D(5)[1] */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D(5)[1] */
	TCC_PIN(TCC_PINCTRL_PIN_PB2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[2] */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D(5)[2] */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D(5)[2] */
	TCC_PIN(TCC_PINCTRL_PIN_PB3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[3] */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D(5)[3] */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D(5)[3] */
	TCC_PIN(TCC_PINCTRL_PIN_PB4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[4] */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D(5)[4] */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D(5)[4] */
	TCC_PIN(TCC_PINCTRL_PIN_PB5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[5] */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D(5)[5] */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D(5)[5] */
	TCC_PIN(TCC_PINCTRL_PIN_PB6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[6] */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D(5)[6] */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D(5)[6] */
	TCC_PIN(TCC_PINCTRL_PIN_PB7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[7] */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_D(5)[7] */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_D(5)[7] */
	TCC_PIN(TCC_PINCTRL_PIN_PB8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[8] */
		TCC_FUNCTION(0x4, "gpsb1")),		/* SFRM(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PB9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[9] */
		TCC_FUNCTION(0x4, "gpsb1")),		/* SCLK(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PB10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[10] */
		TCC_FUNCTION(0x4, "gpsb1")),		/* SDI(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PB11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[11] */
		TCC_FUNCTION(0x4, "gpsb1")),		/* SDO(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PB12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[12] */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_CMD(5) */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_BUS(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PB13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XD[13] */
		TCC_FUNCTION(0x2, "sd5"),		/* SD_CLK(5) */
		TCC_FUNCTION(0x3, "ms5")),		/* MS_CLK(5) */
	TCC_PIN(TCC_PINCTRL_PIN_PB14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi")),		/* EDI_XD[14] */
	TCC_PIN(TCC_PINCTRL_PIN_PB15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi")),		/* EDI_XD[15] */
	TCC_PIN(TCC_PINCTRL_PIN_PB16,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_WEN[0] */
		TCC_FUNCTION(0x6, "edi")),		/* EDI_RDY[2] */
	TCC_PIN(TCC_PINCTRL_PIN_PB17,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_WEN[1] */
		TCC_FUNCTION(0x4, "gpsb0")),		/* SFRM(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PB18,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_OEN[0] */
		TCC_FUNCTION(0x6, "edi")),		/* EDI_RDY[3] */
	TCC_PIN(TCC_PINCTRL_PIN_PB19,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_OEN[1] */
		TCC_FUNCTION(0x4, "gpsb0")),		/* SCLK(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PB20,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi")),		/* EDI_XA[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PB21,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XA[1] */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D(6)[4] */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D(6)[4] */
	TCC_PIN(TCC_PINCTRL_PIN_PB22,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_XA[2] */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D(6)[5] */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D(6)[5] */
	TCC_PIN(TCC_PINCTRL_PIN_PB23,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_CSN[0] */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D(6)[6] */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D(6)[6] */
	TCC_PIN(TCC_PINCTRL_PIN_PB24,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_CSN[1] */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D(6)[7] */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D(6)[7] */
	TCC_PIN(TCC_PINCTRL_PIN_PB25,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_CSN[2] */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D(6)[0] */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D(6)[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PB26,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_CSN[3] */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D(6)[1] */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D(6)[1] */
	TCC_PIN(TCC_PINCTRL_PIN_PB27,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_CSN[4] */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D(6)[2] */
		TCC_FUNCTION(0x3, "ms6"),		/* MS_D(6)[2] */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[17] */
	TCC_PIN(TCC_PINCTRL_PIN_PB28,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_RDY[0] */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_D(6)[3] */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_D(6)[3] */
	TCC_PIN(TCC_PINCTRL_PIN_PB29,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_RDY[1] */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_CMD(6) */
		TCC_FUNCTION(0x3, "ms6")),		/* MS_BUS(6) */
	TCC_PIN(TCC_PINCTRL_PIN_PB30,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_CSN[5] */
		TCC_FUNCTION(0x2, "sd6"),		/* SD_CLK(6) */
		TCC_FUNCTION(0x3, "ms6"),		/* MS_CLK(6) */
		TCC_FUNCTION(0x4, "gpsb0"),		/* SDI(0) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[16] */
	TCC_PIN(TCC_PINCTRL_PIN_PB31,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "edi"),		/* EDI_CSN[6] */
		TCC_FUNCTION(0x4, "gpsb0"),		/* SDO(0) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[15] */

	TCC_PIN(TCC_PINCTRL_PIN_PC0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[0] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[0] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[0] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PC1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[1] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[1] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[1] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[1] */
	TCC_PIN(TCC_PINCTRL_PIN_PC2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[2] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[2] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[2] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[2] */
	TCC_PIN(TCC_PINCTRL_PIN_PC3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[3] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[3] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[3] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[3] */
	TCC_PIN(TCC_PINCTRL_PIN_PC4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[4] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[4] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[4] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[4] */
	TCC_PIN(TCC_PINCTRL_PIN_PC5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[5] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[5] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[5] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[5] */
	TCC_PIN(TCC_PINCTRL_PIN_PC6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[6] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[6] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[6] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[6] */
	TCC_PIN(TCC_PINCTRL_PIN_PC7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[7] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[7] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[7] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[7] */
	TCC_PIN(TCC_PINCTRL_PIN_PC8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[8] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[8] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[8] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[8] */
	TCC_PIN(TCC_PINCTRL_PIN_PC9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[9] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[9] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[9] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[9] */
	TCC_PIN(TCC_PINCTRL_PIN_PC10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[10] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[10] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[10] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[10] */
	TCC_PIN(TCC_PINCTRL_PIN_PC11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[11] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[11] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[11] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[11] */
	TCC_PIN(TCC_PINCTRL_PIN_PC12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[12] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[12] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[12] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[12] */
	TCC_PIN(TCC_PINCTRL_PIN_PC13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[13] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[13] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[13] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[13] */
	TCC_PIN(TCC_PINCTRL_PIN_PC14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[14] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[14] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[14] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[14] */
	TCC_PIN(TCC_PINCTRL_PIN_PC15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[15] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[15] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[15] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[15] */
	TCC_PIN(TCC_PINCTRL_PIN_PC16,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[16] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[16] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[16] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[16] */
	TCC_PIN(TCC_PINCTRL_PIN_PC17,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[17] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[17] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[17] */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXD[17] */
	TCC_PIN(TCC_PINCTRL_PIN_PC18,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[18] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[18] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[18] */
	TCC_PIN(TCC_PINCTRL_PIN_PC19,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[19] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[19] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[19] */
	TCC_PIN(TCC_PINCTRL_PIN_PC20,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[20] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[20] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[20] */
	TCC_PIN(TCC_PINCTRL_PIN_PC21,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[21] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[21] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[21] */
	TCC_PIN(TCC_PINCTRL_PIN_PC22,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[22] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[22] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[22] */
	TCC_PIN(TCC_PINCTRL_PIN_PC23,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XD[23] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXD[23] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[23] */
	TCC_PIN(TCC_PINCTRL_PIN_PC24,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_WEN */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LACBIAS */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LACBIAS */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPWRN */
	TCC_PIN(TCC_PINCTRL_PIN_PC25,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_OEN */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LPXCLK */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXCLK */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPRDN */
	TCC_PIN(TCC_PINCTRL_PIN_PC26,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_XA */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LHSYNC */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LHSYNC */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPXA */
	TCC_PIN(TCC_PINCTRL_PIN_PC27,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_CSN[0] */
		TCC_FUNCTION(0x2, "lcd0rgb"),		/* L0_LVSYNC */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LVSYNC */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPCSN0 */
	TCC_PIN(TCC_PINCTRL_PIN_PC28,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* LCD0_CSN[1] */
		TCC_FUNCTION(0x2, "gpsb10"),		/* SDO(10) */
		TCC_FUNCTION(0x6, "lcd_bypass")),	/* HPCSN1 */
	TCC_PIN(TCC_PINCTRL_PIN_PC29,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "gpsb10")),		/* SDI(10) */
	TCC_PIN(TCC_PINCTRL_PIN_PC30,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd1"),		/* EXT_LVS1 */
		TCC_FUNCTION(0x2, "gpsb10")),		/* SCLK(10) */
	TCC_PIN(TCC_PINCTRL_PIN_PC31,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "lcd0"),		/* EXT_LVS0 */
		TCC_FUNCTION(0x2, "gpsb10")),		/* SFRM(10) */

	TCC_PIN(TCC_PINCTRL_PIN_PD0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1")),		/* I2S_BCLK(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PD1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1")),		/* I2S_LRCK(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PD2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1")),		/* I2S_MCLK(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PD3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1")),		/* I2S_DAO0(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PD4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1")),		/* I2S_DAI0(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PD5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* I2S_DAO1(1) */
		TCC_FUNCTION(0x2, "gpsb11")),		/* SFRM(11) */
	TCC_PIN(TCC_PINCTRL_PIN_PD6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* I2S_DAI1(1) */
		TCC_FUNCTION(0x2, "gpsb11")),		/* SCLK(11) */
	TCC_PIN(TCC_PINCTRL_PIN_PD7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* I2S_DAO2(1) */
		TCC_FUNCTION(0x2, "gpsb11")),		/* SDI(11) */
	TCC_PIN(TCC_PINCTRL_PIN_PD8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* I2S_DAI2(1) */
		TCC_FUNCTION(0x2, "gpsb11")),		/* SDO(11) */
	TCC_PIN(TCC_PINCTRL_PIN_PD9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* I2S_DAO1(1) */
		TCC_FUNCTION(0x2, "gpsb6")),		/* SFRM(6) */
	TCC_PIN(TCC_PINCTRL_PIN_PD10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2s1"),		/* I2S_DAI3(1) */
		TCC_FUNCTION(0x2, "gpsb6"),		/* SCLK(6) */
		TCC_FUNCTION(0x3, "ts2")),		/* TS_DATA(2)[7] */
	TCC_PIN(TCC_PINCTRL_PIN_PD11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "spdif1"),		/* SPDIF_TX(1) */
		TCC_FUNCTION(0x2, "gpsb6"),		/* SDI(6) */
		TCC_FUNCTION(0x3, "ts2")),		/* TS_DATA(2)[6] */
	TCC_PIN(TCC_PINCTRL_PIN_PD12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "spdif1"),		/* SPDIF_RX(1) */
		TCC_FUNCTION(0x2, "gpsb6"),		/* SDO(6) */
		TCC_FUNCTION(0x3, "ts2")),		/* TS_SYNC(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart4"),		/* UT_TXD(4) */
		TCC_FUNCTION(0x3, "ts2"),		/* TS_DATA(2)[5] */
		TCC_FUNCTION(0x4, "i2s0")),		/* BCLK(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PD14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart4"),		/* UT_RXD(4) */
		TCC_FUNCTION(0x3, "ts2"),		/* TS_DATA(2)[4] */
		TCC_FUNCTION(0x4, "i2s0")),		/* LRCK(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PD15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart4"),		/* UT_CTS(4) */
		TCC_FUNCTION(0x2, "gpsb12"),		/* SFRM(12) */
		TCC_FUNCTION(0x3, "ts2"),		/* TS_VALID(2) */
		TCC_FUNCTION(0x4, "i2s0")),		/* MCLK(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PD16,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart4"),		/* UT_RTS(4) */
		TCC_FUNCTION(0x2, "gpsb12"),		/* SCLK(12) */
		TCC_FUNCTION(0x3, "ts2"),		/* TS_CLK(2) */
		TCC_FUNCTION(0x4, "i2s0")),		/* DAO0(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PD17,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart5"),		/* UT_TXD(5) */
		TCC_FUNCTION(0x3, "ts2"),		/* TS_DATA(2)[3] */
		TCC_FUNCTION(0x4, "i2s0")),		/* DAI0(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PD18,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart5"),		/* UT_RXD(5) */
		TCC_FUNCTION(0x3, "ts2"),		/* TS_DATA(2)[2] */
		TCC_FUNCTION(0x4, "i2s2")),		/* BCLK(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD19,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart5"),		/* UT_CTS(5) */
		TCC_FUNCTION(0x2, "gpsb12"),		/* SDI(12) */
		TCC_FUNCTION(0x3, "ts2"),		/* TS_DATA(2)[1] */
		TCC_FUNCTION(0x4, "i2s2")),		/* LRCK(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD20,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart5"),		/* UT_RTS(5) */
		TCC_FUNCTION(0x2, "gpsb12"),		/* SDO(12) */
		TCC_FUNCTION(0x3, "ts2"),		/* TS_DATA(2)[0] */
		TCC_FUNCTION(0x4, "i2s2")),		/* MCLK(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD21,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x3, "spdif0"),		/* SPDIF_TX(0) */
		TCC_FUNCTION(0x4, "i2s2")),		/* DAO0(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PD22,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "gpsb3"),		/* SDO(3) */
		TCC_FUNCTION(0x3, "spdif0"),		/* SPDIF_RX(0) */
		TCC_FUNCTION(0x4, "i2s2"),		/* DAI0(2) */
		TCC_FUNCTION(0x5, "i2c3")),		/* SCL(3) */
	TCC_PIN(TCC_PINCTRL_PIN_PD23,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cd"),		/* CD_BCLKI */
		TCC_FUNCTION(0x2, "gpsb3"),		/* SDI(3) */
		TCC_FUNCTION(0x5, "i2c3")),		/* SDA(3) */
	TCC_PIN(TCC_PINCTRL_PIN_PD24,
		TCC_FUNCTION(0x0, "gpio"),		/* XXX datasheet does not mention this function! */
		TCC_FUNCTION(0x1, "cd"),		/* CD_LRCKI */
		TCC_FUNCTION(0x2, "gpsb3"),		/* SCLK(3) */
		TCC_FUNCTION(0x5, "hdmi")),		/* HDMI_CEC */
	TCC_PIN(TCC_PINCTRL_PIN_PD25,
		TCC_FUNCTION(0x0, "gpio"),		/* XXX datasheet does not mention this function! */
		TCC_FUNCTION(0x1, "cd"),		/* CD_DI */
		TCC_FUNCTION(0x2, "gpsb3"),		/* SFRM(3) */
		TCC_FUNCTION(0x5, "hdmi")),		/* HDMI_HPD */

	TCC_PIN(TCC_PINCTRL_PIN_PE0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart0")),		/* UT_TXD(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PE1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart0")),		/* UT_RXD(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PE2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart0"),		/* UT_CTX(0) */
		TCC_FUNCTION(0x2, "gpsb5"),		/* SFRM(5) */
		TCC_FUNCTION(0x5, "cam")),		/* SENSOR_PWDN */
	TCC_PIN(TCC_PINCTRL_PIN_PE3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart0"),		/* UT_RTX(0) */
		TCC_FUNCTION(0x2, "gpsb5"),		/* SCLK(5) */
		TCC_FUNCTION(0x5, "cam")),		/* FL_TRIG */
	TCC_PIN(TCC_PINCTRL_PIN_PE4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "camin"),		/* CFIELD */
		TCC_FUNCTION(0x5, "cam")),		/* FLASH_TRIG */
	TCC_PIN(TCC_PINCTRL_PIN_PE5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x5, "cam")),		/* PREFLIGHT_TRIG */
	TCC_PIN(TCC_PINCTRL_PIN_PE6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "gpsb5"),		/* SDI(5) */
		TCC_FUNCTION(0x3, "sd4"),		/* SD_CLK(4) */
		TCC_FUNCTION(0x4, "ms4"),		/* MS_CLK(4) */
		TCC_FUNCTION(0x5, "cam")),		/* SHUTTER_TRIG */
	TCC_PIN(TCC_PINCTRL_PIN_PE7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "gpsb5"),		/* SDO(5) */
		TCC_FUNCTION(0x3, "sd4"),		/* SD_CMD(4) */
		TCC_FUNCTION(0x4, "ms4"),		/* MS_BUS(4) */
		TCC_FUNCTION(0x5, "cam")),		/* SHUTTER_OPEN */
	TCC_PIN(TCC_PINCTRL_PIN_PE8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "gpsb4"),		/* SFRM(4) */
		TCC_FUNCTION(0x3, "sd4"),		/* SD_D(4)[0] */
		TCC_FUNCTION(0x4, "ms4"),		/* MS_D(4)[0] */
		TCC_FUNCTION(0x5, "cam")),		/* CPD[8] */
	TCC_PIN(TCC_PINCTRL_PIN_PE9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "gpsb4"),		/* SCLK(4) */
		TCC_FUNCTION(0x3, "sd4"),		/* SD_D(4)[1] */
		TCC_FUNCTION(0x4, "ms4"),		/* MS_D(4)[1] */
		TCC_FUNCTION(0x5, "cam")),		/* CPD[9] */
	TCC_PIN(TCC_PINCTRL_PIN_PE10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart3"),		/* UT_TXD(3) */
		TCC_FUNCTION(0x2, "gpsb4"),		/* SDI(4) */
		TCC_FUNCTION(0x3, "sd4"),		/* SD_D(4)[2] */
		TCC_FUNCTION(0x4, "ms4"),		/* MS_D(4)[2] */
		TCC_FUNCTION(0x5, "cam")),		/* CPD[10] */
	TCC_PIN(TCC_PINCTRL_PIN_PE11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "uart3"),		/* UT_RXD(3) */
		TCC_FUNCTION(0x2, "gpsb4"),		/* SDO(4) */
		TCC_FUNCTION(0x3, "sd4"),		/* SD_D(4)[3] */
		TCC_FUNCTION(0x4, "ms4"),		/* MS_D(4)[3] */
		TCC_FUNCTION(0x5, "cam")),		/* CPD[11] */
	TCC_PIN(TCC_PINCTRL_PIN_PE12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CAM_D[0] */
		TCC_FUNCTION(0x3, "ts1")),		/* TS_D(1)[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PE13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CAM_D[1] */
		TCC_FUNCTION(0x3, "ts1")),		/* TS_D(1)[1] */
	TCC_PIN(TCC_PINCTRL_PIN_PE14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CAM_D[2] */
		TCC_FUNCTION(0x3, "ts1")),		/* TS_D(1)[2] */
	TCC_PIN(TCC_PINCTRL_PIN_PE15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CAM_D[3] */
		TCC_FUNCTION(0x3, "ts1")),		/* TS_D(1)[3] */
	TCC_PIN(TCC_PINCTRL_PIN_PE16,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CAM_D[4] */
		TCC_FUNCTION(0x3, "ts1")),		/* TS_D(1)[4] */
	TCC_PIN(TCC_PINCTRL_PIN_PE17,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CAM_D[5] */
		TCC_FUNCTION(0x3, "ts1")),		/* TS_D(1)[5] */
	TCC_PIN(TCC_PINCTRL_PIN_PE18,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CAM_D[6] */
		TCC_FUNCTION(0x3, "ts1")),		/* TS_D(1)[6] */
	TCC_PIN(TCC_PINCTRL_PIN_PE19,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CAM_D[7] */
		TCC_FUNCTION(0x3, "ts1")),		/* TS_D(1)[7] */
	TCC_PIN(TCC_PINCTRL_PIN_PE20,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CAM_ICLKI */
		TCC_FUNCTION(0x3, "ts1")),		/* TSCLK(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PE21,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CAM_VS */
		TCC_FUNCTION(0x3, "ts1")),		/* TSSYNC(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PE22,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam"),		/* CAM_HS */
		TCC_FUNCTION(0x3, "ts1")),		/* TSVALID(1) */
	TCC_PIN(TCC_PINCTRL_PIN_PE23,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "cam")),		/* CAM_ICLKO */
	TCC_PIN(TCC_PINCTRL_PIN_PE24,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2c2")),		/* SCL(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PE25,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "i2c2")),		/* SDA(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PE26,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "sd7"),		/* SD_CMD(7) */
		TCC_FUNCTION(0x3, "ms7")),		/* MS_BUS(7) */
	TCC_PIN(TCC_PINCTRL_PIN_PE27,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "sd7"),		/* SD_CLK(7) */
		TCC_FUNCTION(0x3, "ms7")),		/* MS_CLK(7) */
	TCC_PIN(TCC_PINCTRL_PIN_PE28,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "sd7"),		/* SD_D(7)[0] */
		TCC_FUNCTION(0x3, "ms7")),		/* MS_D(7)[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PE29,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "sd7"),		/* SD_D(7)[1] */
		TCC_FUNCTION(0x3, "ms7")),		/* MS_D(7)[1] */
	TCC_PIN(TCC_PINCTRL_PIN_PE30,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "sd7"),		/* SD_D(7)[2] */
		TCC_FUNCTION(0x3, "ms7")),		/* MS_D(7)[2] */
	TCC_PIN(TCC_PINCTRL_PIN_PE31,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "sd7"),		/* SD_D(7)[3] */
		TCC_FUNCTION(0x3, "ms7")),		/* MS_D(7)[3] */

	TCC_PIN(TCC_PINCTRL_PIN_PF0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[0] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D(3)[0] */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD0 */
		TCC_FUNCTION(0x4, "ts0"),		/* TS_D(0)[0] */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D(3)[0] */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[3] */
	TCC_PIN(TCC_PINCTRL_PIN_PF1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[1] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D(3)[1] */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD1 */
		TCC_FUNCTION(0x4, "ts0"),		/* TS_D(0)[1] */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D(3)[1] */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[4] */
	TCC_PIN(TCC_PINCTRL_PIN_PF2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[2] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D(3)[2] */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD2 */
		TCC_FUNCTION(0x4, "ts0"),		/* TS_D(0)[2] */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D(3)[2] */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[5] */
	TCC_PIN(TCC_PINCTRL_PIN_PF3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[3] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D(3)[3] */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD3 */
		TCC_FUNCTION(0x4, "ts0"),		/* TS_D(0)[3] */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D(3)[3] */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[6] */
	TCC_PIN(TCC_PINCTRL_PIN_PF4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[4] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D(3)[4] */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD4 */
		TCC_FUNCTION(0x4, "ts0"),		/* TS_D(0)[4] */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D(3)[4] */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[7] */
	TCC_PIN(TCC_PINCTRL_PIN_PF5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[5] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D(3)[5] */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD5 */
		TCC_FUNCTION(0x4, "ts0"),		/* TS_D(0)[5] */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D(3)[5] */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[8] */
	TCC_PIN(TCC_PINCTRL_PIN_PF6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[6] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D(3)[6] */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD6 */
		TCC_FUNCTION(0x4, "ts0"),		/* TS_D(0)[6] */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D(3)[6] */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[9] */
	TCC_PIN(TCC_PINCTRL_PIN_PF7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[7] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_D(3)[7] */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD7 */
		TCC_FUNCTION(0x4, "ts0"),		/* TS_D(0)[7] */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_D(3)[7] */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[10] */
	TCC_PIN(TCC_PINCTRL_PIN_PF8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[8] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_CMD(3) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD8 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSVALID(0) */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_BUS(3) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[11] */
	TCC_PIN(TCC_PINCTRL_PIN_PF9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[9] */
		TCC_FUNCTION(0x2, "sd3"),		/* SD_CLK(3) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD9 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSCLK(0) */
		TCC_FUNCTION(0x5, "ms3"),		/* MS_CLK(3) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[12] */
	TCC_PIN(TCC_PINCTRL_PIN_PF10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[10] */
		TCC_FUNCTION(0x2, "gpsb7"),		/* SDO(7) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD10 */
		TCC_FUNCTION(0x4, "ts0"),		/* TSSYNC(0) */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[13] */
	TCC_PIN(TCC_PINCTRL_PIN_PF11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[11] */
		TCC_FUNCTION(0x2, "gpsb7"),		/* SDI(7) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD11 */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[14] */
	TCC_PIN(TCC_PINCTRL_PIN_PF12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[12] */
		TCC_FUNCTION(0x2, "gpsb7"),		/* SCLK(7) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD12 */
		TCC_FUNCTION(0x5, "edi"),		/* EDIXD16 */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[20] */
	TCC_PIN(TCC_PINCTRL_PIN_PF13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[13] */
		TCC_FUNCTION(0x2, "gpsb7"),		/* SFRM(7) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD13 */
		TCC_FUNCTION(0x5, "edi"),		/* EDIXD17 */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[21] */
	TCC_PIN(TCC_PINCTRL_PIN_PF14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[14] */
		TCC_FUNCTION(0x2, "gpsb8"),		/* SDO(8) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD14 */
		TCC_FUNCTION(0x5, "edi"),		/* EDIXD18 */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[22] */
	TCC_PIN(TCC_PINCTRL_PIN_PF15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[15] */
		TCC_FUNCTION(0x2, "gpsb8"),		/* SDI(8) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXD15 */
		TCC_FUNCTION(0x5, "edi"),		/* EDIXD19 */
		TCC_FUNCTION(0x6, "edi")),		/* EDIXA[23] */
	TCC_PIN(TCC_PINCTRL_PIN_PF16,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[16] */
		TCC_FUNCTION(0x2, "gpsb8"),		/* SCLK(8) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXA0 */
		TCC_FUNCTION(0x5, "edi")),		/* EDIXD20 */
	TCC_PIN(TCC_PINCTRL_PIN_PF17,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXD[17] */
		TCC_FUNCTION(0x2, "gpsb8"),		/* SFRM(8) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXA1 */
		TCC_FUNCTION(0x5, "edi")),		/* EDIXD21 */
	TCC_PIN(TCC_PINCTRL_PIN_PF18,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPRDN */
		TCC_FUNCTION(0x2, "sd1"),		/* SD_D(1)[3] */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDXA2 */
		TCC_FUNCTION(0x4, "ms1"),		/* MS_D(1)[3] */
		TCC_FUNCTION(0x5, "edi")),		/* EDIXD22 */
	TCC_PIN(TCC_PINCTRL_PIN_PF19,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPWRN */
		TCC_FUNCTION(0x2, "sd1"),		/* SD_D(1)[2] */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDCSN1 */
		TCC_FUNCTION(0x4, "ms1"),		/* MS_D(1)[2] */
		TCC_FUNCTION(0x5, "edi")),		/* EDIXD23 */
	TCC_PIN(TCC_PINCTRL_PIN_PF20,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPCSN0 */
		TCC_FUNCTION(0x2, "sd1"),		/* SD_D(1)[1] */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDRDY */
		TCC_FUNCTION(0x4, "ms1"),		/* MS_D(1)[1] */
		TCC_FUNCTION(0x5, "edi")),		/* EDIXD24 */
	TCC_PIN(TCC_PINCTRL_PIN_PF21,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPCSN1 */
		TCC_FUNCTION(0x2, "sd1"),		/* SD_D(1)[0] */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDCSN0 */
		TCC_FUNCTION(0x4, "ms1"),		/* MS_D(1)[0] */
		TCC_FUNCTION(0x5, "edi")),		/* EDIXD25 */
	TCC_PIN(TCC_PINCTRL_PIN_PF22,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPXA */
		TCC_FUNCTION(0x2, "sd1"),		/* SD_CMD(1) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDACK */
		TCC_FUNCTION(0x4, "ms1"),		/* MS_BUS(1) */
		TCC_FUNCTION(0x5, "edi")),		/* EDIXD26 */
	TCC_PIN(TCC_PINCTRL_PIN_PF23,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPINT0 */
		TCC_FUNCTION(0x2, "sd1"),		/* SD_CLK(1) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDRQ */
		TCC_FUNCTION(0x4, "ms1"),		/* MS_CLK(1) */
		TCC_FUNCTION(0x5, "edi")),		/* EDIXD27 */
	TCC_PIN(TCC_PINCTRL_PIN_PF24,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "ehi"),		/* HPINT1 */
		TCC_FUNCTION(0x2, "gpsb9"),		/* SDO(9) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDIOW */
		TCC_FUNCTION(0x5, "edi")),		/* EDIXD28 */
	TCC_PIN(TCC_PINCTRL_PIN_PF25,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "gpsb9"),		/* SDI(9) */
		TCC_FUNCTION(0x3, "hdd"),		/* HDDIOR */
		TCC_FUNCTION(0x5, "edi")),		/* EDIXD29 */
	TCC_PIN(TCC_PINCTRL_PIN_PF26,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "gpsb9"),		/* SCLK(9) */
		TCC_FUNCTION(0x5, "edi"),		/* EDIXD30 */
		TCC_FUNCTION(0x6, "uart2")),		/* UT_TXD(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PF27,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "gpsb9"),		/* SFRM(9) */
		TCC_FUNCTION(0x5, "edi"),		/* EDIXD31 */
		TCC_FUNCTION(0x6, "uart2")),		/* UT_RXD(2) */

	TCC_PIN(TCC_PINCTRL_PIN_PG0,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_TXD[0] */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD16 */
		TCC_FUNCTION(0x3, "sd2"),		/* SD_D(2)[0] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[0] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[0] */
		TCC_FUNCTION(0x6, "ms2")),		/* MS_D(2)[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PG1,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_TXD[1] */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD17 */
		TCC_FUNCTION(0x3, "sd2"),		/* SD_D(2)[1] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[1] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[1] */
		TCC_FUNCTION(0x6, "ms2")),		/* MS_D(2)[1] */
	TCC_PIN(TCC_PINCTRL_PIN_PG2,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_TXD[2] */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD18 */
		TCC_FUNCTION(0x3, "sd2"),		/* SD_D(2)[2] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[2] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[2] */
		TCC_FUNCTION(0x6, "ms2")),		/* MS_D(2)[2] */
	TCC_PIN(TCC_PINCTRL_PIN_PG3,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_TXD[3] */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD19 */
		TCC_FUNCTION(0x3, "sd2"),		/* SD_D(2)[3] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[3] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[3] */
		TCC_FUNCTION(0x6, "ms2")),		/* MS_D(2)[3] */
	TCC_PIN(TCC_PINCTRL_PIN_PG4,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_TXEN */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD20 */
		TCC_FUNCTION(0x3, "sd2"),		/* SD_D(2)[4] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[4] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[4] */
		TCC_FUNCTION(0x6, "ms2")),		/* MS_D(2)[4] */
	TCC_PIN(TCC_PINCTRL_PIN_PG5,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_TXER */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD21 */
		TCC_FUNCTION(0x3, "sd2"),		/* SD_D(2)[5] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[5] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[5] */
		TCC_FUNCTION(0x6, "ms2")),		/* MS_D(2)[5] */
	TCC_PIN(TCC_PINCTRL_PIN_PG6,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_TXCLK */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD22 */
		TCC_FUNCTION(0x3, "sd2"),		/* SD_D(2)[6] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[6] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[6] */
		TCC_FUNCTION(0x6, "ms2")),		/* MS_D(2)[6] */
	TCC_PIN(TCC_PINCTRL_PIN_PG7,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_RXCLK */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD23 */
		TCC_FUNCTION(0x3, "sd2"),		/* SD_D(2)[7] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[7] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[7] */
		TCC_FUNCTION(0x6, "ms2")),		/* MS_D(2)[7] */
	TCC_PIN(TCC_PINCTRL_PIN_PG8,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_RXD[0] */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD24 */
		TCC_FUNCTION(0x3, "sd2"),		/* SD_CLK(2) */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[8] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[8] */
		TCC_FUNCTION(0x6, "ms2")),		/* MS_CLK(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PG9,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_RXD[1] */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD25 */
		TCC_FUNCTION(0x3, "sd2"),		/* SD_CMD(2) */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[9] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[9] */
		TCC_FUNCTION(0x6, "ms2")),		/* MS_BUS(2) */
	TCC_PIN(TCC_PINCTRL_PIN_PG10,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_RXD[2] */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD26 */
		TCC_FUNCTION(0x3, "ts3"),		/* TS_D(3)[0] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[10] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[10] */
	TCC_PIN(TCC_PINCTRL_PIN_PG11,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_RXD[3] */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD27 */
		TCC_FUNCTION(0x3, "ts3"),		/* TS_D(3)[1] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[11] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[11] */
	TCC_PIN(TCC_PINCTRL_PIN_PG12,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_RXDV */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD28 */
		TCC_FUNCTION(0x3, "ts3"),		/* TS_D(3)[2] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[12] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[12] */
	TCC_PIN(TCC_PINCTRL_PIN_PG13,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_RXER */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD29 */
		TCC_FUNCTION(0x3, "ts3"),		/* TS_D(3)[3] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[13] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[13] */
	TCC_PIN(TCC_PINCTRL_PIN_PG14,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_COL */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD30 */
		TCC_FUNCTION(0x3, "ts3"),		/* TS_D(3)[4] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[14] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[14] */
	TCC_PIN(TCC_PINCTRL_PIN_PG15,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_CRS */
		TCC_FUNCTION(0x2, "edi"),		/* EDIXD31 */
		TCC_FUNCTION(0x3, "ts3"),		/* TS_D(3)[5] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[15] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[15] */
	TCC_PIN(TCC_PINCTRL_PIN_PG16,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_REFCLK */
		TCC_FUNCTION(0x2, "edi"),		/* EDI_CSN[7] */
		TCC_FUNCTION(0x3, "ts3"),		/* TS_D(3)[6] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[16] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[16] */
	TCC_PIN(TCC_PINCTRL_PIN_PG17,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_MDC */
		TCC_FUNCTION(0x2, "edi"),		/* EDI_CSN[8] */
		TCC_FUNCTION(0x3, "ts3"),		/* TS_D(3)[7] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[17] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[17] */
	TCC_PIN(TCC_PINCTRL_PIN_PG18,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_MDO/I */
		TCC_FUNCTION(0x2, "edi"),		/* EDI_CSN[9] */
		TCC_FUNCTION(0x3, "ts3"),		/* TSVALID(3) */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[18] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[18] */
	TCC_PIN(TCC_PINCTRL_PIN_PG19,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_GTXCLK */
		TCC_FUNCTION(0x2, "edi"),		/* EDI_CSN[10] */
		TCC_FUNCTION(0x3, "ts3"),		/* TS_CLK(3) */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[19] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[19] */
	TCC_PIN(TCC_PINCTRL_PIN_PG20,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_TXD[4] */
		TCC_FUNCTION(0x2, "edi"),		/* EDI_CSN[11] */
		TCC_FUNCTION(0x3, "ts3"),		/* TSSYNC(3) */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[20] */
		TCC_FUNCTION(0x5, "lcd1rgb")),		/* L1_LPXD[20] */
	TCC_PIN(TCC_PINCTRL_PIN_PG21,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_TXD[5] */
		TCC_FUNCTION(0x2, "trace"),		/* TRACECLK */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D(0)[7] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[21] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[21] */
		TCC_FUNCTION(0x6, "ms0")),		/* MS_D(0)[7] */
	TCC_PIN(TCC_PINCTRL_PIN_PG22,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_TXD[6] */
		TCC_FUNCTION(0x2, "trace"),		/* TRACECTL */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D(0)[6] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[22] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[22] */
		TCC_FUNCTION(0x6, "ms0")),		/* MS_D(0)[6] */
	TCC_PIN(TCC_PINCTRL_PIN_PG23,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_TXD[7] */
		TCC_FUNCTION(0x2, "trace"),		/* TRACESWO */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D(0)[5] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XD[23] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXD[23] */
		TCC_FUNCTION(0x6, "ms0")),		/* MS_D(0)[5] */
	TCC_PIN(TCC_PINCTRL_PIN_PG24,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_RXD[4] */
		TCC_FUNCTION(0x2, "trace"),		/* TRACEDT[0] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D(0)[4] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_WEN */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LACBIAS */
		TCC_FUNCTION(0x6, "ms0")),		/* MS_D(0)[4] */
	TCC_PIN(TCC_PINCTRL_PIN_PG25,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_RXD[5] */
		TCC_FUNCTION(0x2, "trace"),		/* TRACEDT[1] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D(0)[3] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_OEN */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LPXCLK */
		TCC_FUNCTION(0x6, "ms0")),		/* MS_D(0)[3] */
	TCC_PIN(TCC_PINCTRL_PIN_PG26,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_RXD[6] */
		TCC_FUNCTION(0x2, "trace"),		/* TRACEDT[2] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D(0)[2] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_XA */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LHSYNC */
		TCC_FUNCTION(0x6, "ms0")),		/* MS_D(0)[2] */
	TCC_PIN(TCC_PINCTRL_PIN_PG27,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x1, "eth"),		/* ET_RXD[7] */
		TCC_FUNCTION(0x2, "trace"),		/* TRACEDT[3] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D(0)[1] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_CSN[0] */
		TCC_FUNCTION(0x5, "lcd1rgb"),		/* L1_LVSYNC */
		TCC_FUNCTION(0x6, "ms0")),		/* MS_D(0)[1] */
	TCC_PIN(TCC_PINCTRL_PIN_PG28,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "trace"),		/* TRACEDT[4] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_D(0)[0] */
		TCC_FUNCTION(0x4, "lcd1"),		/* LCD1_CSN[1] */
		TCC_FUNCTION(0x5, "gpsb2"),		/* SDO(2) */
		TCC_FUNCTION(0x6, "ms0")),		/* MS_D(0)[0] */
	TCC_PIN(TCC_PINCTRL_PIN_PG29,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "trace"),		/* TRACEDT[5] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_CLK(0) */
		TCC_FUNCTION(0x5, "gpsb2"),		/* SDI(2) */
		TCC_FUNCTION(0x6, "ms0")),		/* MS_CLK(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PG30,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "trace"),		/* TRACEDT[6] */
		TCC_FUNCTION(0x3, "sd0"),		/* SD_CMD(0) */
		TCC_FUNCTION(0x4, "lcd1"),		/* EXTLVS1 */
		TCC_FUNCTION(0x5, "gpsb2"),		/* SCLK(2) */
		TCC_FUNCTION(0x6, "ms0")),		/* MS_BUS(0) */
	TCC_PIN(TCC_PINCTRL_PIN_PG31,
		TCC_FUNCTION(0x0, "gpio"),
		TCC_FUNCTION(0x2, "trace"),		/* TRACEDT[7] */
		TCC_FUNCTION(0x4, "lcd0"),		/* EXTLVS0 */
		TCC_FUNCTION(0x5, "gpsb2")),		/* SFRM(2) */
};

static const int tcc88xx_eints[] = {
	3, 4, 5, 6, 51, 52, 56, 57
};

static const int tcc88xx_bank_offsets[] = {
	0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x1c0,
};

static const struct tcc_pinctrl_desc tcc88xx_pinctrl_data = {
	.pins = tcc88xx_pins,
	.npins = ARRAY_SIZE(tcc88xx_pins),

	.bank_offsets = tcc88xx_bank_offsets,
	.nbanks = ARRAY_SIZE(tcc88xx_bank_offsets),

	.neints = ARRAY_SIZE(tcc88xx_eints),
	.eint_irqs = tcc88xx_eints,
	.eint_sources = tcc88xx_eint_sources,
	.eintsel_offset = 0x184,
};

static struct tcc_pinctrl_group *
tcc_pinctrl_find_group_by_name(struct tcc_pinctrl *pctl, const char *group)
{
	int i;

	for (i = 0; i < pctl->ngroups; i++) {
		struct tcc_pinctrl_group *grp = pctl->groups + i;

		if (!strcmp(grp->name, group))
			return grp;
	}

	return NULL;
}

static struct tcc_pinctrl_function *
tcc_pinctrl_find_function_by_name(struct tcc_pinctrl *pctl,
				    const char *name)
{
	struct tcc_pinctrl_function *func = pctl->functions;
	int i;

	for (i = 0; i < pctl->nfunctions; i++) {
		if (!func[i].name)
			break;

		if (!strcmp(func[i].name, name))
			return func + i;
	}

	return NULL;
}

static struct tcc_desc_function *
tcc_pinctrl_desc_find_function_by_name(struct tcc_pinctrl *pctl,
					 const char *pin_name,
					 const char *func_name)
{
	int i;

	for (i = 0; i < pctl->desc->npins; i++) {
		const struct tcc_desc_pin *pin = pctl->desc->pins + i;

		if (!strcmp(pin->pin.name, pin_name)) {
			struct tcc_desc_function *func = pin->functions;

			while (func->name) {
				if (!strcmp(func->name, func_name))
					return func;

				func++;
			}
		}
	}

	return NULL;
}

static int tcc_pctrl_get_groups_count(struct pinctrl_dev *pctldev)
{
	struct tcc_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->ngroups;
}

static const char *tcc_pctrl_get_group_name(struct pinctrl_dev *pctldev,
					      unsigned group)
{
	struct tcc_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->groups[group].name;
}

static int tcc_pctrl_get_group_pins(struct pinctrl_dev *pctldev,
				      unsigned group,
				      const unsigned **pins,
				      unsigned *num_pins)
{
	struct tcc_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	*pins = (unsigned *)&pctl->groups[group].pin;
	*num_pins = 1;

	return 0;
}

static int tcc_pctrl_dt_node_to_map(struct pinctrl_dev *pctldev,
				      struct device_node *node,
				      struct pinctrl_map **map,
				      unsigned *num_maps)
{
	struct tcc_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	unsigned long *pinconfig;
	struct property *prop;
	const char *function;
	const char *group;
	int ret, nmaps, i = 0;
	u32 val;

	*map = NULL;
	*num_maps = 0;

	ret = of_property_read_string(node, "tcc,function", &function);
	if (ret) {
		dev_err(pctl->dev,
			"missing tcc,function property in node %s\n",
			node->name);
		return -EINVAL;
	}

	nmaps = of_property_count_strings(node, "tcc,pins") * 2;
	if (nmaps < 0) {
		dev_err(pctl->dev,
			"missing tcc,pins property in node %s\n",
			node->name);
		return -EINVAL;
	}

	*map = kmalloc(nmaps * sizeof(struct pinctrl_map), GFP_KERNEL);
	if (!map)
		return -ENOMEM;

	of_property_for_each_string(node, "tcc,pins", prop, group) {
		struct tcc_pinctrl_group *grp =
			tcc_pinctrl_find_group_by_name(pctl, group);
		int j = 0, configlen = 0;

		if (!grp) {
			dev_err(pctl->dev, "unknown pin %s", group);
			continue;
		}

		if (!tcc_pinctrl_desc_find_function_by_name(pctl,
							      grp->name,
							      function)) {
			dev_err(pctl->dev, "unsupported function %s on pin %s",
				function, group);
			continue;
		}

		(*map)[i].type = PIN_MAP_TYPE_MUX_GROUP;
		(*map)[i].data.mux.group = group;
		(*map)[i].data.mux.function = function;

		i++;

		if (of_find_property(node, "tcc,drive", NULL))
			configlen++;
		if (of_find_property(node, "tcc,pull", NULL))
			configlen++;

		if (configlen > 0) {
		(*map)[i].type = PIN_MAP_TYPE_CONFIGS_GROUP;
		(*map)[i].data.configs.group_or_pin = group;

		pinconfig = kzalloc(configlen * sizeof(*pinconfig), GFP_KERNEL);

		if (!of_property_read_u32(node, "tcc,drive", &val)) {
			u16 strength = (val + 1) * 10;
			pinconfig[j++] =
				pinconf_to_config_packed(PIN_CONFIG_DRIVE_STRENGTH,
							 strength);
		}

		if (!of_property_read_u32(node, "tcc,pull", &val)) {
			enum pin_config_param pull = PIN_CONFIG_END;
			if (val == 1)
				pull = PIN_CONFIG_BIAS_PULL_UP;
			else if (val == 2)
				pull = PIN_CONFIG_BIAS_PULL_DOWN;
			pinconfig[j++] = pinconf_to_config_packed(pull, 0);
		}

		(*map)[i].data.configs.configs = pinconfig;
		(*map)[i].data.configs.num_configs = configlen;

		i++;
		}
	}

	*num_maps = i;

	return 0;
}

static void tcc_pctrl_dt_free_map(struct pinctrl_dev *pctldev,
				    struct pinctrl_map *map,
				    unsigned num_maps)
{
	int i;

	for (i = 0; i < num_maps; i++) {
		if (map[i].type == PIN_MAP_TYPE_CONFIGS_GROUP)
			kfree(map[i].data.configs.configs);
	}

	kfree(map);
}

static const struct pinctrl_ops tcc_pctrl_ops = {
	.dt_node_to_map		= tcc_pctrl_dt_node_to_map,
	.dt_free_map		= tcc_pctrl_dt_free_map,
	.get_groups_count	= tcc_pctrl_get_groups_count,
	.get_group_name		= tcc_pctrl_get_group_name,
	.get_group_pins		= tcc_pctrl_get_group_pins,
};

static int tcc_pconf_group_get(struct pinctrl_dev *pctldev,
				 unsigned group,
				 unsigned long *config)
{
	struct tcc_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	*config = pctl->groups[group].config;

	return 0;
}

static int tcc_pconf_group_set(struct pinctrl_dev *pctldev,
				 unsigned group,
				 unsigned long config)
{
	struct tcc_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	struct tcc_pinctrl_group *g = &pctl->groups[group];
	u32 val, mask;
	u16 strength;
	u8 dlevel;

	switch (pinconf_to_config_param(config)) {
	case PIN_CONFIG_DRIVE_STRENGTH:
		strength = pinconf_to_config_argument(config);
		if (strength > 40)
			return -EINVAL;
		/*
		 * We convert from mA to what the register expects:
		 *   0: 10mA
		 *   1: 20mA
		 *   2: 30mA
		 *   3: 40mA
		 */
		dlevel = strength / 10 - 1;
		val = readl(pctl->membase + tcc_dlevel_reg(pctl->desc, g->pin));
	        mask = DLEVEL_PINS_MASK << tcc_dlevel_offset(g->pin);
		writel((val & ~mask) | dlevel << tcc_dlevel_offset(g->pin),
			pctl->membase + tcc_dlevel_reg(pctl->desc, g->pin));
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		val = readl(pctl->membase + tcc_pull_reg(pctl->desc, g->pin));
		mask = PULL_PINS_MASK << tcc_pull_offset(g->pin);
		writel((val & ~mask) | 1 << tcc_pull_offset(g->pin),
			pctl->membase + tcc_pull_reg(pctl->desc, g->pin));
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		val = readl(pctl->membase + tcc_pull_reg(pctl->desc, g->pin));
		mask = PULL_PINS_MASK << tcc_pull_offset(g->pin);
		writel((val & ~mask) | 2 << tcc_pull_offset(g->pin),
			pctl->membase + tcc_pull_reg(pctl->desc, g->pin));
		break;
	default:
		break;
	}

	/* cache the config value */
	g->config = config;

	return 0;
}

static const struct pinconf_ops tcc_pconf_ops = {
	.pin_config_group_get	= tcc_pconf_group_get,
	.pin_config_group_set	= tcc_pconf_group_set,
};

static int tcc_pmx_get_funcs_cnt(struct pinctrl_dev *pctldev)
{
	struct tcc_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->nfunctions;
}

static const char *tcc_pmx_get_func_name(struct pinctrl_dev *pctldev,
					   unsigned function)
{
	struct tcc_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	return pctl->functions[function].name;
}

static int tcc_pmx_get_func_groups(struct pinctrl_dev *pctldev,
				     unsigned function,
				     const char * const **groups,
				     unsigned * const num_groups)
{
	struct tcc_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	*groups = pctl->functions[function].groups;
	*num_groups = pctl->functions[function].ngroups;

	return 0;
}

static void tcc_pmx_set(struct pinctrl_dev *pctldev,
				 unsigned pin,
				 u8 config)
{
	struct tcc_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);

	u32 val = readl(pctl->membase + tcc_mux_reg(pctl->desc, pin));
	u32 mask = MUX_PINS_MASK << tcc_mux_offset(pin);
	writel((val & ~mask) | config << tcc_mux_offset(pin),
		pctl->membase + tcc_mux_reg(pctl->desc, pin));
}

static int tcc_pmx_enable(struct pinctrl_dev *pctldev,
			    unsigned function,
			    unsigned group)
{
	struct tcc_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	struct tcc_pinctrl_group *g = pctl->groups + group;
	struct tcc_pinctrl_function *func = pctl->functions + function;
	struct tcc_desc_function *desc =
		tcc_pinctrl_desc_find_function_by_name(pctl,
							 g->name,
							 func->name);

	if (!desc)
		return -EINVAL;

	tcc_pmx_set(pctldev, g->pin, desc->muxval);

	return 0;
}

static int
tcc_pmx_gpio_set_direction(struct pinctrl_dev *pctldev,
			struct pinctrl_gpio_range *range,
			unsigned offset,
			bool input)
{
	struct tcc_pinctrl *pctl = pinctrl_dev_get_drvdata(pctldev);
	struct tcc_desc_function *desc;
	char pin_name[TCC_PIN_NAME_MAX_LEN];
	u8 bank, pin;
	u8 index;
	u32 reg;
	int ret;

	bank = (offset) / PINS_PER_BANK;
	pin = (offset) % PINS_PER_BANK;

	ret = sprintf(pin_name, "P%c%d", 'A' + bank, pin);
	if (!ret)
		goto error;

	desc = tcc_pinctrl_desc_find_function_by_name(pctl,
							pin_name,
							"gpio");
	if (!desc) {
		ret = -EINVAL;
		goto error;
	}

	tcc_pmx_set(pctldev, offset, desc->muxval);

	reg = tcc_dir_reg(pctl->desc, offset);
	index = tcc_dir_offset(offset);
	if (input)
		writel(readl(pctl->membase + reg) &
			~BIT(index), pctl->membase + reg);
	else
		writel(readl(pctl->membase + reg) |
			BIT(index), pctl->membase + reg);

	ret = 0;

error:
	return ret;
}

static const struct pinmux_ops tcc_pmx_ops = {
	.get_functions_count	= tcc_pmx_get_funcs_cnt,
	.get_function_name	= tcc_pmx_get_func_name,
	.get_function_groups	= tcc_pmx_get_func_groups,
	.enable			= tcc_pmx_enable,
	.gpio_set_direction	= tcc_pmx_gpio_set_direction,
};

static struct pinctrl_desc tcc_pctrl_desc = {
	.confops	= &tcc_pconf_ops,
	.pctlops	= &tcc_pctrl_ops,
	.pmxops		= &tcc_pmx_ops,
};

static int tcc_pinctrl_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	return pinctrl_request_gpio(chip->base + offset);
}

static void tcc_pinctrl_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	pinctrl_free_gpio(chip->base + offset);
}

static int tcc_pinctrl_gpio_direction_input(struct gpio_chip *chip,
					unsigned offset)
{
	return pinctrl_gpio_direction_input(chip->base + offset);
}

static int tcc_pinctrl_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct tcc_pinctrl *pctl = dev_get_drvdata(chip->dev);

	u32 reg = tcc_data_reg(pctl->desc, offset);
	u8 index = tcc_data_offset(offset);
	u32 val = (readl(pctl->membase + reg) >> index) & DATA_PINS_MASK;

	return val;
}

static int tcc_pinctrl_gpio_direction_output(struct gpio_chip *chip,
					unsigned offset, int value)
{
	return pinctrl_gpio_direction_output(chip->base + offset);
}

static void tcc_pinctrl_gpio_set(struct gpio_chip *chip,
				unsigned offset, int value)
{
	struct tcc_pinctrl *pctl = dev_get_drvdata(chip->dev);
	u32 reg = tcc_data_reg(pctl->desc, offset);
	u8 index = tcc_data_offset(offset);

	writel((value & DATA_PINS_MASK) << index, pctl->membase + reg);
}

static int tcc_pinctrl_gpio_to_irq(struct gpio_chip *chip,
				unsigned offset)
{
	struct tcc_pinctrl *pctl = dev_get_drvdata(chip->dev);
	int i;

	for (i = 0; i < pctl->desc->neints; i++)
		if (pctl->eints[i] == offset)
			return pctl->desc->eint_irqs[i];

	return -EINVAL;
}

static int tcc_pinctrl_gpio_of_xlate(struct gpio_chip *gc,
				const struct of_phandle_args *gpiospec,
				u32 *flags)
{
	int pin, base;

	base = PINS_PER_BANK * gpiospec->args[0];
	pin = base + gpiospec->args[1];

	if (pin > (gc->base + gc->ngpio))
		return -EINVAL;

	if (flags)
		*flags = gpiospec->args[2];

	return pin;
}

static struct gpio_chip tcc_pinctrl_gpio_chip = {
	.owner			= THIS_MODULE,
	.request		= tcc_pinctrl_gpio_request,
	.free			= tcc_pinctrl_gpio_free,
	.direction_input	= tcc_pinctrl_gpio_direction_input,
	.direction_output	= tcc_pinctrl_gpio_direction_output,
	.get			= tcc_pinctrl_gpio_get,
	.set			= tcc_pinctrl_gpio_set,
	.to_irq			= tcc_pinctrl_gpio_to_irq,
	.of_xlate		= tcc_pinctrl_gpio_of_xlate,
	.of_gpio_n_cells	= 3,
	.can_sleep		= 0,
};

static struct of_device_id tcc_pinctrl_match[] = {
	{ .compatible = "tcc,tcc92xx-pinctrl", .data = (void *)&tcc92xx_pinctrl_data },
	{ .compatible = "tcc,tcc88xx-pinctrl", .data = (void *)&tcc88xx_pinctrl_data },
	{}
};
MODULE_DEVICE_TABLE(of, tcc_pinctrl_match);

static int tcc_pinctrl_add_function(struct tcc_pinctrl *pctl,
					const char *name)
{
	struct tcc_pinctrl_function *func = pctl->functions;

	while (func->name) {
		/* function already there */
		if (strcmp(func->name, name) == 0) {
			func->ngroups++;
			return -EEXIST;
		}
		func++;
	}

	func->name = name;
	func->ngroups = 1;

	pctl->nfunctions++;

	return 0;
}

static int tcc_pinctrl_build_state(struct platform_device *pdev)
{
	struct tcc_pinctrl *pctl = platform_get_drvdata(pdev);
	int i;

	pctl->ngroups = pctl->desc->npins;

	/* Allocate groups */
	pctl->groups = devm_kzalloc(&pdev->dev,
				    pctl->ngroups * sizeof(*pctl->groups),
				    GFP_KERNEL);
	if (!pctl->groups)
		return -ENOMEM;

	for (i = 0; i < pctl->desc->npins; i++) {
		const struct tcc_desc_pin *pin = pctl->desc->pins + i;
		struct tcc_pinctrl_group *group = pctl->groups + i;

		group->name = pin->pin.name;
		group->pin = pin->pin.number;
	}

	/*
	 * We suppose that we won't have any more functions than pins,
	 * we'll reallocate that later anyway
	 */
	pctl->functions = devm_kzalloc(&pdev->dev,
				pctl->desc->npins * sizeof(*pctl->functions),
				GFP_KERNEL);
	if (!pctl->functions)
		return -ENOMEM;

	/* Count functions and their associated groups */
	for (i = 0; i < pctl->desc->npins; i++) {
		const struct tcc_desc_pin *pin = pctl->desc->pins + i;
		struct tcc_desc_function *func = pin->functions;

		while (func->name) {
			tcc_pinctrl_add_function(pctl, func->name);
			func++;
		}
	}

	pctl->functions = krealloc(pctl->functions,
				pctl->nfunctions * sizeof(*pctl->functions),
				GFP_KERNEL);

	for (i = 0; i < pctl->desc->npins; i++) {
		const struct tcc_desc_pin *pin = pctl->desc->pins + i;
		struct tcc_desc_function *func = pin->functions;

		while (func->name) {
			struct tcc_pinctrl_function *func_item;
			const char **func_grp;

			func_item = tcc_pinctrl_find_function_by_name(pctl,
									func->name);
			if (!func_item)
				return -EINVAL;

			if (!func_item->groups) {
				func_item->groups =
					devm_kzalloc(&pdev->dev,
						     func_item->ngroups * sizeof(*func_item->groups),
						     GFP_KERNEL);
				if (!func_item->groups)
					return -ENOMEM;
			}

			func_grp = func_item->groups;
			while (*func_grp)
				func_grp++;

			*func_grp = pin->pin.name;
			func++;
		}
	}

	return 0;
}

static void tcc_pinctrl_configure_eints(struct tcc_pinctrl *pctl)
{
	char propname[64];
	int i, eint_gpio;

	/* Initialize EINT => GPIO mapping table */
	for (i = 0; i < pctl->desc->neints; i++)
		pctl->eints[i] = -1;

	/* Check for external interrupt configuration */
	for (i = 0; i < pctl->desc->neints; i++) {
		snprintf(propname, sizeof(propname), "tcc,eint%d-gpio", i);
		eint_gpio = of_get_named_gpio_flags(pctl->dev->of_node, propname, 0, NULL);
		if (eint_gpio >= 0) {
			/* Make sure GPIO is from our chip */
			if (eint_gpio >= pctl->chip->base &&
				eint_gpio < pctl->chip->base + pctl->chip->ngpio) {
				/* Lookup gpio in our sources table */
				int j, offset = eint_gpio - pctl->chip->base;
				for (j = 0; pctl->desc->eint_sources[j] != -1; j++) {
					if (pctl->desc->eint_sources[j] == offset)
						break;
				}

				if (pctl->desc->eint_sources[j] != -1) {
					/* Setup EINTSEL to match this config */
					u32 val = readl_relaxed(pctl->membase + pctl->desc->eintsel_offset + (i & 0xfc));
					val &= ~(0xff << (i & 3) * 8);
					val |= j << (i & 3) * 8;
					writel_relaxed(val, pctl->membase + pctl->desc->eintsel_offset + (i & 0xfc));
					/* ... and store mapping to make gpio_to_irq happy */
					pctl->eints[i] = offset;
				} else {
					dev_err(pctl->dev, "invalid gpio %d specified for eint%d, ignored\n", eint_gpio, i);
				}
			} else {
				dev_err(pctl->dev, "external gpio specified for '%s', ignored\n", propname);
			}
		}
	}
}

static int tcc_pinctrl_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	const struct of_device_id *device;
	struct pinctrl_pin_desc *pins;
	struct tcc_pinctrl *pctl;
	int i, ret, last_pin;

	pctl = devm_kzalloc(&pdev->dev, sizeof(*pctl), GFP_KERNEL);
	if (!pctl)
		return -ENOMEM;
	platform_set_drvdata(pdev, pctl);

	pctl->membase = of_iomap(node, 0);
	if (!pctl->membase)
		return -ENOMEM;

	device = of_match_device(tcc_pinctrl_match, &pdev->dev);
	if (!device)
		return -ENODEV;

	pctl->desc = (struct tcc_pinctrl_desc *)device->data;

	ret = tcc_pinctrl_build_state(pdev);
	if (ret) {
		dev_err(&pdev->dev, "dt probe failed: %d\n", ret);
		return ret;
	}

	pins = devm_kzalloc(&pdev->dev,
			    pctl->desc->npins * sizeof(*pins),
			    GFP_KERNEL);
	if (!pins)
		return -ENOMEM;

	for (i = 0; i < pctl->desc->npins; i++)
		pins[i] = pctl->desc->pins[i].pin;

	tcc_pctrl_desc.name = dev_name(&pdev->dev);
	tcc_pctrl_desc.owner = THIS_MODULE;
	tcc_pctrl_desc.pins = pins;
	tcc_pctrl_desc.npins = pctl->desc->npins;
	pctl->dev = &pdev->dev;
	pctl->pctl_dev = pinctrl_register(&tcc_pctrl_desc,
					  &pdev->dev, pctl);
	if (!pctl->pctl_dev) {
		dev_err(&pdev->dev, "couldn't register pinctrl driver\n");
		return -EINVAL;
	}

	pctl->chip = devm_kzalloc(&pdev->dev, sizeof(*pctl->chip), GFP_KERNEL);
	if (!pctl->chip) {
		ret = -ENOMEM;
		goto pinctrl_error;
	}

	last_pin = pctl->desc->pins[pctl->desc->npins - 1].pin.number;
	pctl->chip = &tcc_pinctrl_gpio_chip;
	pctl->chip->ngpio = round_up(last_pin, PINS_PER_BANK);
	pctl->chip->label = dev_name(&pdev->dev);
	pctl->chip->dev = &pdev->dev;
	pctl->chip->base = 0;

	ret = gpiochip_add(pctl->chip);
	if (ret)
		goto pinctrl_error;

	for (i = 0; i < pctl->desc->npins; i++) {
		const struct tcc_desc_pin *pin = pctl->desc->pins + i;

		ret = gpiochip_add_pin_range(pctl->chip, dev_name(&pdev->dev),
					     pin->pin.number,
					     pin->pin.number, 1);
		if (ret)
			goto gpiochip_error;
	}

	tcc_pinctrl_configure_eints(pctl);

	dev_info(&pdev->dev, "initialized Telechips PIO driver\n");

	return 0;

gpiochip_error:
	if (gpiochip_remove(pctl->chip))
		dev_err(&pdev->dev, "failed to remove gpio chip\n");
pinctrl_error:
	pinctrl_unregister(pctl->pctl_dev);
	return ret;
}

static struct platform_driver tcc_pinctrl_driver = {
	.probe = tcc_pinctrl_probe,
	.driver = {
		.name = "tcc-pinctrl",
		.owner = THIS_MODULE,
		.of_match_table = tcc_pinctrl_match,
	},
};


static int __init tcc_pinctrl_init(void)
{
	return platform_driver_register(&tcc_pinctrl_driver);
}
core_initcall(tcc_pinctrl_init);

static void tcc_pinctrl_exit(void)
{
	platform_driver_unregister(&tcc_pinctrl_driver);
}
module_exit(tcc_pinctrl_exit);

MODULE_AUTHOR("Ithamar R. Adema <ithamar@upgrade-android.com>");
MODULE_DESCRIPTION("Telechips pinctrl driver");
MODULE_LICENSE("GPL");
