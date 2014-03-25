/* linux/arch/arm/mach-tcc92xx/board-tcc9200s.c
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
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/usb/android_composite.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#include <linux/synaptics_i2c_rmi.h>

#include <mach/gpio.h>
#include <mach/bsp.h>
#include <mach/common.h>
#include <mach/irqs.h>

#include <plat/nand.h>

#include "devices.h"
#include "board-tcc9200s.h"

extern void __init tcc9200_irq_init(void);
extern void __init tcc9200_map_common_io(void);

static void __init tcc9200_init_irq(void)
{
    tcc9200_irq_init();
//    tcc9200_gpio_init();
}

static void tcc9200s_nand_init(void)
{
	unsigned int gpio_wp = GPIO_NAND_WP;
	unsigned int gpio_rdy = GPIO_NAND_RDY;

	tcc_gpio_config(gpio_wp, GPIO_FN(0));
	tcc_gpio_config(gpio_rdy, GPIO_FN(0));

	gpio_request(gpio_wp, "nand_wp");
	gpio_direction_output(gpio_wp, 1);

	gpio_request(gpio_rdy, "nand_rdy");
	gpio_direction_input(gpio_rdy);
}

static int tcc9200s_nand_ready(void)
{
	return !gpio_get_value(GPIO_NAND_RDY);
}

static struct tcc_nand_platform_data tcc_nand_platdata = {
	.parts		= NULL,
	.nr_parts	= 0,
	.gpio_wp	= GPIO_NAND_WP,
	.init		= tcc9200s_nand_init,
	.ready		= tcc9200s_nand_ready,
};

static struct platform_device tcc_nand_device = {
	.name		= "tcc_mtd",
	.id			= -1,
	.dev		= {
		.platform_data = &tcc_nand_platdata,
	},
};

static void tcc_add_nand_device(void)
{
	tcc_get_partition_info(&tcc_nand_platdata.parts, &tcc_nand_platdata.nr_parts);
	platform_device_register(&tcc_nand_device);
}



#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_I2C_RMI
static int synaptics_ts_power(int on)
{
	return 0;
}

static struct synaptics_i2c_rmi_platform_data synaptics_ts_data[] = {
	{
                .version = 0x0101,
                .power = synaptics_ts_power,
                .flags = SYNAPTICS_FLIP_Y | SYNAPTICS_SNAP_TO_INACTIVE_EDGE,
                .inactive_left = -50 * 0x10000 / 4334,
                .inactive_right = -50 * 0x10000 / 4334,
                .inactive_top = -40 * 0x10000 / 6696,
                .inactive_bottom = -40 * 0x10000 / 6696,
                .snap_left_on = 50 * 0x10000 / 4334,
                .snap_left_off = 60 * 0x10000 / 4334,
                .snap_right_on = 50 * 0x10000 / 4334,
                .snap_right_off = 60 * 0x10000 / 4334,
                .snap_top_on = 100 * 0x10000 / 6696,
                .snap_top_off = 110 * 0x10000 / 6696,
                .snap_bottom_on = 100 * 0x10000 / 6696,
                .snap_bottom_off = 110 * 0x10000 / 6696,
        },
        {
                .flags = SYNAPTICS_FLIP_Y | SYNAPTICS_SNAP_TO_INACTIVE_EDGE,
                .inactive_left = ((4674 - 4334) / 2 + 200) * 0x10000 / 4334,
                .inactive_right = ((4674 - 4334) / 2 + 200) * 0x10000 / 4334,
                .inactive_top = ((6946 - 6696) / 2) * 0x10000 / 6696,
                .inactive_bottom = ((6946 - 6696) / 2) * 0x10000 / 6696,
        }
};
#endif

#ifdef CONFIG_I2C_BOARDINFO
static struct i2c_board_info __initdata i2c_devices0[] = {
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_I2C_RMI
	{
		I2C_BOARD_INFO(SYNAPTICS_I2C_RMI_NAME, 0x20),
		.platform_data = synaptics_ts_data,
	},
#endif
#ifdef CONFIG_TOUCHSCREEN_TCC_SYNAPTICS
	{
		I2C_BOARD_INFO("tcc-synaptics-ts", 0x20),
		.irq = INT_EI11,
	},
#endif
};
#endif

/*----------------------------------------------------------------------
 * Device     : USB Android Gadget 
 * Description: 
 *----------------------------------------------------------------------*/
static struct usb_mass_storage_platform_data mass_storage_pdata = {
#ifdef CONFIG_SCSI
	.nluns = 4, // for iNand
#else
	.nluns = 3,
#endif
	.vendor = "Telechips, Inc.",
	.product = "TCC9200s",
	.release = 0x0100,
};

static struct platform_device usb_mass_storage_device = {
	.name = "usb_mass_storage",
	.id = -1,
	.dev = {
		.platform_data = &mass_storage_pdata,
	},
};

#ifdef CONFIG_USB_ANDROID_RNDIS
static struct usb_ether_platform_data rndis_pdata = {
	/* ethaddr is filled by board_serialno_setup */
	.vendorID	= 0x18d1,
	.vendorDescr	= "Telechips, Inc.",
};

static struct platform_device rndis_device = {
	.name	= "rndis",
	.id	= -1,
	.dev	= {
		.platform_data = &rndis_pdata,
	},
};
#endif

static char *usb_functions_ums[] = {
	"usb_mass_storage",
};

static char *usb_functions_ums_adb[] = {
	"usb_mass_storage",
	"adb",
};

static char *usb_functions_rndis[] = {
	"rndis",
};

static char *usb_functions_rndis_adb[] = {
	"rndis",
	"adb",
};


static char *usb_functions_all[] = {
#ifdef CONFIG_USB_ANDROID_RNDIS
	"rndis",
#endif
	"usb_mass_storage",
	"adb",
#ifdef CONFIG_USB_ANDROID_ACM
	"acm",
#endif
};

static struct android_usb_product usb_products[] = {
	{
		.product_id	= 0xb058, /* Telechips UMS PID */
		.num_functions	= ARRAY_SIZE(usb_functions_ums),
		.functions	= usb_functions_ums,
	},
	{
		.product_id	= 0xdeed,
		.num_functions	= ARRAY_SIZE(usb_functions_ums_adb),
		.functions	= usb_functions_ums_adb,
	},
	{
		.product_id	= 0x0002,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis),
		.functions	= usb_functions_rndis,
	},
	{
		.product_id	= 0x0003,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis_adb),
		.functions	= usb_functions_rndis_adb,
	},
};

static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id      = 0x18D1,
	.product_id     = 0x0001,
	.version	= 0x0100,
	.product_name	= "TCC9200s",
	.manufacturer_name = "Telechips, Inc.",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id		= -1,
	.dev		= {
		.platform_data = &android_usb_pdata,
	},
};

#if defined(CONFIG_TCC_WATCHDOG)
static struct platform_device tccwdt_device = {
	.name	= "tcc-wdt",
	.id		= -1,
};
#endif

static struct platform_device *tcc9200s_devices[] __initdata = {
#if defined(CONFIG_I2C_TCC92XX)
	&tcc9200_i2c_device,
#endif
	&tcc_otg_device,
#ifdef CONFIG_USB_ANDROID_RNDIS
	&rndis_device,
#endif
	&usb_mass_storage_device,
	&android_usb_device,
#if defined(CONFIG_TCC_WATCHDOG)
	&tccwdt_device,
#endif
};

static int __init board_serialno_setup(char *serialno)
{
	android_usb_pdata.serial_number = serialno;
	return 1;
}
__setup("androidboot.serialno=", board_serialno_setup);

static void __init tcc9200_init_machine(void)
{
#ifdef CONFIG_I2C_BOARDINFO
	i2c_register_board_info(0, i2c_devices0, ARRAY_SIZE(i2c_devices0));
#endif
	tcc_add_nand_device();

	platform_add_devices(tcc9200s_devices, ARRAY_SIZE(tcc9200s_devices));
}

static void __init tcc9200_map_io(void)
{
	tcc9200_map_common_io();
}

MACHINE_START(TCC9200S, "tcc9200s")
    /* Maintainer: Telechips Linux BSP Team <linux@telechips.com> */
    .phys_io        = 0xf0000000,
    .io_pg_offst    = ((0xf0000000) >> 18) & 0xfffc,
    .boot_params    = PHYS_OFFSET + 0x00000100,
    .map_io         = tcc9200_map_io,
    .init_irq       = tcc9200_init_irq,
    .init_machine   = tcc9200_init_machine,
    .timer          = &tcc9200_timer,
MACHINE_END
