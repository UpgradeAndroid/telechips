/* linux/arch/arm/mach-tcc93xx/board-tcc9300-panel.c
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <mach/gpio.h>
#include <mach/tcc_fb.h>
#include "devices.h"
#include "board-tcc9300.h"

#define DEFAULT_BACKLIGHT_BRIGHTNESS	102

static struct lcd_platform_data lcd_pdata = {
	.power_on	= GPIO_LCD_ON,
	.display_on	= GPIO_LCD_DISPLAY,
	.bl_on		= GPIO_LCD_BL,
	.reset		= GPIO_LCD_RESET,
};

#ifdef CONFIG_LCD_LMS350DF01
static struct platform_device lms350df01_lcd = {
	.name	= "lms350df01_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_LMS480KF01
static struct platform_device lms480kf01_lcd = {
	.name	= "lms480kf01_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_DX08D11VM0AAA
static struct platform_device dx08d11vm0aaa_lcd = {
	.name	= "dx08d11vm0aaa_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_LB070WV6
static struct platform_device lb070wv6_lcd = {
	.name	= "lb070wv6_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_CLAA104XA01CW
static struct platform_device claa104xa01cw_lcd = {
	.name	= "claa104xa01cw_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_HT121WX2
static struct platform_device ht121wx2_lcd = {
	.name	= "ht121wx2_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_TD043MGEB1
static struct platform_device td043mgeb1_lcd = {
	.name	= "td043mgeb1_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_AT070TN93
static struct platform_device at070tn93_lcd = {
	.name	= "at070tn93_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

static void tcc9300_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct lcd_panel *lcd_panel = tccfb_get_panel();

	if (lcd_panel->set_backlight_level)
		lcd_panel->set_backlight_level(lcd_panel, value);
}

static struct led_classdev tcc9300_backlight_led = {
	.name		= "lcd-backlight",
	.brightness	= DEFAULT_BACKLIGHT_BRIGHTNESS,
	.brightness_set	= tcc9300_brightness_set,
};

static int tcc9300_backlight_probe(struct platform_device *pdev)
{

	gpio_request(GPIO_LCD_ON, "lcd_on");
	gpio_request(GPIO_LCD_BL, "lcd_bl");
	gpio_request(GPIO_LCD_DISPLAY, "lcd_display");
	gpio_request(GPIO_LCD_RESET, "lcd_reset");

	gpio_direction_output(GPIO_LCD_ON, 1);
	gpio_direction_output(GPIO_LCD_BL, 1);
	gpio_direction_output(GPIO_LCD_DISPLAY, 1);
	gpio_direction_output(GPIO_LCD_RESET, 1);
	
	return led_classdev_register(&pdev->dev, &tcc9300_backlight_led);
}

static int tcc9300_backlight_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&tcc9300_backlight_led);
	return 0;
}

static struct platform_driver tcc9300_backlight_driver = {
	.probe	= tcc9300_backlight_probe,
	.remove	= tcc9300_backlight_remove,
	.driver	= {
		.name	= "tcc9300-backlight",
		.owner	= THIS_MODULE,
	},
};

static struct platform_device tcc9300_backlight = {
	.name = "tcc9300-backlight",
};

int __init tcc9300_init_panel(void)
{
	int ret;

	if (!machine_is_tcc9300())
		return 0;

	switch (tcc_panel_id) {
#ifdef CONFIG_LCD_LMS350DF01
	case PANEL_ID_LMS350DF01:
		platform_device_register(&lms350df01_lcd);
		break;
#endif

#ifdef CONFIG_LCD_LMS480KF01
	case PANEL_ID_LMS480KF01:
		platform_device_register(&lms480kf01_lcd);
		break;
#endif

#ifdef CONFIG_LCD_DX08D11VM0AAA
	case PANEL_ID_DX08D11VM0AAA:
		platform_device_register(&dx08d11vm0aaa_lcd);
		break;
#endif

#ifdef CONFIG_LCD_LB070WV6
	case PANEL_ID_LB070WV6:
		platform_device_register(&lb070wv6_lcd);
		break;
#endif

#ifdef CONFIG_LCD_CLAA104XA01CW
	case PANEL_ID_CLAA104XA01CW:
		platform_device_register(&claa104xa01cw_lcd);
		break;
#endif

#ifdef CONFIG_LCD_HT121WX2
	case PANEL_ID_HT121WX2:
		platform_device_register(&ht121wx2_lcd);
		break;
#endif

#ifdef CONFIG_LCD_TD043MGEB1
	case PANEL_ID_TD043MGEB1:
		platform_device_register(&td043mgeb1_lcd);
		break;
#endif

#ifdef CONFIG_LCD_AT070TN93
	case PANEL_ID_AT070TN93:
		platform_device_register(&at070tn93_lcd);
		break;
#endif

	default:
		pr_err("Not supported LCD panel type %d\n", tcc_panel_id);
		return -EINVAL;
	}

	ret = platform_device_register(&tcc_lcd_device);
	if (ret)
		return ret;

	platform_device_register(&tcc9300_backlight);
	ret = platform_driver_register(&tcc9300_backlight_driver);
	if (ret)
		return ret;

	return 0;
}
device_initcall(tcc9300_init_panel);
