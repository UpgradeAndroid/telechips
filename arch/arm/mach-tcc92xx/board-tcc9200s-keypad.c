/* arch/arm/mach-tcc92xx/board-tcc9200s-keypad.c
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

static unsigned int tcc9200s_col_gpios[] = { GPIO_PORTA|5, GPIO_PORTA|6, GPIO_PORTA|7 };
static unsigned int tcc9200s_row_gpios[] = { GPIO_PORTA|2, GPIO_PORTA|3, GPIO_PORTA|12 };

#define KEYMAP_INDEX(col, row) ((col)*ARRAY_SIZE(tcc9200s_row_gpios) + (row))

static const unsigned short tcc9200s_keymap[] = {
	[KEYMAP_INDEX(0, 0)] = KEY_MENU,
	[KEYMAP_INDEX(0, 1)] = KEY_HOME,
	[KEYMAP_INDEX(0, 2)] = KEY_ENTER,

	[KEYMAP_INDEX(1, 0)] = KEY_LEFT,
	[KEYMAP_INDEX(1, 1)] = KEY_RIGHT,
	[KEYMAP_INDEX(1, 2)] = KEY_BACK,

	[KEYMAP_INDEX(2, 0)] = KEY_CAMERA,
	[KEYMAP_INDEX(2, 1)] = KEY_UP,
	[KEYMAP_INDEX(2, 2)] = KEY_DOWN,
};

static struct gpio_event_matrix_info tcc9200s_keypad_matrix_info = {
	.info.func = gpio_event_matrix_func,
	.keymap = tcc9200s_keymap,
	.output_gpios = tcc9200s_col_gpios,
	.input_gpios = tcc9200s_row_gpios,
	.noutputs = ARRAY_SIZE(tcc9200s_col_gpios),
	.ninputs = ARRAY_SIZE(tcc9200s_row_gpios),
	.settle_time.tv.nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.debounce_delay.tv.nsec = 50 * NSEC_PER_MSEC,
#if 1 // XXX: disable interrupt for now
	.flags = GPIOKPF_DRIVE_INACTIVE /*| GPIOKPF_PRINT_UNMAPPED_KEYS | GPIOKPF_PRINT_MAPPED_KEYS*/,
#else
	.flags = GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_PRINT_UNMAPPED_KEYS,
#endif
};

static struct gpio_event_info *tcc9200s_keypad_info[] = {
	&tcc9200s_keypad_matrix_info.info,
};

static struct gpio_event_platform_data tcc9200s_keypad_data = {
	.name = "tcc-gpiokey",
	.info = tcc9200s_keypad_info,
	.info_count = ARRAY_SIZE(tcc9200s_keypad_info),
};

static struct platform_device tcc9200s_keypad_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev = {
		.platform_data = &tcc9200s_keypad_data,
	},
};

static int __init tcc9200s_keypad_init(void)
{
	if (!machine_is_tcc9200s())
		return 0;
	return platform_device_register(&tcc9200s_keypad_device);
}

device_initcall(tcc9200s_keypad_init);
