/*
 * arch/arm/mach-tcc93xx/tcc_clocktbl_ddr3.h
 *
 * TCC93xx cpufreq driver
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

static struct tcc_freq_table_t gtClockLimitTable[] = {
	/*  CPU /   DDI /   MEM /   GPU /    IO /  VBUS /  VCOD /   SMU /  HSIO /   CAM /  DSUB */
	{ 100000,      0, 125000,      0,  40000,      0,      0,  36000,  96000,      0,      0},
	{ 144000,      0, 125000,      0,  40000,      0,      0,  36000,  96000,      0,      0},
	{ 200000,      0, 125000,      0,  40000,      0,      0,  50000,  96000,      0,      0},
	{ 300000,      0, 150000,      0,  40000,      0,      0,  75000,  96000,      0,      0},
	{ 400000,      0, 200000,      0,  93333,      0,      0, 100000,  96000,      0,      0},
	{ 500000,      0, 250000,      0, 146667,      0,      0, 125000,  96000,      0,      0},
	{ 600000,      0, 300000,      0, 200000,      0,      0, 150000,  96000,      0,      0},
	{ 700000,      0, 350000,      0, 253333,      0,      0, 175000,  96000,      0,      0},
#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
	{ 800000,      0, 400000,      0, 360000,      0,      0, 200000,  96000,      0,      0},
#else
	{ 800000,      0, 400000,      0, 306667,      0,      0, 200000,  96000,      0,      0},
#endif
};

const struct tcc_freq_table_t gtCameraClockLimitTable[] =
{
	{ 144000, 150000, 350000,      0,      0,      0,      0,      0,      0, 250000,      0 },
	{ 292000, 192000, 400000,      0,      0,      0,      0,      0,      0, 250000,      0 },
	{ 800000, 297000, 400000,      0,      0,      0,      0,      0,      0, 250000,      0 },	//HD1080P,	// 1920 x 1080		
};

const struct tcc_freq_table_t gtISPCameraClockLimitTable[] =
{
	{ 144000, 144000, 350000,      0,      0,      0,      0,      0,      0, 250000,      0 },
	{ 292000, 192000, 400000,      0,      0,      0,      0,      0,      0, 250000,      0 },
	{ 800000, 297000, 400000,      0,      0,      0,      0,      0,      0, 250000,      0 },	//HD1080P,	// 1920 x 1080		
};

const struct tcc_freq_table_t gtVpuNormalClockLimitTable[] =
{
	{ 144000, 192000, 150000,      0,      0, 150000, 128000,      0,      0,      0,      0 },	//D1,		// 720 x 480
	{ 292000, 192000, 250000,      0,      0, 192000, 150000,      0,      0,      0,      0 },	//HD720P,	// 1280 x 720
#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
	{ 600000, 297000, 400000,      0,      0, 360000, 297000,      0,      0,      0,      0 },	//HD1080P,	// 1920 x 1080
#else
	{ 600000, 297000, 400000,      0,      0, 384000, 297000,      0,      0,      0,      0 },	//HD1080P,	// 1920 x 1080
#endif
};

const struct tcc_freq_table_t gtJpegClockLimitTable[]= {
	{      0, 192000, 150000,      0,      0, 150000, 128000,      0,      0,      0,      0 }, //D1,	
	{      0, 192000, 250000,      0,      0, 192000, 150000,      0,      0,      0,      0 }, //HD720P
#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
	{      0, 297000, 400000,      0,      0, 360000, 297000,      0,      0,      0,      0 },	//HD1080P,	// 1920 x 1080	
#else
	{      0, 297000, 400000,      0,      0, 384000, 297000,      0,      0,      0,      0 },	//HD1080P,	// 1920 x 1080	
#endif
};

const struct tcc_freq_table_t gtJpegMaxClockLimitTable = {
#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
	  800000, 297000, 400000,      0,      0, 360000, 297000,      0,      0,      0,      0		   	
#else
	  800000, 297000, 400000,      0,      0, 384000, 297000,      0,      0,      0,      0		   	
#endif
};

const struct tcc_freq_table_t gtHdmiClockLimitTable = {
	  800000, 400000, 400000,      0, 250000,      0,      0,      0,      0,      0,      0
};

const struct tcc_freq_table_t gtMaliClockLimitTable = {
	       0,      0, 320000, 250000,      0,      0,      0,      0,      0,      0,      0
};

const struct tcc_freq_table_t stFBClockLimitTable = {
//	       0, 108000, 125000,      0,  96000,      0,      0,      0,      0,      0,      0 	// RGB565
	       0, 108000, 125000,      0,  96000,      0,      0,      0,      0,      0,      0 	// RGB888
};

const struct tcc_freq_table_t stPowerResumeClockLimitTable = {
	  600000,      0, 300000,      0, 200000,      0,      0, 150000,      0,      0,      0
};

const struct tcc_freq_table_t gtTvClockLimitTable = {
	       0, 400000, 400000,      0,      0,      0,      0,      0,      0,      0,      0
};

const struct tcc_freq_table_t gtEthernetClockLimitTable = {
	  800000, 297000, 320000,      0, 192000,      0,      0,      0,      0,      0,      0
};

const struct tcc_freq_table_t gtBtClockLimitTable = {
	  500000,      0, 166000,      0,      0,      0,      0,      0,      0,      0,      0
};

const struct tcc_freq_table_t gtUSBClockLimitTable = {
	       0,      0,      0,      0,  96000,      0,      0,      0,      0,      0,      0
};

const struct tcc_freq_table_t gtAppClockLimitTable = {
	  800000,      0, 320000,      0,      0,      0,      0,      0,      0,      0,      0
};

const struct tcc_freq_table_t gtRemoconClockLimitTable = {
	       0,      0,      0,      0, 192000,      0,      0,      0,      0,      0,      0
};

const struct tcc_freq_table_t gtHSIOClockLimitTable = {
	       0,      0, 320000,      0,      0,      0,      0,      0, 250000,      0,      0
};

const struct tcc_freq_table_t gtHSIONormalClockLimitTable = {
	       0,      0,      0,      0,      0,      0,      0,      0, 120000,      0,      0
};

EXPORT_SYMBOL(gtCameraClockLimitTable);
EXPORT_SYMBOL(gtISPCameraClockLimitTable);
EXPORT_SYMBOL(gtVpuNormalClockLimitTable);
EXPORT_SYMBOL(gtJpegClockLimitTable);
EXPORT_SYMBOL(gtJpegMaxClockLimitTable);
EXPORT_SYMBOL(gtHdmiClockLimitTable);
EXPORT_SYMBOL(gtMaliClockLimitTable);
EXPORT_SYMBOL(stFBClockLimitTable);
EXPORT_SYMBOL(gtTvClockLimitTable);
EXPORT_SYMBOL(gtEthernetClockLimitTable);
EXPORT_SYMBOL(gtBtClockLimitTable);
EXPORT_SYMBOL(gtAppClockLimitTable);
EXPORT_SYMBOL(gtUSBClockLimitTable);
EXPORT_SYMBOL(gtRemoconClockLimitTable);
EXPORT_SYMBOL(gtHSIOClockLimitTable);
EXPORT_SYMBOL(gtHSIONormalClockLimitTable);
