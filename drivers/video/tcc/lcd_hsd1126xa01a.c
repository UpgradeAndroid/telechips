/*
 * lcd_hsd1126xa01a.c -- support for HSD1126XA01A LVDS Panel
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
	pTIMER->TREF0	= 0x1F4;
	pTIMER->TMREF0	= 0x1F4 + 0x39;
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
backlight_setpwm(unsigned int level)
{
	int val = (level * 100) / 255;
	if (val) {
		val *= 345;
		val /= 100;
		val += 5;
	}

	pr_info("%s val: %d\n", __func__, val);
	pTIMER = (volatile PTIMER)tcc_p2v(HwTMR_BASE);
	pTIMER->TMREF0 = val;
}

static void
lcd_power_seton(int on)
{
	pr_info("%s: on=%d\n", __func__, on);

	if (on) {
		gpio_direction_output(TCC_GPC(28), 1);
		msleep(20);
		gpio_direction_output(TCC_GPG(2), 1);
		msleep(400);
	} else {
		msleep(1);
	}
}

static int hsd1126xa01a_set_power(struct lcd_panel *panel, int on, unsigned int lcd_num)
{
	struct lcd_platform_data *pdata = panel->dev->platform_data;
	printk("%s : %d %d  \n", __func__, on, lcd_bl_level);

	mutex_lock(&panel_lock);
	lcd_pwr_state = on;

	if (on) {
		lcd_power_seton(1);
		LCDC_IO_Set(lcd_num, panel->bus_width);
		msleep(20);
		lcdc_initialize(panel, lcd_num);

		if (lcd_bl_level != 0) {
			backlight_setpwm(lcd_bl_level);
			backlight_pwmenable(1);
		} else {
			msleep(10);
		}
	} else {
		LCDC_IO_Disable(lcd_num, panel->bus_width);
	}

	return 0;
}

static int hsd1126xa01a_set_backlight_level(struct lcd_panel *panel, int level)
{
	struct lcd_platform_data *pdata = panel->dev->platform_data;

	printk("### %s power =(%d) level = (%d) \n",__func__, lcd_pwr_state, level);
	mutex_lock(&panel_lock);
	lcd_bl_level = level;

	if (level >= 7) {
		backlight_setpwm(level);

		if (lcd_pwr_state)
			backlight_pwmenable(1);
	} else {
		backlight_pwmenable(0);
	}

	mutex_unlock(&panel_lock);
	return 0;
}

static int hsd1126xa01a_panel_init(struct lcd_panel *panel)
{
	printk("%s : %d\n", __func__, 0);
	backlight_pwminit();
	return 0;
}

static struct lcd_panel hsd1126xa01a_panel = {
	.name		= "HSD1126XA01A",
	.manufacturer	= "HSD",
	.dev		= NULL,
	.id		= PANEL_ID_HSD1126XA01A, //12
	.xres		= 1024,
	.yres		= 768,
	.width		= 150,
	.height		= 108,
	.bpp		= 24,
	.clk_freq	= 60000,
	.clk_div	= 2,
	.bus_width	= 18,
	.lpw		= 0,
	.lpc		= 1024,
	.lswc		= 0,
	.lewc		= 320,
	.vdb		= 0,
	.vdf		= 0,
	.fpw1		= 0,
	.flc1		= 768,
	.fswc1		= 38,
	.fewc1		= 38,
	.fpw2		= 0,
	.flc2		= 768,
	.fswc2		= 38,
	.fewc2		= 38,
	.sync_invert	= IV_INVERT | IH_INVERT,
	.init		= hsd1126xa01a_panel_init,
	.set_power	= hsd1126xa01a_set_power,
	.set_backlight_level = hsd1126xa01a_set_backlight_level,
};

static int hsd1126xa01a_probe(struct platform_device *pdev)
{
	struct lcd_platform_data *pdata = pdev->dev.platform_data;
	pr_info("%s\n", __func__);
	mutex_init(&panel_lock);
	lcd_pwr_state = 1;

	gpio_request(pdata->power_on, "lcd_on");
	gpio_request(pdata->bl_on, "lcd_bl");
	gpio_request(pdata->display_on, "lcd_display");
	gpio_request(pdata->reset, "lcd_reset");

	gpio_direction_output(pdata->power_on, 1);
	gpio_direction_output(pdata->display_on, 1);
	gpio_direction_output(pdata->bl_on, 1);
	gpio_direction_output(pdata->reset, 1);

	hsd1126xa01a_panel.dev = &pdev->dev;

	lvds_clk = clk_get(0, "lvds");
	BUG_ON(lvds_clk == NULL);
	clk_enable(lvds_clk);	

	tccfb_register_panel(&hsd1126xa01a_panel);

	return 0;
}

static int hsd1126xa01a_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver hsd1126xa01a_lcd = {
	.probe	= hsd1126xa01a_probe,
	.remove	= hsd1126xa01a_remove,
	.driver	= {
		.name	= "hsd1126xa01a_lcd",
		.owner	= THIS_MODULE,
	},
};

static __init int hsd1126xa01a_init(void)
{
	pr_info("~ %s ~\n", __func__);
	return platform_driver_register(&hsd1126xa01a_lcd);
}

static __exit void hsd1126xa01a_exit(void)
{
	return platform_driver_unregister(&hsd1126xa01a_lcd);
}



subsys_initcall(hsd1126xa01a_init);
module_exit(hsd1126xa01a_exit);

MODULE_DESCRIPTION("HSD1126XA01A LCD driver");
MODULE_LICENSE("GPL");
