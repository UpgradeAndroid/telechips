/* arch/arm/mach-tcc92xx/board-tcc9200s-panel.c
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

#include <asm/mach-types.h>

#if 0
tcc9200s_panels[] = {
#ifdef CONFIG_LCD_DX08D11VM0AAA
	{
		.name = "dx08d11vm0aaa",
		.xres = 480,
		.yres = 800,
	},
#endif
};

#ifdef CONFIG_BACKLIGHT_DX08D11VM0AAA
[] = {
	{
		.name = "dx08d11vm0aaa",
	},
};
#endif
#endif

int __init tcc9200s_init_panel(void)
{
	if (!machine_is_tcc9200s())
		return 0;
	return 0;
}

device_initcall(tcc9200s_init_panel);
