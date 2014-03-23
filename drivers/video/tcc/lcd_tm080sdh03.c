/*
 * tm080sdh03_lcd.c -- support for SAMSUNG TM080SDH03
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
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/io.h>

#include <mach/tcc_fb.h>
#include <mach/gpio.h>
#include <mach/tca_lcdc.h>
#include <mach/TCC_LCD_DriverCtrl.h>
#include <mach/TCC_LCD_Interface.h>
#include <mach/bsp.h>

static struct mutex panel_lock;
static char lcd_pwr_state;
static unsigned int lcd_bl_level = 255;
static char bl_pwr_state;

static volatile PTIMER pTIMER;
static char pwminitflag = 0;

extern void lcdc_initialize(struct lcd_panel *lcd_spec, unsigned int lcdc_num);

static void
lcd_power_seton(int on)
{
	pr_info("%s\n", __func__);
	if (on) {
		volatile PGPION pGPIOC = (volatile PGPION)tcc_p2v(HwGPIOC_BASE);
		volatile PGPION pGPIOG = (volatile PGPION)tcc_p2v(HwGPIOG_BASE);
		lcd_pwr_state = 1;

		pGPIOC->GPEN |= (1 << 28);
		pGPIOC->GPDAT |= (1 << 28);
		msleep(20);
		pGPIOG->GPEN |= (1 << 2);
		pGPIOG->GPDAT |= (1 << 2);
		msleep(400);
	} else {
		msleep(1);
	}

	lcd_pwr_state = on;
}

static void
backlight_pwminit(void)
{
	if (pwminitflag)
		return;

	pr_info("%s\n", __func__);

	pwminitflag = 1;

	pTIMER = (volatile PTIMER)tcc_p2v(HwTMR_BASE);
	pTIMER->TCFG0	= 0x0105;
	pTIMER->TCNT0	= 0;
	pTIMER->TREF0	= 0x1f4;
	pTIMER->TMREF0	= 0xde;
	pTIMER->TCFG0	= 0x0105;
/*
	gpio_request(TCC_GPA(4), "bl_pwm");
	gpio_request(TCC_GPG(2), "bl_power");
	gpio_request(TCC_GPC(28), "lcd_reset");
*/
	tcc_gpio_config(TCC_GPA(4), GPIO_FN(2) | GPIO_CD(3));
}

static void
backlight_pwmenable(int on)
{
	pr_info("%s %d\n", __func__, on);
	if (on) {
		gpio_direction_output(TCC_GPG(2), 1);
		msleep(5);
		tcc_gpio_config(TCC_GPA(4), GPIO_FN(2) | GPIO_CD(3));
		msleep(20);
	} else {
		tcc_gpio_config(TCC_GPA(4), GPIO_FN(0));
		gpio_direction_output(TCC_GPG(2), 0);
	}
}

static void
backlight_setpwm(unsigned int level)
{
	unsigned int val = (level * 100) / 255;
	backlight_pwminit();

	if (val)
		val = ((val * 7) / 10) + 110;

	pr_info("%s %u (val=%u)\n", __func__, level, val);

	pTIMER->TMREF0 = val;
}



static int tm080sdh03_panel_init(struct lcd_panel *panel)
{
	pr_info("%s\n", __func__);
	backlight_pwminit();
	return 0;
}

static int tm080sdh03_set_power(struct lcd_panel *panel, int on, unsigned int lcdc_num)
{
	pr_info("%s %d [%u]\n", __func__, on, lcdc_num);
	mutex_lock(&panel_lock);
	lcd_pwr_state = on;

	if (on) {
		lcd_power_seton(1);
		lcdc_initialize(panel, lcdc_num);
		LCDC_IO_Set(lcdc_num, panel->bus_width);

		if (lcd_bl_level != 0) {
			mdelay(80);
			backlight_setpwm(lcd_bl_level);
			backlight_pwmenable(1);
		} else
			mdelay(10);
	} else {
		mdelay(10);
		backlight_pwmenable(0);
		LCDC_IO_Disable(lcdc_num, panel->bus_width);
	}

	mutex_unlock(&panel_lock);
	return 0;
}

static int tm080sdh03_set_backlight_level(struct lcd_panel *panel, int level)
{
	pr_info("%s %d\n", __func__, level);
	mutex_lock(&panel_lock);

	printk("### %s  level=%d       lcd_pwr_state =%d ###\n",
		__func__, level, lcd_pwr_state);

	lcd_bl_level = level;
	if (level > 7)
		backlight_setpwm(level);

	backlight_pwmenable(level > 7);

	mutex_unlock(&panel_lock);
	return 0;
}

static struct lcd_panel tm080sdh03_panel = {
	.name		= "TM080SDH03",
	.manufacturer	= "Samsung",
	.dev		= NULL,
	.id		= PANEL_ID_TM080SDH03,
	.xres		= 800,
	.yres		= 600,
	.width		= 103,
	.height		= 62,
	.bpp		= 24,
	.clk_freq	= 400000,
	.clk_div	= 1,
	.bus_width	= 24,
	.lpw		= 8,
	.lpc		= 800,
	.lswc		= 77,
	.lewc		= 28,
	.vdb		= 46,
	.vdf		= 210,
	.fpw1		= 0,
	.flc1		= 600,
	.fswc1		= 37,
	.fewc1		= 12,
	.fpw2		= 0,
	.flc2		= 600,
	.fswc2		= 37,
	.fewc2		= 12,
	.sync_invert	= 6,
	.init		= tm080sdh03_panel_init,
	.set_power	= tm080sdh03_set_power,
	.set_backlight_level = tm080sdh03_set_backlight_level,
};

static int tm080sdh03_probe(struct platform_device *pdev)
{
	pr_info("%s\n", __func__);
	mutex_init(&panel_lock);

	lcd_pwr_state = 1;
	tm080sdh03_panel.dev = &pdev->dev;
	tccfb_register_panel(&tm080sdh03_panel);

	return 0;
}

static int tm080sdh03_remove(struct platform_device *pdev)
{
	pr_info("%s\n", __func__);
	return 0;
}

static struct platform_driver tm080sdh03_lcd = {
	.probe	= tm080sdh03_probe,
	.remove	= tm080sdh03_remove,
	.driver	= {
		.name	= "tm080sdh03_lcd",
		.owner	= THIS_MODULE,
	},
};

static __init int tm080sdh03_init(void)
{
	pr_info("~ %s ~\n", __func__);
	return platform_driver_register(&tm080sdh03_lcd);
}

static __exit void tm080sdh03_exit(void)
{
	pr_info("%s\n", __func__);
	return platform_driver_unregister(&tm080sdh03_lcd);
}

subsys_initcall(tm080sdh03_init);
module_exit(tm080sdh03_exit);

MODULE_DESCRIPTION("TM080SDH03 LCD driver");
MODULE_LICENSE("GPL");
