/*
 * Device Tree support for Telechips TCC92XX SoCs.
 *
 * Copyright (C) 2014 Upgrade Android.
 *
 * Ithamar R. Adema <ithamar@upgrade-android.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/clocksource.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/irqchip.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/io.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/system_misc.h>

#include <linux/clk/tcc.h>

void __init tcc92xx_map_io(void)
{
#if defined(CONFIG_DEBUG_TCC_UART)
	extern void __init tcc_map_lluart(void);
	tcc_map_lluart();
#endif
}

static void __init tcc92xx_timer_init(void)
{
	tcc92xx_clocks_init();
	clocksource_of_init();
}

static void __init tcc92xx_dt_init(void)
{
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
}

static const char * const tcc92xx_board_dt_compat[] = {
	"tcc,tcc92xx",
	NULL,
};

DT_MACHINE_START(TCC92XX_DT, "TCC92XX (Device Tree)")
	.nr_irqs	= 64,
	.init_machine	= tcc92xx_dt_init,
	.map_io		= tcc92xx_map_io,
	.init_irq	= irqchip_init,
	.init_time	= tcc92xx_timer_init,
	.dt_compat	= tcc92xx_board_dt_compat,
MACHINE_END
