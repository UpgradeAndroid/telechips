/*
 * Copyright (c) 2011 Telechips, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the 
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _PLATFORM_REG_PHYSICAL_H_
#define _PLATFORM_REG_PHYSICAL_H_

#ifdef __cplusplus
extern "C" {
#endif

//---> Sangwon_temp
	/* temporally type */
	typedef struct {
		unsigned VALUE			:16;
		unsigned				:16;
	} TCC_DEF16BIT_IDX_TYPE;

	typedef union {
		unsigned short			nREG;
		TCC_DEF16BIT_IDX_TYPE	bREG;
	} TCC_DEF16BIT_TYPE;

	typedef struct {
		unsigned VALUE			:32;
	} TCC_DEF32BIT_IDX_TYPE;

	typedef union {
		unsigned long			nREG;
		TCC_DEF32BIT_IDX_TYPE	bREG;
	} TCC_DEF32BIT_TYPE;
// <--- Sangwon_temp


/************************************************************************
*	Bit Field Definition
************************************************************************/
#define	Hw37									(1LL << 37)
#define	Hw36									(1LL << 36)
#define	Hw35									(1LL << 35)
#define	Hw34									(1LL << 34)
#define	Hw33									(1LL << 33)
#define	Hw32									(1LL << 32)
#define	Hw31									0x80000000
#define	Hw30									0x40000000
#define	Hw29									0x20000000
#define	Hw28									0x10000000
#define	Hw27									0x08000000
#define	Hw26									0x04000000
#define	Hw25									0x02000000
#define	Hw24									0x01000000
#define	Hw23									0x00800000
#define	Hw22									0x00400000
#define	Hw21									0x00200000
#define	Hw20									0x00100000
#define	Hw19									0x00080000
#define	Hw18									0x00040000
#define	Hw17									0x00020000
#define	Hw16									0x00010000
#define	Hw15									0x00008000
#define	Hw14									0x00004000
#define	Hw13									0x00002000
#define	Hw12									0x00001000
#define	Hw11									0x00000800
#define	Hw10									0x00000400
#define	Hw9										0x00000200
#define	Hw8										0x00000100
#define	Hw7										0x00000080
#define	Hw6										0x00000040
#define	Hw5										0x00000020
#define	Hw4										0x00000010
#define	Hw3										0x00000008
#define	Hw2										0x00000004
#define	Hw1										0x00000002
#define	Hw0										0x00000001
#define	HwZERO									0x00000000



/*******************************************************************************
*
*    TCC892x DataSheet PART 2 SMU & PMU
*
********************************************************************************/
#include "structures_smu_pmu.h"

#define HwCKC_BASE								(0x74000000)

#define HwPIC_BASE								(0x74100000)
#define HwVIC_BASE								(0x74100200)

#define HwTMR_BASE								(0x74300000)

#define HwPMU_BASE								(0x74400000)

#define HwSMUI2C_BASE							(0x74500000)

#define HwGPIO_BASE								(0x74200000)
#define HwGPIOA_BASE							(0x74200000)
#define HwGPIOB_BASE							(0x74200040)
#define HwGPIOC_BASE							(0x74200080)
#define HwGPIOD_BASE							(0x742000C0)
#define HwGPIOE_BASE							(0x74200100)
#define HwGPIOF_BASE							(0x74200140)
#define HwGPIOG_BASE							(0x74200180)
#define HwGPIOHDMI_BASE							(0x742001C0)
#define HwGPIOADC_BASE							(0x74200200)

#define HwSMUCONFIG_BASE						(0x74600000)



/*******************************************************************************
*
*    TCC892x DataSheet PART 3 GRAPHIC BUS
*
********************************************************************************/
#include "structures_graphic.h"

#define HwGPU_BASE								(0x70000000)

#define HwGRPBUSCONFIG_BASE						(0x70010000)



/*******************************************************************************
*
*    TCC88x DataSheet PART 4 MEMORY BUS
*
********************************************************************************/
#include "structures_memory.h"

#define HwMEMORY_BASE							(0x73000000)



/*******************************************************************************
*
*    TCC88x DataSheet PART 5 IO BUS
*
********************************************************************************/
#include "structures_io.h"

#define HwEHI_BASE								(0x76000000)

#define HwMPEFEC_BASE							(0x76010000)

#define HwSDMMC0_BASE							(0x76020000)
#define HwSDMMC1_BASE							(0x76020200)
#define HwSDMMC2_BASE							(0x76020400)
#define HwSDMMC3_BASE							(0x76020600)
#define HwSDMMC_CHCTRL_BASE						(0x76020800)

#define HwGDMA0_BASE							(0x76030000)
#define HwGDMA1_BASE							(0x76030100)
#define HwGDMA2_BASE							(0x76030200)

#define HwOVERLAYMIXER_BASE						(0x76040000)

#define HwPWM_BASE								(0x76050000)

#define HwSMC_BASE								(0x76060000)

#define HwRTC_BASE								(0x76062000)

#define HwREMOTE_BASE							(0x76063000)

#define HwTSADC_BASE							(0x76064000)

#define HwEDI_BASE								(0x76065000)

#define HwIOBUSCFG_BASE							(0x76066000)

#define HwPROTECT_BASE							(0x76067000)

#define HwAXIBM_BASE							(0x76068000)

#define HwAUDIO1_ADMA_BASE						(0x76100000)
#define HwAUDIO1_DAI_BASE						(0x76101000)
#define HwAUDIO1_CDIF_BASE						(0x76101080)
#define HwAUDIO1_SPDIFTX_BASE					(0x76102000)

#define HwAUDIO0_ADMA_BASE						(0x76200000)
#define HwAUDIO0_DAI_BASE						(0x76201000)
#define HwAUDIO0_CDIF_BASE						(0x76201080)
#define HwAUDIO0_SPDIFTX_BASE					(0x76202000)
#define HwAUDIO0_SPDIFRX_BASE					(0x76202800)

#define HwI2C_MASTER0_BASE						(0x76300000)
#define HwI2C_MASTER1_BASE						(0x76310000)
#define HwI2C_MASTER2_BASE						(0x76320000)
#define HwI2C_MASTER3_BASE						(0x76330000)
#define HwI2C_SLAVE0_BASE						(0x76340000)
#define HwI2C_SLAVE1_BASE						(0x76350000)
#define HwI2C_PORTCFG_BASE						(0x76360000)

#define HwUART0_BASE							(0x76370000)
#define HwUART1_BASE							(0x76380000)
#define HwUART2_BASE							(0x76390000)
#define HwUART3_BASE							(0x763A0000)
#define HwUART4_BASE							(0x763B0000)
#define HwUART5_BASE							(0x763C0000)
#define HwUART6_BASE							(0x763D0000)
#define HwUART7_BASE							(0x763E0000)
#define HwUART_PORTCFG_BASE						(0x763F0000)

#define HwIDE_BASE								(0x76400000)

#define HwMSTICK_BASE							(0x76500000)

#define HwNFC_BASE								(0x76600000)
#define HwECC_BASE								(0x76600200)

#define HwSMC0_BASE								(0x76700000)
#define HwSMC1_BASE								(0x76700400)
#define HwSMC2_BASE								(0x76700800)
#define HwSMC3_BASE								(0x76700C00)

#define HwTSIF0_BASE							(0x76800000)
#define HwTSIF1_BASE							(0x76810000)
#define HwTSIF2_BASE							(0x76820000)
#define HwTSIF_PORTCFG_BASE						(0x76830000)
#define HwTSIF_DMA0_BASE						(0x76840000)
#define HwTSIF_DMA1_BASE						(0x76850000)
#define HwTSIF_DMA2_BASE						(0x76860000)
#define HwTSIF_PID_BASE							(0x76870000)

#define HwGPSB0_BASE							(0x76900000)
#define HwGPSB1_BASE							(0x76910000)
#define HwGPSB2_BASE							(0x76920000)
#define HwGPSB3_BASE							(0x76930000)
#define HwGPSB4_BASE							(0x76940000)
#define HwGPSB5_BASE							(0x76950000)
#define HwGPSB_PORTCFG_BASE						(0x76960000)
#define HwGPSB_PIDTBL_BASE						(0x76970000)

#define HwUSBOTG_BASE							(0x76A00000)

// Source Control
#define Hw2D_SACTRL_S2_ARITHMODE 			(Hw10+Hw9+Hw8)
#define Hw2D_SACTRL_S1_ARITHMODE 			(Hw6+Hw5+Hw4)
#define Hw2D_SACTRL_S0_ARITHMODE 			(Hw2+Hw1+Hw0)
#define Hw2D_SFCTRL_S2_Y2REN 				(Hw26)
#define Hw2D_SFCTRL_S1_Y2REN 				(Hw25)
#define Hw2D_SFCTRL_S0_Y2REN 				(Hw24)
#define Hw2D_SFCTRL_S2_Y2RMODE 				(Hw21+Hw20)
#define Hw2D_SFCTRL_S1_Y2RMODE 				(Hw19+Hw18)
#define Hw2D_SFCTRL_S0_Y2RMODE 				(Hw17+Hw16)
#define Hw2D_SACTRL_S2_CHROMAEN				(Hw18)
#define Hw2D_SACTRL_S1_CHROMAEN				(Hw17)
#define Hw2D_SACTRL_S0_CHROMAEN				(Hw16)
#define Hw2D_SFCTRL_S2_SEL					(Hw5+Hw4)
#define Hw2D_SFCTRL_S1_SEL					(Hw3+Hw2)
#define Hw2D_SFCTRL_S0_SEL					(Hw1+Hw0)

// Source Operator Pattern
#define	HwGE_OP_ALL					(HwGE_ALPHA + HwGE_PAT_RY + HwGE_PAT_GU + HwGE_PAT_BV)
#define	HwGE_ALPHA					(HwGE_PAT_GU + HwGE_PAT_BV )						// ALPHA VALUE
#define	HwGE_PAT_RY					(Hw16+Hw17+Hw18+Hw19+Hw20+Hw21+Hw22+Hw23)	// Pattern Value RED,   Y
#define	HwGE_PAT_GU					(Hw8+Hw9+Hw10+Hw11+Hw12+Hw13+Hw14+Hw15)		// Pattern Value GREEN, U
#define	HwGE_PAT_BV					(Hw0+Hw1+Hw2+Hw3+Hw4+Hw5+Hw6+Hw7)			// Pattern Value BULE,  V

// Source Operation Control
#define	HwGE_OP_CTRL_ACON1			(Hw30+Hw29+Hw28)							// Alpha-value control 1
#define	HwGE_OP_CTRL_ACON0			(Hw26+Hw25+Hw24)							// Alpha-value control 0
#define	HwGE_OP_CTRL_CCON1			(Hw23+Hw22+Hw21+Hw20)						// color control 1
#define	HwGE_OP_CTRL_CCON0			(Hw19+Hw18+Hw17+Hw16)						// color control 1
#define	HwGE_OP_CTRL_ATUNE			(Hw13+Hw12)									// Alpha value tuning 
#define 	HwGE_OP_CTRL_CSEL			(Hw9+Hw8)									// chroma-key 
#define   HwGE_OP_CTRL_OPMODE		(Hw4+Hw3+Hw2+Hw1+Hw0)						// operation mode


/*******************************************************************************
*
*    TCC88x DataSheet PART 6 HSIO BUS
*
********************************************************************************/
#include "structures_hsio.h"

#define HwGMAC_BASE								(0x71700000)

#define HwUSB20HOST_EHCI_BASE					(0x71200000)
#define HwUSB20HOST_OHCI_BASE					(0x71300000)

#define HwHSIOBUSCFG_BASE						(0x71EA0000)

#define HwDMAX_BASE								(0x71EB0000)

#define HwCIPHER_BASE							(0x71EC0000)



/*******************************************************************************
*
*    TCC88x DataSheet PART 7 DISPLAY BUS
*
********************************************************************************/
#include "structures_display.h"

#define HwVIOC_BASE								(0x72000000)

#define	BASE_ADDR_CPUIF				(0x72010000)
#define 	BASE_ADDR_VIOC				HwVIOC_BASE
/* DISP */
#define	HwVIOC_LCD  		((volatile int *)(BASE_ADDR_VIOC  + 0x0000        ))	// [15:14] == 2'b00, [13:12] == 1'b00
#define	HwVIOC_DISP0		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0000)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_DISP1		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0100)) // 64 words	// TCC8910 
#define	HwVIOC_DISP2		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0200)) // 64 words	// TCC8910

/* RDMA */
#define	HwVIOC_RDMA00	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0400)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_RDMA01	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0500)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_RDMA02	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0600)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_RDMA03	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0700)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_RDMA04	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0800)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_RDMA05	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0900)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_RDMA06	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0A00)) // 64 words	// TCC8910 | TCC8010
#if defined (VIOC_TCC8910) || defined (VIOC_TCC8920)
#define	HwVIOC_RDMA07	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0B00)) // 64 words	// TCC8910
#define	HwVIOC_RDMA08	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0C00)) // 64 words	// TCC8910
#define	HwVIOC_RDMA09	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0D00)) // 64 words	// TCC8910
#define	HwVIOC_RDMA10	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0E00)) // 64 words	// TCC8910
#define	HwVIOC_RDMA11	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x0F00)) // 64 words	// TCC8910
#define	HwVIOC_RDMA12	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1000)) // 64 words	// TCC8910
#define	HwVIOC_RDMA13	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1100)) // 64 words	// TCC8910
#define	HwVIOC_RDMA14	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1200)) // 64 words	// TCC8910
#define	HwVIOC_RDMA15	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1300)) // 64 words	// TCC8910
#define	HwVIOC_RDMA16	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1400)) // 64 words	// TCC8910
#define	HwVIOC_RDMA17	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1500)) // 64 words	// TCC8910
#endif

/* WMIX */
#define	HwVIOC_WMIX0		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1800)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_WMIX1		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1900)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_WMIX2		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1A00)) // 64 words	// TCC8910 | TCC8010
#if defined (VIOC_TCC8910) || defined (VIOC_TCC8920)
#define	HwVIOC_WMIX3		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1B00)) // 64 words	// TCC8910
#define	HwVIOC_WMIX4		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1C00)) // 64 words	// TCC8910
#define	HwVIOC_WMIX5		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1D00)) // 64 words	// TCC8910
#define	HwVIOC_WMIX6		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1E00)) // 64 words	// TCC8910
#endif

#define	HwVIOC_WMIX0_ALPHA		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1840)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_WMIX1_ALPHA		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1940)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_WMIX2_ALPHA		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x1A40)) // 64 words	// TCC8910 | TCC8010

/* SCALER */
#define	HwVIOC_SC0   		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x2000)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_SC1   		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x2100)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_SC2   		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x2200)) // 64 words	// TCC8910 | TCC8010
#if defined (VIOC_TCC8910) || defined (VIOC_TCC8920)
#define	HwVIOC_SC3   		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x2300)) // 64 words	// TCC8910
#endif
/* WDMA */
#define	HwVIOC_WDMA00	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x2800)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_WDMA01	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x2900)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_WDMA02	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x2A00)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_WDMA03	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x2B00)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_WDMA04	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x2C00)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_WDMA05	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x2D00)) // 64 words	// TCC8910 | TCC8010
#define	HwVIOC_WDMA06	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x2E00)) // 64 words	// TCC8910 | TCC8010
#if defined (VIOC_TCC8910) || defined (VIOC_TCC8920)
#define	HwVIOC_WDMA07	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x2F00)) // 64 words	// TCC8910
#define	HwVIOC_WDMA08	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x3000)) // 64 words	// TCC8910
#endif
/* DEINTLS */
#define	HwVIOC_DEINTLS	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x3800)) // 64 words	// TCC8910 | TCC8010
#if defined (VIOC_TCC8910) || defined (VIOC_TCC8920)
#define	HwVIOC_FDLY0		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x3900)) // 64 words	// TCC8910
#define	HwVIOC_FDLY1		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x3A00)) // 64 words	// TCC8910
#define	HwVIOC_FIFO 		((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x3B00)) // 64 words	// TCC8910
#define	HwVIOC_DEBLOCK	((volatile int *)(BASE_ADDR_VIOC  + 0x0000 + 0x3C00)) // 64 words	// TCC8910
#endif
//*********************************************************************************************************************************************
// [15:14] == 2'b01
//*********************************************************************************************************************************************
/* VIN */
#define	HwVIOC_VINDEMUX		((volatile int *)(BASE_ADDR_VIOC  + 0xA800		  ))				// TCC8910 | TCC8010
#define	HwVIOC_VIN00		((volatile int *)(BASE_ADDR_VIOC  + 0x4000        ))				// TCC8910 | TCC8010
#define	HwVIOC_VIN01		((volatile int *)(BASE_ADDR_VIOC  + 0x4400        ))				// TCC8910 | TCC8010 
#define	HwVIOC_VIN10		((volatile int *)(BASE_ADDR_VIOC  + 0x5000        ))				// TCC8910 | TCC8010 
#define	HwVIOC_VIN11		((volatile int *)(BASE_ADDR_VIOC  + 0x5400        ))				// TCC8910 | TCC8010 
#if defined (VIOC_TCC8910) || defined (VIOC_TCC8920)
#define	HwVIOC_VIN20		((volatile int *)(BASE_ADDR_VIOC  + 0x6000        ))				// TCC8910 
#define	HwVIOC_VIN21		((volatile int *)(BASE_ADDR_VIOC  + 0x6400        ))				// TCC8910 
#define	HwVIOC_VIN30		((volatile int *)(BASE_ADDR_VIOC  + 0x7000        ))				// TCC8910 
#define	HwVIOC_VIN31		((volatile int *)(BASE_ADDR_VIOC  + 0x7400        ))				// TCC8910 
#endif

//*********************************************************************************************************************************************
// [15:14] == 2'b10
//*********************************************************************************************************************************************
#define	HwVIOC_FILT2D		((volatile int *)(BASE_ADDR_VIOC  + 0x8000        ))				// TCC8910
#define	HwVIOC_LUT   		((volatile int *)(BASE_ADDR_VIOC  + 0x9000        ))				// TCC8910 | TCC8010
#define	HwVIOC_LUT_TAB	((volatile int *)(BASE_ADDR_VIOC  + 0x9400		))
#define	HwVIOC_CONFIG		((volatile int *)(BASE_ADDR_VIOC  + 0xA000        ))				// TCC8910 | TCC8010
#define	HwVIOC_IREQ   		((volatile int *)(BASE_ADDR_VIOC  + 0xA000 + 0x000)) //  16 word
#define	HwVIOC_PCONFIG	((volatile int *)(BASE_ADDR_VIOC  + 0xA000 + 0x040)) //  32 word
#define	HwVIOC_POWER  		((volatile int *)(BASE_ADDR_VIOC  + 0xA000 + 0x0C0)) //  16 word
#define	HwVIOC_FCODEC		((volatile int *)(BASE_ADDR_VIOC  + 0xB000        ))				// TCC8910

//*********************************************************************************************************************************************
// [15:14] == 2'b11
//*********************************************************************************************************************************************
/* VIOC TIMER */
#define	HwVIOC_TIMER_BASE	((volatile int *)(BASE_ADDR_VIOC  + 0xC000        ))				// TCC8910 | TCC8010
#define	HwVIOC_TIMER 			((volatile int *)(BASE_ADDR_VIOC  + 0xC000		  ))

/* VIQE */
#define	HwVIOC_VIQE0_BASE	((volatile int *)(BASE_ADDR_VIOC  + 0xD000        ))				// TCC8910 | TCC8010
#if defined (VIOC_TCC8910) || defined (VIOC_TCC8920)
#define	HwVIOC_VIQE1_BASE		((volatile int *)(BASE_ADDR_VIOC  + 0xE000        ))				// TCC8910 
#endif
#define 	HwVIOC_VIQE0 			(HwVIOC_VIQE0_BASE)
#if defined (VIOC_TCC8910) || defined (VIOC_TCC8920)
#define 	HwVIOC_VIQE1 			(HwVIOC_VIQE1_BASE)
#define	HwVIOC_MMU_BASE		((volatile int *)(BASE_ADDR_VIOC  + 0xF000        ))				// TCC8910 
#define	HwVIOC_MMU   			((volatile int *)(BASE_ADDR_VIOC  + 0xF000		  ))
#endif

/* CPUIF */
#define	HwVIOC_CPUIF 			((volatile int *)(BASE_ADDR_CPUIF + 0x0000      ))	// 
#define	HwVIOC_CPUIF0			((volatile int *)(BASE_ADDR_CPUIF + 0x0000		))					// TCC8910 | TCC8010
// [09:08] == 2'b00,
// [09:08] == 2'b00, [07:06] == 2'b01, [05] == 1'b0
// [09:08] == 2'b00, [07:06] == 2'b01, [05] == 1'b1
// [09:08] == 2'b00, [07:06] == 2'b10, [05] == 1'b0
// [09:08] == 2'b00, [07:06] == 2'b10, [05] == 1'b1
#if defined (VIOC_TCC8910) || defined (VIOC_TCC8920)
#define	HwVIOC_CPUIF1			((volatile int *)(BASE_ADDR_CPUIF + 0x0100		))					// TCC8910
#endif

// [09:08] == 2'b01,
// [09:08] == 2'b01, [07:06] == 2'b01, [05] == 1'b0
// [09:08] == 2'b01, [07:06] == 2'b01, [05] == 1'b1
// [09:08] == 2'b01, [07:06] == 2'b10, [05] == 1'b0
// [09:08] == 2'b01, [07:06] == 2'b10, [05] == 1'b1
/* OUT CONFIGURE */
#define HwVIOC_OUTCFG			((volatile int *)(BASE_ADDR_CPUIF + 0x0200		))	// [09:08] == 2'b10, 


#define HwNTSCPAL_BASE							(0x72200000)
#define HwNTSCPAL_ENC_CTRL_BASE					(0x72200800)

#define HwHDMI_CTRL_BASE						(0x72300000)
#define HwHDMI_CORE_BASE						(0x72310000)
#define HwHDMI_AES_BASE							(0x72320000)
#define HwHDMI_SPDIF_BASE						(0x72330000)
#define HwHDMI_I2S_BASE							(0x72340000)
#define HwHDMI_CEC_BASE							(0x72350000)

#define HwDDI_CONFIG_BASE						(0x72380000)



/*******************************************************************************
*
*    TCC88x DataSheet PART 8 VIDEO BUS
*
********************************************************************************/
#include "structures_video.h"


#ifdef __cplusplus
}
#endif

#endif /* _PLATFORM_REG_PHYSICAL_H_ */
