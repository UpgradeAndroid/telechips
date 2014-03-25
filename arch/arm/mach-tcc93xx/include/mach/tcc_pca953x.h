/*
 * linux/arch/arm/mach-tcc9300/include/mach/tcc_pca953x.h
 *
 * Author:  <linux@telechips.com>
 * Created: 21th March, 2009 
 * Description: GPIO_Expender driver
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

#include <mach/bsp.h>


/*
 * EXPORT_SYMBOL
 */
extern int tcc_pca953x_setup(int slave, int name, int direction, int value, int mode);
extern int tcc_pca953x_read(int slave, unsigned char cmd, unsigned char *rd_buf);
extern int tcc_pca953x_write(int slave, const unsigned char *wr_buf, int count);

/*
 * PCA953x I2C slave address
 */
#define PCA9539_U1_SLAVE_ADDR	0x74
#define PCA9539_U2_SLAVE_ADDR	0x77
#define PCA9539_U3_SLAVE_ADDR	0x75
#define PCA9538_U14_SLAVE_ADDR	0x70

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
/* 
 * PCA9539 U1 
 */
//PORT0
#define LCD_ON          Hw0
#define CODEC_ON        Hw1
#define SD0_ON          Hw2
#define SD1_ON          Hw3
#define SD2_ON          Hw4
#define EXT_CODEC_ON    Hw5
#define GPS_PWREN       Hw6
#define BT_ON           Hw7
//PORT1
#define DXB_ON          Hw8
#define IPOD_ON         Hw9
#define CAS_ON          Hw10
#define FMTC_ON         Hw11
#define PCAM_PWR_ON     Hw12
#define CAM1_ON         Hw13
#define CAM2_ON         Hw14
#define ATAPI_ON        Hw15

/* 
 * PCA9539 U2 
 */
//PORT0
#define MUTE_CTL        Hw0
#define LVDSIVT_ON      Hw1
#define LVDS_LP_CTRL    Hw2
#define AUTH_RST        Hw3
#define BT_RST          Hw4
#define SDWF_RST        Hw5
#define CAS_RST         Hw6
#define CAS_GP          Hw7
//PORT1
#define DXB1_PD         Hw8
#define DXB2_PD         Hw9
#define PWR_CONTROL0    Hw10
#define PWR_CONTROL1    Hw11
#define HDD_RST         Hw12
#define OTG0_VBUS_EN    Hw13
#define OTG1_VBUS_EN    Hw14
#define HOST_VBUS_EN    Hw15

/* 
 * PCA9539 U3 
 */
//PORT0
#define FMT_RST         Hw0
#define FMT_IRQ         Hw1
#define BT_WAKE         Hw2
#define BT_HWAKE        Hw3
#define PHY2_ON         Hw4
#define COMPASS_RST     Hw5
#define CAM_FL_EN       Hw6
#define CAM2_FL_EN      Hw7
//PORT1
#define CAM2_RST        Hw8
#define CAM2_PWDN       Hw9
#define LCD_RST         Hw10
#define TV_SLEEP        Hw11
#define ETH_ON          Hw12
#define ETH_RST         Hw13
#define SMART_AUX1      Hw14
#define SMART_AUX2      Hw15

/* 
 * PCA9538 U14
 *     - TCC93XX_PWR_DISCRETE_SV0.1
 */
//PORT0 only
#define HDMI_ON         Hw0
#define MIPI_ON         Hw1
#define TP1_NC          Hw2
#define VDD5V_EN        Hw3		// VDD 5V Output Enable - SATA, HDMI etc... (Active High)
#define TP2_NC          Hw4
#define PWR_GP0         Hw5		// Input, Charge Done (Active Low)
#define PWR_GP2         Hw6		// USB Input Current Limit Selection (Low:100mA, High:500mA-Default)
#define PWR_GP3         Hw7		// USB Suspend Mode Selection (Low:Normal-Default, High:USB Suspend)


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
