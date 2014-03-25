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
#define IOCTL_PROCESSOR_NAME                (L"TCC89x/91x/92x")
#define IOCTL_PROCESSOR_CORE                (L"ARM11")

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
		PCUSB1PHY,
		PCMIPI_PLL,
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
		PERI_CIFMC,  
		PERI_CIFSC, 
		PERI_OUT0, 
		PERI_OUT1, 
		PERI_HDMI, 
		PERI_SDMMC2,
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
		PERI_AUD,  
		PERI_PDM,  
		PERI_LCDSI1,
		PERI_SDMMC1, 
		PERI_I2C1,
		PERI_AUD1,
		PERI_I2C2,
		PERI_ISPS,
		PERI_ISPJ,
		PERI_NFC,
		PERI_GMAC,
		PERI_SATA,
		PERI_SDMMC3,
		PERI_SPDIF1,
		PERI_SATA_REF,
	};
	
	enum{/*for PWROFF Register*/
		PMU_VIDEODAC = 0,
		PMU_HDMIPHY,
		PMU_USB1NANOPHY,
		PMU_USBNANOPHY,
		PMU_SATAPHY,
		PMU_DDIBUSISOLATION,
		PMU_DDIBUSPRE,
		PMU_DDIBUS,
		PMU_DDISUBBUSISOLATION,
		PMU_DDISUBBUSPRE,
		PMU_DDISUBBUS,
		PMU_VIDEOBUSISOLATION,
		PMU_VIDEOBUSPRE,		
		PMU_VIDEOBUS,
		PMU_GRAPHICBUSISOLATION,
		PMU_GRAPHICBUSPRE,
		PMU_GRAPHICBUS,
		PMU_CAMBUSISOLATION,
		PMU_CAMBUSPRE,
		PMU_CAMBUS,
		PMU_HSBUSISOLATION,
		PMU_HSBUSPRE,
		PMU_HSBUS,
		PMU_MEMORYBUSISOLATION,
		PMU_MEMORYBUSPRE,
		PMU_MEMORYBUS,
		PMU_MIPIPHY,
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
		RESET_DDISUBBUS,
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
		CLKCTRL4,		//FBUS_IO
		CLKCTRL5,		//FBUS_VBUS
		CLKCTRL6,		//FBUS_VCODEC
		CLKCTRL7,		//FBUS_SMU
		CLKCTRL8,		//FBUS_HSBUS
		CLKCTRL9,		//FBUS_CAMBUS
		CLKCTRL10,		//FBUS_DDISUBBUS
	};
	
	enum {
		NORMAL_MD = 0,
		DYNAMIC_MD,
	};

	enum {
		RB_PREFETCHWRITEBUFFER = 0,
		RB_EHI0, 
		RB_EHI1, 
		RB_USB20OTG,
		RB_NFCCONTROLLER ,
		RB_SDMMCPORTMUXCONTROLLER,
		RB_SDMMC0CONTROLLER ,
		RB_SDMMC1CONTROLLER ,
		RB_SDMMC2CONTROLLER ,
		RB_GDMA0CONTROLLER , 
		RB_GDMA1CONTROLLER , 
		RB_GDMA2CONTROLLER , 
		RB_PWMCONTROLLER, 
		RB_2DCONTROLLER,
		RB_SDMMC3CONTROLLER,
		RB_REMOTECONTROLLER,
		RB_TSIF0CONTROLLER,
		RB_TSIF1CONTROLLER,
		RB_ADMA0CONTROLLER,
		RB_DAI0CONTROLLER,
		RB_ADMA1CONTROLLER,
		RB_DAI1CONTROLLER,
		RB_UARTCONTROLLER0 ,
		RB_UARTCONTROLLER1 ,
		RB_UARTCONTROLLER2 ,
		RB_UARTCONTROLLER3 ,
		RB_UARTCONTROLLER4 ,
		RB_UARTCONTROLLER5 ,
		RB_I2CCONTROLLER0,
		RB_I2CCONTROLLER1,
		RB_I2CCONTROLLER2,
		RB_GPSBCONTROLLER0 , //31
		RB_GPSBCONTROLLER1 ,
		RB_GPSBCONTROLLER2 ,
		RB_GPSBCONTROLLER3 ,
		RB_GPSBCONTROLLER4 ,
		RB_GPSBCONTROLLER5 ,
		RB_TSADCCONTROLLER,
		RB_GPIOCONTROLLER,
		RB_SPDIF0CONTROLLER,
		RB_PROTECTCONTROLLER,
		RB_SPDIF1CONTROLLER,
		
		RB_ALLPERIPERALS,
	
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

	enum{ /* gpu power down filed */
		GRPPWDN_MALI = 0,
		GRPPWDN_SETPMAX,
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
		EXT_INTR_SRC_TSWKU = 75,
		EXT_INTR_SRC_TSSTOP = 76,
		EXT_INTR_SRC_TSUPDN = 77,
		EXT_INTR_SRC_RMWKUP = 78,
		EXT_INTR_SRC_PMKUP = 79,
		EXT_INTR_SRC_USB1_VBON = 80,
		EXT_INTR_SRC_USB1_VBOFF = 81,
		EXT_INTR_SRC_USB0_VBON = 82,
		EXT_INTR_SRC_USB0_VBOFF = 83,
	};

/***************************************Interrup****************************************************/
enum {
	IRQ_TC0	=0,	// 0 	0x0  Timer 0 interrupt enable 
	IRQ_TC1,		// 1 	0x0  Timer 1 interrupt enable 
	IRQ_SMUI2C,  // 2   0x0	SMU_I2C interrupt enable 
	IRQ_EI0,		// 3 	0x0  External interrupt 0 enable 
	IRQ_EI1,		// 4 	0x0  External interrupt 1 enable 
	IRQ_EI2,		// 5 	0x0  External interrupt 2 enable 
	IRQ_EI3,		// 6 	0x0  External interrupt 3 enable 
	IRQ_EI4,		// 7 	0x0  External interrupt 4 enable 
	IRQ_EI5,		// 8 	0x0  External interrupt 5 enable 
	IRQ_EI6,		// 9 	0x0  External interrupt 6 enable 
	IRQ_EI7,  	// 10  0x0  External interrupt 7 enable 
	IRQ_EI8,  	// 11  0x0  External interrupt 8 enable 
	IRQ_EI9,  	// 12  0x0  External interrupt 9 enable 
	IRQ_EI10,  	// 13  0x0  External interrupt 10 enable 
	IRQ_EI11,  	// 14  0x0  External interrupt 11 enable 
	IRQ_SC0,  	// 15  0x0  Mem-to-Mem scaler 0 interrupt enable 
	IRQ_SC1,  	// 16  0x0  Mem-to-Mem scaler 0 interrupt enable 
	IRQ_CAM,  	// 17  0x0  Camera interrupt enable 
	IRQ_LCD0,  	// 18  0x0  LCD controller 0 interrupt enable 
	IRQ_LCD1,  	// 19  0x0  LCD controller 1 interrupt enable 
	IRQ_VIPET, 	// 20  0x0 VIPET controller interrupt enable Note: the interrupt request signal is active low. 21	JPGE  RW  0x0  JPEG Encoder interrupt enable 
	IRQ_JPGE,  	// 21  0x0  JPEG Decoder interrupt enable 
	IRQ_ISP2,//IRQ_JPGD,  	// 22  0x0  JPEG Decoder interrupt enable 
	IRQ_VCDC,  	// 23  0x0  Video CODEC interrupt enable 
	IRQ_3DPP,  	// 24  0x0  3D Pixel Processor interrupt enable 
	IRQ_3DGP,  	// 25  0x0  3D Geometry Processor interrupt enable 
	IRQ_3DMMU,  	// 26  0x0	3D MMU interrupt enable 
	IRQ_G2D,  	// 27  0x0  Graphic Engine 2D Hardware Interrupt Enable 
	IRQ_TSADC,  	// 28  0x0	TSADC interrupt enable 
	IRQ_DMA,  	// 29  0x0  DMA controller interrupt enable 
	IRQ_ECC,  	// 30  0x0  ECC interrupt enable 
	IRQ_EHI0,  	// 31  0x0  External interrupt 0 enable 
	IRQ_EHI1,  	// 32  0x0  External interrupt 1 enable 
	IRQ_CAN,  	// 33  0x0  CAN interrupt enable 
	IRQ_HDMI, 	// 34  0x0  HDMI interrupt enable 
	IRQ_SATA,  	// 35  0x0  SATA Host interrupt enable 
	IRQ_GPSB,  	// 36  0x0  GPSB Interrupt Enable 
	IRQ_HDD,  	// 37  0x0  HDD controller interrupt enable 
	IRQ_I2C,  	// 38  0x0  I2C interrupt enable 
	IRQ_MPEFEC, 	// 39  0x0	MPEFEC interrupt enable 
	IRQ_MS,		// 40	0x0  Memory Stick interrupt enable 
	IRQ_NFC, 	// 41  0x0  Nand flash controller interrupt enable 
	IRQ_RMT,  	// 42  0x0  Remote Control interrupt enable 
	IRQ_RTC,  	// 43  0x0  RTC interrupt enable 
	IRQ_SD0,  	// 44  0x0  SD/MMC 0 interrupt enable 
	IRQ_SD1,  	// 45  0x0  SD/MMC 1 interrupt enable 
	IRQ_SPDTX,  	// 46  0x0	SPDIF transmitter interrupt enable 
	IRQ_UART,  	// 47  0x0  UART interrupt enable 
	IRQ_UOTG,  	// 48  0x0  USB 2.0 OTG0 interrupt enable 
	IRQ_UOTG1, 	// 49  0x0  USB 2.0 OTG1 interrupt enable 
	IRQ_GPS0,  	// 50  0x0  GPS RTC expired interrupt enable 
	IRQ_GPS1,  	// 51  0x0  GPS TCXO expired interrupt enable 
	IRQ_GPS2,  	// 52  0x0  GPS AGPS interrupt enable 
	IRQ_TSIF0,  	// 53  0x0	TS interface 0 interrupt enable 
	IRQ_TSIF1,  	// 54  0x0	TS interface 1 interrupt enable 
	IRQ_CDRX,  	// 55  0x0  CDIF receive interrupt enable 
	IRQ_ISP3,//IRQ_DAIRX,  	// 56  0x0	DAI receive interrupt enable 
	IRQ_DAITX,  	// 57  0x0	DAI transmit interrupt enable 
	IRQ_ADMA,  	// 58  0x0  AUDIO DMA interrupt enable 
	IRQ_AUDIO,  	// 59  0x0	AUDIO interrupt enable 
	IRQ_APMU, 	// 60 0x0 ARM System Metrics interrupt enable Note: the interrupt request signal is active low. 
	IRQ_AIRQ, 	// 61 0x0 Non secure ARM DMA interrupt enable Note: the interrupt request signal is active low. 
	IRQ_ISP0,//IRQ_ASIRQ, 	// 62 0x0 Secure ARM DMA select interrupt enable Note: the interrupt request signal is active low. 
	IRQ_ISP1,//IRQ_AEIRQ, 	// 63 0x0 Not maskable error ARM DMA interrupt enable Note: the interrupt request signal is active low. 
};

typedef struct _rtctime { 
  unsigned int wYear; 
  unsigned int wMonth; 
  unsigned int wDayOfWeek; 
  unsigned int wDay; 
  unsigned int wHour; 
  unsigned int wMinute; 
  unsigned int wSecond; 
  unsigned int wMilliseconds; 
} rtctime;

enum {
	HSIO_PWB,
	HSIO_SATAD,
	HSIO_GMAC,
	HSIO_USBOTG,
	HSIO_GDMA,
	HSIO_MSTICK,
	HSIO_CIPHER,
	HSIO_SATAH,
};

enum {
	CAMBUS_CIF,
	CAMBUS_ISP
};

enum {
	DDISUBBUS_HCLK_MIPIDSI,
	DDISUBBUS_HCLK_EHI
};

#define HSIOBUS_USB20OTG1		0x00000001
#define HSIOBUS_SATA			0x00000002
#define HSIOBUS_GMAC			0x00000004
#define HSIOBUS_MSTICKHOST		0x00000008
#define HSIOBUS_CIPHER			0x00000010


#ifdef __cplusplus
}
#endif

#endif // __GLOBALS_H__
