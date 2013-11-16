/*
 * lb080wv6_lcd.c -- support for LG PHILIPS LB080WV6 LCD
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
static char pwm_timer_inited;

extern void lcdc_initialize(struct lcd_panel *lcd_spec, unsigned int lcdc_num);

void
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

void
backlight_pwminit(void)
{
	pr_info("%s\n", __func__);
	if (pwm_timer_inited)
		return;

	pwm_timer_inited = 1;

	pTIMER = (volatile PTIMER)tcc_p2v(HwTMR_BASE);
	pTIMER->TCFG0	= 0x0105;
	pTIMER->TCNT0	= 0;
	pTIMER->TREF0	= 0x640;
	pTIMER->TMREF0	= 0x1a9;
	pTIMER->TCFG0	= 0x0105;
/*
	gpio_request(TCC_GPA(4), "bl_pwm");
	gpio_request(TCC_GPG(2), "bl_power");
	gpio_request(TCC_GPC(28), "lcd_reset");
*/
	tcc_gpio_config(TCC_GPA(4), GPIO_FN(2));
}

void
backlight_pwmenable(int on)
{
	pr_info("%s\n", __func__);
	if (on) {
		gpio_direction_output(TCC_GPG(2), 1);
		msleep(5);
		tcc_gpio_config(TCC_GPA(4), GPIO_FN(2));
		msleep(20);
	} else {
		tcc_gpio_config(TCC_GPA(4), GPIO_FN(0));
		gpio_direction_output(TCC_GPG(2), 0);
	}
}

void
backlight_setpwm(unsigned int level)
{
	unsigned int val = (level * 100) / 255;
	pr_info("%s\n", __func__);
	backlight_pwminit();

	if (val > 5)
		val = (val + 60) << 1;
	else
		val = 150;

	pTIMER->TMREF0 = val;
}

void
backlight_step_pwm(unsigned int level_start, unsigned int level_end)
{
	unsigned int i;
	pr_info("%s\n", __func__);

	if (level_end == 0) {
		/* We're fading out */
		if (level_start <= 7)
			return;

		level_start -= 2;

		for (i=1; i <= 100; i++) {
			if (level_start <= 6)
				return;
			backlight_setpwm(level_start);
			mdelay(1);
		}
	} else {
		if (level_start > 7) {
			for (i=2; i != 200 && i < level_end; i++) {
				backlight_setpwm(i);
				mdelay(1);
			}
		}
	}

	backlight_setpwm(level_end);
}

static int lb080wv6_panel_init(struct lcd_panel *panel)
{
	pr_info("%s\n", __func__);
	backlight_pwminit();
	return 0;
}

static int lb080wv6_set_power(struct lcd_panel *panel, int on, unsigned int lcdc_num)
{
	pr_info("%s\n", __func__);
	mutex_lock(&panel_lock);
	lcd_pwr_state = on;

	if (on) {
		lcd_power_seton(1);
		lcdc_initialize(panel, lcdc_num);
		LCDC_IO_Set(lcdc_num, panel->bus_width);

		if (lcd_bl_level != 0) {
			mdelay(80);
			backlight_setpwm(0);
			backlight_pwmenable(1);
			backlight_step_pwm(1,lcd_bl_level);
			bl_pwr_state = 1;
		} else
			mdelay(10);
	} else {
		mdelay(10);
		backlight_step_pwm(lcd_bl_level,0);
		backlight_pwmenable(0);
		bl_pwr_state = 0;
		lcd_power_seton(0);
		lcd_pwr_state = 0;
		LCDC_IO_Disable(lcdc_num, panel->bus_width);
	}

	mutex_unlock(&panel_lock);
	return 0;
}

static int lb080wv6_set_backlight_level(struct lcd_panel *panel, int level)
{
	pr_info("%s\n", __func__);
	mutex_lock(&panel_lock);

	if (lcd_pwr_state) {
		printk("### %s  level=%d       lcd_pwr_state =%d ###\n",
			__func__, level, lcd_pwr_state);

		lcd_bl_level = level;
		if (level <= 7) {
			/* backlight too low, disable pwm */
			backlight_pwmenable(0);
		} else {
			if (bl_pwr_state != 0) {
				/* backlight pwm was already on; just set */
				backlight_setpwm(level);
			} else {
				/* backlight was off; gently turn it on */
				backlight_setpwm(0);
				backlight_pwmenable(1);
				backlight_step_pwm(level,1);
				bl_pwr_state = 1;
			}
		}
	}

	mutex_unlock(&panel_lock);
	return 0;
}

static struct lcd_panel lb080wv6_panel = {
	.name		= "LB070WV6",
	.manufacturer	= "Samsung",
	.dev		= NULL,
	.id		= PANEL_ID_LB070WV6,
	.xres		= 800,
	.yres		= 480,
	.width		= 103,
	.height		= 62,
	.bpp		= 24,
	.clk_freq	= 330000,
	.clk_div	= 1,
	.bus_width	= 24,
	.lpw		= 128,
	.lpc		= 800,
	.lswc		= 86,
	.lewc		= 40,
	.vdb		= 20,
	.vdf		= 10,
	.fpw1		= 525,
	.flc1		= 480,
	.fswc1		= 31,
	.fewc1		= 10,
	.fpw2		= 525,
	.flc2		= 480,
	.fswc2		= 31,
	.fewc2		= 10,
	.sync_invert	= IV_INVERT | IH_INVERT,
	.init		= lb080wv6_panel_init,
	.set_power	= lb080wv6_set_power,
	.set_backlight_level = lb080wv6_set_backlight_level,
};

static int lb080wv6_probe(struct platform_device *pdev)
{
	pr_info("%s\n", __func__);
	mutex_init(&panel_lock);

	lb080wv6_panel.dev = &pdev->dev;
	tccfb_register_panel(&lb080wv6_panel);

	return 0;
}

static int lb080wv6_remove(struct platform_device *pdev)
{
	pr_info("%s\n", __func__);
	return 0;
}

static struct platform_driver lb080wv6_lcd = {
	.probe	= lb080wv6_probe,
	.remove	= lb080wv6_remove,
	.driver	= {
		.name	= "lb080wv6_lcd",
		.owner	= THIS_MODULE,
	},
};

static __init int lb080wv6_init(void)
{
	pr_info("~ %s ~\n", __func__);
	return platform_driver_register(&lb080wv6_lcd);
}

static __exit void lb080wv6_exit(void)
{
	pr_info("%s\n", __func__);
	return platform_driver_unregister(&lb080wv6_lcd);
}

subsys_initcall(lb080wv6_init);
module_exit(lb080wv6_exit);

MODULE_DESCRIPTION("LB080WV6 LCD driver");
MODULE_LICENSE("GPL");
