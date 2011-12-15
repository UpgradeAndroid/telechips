/*
 * at070tn93_lcd.c -- support for TPO AT070TN93 LCD
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
#include <linux/mutex.h>

#include <mach/hardware.h>
#include <mach/bsp.h>
#include <asm/io.h>
#include <asm/mach-types.h>

#include <mach/tcc_fb.h>
#include <mach/gpio.h>
#include <mach/tca_lcdc.h>
#include <mach/TCC_LCD_Interface.h>

#if defined(CONFIG_ARCH_TCC892X)
#include <mach/reg_physical.h>
#endif


static struct mutex panel_lock;
static char lcd_pwr_state;
static unsigned int lcd_bl_level;
char lcdc_number;
extern void lcdc_initialize(struct lcd_panel *lcd_spec);



static int at070tn93_panel_init(struct lcd_panel *panel)
{
	printk("%s : %d\n", __func__, 0);
	return 0;
}

static int at070tn93_set_power(struct lcd_panel *panel, int on, unsigned int lcd_num)
{
	struct lcd_platform_data *pdata = panel->dev->platform_data;

	mutex_lock(&panel_lock);
	lcd_pwr_state = on;

	printk("%s : %d %d  lcd number = (%d) \n", __func__, on, lcd_bl_level, lcd_num);

	if (on) {
		
		gpio_set_value(pdata->power_on, 1);
		udelay(100);

		gpio_set_value(pdata->reset, 1);
		msleep(20);

		lcdc_initialize(panel);
		LCDC_IO_Set(lcd_num, panel->bus_width);

		if(lcd_bl_level)		{
			msleep(80); 	
		#if defined(CONFIG_ARCH_TCC892X)
			tcc_gpio_config(pdata->bl_on, GPIO_FN(9));
		#else
			tcc_gpio_config(pdata->bl_on, GPIO_FN(2));
		#endif
		}else{
			msleep(80);
		}
	}
	else 
	{
		msleep(10);
		gpio_set_value(pdata->reset, 0);
		gpio_set_value(pdata->power_on, 0);

		LCDC_IO_Disable(lcd_num, panel->bus_width);

	}
	mutex_unlock(&panel_lock);

	return 0;
}

static int at070tn93_set_backlight_level(struct lcd_panel *panel, int level)
{

	#define MAX_BL_LEVEL	255	
	volatile PTIMER pTIMER;

	struct lcd_platform_data *pdata = panel->dev->platform_data;

//	printk("%s : %d\n", __func__, level);
	
	mutex_lock(&panel_lock);
	lcd_bl_level = level;

#if 1
	if (level == 0) {
		tcc_gpio_config(pdata->bl_on, GPIO_FN(0));
		gpio_set_value(pdata->bl_on, 0);
	} else {

#if defined(CONFIG_ARCH_TCC892X)

		if(lcd_pwr_state)
			tcc_gpio_config(pdata->bl_on, GPIO_FN(9));

		pTIMER	= (volatile PTIMER)tcc_p2v(HwTMR_BASE);
		pTIMER->TREF1.nREG = MAX_BL_LEVEL;
		pTIMER->TCFG1.nREG	= 0x105;	
		pTIMER->TMREF1.nREG = (level | 0x07);
		pTIMER->TCFG1.nREG	= 0x105;
#else
	
		if(lcd_pwr_state)
			tcc_gpio_config(pdata->bl_on, GPIO_FN(2));

		pTIMER	= (volatile PTIMER)tcc_p2v(HwTMR_BASE);
		pTIMER->TREF0 = MAX_BL_LEVEL;
		pTIMER->TCFG0	= 0x105;	
		pTIMER->TMREF0 = (level | 0x07);
		pTIMER->TCFG0	= 0x105;
#endif

	}
#endif//
	mutex_unlock(&panel_lock);

	return 0;
}

static struct lcd_panel at070tn93_panel = {
	.name		= "AT070TN93",
	.manufacturer	= "INNOLUX",
	.id		= PANEL_ID_AT070TN93,
	.xres		= 800,
	.yres		= 480,
	.width		= 154,
	.height		= 85,
	.bpp		= 24,
	.clk_freq	= 310000,
	.clk_div	= 2,
	.bus_width	= 24,
	.lpw		= 10,
	.lpc		= 800,
	.lswc		= 34,
	.lewc		= 130,
	.vdb		= 0,
	.vdf		= 0,
	.fpw1		= 1,
	.flc1		= 480,
	.fswc1		= 20,
	.fewc1		= 21,
	.fpw2		= 1,
	.flc2		= 480,
	.fswc2		= 20,
	.fewc2		= 21,
	.sync_invert	= IV_INVERT | IH_INVERT,
	.init		= at070tn93_panel_init,
	.set_power	= at070tn93_set_power,
	.set_backlight_level = at070tn93_set_backlight_level,
};

static int at070tn93_probe(struct platform_device *pdev)
{
	printk("%s : %d\n", __func__, 0);

	mutex_init(&panel_lock);
	lcd_pwr_state = 1;

	at070tn93_panel.dev = &pdev->dev;
	
	tccfb_register_panel(&at070tn93_panel);
	return 0;
}

static int at070tn93_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver at070tn93_lcd = {
	.probe	= at070tn93_probe,
	.remove	= at070tn93_remove,
	.driver	= {
		.name	= "at070tn93_lcd",
		.owner	= THIS_MODULE,
	},
};

static __init int at070tn93_init(void)
{
	printk("~ %s ~ \n", __func__);
	return platform_driver_register(&at070tn93_lcd);
}

static __exit void at070tn93_exit(void)
{
	platform_driver_unregister(&at070tn93_lcd);
}

subsys_initcall(at070tn93_init);
module_exit(at070tn93_exit);

MODULE_DESCRIPTION("AT070TN93 LCD driver");
MODULE_LICENSE("GPL");
