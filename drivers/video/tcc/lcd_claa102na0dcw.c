
/*
 * claa102na0dcw_lcd.c -- support for CPT CLAA104XA01CW LCD
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
#include <linux/clk.h>

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
extern void lcdc_initialize(struct lcd_panel *lcd_spec);

static struct clk *lvds_clk;

static int claa102na0dcw_panel_init(struct lcd_panel *panel)
{
	printk("%s : %d\n", __func__, 0);
	return 0;
}

static int claa102na0dcw_set_power(struct lcd_panel *panel, int on)
{
	PDDICONFIG	pDDICfg;
	unsigned int P, M, S, VSEL;

	struct lcd_platform_data *pdata = panel->dev->platform_data;
//	printk("%s : %d %d  \n", __func__, on, panel->bl_level);

	mutex_lock(&panel_lock);
	lcd_pwr_state = on;

	pDDICfg = (volatile PDDICONFIG)tcc_p2v(HwDDI_CONFIG_BASE);

	if (on) {

		//gpio_set_value( (GPIO_PORTF|13) , 1);	// LVDS_Module_PWR	
		gpio_set_value(pdata->power_on, 1);		
		gpio_set_value(pdata->reset, 1);		// LVDS LED_EN
		
		// LVDS power on
		clk_enable(lvds_clk);	
		
		lcdc_initialize(panel);
	
		// LVDS reset
		BITSET(pDDICfg->LVDS_CTRL, Hw1);	// reset
		BITCLR(pDDICfg->LVDS_CTRL, Hw1);	// normal
	
		pDDICfg->LVDS_TXO_SEL0 = 0x15141312; // SEL_03, SEL_02, SEL_01, SEL_00,
		pDDICfg->LVDS_TXO_SEL1 = 0x0B0A1716; // SEL_07, SEL_06, SEL_05, SEL_04,
		pDDICfg->LVDS_TXO_SEL2 = 0x0F0E0D0C; // SEL_11, SEL_10, SEL_09, SEL_08,
		pDDICfg->LVDS_TXO_SEL3 = 0x05040302; // SEL_15, SEL_14, SEL_13, SEL_12,
		pDDICfg->LVDS_TXO_SEL4 = 0x1A190706; // SEL_19, SEL_18, SEL_17, SEL_16,
		pDDICfg->LVDS_TXO_SEL5 = 0x1F1E1F18; // 						SEL_20,
		pDDICfg->LVDS_TXO_SEL6 = 0x1F1E1F1E;
		pDDICfg->LVDS_TXO_SEL7 = 0x1F1E1F1E;
		pDDICfg->LVDS_TXO_SEL8 = 0x1F1E1F1E;
		
		// LVDS Select
	//	BITCLR(pDDICfg->LVDS_CTRL, Hw0); //LCDC0
		BITSET(pDDICfg->LVDS_CTRL, Hw0); //LCDC1

		
#ifdef CONFIG_ARCH_TCC88XX
		M = 10; P = 10; S = 0; VSEL = 0;
		BITCSET(pDDICfg->LVDS_CTRL, Hw21- Hw4, (VSEL<<4)|(S<<5)|(M<<8)|(P<<15)); //LCDC1
	   	BITSET(pDDICfg->LVDS_CTRL, Hw1);    // reset
#endif//
		
		// LVDS enable
		BITSET(pDDICfg->LVDS_CTRL, Hw2);	// enable
	}
	
	else 
	{
		#ifdef CONFIG_ARCH_TCC88XX	
		BITCLR(pDDICfg->LVDS_CTRL, Hw1);	// reset
		#endif//
		
		BITCLR(pDDICfg->LVDS_CTRL, Hw2);	// disable
		
		clk_disable(lvds_clk);	

		gpio_set_value(pdata->reset, 0);		
		gpio_set_value(pdata->power_on, 0);
		//gpio_set_value( (GPIO_PORTF|13) , 0);	// LVDS_Module_PWR	
	}
	
	mutex_unlock(&panel_lock);

	if(on)
		panel->set_backlight_level(panel , panel->bl_level);
	
	return 0;
}

static int claa102na0dcw_set_backlight_level(struct lcd_panel *panel, int level)
{

	#define MAX_BL_LEVEL	255	
	volatile PTIMER pTIMER;
	
	struct lcd_platform_data *pdata = panel->dev->platform_data;
	panel->bl_level = level;

	mutex_lock(&panel_lock);


#if 1	
	if (level == 0) {
		tcc_gpio_config(pdata->bl_on, GPIO_FN(0)); 	
		gpio_set_value(pdata->bl_on, 0);
	} else {
		tcc_gpio_config(pdata->display_on, GPIO_FN(2));	
		
		pTIMER	= (volatile PTIMER)tcc_p2v(HwTMR_BASE);
		pTIMER->TREF0   = MAX_BL_LEVEL;	
		pTIMER->TCFG0   = 0x105;
		pTIMER->TMREF0 = (level | 0x07);	// 23.5 KHz 
		pTIMER->TCFG0	= 0x105;

		gpio_set_value(pdata->bl_on, 1);
	}
#endif//

	mutex_unlock(&panel_lock);
	return 0;
}
static struct lcd_panel claa102na0dcw_panel = {
	.name		= "CLAA102NA0DCW",
	.manufacturer	= "CHI_MEI",
	.id			= PANEL_ID_CLAA102NA0DCW,			// CLAA102NA0DCW is announced in the tcclcd.h (in /tcc88xx)	
	.xres		= 1024,		
	.yres		= 600,	
	.width		= 222,		
	.height		= 130,		
	.bpp			= 32,	
	.clk_freq		= 450000, 
	.clk_div		= 2,	
	.bus_width	= 24,		
	.lpw			= 0,
	.lpc			= 1024,			
	.lswc		= 0,		
	.lewc		= 176,	
	.vdb			= 0,
	.vdf			= 0,	
	.fpw1		= 0,
	.flc1			= 600,	
	.fswc1		= 0,
	.fewc1		= 25,	
	.fpw2		= 0,
	.flc2			= 600,
	.fswc2		= 0,
	.fewc2		= 25,	
	
	.init		= claa102na0dcw_panel_init,
	.set_power	= claa102na0dcw_set_power,
	.set_backlight_level = claa102na0dcw_set_backlight_level,
};


static int claa102na0dcw_probe(struct platform_device *pdev)
{
	struct lcd_platform_data *pdata = pdev->dev.platform_data;
	printk("%s : %s\n", __func__,  pdev->name);

	
	mutex_init(&panel_lock);
	lcd_pwr_state = 1;

	gpio_request(pdata->power_on, "lvds_on");
	gpio_request(pdata->bl_on, "lvds_bl");
	gpio_request(pdata->display_on, "lvds_display");
	gpio_request(pdata->reset, "lvds_reset");

	gpio_direction_output(pdata->power_on, 1);
	gpio_direction_output(pdata->bl_on, 1);
	gpio_direction_output(pdata->display_on, 1);
	gpio_direction_output(pdata->reset, 1);

	claa102na0dcw_panel.dev = &pdev->dev;

	lvds_clk = clk_get(0, "lvds");
	BUG_ON(lvds_clk == NULL);
	clk_enable(lvds_clk);	
	
	tccfb_register_panel(&claa102na0dcw_panel);
	return 0;
}

static int claa102na0dcw_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver claa102na0dcw_lcd = {
	.probe	= claa102na0dcw_probe,
	.remove	= claa102na0dcw_remove,
	.driver	= {
		.name	= "claa102na0dcw_lcd",
		.owner	= THIS_MODULE,
	},
};

static __init int claa102na0dcw_init(void)
{
	printk("~ %s ~ \n", __func__);
	return platform_driver_register(&claa102na0dcw_lcd);
}

static __exit void claa102na0dcw_exit(void)
{
	return platform_driver_unregister(&claa102na0dcw_lcd);
}

subsys_initcall(claa102na0dcw_init);
module_exit(claa102na0dcw_exit);

MODULE_DESCRIPTION("CLAA102NA0DCW LCD driver");
MODULE_LICENSE("GPL");

