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
#define IOCTL_PROCESSOR_NAME                (L"TCC89X")
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
		PERI_LCDSI, 
		PERI_CIFMC,  
		PERI_CIFSC, 
		PERI_OUT0, 
		PERI_OUT1, 
		PERI_HDMI, 
		PERI_USB11H, 
		PERI_SDMMC0, 
		PERI_MSTICK,  
		PERI_I2C, 
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
		PERI_CAN,  
		PERI_Reserved0, 
		PERI_SDMMC1, 
		PERI_Reserved1, 
		PERI_DAI,  
	};
	
	enum{/*for PWROFF Register*/
		PMU_VIDEODAC = 0,
		PMU_HDMIPHY,
		PMU_LVDSPHY,
		PMU_USBNANOPHY,
		PMU_SATAPHY,
		PMU_MEMORYBUS,
		PMU_VIDEOBUS,
		PMU_DDIBUS,
		PMU_GRAPHICBUS,
		PMU_IOBUS,
	};
	
	enum{/* for SWRESET */
		RESET_CPU = 0,
		RESET_DDIBUS,
		RESET_MEMBUS,
		RESET_GRAPBUS,
		RESET_IOBUS,
		RESET_VIDEOBUS,
		RESET_VIDEOCORE,
		RESET_SMU,
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
		CLKCTRL4,		//FBUS_IOB
		CLKCTRL5,		//FBUS_VBUS
		CLKCTRL6,		//FBUS_VCODEC
		CLKCTRL7,		//FBUS_SMU
	};
	
	enum {
		NORMAL_MD = 0,
		DYNAMIC_MD,
	};
	
	enum {
		RB_USB11H	= 0,
		RB_USB20OTG, 
		RB_IDECONTROLLER, 
		RB_DMACONTROLLER , 
		RB_SDMMCCONTROLLER ,
		RB_SATAHCONTROLLER , 
		RB_MEMORYSTICKCONTROLLER , 
		RB_I2CCONTROLLER ,
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
		RB_DAICDIFCONTROLLER ,
		RB_ECCCONTROLLER ,
		RB_SPDIFTXCONTROLLER,
		RB_RTCCONTROLLER ,
		RB_TSADCCONTROLLER,
		RB_GPSCONTROLLER ,
		RB_RESERVEDCONTROLLER, 
		RB_CANCONTROLLER,
		RB_ADMACONTROLLER,	// 31
	
		RB_MPE_FECCONTROLLER, 
		RB_TSIFCONTROLLER,
		RB_SRAMCONTROLLER,
		
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
		DDIPWDN_CIF = 0,
		DDIPWDN_VIQE,
		DDIPWDN_LCDC0,
		DDIPWDN_LCDC1,
		DDIPWDN_LCDSI,
		DDIPWDN_MSCL0,
		DDIPWDN_MSCL1,
		DDIPWDN_DDIC,
		DDIPWDN_HDMI,
		DDIPWDN_STEPMAX,
		};

	enum{ /* vbus Power Down Field  */
		VBUSPWDN_JPEGENCODE = 0,
		VBUSPWDN_JPEGDECODE,
		VBUSPWDN_VIDEOCODEC,
		VBUSPWDN_VIDEOCACHE,
		VBUSPWDN_STEPMAX,
		};

	enum{ /* grp Power Down Field	*/
		GRPPWDN_MALI = 0,
		GRPPWDN_OVERLAY,
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

	enum{ /* External Interrupt Source Table */
		EXT_INTR_SRC_TSWKU 		= 56,
		EXT_INTR_SRC_TSSTOP   		= 57,
		EXT_INTR_SRC_TSUPDN 		= 58,
		EXT_INTR_SRC_RSVD0  		= 59,
		EXT_INTR_SRC_RSVD1  		= 60,
		EXT_INTR_SRC_PMKUP 		= 61,
		EXT_INTR_SRC_USB_VBON  	= 62,
		EXT_INTR_SRC_USB_VBOFF 	= 63,
	};

#define	ETCMASK_USBPHYOFF  		0x00000001
#define   ETCMASK_USBPHYON    		0x00000002
#define	ETCMASK_3DGPUOFF  		0x00000004
#define   ETCMASK_3DGPUON    		0x00000008
#define	ETCMASK_OVERLAYMIXEROFF  0x00000010
#define   ETCMASK_OVERLAYMIXERON    0x00000020
	
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
	IRQ_SC1,  	// 16  0x0  Mem-to-Mem scaler 1 interrupt enable 
	IRQ_CAM,  	// 17  0x0  Camera interrupt enable 
	IRQ_LCD0,  	// 18  0x0  LCD controller 0 interrupt enable 
	IRQ_LCD1,  	// 19  0x0  LCD controller 1 interrupt enable 
	IRQ_VIPET, 	// 20  0x0 VIPET controller interrupt enable Note: the interrupt request signal is active low. 21	JPGE  RW  0x0  JPEG Encoder interrupt enable 
	IRQ_JPGD,  	// 21  0x0  JPEG Decoder interrupt enable 
	IRQ_JPGE,  	// 22  0x0  JPEG Decoder interrupt enable 
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
	IRQ_UOTG,  	// 48  0x0  USB 2.0 OTG interrupt enable 
	IRQ_U11H,  	// 49  0x0  USB 1.1 host interrupt enable 
	IRQ_GPS0,  	// 50  0x0  GPS RTC expired interrupt enable 
	IRQ_GPS1,  	// 51  0x0  GPS TCXO expired interrupt enable 
	IRQ_GPS2,  	// 52  0x0  GPS AGPS interrupt enable 
	IRQ_TSIF0,  	// 53  0x0	TS interface 0 interrupt enable 
	IRQ_TSIF1,  	// 54  0x0	TS interface 1 interrupt enable 
	IRQ_CDRX,  	// 55  0x0  CDIF receive interrupt enable 
	IRQ_DAIRX,  	// 56  0x0	DAI receive interrupt enable 
	IRQ_DAITX,  	// 57  0x0	DAI transmit interrupt enable 
	IRQ_ADMA,  	// 58  0x0  AUDIO DMA interrupt enable 
	IRQ_AUDIO,  	// 59  0x0	AUDIO interrupt enable 
	IRQ_APMU, 	// 60 0x0 ARM System Metrics interrupt enable Note: the interrupt request signal is active low. 
	IRQ_AIRQ, 	// 61 0x0 Non secure ARM DMA interrupt enable Note: the interrupt request signal is active low. 
	IRQ_ASIRQ, 	// 62 0x0 Secure ARM DMA select interrupt enable Note: the interrupt request signal is active low. 
	IRQ_AEIRQ, 	// 63 0x0 Not maskable error ARM DMA interrupt enable Note: the interrupt request signal is active low. 
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

#ifdef __cplusplus
}
#endif

#endif // __GLOBALS_H__
