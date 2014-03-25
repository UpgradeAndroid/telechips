/****************************************************************************
 *   FileName    : TCC92x_Physical.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/

/****************************************************************************

  Revision History

 ****************************************************************************

 ****************************************************************************/

/************************************************************************
*	TCC92x Internal Register Definition File
************************************************************************/
#ifndef	__TCC92x_H__
#define	__TCC92x_H__


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
*	 TCC9200_DataSheet_PART 2 SMU & PMU_V0.00 Dec.11 2008
********************************************************************************/
/************************************************************************
*	1. Clock Controller Register Define			(Base Addr = 0xF0400000) // R/W
************************************************************************/
//---------------------------------------------------------------------------------------------
//31  | 30  | 29  | 28  | 27  | 26  | 25  | 24  | 23  | 22  | 21  | 20  | 19  | 18  | 17  | 16 |
//  												 		|CFGEN|MODE | NCKOE/DPRD           |
//15  | 14  | 13  | 12  | 11  | 10  |  9  |  8  |  7  |   6 |   5 |   4 |   3 |   2 |   1 |  0 |
//   NCKOE/DMIN  		|		NCKOE/DMAX      |		  NCKOE/DCDIV   |     |	    CKSEL      |
//----------------------------------------------------------------------------------------------   
#define HwCLK_BASE					*(volatile unsigned long *)0xF0400000
#define HwCKC						((PCKC)&HwCLK_BASE)

// Clock Chennal 0 ~ 7
enum {
	HwCKC_SYS_CPU	= 0,			// CPU Clock Frequency
	HwCKC_SYS_DBUS,					// Bus Clock for Display Device (Video In/Out + Display Related Hardwares)
	HwCKC_SYS_MBUS,					// CPU Interface and Memory Bus Clock
	HwCKC_SYS_GBUS,					// Bus Clock for Graphic Device
	HwCKC_SYS_IOBUS,				// Bus Clock for I/O Interface Devices
	HwCKC_SYS_VBUS,					// Bus Clock for Video Processing (Video Codec)
	HwCKC_SYS_VCOD,					// Core Clock for Video Codec (Moving Video Only)
	HwCKC_SYS_SMU,					// Bus Clock for System Management Unit
	HwCKC_SYS_MAX
};
#if 1
// Clock Control
#define	HwCLKCTRL_EN				Hw21										// Clock Controller Enable Register (0:Disable, 1:Enable)
#define	HwCLKCTRL_MD				Hw20										// Clock control mode (0:Normal mode, 1:Dynamic control mode)
#define	HwCLKCTRL_CKSEL_Fpll(X)		(X)											// Direct output from PLL0
#define	HwCLKCTRL_CKSEL_Fxin		Hw2											// Direct output from XIN
#define	HwCLKCTRL_CKSEL_Fplldiv(X)	(X+5)										// Divided output from PLL0
#define	HwCLKCTRL_CKSEL_Fxtin		(Hw2|Hw1|Hw0)								// Divided output from XTIN

// PLL Control
#define HwPLLCFG_PD_DIS				~Hw31										// PLL Disable
#define	HwPLLCFG_PD_EN				Hw31										// PLL Enable
#define MAX_PLL_CH					4											// Pll Channel number

// PLL Divide
#define	HwCLKDIVC_P0E_EN			Hw31										// PLL0 Divider Enable
#define	HwCLKDIVC_P0E_DIS			~Hw31										// PLL0 Divider Disable
#define	HwCLKCTRL_P0DIV(X)			((X)*Hw24)									// PLL0 divisor value (FP0DIV = FPLL0/(P0DIV+1)
#define	HwCLKCTRL_P0DIV_MASK		HwCLKCTRL_P0DIV(63)
#define	HwCLKDIVC_P1E_EN			Hw23										// PLL1 Divider Enable
#define	HwCLKDIVC_P1E_DIS			~Hw23										// PLL1 Divider Disable
#define	HwCLKCTRL_P1DIV(X)			((X)*Hw16)									// PLL1 divisor value (FP1DIV = FPLL1/(P1DIV+1)
#define	HwCLKCTRL_P1DIV_MASK		HwCLKCTRL_P1DIV(63)
#define	HwCLKDIVC_P2E_EN			Hw15										// PLL2 Divider Enable
#define	HwCLKDIVC_P2E_DIS			~Hw15										// PLL2 Divider Disable
#define	HwCLKCTRL_P2DIV(X)			((X)*Hw8)									// PLL2 divisor value (FP2DIV = FPLL2/(P2DIV+1)
#define	HwCLKCTRL_P2DIV_MASK		HwCLKCTRL_P2DIV(63)
#define	HwCLKDIVC_P3E_EN			Hw7											// PLL3 Divider Enable
#define	HwCLKDIVC_P3E_DIS			~Hw7										// PLL3 Divider Disable
#define	HwCLKCTRL_P3DIV(X)			((X)*Hw0)									// PLL3 divisor value (FP3DIV = FPLL3/(P3DIV+1)
#define	HwCLKCTRL_P3DIV_MASK		HwCLKCTRL_P3DIV(63)
// XIN/XTIN Divide
#define	HwCLKDIVC_XE_EN				Hw15										// XIN Divider Enable
#define	HwCLKDIVC_XE_DIS			~Hw15										// XIN Divider Disable
#define	HwCLKCTRL_XDIV(X)			((X)*Hw8)									// XIN divisor value (FXDIV = FXIN/(XDIV+1)
#define	HwCLKCTRL_XDIV_MASK			HwCLKCTRL_XDIV(63)
#define	HwCLKDIVC_XTE_EN			Hw7											// XTIN Divider Enable
#define	HwCLKDIVC_XTE_DIS			~Hw7										// XTIN Divider Disable
#define	HwCLKCTRL_XTDIV(X)			((X)*Hw0)									// XTIN divisor value (FXTDIV = FXTIN/(XTDIV+1)
#define	HwCLKCTRL_XTDIV_MASK		HwCLKCTRL_XTDIV(63)

// SWRESET
#define	HwSWRESET_CPU_ON			Hw0											// Reset CPU
#define	HwSWRESET_DDIB_ON			Hw1											// Reset DDI Bus
#define	HwSWRESET_MB_ON				Hw2											// Reset Memory Bus
#define	HwSWRESET_GB_ON				Hw3											// Reset Graphic Bus
#define	HwSWRESET_IO_ON				Hw4											// Reset I/O Bus
#define	HwSWRESET_VB_ON				Hw5											// Reset Video Bus
#define	HwSWRESET_VCC_ON			Hw6											// Reset Video Codec Core
#define	HwSWRESET_SMU_ON			Hw7											// Reset SMU

// PCK Control
#define	HwPCK_MD_DIV				Hw31										// 1:DIVIDER mode 0: DCO mode
#define	HwPCK_EN_EN					Hw28										// Clock Divider Enable
#define	HwPCK_SEL(X)				((X)*Hw24)		
#define	HwPCK_SEL_PLL(X)			HwPCK_SEL(X)								// PLL0~3 Direct Output
#define	HwPCK_SEL_XIN				HwPCK_SEL(4)								// XIN Output
#define	HwPCK_SEL_PLLDIV(X)			HwPCK_SEL(X+5)								// PLL0~3 Divider Output
#define	HwPCK_SEL_XTIN				HwPCK_SEL(9)								// XTIN Output
#define	HwPCK_SEL_EXTCLK			HwPCK_SEL(10)								// External Clock
#define	HwPCK_SEL_XINDIV			HwPCK_SEL(11)								// XIN Divider Output
#define	HwPCK_SEL_XTINDIV			HwPCK_SEL(12)								// XTIN Divider Output
#define	HwPCK_SEL_HDMI_TMDS			HwPCK_SEL(11)								// HDMI TMDS clock. HwPCK_LCD0, HwPCK_LCD1, HwPCK_LCDSI, HwPCK_HDMI
#define	HwPCK_SEL_HDMI_PCLK			HwPCK_SEL(12)								// HDMI PCLK clock. HwPCK_LCD0, HwPCK_LCD1, HwPCK_LCDSI, HwPCK_HDMI
#define	HwPCK_SEL_HDMI_27MHZ		HwPCK_SEL(13)								// HDMI Oscillator clock(27MHz)
#define	HwPCK_SEL_NOT_USED			HwPCK_SEL(14)								// Do Not Used
#define	HwPCK_SEL_U48MHZ			HwPCK_SEL(15)								// 48MHz Clock from nanoPHY (USB PHY)
#define	HwPCK_SEL_MASK				HwPCK_SEL(15)
#endif

/************************************************************************
*	2. Vectored Priority Interrupt Controller Register Map(Base Addr = 0xF0401000)
************************************************************************/
#define	HwPIC_BASE					*(volatile unsigned long *)0xF0401000
#define	HwVIC_BASE					*(volatile unsigned long *)0xF0401200
#define HwPIC						((PPIC)&HwPIC_BASE)
#define HwVIC						((PVIC)&HwVIC_BASE)


// Interrupt Enable 0
#define	HwINT0_EHI0					Hw31										// R/W, External Host Interface0 Interrupt Enable
#define	HwINT0_ECC					Hw30										// R/W, ECC Interrupt Enable 
#define	HwINT0_DMA					Hw29										// R/W, DMA Controller Interrupt Enable
#define	HwINT0_TSADC				Hw28										// R/W, TSADC Interrupt Enable
#define	HwINT0_G2D					Hw27										// R/W, Graphic Engine 2D Hardware Interrupt Enable
#define	HwINT0_3DMMU				Hw26										// R/W, 3D MMU Interrupt Enable
#define	HwINT0_3DGP					Hw25										// R/W, 3D Geometary Interrupt Enable
#define	HwINT0_3DPP					Hw24										// R/W, 3D Pixel Processor Interrupt Enable
#define	HwINT0_VCDC					Hw23										// R/W, Video CODEC Interrupt Enable
#define	HwINT0_JPGE					Hw22										// R/W, JPEG Decoder Interrupt Enable
#define	HwINT0_JPGD					Hw21										// R/W, JPEG Encoder Interrupt Enable
#define	HwINT0_VIPET				Hw20										// R/W, VIPET Controller Interrupt Enable
#define	HwINT0_LCD1					Hw19										// R/W, LCD Controller1 Interrupt Enable
#define	HwINT0_LCD0					Hw18										// R/W, LCD Controller0 Interrupt Enable
#define	HwINT0_CAM					Hw17										// R/W, Camera Interrupt Enable
#define	HwINT0_SC1					Hw16										// R/W, Mem-to-Mem Scaler1 Interrupt Enable
#define	HwINT0_SC0					Hw15										// R/W, Mem-to-Mem Scaler0 Interrupt Enable
#define	HwINT0_EI11					Hw14										// R/W, External Interrupt11 Enable
#define	HwINT0_EI10					Hw13										// R/W, External Interrupt10 Enable
#define	HwINT0_EI9					Hw12										// R/W, External Interrupt9 Enable
#define	HwINT0_EI8					Hw11										// R/W, External Interrupt8 Enable
#define	HwINT0_EI7					Hw10										// R/W, External Interrupt7 Enable
#define	HwINT0_EI6					Hw9											// R/W, External Interrupt6 Enable
#define	HwINT0_EI5					Hw8											// R/W, External Interrupt5 Enable
#define	HwINT0_EI4					Hw7											// R/W, External Interrupt4 Enable
#define	HwINT0_EI3					Hw6											// R/W, External Interrupt3 Enable
#define	HwINT0_EI2					Hw5											// R/W, External Interrupt2 Enable
#define	HwINT0_EI1					Hw4											// R/W, External Interrupt1 Enable
#define	HwINT0_EI0					Hw3											// R/W, External Interrupt0 Enable
#define	HwINT0_SMUI2C				Hw2											// R/W, SMU_I2C Interrupt Enable
#define	HwINT0_TC1					Hw1											// R/W, Timer1 Interrupt Enable
#define	HwINT0_TC0					Hw0											// R/W, Timer0 Interrupt Enable

// Interrupt Enable 1
#define	HwINT1_AEIRQ				Hw31										// R/W, Not maskable error ARM DMA interrupt enable
#define	HwINT1_ASIRQ				Hw30										// R/W, Secure ARM DMA select interrupt enable
#define	HwINT1_AIRQ					Hw29										// R/W, Non secure ARM DMA interrupt enable
#define	HwINT1_APMU					Hw28										// R/W, ARM System Metrics interrupt enable
#define	HwINT1_AUDIO				Hw27										// R/W, AUDIO interrupt enable
#define	HwINT1_ADMA					Hw26										// R/W, AUDIO DMA interrupt enable
#define	HwINT1_DAITX				Hw25										// R/W, DAI transmit interrupt enable
#define	HwINT1_DAIRX				Hw24										// R/W, DAI receive interrupt enable
#define	HwINT1_CDRX					Hw23										// R/W, CDIF receive interrupt enable
#define	HwINT1_TSIF1				Hw22										// R/W, TS interface 1 interrupt enable
#define	HwINT1_TSIF0				Hw21										// R/W, TS interface 0 interrupt enable
#define	HwINT1_GPS2					Hw20										// R/W, GPS AGPS interrupt enable
#define	HwINT1_GPS1					Hw19										// R/W, GPS TCXO expired interrupt enable
#define	HwINT1_GPS0					Hw18										// R/W, GPS RTC expired interrupt enable
#define	HwINT1_NotUsed				Hw17										// R/W, Reserved
#define	HwINT1_UOTG					Hw16										// R/W, USB 2.0 OTG interrupt enable
#define	HwINT1_UART					Hw15										// R/W, UART interrupt enable
#define	HwINT1_SPDTX				Hw14										// R/W, SPDIF transmitter interrupt enable
#define	HwINT1_SD1					Hw13										// R/W, SD/MMC 1 interrupt enable
#define	HwINT1_SD0					Hw12										// R/W, SD/MMC 0 interrupt enable
#define	HwINT1_RTC					Hw11										// R/W, RTC interrupt enable
#define	HwINT1_RMT					Hw10										// R/W, Remote Control interrupt enable
#define	HwINT1_NFC					Hw9											// R/W, Nand flash controller interrupt enable
#define	HwINT1_MS					Hw8											// R/W, Memory Stick interrupt enable
#define	HwINT1_MPEFEC				Hw7											// R/W, MPEFEC interrupt enable
#define	HwINT1_I2C					Hw6											// R/W, I2C interrupt enable
#define	HwINT1_HDD					Hw5											// R/W, HDD controller interrupt enable
#define	HwINT1_GPSB					Hw4											// R/W, GPSB Interrupt Enable
#define	HwINT1_NotUsed1				Hw3											// R/W, Reserved
#define	HwINT1_HDMI					Hw2											// R/W, HDMI interrupt enable
#define	HwINT1_NotUsed2				Hw1											// R/W, Reserved
#define	HwINT1_EHI1					Hw0											// R/W, External Host Interface1 Interrupt Enable

#define	HwALLMSK_FIQ				Hw1										// FIQ mask register
#define	HwALLMSK_IRQ				Hw0										// IRQ mask register
	
/***********************************************************************
*	3. Timer/Counter Register Map (Base Address = 0xF0403000) 
************************************************************************/
#define	HwTMR_BASE					*(volatile unsigned long *)0xF0403000	// Timer/Counter Base Register

/***********************************************************************
*	4. PMU(POWER MANAGEMENT UNIT) Register Map (Base Address = 0xF0404000) 
************************************************************************/
#define	HwPMU_BASE  				*(volatile unsigned long *)0xF0404000 	//R/W   PMU Control Register 
#define 	HwPMU_PWR					*(volatile unsigned long *)0xF0404018
#define 	HwPMU						((PPMU)&HwPMU_BASE)
// PMU Control
#define	HwPMU_CONTROL_IOR			Hw31										// I/O Retension Enable Register (0:Not-Retension, 1:Retension)
#define	HwPMU_CONTROL_FWKU			Hw30										// Fast Wakeup Enable Register (0:Normal, 1:Fast)
#define	HwPMU_CONTROL_FPO			Hw29										// Fast Power-Off (0:Normal, 1:Fast)
#define	HwPMU_CONTROL_AISO			Hw18										// Touch Screen ADC Isolation Enable Register (0:Isolated, 1:Not-Isolated)
#define	HwPMU_CONTROL_ASTM			Hw17										// Touch Screen ADC Stop Mode Register (0:Normal, 1:Stop Mode)
#define	HwPMU_CONTROL_APEN			Hw16										// Touch Screen ADC Power Enable Register (0:Disable, 1:Enable)
#define	HwPMU_CONTROL_INITR			Hw4											// Boot Memory Configuration Register with ITCM (0:Boot-ROM, 1:ITCM)
#define	HwPMU_CONTROL_DPDN			Hw3											// Deep Power Down Mode Register (0:Normal, 1:Deep Power-Down)
#define	HwPMU_CONTROL_PDN			Hw2											// Power Down Mode Register (0:Normal, 1:Power-Down)
#define	HwPMU_CONTROL_POFF			Hw1											// Power Off Mode Register (0:Normal, 1:Power-Off)
#define	HwPMU_CONTROL_XIEN			Hw0											// Main Oscillatro Enable Register (0:Disable, 1:Enable)

// Wake UP Control
#define	HwPMU_WKUP_GPIOE25			Hw31
#define	HwPMU_WKUP_GPIOE24			Hw30
#define	HwPMU_WKUP_GPIOE05			Hw29
#define	HwPMU_WKUP_GPIOE04			Hw28
#define	HwPMU_WKUP_GPIOB31			Hw27
#define	HwPMU_WKUP_GPIOB30			Hw26
#define	HwPMU_WKUP_GPIOA15			Hw25
#define	HwPMU_WKUP_GPIOA14			Hw24
#define	HwPMU_WKUP_GPIOA13			Hw23
#define	HwPMU_WKUP_GPIOA12			Hw22
#define	HwPMU_WKUP_GPIOA11			Hw21
#define	HwPMU_WKUP_GPIOA10			Hw20
#define	HwPMU_WKUP_GPIOA07			Hw19
#define	HwPMU_WKUP_GPIOA06			Hw18
#define	HwPMU_WKUP_GPIOA05			Hw17
#define	HwPMU_WKUP_GPIOA04			Hw16
#define	HwPMU_WKUP_GPIOA03			Hw15
#define	HwPMU_WKUP_GPIOA02			Hw14
#define	HwPMU_WKUP_TSC_UPDOWN		Hw13										// Touch Screen Up/Down Signal
#define	HwPMU_WKUP_TSC_STOP_WKU		Hw12										// Touch Screen Stop Wakeup Signal
#define	HwPMU_WKUP_GPIOD18			Hw11
#define	HwPMU_WKUP_TSC_WKU			Hw10										// Touch Screen Wake-Up Signal
#define	HwPMU_WKUP_GPIOF23			Hw9
#define	HwPMU_WKUP_GPIOF24			Hw8
#define	HwPMU_WKUP_GPIOF25			Hw7
#define	HwPMU_WKUP_GPIOF26			Hw6
#define	HwPMU_WKUP_GPIOF27			Hw5
#define	HwPMU_WKUP_GPIOC31			Hw4
#define	HwPMU_WKUP_GPIOC30			Hw3
#define	HwPMU_WKUP_GPIOC29			Hw2
#define	HwPMU_WKUP_GPIOC28			Hw1
#define	HwPMU_WKUP_RTCWKUP			Hw0											// RTC Wakeup Output Signal

// WatchDoc Control
#define	HwPMU_WATCHDOC_EN			Hw31										// Watchdog Enable Register (0:disable, 1:enable)
#define	HwPMU_WATCHDOC_CLR			Hw30										// Watchdog Clear Register

// Power Off Control
#define	HwPMU_PWROFF_IOB			Hw9											// I/O Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_GB				Hw8											// Graphic Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_DB				Hw7											// DDI Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_VB				Hw6											// Video Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_MB				Hw5											// Memory Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_RSVD2			Hw4											// Reserved
#define	HwPMU_PWROFF_UP				Hw3											// USB Nano Phy Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_RSVD1			Hw2											// Reserved
#define	HwPMU_PWROFF_HD				Hw1											// HDMI Phy Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_DAC			Hw0											// Video DAC Power-Off Control Register (0:Power-On, 1:Power-Off)

/*******************************************************************************
*	5. SMUI2C Controller Register Define   (Base Addr = 0xF0405000)
********************************************************************************/
#define HwSMU_I2CMASTER0_BASE     	*(volatile unsigned long *)0xF0405000
#define HwSMU_I2CMASTER1_BASE     	*(volatile unsigned long *)0xF0405040
#define HwSMU_I2CICLK_BASE     		*(volatile unsigned long *)0xF0405080	//I2C_SCL divider Regist
#define HwI2CSTATUS_BASE            *(volatile unsigned long *)0xF05300C0

/*******************************************************************************
*	 TCC9200_DataSheet_PART 3 GPIO_V0.00 Dec.11 2008
********************************************************************************/
/************************************************************************
*	1. GPIO Register Map (Base Address = 0xF0102000) 
************************************************************************/
#define	HwGPIO_BASE  				*(volatile unsigned long *)0xF0102000  	// 
#define	HwGPIOA_BASE  				*(volatile unsigned long *)0xF0102000  	// 
#define	HwGPIOB_BASE  				*(volatile unsigned long *)0xF0102040  	// 
#define	HwGPIOC_BASE  				*(volatile unsigned long *)0xF0102080  	// 
#define	HwGPIOD_BASE  				*(volatile unsigned long *)0xF01020C0  	// 
#define	HwGPIOE_BASE  				*(volatile unsigned long *)0xF0102100  	// 
#define	HwGPIOF_BASE  				*(volatile unsigned long *)0xF0102140  	// 
#define	HwEINTSEL_BASE  			*(volatile unsigned long *)0xF0102180  	// 
#define HwGPIO						((PGPIO)&HwGPIO_BASE)
#define HwGPIOA						((PGPION)&HwGPIOA_BASE)
#define HwGPIOB						((PGPION)&HwGPIOB_BASE)
#define HwGPIOC						((PGPION)&HwGPIOC_BASE)
#define HwGPIOD						((PGPION)&HwGPIOD_BASE)
#define HwGPIOE						((PGPION)&HwGPIOE_BASE)
#define HwGPIOF						((PGPION)&HwGPIOF_BASE)
#define HwEINTSEL					((PGPIOINT)&HwEINTSEL_BASE)

#define	HwPORTCFG_GPFN0(X)			((X)<<0)	// 0~3
#define HwPORTCFG_GPFN0_MASK		(0xF)		// HwPORTCFG_GPFN0(15)
#define	HwPORTCFG_GPFN1(X)			((X)<<4)	// 4~7
#define HwPORTCFG_GPFN1_MASK		(0xF<<4)	// HwPORTCFG_GPFN1(15)
#define	HwPORTCFG_GPFN2(X)			((X)<<8)	// 8~11
#define HwPORTCFG_GPFN2_MASK		(0xF<<8)	// HwPORTCFG_GPFN2(15)
#define	HwPORTCFG_GPFN3(X)			((X)<<12)	// 12~15
#define HwPORTCFG_GPFN3_MASK		(0xF<<12)	// HwPORTCFG_GPFN3(15)
#define	HwPORTCFG_GPFN4(X)			((X)<<16)	// 16~19
#define HwPORTCFG_GPFN4_MASK		(0xF<<16)	// HwPORTCFG_GPFN4(15)
#define	HwPORTCFG_GPFN5(X)			((X)<<20)	// 20~23
#define HwPORTCFG_GPFN5_MASK		(0xF<<20)	// HwPORTCFG_GPFN5(15)
#define	HwPORTCFG_GPFN6(X)			((X)<<24)	// 24~27
#define HwPORTCFG_GPFN6_MASK		(0xF<<24)	// HwPORTCFG_GPFN6(15)
#define	HwPORTCFG_GPFN7(X)			((X)<<28)	// 28~31
#define HwPORTCFG_GPFN7_MASK		(0xF<<28)	// HwPORTCFG_GPFN7(15)

// Interrupt Selection
typedef enum
{
	SEL_GPIOA0, 	SEL_GPIOA1, 	SEL_GPIOA2, 	SEL_GPIOA3, 	SEL_GPIOA4, 	// 0
	SEL_GPIOA5, 	SEL_GPIOA6, 	SEL_GPIOA7, 	SEL_GPIOA8, 	SEL_GPIOA9, 	// 5
	SEL_GPIOA10,	SEL_GPIOA11,	SEL_GPIOA12,	SEL_GPIOA13,	SEL_GPIOA14,	// 10
	SEL_GPIOA15,	SEL_GPIOD5, 	SEL_GPIOD6, 	SEL_GPIOD7, 	SEL_GPIOD8, 	// 15
	SEL_GPIOD9, 	SEL_GPIOD10,	SEL_GPIOD13,	SEL_GPIOD14,	SEL_GPIOD15,	// 20
	SEL_GPIOD16,	SEL_GPIOD17,	SEL_GPIOD18,	SEL_GPIOD19,	SEL_GPIOD20,	// 25
	SEL_GPIOD23,	SEL_GPIOD24,	SEL_GPIOF25,	SEL_GPIOF26,	SEL_GPIOF27,	// 30
	SEL_GPIOF20,	SEL_GPIOF17,	SEL_GPIOF13,	SEL_GPIOF10,	SEL_GPIOF8, 	// 35
	SEL_GPIOC28,	SEL_GPIOC29,	SEL_GPIOC30,	SEL_GPIOC31,	SEL_GPIOC9, 	// 40
	SEL_GPIOC13,	SEL_GPIOB28,	SEL_GPIOB29,	SEL_GPIOB30,	SEL_GPIOB31,	// 45
	SEL_GPIOB8, 	SEL_GPIOB12,	SEL_GPIOB4, 	SEL_GPIOB5, 	SEL_GPIOB24,	// 50
	SEL_GPIOB25,	SEL_TSWKU,		SEL_TSSTOP, 	SEL_TSUPDN, 	SEL_NotUsed,	// 55
	SEL_NotUsed1,	SEL_MKUP,		SEL_USB_VBON,	SEL_USB_VBOFF					// 60
} ESEL;


#define HwEINTSEL2_EINT11(X)		((X)*Hw24)									// EINT11 Source Selection
#define HwEINTSEL2_EINT11_MASK		HwEINTSEL2_EINT11(63)
#define HwEINTSEL2_EINT10(X)		((X)*Hw16)									// EINT10 Source Selection
#define HwEINTSEL2_EINT10_MASK		HwEINTSEL2_EINT10(63)
#define HwEINTSEL2_EINT9(X) 		((X)*Hw8)									// EINT9 Source Selection
#define HwEINTSEL2_EINT9_MASK		HwEINTSEL2_EINT9(63)
#define HwEINTSEL2_EINT8(X) 		((X)*Hw0)									// EINT8 Source Selection
#define HwEINTSEL2_EINT8_MASK		HwEINTSEL2_EINT8(63)
#define HwEINTSEL1_EINT7(X) 		((X)*Hw24)									// EINT7 Source Selection
#define HwEINTSEL1_EINT7_MASK		HwEINTSEL1_EINT7(63)
#define HwEINTSEL1_EINT6(X) 		((X)*Hw16)									// EINT6 Source Selection
#define HwEINTSEL1_EINT6_MASK		HwEINTSEL1_EINT6(63)
#define HwEINTSEL1_EINT5(X) 		((X)*Hw8)									// EINT5 Source Selection
#define HwEINTSEL1_EINT5_MASK		HwEINTSEL1_EINT5(63)
#define HwEINTSEL1_EINT4(X) 		((X)*Hw0)									// EINT4 Source Selection
#define HwEINTSEL1_EINT4_MASK		HwEINTSEL1_EINT4(63)
#define HwEINTSEL0_EINT3(X) 		((X)*Hw24)									// EINT3 Source Selection
#define HwEINTSEL0_EINT3_MASK		HwEINTSEL0_EINT3(63)
#define HwEINTSEL0_EINT2(X) 		((X)*Hw16)									// EINT2 Source Selection
#define HwEINTSEL0_EINT2_MASK		HwEINTSEL0_EINT2(63)
#define HwEINTSEL0_EINT1(X) 		((X)*Hw8)									// EINT1 Source Selection
#define HwEINTSEL0_EINT1_MASK		HwEINTSEL0_EINT1(63)
#define HwEINTSEL0_EINT0(X) 		((X)*Hw0)									// EINT0 Source Selection
#define HwEINTSEL0_EINT0_MASK		HwEINTSEL0_EINT0(63)


/*******************************************************************************
*	 TCC9200_DataSheet_PART 4 CORE & MEMORY BUS_V0.00 Dec.11 2008
********************************************************************************/
/************************************************************************
*	3. DRAM CONTROLLER Register Map (Base Address = 0xF0301000) 
************************************************************************/
#define	HwDRAM_BASE  				*(volatile unsigned long *)0xF0301000  	//
#define	HwDRAMM0_BASE  				*(volatile unsigned long *)0xF0301000  	//
#define	HwDRAMM1_BASE  				*(volatile unsigned long *)0xF0302000  	//
#define	HwDRAMMISC_BASE  			*(volatile unsigned long *)0xF0303000  	//
#define	HwDRAMPHY_BASE  			*(volatile unsigned long *)0xF0304400  	//
#define	HwDRAMMEMBUS_BASE  			*(volatile unsigned long *)0xF0305004  	//

/************************************************************************
*	4-1. MISC CORE BUS CONFIGURATION REGISTERS				(Base Addr = 0xF0101000)
************************************************************************/
#define	HwCORECFG_BASE				*(volatile unsigned long *)0xF0101000	
       
/************************************************************************
*	4-2. Virtual MMU Table  Register Define	(Base Addr = 0xF7000000)
************************************************************************/
#define	HwVMT_BASE					*(volatile unsigned long *)0x20000000	// VMT Base Regiseter
#define	HwREGION_BASE				*(volatile unsigned long *)0xF0600000	// R/W, Configuration Register for Region 0
#define   HwREGION						((PVMTREGION)&HwREGION_BASE)
// Configuration
#define	HwVMT_SZ(X)					(((X)-1)*Hw12)
#define	SIZE_4GB					32
#define	SIZE_2GB					31
#define	SIZE_1GB					30
#define	SIZE_512MB					29
#define	SIZE_256MB					28
#define	SIZE_128MB					27
#define	SIZE_64MB					26
#define	SIZE_32MB					25
#define	SIZE_16MB					24
#define	SIZE_8MB					23
#define	SIZE_4MB					22
#define	SIZE_2MB					21
#define	SIZE_1MB					20
#define	HwVMT_REGION_AP_ALL			(Hw11+Hw10)
#define	HwVMT_REGION_EN				Hw9											// Region Enable Register
#define	HwVMT_DOMAIN(X)				((X)*Hw5)
#define	HwVMT_CACHE_ON				Hw3											// Cacheable Register
#define	HwVMT_CACHE_OFF				HwZERO
#define	HwVMT_BUFF_ON				Hw2											// Bufferable Register
#define	HwVMT_BUFF_OFF				HwZERO


/*******************************************************************************
*	 TCC9200_DataSheet_PART 5 IO BUS_V0.00 Dec.11 2008
********************************************************************************/
/*******************************************************************************
*	 4. Memory Stick Host Controller Register Define   (Base Addr = 0xF0590000)
********************************************************************************/
#define HwSMSHC_BASE                *(volatile unsigned long *)0xF0590000
#define HwSMSHCPORTCFG_BASE			*(volatile unsigned long *)0xF05F1000
#define HwSMSHC						((PSMSHC)&HwSMSHC_BASE)
#define HwSMSHCPORTCFG				((PSMSHCPORTCFG)&HwSMSHCPORTCFG_BASE)


/********************************************************************************
*	 5. SD/SDIO/MMC/CE-ATA Host Controller Register Define   (Base Addr = 0xF0590000)
********************************************************************************/
//Core 0 Slot 0
#define HwSDCORE0SLOT0_BASE         *(volatile unsigned long *)0xF05A0000
//Core 0 Slot 1
#define HwSDCORE0SLOT1_BASE         *(volatile unsigned long *)0xF05A0100
//Core 1 Slot 2
#define HwSDCORE1SLOT0_BASE         *(volatile unsigned long *)0xF05A0200
//Core 1 Slot 3
#define HwSDCORE1SLOT1_BASE         *(volatile unsigned long *)0xF05A0300
// Channel Control Register
#define HwSDCHCTRL_BASE             *(volatile unsigned long *)0xF05A0800 // R/W 0x0000 SD/MMC port control register
#define HwSDCORE0SLOT0				((PSDHOST)&HwSDCORE0SLOT0_BASE)
#define HwSDCORE0SLOT1				((PSDHOST)&HwSDCORE0SLOT1_BASE)
#define HwSDCORE1SLOT0				((PSDHOST)&HwSDCORE1SLOT0_BASE)
#define HwSDCORE1SLOT1				((PSDHOST)&HwSDCORE1SLOT1_BASE)
#define HwSDCHCTRL					((PSDCHCTRL)&HwSDCHCTRL_BASE)


#define HwSD_COM_TRANS_ABORT	Hw23+Hw22
#define HwSD_COM_TRANS_DATSEL	Hw21		// data present select
#define HwSD_COM_TRANS_CICHK	Hw20		// command index check enable
#define HwSD_COM_TRANS_CRCHK	Hw19		// command CRC check enable
#define HwSD_COM_TRANS_SPI		Hw7 		// SPI mode
#define HwSD_COM_TRANS_ATACMD	Hw6 		// cmd completion enable for CE-ATA
#define HwSD_COM_TRANS_MS		Hw5 		// multi/single block select
#define HwSD_COM_TRANS_DIR		Hw4 		// data transfer direction select
#define HwSD_COM_TRANS_ACMD12	Hw2 		// auto CMD12 enable
#define HwSD_COM_TRANS_BCNTEN	Hw1 		// block count enable
#define HwSD_COM_TRANS_DMAEN	Hw0 		// DMA Enable


#define HwSDCLKSEL_DIV_256		0x80
#define HwSDCLKSEL_DIV_128		0x40
#define HwSDCLKSEL_DIV_64		0x20
#define HwSDCLKSEL_DIV_32		0x10
#define HwSDCLKSEL_DIV_16		0x08
#define HwSDCLKSEL_DIV_8		0x04
#define HwSDCLKSEL_DIV_4		0x02
#define HwSDCLKSEL_DIV_2		0x01
#define HwSDCLKSEL_DIV_0		0x00
#define HwSDCLKSEL_SCK_EN		Hw2
#define HwSDCLKSEL_INCLK_STABLE Hw1
#define HwSDCLKSEL_INCLK_EN 	Hw0

#define HwSD_POWER_POW			Hw8 		// SD bus power
#define HwSD_POWER_SD8			Hw5 		// SD 8-bit mode
#define HwSD_POWER_HS			Hw2 		// high speed enable
#define HwSD_POWER_SD4			Hw1 		// data transfer width
#define HwSD_POWER_VOL33		Hw11+Hw10+Hw9

#define HwSD_CTRL_SDMA			0x00
#define HwSD_CTRL_ADMA32		Hw4
#define HwSD_CTRL_DMA_MASK		Hw4+Hw3

#define HwSD_SRESET_RSTALL		Hw0 	// software reset for All
#define HwSD_SRESET_RSTCMD		Hw1 	// software reset for CMD line
#define HwSD_SRESET_RSTDAT		Hw2 	// software reset for DAT line


// Port Control
#define HwSD_PORTCTRL_CD(X) 		(Hw30 << (X))								// Card Detection for SLOT X. (X = 0 or 1)
#define HwSD_PORTCTRL_WP(X) 		(Hw27 << (X))								// Write Protect for SLOT X. (X = 0 or 1)
#define HwSD_PORTCTRL_SLOT3(X)		((X) * Hw12)								// Port Select for SLOT 3 (X = 0 ~ 7)
#define HwSD_PORTCTRL_SLOT3_MASK	HwSD_PORTCTRL_SLOT3(15)
#define HwSD_PORTCTRL_SLOT2(X)		((X) * Hw8) 								// Port Select for SLOT 2 (X = 0 ~ 7)
#define HwSD_PORTCTRL_SLOT2_MASK	HwSD_PORTCTRL_SLOT2(15)
#define HwSD_PORTCTRL_SLOT1(X)		((X) * Hw4) 								// Port Select for SLOT 1 (X = 0 ~ 7)
#define HwSD_PORTCTRL_SLOT1_MASK	HwSD_PORTCTRL_SLOT1(15)
#define HwSD_PORTCTRL_SLOT0(X)		((X) * Hw0) 								// Port Select for SLOT 0 (X = 0 ~ 7)
#define HwSD_PORTCTRL_SLOT0_MASK	HwSD_PORTCTRL_SLOT0(15)

#define HwSD_STATE_SDWP 		Hw19
#define HwSD_STATE_NODAT		Hw1 		// data inhibit
#define HwSD_STATE_NOCMD		Hw0 		// command inhibit

#define HwSDINT_STATUS_ADMA		Hw25		// ADMA error
#define HwSDINT_STATUS_DATEND	Hw22		// data end bit error
#define HwSDINT_STATUS_DATCRC	Hw21		// data crc error
#define HwSDINT_STATUS_DATTIME	Hw20		// data timeout error
#define HwSDINT_STATUS_CINDEX	Hw19		// command index error
#define HwSDINT_STATUS_CMDEND	Hw18		// command command end bit error
#define HwSDINT_STATUS_CMDCRC	Hw17		// command crc error
#define HwSDINT_STATUS_CMDTIME	Hw16		// command timeout error
#define HwSDINT_STATUS_ERR		Hw15		// error interrupt
#define HwSDINT_STATUS_CDINT	Hw8 		// card interrupt
#define HwSDINT_STATUS_CDOUT	Hw7 		// card removal
#define HwSDINT_STATUS_CDIN 	Hw6 		// card insertion
#define HwSDINT_STATUS_RDRDY	Hw5 		// buffer read ready
#define HwSDINT_STATUS_WRRDY	Hw4 		// buffer write ready
#define HwSDINT_STATUS_DMA		Hw3 		// DMA interrupt
#define HwSDINT_STATUS_BLKGAP	Hw2 		// block gap event
#define HwSDINT_STATUS_TDONE	Hw1 		// transfer complete
#define HwSDINT_STATUS_CDONE	Hw0 		// command complete

#define HwSDINT_EN_ADMA 		Hw25		// ADMA error signal enable
#define HwSDINT_EN_ACMD12		Hw24		// auto CMD12 error signal enable
#define HwSDINT_EN_CLIMIT		Hw23		// current limit error signal enable
#define HwSDINT_EN_DATEND		Hw22		// data end bit error signal enable
#define HwSDINT_EN_DATCRC		Hw21		// data crc error signal enable
#define HwSDINT_EN_DATTIME		Hw20		// data timeout error signal enable
#define HwSDINT_EN_CINDEX		Hw19		// command index error signal enable
#define HwSDINT_EN_CMDEND		Hw18		// command end bit error signal enable
#define HwSDINT_EN_CMDCRC		Hw17		// command crc error signal enable
#define HwSDINT_EN_CMDTIME		Hw16		// command timeout error signal enable
#define HwSDINT_EN_CDINT		Hw8 		// card interrupt signal enable
#define HwSDINT_EN_CDOUT		Hw7 		// card removal signal enable
#define HwSDINT_EN_CDIN 		Hw6 		// card insertion signal enable
#define HwSDINT_EN_RDRDY		Hw5 		// buffer read ready signal enable
#define HwSDINT_EN_WRRDY		Hw4 		// buffer write ready signal enable
#define HwSDINT_EN_DMA			Hw3 		// DMA interrupt signal enable
#define HwSDINT_EN_BLKGAP		Hw2 		// block gap event signal enable
#define HwSDINT_EN_TDONE		Hw1 		// transfer complete signal enable
#define HwSDINT_EN_CDONE		Hw0 		// command complete signal enable


#define  HwSDINT_NORMAL_MASK	0x00007FFF
#define  HwSDINT_ERROR_MASK 0xFFFF8000

#define  HwSDINT_CMD_MASK	(HwSDINT_EN_CDONE | HwSDINT_EN_CMDTIME | \
		HwSDINT_EN_CMDCRC | HwSDINT_EN_CMDEND | HwSDINT_EN_CINDEX)
		
#define  HwSDINT_DATA_MASK	(HwSDINT_EN_TDONE | HwSDINT_EN_DMA | \
		HwSDINT_EN_RDRDY | HwSDINT_EN_WRRDY | \
		HwSDINT_EN_DATTIME | HwSDINT_EN_DATCRC | \
		HwSDINT_EN_DATEND| HwSDINT_EN_ADMA)

/*******************************************************************************
*	 6. NAND Flash Controller(NFC) Register Define   (Base Addr = 0xF05B0000)
********************************************************************************/
#define HwNFC_BASE                  *(volatile unsigned long *)0xF05B0000
#define HwNFC						((PNFC)&HwNFC_BASE)

// NFC Control Register
#define	HwNFC_CTRL_RDYIEN_EN		Hw31							// Nand Flash Ready Interrupt Enable
#define	HwNFC_CTRL_RDYIEN_DIS		~Hw31							// Nand Flash Ready Interrupt Disable
#define	HwNFC_CTRL_PROGIEN_EN		Hw30							// Nand Flash Program Interrupt Enable 
#define	HwNFC_CTRL_PROGIEN_DIS		~Hw30							// Nand Flash Program Interrupt Disable
#define	HwNFC_CTRL_READIEN_EN		Hw29							// Nand Flash Read Interrupt Enable
#define	HwNFC_CTRL_READIEN_DIS		~Hw29							// Nand Flash Read Interrupt Disable
#define	HwNFC_CTRL_DEN_EN			Hw28							// Nand Flash DMA Request Enable
#define	HwNFC_CTRL_DEN_DIS			~Hw28							// Nand Flash DMA Request Disable
#define	HwNFC_CTRL_FS_RDY			Hw27							// FIFO status is Ready to write and read in FIFO
#define	HwNFC_CTRL_FS_BUSY			~Hw27							// FIFO status is Busy to write and read in FIFO
#define	HwNFC_CTRL_BW_16			Hw26							// Bus width = 8bit
#define	HwNFC_CTRL_BW_8				HwZERO							// Bus width = 16bit
#define	HwNFC_CTRL_CS3SEL_1			Hw25							// Nand Flash nCS3 is High (Disabled)
#define	HwNFC_CTRL_CS3SEL_0			HwZERO							// Nand Flash nCS3 is Low (Enabled)
#define	HwNFC_CTRL_CS2SEL_1			Hw24							// Nand Flash nCS2 is High (Disabled)
#define	HwNFC_CTRL_CS2SEL_0			HwZERO							// Nand Flash nCS2 is Low (Enabled)
#define	HwNFC_CTRL_CS1SEL_1			Hw23							// Nand Flash nCS1 is High (Disabled)
#define	HwNFC_CTRL_CS1SEL_0			HwZERO							// Nand Flash nCS1 is Low (Enabled)
#define	HwNFC_CTRL_CS0SEL_1			Hw22							// Nand Flash nCS0 is High (Disabled)
#define	HwNFC_CTRL_CS0SEL_0			HwZERO							// Nand Flash nCS0 is Low (Enabled)
#define	HwNFC_CTRL_CFG_nCS3			HwNFC_CTRL_CS3SEL_1
#define	HwNFC_CTRL_CFG_nCS2			HwNFC_CTRL_CS2SEL_1
#define	HwNFC_CTRL_CFG_nCS1			HwNFC_CTRL_CS1SEL_1
#define	HwNFC_CTRL_CFG_nCS0			HwNFC_CTRL_CS0SEL_1
#define	HwNFC_CTRL_CSnSEL(X)		((X)*Hw22)						// Nand Flash nCS[3:0] Set
#define	HwNFC_CTRL_CFG_NOACT		HwNFC_CTRL_CSnSEL(15)
#define	HwNFC_CTRL_RDY_RDY			Hw21							// External Nand Flash Controller is Ready
#define	HwNFC_CTRL_RDY_BUSY			~Hw21							// External Nand Flash Controller is Busy
#define	HwNFC_CTRL_BSIZE(X)			((X)*Hw19)
#define	HwNFC_CTRL_BSIZE_1			HwNFC_CTRL_BSIZE(0)				// 1Read/Write
#define	HwNFC_CTRL_BSIZE_2			HwNFC_CTRL_BSIZE(1)				// 2Read/Write
#define	HwNFC_CTRL_BSIZE_4			HwNFC_CTRL_BSIZE(2)				// 4Read/Write
#define	HwNFC_CTRL_BSIZE_8			HwNFC_CTRL_BSIZE(3)				// 8Read/Write
#define	HwNFC_CTRL_BSIZE_MASK		HwNFC_CTRL_BSIZE(3)
#define	HwNFC_CTRL_PSIZE(X)			((X)*Hw16)
#define	HwNFC_CTRL_PSIZE_256		HwNFC_CTRL_PSIZE(0)				// 1 Page = 256 Half-Word
#define	HwNFC_CTRL_PSIZE_512		HwNFC_CTRL_PSIZE(1)				// 1 Page = 512 Byte
#define	HwNFC_CTRL_PSIZE_1024		HwNFC_CTRL_PSIZE(2)				// 1 Page = 1024 Half-Word
#define	HwNFC_CTRL_PSIZE_2048		HwNFC_CTRL_PSIZE(3)				// 1 Page = 2048 Byte
#define	HwNFC_CTRL_PSIZE_4096		HwNFC_CTRL_PSIZE(4)				// 1 Page = 4096 Byte
#define	HwNFC_CTRL_PSIZE_MASK		HwNFC_CTRL_PSIZE(7)
#define	HwNFC_CTRL_MASK_EN			Hw15							// Address/Command Mask Enable
#define	HwNFC_CTRL_CADDR			Hw12							// Number of Address Cycle
#define	HwNFC_CTRL_bSTP(X)			((X)*Hw8)						// Number of Base cycle for Setup Time
#define	HwNFC_CTRL_bSTP_MASK		HwNFC_CTRL_bSTP(15)
#define	HwNFC_CTRL_bPW(X)			((X)*Hw4)						// Number of Base cycle for Pulse Width
#define	HwNFC_CTRL_bPW_MASK			HwNFC_CTRL_bPW(15)
#define	HwNFC_CTRL_bHLD(X)			((X)*Hw0)						// Number of Base cycle for Hold Time
#define	HwNFC_CTRL_bHLD_MASK		HwNFC_CTRL_bHLD(15)

#define	HwNFC_IREQ_FLAG2			Hw6						//
#define	HwNFC_IREQ_FLAG1			Hw5						//
#define	HwNFC_IREQ_FLAG0			Hw4						//
#define	HwNFC_IREQ_IRQ2				Hw2						// Ready Interrupt
#define	HwNFC_IREQ_IRQ1				Hw1						// Program Interrupt
#define	HwNFC_IREQ_IRQ0				Hw0						// Reading Interrupt

/*******************************************************************************
*	 7. Static Memory Controller(SMC) Register Define   (Base Addr = 0xF05F0000)
********************************************************************************/
#define HwSMC_BASE                  *(volatile unsigned long *)0xF05F0000
#define HwSMC_STATUS                *(volatile unsigned long *)0xF05F0000 // R/W Unknown Status Register
#define HwSMC_CSNCFG0               *(volatile unsigned long *)0xF05F0020 // R 0x4b40_3183 External Chip Select0 Config Register
#define HwSMC_CSNCFG1               *(volatile unsigned long *)0xF05F0024 // R/W 0x4b40_1104 External Chip Select1 Config Register
#define HwSMC_CSNCFG2               *(volatile unsigned long *)0xF05F0028 // W 0x4b40_4082 External Chip Select2 Config Register
#define HwSMC_CSNCFG3               *(volatile unsigned long *)0xF05F002C // R/W 0x4b40_20C5 External Chip Select3 Config. Register
#define HwSMC_CSNOFFSET             *(volatile unsigned long *)0xF05F0030 // R/W 0x0 Wapping Address Mode OFFSET Register
#define HwSMC_INDIRADDR             *(volatile unsigned long *)0xF05F0034 // R/W 0x0 Indirect Address

/*******************************************************************************
*	 8. External Device Interface (EDI) Register Define   (Base Addr = 0xF05F6000)
********************************************************************************/
#define HwEDI_BASE                  *(volatile unsigned long *)0xF05F6000
#define HwEDI						((PEDI)&HwEDI_BASE)

/*******************************************************************************
*	 9. IDE Controller Register Define   (Base Addr = 0xF0520000)
********************************************************************************/
#define HwIDE_BASE                  *(volatile unsigned long *)0xF0520000

/*******************************************************************************
*	 10. SATA Interface Register Define   (Base Addr = 0xF0560000)
********************************************************************************/
#define HwSATA_BASE                 *(volatile unsigned long *)0xF0560000
//SCR5-SCR15 0x38-0x60 32 See description 0x0 Reserved for SATA Dependencies: Reads to these locations return zeros; writes have no effect

/*******************************************************************************
*	 11-1. Audio DMA Controller Register Define   (Base Addr = 0xF0533000)
********************************************************************************/
#define HwADMA_BASE                 *(volatile unsigned long *)0xF0533000

/*******************************************************************************
*	 11-2. DAI Register Define   (Base Addr = 0xF0534000)
********************************************************************************/
#define HwADMA_DAIBASE              *(volatile unsigned long *)0xF0534000

/*******************************************************************************
*	 11-3. CDIF Register Define   (Base Addr = 0xF0534000)
********************************************************************************/
#define HwADMA_CDIFBASE             *(volatile unsigned long *)0xF0534080

/*******************************************************************************
*	 11-4. SPDIF Register Define   (Base Addr = 0xF0535000/0xF0535800)
********************************************************************************/
#define HwADMA_SPDIFTXBASE          *(volatile unsigned long *)0xF0535000

/*******************************************************************************
*	 12-1. DAI Register Define   (Base Addr = 0xF0537000
********************************************************************************/
#define HwDAI_BASE                  *(volatile unsigned long *)0xF0537000

/*******************************************************************************
*	 12-2. CDIF Register Define   (Base Addr = 0xF0537000
********************************************************************************/
#define HwCDIF_BASE                 *(volatile unsigned long *)0xF0537080

/*******************************************************************************
*	 13. SPDIF Register Define   (Base Addr = 0xF0538000)
********************************************************************************/
#define HwSPDIF_BASE                *(volatile unsigned long *)0xF0538000

/*******************************************************************************
*	 14-1. USB1.1 HOST Controller & Transceiver       (Base Addr = 0xF0500000)
********************************************************************************/
#define HwUSBHOST_BASE              *(volatile unsigned long *)0xF0500000

/*******************************************************************************
*	 14-2 USB1.1 HOST Configuration Register        (Base Addr = 0xF05F5000)
********************************************************************************/
#define HwUSBHOSTCFG_BASE           *(volatile unsigned long *)0xF05F5000

/*******************************************************************************
*	 15-1. USB2.0 OTG Controller Define   (Base Addr = 0xF0550000)
********************************************************************************/
#define HwUSB20OTG_BASE             *(volatile unsigned long *)0xF0550000

/*******************************************************************************
*	 15-2. USB OTG Configuration Register Define   (Base Addr = 0xF05F5000)
********************************************************************************/
#define HwUSBOTGCFG_BASE            *(volatile unsigned long *)0xF05F5000

/*******************************************************************************
*	 15-3. USB PHY Configuration Register Define   (Base Addr = 0xF05F5028)
********************************************************************************/
#define HwUSBPHYCFG_BASE            *(volatile unsigned long *)0xF05F5028

/*******************************************************************************
*	 16. External Host Interface Register Define   (Base Addr = 0xF0570000/0xF0580000)
********************************************************************************/
#define HwEHICS0_BASE               *(volatile unsigned long *)0xF0570000
#define HwEHICS1_BASE               *(volatile unsigned long *)0xF0580000

/*******************************************************************************
*	 17. General Purpose Serial Bus (GPSB) Register Define   (Base Addr = 0xF0538000)
********************************************************************************/
#if 0
#define HwGPSBCH0_BASE              *(volatile unsigned long *)0xF0057000
#define HwGPSBCH1_BASE              *(volatile unsigned long *)0xF0057100
#define HwGPSBCH2_BASE              *(volatile unsigned long *)0xF0057200
#define HwGPSBCH3_BASE              *(volatile unsigned long *)0xF0057300
#define HwGPSBCH4_BASE              *(volatile unsigned long *)0xF0057400
#define HwGPSBCH5_BASE              *(volatile unsigned long *)0xF0057500
#define HwGPSBPORTCFG_BASE          *(volatile unsigned long *)0xF0057800
#define HwGPSBPIDTABLE_BASE         *(volatile unsigned long *)0xF0057F00

#define	HwGPSB_PIDT(X)				*(volatile unsigned long *)(0xF0057F00+(X)*4)	// R/W, PID Table Register
#define	HwGPSB_PIDT_CH2				Hw31											// Channel 2 enable
#define	HwGPSB_PIDT_CH1				Hw30											// Channel 1 enable
#define	HwGPSB_PIDT_CH0				Hw29											// Channel 0 enable
#else
#define HwGPSBCH0_BASE              *(volatile unsigned long *)0xF0536000
#define HwGPSBCH1_BASE              *(volatile unsigned long *)0xF0536100
#define HwGPSBCH2_BASE              *(volatile unsigned long *)0xF0536200
#define HwGPSBCH3_BASE              *(volatile unsigned long *)0xF0536300
#define HwGPSBCH4_BASE              *(volatile unsigned long *)0xF0536400
#define HwGPSBCH5_BASE              *(volatile unsigned long *)0xF0536500
#define HwGPSBPORTCFG_BASE          *(volatile unsigned long *)0xF0536800
#define HwGPSBPIDTABLE_BASE         *(volatile unsigned long *)0xF0536F00

#define	HwGPSB_BASE(X)				*(volatile unsigned long *)(0xF0536000+(X)*0x100)
#define	HwGPSB_MAX_CH				4
// Status
#define	HwGPSB_STAT_WOR				Hw8											// Write FIFO over-run error flag
#define	HwGPSB_STAT_RUR				Hw7											// Read FIFO under-run error flag
#define	HwGPSB_STAT_WUR				Hw6											// Write FIFO under-run error flag
#define	HwGPSB_STAT_ROR				Hw5											// Read FIFO over-run error flag
#define	HwGPSB_STAT_RF				Hw4											// Read FIFO full flag
#define	HwGPSB_STAT_WE				Hw3											// Write FIFO empty flag
#define	HwGPSB_STAT_RNE				Hw2											// Read FIFO not empty flag
#define	HwGPSB_STAT_WTH				Hw1											// Wrtie FIFO valid entry count is under threshold
#define	HwGPSB_STAT_RTH				Hw0											// Read FIFO valid entry increased over threshold

#define	HwGPSB_PIDT(X)				*(volatile unsigned long *)(0xF0536F00+(X)*4)	// R/W, PID Table Register
#define	HwGPSB_PIDT_CH2				Hw31											// Channel 2 enable
#define	HwGPSB_PIDT_CH1				Hw30											// Channel 1 enable
#define	HwGPSB_PIDT_CH0				Hw29											// Channel 0 enable

#define	HwGPSB_DMAICR_ISD			Hw29										// IRQ Status for "Done Interrupt"
#define	HwGPSB_DMAICR_ISP			Hw28										// IRQ Status for "Packet Interrupt"
#define	HwGPSB_DMAICR_IRQS_TRANS	Hw20										// IRQ is generated when transmit is done.
#define	HwGPSB_DMAICR_IRQS_RCV		HwZERO										// IRQ is generated when receiving is done.
#define	HwGPSB_DMAICR_IED_EN		Hw17										// Enable "Done Interrupt"
#define	HwGPSB_DMAICR_IEP_EN		Hw16										// Enable "Packet Interrupt"
#define	HwGPSB_DMAICR_IRQPCNT_MASK	(Hw13-1)

// DMA Control
#define	HwGPSB_DMACTRL_DTE			Hw31										// Transmit DMA request enable
#define	HwGPSB_DMACTRL_DRE			Hw30										// Receive DMA request enable
#define	HwGPSB_DMACTRL_CT			Hw29										// Continuous mode enable
#define	HwGPSB_DMACTRL_END			Hw28										// Byte endian mode register
#define	HwGPSB_DMACTRL_MP			Hw19										// PID match mode register
#define	HwGPSB_DMACTRL_MS			Hw18										// Sync byte match control register
#define	HwGPSB_DMACTRL_TXAM(X)		((X)*Hw16)
#define	HwGPSB_DMACTRL_TXAM_MULTI	HwGPSB_DMACTRL_TXAM(0)						// TX address is rolling within full packet range
#define	HwGPSB_DMACTRL_TXAM_FIXED	HwGPSB_DMACTRL_TXAM(1)						// TX address is fixed to TXBASE
#define	HwGPSB_DMACTRL_TXAM_SINGLE	HwGPSB_DMACTRL_TXAM(2)						// TX address is rolling within 1 packet range
#define	HwGPSB_DMACTRL_TXAM_MASK	HwGPSB_DMACTRL_TXAM(3)
#define	HwGPSB_DMACTRL_RXAM(X)		((X)*Hw14)
#define	HwGPSB_DMACTRL_RXAM_MULTI	HwGPSB_DMACTRL_RXAM(0)						// RX address is rolling within full packet range
#define	HwGPSB_DMACTRL_RXAM_FIXED	HwGPSB_DMACTRL_RXAM(1)						// RX address is fixed to RXBASE
#define	HwGPSB_DMACTRL_RXAM_SINGLE	HwGPSB_DMACTRL_RXAM(2)						// RX address is rolling within 1 packet range
#define	HwGPSB_DMACTRL_RXAM_MASK	HwGPSB_DMACTRL_RXAM(3)
#define	HwGPSB_DMACTRL_MD_NOR		HwZERO										// normal mode
#define	HwGPSB_DMACTRL_MD_MP2TS		Hw4											// MPEG2-TS mode
#define	HwGPSB_DMACTRL_PCLR			Hw2											// Clear TX/RX Packet Counter
#define	HwGPSB_DMACTRL_EN			Hw0											// DMA enable

// Interrupt Enable
#define	HwGPSB_INTEN_DW				Hw31										// DMA request enable for transmit FIFO
#define	HwGPSB_INTEN_DR				Hw30										// DMA request enable for receive FIFO
#define	HwGPSB_INTEN_SWD_BHW		Hw27										// Swap byte in half-word
#define	HwGPSB_INTEN_SWD_HWW		Hw26										// Swap half-word in word
#define	HwGPSB_INTEN_SRD_BHW		Hw25										// Swap byte in half-word
#define	HwGPSB_INTEN_SRD_HWW		Hw24										// Swap half-word in word
#define	HwGPSB_INTEN_CFGWTH(X)		((X)*Hw20)									// Write FIFO threshold for Interrupt or DMA Request
#define	HwGPSB_INTEN_CFGWTH_MASK	HwGPSB_INTEN_CFGWTH(7)
#define	HwGPSB_INTEN_CFGRTH(X)		((X)*Hw16)									// Read FIFO threshold for Interrupt or DMA Request
#define	HwGPSB_INTEN_CFGRTH_MASK	HwGPSB_INTEN_CFGRTH(7)
#define	HwGPSB_INTEN_RC				Hw15										// Clear status[8:0] at the end of read cycle
#define	HwGPSB_INTEN_WOR			Hw8											// Write FIFO over-run error interrupt enable
#define	HwGPSB_INTEN_RUR			Hw7											//Read FIFO under-run error flag interrupt enable
#define	HwGPSB_INTEN_WUR			Hw6											// Write FIFO under-run error flag interrupt enable
#define	HwGPSB_INTEN_ROR			Hw5											// Read FIFO over-run error flag interrupt enable
#define	HwGPSB_INTEN_RF				Hw4											// Read FIFO full flag interrupt enable
#define	HwGPSB_INTEN_WE				Hw3											// Write FIFO empty flag interrupt enable
#define	HwGPSB_INTEN_RNE			Hw2											// Read FIFO not empty flag interrupt enable
#define	HwGPSB_INTEN_WTH			Hw1											// Wrtie FIFO valid entry count is under threshold interrupt enable
#define	HwGPSB_INTEN_RTH			Hw0											// Read FIFO valid entry increased over threshold interrupt enable


// Mode
#define	HwGPSB_MODE_DIVLDV(X)		((X)*Hw24)
#define	HwGPSB_MODE_DIVLDV_MASK		HwGPSB_MODE_DIVLDV(255)
#define	HwGPSB_MODE_TRE				Hw23										// Master recovery time (TRE+1)*SCKO
#define	HwGPSB_MODE_THL				Hw22										// Master hold time (THL+1)*SCKO
#define	HwGPSB_MODE_TSU				Hw21										// Master setup time (TSU+1)*SCKO
#define	HwGPSB_MODE_PCS				Hw20										// Polarity control for CS(FRM) - Master Only
#define	HwGPSB_MODE_PCS_HIGH		Hw20
#define	HwGPSB_MODE_PCD				Hw19										// Polarity control for CMD(FRM)- Master Only
#define	HwGPSB_MODE_PCD_HIGH		Hw19
#define	HwGPSB_MODE_PWD				Hw18										// Polarity control for transmit data - Master Only
#define	HwGPSB_MODE_PWD_RISE		Hw18
#define	HwGPSB_MODE_PRD				Hw17										// Polarity control for receive data - Master Only
#define	HwGPSB_MODE_PRD_FALL		Hw17
#define	HwGPSB_MODE_PCK				Hw16										// Polarity control for serial clock
#define	HwGPSB_MODE_CRF				Hw15										// Clear receive FIFO counter
#define	HwGPSB_MODE_CWF				Hw14										// Clear transmit FIFO counter
#define	HwGPSB_MODE_BWS(X)			((X)*Hw8)									// Bit width Selection.(BWS+1. Valid = 7~31)
#define	HwGPSB_MODE_BWS_MASK		HwGPSB_MODE_BWS(31)
#define	HwGPSB_MODE_SD				Hw7											// Data shift direction control
#define	HwGPSB_MODE_LB				Hw6											// Data looop-back enable
#define	HwGPSB_MODE_CTF				Hw4											// Continuous transfer mode enable
#define	HwGPSB_MODE_EN				Hw3											// Operation enable
#define	HwGPSB_MODE_SLV				Hw2											// Slave mode configuration
#define	HwGPSB_MODE_MD_SPI			HwZERO										// SPI compatible
#define	HwGPSB_MODE_MD_SSP			Hw0											// SSP Compatible

#endif

/*******************************************************************************
*	 18. The Transport Stream Interface (TSIF) Register Define   (Base Addr = 0xF0538000)
********************************************************************************/
#define HwTSIF_BASE                 *(volatile unsigned long *)0xF053B000
#define HwTSIFPORTSEL_BASE          *(volatile unsigned long *)0xF053B800

/*******************************************************************************
*	 19. GPS Interface Register Define   (Base Addr = )
********************************************************************************/


/*******************************************************************************
*	 20. Remote Control Interface Register Define   (Base Addr = 0xF05F3000)
********************************************************************************/
#define HwREMOTE_BASE				*(volatile unsigned long *)0xF05F3000


/*******************************************************************************
*	 21. I2C Controller Register Define   (Base Addr = 0xF0530000)
********************************************************************************/
#define HwI2CMASTER0_BASE          *(volatile unsigned long *)0xF0530000
#define HwI2CMASTER1_BASE          *(volatile unsigned long *)0xF0530040

#define HwI2CSLAVE_BASE            *(volatile unsigned long *)0xF0530080
#define HwI2CSTATUS_BASE           *(volatile unsigned long *)0xF05300C0

#define HwI2CMASTER0			((PSMUI2CMASTER)&HwI2CMASTER0_BASE)
#define HwI2CMASTER1			((PSMUI2CMASTER)&HwI2CMASTER1_BASE)

#define HwSMUI2C_MASTER0_BASE		*(volatile unsigned long *)(0xF0405000)
#define HwSMUI2C_MASTER1_BASE		*(volatile unsigned long *)(0xF0405040)

#define HwSMUI2C_COMMON_BASE		 *(volatile unsigned long *)0xF0405080	// SMU_I2C Common Registers

/*******************************************************************************
*	 22. UART Controller Register Define   (Base Addr = 0xF0538000)
********************************************************************************/
#define HwUARTCH0_BASE             *(volatile unsigned long *)0xF0532000
#define HwUARTCH1_BASE             *(volatile unsigned long *)0xF0532100
#define HwUARTCH2_BASE             *(volatile unsigned long *)0xF0532200
#define HwUARTCH3_BASE             *(volatile unsigned long *)0xF0532300
#define HwUARTCH4_BASE             *(volatile unsigned long *)0xF0532400
#define HwUARTCH5_BASE             *(volatile unsigned long *)0xF0532500
#define HwUARTPORTMUX_BASE         *(volatile unsigned long *)0xF0532600
#define HwUART0						((PUART)&HwUARTCH0_BASE)
#define HwUART1						((PUART)&HwUARTCH1_BASE)
#define HwUART2						((PUART)&HwUARTCH2_BASE)
#define HwUART3						((PUART)&HwUARTCH3_BASE)
#define HwUART4						((PUART)&HwUARTCH4_BASE)
#define HwUART5						((PUART)&HwUARTCH5_BASE)

#define HwUART_BASE(X)				*(volatile unsigned long *)(0xF0532000+(X)*0x100)

#define	HwUART_CHSEL				*(volatile unsigned long *)0xF0532600	// R/W, Channel Selection Register
#define HwUART_CHSEL_MASK(X)		(7<<((X)*4))							// X: Channel Number
#define HwUART_CHSEL_SEL_PORT(X,Y)	((Y)<<((X)*4))							// X: Channel Number, Y: Port Numbert

/*******************************************************************************
*	 23. CAN Controller Register Define   (Base Addr = 0xF0531000)
********************************************************************************/
#define HwCAN_BASE                 *(volatile unsigned long *)0xF0531000

/*******************************************************************************
*	 24. DMA Controller Register Define   (Base Addr = 0xF0540000)
********************************************************************************/
#define HwGDMA0_BASE               *(volatile unsigned long *)0xF0540000
#define HwGDMA1_BASE               *(volatile unsigned long *)0xF0540100
#define HwGDMA2_BASE               *(volatile unsigned long *)0xF0540200
#define HwGDMA3_BASE               *(volatile unsigned long *)0xF0540300

/*******************************************************************************
*	 25. Real Time Clock(RTC) Register Define   (Base Addr = 0xF05F2000)
********************************************************************************/
#define HwRTC_BASE                 *(volatile unsigned long *)0xF05F2000

/*******************************************************************************
*	 26. TouchScreen ADC (TSADC) Register Define   (Base Addr = 0xF05F4000)
********************************************************************************/
#define HwTSADC_BASE               *(volatile unsigned long *)0xF05F4000
#define HwTSADC					((PTSADC)&HwTSADC_BASE)
// Control
#define HwADCCON_RES_12				Hw16										// 12 bits A/D Conversion
#define HwADCCON_RES_10				HwZERO										// 10 bits A/D Conversion
#define HwADCCON_EFLAG				Hw15										// End of Conversion(EOC) flag
#define HwADCCON_PS_EN				Hw14										// Enable Prescaler
#define HwADCCON_PS_VAL(X)			((X)*Hw6)									// Value of Prescaler (4~255)
#define HwADCCON_PS_VAL_MASK		HwADCCON_PS_VAL(0xFF)
#define	HwADCCON_ASEL(X)			((X)*Hw3)
#define	HwADCCON_ASEL_CH0			HwADCCON_ASEL(0)
#define	HwADCCON_ASEL_CH1			HwADCCON_ASEL(1)
#define	HwADCCON_ASEL_CH2			HwADCCON_ASEL(2)
#define	HwADCCON_ASEL_CH3			HwADCCON_ASEL(3)
#define	HwADCCON_ASEL_CH4			HwADCCON_ASEL(4)
#define	HwADCCON_ASEL_CH5			HwADCCON_ASEL(5)
#define	HwADCCON_ASEL_CH6			HwADCCON_ASEL(6)
#define	HwADCCON_ASEL_CH7			HwADCCON_ASEL(7)
#define	HwADCCON_ASEL_MASK			HwADCCON_ASEL(7)
#define	HwADCCON_STBY				Hw2											// Stand-By mode selection
#define	HwADCCON_RD_ST				Hw1											// A/D Conversion Start by data read
#define HwADCCON_EN_ST				Hw0											// A/D Conversion Start by Enable signal

// Touch Screen
#define HwADCTSC_DET_DOWN			Hw8											// Detect Stylus-DOWN status
#define HwADCTSC_DET_UP				HwZERO										// Detect Stylus-UP status
#define HwADCTSC_YMEN				Hw7											// YM = VSSA (GND)
#define HwADCTSC_YMDIS				HwZERO										// YM = AIN4, Hi-z
#define HwADCTSC_YPEN				HwZERO										// YP = VDDA
#define HwADCTSC_YPDIS				Hw6											// YP = AIN5, Hi-z
#define HwADCTSC_XMEN				Hw5											// XM = VSSA (GND)
#define HwADCTSC_XMDIS				HwZERO										// XM = AIN6, Hi-z
#define HwADCTSC_XPEN				HwZERO										// XP = VDDA
#define HwADCTSC_XPDIS				Hw4											// XP = AIN7, Hi-z
#define HwADCTSC_PU_EN				HwZERO										// XP Pull-up Enable
#define HwADCTSC_PU_DIS				Hw3											// XP Pull-up Disable
#define HwADCTSC_AUTO_CONV			Hw2											// Automatic(sequencial) conversion of X-pos and Y-pos
#define HwADCTSC_SEL_NONE			HwZERO										// No operation
#define HwADCTSC_SEL_X				Hw0											// X-pos conversion
#define HwADCTSC_SEL_Y				Hw1											// Y-pos conversion
#define HwADCTSC_SEL_WAIT			(Hw0|Hw1)									// Waiting interrupt mode
#define HwADCTSC_SEL_MASK			(Hw0|Hw1)

// ADC Delay
#define HwADCDLY_SEL(X)				(0xFFFF&X)

// Touch UP/DOWN Interrupt
#define HwADCTSC_INT_UP				Hw1
#define HwADCTSC_INT_DOWN			Hw0







/*******************************************************************************
*	 27. Error Correction Code Register Define   (Base Addr = 0xF0539000)
********************************************************************************/
#define HwECC_BASE                 *(volatile unsigned long *)0xF0539000
#define HwECCERRADDR_BASE          *(volatile unsigned long *)0xF0539050
#define HwECC						((PECC)&HwECC_BASE)
#define HwECC_EADDR					((PECCERRADDR)&HwECCERRADDR_BASE)

// ECC Control
#define	HwECC_CTRL_IEN_MECC16_EN	Hw20										// MLC ECC16 Decoding Interrupt Enable
#define	HwECC_CTRL_IEN_MECC16_DIS	~Hw20										// MLC ECC16 Decoding Interrupt Disable
#define	HwECC_CTRL_IEN_MECC14_EN	Hw19										// MLC ECC14 Decoding Interrupt Enable
#define	HwECC_CTRL_IEN_MECC14_DIS	~Hw19										// MLC ECC14 Decoding Interrupt Disable
#define	HwECC_CTRL_IEN_MECC12_EN	Hw18										// MLC ECC12 Decoding Interrupt Enable
#define	HwECC_CTRL_IEN_MECC12_DIS	~Hw18										// MLC ECC12 Decoding Interrupt Disable
#define	HwECC_CTRL_IEN_MECC8_EN		Hw17										// MLC ECC8 Decoding Interrupt Enable
#define	HwECC_CTRL_IEN_MECC8_DIS	~Hw17										// MLC ECC8 Decoding Interrupt Disable
#define	HwECC_CTRL_IEN_MECC4_EN		Hw16										// MLC ECC4 Decoding Interrupt Enable
#define	HwECC_CTRL_IEN_MECC4_DIS	~Hw16										// MLC ECC4 Decoding Interrupt Disable

// ECC Disable
#define	HwECC_CTRL_EN_SLCEN			Hw2											// SLC ECC Encoding Enable
#define	HwECC_CTRL_EN_SLCDE			(Hw2|Hw0)									// SLC ECC Decoding Enable
#define	HwECC_CTRL_EN_MCL4EN		(Hw2|Hw1)									// MLC ECC4 Encoding Enable
#define	HwECC_CTRL_EN_MCL4DE		(Hw2|Hw1|Hw0)								// MLC ECC4 Decoding Enable
#define	HwECC_CTRL_EN_MCL8EN		(Hw3)										// MLC ECC8 Encoding Enable
#define	HwECC_CTRL_EN_MCL8DE		(Hw3|Hw0)									// MLC ECC8 Decoding Enable
#define	HwECC_CTRL_EN_MCL12EN		(Hw3|Hw1)									// MLC ECC12 Encoding Enable
#define	HwECC_CTRL_EN_MCL12DE		(Hw3|Hw1|Hw0)								// MLC ECC12 Decoding Enable
#define	HwECC_CTRL_EN_MCL14EN		(Hw3|Hw2)									// MLC ECC14 Encoding Enable
#define	HwECC_CTRL_EN_MCL14DE		(Hw3|Hw2|Hw0)								// MLC ECC14 Decoding Enable
#define	HwECC_CTRL_EN_MCL16EN		(Hw3|Hw2|Hw1)								// MLC ECC16 Encoding Enable
#define	HwECC_CTRL_EN_MCL16DE		(Hw3|Hw2|Hw1|Hw0)							// MLC ECC16 Decoding Enable
#define	HwECC_CTRL_EN_DIS			~(Hw3|Hw2|Hw1|Hw0)							// ECC Disable

// ECC Error Number
#define	HwERR_NUM_ERR1				Hw0											// Correctable Error(SLC), Error Occurred(MLC3), 1 Error Occurred(MLC4)
#define	HwERR_NUM_ERR2				Hw1											// 2 Error Occurred(MLC4)
#define	HwERR_NUM_ERR3				(Hw1|Hw0)									// 3 Error Occurred(MLC4)
#define	HwERR_NUM_ERR4				Hw2											// 4 Error Occurred(MLC4)
#define	HwERR_NUM_ERR5				(Hw2|Hw0)									// 5 Error Occurred(MLC4)
#define	HwERR_NUM_ERR6				(Hw2|Hw1)									// 5 Error Occurred(MLC4)
#define	HwERR_NUM_ERR7				(Hw2|Hw1|Hw0)								// 5 Error Occurred(MLC4)
#define	HwERR_NUM_ERR8				Hw3											// 5 Error Occurred(MLC4)
#define	HwERR_NUM_ERR9				(Hw3|Hw0)									// 5 Error Occurred(MLC4)
#define	HwERR_NUM_ERR10				(Hw3|Hw1)									// 5 Error Occurred(MLC4)
#define	HwERR_NUM_ERR11				(Hw3|Hw1|Hw0)								// 5 Error Occurred(MLC4)
#define	HwERR_NUM_ERR12				(Hw3|Hw2)									// 5 Error Occurred(MLC4)
#define	HwERR_NUM_ERR13				(Hw3|Hw2|Hw0)								// 5 Error Occurred(MLC4)
#define	HwERR_NUM_ERR14				(Hw3|Hw2|Hw1)								// 5 Error Occurred(MLC4)
#define	HwERR_NUM_ERR15				(Hw3|Hw2|Hw1|Hw0)							// 5 Error Occurred(MLC4)
#define	HwERR_NUM_ERR16				Hw4											// 5 Error Occurred(MLC4)
#define	HwERR_NUM_NOERR				HwZERO										// No Error
#define	HwERR_NUM_CORIMP			(Hw1|Hw0)									// Correction Impossible(SLC, MLC4)

// ECC Interrupt Control
#define	HwECC_IREQ_SEF				Hw17										// SLC ECC Encoding Flag Register
#define	HwECC_IREQ_SDF				Hw16										// SLC ECC Decoding Flag Register
#define	HwECC_IREQ_M4EF				Hw19										// MLC ECC4 Encoding Flag Register
#define	HwECC_IREQ_M4DF				Hw18										// MLC ECC4 Decoding Flag Register
#define	HwECC_IREQ_M8EF				Hw21										// MLC ECC8 Encoding Flag Register
#define	HwECC_IREQ_M8DF				Hw20										// MLC ECC8 Decoding Flag Register
#define	HwECC_IREQ_M12EF			Hw23										// MLC ECC12 Encoding Flag Register
#define	HwECC_IREQ_M12DF			Hw22										// MLC ECC12 Decoding Flag Register
#define	HwECC_IREQ_M14EF			Hw25										// MLC ECC14 Encoding Flag Register
#define	HwECC_IREQ_M14DF			Hw24										// MLC ECC14 Decoding Flag Register
#define	HwECC_IREQ_M16EF			Hw27										// MLC ECC16 Encoding Flag Register
#define	HwECC_IREQ_M16DF			Hw26										// MLC ECC16 Decoding Flag Register
#define	HwECC_IREQ_M4DI				Hw2											// MLC ECC4 Decoding Interrupt Request Register
#define	HwECC_IREQ_M8DI				Hw4											// MLC ECC8 Decoding Interrupt Request Register
#define	HwECC_IREQ_M12DI			Hw6											// MLC ECC12 Decoding Interrupt Request Register
#define	HwECC_IREQ_M14DI			Hw8											// MLC ECC14 Decoding Interrupt Request Register
#define	HwECC_IREQ_M16DI			Hw10										// MLC ECC16 Decoding Interrupt Request Register
#define	HwECC_IREQ_CLR				(Hw27|Hw26|Hw25|Hw24|Hw23|Hw22|Hw21|Hw20|Hw19|Hw18|Hw17|Hw16|Hw10|Hw8|Hw6|Hw4|Hw2)

/*******************************************************************************
*	 28. Multi-Protocol Encapsulation Forward Error Correction (MPEFEC)
*    Register Define   (Base Addr = 0xF0510000)
********************************************************************************/
#define HwMPEFEC_BASE              *(volatile unsigned long *)0xF0510000

/*******************************************************************************
*	 29. IOBUS Configuration Register Define   (Base Addr = 0xF05F5000)
********************************************************************************/
#define HwIOBUSCFG_BASE            *(volatile unsigned long *)0xF05F5000
#define HwIOBUSCFG					((PIOBUSCFG)&HwIOBUSCFG_BASE)

// IOBUS AHB 0
#define HwIOBUSCFG_USB				Hw1											// USB2.0 OTG
#define HwIOBUSCFG_IDE				Hw2											// IDE Controller
#define HwIOBUSCFG_DMA				Hw3											// DMA Controller
#define HwIOBUSCFG_SD				Hw4											// SD/MMC Controller
#define HwIOBUSCFG_MS				Hw6											// Memory Stick Controller
#define HwIOBUSCFG_I2C				Hw7											// I2C Controller
#define HwIOBUSCFG_NFC				Hw8											// NFC Controller
#define HwIOBUSCFG_EHI0				Hw9											// External Host Interface 0
#define HwIOBUSCFG_EHI1				Hw10										// External Host Interface 1
#define HwIOBUSCFG_UART0			Hw11										// UART Controller 0
#define HwIOBUSCFG_UART1			Hw12										// UART Controller 1
#define HwIOBUSCFG_UART2			Hw13										// UART Controller 2
#define HwIOBUSCFG_UART3			Hw14										// UART Controller 3
#define HwIOBUSCFG_UART4			Hw15										// UART Controller 4
#define HwIOBUSCFG_UART5			Hw16										// UART Controller 5
#define HwIOBUSCFG_GPSB0			Hw17										// GPSB Controller 0
#define HwIOBUSCFG_GPSB1			Hw18										// GPSB Controller 1
#define HwIOBUSCFG_GPSB2			Hw19										// GPSB Controller 2
#define HwIOBUSCFG_GPSB3			Hw20										// GPSB Controller 3
#define HwIOBUSCFG_GPSB4			Hw21										// GPSB Controller 4
#define HwIOBUSCFG_GPSB5			Hw22										// GPSB Controller 5
#define HwIOBUSCFG_DAI				Hw23										// DAI/CDIF Interface
#define HwIOBUSCFG_ECC				Hw24										// ECC Calculator
#define HwIOBUSCFG_SPDIF			Hw25										// SPDIF Tx Controller
#define HwIOBUSCFG_RTC				Hw26										// RTC
#define HwIOBUSCFG_TSADC			Hw27										// TSADC Controller
#define HwIOBUSCFG_GPS				Hw28										// GPS Interface
#define HwIOBUSCFG_ADMA				Hw31										// Audio DMA Controller

// IOBUS AHB 1
#define HwIOBUSCFG_MPE				Hw0											// MPE_FEC
#define HwIOBUSCFG_TSIF				Hw1											// TSIF
#define HwIOBUSCFG_SRAM				Hw2											// SRAM Controller

#define	HwIOBUSCFG_STORAGE_ECC		~(Hw17|Hw16)							// Storage Bus
#define	HwIOBUSCFG_STORAGE_AHB_BUS1	Hw16							// I/O bus
#define	HwIOBUSCFG_STORAGE_AHB_BUS2	Hw17							// General purpose SRAM or DTCM
#define	HwIOBUSCFG_STORAGE_NFC		(Hw17|Hw16)						// Main processor data bus

/************************************************************************
*	Channel 0 Memory Controller Register Define	(Base Addr = 0xF1000000)
************************************************************************/
#define	HwEMC_BASE				   *(volatile unsigned long *)0xF1000000	// External Memory Controller Base Register

/*******************************************************************************
*	 TCC9200_DataSheet_PART 6 DDI_BUS_V0.00 Dec.11 2008
********************************************************************************/
/************************************************************************
*	4. LCD INTERFACE Register Define				(Base Addr = 0xF0200000)
************************************************************************/
#define	HwLCDC0_BASE				*(volatile unsigned long *)0xF0200000	// LCDC0 Control Base Register
#define HwLCDLUT0_BASE				*(volatile unsigned long *)0xF0200400	// LCD LUT 0 Base Register
#define	HwLCDC1_BASE				*(volatile unsigned long *)0xF0204000	// LCDC1 Control Base Register
#define HwLCDLUT1_BASE				*(volatile unsigned long *)0xF0204400	// LCD LUT 1 Base Register

#define HwLCDC0_CH_BASE(X)			*(volatile unsigned long *)(0xF0200000 + 0x80 + (0x38 * X))
#define HwLCDC1_CH_BASE(X)			*(volatile unsigned long *)(0xF0204000 + 0x80 + (0x38 * X))


#define HwLCD						((PLCDC)&HwLCDC0_BASE)
#define HwLCD1						((PLCDC)&HwLCDC1_BASE)

#define HwLCD						((PLCDC)&HwLCDC0_BASE)
#define HwLCD1						((PLCDC)&HwLCDC1_BASE)

// LCD Control
#define	HwLCTRL_EVP					Hw31										// External Vsync Polarity
#define	HwLCTRL_EVS					Hw30										// External Vsync Enable
#define	HwLCTRL_R2YMD				(Hw29+Hw28+Hw27+Hw26)						// RGB to YCbCr Conversion Option
#define	HwLCTRL_GEN					Hw25										// Gamma Correction Enable Bit
#define	HwLCTRL_656					Hw24										// CCIR 656 Mode
#define	HwLCTRL_CKG					Hw23										// Clock Gating Enable for Timing Generator
#define	HwLCTRL_BPP					(Hw22+Hw21+Hw20)							// Bit Per Pixel for STN_LCD
#define	HwLCTRL_PXDW				(Hw19+Hw18+Hw17+Hw16)						// PXDW
#define	HwLCTRL_ID					Hw15										// Inverted Data Enable
#define	HwLCTRL_IV					Hw14										// Inverted Vertical Sync
#define	HwLCTRL_IH					Hw13										// Inverted Horizontal Sync
#define	HwLCTRL_IP					Hw12										// Inverted Pixel Clock
#define	HwLCTRL_CLEN				Hw11										// Clipping Enable
#define	HwLCTRL_DP					Hw10										// Double Pixel Data
#define	HwLCTRL_R2Y					Hw9											// RGB to YCbCr Converter Enable for Output 
#define	HwLCTRL_NI					Hw8											// Non-Interlace
#define	HwLCTRL_TV					Hw7											// TV mode 
#define	HwLCTRL_TFT					Hw6											// TFT LCD mode
#define	HwLCTRL_STN					Hw5											// STN LCD mode
#define	HwLCTRL_MSEL				Hw4											// Master Select for Image 0
#define	HwLCTRL_OVP					(Hw3+Hw2+Hw1)								// Overlay priority
#define	HwLCTRL_LEN					Hw0											// LCD Controller Enable

// LCD Clock Divider
#define	HwLCLKDIV_CS				Hw31										// Clock Source		
#define	HwLCLKDIV_ACDIV				Hw16										// AC bias clock divisor (STN only)
#define	HwLCLKDIV_LCLKDIV			Hw8											// LCLK clocks divider 
#define	HwLCLKDIV_PXCLKDIV			(Hw8 - Hw0)											// Pixel Clock Divider

// LCD Vertical Timing 1
#define	HwLVTIME1_VDB				Hw27										// Back porchVSYNC delay	
#define	HwLVTIME1_VDF				Hw22										// Front porch of VSYNC delay	
#define	HwLVTIME1_FPW				Hw16										// Frame pulse width	 
#define	HwLVTIME1_FLC				Hw0											// Frame line count	 

// LCD Status
#define	HwLSTATUS_VS				Hw31										// Monitoring vertical sync
#define	HwLSTATUS_BUSY				Hw30										// Busy signal
#define	HwLSTATUS_EF				Hw29										// Even-Field(Read Only). 0:Odd field or frame, 1:Even field or frame
#define	HwLSTATUS_DEOF				Hw28										// DMA End of Frame flag
#define	HwLSTATUS_I0EOF				Hw27										// Image 0 End of Frame flag
#define	HwLSTATUS_I1EOF				Hw26										// Image 1 End of Frame flag
#define	HwLSTATUS_I2EOF				Hw25										// Image 2 End of Frame flag
#define	HwLSTATUS_IE2F				Hw12										// Image 2 end-of-frame falling edge flag
#define	HwLSTATUS_IE2R				Hw11										// Image 2 end-of-frame rising edge flag
#define	HwLSTATUS_IE1F				Hw10										// Image 1 end-of-frame falling edge flag
#define	HwLSTATUS_IE1R				Hw9											// Image 1 end-of-frame rising edge flag
#define	HwLSTATUS_IE0F				Hw8											// Image 0 end-of-frame falling edge flag
#define	HwLSTATUS_IE0R				Hw7											// Image 0 end-of-frame rising edge flag
#define	HwLSTATUS_DEF				Hw6											// DMA end-of-frame falling edge flag
#define	HwLSTATUS_DER				Hw5											// DMA end-of-frame rising edge flag
#define	HwLSTATUS_DD				Hw4											// Disable done
#define	HwLSTATUS_RU				Hw3											// Register update flag
#define	HwLSTATUS_VSF				Hw2											// VS falling flag
#define	HwLSTATUS_VSR				Hw1											// VS rising flag
#define	HwLSTATUS_FU				Hw0											// LCD output fifo under-run flag.

// LCD Interrupt
#define	HwLIM_FU					Hw0											//LCD output fifo under-run interrupt mask
#define	HwLIM_VSR					Hw1											// VS rising interrupt mask
#define	HwLIM_VSF					Hw2											// VS falling interrupt mask
#define	HwLIM_RU					Hw3											// Register update interrupt mask
#define	HwLIM_DD					Hw4											// Disable done interrupt mask
#define	HwLIM_DER					Hw5											// DMA end-of-frame rising edge interrupt mask
#define	HwLIM_DEF					Hw6											// DMA end-of-frame falling edge interrupt mask
#define	HwLIM_IE0R					Hw7											// Image 0 end-of-frame rising edge interrupt mask
#define	HwLIM_IE0F					Hw8											// Image 0 end-of-frame falling edge interrupt mask
#define	HwLIM_IE1R					Hw9											// Image 1 end-of-frame rising edge interrupt mask
#define	HwLIM_IE1F					Hw10										// Image 1 end-of-frame falling edge interrupt mask
#define	HwLIM_IE2R					Hw11										// Image 2 end-of-frame rising edge interrupt mask
#define	HwLIM_IE2F					Hw12										// Image 2 end-of-frame falling edge interrupt mask

// LCD Image Control
#define	HwLIC_INTL					Hw31										// DMA interlace type
#define	HwLIC_AEN					Hw30										// AlphaBlending Function for Each Image
#define	HwLIC_CEN					Hw29										// Chroma-Key Function for Each Image
#define	HwLIC_IEN					Hw28										// Image Display Function for Each Image
#define	HwLIC_SRC					Hw27										// Image Source Select
#define	HwLIC_AOPT					(Hw26+Hw25)									// Alpha -blending Option Selection Bits
#define	HwLIC_ASEL					Hw24										// Image Displaying Function for Each Image
#define	HwLIC_PD					Hw15										// Bit padding
#define	HwLIC_Y2RMD					(Hw10+Hw9)									// YCbCr to RGB Conversion Option
#define	HwLIC_Y2R					Hw8											// YCbCr to RGB Conversion Enable Bit
#define	HwLIC_BR					Hw7											// Bit Reverse
#define	HwLIC_FMT					(Hw4+Hw3+Hw2+Hw1+Hw0)						// Image Format


// LCD Image Scale Ratio
#define	HwLISCALE_Y					(Hw7+Hw6+Hw5+Hw4)							// Y DownScale
#define	HwLISCALE_X					(Hw3+Hw2+Hw1+Hw0)							// X DownScale

// LCD Alpha Information
#define	HwLIA_A0					(Hw24-Hw16)									// Alpha Value 0
#define	HwLIA_A1					(Hw8-Hw0)									// Alpha Value 1

// LCD Chroma-key 
#define	HwLICKEY					(Hw8-Hw0)									// Chroma key 
#define	HwLICKEYMASK				(Hw24-Hw16)									// Chroma key mask













/************************************************************************
*	5. LCD System Interface Register Define		(Base Addr = 0xF0200400)
************************************************************************/
#define	HwLCDSI_BASE				*(volatile unsigned long *)0xF020C400	// LCDSI Base Register
#define HwLCDSI						((PLCDSI)&HwLCDSI_BASE)

// Control for LCDSI
#define	HwLCDSICTRL_IA				Hw15										// LACBIAS(Data Enable) signal is active low
#define	HwLCDSICTRL_IVS				Hw14										// LYSYNC signal is active low
#define	HwLCDSICTRL_EN				Hw8											// If IM is high, CS1 is active during operationgs. Otherwise, it is no applicable. These bits are only available when IM is high
#define	HwLCDSICTRL_CS				Hw7											// If IM is high, CS1 is active during operationgs. Otherwise, it is no applicable. These bits are only available when IM is high
#define	HwLCDSICTRL_RSP				Hw6											// If IM is high, RS is high. Otherwise, it is not applicable
#define	HwLCDSICTRL_FMT				(Hw2 + Hw3 + Hw4 + Hw5)						// LCDSI pixel data output(8bits) : D1[7:0], D1[15:8]
																				
// LCDSI MODE
#define	HwLCDSICTRL0_OM				Hw0																		

// Control for nCS0 when RS = 0
#define	HwLCDSICTRL_BW				(Hw31+Hw15)									// LCDXD data width is 8bits. Prefix W means writing operation

/***********************************************************************
*	6. Memory to Memory Scaler Register Define	(Base Addr = 0xF0210000/0xF0220000)
************************************************************************/
#define HwM2MSCALER0_BASE           *(volatile unsigned long *)0xF0210000
#define HwM2MSCALER1_BASE           *(volatile unsigned long *)0xF0220000
#define HwM2MSCALER0				((PM2MSCALER)&HwM2MSCALER0_BASE)
#define HwM2MSCALER1				((PM2MSCALER)&HwM2MSCALER1_BASE)

//-------------------------------------------- Scaler0 --------------------------------------------
// Scaler source image Y base address
#define	HwMSC_SRC_Y_BASE_ADDR		0xFFFFFFFC									// HwMSC_SRC_Y_BASE_ADDR [31: 2]		// Scaler source image Y base
// Scaler source image U base address
#define	HwMSC_SRC_U_BASE_ADDR		0xFFFFFFFC									// HwMSC_SRC_U_BASE_ADDR [31: 2]		// Scaler source image U base
// Scaler source image V base address
#define	HwMSC_SRC_V_BASE_ADDR		0xFFFFFFFC									// HwMSC_SRC_V_BASE_ADDR [31: 2]		// Scaler source image V base

// Source image size
#define	HwMSC_SRC_SIZE_H			0x00000FFF									// HwMSC_SRC_SIZE_H [11: 0]		// Input image Horizontal size by pixel
#define	HwMSC_SRC_SIZE_V			0x0FFF0000									// HwMSC_SRC_SIZE_V [27: 16]		// Input image Vertical size by pixel

// Source image line offset
#define	HwMSC_SRC_OFFSET_Y			0x00000FFF									// HwMSC_SRC_OFFSET_Y [11: 0]		// Input image line offset of luminace
#define	HwMSC_SRC_OFFSET_C			0x0FFF0000									// HwMSC_SRC_OFFSET_C [27: 16]		// Input image line offset of chrominance

// Source image Configuration
#define	HwMSC_SRC_CFG_INTM			Hw5											// interleaved mode cb/cr order	[5]
#define	HwMSC_SRC_CFG_INTLV			Hw3											// interleaved type [3]
#define	HwMSC_SRC_CFG_YUV422_SEQ0	HwZERO										// YUV422 SEQuential order 0
#define	HwMSC_SRC_CFG_YUV422_SEQ1	Hw0											// YUV422 SEQuential order 1
#define	HwMSC_SRC_CFG_YUV422_SEP	Hw1											// YUV422 SEPERATED
#define	HwMSC_SRC_CFG_YUV420_SEP	(Hw1|Hw0)									// YUV420 SEPERATED
#define	HwMSC_SRC_CFG_RGB565		Hw2											// YUV422 SEPERATED
#define	HwMSC_SRC_CFG_RGB555		(Hw2|Hw0)									// YUV422 SEPERATED
#define	HwMSC_SRC_CFG_RGB444		(Hw2|Hw1)									// YUV422 SEPERATED
#define	HwMSC_SRC_CFG_RGB454		(Hw2|Hw1|Hw0)
//#define	HwMSC_SRC_CFG_INVALID		(Hw2|Hw1|Hw0)								// INVALID

// Scaler destination image Y base address
#define	HwMSC_DST_Y_BASE_ADDR		0xFFFFFFFC									// HwMSC_DST_Y_BASE_ADDR [31: 2]		// Scaler destination image Y base
// Scaler destination image U base address
#define	HwMSC_DST_U_BASE_ADDR		0xFFFFFFFC									// HwMSC_DST_U_BASE_ADDR [31: 2]		// Scaler destination image U base
// Scaler destination image V base address
#define	HwMSC_DST_V_BASE_ADDR		0xFFFFFFFC									// HwMSC_DST_V_BASE_ADDR [31: 2]		// Scaler destination image V base

// Destination image size
#define	HwMSC_DST_SIZE_H			0x00000FFF									// HwMSC_DST_SIZE_H	[11: 0]		// Input image Horizontal size by pixel
#define	HwMSC_DST_SIZE_V			0x0FFF0000									// HwMSC_DST_SIZE_V	[27: 16]		// Input image Vertical size by pixel

// Destination image line offset
#define	HwMSC_DST_OFFSET_Y			0x00000FFF									// HwMSC_DST_OFFSET_Y [11: 0]		// Input image line offset of luminace
#define	HwMSC_DST_OFFSET_C			0x0FFF0000									// HwMSC_DST_OFFSET_C [27: 16]		// Input image line offset of chrominance

// Destination image Configuration
#define	HwMSC_DST_CFG_COP			Hw11										// COP	[11]
#define	HwMSC_DST_CFG_WAIT			0x00000700									// WAIT	[10:8]
#define	HwMSC_DST_CFG_RDY			Hw6											// READY [6]
#define	HwMSC_DST_CFG_INTM			Hw5											// interleaved mode cb/cr order	[5]
#define	HwMSC_DST_CFG_PATH			Hw4											// [4]	0 : to memory (Master mode, Scaler master writes result to memory)
#define	HwMSC_DST_CFG_INTLV			Hw3											// interleaved type [3]

// 070802_hjb		//	1 : LCD (Slave mode, LCD master reads scaling results)
#define	HwMSC_DST_CFG_YUV422_SEQ0	HwZERO										// YUV422 SEQuential order 0
#define	HwMSC_DST_CFG_YUV422_SEQ1	Hw0											// YUV422 SEQuential order 1
#define	HwMSC_DST_CFG_YUV422_SEP	Hw1											// YUV422 SEPERATED
#define	HwMSC_DST_CFG_YUV420_SEP	(Hw1|Hw0)									// YUV420 SEPERATED
#define	HwMSC_DST_CFG_RGB565		Hw2											// YUV422 SEPERATED
#define	HwMSC_DST_CFG_RGB555		(Hw2|Hw0)									// YUV422 SEPERATED
#define	HwMSC_DST_CFG_RGB444		(Hw2|Hw1)									// YUV422 SEPERATED
#define	HwMSC_DST_CFG_RGB454		(Hw2|Hw1|Hw0)
//#define	HwMSC_DST_CFG_INVALID		(Hw2|Hw1|Hw0)								// INVALID

// Scale ratio
#define	HwMSC_HRATIO				0x00003FFF									// HwMSC_HRATIO [29:16] Horizontal scale factor
#define	HwMSC_VRATIO				0x3FFF0000									// HwMSC_VRATIO [13:0]	Vertical scale factor

// Scaler control
#define	HwMSC_INPTH					Hw24										// Rolling Operation Mode Enable
#define	HwMSC_REN					Hw23										// Rolling Operation Mode Enable
#define	HwMSC_MEN					Hw22										// middle Operation Mode Enable
#define	HwMSC_RLS					Hw19										// Release Stop mode
#define	HwMSC_RGSM					Hw17										// Rolling Go Stop Mode 
#define	HwMSC_MGSM					Hw16										// Middle Go Stop Mode
#define	HwMSC_RIRQEN				Hw7											// Rolling interrupt enable
#define	HwMSC_MIRQEN				Hw6											// Middle interrupt enable
#define	HwMSC_CTRL_CON_EN			Hw5	
#define	HwMSC_CTRL_BUSY_EN			Hw2											// HwMSC_CTRL_BUSY_EN [2] Scaler BUSY interrupt enable
#define	HwMSC_CTRL_RDY_EN			Hw1											// HwMSC_CTRL_RDY_EN	[1] Scaler READY intertupt enable
#define	HwMSC_CTRL_EN				Hw0											// HwMSC_CTRL_EN	[0] Scaler enable

// Scaler status
#define	HwMSC_IR					Hw7											// Rolling interrupt flag
#define	HwMSC_IM					Hw6											// Middle interrupt flag
#define	HwMSC_STATUS_IBUSY			Hw5											// HwMSC_STATUS_IBUSY [5] BUSY INTERRUPT FLAG
#define	HwMSC_STATUS_IRDY			Hw4											// HwMSC_STATUS_IRDY	[4] READY INTERRUPT FLAG
#define	HwMSC_STATUS_BUSY			Hw1											// HwMSC_STATUS_BUSY  [1] BUSY Status FLAG
#define	HwMSC_STATUS_RDY			Hw0											// HwMSC_STATUS_RDY	[0] READY Status FLAG


#define	HwMSC_DST_ROLL_CNT			0x0FFF0000
#define	HwMSC_DST_MID_CNT			0x00000FFF

#define	HwMSC_C_R_CNT				0x0FFF0000

/************************************************************************
*	7. NTSC/PAL ENCODER Composite Output Register Define (Base Addr = 0xF9000000)
************************************************************************/
#define 	HwTVE_VEN_BASE				*(volatile unsigned long *)0xF0240800
#define	HwTVE_BASE					*(volatile unsigned long *)0xF0240000	// TV Encoder Base Register

#define 	HwTVE						((PNTSCPAL)&HwTVE_BASE)
#define 	HwTVE_VEN					((PNTSCPALOP)&HwTVE_VEN_BASE)

// Encoder Mode Control A
#define	HwTVECMDA_PWDENC_PD			Hw7											// Power down mode for entire digital logic of TV encoder
#define	HwTVECMDA_FDRST_1			Hw6											// Chroma is free running as compared to H-sync
#define	HwTVECMDA_FDRST_0			HwZERO										// Relationship between color burst & H-sync is maintained for video standards
#define	HwTVECMDA_FSCSEL(X)			((X)*Hw4)
#define	HwTVECMDA_FSCSEL_NTSC		HwTVECMDA_FSCSEL(0)							// Color subcarrier frequency is 3.57954545 MHz for NTSC
#define	HwTVECMDA_FSCSEL_PALX		HwTVECMDA_FSCSEL(1)							// Color subcarrier frequency is 4.43361875 MHz for PAL-B,D,G,H,I,N
#define	HwTVECMDA_FSCSEL_PALM		HwTVECMDA_FSCSEL(2)							// Color subcarrier frequency is 3.57561149 MHz for PAL-M
#define	HwTVECMDA_FSCSEL_PALCN		HwTVECMDA_FSCSEL(3)							// Color subcarrier frequency is 3.58205625 MHz for PAL-combination N
#define	HwTVECMDA_FSCSEL_MASK		HwTVECMDA_FSCSEL(3)
#define	HwTVECMDA_PEDESTAL			Hw3											// Video Output has a pedestal
#define	HwTVECMDA_NO_PEDESTAL		HwZERO										// Video Output has no pedestal
#define	HwTVECMDA_PIXEL_SQUARE		Hw2											// Input data is at square pixel rates.
#define	HwTVECMDA_PIXEL_601			HwZERO										// Input data is at 601 rates.
#define	HwTVECMDA_IFMT_625			Hw1											// Output data has 625 lines
#define	HwTVECMDA_IFMT_525			HwZERO										// Output data has 525 lines
#define	HwTVECMDA_PHALT_PAL			Hw0											// PAL encoded chroma signal output
#define	HwTVECMDA_PHALT_NTSC		HwZERO										// NTSC encoded chroma signal output

// Encoder Mode Control B
#define	HwTVECMDB_YBIBLK_BLACK		Hw4											// Video data is forced to Black level for Vertical non VBI processed lines.
#define	HwTVECMDB_YBIBLK_BYPASS		HwZERO										// Input data is passed through forn non VBI processed lines.
#define	HwTVECMDB_CBW(X)			((X)*Hw2)
#define	HwTVECMDB_CBW_LOW			HwTVECMDB_CBW(0)							// Low Chroma band-width
#define	HwTVECMDB_CBW_MEDIUM		HwTVECMDB_CBW(1)							// Medium Chroma band-width
#define	HwTVECMDB_CBW_HIGH			HwTVECMDB_CBW(2)							// High Chroma band-width
#define	HwTVECMDB_CBW_MASK			HwTVECMDB_CBW(3)							// 
#define	HwTVECMDB_YBW(X)			((X)*Hw0)
#define	HwTVECMDB_YBW_LOW			HwTVECMDB_YBW(0)							// Low Luma band-width
#define	HwTVECMDB_YBW_MEDIUM		HwTVECMDB_YBW(1)							// Medium Luma band-width
#define	HwTVECMDB_YBW_HIGH			HwTVECMDB_YBW(2)							// High Luma band-width
#define	HwTVECMDB_YBW_MASK			HwTVECMDB_YBW(3)							// 

// Encoder Clock Generator
#define	HwTVEGLK_XT24_24MHZ			Hw4											// 24MHz Clock input
#define	HwTVEGLK_XT24_27MHZ			HwZERO										// 27MHz Clock input
#define	HwTVEGLK_GLKEN_RST_EN		Hw3											// Reset Genlock
#define	HwTVEGLK_GLKEN_RST_DIS		~Hw3										// Release Genlock
#define	HwTVEGLK_GLKE(X)			((X)*Hw1)
#define	HwTVEGLK_GLKE_INT			HwTVEGLK_GLKE(0)							// Chroma Fsc is generated from internal constants based on current user setting
#define	HwTVEGLK_GLKE_RTCO			HwTVEGLK_GLKE(2)							// Chroma Fsc is adjusted based on external RTCO input
#define	HwTVEGLK_GLKE_CLKI			HwTVEGLK_GLKE(3)							// Chroma Fsc tracks non standard encoder clock (CLKI) frequency
#define	HwTVEGLK_GLKE_MASK			HwTVEGLK_GLKE(3)							//
#define	HwTVEGLK_GLKEN_GLKPL_HIGH	Hw0											// PAL ID polarity is active high
#define	HwTVEGLK_GLKEN_GLKPL_LOW	HwZERO										// PAL ID polarity is active low

// Encoder Mode Control C
#define	HwTVECMDC_CSMDE_EN			Hw7											// Composite Sync mode enabled
#define	HwTVECMDC_CSMDE_DIS			~Hw7										// Composite Sync mode disabled (pin is tri-stated)
#define	HwTVECMDC_CSMD(X)			((X)*Hw5)
#define	HwTVECMDC_CSMD_CSYNC		HwTVECMDC_CSMD(0)							// CSYN pin is Composite sync signal
#define	HwTVECMDC_CSMD_KEYCLAMP		HwTVECMDC_CSMD(1)							// CSYN pin is Keyed clamp signal
#define	HwTVECMDC_CSMD_KEYPULSE		HwTVECMDC_CSMD(2)							// CSYN pin is Keyed pulse signal
#define	HwTVECMDC_CSMD_MASK			HwTVECMDC_CSMD(3)
#define	HwTVECMDC_RGBSYNC(X)		((X)*Hw3)
#define	HwTVECMDC_RGBSYNC_NOSYNC	HwTVECMDC_RGBSYNC(0)						// Disable RGBSYNC (when output is configured for analog EGB mode)
#define	HwTVECMDC_RGBSYNC_RGB		HwTVECMDC_RGBSYNC(1)						// Sync on RGB output signal (when output is configured for analog EGB mode)
#define	HwTVECMDC_RGBSYNC_G			HwTVECMDC_RGBSYNC(2)						// Sync on G output signal (when output is configured for analog EGB mode)
#define	HwTVECMDC_RGBSYNC_MASK		HwTVECMDC_RGBSYNC(3)

// DAC Output Selection
#define	HwTVEDACSEL_DACSEL_CODE0	HwZERO										// Data output is diabled (output is code '0')
#define	HwTVEDACSEL_DACSEL_CVBS		Hw0											// Data output in CVBS format

// DAC Power Down
#define	HwTVEDACPD_PD_EN			Hw0											// DAC Power Down Enabled
#define	HwTVEDACPD_PD_DIS			~Hw0										// DAC Power Down Disabled

// Sync Control
#define	HwTVEICNTL_FSIP_ODDHIGH		Hw7											// Odd field active high
#define	HwTVEICNTL_FSIP_ODDLOW		HwZERO										// Odd field active low
#define	HwTVEICNTL_VSIP_HIGH		Hw6											// V-sync active high
#define	HwTVEICNTL_VSIP_LOW			HwZERO										// V-sync active low
#define	HwTVEICNTL_HSIP_HIGH		Hw5											// H-sync active high
#define	HwTVEICNTL_HSIP_LOW			HwZERO										// H-sync active low
#define	HwTVEICNTL_HSVSP_RISING		Hw4											// H/V-sync latch enabled at rising edge
#define	HwTVEICNTL_HVVSP_FALLING	HwZERO										// H/V-sync latch enabled at falling edge
#define	HwTVEICNTL_VSMD_START		Hw3											// Even/Odd field H/V sync output are aligned to video line start
#define	HwTVEICNTL_VSMD_MID			HwZERO										// Even field H/V sync output are aligned to video line midpoint
#define	HwTVEICNTL_ISYNC(X)			((X)*Hw0)
#define	HwTVEICNTL_ISYNC_FSI		HwTVEICNTL_ISYNC(0)							// Alignment input format from FSI pin
#define	HwTVEICNTL_ISYNC_HVFSI		HwTVEICNTL_ISYNC(1)							// Alignment input format from HSI,VSI,FSI pin
#define	HwTVEICNTL_ISYNC_HVSI		HwTVEICNTL_ISYNC(2)							// Alignment input format from HSI,VSI pin
#define	HwTVEICNTL_ISYNC_VFSI		HwTVEICNTL_ISYNC(3)							// Alignment input format from VSI,FSI pin
#define	HwTVEICNTL_ISYNC_VSI		HwTVEICNTL_ISYNC(4)							// Alignment input format from VSI pin
#define	HwTVEICNTL_ISYNC_ESAV_L		HwTVEICNTL_ISYNC(5)							// Alignment input format from EAV,SAV codes (line by line)
#define	HwTVEICNTL_ISYNC_ESAV_F		HwTVEICNTL_ISYNC(6)							// Alignment input format from EAV,SAV codes (frame by frame)
#define	HwTVEICNTL_ISYNC_FREE		HwTVEICNTL_ISYNC(7)							// Alignment is free running (Master mode)
#define	HwTVEICNTL_ISYNC_MASK		HwTVEICNTL_ISYNC(7)

// Offset Control
#define	HwTVEHVOFFST_INSEL(X)		((X)*Hw6)
#define	HwTVEHVOFFST_INSEL_BW16_27MHZ		HwTVEHVOFFST_INSEL(0)				// 16bit YUV 4:2:2 sampled at 27MHz
#define	HwTVEHVOFFST_INSEL_BW16_13P5MH		HwTVEHVOFFST_INSEL(1)				// 16bit YUV 4:2:2 sampled at 13.5MHz
#define	HwTVEHVOFFST_INSEL_BW8_13P5MHZ		HwTVEHVOFFST_INSEL(2)				// 8bit YUV 4:2:2 sampled at 13.5MHz
#define	HwTVEHVOFFST_INSEL_MASK		HwTVEHVOFFST_INSEL(3)
#define	HwTVEHVOFFST_VOFFST_256		Hw3											// Vertical offset bit 8 (Refer to HwTVEVOFFST)
#define	HwTVEHVOFFST_HOFFST_1024	Hw2											// Horizontal offset bit 10 (Refer to HwTVEHOFFST)
#define	HwTVEHVOFFST_HOFFST_512		Hw1											// Horizontal offset bit 9 (Refer to HwTVEHOFFST)
#define	HwTVEHVOFFST_HOFFST_256		Hw0											// Horizontal offset bit 8 (Refer to HwTVEHOFFST)

// Sync Output Control
#define	HwTVEHSVSO_VSOB_256			Hw6											// VSOB bit 8 (Refer to HwVSOB)
#define	HwTVEHSVSO_HSOB_1024		Hw5											// HSOB bit 10 (Refer to HwHSOB)
#define	HwTVEHSVSO_HSOB_512			Hw4											// HSOB bit 9 (Refer to HwHSOB)
#define	HwTVEHSVSO_HSOB_256			Hw3											// HSOB bit 8 (Refer to HwHSOB)
#define	HwTVEHSVSO_HSOE_1024		Hw2											// HSOE bit 10 (Refer to HwHSOE)
#define	HwTVEHSVSO_HSOE_512			Hw1											// HSOE bit 9 (Refer to HwHSOE)
#define	HwTVEHSVSO_HSOE_256			Hw0											// HSOE bit 8 (Refer to HwHSOE)

// Trailing Edge of Vertical Sync Control
#define	HwTVEVSOE_VSOST(X)			((X)*Hw6)									// Programs V-sync relative location for Odd/Even Fields.
#define	HwTVEVSOE_NOVRST_EN			Hw5											// No vertical reset on every field
#define	HwTVEVSOE_NOVRST_NORMAL		HwZERO										// Normal vertical reset operation (interlaced output timing)
#define	HwTVEVSOE_VSOE(X)			((X)*Hw0)									// Trailing Edge of Vertical Sync Control

// VBI Control Register
#define	HwTVEVCTRL_VBICTL(X)		((X)*Hw5)									// VBI Control indicating the current line is VBI.
#define	HwTVEVCTRL_VBICTL_NONE		HwTVEVCTRL_VBICTL(0)						// Do nothing, pass as active video.
#define	HwTVEVCTRL_VBICTL_10LINE	HwTVEVCTRL_VBICTL(1)						// Insert blank(Y:16, Cb,Cr: 128), for example, 10 through 21st line.
#define	HwTVEVCTRL_VBICTL_1LINE		HwTVEVCTRL_VBICTL(2)						// Insert blank data 1 line less for CC processing.
#define	HwTVEVCTRL_VBICTL_2LINE		HwTVEVCTRL_VBICTL(3)						// Insert blank data 2 line less for CC and CGMS processing.
#define	HwTVEVCTRL_MASK				HwTVEVCTRL_VBICTL(3)					
#define	HwTVEVCTRL_CCOE_EN			Hw4											// Closed caption odd field enable.
#define	HwTVEVCTRL_CCEE_EN			Hw3											// Closed caption even field enable.
#define	HwTVEVCTRL_CGOE_EN			Hw2											// Copy generation management system enable odd field.
#define	HwTVEVCTRL_CGEE_EN			Hw1											// Copy generation management system enable even field.
#define	HwTVEVCTRL_WSSE_EN			Hw0											// Wide screen enable.

// Connection between LCDC & TVEncoder Control
#define	HwTVEVENCON_EN_EN			Hw0											// Connection between LCDC & TVEncoder Enabled
#define	HwTVEVENCON_EN_DIS			~Hw0										// Connection between LCDC & TVEncoder Disabled

// I/F between LCDC & TVEncoder Selection
#define	HwTVEVENCIF_MV_1			Hw1											// reserved
#define	HwTVEVENCIF_FMT_1			Hw0											// PXDATA[7:0] => CIN[7:0], PXDATA[15:8] => YIN[7:0]
#define	HwTVEVENCIF_FMT_0			HwZERO										// PXDATA[7:0] => YIN[7:0], PXDATA[15:8] => CIN[7:0]

/************************************************************************
*	8. HDMI Register Define				(Base Addr = 0xF0254000)
************************************************************************/
//Controller register base address 
#define HwHDMICTRL_BASE				*(volatile unsigned long *)0xF0254000	//Controller register base address 
	
//HDMI register base address 
#define HwHDMICORE_BASE  			*(volatile unsigned long *)0xF0255000  
#define HwHDMICORE					((PHDMICORE)&HwHDMICORE_BASE)

//AES register base address 
#define HwHDMIAES_BASE  			*(volatile unsigned long *)0xF0256000  //AES register base address 

//SPDIF Receiver register base address 
#define HwHDMISPDIF_BASE  			*(volatile unsigned long *)0xF0257000  

//I2S Receiver register base address 
#define HwHDMII2S_BASE  			*(volatile unsigned long *)0xF0258000  
				
 //CEC register base address 					
#define HwHDMICEC_BASE  			*(volatile unsigned long *)0xF0259000 

/***********************************************************************
*	 9-1. Camera Interface Register Define			(Base Addr = 0xF0230000)
************************************************************************/
#define	HwCIF_BASE					*(volatile unsigned long *)0xF0230000	// CIF Base Register
#define HwCIF						((PCIF)&HwCIF_BASE)

// Input Image Color/Pattern Configuration 1
#define	HwICPCR1_ON					Hw31										// On/Off on CIF >> 0:Can't operate CIF , 1:Operating CIF
#define	HwICPCR1_PWD				Hw30										// Power down mode in camera >> 0:Disable, 1:Power down mode , This power down mode is connected the PWDN of camera sensor
#define	HwICPCR1_TV					Hw29										// TV signal   0: CIF sync signal, 1: TV sync signal
#define	HwICPCR1_TI					Hw28										// Vertical blasnk insert   0: Do not insert vertical blank in even field, 1: insert vertical blank in 1 line in even field
#define	HwICPCR1_UF					Hw27										// Use field signal (in interlace mode, it is decided whether field signal is used or not)  0: Don't Use , 1: Use field signal
#define	HwICPCR1_BPS				Hw23										// Bypass Scaler >> 0:Non, 1:Bypass
#define	HwICPCR1_POL				Hw21										// PXCLK Polarity >> 0:Positive edge, 1:Negative edge
#define	HwICPCR1_SKPF				(Hw20|Hw19|Hw18)							// Skip Frame >> 0~7 #Frames skips	[20:18]
#define	HwICPCR1_M420_ZERO			HwZERO										// Format Convert (YUV422->YUV420) , Not-Convert
#define	HwICPCR1_M420_ODD			Hw17										// converted in odd line skip
#define	HwICPCR1_M420_EVEN			(Hw17|Hw16)									// converted in even line skip
#define	HwICPCR1_BP					Hw15										// Bypass
#define	HwICPCR1_BBS_LSB8			Hw14										// When bypass 16bits mode, LSB 8bits are stored in first
#define	HwICPCR1_C656				Hw13										// Convert 656 format 0:Disable, 1:Enable
#define	HwICPCR1_CP_RGB				Hw12										// RGB(555,565,bayer) color pattern
#define	HwICPCR1_PF_444				HwZERO										// 4:4:4 format
#define	HwICPCR1_PF_422				Hw10										// 4:2:2 format
#define	HwICPCR1_PF_420				Hw11										// 4:2:0 format or RGB(555,565,bayer) mode
#define	HwICPCR1_RGBM_BAYER			HwZERO										// Bayer RGB Mode
#define	HwICPCR1_RGBM_RGB555		Hw8											// RGB555 Mode
#define	HwICPCR1_RGBM_RGB565		Hw9											// RGB565 Mode
#define	HwICPCR1_RGBBM_16			HwZERO										// 16bit mode
#define	HwICPCR1_RGBBM_8DISYNC		Hw6											// 8bit disable sync
#define	HwICPCR1_RGBBM_8			Hw7											// 8bit mode
#define	HwICPCR1_CS_RGBMG			HwZERO										// 555RGB:RGB(MG), 565RGB:RGB, 444/422/420:R/Cb/U first, Bayer RGB:BG->GR, CCIR656:YCbYCr
#define	HwICPCR1_CS_RGBLG			Hw4											// 555RGB:RGB(LG), 565RGB:RGB, 444/422/420:R/Cb/U first, Bayer RGB:GR->BG, CCIR656:YCrYCb
#define	HwICPCR1_CS_BGRMG			Hw5											// 555RGB:BGR(MG), 565RGB:BGR, 444/422/420:B/Cr/V first, Bayer RGB:RG->GB, CCIR656:CbYCrY
#define	HwICPCR1_CS_BGRLG			(Hw5|Hw4)									// 555RGB:BGR(LG), 565RGB:BGR, 444/422/420:B/Cr/V first, Bayer RGB:GB->RG, CCIR656:CrYCbY
#define	HwICPCR1_BO_SW				Hw2											// Switch the MSB/LSB 8bit Bus
#define	HwICPCR1_HSP_HIGH			Hw1											// Active high
#define	HwICPCR1_VSP_HIGH			Hw0											// Active high

// CCIR656 Format Configuration 1
#define	Hw656FCR1_PSL_1ST			HwZERO										// The status word is located the first byte of EAV & SAV
#define	Hw656FCR1_PSL_2ND			Hw25										// The status word is located the second byte of EAV & SAV
#define	Hw656FCR1_PSL_3RD			Hw26										// The status word is located the third byte of EAV & SAV
#define	Hw656FCR1_PSL_4TH			(Hw26|Hw25)									// The status word is located the forth byte of EAV & SAV
																				//FPV [23:16] 0x00FF0000,	SPV [15:8] 0x0000FF00, TPV [7:0]	0x000000FF
// CMOSIF DMA Configuratin 1
#define	HwCDCR1_TM_INC				Hw3											// INC Transfer
#define	HwCDCR1_LOCK_ON				Hw2											// Lock Transfer
#define	HwCDCR1_BS_1				HwZERO										// The DMA transfers the image data as 1 word to memory
#define	HwCDCR1_BS_2				Hw0											// The DMA transfers the image data as 2 word to memory
#define	HwCDCR1_BS_4				Hw1											// The DMA transfers the image data as 4 word to memory
#define	HwCDCR1_BS_8				(Hw1|Hw0)									// The DMA transfers the image data as 8 word to memory (default)

// FIFO Status
#define	HwFIFOSTATE_CLR				Hw21										// Clear FIFO states, 1:Clear, 0:Not Clear
#define	HwFIFOSTATE_REO				Hw19										// Overlay FIFO Read ERROR,	1:The empty signal of input overlay FIFO and read enable signal are High, 0:The empty signal of overlay FIFO is low, or empty is High and read enable signal is Low.
#define	HwFIFOSTATE_REV				Hw18										// V(B) Channel FiFO Read ERROR, 1:The empty signal of input V(B) channel FIFO and read enable signal are High, 0:The empty signal of V(B) channel FIFO is Low, or empty is High and read enable signal is Low.
#define	HwFIFOSTATE_REU				Hw17										// U(R) Channel FiFO Read ERROR, 1:The empty signal of input U(R) channel FIFO and read enable signal are High, 0:The empty signal of U(R) channel FIFO is Low, or empty is High and read enable signal is Low.
#define	HwFIFOSTATE_REY				Hw16										// Y(G) Channel FiFO Read ERROR, 1:The empty signal of input Y(G) channel FIFO and read enable signal are High, 0:The empty signal of Y(G) channel FIFO is Low, or empty is High and read enable signal is Low.
#define	HwFIFOSTATE_WEO				Hw13										// Overlay FIFO Write ERROR, 1:The full signal of overlay FIFO and write enable signal are High, 0:The full signal of overlay FIFO is Low, or full is High and write enable signal is Low.
#define	HwFIFOSTATE_WEV				Hw12										// V(B) Channel FiFO Write ERROR, 1:The full signal of V(B) channel FIFO and write enable signal are High, 0:The full signal of V(B) channel FIFO is Low, or full is High and write enable signal is Low.
#define	HwFIFOSTATE_WEU				Hw11										// U(R) Channel FiFO Write ERROR, 1:The full signal of U(R) channel FIFO and write enable signal are High, 0:The full signal of U(R) channel FIFO is Low, or full is High and write enable signal is Low.
#define	HwFIFOSTATE_WEY				Hw10										// Y(G) Channel FiFO Write ERROR, 1:The full signal of Y channel FIFO and write enable signal are High, 0:The full signal of Y channel FIFO is Low, or full is High and write enable signal is Low.
#define	HwFIFOSTATE_EO				Hw8											// Overlay FIFO Empty Signal, 1:The state of overlay FIFO is empty, 0:The state of overlay FIFO is non-empty.
#define	HwFIFOSTATE_EV				Hw7											// V(B) Channel FiFO Empty Signal, 1:The state of V(B) channel FIFO is empty, 0:The state of V(B) channel FIFO is non-empty.
#define	HwFIFOSTATE_EU				Hw6											// U(R) Channel FiFO Empty Signal, 1:The state of U(R) channel FIFO is empty, 0:The state of U(R) channel FIFO is non-empty.
#define	HwFIFOSTATE_EY				Hw5											// Y(G) Channel FiFO Empty Signal, 1:The state of Y channel FIFO is empty, 0:The state of Y channel FIFO is non-empty.
#define	HwFIFOSTATE_FO				Hw3											// Overlay FiFO FULL Signal, 1:The state of overlay FIFO is full, 0:The state of overlay FIFO is non-full.
#define	HwFIFOSTATE_FV				Hw2											// V(B) Channel FiFO FULL Signal, 1:The state of V(B) channel FIFO is full, 0:The state of V(B) channel FIFO is non-full.
#define	HwFIFOSTATE_FU				Hw1											// U(R) Channel FiFO FULL Signal, 1:The state of U(R) channel FIFO is full, 0:The state of U(R) channel FIFO is non-full.
#define	HwFIFOSTATE_FY				Hw0											// Y(G) Channel FiFO FULL Signal, 1:The state of Y(G) channel FIFO is full, 0:The state of Y(G) channel FIFO is non-full.

// Interrupt & CIF Operating
#define	HwCIRQ_IEN					Hw31										// Interrupt Enable	0:interrupt disable, 1:interrupt enable
#define	HwCIRQ_URV					Hw30										// Update Register in VSYNC	0:Register is update without VSYNC , 1:When VSYNC is posedge, register is updated.
#define	HwCIRQ_ITY					Hw29										// Interrupt Type	0:Pulse type, 1:Hold-up type when respond signal(ICR) is high
#define	HwCIRQ_ICR					Hw28										// Interrupt Clear 0:.... , 1:Interrupt Clear (using ITY is Hold-up type)
#define	HwCIRQ_MVN					Hw26										// Mask interrupt of VS negative edge,	0:Don't mask, 1:Mask
#define	HwCIRQ_MVP					Hw25										// Mask interrupt of VS positive edge,	0:Don't mask, 1:Mask
#define	HwCIRQ_MVIT					Hw24										// Mask interrupt of VCNT Interrupt,	0:Don't mask, 1:Mask
#define	HwCIRQ_MSE					Hw23										// Mask interrupt of Scaler Error,	0:Don't mask, 1:Mask
#define	HwCIRQ_MSF					Hw22										// Mask interrupt of Scaler finish,	0:Don't mask, 1:Mask
#define	HwCIRQ_MENS					Hw21										// Mask interrupt of Encoding start,	0:Don't mask, 1:Mask
#define	HwCIRQ_MRLV					Hw20										// Mask interrupt of Rolling V address,	0:Don't mask, 1:Mask
#define	HwCIRQ_MRLU					Hw19										// Mask interrupt of Rolling U address,	0:Don't mask, 1:Mask
#define	HwCIRQ_MRLY					Hw18										// Mask interrupt of Rolling Y address,	0:Don't mask, 1:Mask
#define	HwCIRQ_MSCF					Hw17										// Mask interrupt of Capture frame,	0:Don't mask, 1:Mask
#define	HwCIRQ_MSOF					Hw16										// Mask interrupt of Stored one frame,	0:Don't mask, 1:Mask
#define	HwCIRQ_VSS					Hw12										// Status of vertical sync, Non-vertical sync blank area.
#define	HwCIRQ_VN					Hw10										// VS negative, 0:-, 1:When VS is generated if negative edge
#define	HwCIRQ_VP					Hw9											// VS positive, 0:-, 1:When VS is generated if positive edge
#define	HwCIRQ_VIT					Hw8											// VCNT Interrupt, 0:-, 1:When VCNT is generated....
#define	HwCIRQ_SE					Hw7											// Scaler Error, 0:-, 1:When Scale operation is not correct.
#define	HwCIRQ_SF					Hw6											// Scaler Finish, 0:-, 1:When Scale operation is finished
#define	HwCIRQ_ENS					Hw5											// Encoding start status, 0:-, 1:When Y address is bigger than encoding start address, this bit is high
#define	HwCIRQ_ROLV					Hw4											// Rolling V address status, 0:-, 1:If V address is move to start address, this bit is high
#define	HwCIRQ_ROLU					Hw3											// Rolling U address starus, 0:-, 1:If U address is move to start address, this bit is high 
#define	HwCIRQ_ROLY					Hw2											// Rolling Y address starus, 0:-, 1:If Y address is move to start address, this bit is high 
#define	HwCIRQ_SCF					Hw1											// Stored captured frame,	0:-, 1:If Captured frame is stored, this bit is high
#define	HwCIRQ_SOF					Hw0											// Stored One frame, 0-, 1:If one frame if stored, this bit is high.

// Overlay Control 1
#define	HwOCTRL1_OCNT_MAX			(Hw29|Hw28|Hw27|Hw26|Hw25|Hw24)				//[28:24] Overlay Count FIFO (Hw27|Hw26|Hw25|Hw24|Hw23)
#define	HwOCTRL1_OM_BLOCK			Hw16										// Overlay Method 0:Full image overlay, 1:Block image overlay	, Full image overlay mode, overlay image size is equal to the input image size.
#define	HwOCTRL1_OE_EN				Hw12										// Overlay enable 0:Disable, 1:Enable
#define	HwOCTRL1_XR1_100			Hw10										// XOR in AP1 is 3 (100%)	0:XOR operation, 1:100%	, When AP1 is 3 and CEN & AEN is 1, We select the 100% alpha value or XOR.
#define	HwOCTRL1_XR0_100			Hw9											// XOR in AP0 is 3 (100%)	0:XOR operation, 1:100%	, When AP0 is 3 and CEN & AEN is 1, We select the 100% alpha value or XOR.
#define	HwOCTRL1_AP1_25				HwZERO										// Alpha Value in alpha is 1		// 25%			
#define	HwOCTRL1_AP1_50				Hw6											// Alpha Value in alpha is 1		// 50%
#define	HwOCTRL1_AP1_75				Hw7											// Alpha Value in alpha is 1		// 75%				
#define	HwOCTRL1_AP1_100			(Hw7|Hw6)									// Alpha Value in alpha is 1		// 100% or XOR operation (for XR value)
#define	HwOCTRL1_AP0_25				HwZERO										// Alpha Value in alpha is 0		// 25%			
#define	HwOCTRL1_AP0_50				Hw4											// Alpha Value in alpha is 0		// 50%
#define	HwOCTRL1_AP0_75				Hw5											// Alpha Value in alpha is 0		// 75%				
#define	HwOCTRL1_AP0_100			(Hw5|Hw4)									// Alpha Value in alpha is 0		// 100% or XOR operation
																				// When 565RGB and AEN, alpha value is depend on AP0 value.
#define	HwOCTRL1_AEN_EN				Hw2											// Alpha enable	0:Disable, 1:Enable
#define	HwOCTRL1_CEN_EN				Hw0											// Chroma key enable	0:Disable, 1:Enable

// Overlay Control 2 
#define	HwOCTRL2_CONV				Hw3											// Color Converter Enable 0:Disable, 1:Enable 
#define	HwOCTRL2_RGB_565			HwZERO										// RGB mode 565RGB 
#define	HwOCTRL2_RGB_555			Hw1											// RGB mode 555RGB
#define	HwOCTRL2_RGB_444			Hw2											// RGB mode 444RGB
#define	HwOCTRL2_RGB_332			(Hw2|Hw1)									// RGB mode 332RGB
#define	HwOCTRL2_MD					Hw0											// Color Mode	0:YUV Color, 1:RGB color

// Overlay Control 3 -- KEY Value 
#define	HwOCTRL3_KEYR_MAX			0x00FF0000									// Chroma-key value R(U), Chroea-key value in R(U) channel, Default value is 0x00
#define	HwOCTRL3_KEYG_MAX			0x0000FF00									// Chroma-key value G(Y), Chroea-key value in G(Y) channel, Default value is 0x00
#define	HwOCTRL3_KEYB_MAX			0x000000FF									// Chroma-key value B(V), Chroea-key value in B(V) channel, Default value is 0x00
 
// Overlay Control 4 -- Mask KEY Value
#define	HwOCTRL4_MKEYR_MAX			0x00FF0000									// Mask Chroma-key value R(U), Chroea-key value in R(U) channel, Default value is 0x00
#define	HwOCTRL4_MKEYG_MAX			0x0000FF00									// Mask Chroma-key value G(Y), Chroea-key value in G(Y) channel, Default value is 0x00
#define	HwOCTRL4_MKEYB_MAX			0x000000FF									// Mask Chroma-key value B(V), Chroea-key value in B(V) channel, Default value is 0x00

// Camera Down Scaler
#define	HwCDS_SFH_1					HwZERO										// Horizontal Scale Factor, 1/1 down scale
#define	HwCDS_SFH_2					Hw4											// Horizontal Scale Factor, 1/2 down scale
#define	HwCDS_SFH_4					Hw5											// Horizontal Scale Factor, 1/4 down scale
#define	HwCDS_SFH_8					(Hw5|Hw4)									// Horizontal Scale Factor, 1/8 down scale
#define	HwCDS_SFV_1					HwZERO										// Vertical Scale Factor, 1/1 down scale
#define	HwCDS_SFV_2					Hw2											// Vertical Scale Factor, 1/2 down scale
#define	HwCDS_SFV_4					Hw3											// Vertical Scale Factor, 1/4 down scale
#define	HwCDS_SFV_8					(Hw3|Hw2)									// Vertical Scale Factor, 1/8 down scale
#define	HwCDS_SEN_EN				Hw0											// Scale enable, 0:Disable, 1:enable

// CMOSIF Capture mode1
#define	HwCCM1_ENCNUM				0xF0000000									// Encode INT number (using CAP mode) [31:28], value area (0~15), Encode interrupt number
#define	HwCCM1_ROLNUMV				0x0F000000									// Rolling number in V (using CAP mode) [27:24], value area (0~15), Rolling number
#define	HwCCM1_ROLNUMU				0x00F00000									// Rolling number in U (using CAP mode) [23:20], value area (0~15), Rolling number
#define	HwCCM1_ROLNUMY				0x000F0000									// Rolling number in Y (using CAP mode) [19:16], value area (0~15), Rolling number
#define	HwCCM1_CB					Hw10										// Capture Busy,	0:-, 1:Capture busy
#define	HwCCM1_EIT					Hw9											// Encodig INT count,	0:Always 1 pulse, 1:Counting encoding INT
#define	HwCCM1_UES					Hw8											// Using Encoding Start Address,	0:disable, 1:Enable
#define	HwCCM1_SKIPNUM				0x000000F0									// Skip frame number (using CAP mode) [7:4], value area (0~15), Skip frame number
#define	HwCCM1_RLV					Hw3											// Rolling address V,	0:disable, 1:Enable
#define	HwCCM1_RLU					Hw2											// Rolling address U,	0:disable, 1:Enable
#define	HwCCM1_RLY					Hw1											// Rolling address Y,	0:disable, 1:Enable
#define	HwCCM1_CAP					Hw0											// Image Capture,	0:Normal, 1:Image Capture

// CMOSIF Capture mode2
#define	HwCCM2_VCNT					0x000000F0									// Description (Using CAP mode) [7:4], Threshold line counter in interrupt 1:16 line, 2:32 line, 3: 48 line...
#define	HwCCM2_VEN					Hw0											// VCNT folling enable (Using CAP mode) 0:Normal(?) Disalbe아닌가?, 1:Enable

// CMOSIF R2Y confiquration
#define	HwCR2Y_FMT					(Hw4|Hw3|Hw2|Hw1)							// FMT[4:1]	0000 -> Input format 16bit 565RGB(RGB sequence) 자세한 사항 750A CIF SPEC. 1-22
#define	HwCR2Y_EN					Hw0											// R2Y Enable,	0:disable, 1:Enable

// CMOSIF Current Line Count
#define	HwCCLC_LCNT					0x0000FFFF									// LCNT[15:0]	Current Line Count



/***********************************************************************
*	 9-2. Effect Register Define			(Base Addr = 0xF0230100)
************************************************************************/
#define	HwCEM_BASE					*(volatile unsigned long *)0xF0230100  //W/R  0x00000000  Effect mode register 
#define HwCEM						((PEFFECT)&HwCEM_BASE)

// CMOSIF Effect mode
#define	HwCEM_UVS					Hw15										// UV Swap	0:u-v-u-v sequence, 1:v-u-v-u sequence
#define	HwCEM_VB					Hw14										// V Bias (V channel value offset),	0:disable, 1:Enable 
#define	HwCEM_UB					Hw13										// U Bias (U channel value offset),	0:disable, 1:Enable
#define	HwCEM_YB					Hw12										// Y Bias (Y channel value offset),	0:disable, 1:Enable
#define	HwCEM_YCS					Hw11										// YC Swap	0:u-y-v-y sequence, 1:y-u-y-v sequence
#define	HwCEM_IVY					Hw10										// Invert Y,	0:disable, 1:Enable 
#define	HwCEM_STC					Hw9											// Strong C,	0:disable, 1:Enable 
#define	HwCEM_YCL					Hw8											// Y Clamp (Y value clipping),	0:disable, 1:Enable 
#define	HwCEM_CS					Hw7											// C Select (Color filter),	0:disable, 1:Enable(Color filter)	
#define	HwCEM_SKT					Hw6											// Sketch Enable,	0:disable, 1:Enable 
#define	HwCEM_EMM					Hw5											// Emboss mode,	0:Positive emboss, 1:Negative emboss
#define	HwCEM_EMB					Hw4											// Emboss,	0:disable, 1:Enable	
#define	HwCEM_NEGA					Hw3											// Negative mode,	0:disable, 1:Enable 
#define	HwCEM_GRAY					Hw2											// Gray mode,	0:disable, 1:Enable 
#define	HwCEM_SEPI					Hw1											// Sepia mode,	0:disable, 1:Enable	
#define	HwCEM_NOR					Hw0											// Normal mode,	0:Effect mode, 1:Normal mode 

// CMOSIF Sepia UV Setting
#define	HwHwCSUV_SEPIA_U			0x0000FF00									// SEPIA_U[15:8] U channel threshold value for sepia
#define	HwHwCSUV_SEPIA_V			0x000000FF									// SEPIA_V[7:0] V channel threshold value for sepia

// CMOSIF Color selection
#define	HwCCS_USTART				0xFF000000									// USTART [31:24]	Color filter range start point of U channel
#define	HwCCS_UEND					0x00FF0000									// UEND	[23:16]	Color filter range end point of U channel
#define	HwCCS_VSTART				0x0000FF00									// VSTART [15:8]	Color filter range start point of V channel
#define	HwCCS_VEND					0x000000FF									// VEND	[7:0]	 Color filter range end point of V channel

// CMOSIF H-filter coefficent
#define	HwCHFC_COEF0				0x00FF0000									// COEF0	[23:16] Horizontal filter coefficient0 for emboss or sketch 
#define	HwCHFC_COEF1				0x0000FF00									// COEF1	[15:8] Horizontal filter coefficient1 for emboss or sketch 
#define	HwCHFC_COEF2				0x000000FF									// COEF2	[7:0] Horizontal filter coefficient2 for emboss or sketch 

// CMOSIF Sketch threshold
#define	HwCST_THRESHOLD				0x000000FF									// Sketch [7:0] Sketch threshold

// CMOSIF Clamp threshold
#define	HwCCT_THRESHOLD				0x000000FF									// Clamp [7:0] Clamp threshold

// CMOSIF BIAS
#define	HwCBR_YBIAS					0x00FF0000									// Y_BIAS [23:16] Y value offset
#define	HwCBR_UBIAS					0x0000FF00									// U_BIAS [15:8]	U value offset
#define	HwCBR_VBIAS					0x000000FF									// V_BIAS [7:0]	V value offset

// CMOSIF Image size
#define	HwCEIS_HSIZE				0x0FFF0000									// HSIZE [26:16]	Horizontal size of input image
#define	HwCEIS_VSIZE				0x00000FFF									// VSIZE [10:0]	Vertical size of input image

#define HwCIC_H2H_WAIT                      0xFFFF0000       // H2H_WAIT [31:16]   Horizontal sync (hs)to hs wait cycle
#define HwCIC_STB_CYCLE                      0x0000FF00      // STB_CYCLE [15:8]  CCIR strobe cycle,  Minimum Value of STB_CYCLE is 4.
#define HwCIC_INP_WAIT                        (Hw6|Hw5|Hw4)      // INP_WAIT [6:4]     ???????????????
#define HwCIC_INPR                                Hw3     // ???????????????
#define HwCIC_FA                                   Hw2     // Flush all
#define HwCIC_INE                                  Hw1     // Inpath Enalbe,   0:disable, 1:Enable 
#define HwCIC_INP                                  Hw0     // Inpath Mode,   0:Camera mode, 1:Memory mode

// Y값은 32전체를 다 세팅하도록 하고 U, V값은 상위 4bit는 생략해도 cif에서 address를 추정하는 것이 가능하다.
//	HwCISA1_SRC_BASE는 Y값만 4비트 더 존재해서 구별해 놓은 것임 ,, 실제로 사용할 경우 그냥 32 address를 할당 한다. 
// CMOSIF INPATH Source address in Y channel
#define	HwCISA1_SRC_BASE			0xF0000000									// SRC_BASE [31:28] Source base address (31 down to 28 bit assign in base address)
#define	HwCISA1_SRC_BASE_Y			0x0FFFFFFF									// SRC_BASE_Y [27:0] Source base address in Y channel (27 down to 0 bit assign in bass address)

// CMOSIF INPATH Source address in U channel
#define	HwCISA2_SRC_TYPE_422SEQ0	HwZERO										// 0: (4:2:2 SEQ0)
#define	HwCISA2_SRC_TYPE_422SEQ1	Hw28										// 1: (4:2:2 SEQ1)
#define	HwCISA2_SRC_TYPE_422SEPA	Hw29										// 2: (4:2:2 Separate)
#define	HwCISA2_SRC_TYPE_420SEPA	(Hw29|Hw28)									// 3: (4:2:0 Separate)
#define	HwCISA2_SRC_BASE_U			0x0FFFFFFF									// SRC_BASE_U [27:0] Source base address in U channal (27 down to 0 bit assign in base address)

// CMOSIF INPATH Source address in V channel
#define	HwCISA3_SRC_BASE_V			0x0FFFFFFF									// SRC_BASE_V [27:0] Source base address in V channal (27 down to 0 bit assign in base address)


// CMOSIF INPATH Source image offset
//#define	HwCISO_SRC_OFFSET_H			0x0FFF0000									// SRC_OFFSET_H [27:16] source address offset in H
//#define	HwCISO_SRC_OFFSET_V			0x00000FFF									// SRC_OFFSET_V [11:0]	source address offset in V
#define	HwCISO_SRC_OFFSET_Y			0x0FFF0000									// SRC_OFFSET_Y [27:16] source address offset in Y channel
#define	HwCISO_SRC_OFFSET_C			0x00000FFF									// SRC_OFFSET_C [11:0]	source address offset in C channel

// CMOSIF INPATH Source image size
#define	HwCISS_SRC_HSIZE			0x0FFF0000									// SRC_HSIZE [27:16] Horizontal size in source image
#define	HwCISS_SRC_VSIZE			0x00000FFF									// SRC_VSIZE [11:0]	Vertical size in source image


// CMOSIF INPATH Destination image size
#define	HwCIDS_DST_HSIZE			0x0FFF0000									// DST_HSIZE [27:16] Horizontal size in destination image
#define	HwCIDS_DST_VSIZE			0x00000FFF									// DST_VSIZE [11:0]	Vertical size in destination image

// HSCALE = SRC_HSIZE*256/DST_HSIZE
// VSCALE = SRC_VSIZE*256/DST_VSIZE
// CMOSIF INPATH Target scale
#define	HwCIS_HSCALE				0x3FFF0000									// HSCALE [29:16] Horizontal scale factor
#define	HwCIS_VSCALE				0x00003FFF									// VSCALE [13:0]	Vertical scale factor



/***********************************************************************
*	 9-3. Scaler Register Define			(Base Addr = 0xF0230200)
************************************************************************/
#define	HwCSC_BASE					*(volatile unsigned long *)0xF0230200  //W/R  0x00000000  Scaler configuration 
#define HwCSC						((PCIFSACLER)&HwCSC_BASE)

// Scaler configuration
#define	HwSCC_EN					Hw0											// Scaler Enable	0:disable, 1:Enable 

// HSCALE = SRC_HSIZE*256/DST_HSIZE
// VSCALE = SRC_VSIZE*256/DST_VSIZE
// Scale factor
#define	HwSCSF_HSCALE				0x3FFF0000									// HSCALE [29:16] Horizontal scale factor
#define	HwSCSF_VSCALE				0x00003FFF									// VSCALE [13:0]	Vertical scale factor

// Image offset
#define	HwSCSO_OFFSET_H				0x0FFF0000									// H [27:16] Horizontal offset
#define	HwSCSO_OFFSET_V				0x00000FFF									// V [11:0]	Vertical offset

// Source image size
#define	HwSCSS_HSIZE				0x0FFF0000									// H [27:16] Horizontal size in source image
#define	HwSCSS_VSIZE				0x00000FFF									// V [11:0]	Vertical size in source image

// Destination image size
#define	HwSCDS_HSIZE				0x0FFF0000									// H [27:16] Horizontal size in destination image
#define	HwSCDS_VSIZE				0x00000FFF									// V [11:0]	Vertical size in destination image

/***********************************************************************
*   10. Video and Image  Quality Enhancer Register Define	(Base Addr = 0xF0230200)
************************************************************************/
#define HwVIQE_BASE                 *(volatile unsigned long *)0xF0252000

/***********************************************************************
*   11. LVDS Register Define                	(Base Addr = 0xF0230200)
************************************************************************/
#define HwDDI_CONFIG_BASE           *(volatile unsigned long *)0xF0251000
#define HwDDI_CONFIG				((PDDICONFIG)&HwDDI_CONFIG_BASE)
#define HwLVDS						((PDDICONFIG)&HwDDI_CONFIG_BASE)
#define HwDDI_CONFIG				((PDDICONFIG)&HwDDI_CONFIG_BASE)

// HDMI Control register
#define	HwDDIC_HDMI_CTRL_SEL_LCDC1	Hw15
#define	HwDDIC_HDMI_CTRL_SEL_LCDC0	HwZERO
#define	HwDDIC_HDMI_CTRL_EN			Hw14
#define	HwDDIC_HDMI_CTRL_RST_HDMI	Hw0
#define	HwDDIC_HDMI_CTRL_RST_SPDIF	Hw1
#define	HwDDIC_HDMI_CTRL_RST_TMDS	Hw2
#define	HwDDIC_HDMI_CTRL_RST_NOTUSE	Hw3

// Power Down
#define	HwDDIC_PWDN_HDMI			Hw8											// HDMI Interface
#define	HwDDIC_PWDN_DDIC			Hw7											// DDIBUS Cache
#define	HwDDIC_PWDN_MSCL1			Hw6											// Memory Scaler 1
#define	HwDDIC_PWDN_MSCL0			Hw5											// Memory Scaler 0
#define	HwDDIC_PWDN_LCDSI			Hw4											// LCDSI Interface
#define	HwDDIC_PWDN_LCDC1			Hw3											// LCD 1 Interface
#define	HwDDIC_PWDN_LCDC0			Hw2											// LCD 0 Interface
#define	HwDDIC_PWDN_VIQE			Hw1											// Video Image Quality Enhancer
#define	HwDDIC_PWDN_CIF				Hw0											// Camera Interface

// Soft Reset
#define	HwDDIC_SWRESET_HDMI			Hw8											// HDMI Interface
#define	HwDDIC_SWRESET_DDIC			Hw7											// DDIBUS Cache
#define	HwDDIC_SWRESET_MSCL1		Hw6											// Memory Scaler 1
#define	HwDDIC_SWRESET_MSCL0		Hw5											// Memory Scaler 0
#define	HwDDIC_SWRESET_LCDSI		Hw4											// LCDSI Interface
#define	HwDDIC_SWRESET_LCDC1		Hw3											// LCD 1 Interface
#define	HwDDIC_SWRESET_LCDC0		Hw2											// LCD 0 Interface
#define	HwDDIC_SWRESET_VIQE			Hw1											// Video Image Quality Enhancer
#define	HwDDIC_SWRESET_CIF			Hw0											// Camera Interface

#define HwDDI_CACHE_BASE            *(volatile unsigned long *)0xF0250000
#define HwDDI_CACHE					((PDDICACHE)&HwDDI_CACHE_BASE)


// DDI CACHE Control
#define	HwDDIC_CTRL_BW				Hw31
#define	HwDDIC_CTRL_CIF_DMA			Hw25
#define	HwDDIC_CTRL_VIQE_DMA2_2		Hw24
#define	HwDDIC_CTRL_VIQE_DMA2_1		Hw23
#define	HwDDIC_CTRL_VIQE_DMA2_0		Hw22
#define	HwDDIC_CTRL_VIQE_DMA1_2		Hw21
#define	HwDDIC_CTRL_VIQE_DMA1_1		Hw20
#define	HwDDIC_CTRL_VIQE_DMA1_0		Hw19
#define	HwDDIC_CTRL_VIQE_DMA0_2		Hw18
#define	HwDDIC_CTRL_VIQE_DMA0_1		Hw17
#define	HwDDIC_CTRL_VIQE_DMA0_0		Hw16
#define	HwDDIC_CTRL_MSCL1_DMA2		Hw15
#define	HwDDIC_CTRL_MSCL1_DMA1		Hw14
#define	HwDDIC_CTRL_MSCL1_DMA0		Hw13
#define	HwDDIC_CTRL_MSCL0_DMA2		Hw12
#define	HwDDIC_CTRL_MSCL0_DMA1		Hw11
#define	HwDDIC_CTRL_MSCL0_DMA0		Hw10
#define	HwDDIC_CTRL_LCD1_DMA2		Hw9
#define	HwDDIC_CTRL_LCD1_DMA1		Hw8
#define	HwDDIC_CTRL_LCD1_DMA0_2		Hw7
#define	HwDDIC_CTRL_LCD1_DMA0_1		Hw6
#define	HwDDIC_CTRL_LCD1_DMA0_0		Hw5
#define	HwDDIC_CTRL_LCD0_DMA2		Hw4
#define	HwDDIC_CTRL_LCD0_DMA1		Hw3
#define	HwDDIC_CTRL_LCD0_DMA0_2		Hw2
#define	HwDDIC_CTRL_LCD0_DMA0_1		Hw1
#define	HwDDIC_CTRL_LCD0_DMA0_0		Hw0

// DDI CACHE Configuration
#define	HwDDIC_CFG_CIF_DMA			(25)
#define	HwDDIC_CFG_VIQE_DMA2_2		(24)
#define	HwDDIC_CFG_VIQE_DMA2_1		(23)
#define	HwDDIC_CFG_VIQE_DMA2_0		(22)
#define	HwDDIC_CFG_VIQE_DMA1_2		(21)
#define	HwDDIC_CFG_VIQE_DMA1_1		(20)
#define	HwDDIC_CFG_VIQE_DMA1_0		(19)
#define	HwDDIC_CFG_VIQE_DMA0_2		(18)
#define	HwDDIC_CFG_VIQE_DMA0_1		(17)
#define	HwDDIC_CFG_VIQE_DMA0_0		(16)
#define	HwDDIC_CFG_MSCL1_DMA2		(15)
#define	HwDDIC_CFG_MSCL1_DMA1		(14)
#define	HwDDIC_CFG_MSCL1_DMA0		(13)
#define	HwDDIC_CFG_MSCL0_DMA2		(12)
#define	HwDDIC_CFG_MSCL0_DMA1		(11)
#define	HwDDIC_CFG_MSCL0_DMA0		(10)
#define	HwDDIC_CFG_LCD1_DMA2		(9)
#define	HwDDIC_CFG_LCD1_DMA1		(8)
#define	HwDDIC_CFG_LCD1_DMA0_2		(7)
#define	HwDDIC_CFG_LCD1_DMA0_1		(6)
#define	HwDDIC_CFG_LCD1_DMA0_0		(5)
#define	HwDDIC_CFG_LCD0_DMA2		(4)
#define	HwDDIC_CFG_LCD0_DMA1		(3)
#define	HwDDIC_CFG_LCD0_DMA0_2		(2)
#define	HwDDIC_CFG_LCD0_DMA0_1		(1)
#define	HwDDIC_CFG_LCD0_DMA0_0		(0)	


#define	DDIC_LCD0_DMA0_0	0
#define	DDIC_LCD0_DMA0_1	1
#define	DDIC_LCD0_DMA0_2	2
#define	DDIC_LCD0_DMA1		3
#define	DDIC_LCD0_DMA2		4
#define	DDIC_LCD1_DMA0_0	5
#define	DDIC_LCD1_DMA0_1	6
#define	DDIC_LCD1_DMA0_2	7
#define	DDIC_LCD1_DMA1		8
#define	DDIC_LCD1_DMA2		9
#define	DDIC_MSC0_DMA_0		10
#define	DDIC_MSC0_DMA_1		11
#define	DDIC_MSC0_DMA_2		12
#define	DDIC_MSC1_DMA_0		13
#define	DDIC_MSC1_DMA_1		14
#define	DDIC_MSC1_DMA_2		15
#define	DDIC_VE_DMA0_0		16
#define	DDIC_VE_DMA0_1		17
#define	DDIC_VE_DMA0_2		18
#define	DDIC_VE_DMA1_0		19
#define	DDIC_VE_DMA1_1		20
#define	DDIC_VE_DMA1_2		21
#define	DDIC_VE_DMA2_0		22
#define	DDIC_VE_DMA2_1		23
#define	DDIC_VE_DMA2_2		24
#define	DDIC_CIF_DMA		25
#define	DDIC_BW				31


#define HwDDIC_CFG_MASK			(0x1F)
#define	HwDDIC_CFG26(X)			((X)<<16)
#define	HwDDIC_CFG27(X)			((X)<<24)
#define	HwDDIC_CFG28(X)			((X))
#define	HwDDIC_CFG29(X)			((X)<<8)
#define	HwDDIC_CFG30(X)			((X)<<16)
#define	HwDDIC_CFG31(X)			((X)<<24)


/*******************************************************************************
*	 TCC9200_DataSheet_PART 7 VIDEO BUS_V0.00 Dec.11 2008
********************************************************************************/
/***********************************************************************
*   4. VIDEO CODEC Register Define                	(Base Addr = 0x0xF0700000)
************************************************************************/
#define HwVIDEOCODEC_BASE           *(volatile unsigned long *)0xF0700000
#define HwVIDEOCACHE_BASE			*(volatile unsigned long *)0xF0701000
#define HwVIDEOCACHE				((PVIDEOCACHE)&HwVIDEOCACHE_BASE)

// Video Cache Ctrl
#define	HwVIDEOCACHE_PWRASVE 			       Hw31								// Video Cache
#define	HwVIDEOCACHE_CACHEON				Hw0									// Video Codec

// Video Cache Region Enable
#define	HwVIDEOCACHE_WRITE3_EN				Hw13								// Read reasion 3  Enable
#define	HwVIDEOCACHE_READ3_EN				Hw12								// Write reasion 3  Enable
#define	HwVIDEOCACHE_WRITE2_EN				Hw9									// Read reasion 2  Enable
#define	HwVIDEOCACHE_READ2_EN				Hw8								       // Write reasion 2  Enable
#define	HwVIDEOCACHE_WRITE1_EN				Hw5									// Read reasion 1  Enable
#define	HwVIDEOCACHE_READ1_EN				Hw4									// Write reasion 1  Enable
#define	HwVIDEOCACHE_WRITE0_EN				Hw1									// Read reasion 0  Enable
#define	HwVIDEOCACHE_READ0_EN				Hw0									// Write reasion 0  Enable

// VWB Ctrl
#define	HwVIDEOCACHE_VWB_TIMEOUT_VALUE	0xFFFF0000							// Timeout value.
#define	HwVIDEOCACHE_VWB_WCBV				Hw5									// Write cache bvalid option.
#define	HwVIDEOCACHE_VWB_RESET				Hw3									// Reset.
#define	HwVIDEOCACHE_VWB_STATUS				Hw2								       // Read only drain status.
#define	HwVIDEOCACHE_VWB_TIMEOUT_EN		Hw1									// Timeout enable.
#define	HwVIDEOCACHE_VWB_DRAIN_EN			Hw0									// Drain enable


#define HwVIDEOBUS_BASE				*(volatile unsigned long *)0xF0702000
#define HwVIDEOBUS					((PVIDEOCODEC)&HwVIDEOBUS_BASE)

// Power Down
#define	HwVIDEOC_PWDN_VIDEO_CACHE 			Hw3									// Video Cache
#define	HwVIDEOC_PWDN_VIDEO_CODEC			Hw2									// Video Codec
#define	HwVIDEOC_PWDN_JPEG_DECODER			Hw1									// Jpeg decoder
#define	HwVIDEOC_PWDN_JPEG_ENCODER			Hw0									// Jpeg Encoder

// Soft Reset
#define	HwVIDEOC_SWRESET_VIDEO_CACHE 		Hw3									// Video Cache
#define	HwVIDEOC_SWRESET_VIDEO_CODEC		Hw2									// Video Codec
#define	HwVIDEOC_SWRESET_JPEG_DECODER		Hw1									// Jpeg decoder
#define	HwVIDEOC_SWRESET_JPEG_ENCODER		Hw0									// Jpeg Encoder


/***********************************************************************
*   5. JPEG CODEC Register Define                	(Base Addr = 0x0xF0710000/0xF0720000)
************************************************************************/
#define HwJPEGDECODER_BASE          *(volatile unsigned long *)0xF0710000
#define HwJPEGENCODER_BASE          *(volatile unsigned long *)0xF0720000
#define HwJPEGDEC					((PJPEGDECODER)&HwJPEGDECODER_BASE)
#define HwJPEGENC					((PJPEGENCODER)&HwJPEGENCODER_BASE)
#define HwVIDEOCACHE_BASE           *(volatile unsigned long *)0xF0701000


/* R/W, JPEG Codec Mode Register */
#define	HwJP_MOD_JPC					HwZERO								/* JPEG Encoder Mode */
#define	HwJP_MOD_JPD					Hw16								/* JPEG Decoder Mode */
#define	HwJP_MOD_MASTER					HwZERO								/* Master Mode */  
#define	HwJP_MOD_SLAVE					Hw0									/* Slave Mode */

/* R/W, Interrupt Mask Register */
#define HwJP_INT_MASK_OPERATION_END		Hw0       								/* Encode/Decode Complete */
#define HwJP_INT_MASK_ERROR             Hw1       								/* Encode/Decode Error */
#define HwJP_INT_MASK_OUTPUT_FIFO       Hw2       								/* Output FIFO Buffer Status */
#define HwJP_INT_MASK_INPUT_FIFO		Hw3       								/* Input FIFO Buffer Status */
#define HwJP_INT_MASK_CODED_BUFFER		Hw4       								/* Output Buffer's Full/Empty Status */
#define HwJP_INT_MASK_YBUF_ROLLING      Hw5

/* R/W, Polling or Interrupt Mode Selection Register */
#define	HwJP_TRG_MOD_POLL				HwZERO								/* Polling Mode. No Interrupt Generated */
#define	HwJP_TRG_MOD_INT				Hw0									/* Interrupt Mode */

/* R/W, Image Format Information Register */
#define	HwJP_CHROMA_YONLY				HwZERO								/* Y Only */
#define	HwJP_CHROMA_420					Hw0									/* YUV420 (Y,U,V Separated Mode) */
#define	HwJP_CHROMA_422					Hw1									/* YUV422 (Y,U,V Separated Mode) */
#define	HwJP_CHROMA_444					(Hw0|Hw1)							/* YUV444 (Y,U,V Separated Mode) */
#define	HwJP_CHROMA_422S				Hw2									/* YUV422S (Y,U,V Separated Mode) */

/*  DECODE ONLY : R/W, Decoder Output Scaling Register*/ 
#define	HwJPD_OUT_SCL_4					Hw0									/* 1/4 (Area Ratio) */
#define	HwJPD_OUT_SCL_16				Hw1									/* 1/16 (Area Ratio) */

/* W, Codec Start Command Register */
#define	HwJP_START_RUN					Hw0									/*  Codec Start   */

/* R, Interrupt Flag Register */
#define	HwJP_INT_FLAG_YBUF_ROLLING		Hw5										/* Y Buffer Rolling Interrupt Status */
#define	HwJP_INT_FLAG_CODED_BUF_STAT	Hw4									/* Coded Buffer Status */
#define	HwJP_INT_FLAG_IFIFO_STAT		Hw3									/* Input FIFO Status */
#define	HwJP_INT_FLAG_OFIFO_STAT		Hw2									/* Output FIFO Status */
#define	HwJP_INT_FLAG_DECODING_ERR		Hw1									/* Decoding Error */
#define	HwJP_INT_FLAG_JOB_FINISHED		Hw0									/* Job Finished */

/*******************************************************************************
*	 TCC9200_DataSheet_PART 8 GRAPHIC BUS_V0.00 Dec.11 2008
********************************************************************************/
/***********************************************************************
*	 4. Overlay Mixer Register Define	(Base Addr = 0xF6000000)
************************************************************************/
#define HwOVERLAYMIXER_BASE         *(volatile unsigned long *)0xF0010000
#define HwOVERLAYMIXER				((POVERLAYMIXER)&HwOVERLAYMIXER_BASE)

// Front-End Channel 0 Control
#define	HwGE_FCHO_OPMODE			(Hw8+Hw9+Hw10)								// Operation Mode
#define	HwGE_FCHO_SDFRM				(Hw0+Hw1+Hw2+Hw3+Hw4)						// Source Data Format

// Front-End Channel 1 Control
#define	HwGE_FCH1_OPMODE			(Hw8+Hw9+Hw10)								// Operation Mode
#define	HwGE_FCH1_SDFRM				(Hw0+Hw1+Hw2+Hw3+Hw4)						// Source Data Format

// Front-End Channel 2 Control
#define	HwGE_FCH2_OPMODE			(Hw8+Hw9+Hw10)								// Operation Mode
#define	HwGE_FCH2_SDFRM				(Hw0+Hw1+Hw2+Hw3+Hw4)						// Source Data Format

// Source Control
#define Hw2D_SCTRL_S2_ARITHMODE 			(Hw27+Hw26+Hw25)
#define Hw2D_SCTRL_S1_ARITHMODE 			(Hw24+Hw23+Hw22)
#define Hw2D_SCTRL_S0_ARITHMODE 			(Hw21+Hw20+Hw19)
#define Hw2D_SCTRL_S2_Y2REN 				(Hw18)
#define Hw2D_SCTRL_S1_Y2REN 				(Hw17)
#define Hw2D_SCTRL_S0_Y2REN 				(Hw16)
#define Hw2D_SCTRL_S2_Y2RMODE 				(Hw14+Hw13)
#define Hw2D_SCTRL_S1_Y2RMODE 				(Hw12+Hw11)
#define Hw2D_SCTRL_S0_Y2RMODE 				(Hw10+Hw9)
#define Hw2D_SCTRL_S2_CHROMAEN				(Hw8)
#define Hw2D_SCTRL_S1_CHROMAEN				(Hw7)
#define Hw2D_SCTRL_S0_CHROMAEN				(Hw6)
#define Hw2D_SCTRL_S2_SEL					(Hw5+Hw4)
#define Hw2D_SCTRL_S1_SEL					(Hw3+Hw2)
#define Hw2D_SCTRL_S0_SEL					(Hw1+Hw0)

// Source Operator Pattern
#define	HwGE_OP_ALL					(HwGE_ALPHA + HwGE_PAT_RY + HwGE_PAT_GU + HwGE_PAT_BV)
#define	HwGE_ALPHA					(Hw24+Hw25+Hw26+Hw27+Hw28+Hw29+Hw30+Hw31)						// ALPHA VALUE
#define	HwGE_PAT_RY					(Hw16+Hw17+Hw18+Hw19+Hw20+Hw21+Hw22+Hw23)	// Pattern Value RED,   Y
#define	HwGE_PAT_GU					(Hw8+Hw9+Hw10+Hw11+Hw12+Hw13+Hw14+Hw15)		// Pattern Value GREEN, U
#define	HwGE_PAT_BV					(Hw0+Hw1+Hw2+Hw3+Hw4+Hw5+Hw6+Hw7)			// Pattern Value BULE,  V

// Source Operation Control
#define	HwGE_OP_CTRL_ASEL1			(Hw23+Hw24)
#define	HwGE_OP_CTRL_CSEL1			(Hw21+Hw22)									// Chroma-key Source Selection
#define	HwGE_OP_CTRL_OP1_MODE		(Hw16+Hw17+Hw18+Hw19+Hw20)					// Operation 1 Mode
#define	HwGE_OP_CTRL_ASEL0			(Hw7+Hw8)
#define	HwGE_OP_CTRL_CSEL0			(Hw5+Hw6)									// Chroma-key Source Selection
#define	HwGE_OP_CTRL_OP0_MODE		(Hw0+Hw1+Hw2+Hw3+Hw4)						// Operation 0 Mode

// Back -End Channel Control
#define	HwGE_BCH_DCTRL_MABC			Hw21
#define	HwGE_BCH_DCTRL_YSEL			Hw18										// YUV4:4:4 to YUVx:x:x Y Control
#define	HwGE_BCH_DCTRL_XSEL			(Hw16+Hw17)									// YUV4:4:4 to YUVx:x:x X Control
#define	HwGE_BCH_DCTRL_CEN			Hw15										// Destination Format Converter Control
#define	HwGE_BCH_DCTRL_CMODE		(Hw13+Hw14)									// RGBtoYUV Converter Type
#define	HwGE_BCH_DCTRL_DSUV			Hw11
#define	HwGE_BCH_DCTRL_OPMODE		(Hw8+Hw9+Hw10)								// Operation Mode COPY, MIRROR, ROTATE 
#define	HwGE_BCH_DCTRL_DOP			Hw6	
#define	HwGE_BCH_DCTRL_DEN			Hw5	
#define	HwGE_BCH_DCTRL_DDFRM		(Hw0+Hw1+Hw2+Hw3+Hw4)						// Destination Data Format

// Graphic Engine Control
#define	HwGE_GE_INT_EN				Hw16										// Graphic Engine Interrupt Enable
#define	HwGE_GE_CTRL_EN				(Hw0+Hw1+Hw2)								// Graphic Engine Enable

// Graphic Engine Interrupt Request
#define	HwGE_GE_IREQ_FLG			Hw16										// Graphic Engine Flag Bit
#define	HwGE_GE_IREQ_IRQ			Hw0											// Graphic Engine Interrupt Request

/*******************************************************************************
*	 5-1. 2D/3D GPU
*
*	 Pixel Processor Register Map Register Define   (Base Addr = 0xF0000000)
********************************************************************************/
#define HwPIXELPROCESSOR_BASE       *(volatile unsigned long *)0xF0000000

/*******************************************************************************
*	 5-2. Geometry Processor Register Map Register Define   (Base Addr = 0xF0000000)
********************************************************************************/
#define HwGEOMETRYPROCESSOR_BASE    *(volatile unsigned long *)0xF0002000

/*******************************************************************************
*	 5-3. MMU Configuration Register Define   (Base Addr = 0xF0003000)
********************************************************************************/
#define HwMMUCONFIG_BASE            *(volatile unsigned long *)0xF0003000

/*******************************************************************************
*	 5-4. GRPBUS Configuration Register Define   (Base Addr = 0xF0004000)
********************************************************************************/
#define HwGRPBUS_BASE               *(volatile unsigned long *)0xF0004000
#define HwGRPBUS					((PGPUGRPBUSCONFIG)&HwGRPBUS_BASE)
#define	HwGRP_OM					Hw1		// Overlay mixer	
#define	HwGRP_GPU					Hw0		// 3D GPU power

/*******************************************************************************
*	 5-5. GRPBUS BWRAP Register Define   (Base Addr = 0xF0005000)
********************************************************************************/
#define HwGRPBUSBWRAP_BASE          *(volatile unsigned long *)0xF0005000


/************************************************************************
*	ETC Define
************************************************************************/
#define PLL_FREQ					5000000
#define PLL_FREQ1					2000000


#if defined(SDMMC_PLL1_USED) 
// CAM_MCLK, CIF_SCALER_CLK, BT_DUALSTACK, SD_MMC
#define PLL1_FREQ					1920000	
#else 

#if 0
	// CAM_MCLK, CIF_SCALER_CLK
	#if defined(FEATURE_CAMCLK_24MHz)
	#define PLL1_FREQ			1920000  
	#elif defined(FEATURE_CAMCLK_24MHz_S5K4BAFB_LG)
	#define PLL1_FREQ			2000000  
	#elif defined(FEATURE_CAMCLK_32MHz)
	#define PLL1_FREQ			1280000 
	#elif defined(FEATURE_CAMCLK_50MHz)
	#define PLL1_FREQ			2000000 
	#elif defined(FEATURE_CAMCLK_80MHz)
	#define PLL1_FREQ			3200000 
	#else
	#define PLL1_FREQ			1920000
	#endif
#endif

#endif



#endif						
