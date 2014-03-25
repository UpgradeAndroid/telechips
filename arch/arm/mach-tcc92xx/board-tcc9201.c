/* lnux/arch/arm/mach-tcc92xx/board-tcc9201.c
 *
 * Author:  <linux@telechips.com>
 * Created: 10th Feb, 2009
 * Description:
 *
 * Copyright (C) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>

#include <mach/gpio.h>
#include <mach/bsp.h>
#include <mach/common.h>
#include <mach/irqs.h>

#include "devices.h"
#include "board-tcc9200s.h"

extern void __init tcc9200_irq_init(void);
extern void __init tcc9200_map_common_io(void);

static void __init tcc9200_init_irq(void)
{
    tcc9200_irq_init();
//    tcc9200_gpio_init();
}

//#ifdef CONFIG_I2C_BOARDINFO
//static struct i2c_board_info __initdata tcc8900_i2c_board_info[] = {
//};
//#endif

static void __init tcc9200_init_machine(void)
{
//#ifdef CONFIG_I2C_BOARDINFO
//	i2c_register_board_info(-1, tcc8900_i2c_board_info, ARRAY_SIZE(tcc8900_i2c_board_info));
//#endif
}


static void __init tcc9200_map_io(void)
{
    tcc9200_map_common_io();
}

MACHINE_START(TCC9201, "tcc9201")
    /* Maintainer: Telechips Linux BSP Team <linux@telechips.com> */
    .phys_io        = 0xf0000000,
    .io_pg_offst    = ((0xf0000000) >> 18) & 0xfffc,
    .boot_params    = PHYS_OFFSET + 0x00000100,
    .map_io         = tcc9200_map_io,
    .init_irq       = tcc9200_init_irq,
    .init_machine   = tcc9200_init_machine,
    .timer          = &tcc9200_timer,
MACHINE_END
