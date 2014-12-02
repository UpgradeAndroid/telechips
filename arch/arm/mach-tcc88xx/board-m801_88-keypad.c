/* arch/arm/mach-tcc92xx/board-m801_88-keypad.c
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

#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/input/tcc_keypad.h>
#include <linux/gpio_event.h>
#include <linux/gpio_keys.h>
#include <linux/io.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <mach/bsp.h>
#include "board-m801_88.h"

#if defined(CONFIG_KEYPAD_TCC_ADC)
static struct resource tcc8800_keypad_resources[] = {
	[0] = {
		.start	= 0xF0102000,
		.end	= 0xF0102030,
		.flags	= IORESOURCE_MEM,
	},
};

struct tcc_key_info m801_88_adc_keys[] = {
	// mid7022{v1,v2,v3,v4}
	{ KEY_BACK,       1027, 1221 },
	{ KEY_HOME,        265,  458 },
	{ KEY_MENU,        525,  723 },
#if 0
	// mid8127
	{ KEY_HOME,       1536, 1876 },
	{ KEY_VOLUMEUP,    265,  458 },
	{ KEY_VOLUMEDOWN,  525,  723 },
	// mid8128
	{ KEY_BACK,       1027, 1221 },
	{ KEY_VOLUMEUP,    265,  458 },
	{ KEY_VOLUMEDOWN,  525,  723 },
	// mid1125{,wan}
	{ KEY_HOME,       1712, 1800 },
	{ KEY_VOLUMEUP,    265,  458 },
	{ KEY_VOLUMEDOWN,  525,  723 },
	// mid1126
	{ KEY_HOME,       1712, 1999 },
	{ KEY_VOLUMEUP,    265,  458 },
	{ KEY_VOLUMEDOWN,  525,  723 },
#endif
};

struct tcc_keypad_platform_data m801_88_keypad_platform_data = {
	.adc_channel = 10,

	.num_keys = ARRAY_SIZE(m801_88_adc_keys),
	.keys = m801_88_adc_keys,
};

static struct platform_device tcc8800_keypad_device = {
	.name			= "tcc-keypad",
	.id 			= -1,
	.resource		= tcc8800_keypad_resources,
	.num_resources		= ARRAY_SIZE(tcc8800_keypad_resources),
	.dev = {
		.platform_data = &m801_88_keypad_platform_data,
	},
};
#endif

#ifdef CONFIG_KEYBOARD_GPIO
/* gpio_keys */
static struct gpio_keys_button m801_88_buttons[] = {
	[0] = {
		.code       = KEY_POWER,
		.gpio       = TCC_GPA(3),
		.active_low = 1,
		.desc       = "powerkey",
		.type       = EV_KEY,
//		.wakeup     = 1,
	},
};

static struct gpio_keys_platform_data m801_88_gpio_keys_platform_data = {
	.buttons  = m801_88_buttons,
	.nbuttons = ARRAY_SIZE(m801_88_buttons),
};

static struct platform_device m801_88_gpio_keys = {
	.name = "gpio-keys",
	.id   = -1,
	.dev  = {
		.platform_data = &m801_88_gpio_keys_platform_data,
	},
};
#endif

static int __init m801_88_keypad_init(void)
{
	if (!machine_is_m801_88())
		return 0;

	platform_device_register(&tcc8800_keypad_device);

#ifdef CONFIG_KEYBOARD_GPIO
	platform_device_register(&m801_88_gpio_keys);
#endif

	return 0;
}

device_initcall(m801_88_keypad_init);
