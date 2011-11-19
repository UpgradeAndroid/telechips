/****************************************************************************
*   FileName    : globals.h
*   Description : 
****************************************************************************
*
*   TCC Version : 1.0
*   Copyright (c) Telechips, Inc.
*   ALL RIGHTS RESERVED
*
****************************************************************************/

//using only global defines, macros.. etc - If you want using this file contact to RYU

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#ifdef __cplusplus
extern "C" {
#endif

//Log Level
#define TC_ERROR 	0x00000001
#define TC_LOG		0x00000002
#define TC_TRACE	0x00000004
#define	TC_DEBUG	0x00000008

	//system info
#define IOCTL_PLATFORM_TYPE                 (L"PLATFORM_TYPE")
#define IOCTL_PLATFORM_OEM                  (L"PLATFORM_OEM")

//------------------------------------------------------------------------------
//  Define:  IOCTL_PROCESSOR_VENDOR/NAME/CORE
//
//  Defines the processor information
//

#define IOCTL_PROCESSOR_VENDOR              (L"Telechips")
#define IOCTL_PROCESSOR_NAME                (L"TCC88xx")
#define IOCTL_PROCESSOR_CORE                (L"Cortex-A8")

//------------------------------------------------------------------------------
//
//  Define:  IOCTL_PROCESSOR_INSTRUCTION_SET
//
//  Defines the processor instruction set information
//
#define IOCTL_PROCESSOR_INSTRUCTION_SET     (0)
#define IOCTL_PROCESSOR_CLOCK_SPEED	    266*1000

//macro defines
/************************************************************************************************
*										 MACRO												   *
************************************************************************************************/
#ifndef BITSET
#define BITSET(X, MASK) 			( (X) |= (unsigned int)(MASK) )
#endif
#ifndef BITSCLR
#define BITSCLR(X, SMASK, CMASK)	( (X) = ((((unsigned int)(X)) | ((unsigned int)(SMASK))) & ~((unsigned int)(CMASK))) )
#endif
#ifndef BITCSET
#define BITCSET(X, CMASK, SMASK)	( (X) = ((((unsigned int)(X)) & ~((unsigned int)(CMASK))) | ((unsigned int)(SMASK))) )
#endif
#ifndef BITCLR
#define BITCLR(X, MASK) 			( (X) &= ~((unsigned int)(MASK)) )
#endif
#ifndef BITXOR
#define BITXOR(X, MASK) 			( (X) ^= (unsigned int)(MASK) )
#endif
#ifndef ISZERO
#define ISZERO(X, MASK) 			(  ! (((unsigned int)(X)) & ((unsigned int)(MASK))) )
#endif

#ifndef ISSET
#define	ISSET(X, MASK)				( (unsigned long)(X) & ((unsigned long)(MASK)) )
#endif


#ifndef ENABLE
#define ENABLE 1
#endif
#ifndef DISABLE
#define DISABLE 0
#endif
#ifndef NOCHANGE
#define NOCHANGE 2
#endif

#ifndef ON
#define ON		1
#endif
#ifndef OFF
#define OFF 	0
#endif

#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1
#endif

#define HwVMT_SZ(X) 							(((X)-1)*Hw12)
	#define SIZE_4GB								32
	#define SIZE_2GB								31
	#define SIZE_1GB								30
	#define SIZE_512MB								29
	#define SIZE_256MB								28
	#define SIZE_128MB								27
	#define SIZE_64MB								26
	#define SIZE_32MB								25
	#define SIZE_16MB								24
	#define SIZE_8MB								23
	#define SIZE_4MB								22
	#define SIZE_2MB								21
	#define SIZE_1MB								20
	#define HwVMT_REGION_AP_ALL 				(Hw11+Hw10)
	#define HwVMT_DOMAIN(X) 					((X)*Hw5)
	#define HwVMT_REGION_EN 					Hw9 							// Region Enable Register
	#define HwVMT_CACHE_ON						Hw3 							// Cacheable Register
	#define HwVMT_CACHE_OFF 					HwZERO
	#define HwVMT_BUFF_ON							Hw2 							// Bufferable Register
	#define HwVMT_BUFF_OFF							HwZERO

	#define HwVMT_REGION0_EN						Hw9 							// Region Enable Register
	#define HwVMT_REGION0_CA						Hw3 							// Cacheable Register
	#define HwVMT_REGION0_BU						Hw2 							// Bufferable Register

/************************************************************************************************
*										 ENUM												   *
************************************************************************************************/
/***************************************CLOCK****************************************************/
	enum
	{
		IDLE_PRIORITY = 0, // Don't Return IDLE_PRIORITY
		LOW_PRIORITY,
		MID_PRIORITY,
		HIGH_PRIORITY,
		MAX_PRIORITY,
	
		CLOCK_PRIORITY_NUM,
	//}stCKC_PRIORITY;
	};

//CKC Enum
	enum{ /* CLKCTRL Clock Source */
		DIRECTPLL0=0,
		DIRECTPLL1,
		DIRECTPLL2,
		DIRECTPLL3,
		DIRECTXIN,
		DIVIDPLL0,
		DIVIDPLL1,
		DIRECTXTIN,
		DIRECTPLL4,
		DIRECTPLL5,
		DIVIDPLL2,
		DIVIDPLL3,
		DIVIDPLL4,
		DIVIDPLL5,
		DIVIDXIN,
		DIVIDXTIN,
	};
	
	enum{ /* Peri. Clock Source */
		PCDIRECTPLL0=0,
		PCDIRECTPLL1,
		PCDIRECTPLL2,
		PCDIRECTPLL3,
		PCDIRECTXIN,
		PCDIVIDPLL0,
		PCDIVIDPLL1,
		PCDIVIDPLL2,
		PCDIVIDPLL3,
		PCDIRECTXTIN,
		PCEXITERNAL, // 10
		PCDIVIDXIN_HDMITMDS,
		PCDIVIDXTIN_HDMIPCLK,
		PCHDMI, 	// 27Mhz
		PCSATA, 	// 25Mhz
		PCUSBPHY,	// 48Mhz
		PCDIVIDXIN,
		PCDIVIDXTIN,
		PCDIRECTPLL4,
		PCDIRECTPLL5,
		PCDIVIDPLL4,
		PCDIVIDPLL5,
	};
	
	enum{ /* Peri. Clock Source */
		PDCO = 0,
		PDIVIDER,
	};
	
	enum {/* Peri. Name */
		PERI_TCX = 0, 
		PERI_TCT, 
		PERI_TCZ, 
		PERI_LCD0,	
		PERI_LCD1,	
		PERI_LCDSI0,
		PERI_LCDSI1,
		PERI_HDMIA,
		PERI_DSI,
		PERI_RESERVED0,
		PERI_HDMI, 
		PERI_USB11H, 
		PERI_SDMMC0, 
		PERI_MSTICK,  
		PERI_I2C0,
		PERI_UART0,  
		PERI_UART1,  
		PERI_UART2,  
		PERI_UART3,  
		PERI_UART4,  
		PERI_UART5,  
		PERI_GPSB0, 
		PERI_GPSB1, 
		PERI_GPSB2, 
		PERI_GPSB3, 
		PERI_GPSB4, 
		PERI_GPSB5, 
		PERI_ADC,  
		PERI_SPDIF,  
		PERI_EHI0, 
		PERI_EHI1, 
		PERI_AUD,  		//Not used
		PERI_PDM,
		PERI_SDMMC1, 
		PERI_SDMMC2,
		PERI_SDMMC3,
		PERI_DAI0,		
		PERI_DAI1,		//7.1 ch audio
		PERI_DAI2,
		PERI_I2C1,
		PERI_SATA_REF0,
		PERI_SATA_REF1,
		PERI_SATAH0,    // Sata Host0
		PERI_SATAH1,    // Sata Host1 and Device
		PERI_USB20H,
		PERI_GMAC,
		PERI_CIFMC,
		PERI_CIFSC,
		PERI_ISPJ,
		PERI_ISPS,
		PERI_FILTER,
		PERI_RESERVED1,
		PERI_RESERVED2,
		PERI_RESERVED3,
		PERI_OUT0, 
		PERI_OUT1, 
	};
	
	/*
	 * PWROFF Register (0xF0404018)
	 */
	enum{                           // bit Name
		PMU_VIDEODAC = 0,           //  0: DAC
		PMU_HDMIPHY,                //  1: HD
		PMU_USB1NANOPHY,            //  2: UP1
		PMU_USBNANOPHY,             //  3: UP0
		PMU_SATAPHY,                //  4: SP0
		PMU_DDIBUSISOLATION,        //  5: DB[0]
		PMU_DDIBUSPRE,              //  6: DB[1]
		PMU_DDIBUS,                 //  7: DB[2]
		PMU_RESERVED08,             //  8: reserved
		PMU_RESERVED09,             //  9: reserved
		PMU_RESERVED10,             // 10: reserved
		PMU_VIDEOBUSISOLATION,      // 11: VB[0]
		PMU_VIDEOBUSPRE,            // 12: VB[1]
		PMU_VIDEOBUS,               // 13: VB[2]
		PMU_GRAPHICBUSISOLATION,    // 14: GB[0]
		PMU_GRAPHICBUSPRE,          // 15: GB[1]
		PMU_GRAPHICBUS,             // 16: GB[2]
		PMU_CAMBUSISOLATION,        // 17: CB[0]
		PMU_CAMBUSPRE,              // 18: CB[1]
		PMU_CAMBUS,                 // 19: CB[2]
		PMU_HSBUSISOLATION,         // 20: HSB[0]
		PMU_HSBUSPRE,               // 21: HSB[1]
		PMU_HSBUS,                  // 22: HSB[2]
		PMU_MEMORYBUSISOLATION,     // 23: MB[0]
		PMU_MEMORYBUSPRE,           // 24: MB[1]
		PMU_MEMORYBUS,              // 25: MB[2]
		PMU_MIPIPHY,                // 26: MP
		PMU_RESERVED27,             // 27: reserved
		PMU_RESERVED28,             // 28: reserved
		PMU_SATA1PHY,               // 29: SP1
		PMU_LVDSPHY,                // 30: LV
	};
	
	enum{/* for SWRESET */
		RESET_RESERVED0 = 0,
		RESET_DDIBUS,
		RESET_RESERVED1,
		RESET_GRAPBUS,
		RESET_RESERVED2,
		RESET_VIDEOBUS,
		RESET_VIDEOCORE,
		RESET_RESERVED3,
		RESET_HSBUS,
		RESET_CAMBUS,
	};
	
	enum {/* clock divider (div+1) */
		CLKDIV0 = 0,
		CLKDIV2 ,
		CLKDIV3 ,
		CLKDIV4 ,
		CLKDIVNONCHANGE,
	};
	
	enum {
		CLKCTRL0 = 0,	//FCORE_CPU
		CLKCTRL1,		//FBUS_DDI
		CLKCTRL2,		//FMEM_BUS
		CLKCTRL3,		//FBUS_GRP
		CLKCTRL4,		//FBUS_IOBUS 
		CLKCTRL5,		//FBUS_VBUS
		CLKCTRL6,		//FBUS_VCODEC
		CLKCTRL7,		//FBUS_SMU
		CLKCTRL8 = 18,		//FBUS_HSBUS
		CLKCTRL9,		//FBUS_CAMBUS
	};

	enum {
		NORMAL_MD = 0,
		DYNAMIC_MD,
	};

	enum {
		RB_USB11HOST,
		RB_USB20OTG, 
		RB_IDECONTROLLER, 
		RB_DMACONTROLLER , 
		RB_SDMMCCHANNELCONTROLLER,	
		RB_SDMMC0CONTROLLER,	// GPIOG[21 ~ 30]
		RB_MSTICKCONTROLLER,
		RB_I2CCONTROLLER0,
		RB_NFCCONTROLLER ,
		RB_EXTHCONTROLLER0 , 
		RB_EXTHCONTROLLER1 ,
		RB_UARTCONTROLLER0 ,
		RB_UARTCONTROLLER1 ,
		RB_UARTCONTROLLER2 ,
		RB_UARTCONTROLLER3 ,
		RB_UARTCONTROLLER4 ,
		RB_UARTCONTROLLER5 ,
		RB_GPSBCONTROLLER0 ,
		RB_GPSBCONTROLLER1 ,
		RB_GPSBCONTROLLER2 ,
		RB_GPSBCONTROLLER3 ,
		RB_GPSBCONTROLLER4 ,
		RB_GPSBCONTROLLER5 ,
		RB_DAI0CONTROLLER,
		RB_DAI1CONTROLLER,
		RB_SPDIFTXCONTROLLER,
		RB_RTCCONTROLLER ,
		RB_TSADCCONTROLLER,
		RB_OVERAYMIXERCONTROLLER,
		RB_RESERVED0,
		RB_PROTECTIONCONTROLLER,
		RB_ADMACONTROLLER,	// 31
		RB_MPE_FECCONTROLLER, 
		RB_TSIF0CONTROLLER,
		RB_SRAMCONTROLLER,
		RB_SDMMC1CONTROLLER,	// GPIOF[18 ~ 26]
		RB_SDMMC2CONTROLLER,	// GPIOG[0 ~ 9]
		RB_SDMMC3CONTROLLER,	// GPIOF[0 ~ 9]
		RB_I2CCONTROLLER1,
		RB_DAI2CONTROLLER,
		RB_PDMCONTROLLER,
		RB_TSIF1CONTROLLER,
		RB_REMOTECONTROLLER,
		RB_ALLPERIPERALS
	};
	
	enum{ /* Fmbus Step */
		FMBUS_141Mhz=0,
		FMBUS_145Mhz,
		FMBUS_150Mhz,
		FMBUS_160Mhz,
		FMBUS_170Mhz,
		FMBUS_180Mhz,
		FMBUS_190Mhz,
		FMBUS_200Mhz,
		FMBUS_210Mhz,
		FMBUS_220Mhz,
		FMBUS_230Mhz,
		FMBUS_240Mhz,
		FMBUS_250Mhz,
		FMBUS_260Mhz,
		FMBUS_270Mhz,
		FMBUS_280Mhz,
		FMBUS_290Mhz,
		FMBUS_300Mhz,
		FMBUS_312Mhz,
		FMBUS_320Mhz,
		FMBUS_330Mhz,
	
		FMBUS_STEPMAX,
	};
	
	enum{ /* ddi Power Down Field  */
		DDIPWDN_RESERVED = 0,
		DDIPWDN_VIQE,
		DDIPWDN_LCDC0,
		DDIPWDN_LCDC1,
		DDIPWDN_LCDSI0,
		DDIPWDN_MSCL0,
		DDIPWDN_MSCL1,
		DDIPWDN_DDIC,
		DDIPWDN_HDMI,
		DDIPWDN_LCDSI1,
		
		DDIPWDN_STEPMAX,
	};

	enum{ /* ddi Power Down Field  */
		GRPPWDN_MALI = 0,
		
		GRPPWDN_STEPMAX,
	};

	enum{ /* ETC Power Down Field  */
		ETC_USBPHYOFF = 0,
		ETC_USBPHYON,
		ETC_3DGPUOFF, 
		ETC_3DGPUON, 
		ETC_OVERLAYMIXEROFF, 
		ETC_OVERLAYMIXERON ,
		
		ETC_STEPMAX,

	};

#define	ETCMASK_USBPHYOFF  		0x00000001
#define   ETCMASK_USBPHYON    		0x00000002
#define	ETCMASK_3DGPUOFF  		0x00000004
#define   ETCMASK_3DGPUON    		0x00000008
#define	ETCMASK_OVERLAYMIXEROFF  0x00000010
#define   ETCMASK_OVERLAYMIXERON    0x00000020
	
	enum{ /* Video BUS CFG Power Down Field  */
		VIDEOBUSCFG_PWDNJPEGENC = 0,
		VIDEOBUSCFG_PWDNRESERVED,
		VIDEOBUSCFG_PWDNVIDEOCODEC, 
		VIDEOBUSCFG_PWDNVIDEOCACHE, 
		
		VIDEOBUSCFG_PWDNSTEPMAX,

	};
	enum{ /* Video BUS CFG Power Down Field  */
		VIDEOBUSCFG_SWRESETJPEGENC = 0,
		VIDEOBUSCFG_SWRESETRESERVED,
		VIDEOBUSCFG_SWRESETVIDEOCODEC, 
		VIDEOBUSCFG_SWRESETVIDEOCACHE, 
		
		VIDEOBUSCFG_SWRESETSTEPMAX,

	};

	enum{ /* External Interrupt Source Table */
		EXT_INTR_SRC_ADC_TS_WKU	= 56,
		EXT_INTR_SRC_ADC_STOP	= 57,
		EXT_INTR_SRC_ADC_UPDOWN	= 58,
		EXT_INTR_SRC_RESERVED0	= 59,
		EXT_INTR_SRC_RESERVED1	= 60,
		EXT_INTR_SRC_RTC_WAKEUP	= 61,
		EXT_INTR_SRC_USB0_VBON	= 62,
		EXT_INTR_SRC_USB0_VBOFF	= 63,
	};

/***************************************Interrup****************************************************/
enum {
	IRQ_TC0	=0,	// 0 	0x0  Timer 0 interrupt enable 
	IRQ_TC32,		// 1 	0x0  Timer 1 interrupt enable 
	IRQ_SMUI2C,  // 2   0x0	SMU_I2C interrupt enable 
	IRQ_EI0,		// 3 	0x0  External interrupt 0 enable 
	IRQ_EI1,		// 4 	0x0  External interrupt 1 enable 
	IRQ_EI2,		// 5 	0x0  External interrupt 2 enable 
	IRQ_TSD,		// 6 	0x0  External interrupt 3 enable 
	IRQ_ISP0,		// 7 	0x0  External interrupt 4 enable 
	IRQ_ISP1,		// 8 	0x0  External interrupt 5 enable 
	IRQ_ISP2,		// 9 	0x0  External interrupt 6 enable 
	IRQ_ISP3,  	// 10  0x0  External interrupt 7 enable 
	IRQ_SATA0,  	// 11  0x0  External interrupt 8 enable 
	IRQ_SATA1,  	// 12  0x0  External interrupt 9 enable 
	IRQ_UB20H,  	// 13  0x0  External interrupt 10 enable 
	IRQ_GMAC,  	// 14  0x0  External interrupt 11 enable 
	IRQ_SC0,  	// 15  0x0  Mem-to-Mem scaler 0 interrupt enable 
	IRQ_SC1,  	// 16  0x0  Mem-to-Mem scaler 0 interrupt enable 
	IRQ_HDMI,  	// 17  0x0  Camera interrupt enable 
	IRQ_LCD0,  	// 18  0x0  LCD controller 0 interrupt enable 
	IRQ_LCD1,  	// 19  0x0  LCD controller 1 interrupt enable 
	IRQ_VIPET, 	// 20  0x0 VIPET controller interrupt enable Note: the interrupt request signal is active low. 21	JPGE  RW  0x0  JPEG Encoder interrupt enable 
	IRQ_MIPID,  	// 21  0x0  JPEG Decoder interrupt enable 
	IRQ_JPGE,  	// 22  0x0  JPEG Decoder interrupt enable 
	IRQ_VCDC,  	// 23  0x0  Video CODEC interrupt enable 
	IRQ_3DPP,  	// 24  0x0  3D Pixel Processor interrupt enable 
	IRQ_3DGP,  	// 25  0x0  3D Geometry Processor interrupt enable 
	IRQ_3DMMU,  	// 26  0x0	3D MMU interrupt enable 
	IRQ_CIF,  	// 27  0x0  Graphic Engine 2D Hardware Interrupt Enable 
	IRQ_TSADC,  	// 28  0x0	TSADC interrupt enable 
	IRQ_GDMA,  	// 29  0x0  DMA controller interrupt enable 
	IRQ_I2C1,  	// 30  0x0  ECC interrupt enable 
	IRQ_EHI0,  	// 31  0x0  External interrupt 0 enable 
	IRQ_EHI1,  	// 32  0x0  External interrupt 1 enable 
	IRQ_SD3,  	// 33  0x0  CAN interrupt enable 
	IRQ_SD2, 	// 34  0x0  HDMI interrupt enable 
	IRQ_OM,  	// 35  0x0  SATA Host interrupt enable 
	IRQ_GPSB,  	// 36  0x0  GPSB Interrupt Enable 
	IRQ_IDE,  	// 37  0x0  HDD controller interrupt enable 
	IRQ_I2C0,  	// 38  0x0  I2C interrupt enable 
	IRQ_MPEFEC, 	// 39  0x0	MPEFEC interrupt enable 
	IRQ_MS,		// 40	0x0  Memory Stick interrupt enable 
	IRQ_NFC, 	// 41  0x0  Nand flash controller interrupt enable 
	IRQ_RMT,  	// 42  0x0  Remote Control interrupt enable 
	IRQ_RTC,  	// 43  0x0  RTC interrupt enable 
	IRQ_SD1,  	// 44  0x0  SD/MMC 0 interrupt enable 
	IRQ_SD0,  	// 45  0x0  SD/MMC 1 interrupt enable 
	IRQ_SPD,  	// 46  0x0	SPDIF transmitter interrupt enable 
	IRQ_UART,  	// 47  0x0  UART interrupt enable 
	IRQ_UOTG,  	// 48  0x0  USB 2.0 OTG0 interrupt enable 
	IRQ_UH11, 	// 49  0x0  USB 2.0 OTG1 interrupt enable 
	IRQ_CIPHER,  	// 50  0x0  GPS RTC expired interrupt enable 
	IRQ_DAI2RX,  	// 51  0x0  GPS TCXO expired interrupt enable 
	IRQ_DAI2TX,  	// 52  0x0  GPS AGPS interrupt enable 
	IRQ_TSIF1,  	// 53  0x0	TS interface 0 interrupt enable 
	IRQ_TSIF0,  	// 54  0x0	TS interface 1 interrupt enable 
	IRQ_CDIF,  	// 55  0x0  CDIF receive interrupt enable 
	IRQ_DAI1RX,  	// 56  0x0	DAI receive interrupt enable 
	IRQ_DAI1TX,  	// 57  0x0	DAI transmit interrupt enable 
	IRQ_ADMA,  	// 58  0x0  AUDIO DMA interrupt enable 
	IRQ_AUDIO,  	// 59  0x0	AUDIO interrupt enable 
	IRQ_CPMU, 	// 60 0x0 ARM System Metrics interrupt enable Note: the interrupt request signal is active low. 
	IRQ_CDMAS, 	// 61 0x0 Non secure ARM DMA interrupt enable Note: the interrupt request signal is active low. 
	IRQ_CDMA, 	// 62 0x0 Secure ARM DMA select interrupt enable Note: the interrupt request signal is active low. 
	IRQ_CERR, 	// 63 0x0 Not maskable error ARM DMA interrupt enable Note: the interrupt request signal is active low. 
};

enum {
	HSIO_PCIE,
	HSIO_SATAH0,
	HSIO_SATAH1,
	HSIO_SATAD,
	HSIO_USB20H,
	HSIO_GMAC,
	HSIO_TSDEMUX,
	HSIO_HSDMA,
	HSIO_CIPHER,
	HSIO_DMAX,
};

#define HSIOBUS_USB20OTG1	 	0x00000001
#define HSIOBUS_SATA			0x00000002
#define HSIOBUS_GMAC			0x00000004
#define HSIOBUS_MSTICKHOST		0x00000008
#define HSIOBUS_CIPHER			0x00000010

enum {
	CAMBUS_CIF,
	CAMBUS_ISP
};

#ifdef __cplusplus
}
#endif

#endif // __GLOBALS_H__
