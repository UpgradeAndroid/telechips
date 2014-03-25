/* arch/arm/mach-tcc93xx/board-tcc9300st-keypad.c
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
#include "board-tcc9300st.h"

static const struct gpio_event_direct_entry tcc9300st_gpio_keymap[] = {
	{ GPIO_PWR_KEY,	KEY_END },
};

static struct gpio_event_input_info tcc9300st_gpio_key_input_info = {
	.info.func = gpio_event_input_func,
	.keymap = tcc9300st_gpio_keymap,
	.keymap_size = ARRAY_SIZE(tcc9300st_gpio_keymap),
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.debounce_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.flags = 0 /*GPIOEDF_PRINT_KEYS*/,
	.type = EV_KEY,
};

static struct gpio_event_info *tcc9300st_gpio_key_info[] = {
	&tcc9300st_gpio_key_input_info.info,
};

static struct gpio_event_platform_data tcc9300st_gpio_key_data = {
	.name = "tcc-gpiokey",
	.info = tcc9300st_gpio_key_info,
	.info_count = ARRAY_SIZE(tcc9300st_gpio_key_info),
};

static struct platform_device tcc9300st_gpio_key_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev = {
		.platform_data = &tcc9300st_gpio_key_data,
	},
};

static int __init tcc9300st_keypad_init(void)
{
	if (!machine_is_tcc9300st())
		return 0;

	platform_device_register(&tcc9300st_gpio_key_device);

	return 0;
}

device_initcall(tcc9300st_keypad_init);
