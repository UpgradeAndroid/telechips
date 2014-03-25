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
	/*  CPU  /   DDI  /   MEM /   GPU  /   IO   / VBUS / VCOD /   SMU */
	{  36000, 1280000,  24000,  960000,  160000,  960000,  960000,  360000 },	// 1050
	{  54000, 1660000,  32000,  960000,  240000,  960000,  960000,  360000 },	// 1050
	{  72000, 1660000,  64000,  960000,  384000,  960000,  960000,  360000 },	// 1050
	{ 126000, 1660000,  64000,  960000,  640000,  960000,  960000,  360000 },	// 1050
	{ 144000, 1660000,  83000,  960000,  830000,  960000,  960000,  360000 },	// 1050
	{ 294000, 1920000, 100000, 1660000, 1280000,  960000,  960000,  750000 },	// 1100
	{ 364500, 1920000, 166000, 1660000, 1280000,  960000,  960000, 1215000 },	// 1100
	{ 425250, 1920000, 166000, 1660000, 1660000,  960000,  960000, 1215000 },	// 1150
	{ 486000, 1920000, 166000, 1660000, 1660000,  960000,  960000, 1215000 },	// 1200
	{ 500000, 1920000, 166000, 1920000, 1660000,  960000,  960000, 1250000 },	// 1200
	#if defined(CONFIG_HIGH_PERFORMANCE)
	{ 540000, 1920000, 192000, 1920000, 1660000,  960000,  960000, 1350000 },	// 1250
	{ 600000, 1920000, 200000, 1920000, 1660000,  960000,  960000, 1500000 },	// 1300
	#if defined(CONFIG_CPU_OVERCLOCKING_700MHZ)
	{ 704000, 1920000, 200000, 1920000, 1660000,  960000,  960000, 1500000 },	// 1400
	#endif
	#endif
};

const struct tcc_freq_table_t gtCameraClockLimitTable[] =
{
	{ 144000, 1660000, 166000, 1660000,       0,       0,       0,       0 },
	{ 144000, 1660000, 166000, 1660000,       0,       0,       0,       0 }
};

const struct tcc_freq_table_t gtVpuNormalClockLimitTable[] =
{
	{ 144000, 1660000, 100000, 1660000,       0,  830000, 1100000,       0 },	//D1,		// 720 x 480	  
#if defined(CONFIG_HIGH_PERFORMANCE)
	{ 294000, 1920000, 200000, 1920000, 	  0, 1920000, 1920000,		 0 },	//HD720P,	// 1280 x 720
#else
	{ 294000, 1920000, 166000, 1920000, 	  0, 1920000, 1660000,		 0 },	//HD720P,	// 1280 x 720
#endif
};

const struct tcc_freq_table_t gtJpegClockLimitTable[]= {
/* CAUTION :: memory clock must sync with camera*/
#if defined(CONFIG_CPU_OVERCLOCKING_700MHZ)
	{ 704000, 1920000, 166000, 1920000, 	  0, 1920000, 1920000,		 0 }, //D1, 
	{ 704000, 1920000, 166000, 1920000, 	  0, 1920000, 1920000,		 0 }  //HD720P
#elif !defined(CONFIG_CPU_OVERCLOCKING_700MHZ) && defined(CONFIG_HIGH_PERFORMANCE)
	{ 600000, 1920000, 166000, 1920000, 	  0, 1920000, 1920000,		 0 }, //D1, 
	{ 600000, 1920000, 166000, 1920000, 	  0, 1920000, 1920000,		 0 }  //HD720P
#else
	{ 500000, 1920000, 166000, 1920000, 	  0, 1920000, 1660000,		 0 }, //D1, 
	{ 500000, 1920000, 166000, 1920000, 	  0, 1920000, 1660000,		 0 }  //HD720P
#endif
	
};

const struct tcc_freq_table_t gtJpegMaxClockLimitTable = {
#if defined(CONFIG_CPU_OVERCLOCKING_700MHZ)
	 704000, 1920000, 200000, 1920000,        0, 1920000, 1920000,       0	// 1400
#elif !defined(CONFIG_CPU_OVERCLOCKING_700MHZ) && defined(CONFIG_HIGH_PERFORMANCE)
	 600000, 1920000, 200000, 1920000,        0, 1920000, 1920000,       0	// 1300
#else
	 500000, 1920000, 166000, 1920000,        0, 1920000, 1660000,       0	// 1200
#endif
};

const struct tcc_freq_table_t gtHdmiClockLimitTable = {
	#if defined(CONFIG_HIGH_PERFORMANCE)
	       0, 1920000, 200000,       0,       0,       0,       0,       0
	#else
	       0, 1920000, 166000,       0,       0,       0,       0,       0
	#endif // CONFIG_HIGH_PERFORMANCE
};

const struct tcc_freq_table_t gtMaliClockLimitTable = {
	  144000,       0,  83000, 1920000,       0,       0,       0,       0
};

const struct tcc_freq_table_t stFBClockLimitTable = {
//	       0, 1280000,  64000,  960000,  960000,       0,       0,       0
	       0, 1280000,  83000,  960000,  960000,       0,       0,       0
};

const struct tcc_freq_table_t stPowerResumeClockLimitTable = {
	  500000,       0, 166000,       0,  166000,       0,       0,	125000
};

const struct tcc_freq_table_t gtTvClockLimitTable = {
	#if defined(CONFIG_HIGH_PERFORMANCE)
	  540000, 1920000, 200000,       0,       0,       0,       0,       0
	#else
	  486000, 1920000, 166000,       0,       0,       0,       0,       0
	#endif // CONFIG_HIGH_PERFORMANCE
};

const struct tcc_freq_table_t gtBtClockLimitTable = {
	  500000,       0, 166000,       0,       0,       0,       0,       0
};

//EXPORT_SYMBOL(gtPerformaceClockLimitTable);
//EXPORT_SYMBOL(gtClockLimitTable);
EXPORT_SYMBOL(gtCameraClockLimitTable);
EXPORT_SYMBOL(gtVpuNormalClockLimitTable);
EXPORT_SYMBOL(gtJpegClockLimitTable);
EXPORT_SYMBOL(gtJpegMaxClockLimitTable);
EXPORT_SYMBOL(gtHdmiClockLimitTable);
EXPORT_SYMBOL(gtMaliClockLimitTable);
EXPORT_SYMBOL(stFBClockLimitTable);
EXPORT_SYMBOL(gtTvClockLimitTable);
EXPORT_SYMBOL(gtBtClockLimitTable);
