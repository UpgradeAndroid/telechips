/*
 * arch/arm/mach-tcc92x/ddr.h
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

#ifndef __DDR_H
#define __DDR_H

#define HwBIT0 (((uint)0x1)<<0)
#define HwBIT1 (((uint)0x1)<<1)
#define HwBIT2 (((uint)0x1)<<2)
#define HwBIT3 (((uint)0x1)<<3)
#define HwBIT4 (((uint)0x1)<<4)
#define HwBIT5 (((uint)0x1)<<5)
#define HwBIT6 (((uint)0x1)<<6)
#define HwBIT7 (((uint)0x1)<<7)
#define HwBIT8 (((uint)0x1)<<8)
#define HwBIT9 (((uint)0x1)<<9)
#define HwBIT10 (((uint)0x1)<<10)
#define HwBIT11 (((uint)0x1)<<11)
#define HwBIT12 (((uint)0x1)<<12)
#define HwBIT13 (((uint)0x1)<<13)
#define HwBIT14 (((uint)0x1)<<14)
#define HwBIT15 (((uint)0x1)<<15)
#define HwBIT16 (((uint)0x1)<<16)
#define HwBIT17 (((uint)0x1)<<17)
#define HwBIT18 (((uint)0x1)<<18)
#define HwBIT19 (((uint)0x1)<<19)
#define HwBIT20 (((uint)0x1)<<20)
#define HwBIT21 (((uint)0x1)<<21)
#define HwBIT22 (((uint)0x1)<<22)
#define HwBIT23 (((uint)0x1)<<23)
#define HwBIT24 (((uint)0x1)<<24)
#define HwBIT25 (((uint)0x1)<<25)
#define HwBIT26 (((uint)0x1)<<26)
#define HwBIT27 (((uint)0x1)<<27)
#define HwBIT28 (((uint)0x1)<<28)
#define HwBIT29 (((uint)0x1)<<29)
#define HwBIT30 (((uint)0x1)<<30)
#define HwBIT31 (((uint)0x1)<<31)

/*
 * PL34X Register Definitions
 */
#define HwDDR2PHY	((volatile DDR2PHY *)(0xf0304400))
#define HwEMCCFG	((volatile EMCCFG *)(0xf0303000))
#define	HwPL340		((volatile PL340 *)(0xf0301000))
#define	HwPL341		((volatile PL341 *)(0xf0302000))

#define	PL34X_GO		0
#define	PL34X_SLEEP		1
#define	PL34X_WAKEUP		2
#define	PL34X_PAUSE		3
#define	PL34X_CONFIGURE		4
#define	PL34X_ACTIVE_PAUSE	7

typedef	struct {
	uint	status;
	uint	command;
	uint	direct_cmd;
	uint	config0;
	uint	refresh;
	uint	cas_latency;	// n/2 cycles : 0 ~ 15
	uint	write_latency;
	uint	tMRD;
	uint	tRAS;
	uint	tRC;
	uint	tRCD;
	uint	tRFC;
	uint	tRP;
	uint	tRRD;
	uint	tWR;
	uint	tWTR;
	uint	tXP;
	uint	tXSR;
	uint	tESR;
	uint	config2;
	uint	config3;
	uint	tFAW;
	uint	undef0[64-22];	// 0x0XX
	uint	config_id0;
	uint	config_id1;
	uint	config_id2;
	uint	config_id3;
	uint	config_id4;
	uint	config_id5;
	uint	config_id6;
	uint	config_id7;
	uint	config_id8;
	uint	config_id9;
	uint	config_id10;
	uint	config_id11;
	uint	config_id12;
	uint	config_id13;
	uint	config_id14;
	uint	config_id15;
	uint	undef1[64-16];	// 0x1XX
	uint	config_chip0;
	uint	config_chip1;
	uint	config_chip2;
	uint	config_chip3;
	uint	undef2[64-4];	// 0x2XX
	uint	user_status;
	uint	user_config0;
	uint	user_config1;
	uint	feature_ctrl;
	uint	undef3[64-4];	// 0x3XX
	uint	undef4[64];	// 0x4XX
	uint	undef5[64];	// 0x5XX
	uint	undef6[64];	// 0x6XX
	uint	undef7[64];	// 0x7XX
	uint	undef8[64];	// 0x8XX
	uint	undef9[64];	// 0x9XX
	uint	undefA[64];	// 0xAXX
	uint	undefB[64];	// 0xBXX
	uint	undefC[64];	// 0xCXX
	uint	undefD[64];	// 0xDXX
	uint	int_cfg;
	uint	int_inputs;
	uint	int_outputs;
	uint	undefE[64-3];	// 0xEXX
	uint	undefF[64-8];	// 0xFXX
	uint	periph_id_0;
	uint	periph_id_1;
	uint	periph_id_2;
	uint	periph_id_3;
	uint	pcell_id_0;
	uint	pcell_id_1;
	uint	pcell_id_2;
	uint	pcell_id_3;	// 0xFFC
} PL341;

typedef	struct {
	uint	memc_status		: 2;
	uint	memory_width		: 2;
	uint	memory_type		: 3;
	uint	memory_chips		: 2;
	uint	memory_banks0		: 1;
	uint	exclusive_monitors	: 2;
	uint	memory_banks1		: 1;
	uint				:19;
}	PL340_MEMCSTR;

typedef	union {
	uint			nReg;
	PL340_MEMCSTR		bReg;
}	PL340_MEMCSTR_U;

typedef	struct {
	uint	column_bits		: 3;
	uint	row_bits		: 3;
	uint	ap_bit			: 1;
	uint	power_down_prd		: 6;
	uint	auto_power_down		: 1;
	uint	stop_mem_clock		: 1;
	uint	memory_burst		: 3;
	uint	qos_master_bits		: 2;
	uint				: 1;
	uint	active_chips		: 2;
	uint	fp_enable		: 1;
	uint	fp_time			: 7;
	uint	sr_enable		: 1;
}	PL340_MEMCFG0;

typedef	union {
	uint			nReg;
	PL340_MEMCFG0		bReg;
}	PL340_MEMCFG0_U;

typedef	struct {
	uint	sync			: 1;
	uint	a_gt_m_sync		: 1;
	uint	dqm_init		: 1;
	uint	cke_init		: 1;
	uint	memory_width		: 2;
	uint	memory_type		: 3;
	uint	read_delay		: 2;
	uint				:21;
}	PL340_MEMCFG1;

typedef	union {
	uint				nReg;
	PL340_MEMCFG1		bReg;
}	PL340_MEMCFG1_U;

typedef	struct {
	uint	max_outs_refs		: 3;
	uint	prescale		: 9;
	uint				:20;
}	PL340_MEMCFG2;

typedef	union {
	uint			nReg;
	PL340_MEMCFG2		bReg;
}	PL340_MEMCFG2_U;

typedef	struct {
	uint	address_mask		: 8;
	uint	address_match		: 8;
	uint	brc_n_rbc		: 1;
	uint				:15;
}	PL340_CHIPCFG;

typedef	union {
	uint			nReg;
	PL340_CHIPCFG		bReg;
}	PL340_CHIPCFG_U;

typedef	struct {
	PL340_MEMCSTR_U		uMEMCSTR;
	uint	command;
	uint	direct_cmd;
	PL340_MEMCFG0_U		uMEMCFG0;
	uint	refresh_prd;
	uint	cas_latency;
	uint	tDQSS;
	uint	tMRD;
	uint	tRAS;
	uint	tRC;
	uint	tRCD;
	uint	tRFC;
	uint	tRP;
	uint	tRRD;
	uint	tWR;
	uint	tWTR;
	uint	tXP;
	uint	tXSR;
	uint	tESR;
	PL340_MEMCFG1_U		uMEMCFG1;
	PL340_MEMCFG2_U		uMEMCFG2;
	uint	undef0[64-21];	// 0x0XX
	uint	config_id0;
	uint	config_id1;
	uint	config_id2;
	uint	config_id3;
	uint	config_id4;
	uint	config_id5;
	uint	config_id6;
	uint	config_id7;
	uint	config_id8;
	uint	config_id9;
	uint	config_id10;
	uint	config_id11;
	uint	config_id12;
	uint	config_id13;
	uint	config_id14;
	uint	config_id15;
	uint	undef1[64-16];	// 0x1XX
	PL340_CHIPCFG_U		uCHIP0;
	PL340_CHIPCFG_U		uCHIP1;
	PL340_CHIPCFG_U		uCHIP2;
	PL340_CHIPCFG_U		uCHIP3;
	uint	undef2[64-4];	// 0x2XX
	uint	undef3[64];	// 0x3XX
	uint	undef4[64];	// 0x4XX
	uint	undef5[64];	// 0x5XX
	uint	undef6[64];	// 0x6XX
	uint	undef7[64];	// 0x7XX
	uint	undef8[64];	// 0x8XX
	uint	undef9[64];	// 0x9XX
	uint	undefA[64];	// 0xAXX
	uint	undefB[64];	// 0xBXX
	uint	undefC[64];	// 0xCXX
	uint	undefD[64];	// 0xDXX
	uint	undefF[64-8];	// 0xFXX
	uint	periph_id_0;
	uint	periph_id_1;
	uint	periph_id_2;
	uint	periph_id_3;
	uint	pcell_id_0;
	uint	pcell_id_1;
	uint	pcell_id_2;
	uint	pcell_id_3;	// 0xFFC
} PL340;

typedef struct {
	uint	GDDRMODE	: 1;	// DDR/DDR2 or GDDR
	uint	DSTBMODE	: 1;	// Differential Strobe Mode
						// 1'b1 : Single Ended DQS Mode
						// 1'b0 : Differential DQS Mode
	uint	IGATEPD		: 1;	// Input Gate Power-Down
						// 1'b0	: Normal
						// 1'b1 : I/O PAD Input Gating Mode
	uint			:29;
}	DDR2_PHYMODE;

typedef	union {
	uint		nReg;
	DDR2_PHYMODE	bReg;
}	DDR2_PHYMODE_U;

typedef	struct {
	uint	DLLON		: 1;	// DLL On (PDEN, ctrl_dll_on)
	uint	DLLSTART	: 1;	// DLL Start (PDSTART, ctrl_start)
	uint	CTRLHALF	: 1;	// If '1', the DLL can run at low frequency (80~100MHz)
	uint	DLLCLOCK	: 1;	// Coarse Lock (PDCL, ctrl_clock)
	uint	DLLFLOCK	: 1;	// Fine Lock Status (PDFL, ctrl_flock)
	uint	LOCKVALUE	:10;	// [9:2] : Coarse Lock Value
					// [1:0] : Fine Lock Value
	uint	    		:17;
}	DDR2_DLLCTRL;

typedef	union {
	uint			nReg;
	DDR2_DLLCTRL	bReg;
}	DDR2_DLLCTRL_U;

typedef	struct {
	uint	START		: 8;	// Start Point
	uint	INCR		: 8;	// Increase amount of start point
	uint			:16;
}	DDR2_DLLPDCFG;

typedef	union {
	uint			nReg;
	DDR2_DLLPDCFG	bReg;
}	DDR2_DLLPDCFG_U;

typedef	struct {
	uint	DELAY		: 3;	
	uint	OFFSET		: 7;
	uint	ADBEN		: 1;
	uint			:21;
}	DDR2_GATECTRL;

typedef	union {
	uint			nReg;
	DDR2_GATECTRL	bReg;
}	DDR2_GATECTRL_U;

typedef	struct {
	uint	OFFSET		: 8;	// 7bits are valid
	uint			:24;
}	DDR2_RDSLICE;

typedef	union {
	uint		nReg;
	DDR2_RDSLICE	bReg;
}	DDR2_RDSLICE_U;

typedef	struct {
	uint	VALUE		: 8;
	uint			:24;
}	DDR2_DLLFORCELOCK;

typedef	union {
	uint			nReg;
	DDR2_DLLFORCELOCK	bReg;
}	DDR2_DLLFORCELOCK_U;

typedef	struct {
	uint	CALSTART	: 1;
	uint	CALUPDATE	: 1;
	uint	CALFORCE	: 1;
	uint	CALPUFORCE	: 3;
	uint	CALPDFORCE	: 3;
	uint	TERM		: 3;
	uint	TERMSEL		: 1;
	uint	DRIVE		: 3;
	uint	CALPEREN	: 1;
	uint	CALPERVAL	: 4;
	uint			:11;
}	DDR2_ZQCTRL;

typedef	union {
	uint			nReg;
	DDR2_ZQCTRL		bReg;
}	DDR2_ZQCTRL_U;

typedef	struct {
	uint	CALEND		: 1;
	uint	CALERROR	: 1;
	uint	CALPUVAL	: 3;
	uint	CALPDVAL	: 3;
	uint			:24;
}	DDR2_ZQSTATUS;

typedef	union {
	uint			nReg;
	DDR2_ZQSTATUS	bReg;
}	DDR2_ZQSTATUS_U;

typedef	struct {
	uint	DELAY		: 3;
	uint			:29;
}	DDR2_RDDELAY;

typedef	union {
	uint		nReg;
	DDR2_RDDELAY	bReg;
}	DDR2_RDDELAY_U;

typedef	struct {
	DDR2_PHYMODE_U		uPHYMODE;		// PHY Mode Control Register
	DDR2_DLLCTRL_U		uDLLCTRL;		// DLL Control & Status Register
	DDR2_DLLPDCFG_U		uDLLPDCFG;		// DLL Phase Detector Configuration Register
	DDR2_GATECTRL_U		uGATECTRL;		// Gate Control Register
	DDR2_RDSLICE_U		uRDSLICE0;		// Read Data Slice 0 Control Register
	DDR2_RDSLICE_U		uRDSLICE1;		// Read data Slice 1 Control Register
	DDR2_RDSLICE_U		uRDSLICE2;		// Read data Slice 2 Control Register
	DDR2_RDSLICE_U		uRDSLICE3;		// Read data Slice 3 Control Register
	DDR2_RDSLICE_U		uCLKDELAY;		// Clock Delay Config Register
	DDR2_DLLFORCELOCK_U	uDLLFORCELOCK;	// DLL Force Lock Value Register
	DDR2_ZQCTRL_U		uZQCTRL;			// ZQ Calibration Control Register
	DDR2_ZQSTATUS_U		uZQSTATUS;		// ZQ Calibration Status Register
	DDR2_RDDELAY_U		uRDDELAY;		// Read Delay Register
}	DDR2PHY;

typedef	struct {
	uint	QOSOVRD		:16;
	uint			: 6;
	uint	SLOWBUS		: 1;			// valid for PL340 in SDRAM mode
	uint	SYNCOPT		: 1;
	uint	BUSWIDTH	: 2;
	uint	MEMTYPE		: 2;
	uint	RDDELAY		: 3;
	uint	SYNC		: 1;
}	PL340CFG0;

typedef	union {
	uint			nReg;
	PL340CFG0		bReg;
}	PL340CFG0_U;

typedef	struct {
	uint	GTMSYNC		: 1;
	uint	CKE_INIT	: 1;
	uint	DQM_INIT	: 1;
	uint	USE_EBI		: 1;
	uint	EBIGNT		: 1;
	uint	EBIBACKOFF	: 1;
	uint	RSTBYPASS	: 1;
	uint			: 1;
	uint			:24;
}	PL340CFG1;

typedef	union {
	uint			nReg;
	PL340CFG1		bReg;
}	PL340CFG1_U;

typedef	struct {
	uint	AXI_SEL		: 1;	// 0 for PL340, 1 for PL341
	uint	IO_SEL		: 1;	// 0 for PL340, 1 for PL341
	uint	PL341_1CS	: 1;
	uint			: 5;
	uint	PL340_CREQ	: 1;
	uint			: 7;
	uint	PL341_CREQ	: 1;
	uint			: 7;
	uint	ADDRMAP		: 1;
	uint			: 7;
}	EMCCONFIG;

typedef	union {
	uint			nReg;
	EMCCONFIG		bReg;
}	EMCCONFIG_U;

typedef	struct {
	uint	FNC_CB		: 2;
	uint			: 6;
	uint	PL340_SEL	: 1;
	uint			: 23;
}	PHYCTRL;

typedef	union {
	uint			nReg;
	PHYCTRL			bReg;
}	PHYCTRL_U;

typedef	struct {
	uint	PD_DQS		: 1;
	uint	PD_XD		: 1;
	uint	PD_DM		: 1;
	uint	PD_BA		: 1;
	uint	PD_XA		: 1;
	uint	PD_CSN		: 1;
	uint	PD_ODT		: 1;
	uint	PD_CAS		: 1;
	uint	PD_RAS		: 1;
	uint	PD_WEN		: 1;
	uint	PD_CKE		: 1;
	uint	PD_CLK		: 1;
	uint	PD_CLKB		: 1;
	uint	PD_GATE		: 1;
	uint	IO_MODE		: 1;
	uint			:17;
}	SSTL_IOCFG;

typedef	union {
	uint			nReg;
	SSTL_IOCFG		bReg;
}	SSTL_IOCFG_U;

typedef	struct {
	PL340CFG0_U		uPL340CFG0;
	PL340CFG1_U		uPL340CFG1;
	uint			nPL340STS0;
	uint			nReserved0;
	PL340CFG0_U		uPL341CFG0;
	PL340CFG1_U		uPL341CFG1;
	uint			nPL341STS0;
	uint			nReserved1;
	EMCCONFIG_U		uCONFIG0  ;
	PHYCTRL_U		uPHYCTRL  ;
	uint			nPHYSTS   ;
	SSTL_IOCFG_U	uIOCFG0   ;
}	EMCCFG;

#define	DDR2PHY_DDR2		0
#define	DDR2PHY_MDDR		1
#define	DDR2PHY_DDR		2
#define	DDR2PHY_SDR		3

#define	IO_CMOS			0
#define	IO_SSTL			1

#define PL34X_STATUS_CONFIG     0
#define PL34X_STATUS_READY      1
#define PL34X_STATUS_PAUSED     2
#define PL34X_STATUS_LOWPOWER   3

typedef struct  {
	unsigned int	nFreq;		// Frequency
	unsigned int	nClockSource;
	unsigned int	nPMS;
	unsigned int	nDiv;
	unsigned short  nPeriod;	// Clock Cycle Period
	unsigned char   nGateDelay;	// Gate Delay
	unsigned char   nReadDelay;	// Read Delay
	unsigned char   nAddrOrder;	// Address Order RAS-BA-CAS or BA-RAS-CAS
	unsigned char   nCW;		// CAS Width
	unsigned char   nRW;		// RAS Width
	unsigned char   nCL;		// CAS Latency
	unsigned char   nBurstLength;	// Burst Length
	unsigned int    tMRD;
	unsigned int    tRAS;
	unsigned int    tRC;
	unsigned int    tRCD;
	unsigned int    tRFC;
	unsigned int    tRP;
	unsigned int    tRRD;
	unsigned int    tWR;
	unsigned int    tWTR;
	unsigned int    tXP;
	unsigned int    tXSR;
	unsigned int    tESR;
	unsigned int    tFAW;
	unsigned int    tREF_CYC;	
}  ddr_config_t;

static inline void ddr_wait_time(uint nCount)
{
	for ( ; nCount > 0 ; nCount --);
}

static inline int ddr_wait_status(uint type)
{
	while ((HwPL340->uMEMCSTR.nReg & 0x3) != type)
		;
	return 0;
}

static inline int ddr_wait_dll_lock(void)
{
	uint i;

	// Wait DLL Lock
	i = 0;
	do {
		i++;
		if (i > 65536)
			return -1;
	} while ((HwDDR2PHY->uDLLCTRL.nReg & (HwBIT4|HwBIT3)) != (HwBIT4|HwBIT3));
	return 0;
}

static inline void ddr_dll_off(void)
{
	HwDDR2PHY->uDLLCTRL.nReg &=  ~HwBIT0    /* DLL-OFF */
		& ~HwBIT1    			/* DLL-Stop running */
		;
	HwDDR2PHY->uZQCTRL.nReg &=  ~HwBIT0	/* Calibration Start */
		& ~HwBIT1 			/* Update Calibration */
		;
	/* SDRAM IO Control Regieter Gatein Signal Power Down*/
	HwEMCCFG->uIOCFG0.bReg.IO_MODE = IO_CMOS;//&= ~0x00004000;
}

static inline int ddr_wait_zqcal (void)
{
	uint	i;

	i = 0;
	do {
		i++;
		if (i > 65536)
			return -1;
	} while ((HwDDR2PHY->uZQSTATUS.nReg & (HwBIT0)) != (HwBIT0));
	if ( HwDDR2PHY->uZQSTATUS.nReg & (HwBIT1) ) {
		return -1;
	}
	return 0;
}

static inline void ddr_update_zqcal(void)
{
	HwDDR2PHY->uZQCTRL.nReg |=  HwBIT1;
	HwDDR2PHY->uZQCTRL.nReg &= ~HwBIT1;
}

static inline void ddr_dll_on(ddr_config_t *mem_cfg)
{
	uint nReg;

	HwEMCCFG->uIOCFG0.bReg.IO_MODE = IO_CMOS;
	HwEMCCFG->uCONFIG0.bReg.AXI_SEL = 0;
	HwEMCCFG->uCONFIG0.bReg.IO_SEL = 1;
	HwEMCCFG->uPHYCTRL.bReg.PL340_SEL = 1;

	HwDDR2PHY->uPHYMODE.nReg = 0x2;
	HwDDR2PHY->uDLLCTRL.nReg = HwBIT0;	// DLL-On

	if (mem_cfg->nFreq < 110000)
		nReg = 0x3030;
	else if (mem_cfg->nFreq < 145000)
		nReg = 0x2525;
	else if (mem_cfg->nFreq < 180000)
		nReg = 0x2020;
	else
		nReg = 0x1f1f;

	HwDDR2PHY->uDLLPDCFG.nReg = nReg;	// Configurable : Frequency
	HwDDR2PHY->uDLLCTRL.nReg = HwBIT0	// DLL-On
		| HwBIT1			// DLL-Start
		;

	ddr_wait_dll_lock();

	HwDDR2PHY->uDLLFORCELOCK.nReg = 0x35;
	HwDDR2PHY->uGATECTRL.nReg = mem_cfg->nGateDelay;
	HwDDR2PHY->uRDDELAY.nReg = mem_cfg->nReadDelay;

	HwDDR2PHY->uZQCTRL.nReg = 0
		| (0x1      << 0)   // Calibration Start
		| (0x0      << 1)   // Update Calibration
		| (0x0      << 2)   // Override ctrl_force_impp[2:0]/impn[2:0]
		| (0x2      << 3)   // Calibration PULL-UP forced value
		| (0x5      << 6)   // Calibration PULL-DOWN forced value
		| (0x0      << 9)   // On-Die Termination Resistor Value Selection
		| (0x1      << 12)  // Termination Selection    : 0 for disable
		| (0x7      << 13)  // Drive Strength
		| (0x0      << 16)  // Periodic Calibration
		| (0x3      << 17)  // Update Counter Load Value
		;

	HwDDR2PHY->uZQCTRL.nReg = 0
		| (0x1      << 0)   // Calibration Start
		| (0x1      << 1)   // Update Calibration
		| (0x0      << 2)   // Override ctrl_force_impp[2:0]/impn[2:0]
		| (0x2      << 3)   // Calibration PULL-UP forced value
		| (0x5      << 6)   // Calibration PULL-DOWN forced value
		| (0x0      << 9)   // On-Die Termination Resistor Value Selection
		| (0x1      << 12)  // Termination Selection    : 0 for disable
		| (0x7      << 13)  // Drive Strength
		| (0x0      << 16)  // Periodic Calibration
		| (0x3      << 17)  // Update Counter Load Value
		;

	HwDDR2PHY->uZQCTRL.nReg = 0
		| (0x1      << 0)   // Calibration Start
		| (0x0      << 1)   // Update Calibration
		| (0x0      << 2)   // Override ctrl_force_impp[2:0]/impn[2:0]
		| (0x2      << 3)   // Calibration PULL-UP forced value
		| (0x5      << 6)   // Calibration PULL-DOWN forced value
		| (0x0      << 9)   // On-Die Termination Resistor Value Selection
		| (0x1      << 12)  // Termination Selection    : 0 for disable
		| (0x7      << 13)  // Drive Strength
		| (0x0      << 16)  // Periodic Calibration
		| (0x3      << 17)  // Update Counter Load Value
		;

	ddr_wait_zqcal();

	ddr_update_zqcal();
}

static inline unsigned int ddr_time_to_cycle(unsigned int nPeriod, unsigned int nTime, unsigned int nMin, unsigned int nMax)
{
	unsigned int nCycle;

	nCycle = 0;

	nCycle = nTime / nPeriod;
	if (nCycle * nPeriod < nTime)
		++nCycle;

	nCycle = (nCycle > nMax) ? nMax : ((nCycle <= nMin) ? nMin : nCycle);

	return (nCycle);
}

#endif
