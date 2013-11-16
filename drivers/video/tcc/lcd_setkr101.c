/*
 * lcd_setkr101.c -- support for HSD1126XA01A LVDS Panel
 *
 * Copyright (C) 2012 Upgrade Android
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
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/hrtimer.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>

#include <mach/tcc_fb.h>
#include <mach/gpio.h>
#include <mach/tca_lcdc.h>
#include <mach/TCC_LCD_Interface.h>
#include <mach/globals.h>
#include <mach/reg_physical.h>
#include <mach/tca_ckc.h>

#include <asm/mach-types.h>
#include <asm/io.h>


static struct mutex panel_lock;
static char lcd_pwr_state;
static unsigned int lcd_bl_level;
extern void lcdc_initialize(struct lcd_panel *lcd_spec, unsigned int lcdc_num);
static struct clk *lvds_clk;


static struct hrtimer bl_timer;
static char hrtimer_inited;
static volatile PTIMER pTIMER;
static char pwm_inited;


static enum hrtimer_restart
audio_power(struct hrtimer *timer)
{
	pr_info("%s\n", __func__);
	gpio_direction_output(TCC_GPG(4), 1);
	return HRTIMER_NORESTART;
}

static void
backlight_pwminit(void)
{
	pr_info("%s 1\n", __func__);
	if (pwm_inited)
		return;

	pr_info("%s 1\n", __func__);

	pwm_inited = 1;

	pTIMER = (volatile PTIMER)tcc_p2v(HwTMR_BASE);
	pTIMER->TCFG0	= 0x0105;
	pTIMER->TCNT0	= 0;
	pTIMER->TREF0	= 0x1FE;
	pTIMER->TMREF0	= 0x1FE - 0x18;
	pTIMER->TCFG0	= 0x0105;
}

static void
backlight_pwmenable(int on)
{
	pr_info("%s on:%d hrtimer_inited: %d\n", __func__, on, hrtimer_inited);
	if (!hrtimer_inited) {
		hrtimer_init(&bl_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		bl_timer.function = audio_power;
		hrtimer_inited = 1;
	}

	if (on) {
		gpio_direction_output(TCC_GPG(2), 1);
		msleep(5);

		tcc_gpio_config(TCC_GPA(4), GPIO_FN(2) | GPIO_CD(3));
		msleep(20);

		hrtimer_start(&bl_timer, ktime_set(0,1), HRTIMER_MODE_ABS);
	} else {
		tcc_gpio_config(TCC_GPA(4), GPIO_FN(0));
		gpio_direction_output(TCC_GPA(4), 0);

		gpio_direction_output(TCC_GPG(2), 0);

		hrtimer_cancel(&bl_timer);
	}
}

static void
lcd_power_seton(int on)
{
	pr_info("%s: on=%d\n", __func__, on);

	if (on) {
		lcd_pwr_state = 1;
		gpio_direction_output(TCC_GPC(28), 1);
		msleep(20);
		gpio_direction_output(TCC_GPG(2), 1);
		msleep(400);
	} else {
		msleep(1);
	}
}

static void
lvds_on(struct lcd_panel *panel)
{
	PDDICONFIG pDDICfg = (volatile PDDICONFIG)tcc_p2v(HwDDI_CONFIG_BASE);
	unsigned int P, M, S, VSEL;

	clk_enable(lvds_clk);

	// LVDS reset
	BITSET(pDDICfg->LVDS_CTRL, Hw1);	// reset
	BITCLR(pDDICfg->LVDS_CTRL, Hw1);	// normal

	pDDICfg->LVDS_TXO_SEL0 = 0x15141312; // SEL_03, SEL_02, SEL_01, SEL_00,
	pDDICfg->LVDS_TXO_SEL1 = 0x0B0A1716; // SEL_07, SEL_06, SEL_05, SEL_04,
	pDDICfg->LVDS_TXO_SEL2 = 0x0F0E0D0C; // SEL_11, SEL_10, SEL_09, SEL_08,
	pDDICfg->LVDS_TXO_SEL3 = 0x05040302; // SEL_15, SEL_14, SEL_13, SEL_12,
	pDDICfg->LVDS_TXO_SEL4 = 0x1A190706; // SEL_19, SEL_18, SEL_17, SEL_16,
	pDDICfg->LVDS_TXO_SEL5 = 0x1F1E1F18; // SEL_20,
	pDDICfg->LVDS_TXO_SEL6 = 0x1F1E1F1E;
	pDDICfg->LVDS_TXO_SEL7 = 0x1F1E1F1E;
	pDDICfg->LVDS_TXO_SEL8 = 0x1F1E1F1E;

	// LVDS Select
	//BITCLR(pDDICfg->LVDS_CTRL, Hw0); //LCDC0
	BITSET(pDDICfg->LVDS_CTRL, Hw0); //LCDC1

	{
		
		if (panel->clk_freq > 600000U) {
			VSEL = S = 0;
			M = P = 10;
		} else if (panel->clk_freq < 450000) {
			VSEL = 0;
			S = 1;
			M = P = 5;
		} else {
			VSEL = S = 1;
			M = P = 7;
		}
		BITCSET(pDDICfg->LVDS_CTRL, Hw21- Hw4, (VSEL<<4)|(S<<5)|(M<<8)|(P<<15)); //LCDC1
	}

	BITSET(pDDICfg->LVDS_CTRL, Hw1);	// reset
	BITSET(pDDICfg->LVDS_CTRL, Hw2);	// enable
}


static int setkr101_set_power(struct lcd_panel *panel, int on, unsigned int lcd_num)
{
	struct lcd_platform_data *pdata = panel->dev->platform_data;
	printk("%s : %d %d  \n", __func__, on, lcd_bl_level);

	mutex_lock(&panel_lock);
	lcd_pwr_state = on;

	if (on) {
		lcd_power_seton(1);
		msleep(20);
		lcdc_initialize(panel, lcd_num);
		LCDC_IO_Set(lcd_num, panel->bus_width);
		lvds_on(panel);

		if (lcd_bl_level != 0) {
			int val = (lcd_bl_level <= 10) ? 20 : (lcd_bl_level * 28) / 19;
			pTIMER = (volatile PTIMER)tcc_p2v(HwTMR_BASE);
			pTIMER->TMREF0 = val;
			backlight_pwmenable(1);
		} else {
			msleep(10);
		}
	} else {
		PDDICONFIG pDDICfg = (volatile PDDICONFIG)tcc_p2v(HwDDI_CONFIG_BASE);
		BITCLR(pDDICfg->LVDS_CTRL, Hw1);	// reset
		BITCLR(pDDICfg->LVDS_CTRL, Hw2);	// disable

		clk_disable(lvds_clk);
		backlight_pwmenable(0);
		LCDC_IO_Disable(lcd_num, panel->bus_width);
		msleep(20);
		lcd_power_seton(0);
	}

	return 0;
}

static int setkr101_set_backlight_level(struct lcd_panel *panel, int level)
{
	struct lcd_platform_data *pdata = panel->dev->platform_data;

	printk("### %s power =(%d) level = (%d) \n",__func__, lcd_pwr_state, level);
	mutex_lock(&panel_lock);
	lcd_bl_level = level;

	if (level >= 7) {
		int val = (level <= 10) ? 20 : (level * 28) / 19;
		pTIMER = (volatile PTIMER)tcc_p2v(HwTMR_BASE);
		pTIMER->TMREF0 = val;

		if (lcd_pwr_state)
			backlight_pwmenable(1);
	} else {
		backlight_pwmenable(0);
	}

	mutex_unlock(&panel_lock);
	return 0;
}

static int setkr101_panel_init(struct lcd_panel *panel)
{
	printk("%s : %d\n", __func__, 0);
	backlight_pwminit();
	return 0;
}

static struct lcd_panel setkr101_panel = {
	.name		= "SETKR101",
	.manufacturer	= "SET",
	.dev		= NULL,
	.id		= PANEL_ID_SETKR101, //13
	.xres		= 1024,
	.yres		= 600,
	.width		= 211,
	.height		= 130,
	.bpp		= 18,
	.clk_freq	= 50400,
	.clk_div	= 1,
	.bus_width	= 18,
	.lpw		= 0,
	.lpc		= 1024,
	.lswc		= 0,
	.lewc		= 320,
	.vdb		= 0,
	.vdf		= 0,
	.fpw1		= 0,
	.flc1		= 600,
	.fswc1		= 25,
	.fewc1		= 25,
	.fpw2		= 0,
	.flc2		= 600,
	.fswc2		= 25,
	.fewc2		= 25,
	.sync_invert	= IV_INVERT | IH_INVERT,
	.init		= setkr101_panel_init,
	.set_power	= setkr101_set_power,
	.set_backlight_level = setkr101_set_backlight_level,
};

static int setkr101_probe(struct platform_device *pdev)
{
	struct lcd_platform_data *pdata = pdev->dev.platform_data;
	pr_info("%s\n", __func__);
	mutex_init(&panel_lock);
	lcd_pwr_state = 1;
	setkr101_panel.dev = &pdev->dev;

	lvds_clk = clk_get(0, "lvds");
	BUG_ON(lvds_clk == NULL);

	lvds_on(&setkr101_panel);

	tccfb_register_panel(&setkr101_panel);

	return 0;
}

static int setkr101_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver setkr101_lcd = {
	.probe	= setkr101_probe,
	.remove	= setkr101_remove,
	.driver	= {
		.name	= "setkr101_lcd",
		.owner	= THIS_MODULE,
	},
};

static __init int setkr101_init(void)
{
	pr_info("~ %s ~\n", __func__);
	return platform_driver_register(&setkr101_lcd);
}

static __exit void setkr101_exit(void)
{
	return platform_driver_unregister(&setkr101_lcd);
}

subsys_initcall(setkr101_init);
module_exit(setkr101_exit);

MODULE_DESCRIPTION("SETKR101 LCD driver");
MODULE_LICENSE("GPL");
