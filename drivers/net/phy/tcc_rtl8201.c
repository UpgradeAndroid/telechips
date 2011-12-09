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

#define RTL8201_PHYSR		0x11
#define RTL8201_PHYSR_DUPLEX	0x2000
#define RTL8201_PHYSR_SPEED	0xc000
#define RTL8201_INER		0x12
#define RTL8201_INER_INIT	0x6400
#define RTL8201_INSR		0x13

MODULE_DESCRIPTION("Realtek PHY driver for TCC9300");
MODULE_AUTHOR("Telechips");
MODULE_LICENSE("GPL");

static int rtl8201_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL8201_INSR);

	return (err < 0) ? err : 0;
}

static int rtl8201_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL8201_INER,
				RTL8201_INER_INIT);
	else
		err = phy_write(phydev, RTL8201_INER, 0);

	return err;
}

/* RTL8211B */
static struct phy_driver rtl8201_driver = {
	.phy_id		= 0x00008201,
	.name		= "RTL8201 Ethernet",
	.phy_id_mask	= 0x001fffff,
	.features	= PHY_BASIC_FEATURES,
	.flags		= PHY_HAS_INTERRUPT,
	.config_aneg	= &genphy_config_aneg,
	.read_status	= &genphy_read_status,
	.ack_interrupt	= &rtl8201_ack_interrupt,
	.config_intr	= &rtl8201_config_intr,
	.driver		= { .owner = THIS_MODULE,},
};

static int __init realtek_init(void)
{
	int ret;

	ret = phy_driver_register(&rtl8201_driver);

	return ret;
}

static void __exit realtek_exit(void)
{
	phy_driver_unregister(&rtl8201_driver);
}

module_init(realtek_init);
module_exit(realtek_exit);
