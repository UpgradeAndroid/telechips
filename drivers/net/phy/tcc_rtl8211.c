/*
 * Based on : linux/drivers/net/phy/realtek.c
 *
 * Driver for Realtek PHYs
 *
 * Author: Telechips <linux@telechips.com>
 * Created : June 22, 2010
 *
 * Copyright (c) 2010 Telechips, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/phy.h>

MODULE_DESCRIPTION("Realtek PHY driver for TCC9300");
MODULE_AUTHOR("Telechips");
MODULE_LICENSE("GPL");

/* RTL8211CL */
static struct phy_driver rtl821x_driver = {
	.phy_id		= 0x001cc912,
	.name		= "RTL8211 Gigabit Ethernet",
	.phy_id_mask	= 0x001fffff,
	.features	= PHY_GBIT_FEATURES,
	.flags		= PHY_POLL,
	.config_aneg	= &genphy_config_aneg,
	.read_status	= &genphy_read_status,
	.driver		= { .owner = THIS_MODULE,},
};

static int __init realtek_init(void)
{
	int ret;

	ret = phy_driver_register(&rtl821x_driver);

	return ret;
}

static void __exit realtek_exit(void)
{
	phy_driver_unregister(&rtl821x_driver);
}

module_init(realtek_init);
module_exit(realtek_exit);
