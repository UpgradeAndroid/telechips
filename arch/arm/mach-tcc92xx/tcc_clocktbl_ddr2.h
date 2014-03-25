/*
 * arch/arm/mach-tcc92x/tcc_clocktbl_ddr2.h
 *
 * TCC92xx cpufreq driver
 *
 * Copyright (C) 2009 Telechips, Inc.
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
	/*  CPU  /  DDI  /   MEM /  GPU  /  IO   / VBUS  / VCOD  /  SMU */
	{  36000, 108000, 125000,      0,  16000,      0,      0,  36000 },
	{  54000, 108000, 125000,      0,  24000,      0,      0,  36000 },
	{  72000, 108000, 125000,      0,  38400,      0,      0,  36000 },
	{ 126000, 108000, 125000,      0,  64000,      0,      0,  36000 },
	{ 144000, 144000, 125000,      0,  96000,      0,      0,  36000 },
	{ 294000, 144000, 125000,      0, 128000,      0,      0,  75000 },
	{ 364500, 192000, 166000,      0, 128000,      0,      0, 121500 },
	{ 425250, 192000, 198000,      0, 144000,      0,      0, 121500 },
	{ 500000, 192000, 240000,      0, 144000,      0,      0, 125000 },
	{ 600000, 216000, 290000,      0, 192000,      0,      0, 150000 },
#if defined(CONFIG_CPU_HIGHSPEED)
	{ 660000, 330000, 330000,      0, 192000,      0,      0, 165000 },
#endif
	{ 720000, 330000, 330000,      0, 192000,      0,      0, 180000 },
#if defined(CONFIG_CPU_HIGHSPEED)
	{1000000, 330000, 330000,      0, 192000,      0,      0, 250000 },
#endif
};

//sync with gtJpegClockLimitTable
const struct tcc_freq_table_t gtCameraClockLimitTable[] =
{
//	{ 144000, 144000, 166000,      0,      0,      0,      0,      0 },
	{ 144000, 144000, 240000,      0,      0,      0,      0,      0 },
	{ 294000, 192000, 240000,      0,      0,      0,      0,      0 },
	{ 600000, 330000, 330000,      0,      0,      0,      0,      0 },	//HD1080P,	// 1920 x 1080
};

const struct tcc_freq_table_t gtVpuNormalClockLimitTable[] =
{
#if defined(CONFIG_TCC_FB_HIGH_CLOCK)
	{ 144000, 192000, 290000,	   96000,	   0, 144000, 128000,	   0 }, //D1,		// 720 x 480
	{ 294000, 192000, 290000,	   96000,	   0, 192000, 144000,	   0 }, //HD720P,	// 1280 x 720
#else
	{ 144000, 144000, 125000,      96000,      0, 144000, 128000,      0 },	//D1,		// 720 x 480
	{ 294000, 192000, 198000,      96000,      0, 192000, 144000,      0 },	//HD720P,	// 1280 x 720
#endif//
	{ 600000, 330000, 330000,      216000,      0, 330000, 216000,      0 },	//HD1080P,	// 1920 x 1080
};

//sync with gtCameraClockLimitTable
const struct tcc_freq_table_t gtJpegClockLimitTable[]= {
	{      0, 144000, 166000,      96000,      0, 144000, 128000,      0 },	//D1,
	{      0, 192000, 198000,      96000,      0, 192000, 144000,      0 },		//HD720P
	{      0, 330000, 330000,      216000,     0, 330000, 216000,      0 },	//HD1080P,	// 1920 x 1080
};

const struct tcc_freq_table_t gtJpegMaxClockLimitTable = {
	  720000, 330000, 330000,      198000,      0, 330000, 216000,      0
};

const struct tcc_freq_table_t gtHdmiClockLimitTable = {
 	  720000, 330000, 330000,      216000,      0,      0,      0,      0
};

const struct tcc_freq_table_t gtMaliClockLimitTable = {
	       0,      0, 198000, 216000,      0,      0,      0,      0
};

const struct tcc_freq_table_t stFBClockLimitTable = {
#if defined(CONFIG_TCC_FB_HIGH_CLOCK)
	  144000, 192000, 240000, 	0,	96000,		0,		0,		0	
#else
	       0, 108000, 125000,      0,  96000,      0,      0,      0
#endif//
};

const struct tcc_freq_table_t stPowerResumeClockLimitTable = {
 	  500000,      0, 240000,      0, 144000,      0,      0, 125000
};

const struct tcc_freq_table_t gtTvClockLimitTable = {
	  294000, 330000, 330000,      0,      0,      0,      0,      0
};

const struct tcc_freq_table_t gtEthernetClockLimitTable = {
	  720000, 330000, 330000,      0, 192000,      0,      0,      0
};

const struct tcc_freq_table_t gtBtClockLimitTable = {
//	  500000,      0, 166000,      0,      0,      0,      0,      0
//	  500000,      0, 240000,      0, 144000,      0,      0,      0
	  720000,      0, 330000,      0, 192000,      0,      0,      0
};

const struct tcc_freq_table_t gtUSBClockLimitTable[] = {
	{      0,      0,      0,      0,  96000,      0,      0,      0 },
	{ 500000,      0, 198000,      0, 166000,      0,      0,      0 },	// Actived
//	{ 500000,      0, 240000,      0, 166000,      0,      0,      0 },	// Actived
};

const struct tcc_freq_table_t gtAppClockLimitTable = {
	  720000,      0, 330000,      0,      0,      0,      0,      0
};

const struct tcc_freq_table_t gtRemoconClockLimitTable = {
	       0,      0,      0,      0, 192000,      0,      0,      0
};

EXPORT_SYMBOL(gtCameraClockLimitTable);
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
