/*
 * linux/arch/arm/mach-tcc8900/include/mach/tcc_pca953x.h
 *
 * Author:  <linux@telechips.com>
 * Created: 21th March, 2009 
 * Description: Tcc250 Driver 
 *
 * Copyright (c) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * ===============================================================================================
 *  USAGE
 * =======
 *
 * Use "tcc_pca953x_setup()" function for management port status.
 *
 * 1. read operation
 *
 *   int tcc_pca953x_setup(int slave, int name, int direction, int value, int mode);
 *
 *   parameters
 *   ----------
 *     int slave     - PCA953x I2C slave address
 *     int name      - fixed 0
 *     int direction - direction of port [OUTPUT/INPUT]
 *     int value     - fixed 0
 *     int mode      - fixed GET_VALUE
 *
 *   return value
 *   ------------
 *     int rd_buf    - 16bit read data returned, -1 is fail
 *
 *   example
 *   -------
 *     // read PCA9539_U2_SLAVE_ADDR port0 and port1
 *     int rd_buf = tcc_pca953x_setup(PCA9539_U2_SLAVE_ADDR, 0, OUTPUT, 0, GET_VALUE);
 *
 * 2. write operation
 *
 *   int tcc_pca953x_setup(int slave, int name, int direction, int value, int mode);
 *
 *   parameters
 *   ----------
 *     int slave     - PCA953x I2C slave address
 *     int name      - control port name, see "Expanded GPIO port map"
 *     int direction - direction of port [OUTPUT/INPUT]
 *     int value     - output mode only. set port value [HIGH/LOW]
 *     int mode      - [SET_DIRECTION/SET_VALUE] or [SET_DIRECTION|SET_VALUE]
 *
 *   return value
 *   ------------
 *     int rd_buf    - 1 is success, -1 is fail
 *
 *   example
 *   -------
 *     + set port direction
 *       // PCA9539_U2_SLAVE_ADDR, ETH_RST, Output mode
 *       tcc_pca953x_setup(PCA9539_U2_SLAVE_ADDR, ETH_RST, OUTPUT, 0, SET_DIRECTION);
 *
 *     + set port value (output mode only)
 *       // PCA9539_U2_SLAVE_ADDR, ETH_RST, Output High
 *       tcc_pca953x_setup(PCA9539_U2_SLAVE_ADDR, ETH_RST, OUTPUT, HIGH, SET_VALUE);
 *
 *     + set both (direction & value)
 *       // PCA9539_U2_SLAVE_ADDR, ETH_RST, Output mode, Output Low
 *       tcc_pca953x_setup(PCA9539_U2_SLAVE_ADDR, ETH_RST, OUTPUT, LOW, SET_DIRECTION|SET_VALUE);
 *
 * 3. direct read/write
 *   + read
 *     tcc_pca953x_read(PCA9539_U2_SLAVE_ADDR, PCA9539_OUTPUT_0, &buf[0]);
 *     tcc_pca953x_read(PCA9539_U2_SLAVE_ADDR, PCA9539_OUTPUT_1, &buf[1]);
 *     printk("PCA9539_U2_SLAVE_ADDR port0(%x), port1(%x)", buf[0], buf[1]);
 *   
 *   + write each port
 *     buf[0] = PCA9539_OUTPUT_0;
 *     buf[1] = 0x12;
 *     tcc_pca953x_write(PCA9539_U2_SLAVE_ADDR, &buf[0], 2);
 *     buf[0] = PCA9539_OUTPUT_1;
 *     buf[1] = 0x34;
 *     tcc_pca953x_write(PCA9539_U2_SLAVE_ADDR, &buf[0], 2);
 *   
 *   + write all port
 *     buf[0] = PCA9539_OUTPUT_0;
 *     buf[1] = 0x12;
 *     buf[2] = 0x34;
 *     tcc_pca953x_write(PCA9539_U3_SLAVE_ADDR, &buf[0], 3);
 *
 * ===============================================================================================
 */
#ifndef __PCA953X_H__
#define __PCA953X_H__

#include <bsp.h>


/*
 * EXPORT_SYMBOL
 */
extern int tcc_pca953x_setup(int slave, int name, int direction, int value, int mode);
extern int tcc_pca953x_read(int slave, unsigned char cmd, unsigned char *rd_buf);
extern int tcc_pca953x_write(int slave, const unsigned char *wr_buf, int count);

/*
 * PCA953x I2C slave address
 */
#define PCA9539_U2_SLAVE_ADDR	0x77
#define PCA9539_U3_SLAVE_ADDR	0x74
#define PCA9538_U4_SLAVE_ADDR	0x70

/*
 * Port setup mode
 */
#define GET_VALUE		0x0001
#define SET_DIRECTION	0x0010
#define SET_VALUE		0x0100

/*
 * Port direction & value
 */
#define INPUT	1
#define OUTPUT	0
#define HIGH	1
#define LOW		0

/*
 * Expanded GPIO port map
 */
/* PCA9539 U2 */
//PORT0
#define	ETH_RST	    Hw0   // DNP : Ethernet Controller Reset
#define	DXB0_RST	Hw1   // DMB, DAB Reset
#define	CAM_RST	    Hw2   // DNP : Camera Module Reset
#define	CAS_RST	    Hw3   // DNP : CAS Reset
#define	AUTH_RST	Hw4   // iPod Auth Reset
#define	FM_RST		Hw5   // FM Transceiver Reset
#define	RTC_RST	    Hw6   // DNP : RTC Reset
#define SATA_ON      Hw6   // SATA_ON
#define	BT_WAKE		Hw7   // DNP : Bluetooth Wakeup
#define HDMI_ON      Hw7   // HDMI_ON
//PORT1
#define	DXB0_IRQ    Hw8   // INPUT
#define	BT_HWAKE	Hw9   // INPUT : Bluetooth Host Wakeup
#define	FM_IRQ		Hw10  // INPUT : FM Receiver IRQ
#define	CP_READY    Hw11  // INPUT : iPod CP Ready
#define	DXB_GP0		Hw12  // DXBGP0
#define	CAM_FL_EN	Hw13  // DNP : Camera Flash Light En/Disable
#define	LCD_BL_EN	Hw13  // DNP : LCD Backlight En/Disable
#define	MUTE_CTL	Hw14  // Audio Mute Control
#define	CAS_GP	    Hw15  // CASGP
#define TV_SLEEP    Hw15  // DNP : TV Sleep Signal
#define HDD_RST     Hw15  // DNP : HDD Reset

/* PCA9539 U3 */
//PORT0
#define	ATAPI		Hw0   // IDE Disk Interface
#define	LCD			Hw1   // LCD Power
#define	LVDSIVT		Hw2   // LVDS Inverter
#define	CAM			Hw3   // Camera Module Power
#define	CODEC		Hw4   // External Audio CODEC
#define	FMTC		Hw5   // FM Transceiver
#define	SD0			Hw6   // SD Card Slot 0
#define	SD1			Hw7   // SD Card Slot 1
//PORT1
#define	BT			Hw8   // Bluetooth
#define	CAS			Hw9   // CAS
#define	CAN			Hw10  // CAN Interface Controller
#define	ETH			Hw11  // Ethernet Controller
#define	DXB			Hw12  // DMB, DAB Power
#define	IPOD		Hw13  // iPOD Connection Power
#define	PWRGP4		Hw14  // GPIO4 Power
#define	LVDSLPCTRL	Hw15  // LVDS LCD Controller

/* PCA9538 U4 */
//PORT0
#if 1
#define	DVBUS       Hw0    // USB Device VBUS
#define	HVBUS_ON    Hw1    // USB Host VBUS
#define HDMI_LVDS_ON Hw2   // HDMI_LVDS Interface Power
#define PWR_GP0      Hw3   // GPIO0 Power
#define PWR_GP2      Hw4   // GPIO2 Power
#define PWR_GP3      Hw5   // GPIO3 Power
#define VCORE_CTL    Hw6   // Core Voltage Power
#define PWR_GP1      Hw7   // GPIO1 Power

#else
#define	DVBUS       Hw0    // USB Device VBUS
#define	HVBUS       Hw1    // USB Host VBUS
#define	SATAHDMI    Hw2    // HDMI_LVDS Interface Power
#define	PWRGP0      Hw3    // GPIO0 Power
#define	PWRGP1      Hw7    // GPIO1 Power
#define	PWRGP2      Hw4    // GPIO2 Power
#define	PWRGP3      Hw5    // GPIO3 Power
#define	VCORECTL    Hw6    // Core Voltage Power
#endif

/*
 * PCA953x command
 */
enum pca9539_cmd
{
	PCA9539_INPUT_0		= 0,
	PCA9539_INPUT_1		= 1,
	PCA9539_OUTPUT_0	= 2,
	PCA9539_OUTPUT_1	= 3,
	PCA9539_INVERT_0	= 4,
	PCA9539_INVERT_1	= 5,
	PCA9539_DIRECTION_0	= 6,
	PCA9539_DIRECTION_1	= 7,
};

enum pca9538_cmd
{
	PCA9538_INPUT_0		= 0,
	PCA9538_OUTPUT_0	= 1,
	PCA9538_INVERT_0	= 2,
	PCA9538_DIRECTION_0	= 3,
};

/*
 * PWR_GP1's status value for USB OHCI & OTG
 *
 *  b 0 0 0 0  0 0 0 0
 *        a b      c d
 *  - a : USB_OTG exist (Hw5)
 *  - b : USB_OHCI exist (Hw4)
 *  - c : USB_OTG status (Hw1)
 *  - d : USB_OHCI status (Hw0)
 */
extern unsigned int PWR_GP1_STATUS;
#define PWR_GP1_OTG_EXIST	Hw5
#define PWR_GP1_OHCI_EXIST	Hw4
#define PWR_GP1_OTG_STATUS	Hw1
#define PWR_GP1_OHCI_STATUS	Hw0

#endif	/*__PCA953X_H__*/
