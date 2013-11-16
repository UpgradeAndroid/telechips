/*
 * include/linux/sis_i2c.h - platform data structure for SiS 81x/9200 family
 *
 * Copyright (C) 2009 SiS, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_SIS_I2C_H
#define _LINUX_SIS_I2C_H

#define SIS_I2C_NAME	"sis_i2c_ts"

struct sis_touch_keys_button {
	int code;
};

struct sis_ts_platform_data {
	unsigned intr;
	unsigned wakeup;
	unsigned power;
	struct sis_touch_keys_button *buttons;
	int nbuttons;
};

#define _I2C_INT_ENABLE		1

#define MAX_FINGERS		10

#define X_SENSE_LINE		38	//8
#define Y_SENSE_LINE		22	//12

#ifdef _FOR_LEGACY_SIS81X
// Fixed point mode in tracking algorothm mapping to 0-4095
#define SIS_MAX_X	        4095
#define SIS_MAX_Y	        4095
#else
// SiS9200 Resolution mode
//#define SIS_MAX_X             4095
//#define SIS_MAX_Y             4095
// SiS9200 Fixed point mode
#define SIS_MAX_X		(X_SENSE_LINE * 128)
#define SIS_MAX_Y		(Y_SENSE_LINE * 128)
#endif

#define SIS_CMD_NORMAL		0x0
#define SIS_CMD_RECALIBRATE	0x87
#define MAX_READ_BYTE_COUNT	16
#define MSK_TOUCHNUM		0x0f
#define MSK_HAS_CRC		0x10
#define MSK_DATAFMT		0xe0
#define MSK_PSTATE		0x0f
#define MSK_PID			0xf0
#define TOUCHDOWN		0x0
#define TOUCHUP			0x1
#define RES_FMT			0x00
#define FIX_FMT			0x40

#ifdef _SMBUS_INTERFACE
#define CMD_BASE		0
#else
#define CMD_BASE		1	//i2c
#endif

#define PKTINFO			CMD_BASE + 1

#define P1TSTATE		2

#define NO_T			0x02
#define SINGLE_T		0x09
#define MULTI_T			0x0e
#define LAST_ONE		0x07
#define LAST_TWO		0x0c
#define BUTTON_T		0x05
#define CRCCNT(x)		((x + 0x1) & (~0x1))

#endif /* _LINUX_SIS_I2C_H */
