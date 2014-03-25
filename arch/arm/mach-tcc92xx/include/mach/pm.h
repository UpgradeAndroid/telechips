
#ifndef __MACH_TCC92XX_PM_H__
#define __MACH_TCC92XX_PM_H__

#include <mach/bsp.h>
#include <mach/TCC92x_Structures.h>
#include <mach/TCC92x_Physical.h>

#define BSP_SUSPEND_MASK      		0x424E4654 //"TFNB"
#define BSP_SUSPEND_KEY			0xE1
#define BSP_SLEEP_KEY			0xE0

#define SRAM_ADDR_STANDBY		0xF0800000//0xEFF00000
#define SRAM_ADDR_VAR			0xEFF01000
#define SRAM_FUNC_SIZE			0x600
#define SDRAM_STORE_ADDRESS		0x20300000

extern unsigned IO_ARM_SetCPSR(unsigned uCPSR);
extern void IO_CKC_EnterSRFInt(unsigned uEHIACK);
extern unsigned IO_ARM_DisableINT(void);
extern unsigned IO_ARM_EnableINT(void);
extern void IO_CKC_EnterSRFInt(unsigned uEHIACK);
extern void	IO_ARM_CleanCACHE(int);
extern void	IO_ARM_FlushCACHE(void);
extern unsigned DEV_mDDR_SetFreq(unsigned freq, unsigned *pll_value, unsigned *clk_value);

#define	SRAM_BASE			0x10000000
#define	SRAM_LIMIT			0x00004000
#define	SRAM_SIZE			0x00004000
#define	DTCM_BASE			0xA0000000
#define	DTCM_LIMIT			0xA0004000
#define	DTCM_SIZE			(DTCM_LIMIT - DTCM_BASE)
#define	ITCM_BASE			0x00000000
#define	ITCM_SIZE			0x00004000
#define	IO_USB_BUFFER0_BASE		(DTCM_BASE + 0x1800)
#define	IO_USB_BUFFER1_BASE		(DTCM_BASE + 0x1A00)
#define	IO_NFC_BUFFER0_BASE		(DTCM_BASE + 0x1C00)
#define	IO_NFC_BUFFER1_BASE		(DTCM_BASE + 0x1E00)

#ifndef hword_of
#define	hword_of(X)					( *(volatile unsigned short *)((X)) )
#endif

#ifndef byte_of
#define	byte_of(X)					( *(volatile unsigned char *)((X)) )
#endif
#ifndef word_of
#define	word_of(X)					( *(volatile unsigned int *)((X)) )
#endif


extern unsigned		uIO_CKC_Fpll[MAX_PLL_CH];		// Current PLL Frequency, 100Hz unit
extern unsigned		uIO_CKC_Fsys[HwCKC_SYS_MAX];	// Current System Clock Frequency, 100Hz unit
extern unsigned		uIO_CKC_Fclk[HwCKC_SYS_MAX];	// Current Generated Clock Frequency, 100Hz unit


typedef struct _TCC92X_REG_{
	unsigned int uMask;
	unsigned int SleepState_WakeAddr;
	unsigned int SleepState_SYSCTL;
	unsigned int SleepState_MMUTTB0;
	unsigned int SleepState_MMUTTB1;
	unsigned int SleepState_MMUTTBCTL;
	unsigned int SleepState_MMUDOMAIN;
	unsigned int SleepState_SVC_SP;
	unsigned int SleepState_SVC_SPSR;
	unsigned int SleepState_FIQ_SPSR;
	unsigned int SleepState_FIQ_R8;
	unsigned int SleepState_FIQ_R9;
	unsigned int SleepState_FIQ_R10;
	unsigned int SleepState_FIQ_R11;
	unsigned int SleepState_FIQ_R12;
	unsigned int SleepState_FIQ_SP;
	unsigned int SleepState_FIQ_LR;
	unsigned int SleepState_ABT_SPSR;
	unsigned int SleepState_ABT_SP;
	unsigned int SleepState_ABT_LR;
	unsigned int SleepState_IRQ_SPSR;
	unsigned int SleepState_IRQ_SP;
	unsigned int SleepState_IRQ_LR;
	unsigned int SleepState_UND_SPSR;
	unsigned int SleepState_UND_SP;
	unsigned int SleepState_UND_LR;
	unsigned int SleepState_SYS_SP;
	unsigned int SleepState_SYS_LR;	


	unsigned	uTemp[0x100];
	unsigned 	int lock;
	/*--------------------------------------------------------------
	 Internal Memory (SRAM & ITCM & DTCM)
	--------------------------------------------------------------*/
	char		uSRAM[SRAM_SIZE];		// max 16kbytes of SRAM
#ifndef _SLEEP_WITH_ITCM_BOOT_
	char		uITCM[ITCM_SIZE];		// max 16kbytes of ITCM
#endif
	char		uDTCM[DTCM_SIZE];		// max 16kbytes of DTCM

	/*--------------------------------------------------------------
	 PART2 - SMU & PMU
	--------------------------------------------------------------*/
	//CKC
	char		uCKC[sizeof(CKC)];

	//VPIC (VECTORED PRIORITY INTERRUPT CONTROLLER)
	char		uPIC[sizeof(PIC)];
	char		uVIC[sizeof(VIC)];

	//TIMER / COUNTER
	char		uTMR[sizeof(TIMER)];

	//PMU (POWER MANAGEMENT UNIT)
	char		uPMU[sizeof(PMU)];

	//SMU_I2C
	char		uSMUI2C_MASTER0[sizeof(SMUI2CMASTER)];
	char		uSMUI2C_MASTER1[sizeof(SMUI2CMASTER)];
	char		uSMUI2C_COMMON[sizeof(SMUI2CICLK)];

	/*--------------------------------------------------------------
	 PART3 - GPIO
	--------------------------------------------------------------*/
	char		uGPIO[sizeof(GPIO)];

	/*--------------------------------------------------------------
	 PART4 - CORE & MEMORY BUS
	--------------------------------------------------------------*/
	//SDR/DDR SDRAM Controller Registers
	char		uDRAM[sizeof(DRAM)];

	//DDR2 SDRAM Controller Registers
	char		uDRAMMX[sizeof(DRAM)];

	//SDRAM PHY Control Registers
	char		uDRAMPHY[sizeof(DRAMPHY)];

	//Miscellaneous Configuration Registers
	char		uDRAMMISC[sizeof(DRAMMISC)];

	//Memory Bus Configuration Registers
	char		uDRAMMEMBUS[sizeof(DRAMMEMBUS)];

	//Core Bus Configuration Registers
	char		uCORECFG[sizeof(MISCCOREBUS)];

	//Virtual MMU Table Registers
	char		uVMTREGION[sizeof(VMTREGION)];

	/*--------------------------------------------------------------
	 PART5 - IO BUS
	--------------------------------------------------------------*/

	//Memory Stick Host Controller
	char		uSMSHC[sizeof(SMSHC)];
	char		uSMSHCPORTCFG[sizeof(SMSHCPORTCFG)];

	//SD/SDIO/MMC/CE_ATA Host Controller 	
	char		uSDCORE0SLOT0[sizeof(SDHOST)];
	char		uSDCORE0SLOT1[sizeof(SDHOST)];
	char		uSDCORE1SLOT0[sizeof(SDHOST)];
	char		uSDCORE1SLOT1[sizeof(SDHOST)];
	char		uSDCHCTRL[sizeof(SDCHCTRL)];

	//NAND Flash Controller
	char		uNFC[sizeof(NFC)];

	//Static Memory Controller
	char		uSMC[sizeof(SMC)];

	//External Device Interface
	char		uEDI[sizeof(EDI)];

	//IDE Controller
	char		uIDE[sizeof(IDE)];

	//Audio DMA
	char		uADMA[sizeof(ADMA)];
	char		uADMA_DAI[sizeof(ADMADAI)];
	char		uADMA_CDIF[sizeof(ADMACDIF)];
	char		uADMA_SPDIFTX[sizeof(ADMASPDIFTX)];
	char		uADMA_SPDIFRX[sizeof(ADMASPDIFRX)];

	//DAI Interface Register
	char		uDAI[sizeof(DAI)];
	char		uCDIF[sizeof(CDIF)];

	//SPDIF TRANSMITTER
	char		uSPDIF[sizeof(SPDIF)];

	//USB 2.0 OTG Controller
	char		uUSB20OTG[sizeof(USB20OTG)];
	char		uUSBOTGCFG[sizeof(USBOTGCFG)];
	char		uUSBPHYCFG[sizeof(USBPHYCFG)];

	//External Host Interface
	char		uEHI0[sizeof(EHI)];
	char		uEHI1[sizeof(EHI)];

	//General Purpose Serial Bus
	char		uGPSB0[sizeof(GPSB)];
	char		uGPSB1[sizeof(GPSB)];
	char		uGPSB2[sizeof(GPSB)];
	char		uGPSB3[sizeof(GPSB)];
	char		uGPSB4[sizeof(GPSB)];
	char		uGPSB5[sizeof(GPSB)];
	char		uGPSBPORTCFG[sizeof(GPSBPORTCFG)];
	char		uGPSBPIDTABLE[sizeof(GPSBPIDTABLE)];

	//Transport Stream Interface
	char		uTSIF[sizeof(TSIF)];
	char		uTSIFPORTSEL[sizeof(TSIFPORTSEL)];

	//GPS
	//char		uGPS[sizeof(GPS)];

	//Remote Control Interface
	char		uREMOCON[sizeof(REMOTECON)];

	//I2C
	char		uI2CMASTER0[sizeof(I2CMASTER)];
	char		uI2CMASTER1[sizeof(I2CMASTER)];
	char		uI2CSLAVE[sizeof(I2CSLAVE)];
	char		uI2CSTATUS[sizeof(I2CSTATUS)];

	//UART
	char		uUART0[sizeof(UART)];
	char		uUART1[sizeof(UART)];
	char		uUART2[sizeof(UART)];
	char		uUART3[sizeof(UART)];
	char		uUART4[sizeof(UART)];
	char		uUART5[sizeof(UART)];
	char		uUARTPORTMUX[sizeof(UARTPORTMUX)];

	//DMA Controller
	char		uGDMA0[sizeof(GDMACTRL)];
	char		uGDMA1[sizeof(GDMACTRL)];
	char		uGDMA2[sizeof(GDMACTRL)];
	char		uGDMA3[sizeof(GDMACTRL)];

	//Real Time Clock
	char		uRTC[sizeof(RTC)];

	//TouchScreen ADC
	char		uTSADC[sizeof(TSADC)];

	//Error Correction Code
	char		uECC[sizeof(SLCECC)];

	//Multi-Protocla Encapsulation Forward Error Correction
	char		uMPEFEC[sizeof(MPEFEC)];

	//IOBUS Configuration Register
	char		uIOBUSCFG[sizeof(IOBUSCFG)];

	/*--------------------------------------------------------------
	 PART6 - DDI_BUS
	--------------------------------------------------------------*/
	//LCD
	char		uLCD[sizeof(LCDC)];
	char		uLCD1[sizeof(LCDC)];
	char		uLCDSI[sizeof(LCDSI)];

	//Memory To Memory Scaler
	char		uM2MSCALER0[sizeof(M2MSCALER)];
	char		uM2MSCALER1[sizeof(M2MSCALER)];

	//NTSCPAL
	char		uNTSCPAL[sizeof(NTSCPAL)];
	char		uNTSCPALOP[sizeof(NTSCPALOP)];

	//HDMI
	char		uHDMICTRL[sizeof(HDMICTRL)];
	char		uHDMICORE[sizeof(HDMICORE)];
	char		uHDMIAES[sizeof(HDMIAES)];
	char		uHDMISPDIF[sizeof(HDMISPDIF)];
	char		uHDMII2S[sizeof(HDMII2S)];
	char		uHDMICEC[sizeof(HDMICEC)];

	//CIF
	char		uCIF[sizeof(CIF)];

	//Effect
	char		uEFFECT[sizeof(EFFECT)];

	//Scaler
	char		uCIFSACLER[sizeof(CIFSACLER)];

	//Video & Image Quality Enhancer
	char		uVIQE[sizeof(VIQE)];

	//DDI_CONFIG
	char		uDDI_CONFIG[sizeof(DDICONFIG)];

	//DDI_CACHE
	char		uDDICACHE[sizeof(DDICACHE)];

	/*--------------------------------------------------------------
	 PART7 - VIDEO BUS
	--------------------------------------------------------------*/

	//Video Codec .. ???

	//Jpeg Codec
	char		uJPEGDECODER[sizeof(JPEGDECODER)];
	char		uJPEGENCODER[sizeof(JPEGENCODER)];

	/*--------------------------------------------------------------
	 PART8 - GRAPHIC BUS
	--------------------------------------------------------------*/

	//Overlay Mixer
	char		uOVERLAYMIXER[sizeof(OVERLAYMIXER)];

	//2D/3D GPU
	char		uGPUPIXELPROCESSOR[sizeof(GPUPIXELPROCESSOR)];
	char		uGPUGEOMETRYPROCESSOR[sizeof(GPUGEOMETRYPROCESSOR)];

	//GRPBUS Configuration
	char		uGPUGRPBUSCONFIG[sizeof(GPUGRPBUSCONFIG)];
	char		uGPUGRPBUSBWRAP[sizeof(GPUGRPBUSBWRAP)];
} TCC92X_REG, *pTCC92X_REG;


extern void IO_ARM_SaveREG(int sram_addr, unsigned int pSvae, void *);
extern void Awake_address(void);
#endif
