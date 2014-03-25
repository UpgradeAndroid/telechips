/* arch/arm/mach-tcc93xx/board-tcc9300-keypad.c
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

#if defined(CONFIG_MACH_TCC9300) || defined(CONFIG_MACH_TCC9300ST)
static unsigned int tcc9300_col_gpios[] = { TCC_GPA(8), TCC_GPG(22) };
static unsigned int tcc9300_row_gpios[] = {TCC_GPA(7), TCC_GPE(3), TCC_GPE(29) };

#define KEYMAP_INDEX(col, row) ((col)*ARRAY_SIZE(tcc9300_row_gpios) + (row))

static const unsigned short tcc9300_keymap[] = {
#if 1
	[KEYMAP_INDEX(0, 0)] = KEY_VOLUMEUP,
    [KEYMAP_INDEX(0, 1)] = KEY_MENU,
	[KEYMAP_INDEX(0, 2)] = KEY_RESERVED,

	[KEYMAP_INDEX(1, 0)] = KEY_VOLUMEDOWN,
	[KEYMAP_INDEX(1, 1)] = KEY_BACK,
	[KEYMAP_INDEX(1, 2)] = KEY_END,
#else
	[KEYMAP_INDEX(0, 0)] = KEY_MENU,	// hold
	[KEYMAP_INDEX(0, 1)] = KEY_UP,		// V+
    [KEYMAP_INDEX(0, 2)] = KEY_ENTER,	// send

	[KEYMAP_INDEX(1, 0)] = KEY_END,		// power
	[KEYMAP_INDEX(1, 1)] = KEY_DOWN,	// V-
	[KEYMAP_INDEX(1, 2)] = KEY_BACK,	// Cam
#endif
};

static struct gpio_event_matrix_info tcc9300_keypad_matrix_info = {
	.info.func = gpio_event_matrix_func,
	.keymap = tcc9300_keymap,
	.output_gpios = tcc9300_col_gpios,
	.input_gpios = tcc9300_row_gpios,
	.noutputs = ARRAY_SIZE(tcc9300_col_gpios),
	.ninputs = ARRAY_SIZE(tcc9300_row_gpios),
	.settle_time.tv.nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.debounce_delay.tv.nsec = 50 * NSEC_PER_MSEC,
	.flags = GPIOKPF_DRIVE_INACTIVE | GPIOKPF_ACTIVE_HIGH /*| GPIOKPF_PRINT_UNMAPPED_KEYS | GPIOKPF_PRINT_MAPPED_KEYS*/,
};

static struct gpio_event_info *tcc9300_keypad_info[] = {
	&tcc9300_keypad_matrix_info.info,
};
#elif defined(CONFIG_MACH_TCC9300CM)
static const struct gpio_event_direct_entry tcc9300_keymap[] = {
	{ GPIO_PORTA|3,	KEY_END },
};

static struct gpio_event_input_info tcc9300_keypad_input_info = {
	.info.func = gpio_event_input_func,
	.keymap = tcc9300_keymap,
	.keymap_size = ARRAY_SIZE(tcc9300_keymap),
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.debounce_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.flags = 0 /*GPIOEDF_PRINT_KEYS*/,
	.type = EV_KEY,
};

static struct gpio_event_info *tcc9300_keypad_info[] = {
	&tcc9300_keypad_input_info.info,
};
#endif

static struct gpio_event_platform_data tcc9300_keypad_data = {
	.name = "tcc-gpiokey",
	.info = tcc9300_keypad_info,
	.info_count = ARRAY_SIZE(tcc9300_keypad_info),
};

static struct platform_device tcc9300_keypad_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev = {
		.platform_data = &tcc9300_keypad_data,
	},
};

static int __init tcc9300_keypad_init(void)
{
#ifndef CONFIG_MACH_TCC9300CM
	if (!machine_is_tcc9300())
		return 0;
#endif

	return platform_device_register(&tcc9300_keypad_device);
}

device_initcall(tcc9300_keypad_init);
