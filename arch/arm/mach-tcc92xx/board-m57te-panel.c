/* arch/arm/mach-tcc92xx/board-m57te-panel.c
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
#include "board-m57te.h"

#define DEFAULT_BACKLIGHT_BRIGHTNESS	102

static struct lcd_platform_data lcd_pdata = {
	.power_on	= GPIO_LCD_ON,
	.display_on	= GPIO_LCD_DISPLAY,
	.bl_on		= GPIO_LCD_BL,
	.reset		= GPIO_LCD_RESET,
};

static struct platform_device td043mgeb1_lcd = {
	.name	= "td043mgeb1_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};


static void m57te_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct lcd_panel *lcd_panel = tccfb_get_panel();

	if (lcd_panel->set_backlight_level)
		lcd_panel->set_backlight_level(lcd_panel, value);
}

static struct led_classdev m57te_backlight_led = {
	.name		= "lcd-backlight",
	.brightness	= DEFAULT_BACKLIGHT_BRIGHTNESS,
	.brightness_set	= m57te_brightness_set,
};

static int m57te_backlight_probe(struct platform_device *pdev)
{
	tcc_gpio_config(GPIO_LCD_ON, GPIO_FN(0));
	tcc_gpio_config(GPIO_LCD_BL, GPIO_FN(0));
	tcc_gpio_config(GPIO_LCD_DISPLAY, GPIO_FN(0));
	tcc_gpio_config(GPIO_LCD_RESET, GPIO_FN(0));

	gpio_request(GPIO_LCD_ON, "lcd_on");
	gpio_request(GPIO_LCD_BL, "lcd_bl");
	gpio_request(GPIO_LCD_DISPLAY, "lcd_display");
	gpio_request(GPIO_LCD_RESET, "lcd_reset");

	return led_classdev_register(&pdev->dev, &m57te_backlight_led);
}

static int m57te_backlight_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&m57te_backlight_led);
	return 0;
}

static struct platform_driver m57te_backlight_driver = {
	.probe	= m57te_backlight_probe,
	.remove	= m57te_backlight_remove,
	.driver	= {
		.name	= "m57te-backlight",
		.owner	= THIS_MODULE,
	},
};

static struct platform_device m57te_backlight = {
	.name = "m57te-backlight",
};

int __init m57te_init_panel(void)
{
	int ret;

	if (!machine_is_m57te())
		return 0;

	printk("m57te LCD panel type %d\n", tcc_panel_id);

	platform_device_register(&td043mgeb1_lcd);

	ret = platform_device_register(&tcc_lcd_device);
	if (ret)
		return ret;

	platform_device_register(&m57te_backlight);
	ret = platform_driver_register(&m57te_backlight_driver);
	if (ret)
		return ret;

	return 0;
}
device_initcall(m57te_init_panel);
