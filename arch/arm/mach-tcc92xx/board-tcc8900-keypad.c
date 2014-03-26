/* arch/arm/mach-tcc92xx/board-tcc8900-keypad.c
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
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include "board-tcc8900.h"

static const struct gpio_event_direct_entry tcc8900_gpio_keymap[] = {
	{ GPIO_PWR_KEY,	KEY_END },
};

static struct gpio_event_input_info tcc8900_gpio_key_input_info = {
	.info.func = gpio_event_input_func,
	.keymap = tcc8900_gpio_keymap,
	.keymap_size = ARRAY_SIZE(tcc8900_gpio_keymap),
	.poll_time.tv64 = 20 * NSEC_PER_MSEC,
	.debounce_time.tv64 = 20 * NSEC_PER_MSEC,
	.flags = 0 /*GPIOEDF_PRINT_KEYS*/,
	.type = EV_KEY,
};

static struct gpio_event_info *tcc8900_gpio_key_info[] = {
	&tcc8900_gpio_key_input_info.info,
};

static struct gpio_event_platform_data tcc8900_gpio_key_data = {
	.name = "tcc-gpiokey",
	.info = tcc8900_gpio_key_info,
	.info_count = ARRAY_SIZE(tcc8900_gpio_key_info),
};

static struct platform_device tcc8900_gpio_key_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev = {
		.platform_data = &tcc8900_gpio_key_data,
	},
};

#if defined(CONFIG_KEYPAD_TCC_ADC)
static struct resource tcc8900_keypad_resources[] = {
	[0] = {
		.start	= 0xF0102000,		
		.end	= 0xF0102030,	
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device tcc8900_keypad_device = {
	.name			= "tcc-keypad",
	.id 			= -1,
	.resource		= tcc8900_keypad_resources,
	.num_resources		= ARRAY_SIZE(tcc8900_keypad_resources),
};
#endif

static int __init tcc8900_keypad_init(void)
{
	if (!machine_is_tcc8900())
		return 0;

	platform_device_register(&tcc8900_gpio_key_device);

#ifdef CONFIG_KEYPAD_TCC_ADC
	platform_device_register(&tcc8900_keypad_device);
#endif

	return 0;
}

device_initcall(tcc8900_keypad_init);
