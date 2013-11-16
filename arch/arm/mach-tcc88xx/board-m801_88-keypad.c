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
#include <linux/gpio_event.h>
#include <linux/gpio_keys.h>
#include <linux/io.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <mach/bsp.h>
#include "board-m801_88.h"

#if 0
static const struct gpio_event_direct_entry m801_88_gpio_keymap[] = {
#if !defined(CONFIG_REGULATOR_AXP192_PEK)
	{ GPIO_PWR_KEY,	KEY_END },
#endif
	{ TCC_GPF(2),	KEY_MENU },	// menu
	{ TCC_GPF(3),	KEY_BACK },	// back
	{ TCC_GPF(4), 	KEY_VOLUMEUP }, // home
	{ TCC_GPF(5), 	KEY_VOLUMEDOWN }, // home
	{ TCC_GPF(6), 	KEY_HOMEPAGE }, // home
};

static struct gpio_event_input_info m801_88_gpio_key_input_info = {
	.info.func = gpio_event_input_func,
	.keymap = m801_88_gpio_keymap,
	.keymap_size = ARRAY_SIZE(m801_88_gpio_keymap),
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.debounce_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.flags = 0 /*GPIOEDF_PRINT_KEYS*/,
	.type = EV_KEY,
};

static struct gpio_event_info *m801_88_gpio_key_info[] = {
	&m801_88_gpio_key_input_info.info,
};

static struct gpio_event_platform_data m801_88_gpio_key_data = {
	.name = "tcc-gpiokey",
	.info = m801_88_gpio_key_info,
	.info_count = ARRAY_SIZE(m801_88_gpio_key_info),
};

static struct platform_device m801_88_gpio_key_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev = {
		.platform_data = &m801_88_gpio_key_data,
	},
};
#elif defined(CONFIG_KEYPAD_TCC_ADC)
static struct resource tcc8800_keypad_resources[] = {
	[0] = {
		.start	= 0xF0102000,
		.end	= 0xF0102030,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device tcc8800_keypad_device = {
	.name			= "tcc-keypad",
	.id 			= -1,
	.resource		= tcc8800_keypad_resources,
	.num_resources		= ARRAY_SIZE(tcc8800_keypad_resources),
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

#if 0
	platform_device_register(&m801_88_gpio_key_device);
#else
	platform_device_register(&tcc8800_keypad_device);
#endif

#ifdef CONFIG_KEYBOARD_GPIO
	platform_device_register(&m801_88_gpio_keys);
#endif

	return 0;
}

device_initcall(m801_88_keypad_init);
