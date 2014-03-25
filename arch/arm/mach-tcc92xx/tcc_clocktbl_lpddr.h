/*
 * arch/arm/mach-tcc92x/tcc_clocktbl_lpddr.h
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
	/*  CPU  /  DDI  /   MEM /  GPU  /   IO  /  VBUS /  VCOD /   SMU */
	{  36000, 128000,  24000,      0,  16000,      0,      0,  36000 },	// 1050
	{  54000, 166000,  32000,      0,  24000,      0,      0,  36000 },	// 1050
	{  72000, 166000,  64000,      0,  38400,      0,      0,  36000 },	// 1050
	{ 126000, 166000,  64000,      0,  64000,      0,      0,  36000 },	// 1050
	{ 144000, 166000,  83000,      0,  83000,      0,      0,  36000 },	// 1050
	{ 294000, 192000, 100000,      0, 128000,      0,      0,  75000 },	// 1100
	{ 364500, 192000, 166000,      0, 128000,      0,      0, 121500 },	// 1100
	{ 425250, 192000, 166000,      0, 166000,      0,      0, 121500 },	// 1150
	{ 486000, 192000, 166000,      0, 166000,      0,      0, 121500 },	// 1200
	{ 500000, 192000, 166000,      0, 166000,      0,      0, 125000 },	// 1200
	#if defined(CONFIG_HIGH_PERFORMANCE)
	{ 540000, 192000, 192000,      0, 166000,      0,      0, 135000 },	// 1250
	{ 600000, 192000, 200000,      0, 166000,      0,      0, 150000 },	// 1300
	#if defined(CONFIG_CPU_OVERCLOCKING_700MHZ)
	{ 704000, 192000, 200000,      0, 166000,      0,      0, 150000 },	// 1400
	#endif
	#endif
};

const struct tcc_freq_table_t gtCameraClockLimitTable[] =
{
	{ 144000, 166000, 166000,      0,      0,      0,      0,      0 },
	{ 144000, 166000, 166000,      0,      0,      0,      0,      0 }
};

const struct tcc_freq_table_t gtVpuNormalClockLimitTable[] =
{
	{ 144000, 166000, 100000,      0,      0,  83000, 110000,      0 },	//D1,		// 720 x 480	  
#if defined(CONFIG_HIGH_PERFORMANCE)
	{ 294000, 192000, 200000,      0,      0, 192000, 192000,      0 },	//HD720P,	// 1280 x 720
#else
	{ 294000, 192000, 166000,      0,      0, 192000, 166000,      0 },	//HD720P,	// 1280 x 720
#endif
};

const struct tcc_freq_table_t gtJpegClockLimitTable[]= {
/* CAUTION :: memory clock must sync with camera*/
#if defined(CONFIG_CPU_OVERCLOCKING_700MHZ)
	{ 704000, 192000, 166000,      0,      0, 192000, 192000,      0 }, //D1, 
	{ 704000, 192000, 166000,      0,      0, 192000, 192000,      0 }  //HD720P
#elif !defined(CONFIG_CPU_OVERCLOCKING_700MHZ) && defined(CONFIG_HIGH_PERFORMANCE)
	{ 600000, 192000, 166000,      0,      0, 192000, 192000,      0 }, //D1, 
	{ 600000, 192000, 166000,      0,      0, 192000, 192000,      0 }  //HD720P
#else
	{ 500000, 192000, 166000,      0,      0, 192000, 166000,      0 }, //D1, 
	{ 500000, 192000, 166000,      0,      0, 192000, 166000,      0 }  //HD720P
#endif
	
};

const struct tcc_freq_table_t gtJpegMaxClockLimitTable = {
#if defined(CONFIG_CPU_OVERCLOCKING_700MHZ)
	 704000, 192000, 200000,      0,       0, 192000, 192000,      0	// 1400
#elif !defined(CONFIG_CPU_OVERCLOCKING_700MHZ) && defined(CONFIG_HIGH_PERFORMANCE)
	 600000, 192000, 200000,      0,       0, 192000, 192000,      0	// 1300
#else
	 500000, 192000, 166000,      0,       0, 192000, 166000,      0	// 1200
#endif
};

const struct tcc_freq_table_t gtHdmiClockLimitTable = {
	#if defined(CONFIG_HIGH_PERFORMANCE)
	       0, 192000, 200000,      0,      0,      0,      0,      0
	#else
	       0, 192000, 166000,      0,      0,      0,      0,      0
	#endif // CONFIG_HIGH_PERFORMANCE
};

const struct tcc_freq_table_t gtMaliClockLimitTable = {
	       0,      0,  83000, 192000,      0,      0,      0,      0
};

const struct tcc_freq_table_t stFBClockLimitTable = {
//	       0, 128000,  64000,      0,  96000,      0,      0,      0
	       0, 128000,  83000,      0,  96000,      0,      0,      0
};

const struct tcc_freq_table_t stPowerResumeClockLimitTable = {
	  500000,       0, 166000,       0,  166000,       0,       0,	125000
};

const struct tcc_freq_table_t gtTvClockLimitTable = {
	#if defined(CONFIG_HIGH_PERFORMANCE)
	  540000, 192000, 200000,      0,      0,      0,      0,      0
	#else
	  486000, 192000, 166000,      0,      0,      0,      0,      0
	#endif // CONFIG_HIGH_PERFORMANCE
};

const struct tcc_freq_table_t gtBtClockLimitTable = {
	  500000,      0, 166000,      0,      0,      0,      0,      0
};

const struct tcc_freq_table_t gtUSBClockLimitTable[] = {
	{     0,       0, 	  0,   	   0, 96000,       0,      0,      0},
	{ 500000,	   0, 198000,	   0, 166000,	   0,	   0,	   0 }, // Actived
//	{ 500000,	   0, 240000,	   0, 166000,	   0,	   0,	   0 }, // Actived		
};

const struct tcc_freq_table_t gtAppClockLimitTable = {
	#if defined(CONFIG_HIGH_PERFORMANCE)
	#if defined(CONFIG_CPU_OVERCLOCKING_700MHZ)
	  704000,      0, 200000,      0,      0,      0,      0,      0
	#else
	  600000,      0, 200000,      0,      0,      0,      0,      0
	#endif
	#else
	  500000,      0, 166000,      0,      0,      0,      0,      0
	#endif
};

const struct tcc_freq_table_t gtRemoconClockLimitTable = {
	        0,      0,      0,     0, 192000,      0,      0,      0
};

EXPORT_SYMBOL(gtCameraClockLimitTable);
EXPORT_SYMBOL(gtVpuNormalClockLimitTable);
EXPORT_SYMBOL(gtJpegClockLimitTable);
EXPORT_SYMBOL(gtJpegMaxClockLimitTable);
EXPORT_SYMBOL(gtHdmiClockLimitTable);
EXPORT_SYMBOL(gtMaliClockLimitTable);
EXPORT_SYMBOL(stFBClockLimitTable);
EXPORT_SYMBOL(gtTvClockLimitTable);
EXPORT_SYMBOL(gtBtClockLimitTable);
EXPORT_SYMBOL(gtAppClockLimitTable);
EXPORT_SYMBOL(gtUSBClockLimitTable);
EXPORT_SYMBOL(gtRemoconClockLimitTable);
