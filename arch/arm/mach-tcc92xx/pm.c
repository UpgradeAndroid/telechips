/* arch/arm/mach-tcc92x/pm.c
 *
 * TCC92xx Power Management Routines
 *
 * Copyright (C) 2009, 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/reboot.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <linux/syscalls.h>		
#include <linux/delay.h>
#include <linux/regulator/machine.h>
#include <mach/pm.h>

#include <mach/system.h>
#include <mach/tcc_pca953x.h>

#define TCC_PMU_BASE	0xf0404000
#define PMU_USERSTS	(TCC_PMU_BASE + 0x14)

#define RECOVERY_MODE	0x77665502
#define FASTBOOT_MODE	0x77665500

//#define FEATURE_DDI_GRP_BACKUP

#define DRAM_PHSY_ADDR 0x4FF00000
#define DRAM_VIRT_ADDR 0x4FF00004
#define DRAM_WAKE_ADDR 0x4FF00008
#define DRAM_DATA_ADDR 0x4FF0000C

#if CONFIG_PM_VERBOSE
#define pmdrv_dbg(fmt, arg...)     printk(fmt, ##arg)
#else
#define pmdrv_dbg(arg...)
#endif

void clk_disable_unused(void);


//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//-------------------------------------------------------------------------------
void NopDelay( void )
{
	__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n");
	__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n");
	__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n");
	__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n");
	__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n");
	__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n");	
}


static void tcc_pm_power_off(void)
{
	// insert board specific code here
#ifdef CONFIG_RTC_DISABLE_ALARM_FOR_PWROFF_STATE		//Disable the RTC Alarm during the power off state
	PRTC pRTC = (PRTC)tcc_p2v(HwRTC_BASE);
	if (pRTC == NULL) {
		printk("failed RTC ioremap()\n");
	}
	else {
		BITCLR(pRTC->RTCCON, Hw7|Hw6);	// Disable - Wake Up Interrupt Output(Hw7), Alarm Interrupt Output(Hw6)

		BITSET(pRTC->RTCCON, Hw1);	// Enable - RTC Write
		BITSET(pRTC->INTCON, Hw0);	// Enable - Interrupt Block Write

		BITCLR(pRTC->RTCALM, Hw7|Hw6|Hw5|Hw4|Hw3|Hw2|Hw1|Hw0);	// Disable - Alarm Control

		BITCSET(pRTC->RTCIM, Hw3|Hw2|Hw1|Hw0, Hw3|Hw2);	// Power down mode, Active HIGH, Disable alarm interrupt

		BITCLR(pRTC->INTCON, Hw0);	// Disable - Interrupt Block Write
		BITCLR(pRTC->RTCCON, Hw1);	// Disable - RTC Write
	}
#endif

	if (machine_is_m57te() || machine_is_m801()) {
//		BITCLR(HwGPIOF->GPDAT, Hw12);	// WIFI_RST
//		BITCLR(HwGPIOF->GPDAT, Hw10);	// WIFI_ON

		BITCLR(HwGPIOA->GPDAT, Hw4);	// LCD_BLCTL
		BITCLR(HwGPIOD->GPDAT, Hw21);	// LCD_PWREN

		BITCLR(HwGPIOA->GPDAT, Hw2);	// SHDN
	}

	while (1)
	    ;
}

//PMU (POWER MANAGEMENT UNIT)
char		uPMU[sizeof(PMU)];
//CKC
char		uCKC[sizeof(CKC)];

//VPIC (VECTORED PRIORITY INTERRUPT CONTROLLER)
char		uPIC[sizeof(PIC)];
char		uVIC[sizeof(VIC)];

//TIMER / COUNTER
char		uTMR[sizeof(TIMER)];

//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//-------------------------------------------------------------------------------

static pTCC92X_REG pSave; 	// Sangwon, suspend bug

unsigned pll_value, clk_value;
unsigned int *pVaddr;
unsigned int *pPaddr;

static void tcc_nfc_suspend(PNFC pBackupNFC, PNFC pHwNFC)
{
	pBackupNFC->NFC_CTRL = pHwNFC->NFC_CTRL;
	pBackupNFC->NFC_CTRL1 = pHwNFC->NFC_CTRL1;
}

static void tcc_nfc_resume(PNFC pHwNFC, PNFC pBackupNFC)
{
	pHwNFC->NFC_CTRL = pBackupNFC->NFC_CTRL;
	pHwNFC->NFC_IREQ = 0x77;
	pHwNFC->NFC_CTRL1 = pBackupNFC->NFC_CTRL1;
}

static void suspend_mode_on(void)
{

//	pTCC92X_REG pSave; 

	// Lock ARM Interrupt
	pSave->lock = IO_ARM_DisableINT();

	
	// Enable Bus Clock for access system register
	//IO BUS - All Enable
	memcpy(pSave->uIOBUSCFG, (char*)&HwIOBUSCFG_BASE, sizeof(IOBUSCFG));
	HwIOBUSCFG->HCLKEN0 = 0xFFFFFFFF;
	HwIOBUSCFG->HCLKEN1 = 0xFFFFFFFF;
	HwIOBUSCFG->HRSTEN0 = 0xFFFFFFFF;
	HwIOBUSCFG->HRSTEN1 = 0xFFFFFFFF;

	//Don't move!! to reduce DAC-current leakage....
	HwTVE->DACPD = 1;

#ifdef FEATURE_DDI_GRP_BACKUP
	//DDI BUS - All Enable
	memcpy(pSave->uDDI_CONFIG, (char*)&HwDDI_CONFIG_BASE, sizeof(DDICONFIG));	
	HwLVDS->PWDN = 0x0;

	//GRAPHIC BUS - All Enable
	
	//GRPBUS Configuration
	memcpy(pSave->uGPUGRPBUSCONFIG, (char*)&HwGRPBUS_BASE, sizeof(GPUGRPBUSCONFIG));
	HwGRPBUS->GRPBUS_PWRDOWN = 0x0;  
#endif

	// Save Memory & system registers
	/*--------------------------------------------------------------
	 Internal Memory ( TCM & SRAM)
	--------------------------------------------------------------*/
	// ITCM
	//memcpy(pSave->uITCM, phys_to_virt(ITCM_BASE), ITCM_SIZE);
	// DTCM
	//memcpy(pSave->uDTCM, phys_to_virt(DTCM_BASE), DTCM_SIZE);
	// SRAM
	//memcpy(pSave->uSRAM, phys_to_virt(SRAM_BASE), SRAM_SIZE);
	//DEV_mDDR_SetFreq(uIO_CKC_Fclk[HwCKC_SYS_MBUS]/10000, &pll_value, &clk_value);

	
	/*--------------------------------------------------------------
	 PART2 - SMU & PMU
	--------------------------------------------------------------*/
	//CKC
	//memcpy(pSave->uCKC, &HwCLK_BASE, sizeof(CKC));
	//VPIC (VECTORED PRIORITY INTERRUPT CONTROLLER)
	//memcpy(pSave->uPIC, &HwPIC_BASE, sizeof(PIC));
	//memcpy(pSave->uVIC, &HwVIC_BASE, sizeof(VIC));
	//TIMER / COUNTER
	//memcpy(pSave->uTMR, &HwTMR_BASE, sizeof(TIMER));
	//PMU (POWER MANAGEMENT UNIT)
	//memcpy(pSave->uPMU, &HwPMU_BASE, sizeof(PMU));
	//CKC
	memcpy(pSave->uCKC, uCKC, sizeof(CKC));
	//VPIC (VECTORED PRIORITY INTERRUPT CONTROLLER)
	memcpy(pSave->uPIC, uPIC, sizeof(PIC));
	memcpy(pSave->uVIC, uVIC, sizeof(VIC));
	//TIMER / COUNTER
	memcpy(pSave->uTMR, uTMR, sizeof(uTMR));	
	memcpy(pSave->uPMU, uPMU, sizeof(uPMU));
	
	//SMU_I2C
	memcpy(pSave->uSMUI2C_MASTER0, (char*)&HwSMUI2C_MASTER0_BASE, sizeof(SMUI2CMASTER));
	memcpy(pSave->uSMUI2C_MASTER1, (char*)&HwSMUI2C_MASTER1_BASE, sizeof(SMUI2CMASTER));
	memcpy(pSave->uSMUI2C_COMMON, (char*)&HwSMUI2C_COMMON_BASE, sizeof(SMUI2CICLK));
	
	/*--------------------------------------------------------------
	 PART3 - GPIO
	--------------------------------------------------------------*/
	memcpy(pSave->uGPIO, (char*)&HwGPIO_BASE, sizeof(GPIO));
	
	/*--------------------------------------------------------------
	 PART4 - CORE & MEMORY BUS
	--------------------------------------------------------------*/
	//SDR/DDR SDRAM Controller Registers
	//memcpy(pSave->uDRAM, &HwDRAMM0_BASE, sizeof(DRAM));
	//DDR2 SDRAM Controller Registers
	//memcpy(pSave->uDRAMMX, &HwDRAMM1_BASE, sizeof(DRAM));
	//SDRAM PHY Control Registers
	//memcpy(pSave->uDRAMPHY, &HwDRAMPHY_BASE, sizeof(DRAMPHY));
	//Miscellaneous Configuration Registers
	//memcpy(pSave->uDRAMMISC, &HwDRAMMISC_BASE, sizeof(DRAMMISC));
	//Memory Bus Configuration Registers
	//memcpy(pSave->uDRAMMEMBUS, &HwDRAMMEMBUS_BASE, sizeof(DRAMMEMBUS));

	//Core Bus Configuration Registers
	memcpy(pSave->uCORECFG, (char*)&HwCORECFG_BASE, sizeof(MISCCOREBUS));
	//Virtual MMU Table Registers
	memcpy(pSave->uVMTREGION, (char*)&HwREGION_BASE, sizeof(VMTREGION));
	
	/*--------------------------------------------------------------
	 PART5 - IO BUS
	--------------------------------------------------------------*/
	//Memory Stick Host Controller
	memcpy(pSave->uSMSHC, (char*)&HwSMSHC_BASE, sizeof(SMSHC));
	memcpy(pSave->uSMSHCPORTCFG, (char*)&HwSMSHC_BASE, sizeof(SMSHCPORTCFG));
	//SD/SDIO/MMC/CE_ATA Host Controller	
	//memcpy(pSave->uSDCORE0SLOT0, (char*)&HwSDCORE0SLOT0_BASE, sizeof(SDHOST));
	//memcpy(pSave->uSDCORE0SLOT1, (char*)&HwSDCORE0SLOT1_BASE, sizeof(SDHOST));
	//memcpy(pSave->uSDCORE1SLOT0, (char*)&HwSDCORE1SLOT0_BASE, sizeof(SDHOST));
	//memcpy(pSave->uSDCORE1SLOT1, (char*)&HwSDCORE1SLOT1_BASE, sizeof(SDHOST));
	//memcpy(pSave->uSDCHCTRL, (char*)&HwSDCHCTRL_BASE, sizeof(SDCHCTRL));
	
	//NAND Flash Controller
	//memcpy(pSave->uNFC, (char*)&HwNFC_BASE, sizeof(NFC));
	tcc_nfc_suspend((PNFC)pSave->uNFC,(PNFC)tcc_p2v(HwNFC_BASE));
	//Static Memory Controller
	memcpy(pSave->uSMC, (char*)&HwSMC_BASE, sizeof(SMC));
	//External Device Interface
	memcpy(pSave->uEDI, (char*)&HwEDI_BASE, sizeof(EDI));
	//IDE Controller
	memcpy(pSave->uIDE, (char*)&HwIDE_BASE, sizeof(IDE));


	//Audio DMA
	memcpy(pSave->uADMA, (char*)&HwADMA_BASE, sizeof(ADMA));
	memcpy(pSave->uADMA_DAI, (char*)&HwADMA_DAIBASE, sizeof(ADMADAI));
	memcpy(pSave->uADMA_CDIF, (char*)&HwADMA_CDIFBASE, sizeof(ADMACDIF));
//	memcpy(pSave->uADMA_SPDIFTX, (char*)&HwADMA_SPDIFTXBASE, sizeof(ADMASPDIFTX));
//	memcpy(pSave->uADMA_SPDIFRX, (char*)&HwSPDIF_BASE, sizeof(ADMASPDIFRX));
#if 0
	//DAI Interface Register
	memcpy(pSave->uDAI, (char*)&HwDAI_BASE, sizeof(DAI));
	memcpy(pSave->uCDIF, (char*)&HwCDIF_BASE, sizeof(CDIF));
#endif

	//SPDIF TRANSMITTER
	memcpy(pSave->uSPDIF, (char*)&HwSPDIF_BASE, sizeof(SPDIF));
	
	//USB 2.0 OTG Controller
	memcpy(pSave->uUSB20OTG, (char*)&HwUSB20OTG_BASE, sizeof(USB20OTG));
	memcpy(pSave->uUSBOTGCFG, (char*)&HwUSBOTGCFG_BASE, sizeof(USBOTGCFG));
	memcpy(pSave->uUSBPHYCFG, (char*)&HwUSBPHYCFG_BASE, sizeof(USBPHYCFG));

	//External Host Interface
	memcpy(pSave->uEHI0, (char*)&HwEHICS0_BASE, sizeof(EHI));
	memcpy(pSave->uEHI1, (char*)&HwEHICS1_BASE, sizeof(EHI));

	//General Purpose Serial Bus
	memcpy(pSave->uGPSB0, (char*)&HwGPSBCH0_BASE, sizeof(GPSB));
	memcpy(pSave->uGPSB1, (char*)&HwGPSBCH1_BASE, sizeof(GPSB));
	memcpy(pSave->uGPSB2, (char*)&HwGPSBCH2_BASE, sizeof(GPSB));
	memcpy(pSave->uGPSB3, (char*)&HwGPSBCH3_BASE, sizeof(GPSB));
	memcpy(pSave->uGPSB4, (char*)&HwGPSBCH4_BASE, sizeof(GPSB));
	memcpy(pSave->uGPSB5, (char*)&HwGPSBCH5_BASE, sizeof(GPSB));
	memcpy(pSave->uGPSBPORTCFG, (char*)&HwGPSBPORTCFG_BASE, sizeof(GPSBPORTCFG));
	memcpy(pSave->uGPSBPIDTABLE, (char*)&HwGPSBPIDTABLE_BASE, sizeof(GPSBPIDTABLE));
	
	//Transport Stream Interface
	//memcpy(pSave->uTSIF, (char*)&HwTSIF_BASE, sizeof(TSIF));
	//memcpy(pSave->uTSIFPORTSEL, (char*)&HwTSIFPORTSEL_BASE, sizeof(TSIFPORTSEL));
	
	//GPS
	//memcpy(pSave->uGPS, (char*)&HwGPS_BASE, sizeof(GPS));
	
	//Remote Control Interface
	memcpy(pSave->uREMOCON, (char*)&HwREMOTE_BASE, sizeof(REMOTECON));
	
	//I2C
	memcpy(pSave->uI2CMASTER0, (char*)&HwI2CMASTER0_BASE, sizeof(I2CMASTER));
	memcpy(pSave->uI2CMASTER1, (char*)&HwI2CMASTER1_BASE, sizeof(I2CMASTER));
	memcpy(pSave->uI2CSLAVE, (char*)&HwI2CSLAVE_BASE, sizeof(I2CSLAVE));
	memcpy(pSave->uI2CSTATUS, (char*)&HwI2CSTATUS_BASE, sizeof(I2CSTATUS));

	pmdrv_dbg("Enter Suspend_mode !!\n");
	// UART0 (Debug UART)
	word_of(&pSave->uUART0[0]+0x04)	= HwUART0->IER;  //0x04
	word_of(&pSave->uUART0[0]+0x0C)	= HwUART0->LCR;  //0x0C
	word_of(&pSave->uUART0[0]+0x10)	= HwUART0->MCR;  //0x10
	word_of(&pSave->uUART0[0]+0x1C)	= HwUART0->SCR;  //0x1C
	BITSET(HwUART0->LCR, Hw7);
	word_of(&pSave->uUART0[0]+0x20)	= HwUART0->DLL;  //0x00
	word_of(&pSave->uUART0[0]+0x24)	= HwUART0->DLM;  //0x04
	word_of(&pSave->uUART0[0]+0x28)	= HwUART0->AFT;  //0x20
	word_of(&pSave->uUART0[0]+0x2C)	= HwUART0->UCR;  //0x24
	word_of(&pSave->uUART0[0]+0x30)	= HwUART0->SCCR; //0x60
	word_of(&pSave->uUART0[0]+0x34)	= HwUART0->STC;  //0x64
	word_of(&pSave->uUART0[0]+0x38)	= HwUART0->IRCFG;//0x80

	// UART1 (BT UART)
	word_of(&pSave->uUART1[0]+0x04)	= HwUART1->IER;  //0x04
	word_of(&pSave->uUART1[0]+0x0C)	= HwUART1->LCR;  //0x0C
	word_of(&pSave->uUART1[0]+0x10)	= HwUART1->MCR;  //0x10
	word_of(&pSave->uUART1[0]+0x1C)	= HwUART1->SCR;  //0x1C
	BITSET(HwUART1->LCR, Hw7);
	word_of(&pSave->uUART1[0]+0x20)	= HwUART1->DLL;  //0x00
	word_of(&pSave->uUART1[0]+0x24)	= HwUART1->DLM;  //0x04
	word_of(&pSave->uUART1[0]+0x28)	= HwUART1->AFT;  //0x20
	word_of(&pSave->uUART1[0]+0x2C)	= HwUART1->UCR;  //0x24
	word_of(&pSave->uUART1[0]+0x30)	= HwUART1->SCCR; //0x60
	word_of(&pSave->uUART1[0]+0x34)	= HwUART1->STC;  //0x64
	word_of(&pSave->uUART1[0]+0x38)	= HwUART1->IRCFG;//0x80

	// UART2
	word_of(&pSave->uUART2[0]+0x04)	= HwUART2->IER;  //0x04
	word_of(&pSave->uUART2[0]+0x0C)	= HwUART2->LCR;  //0x0C
	word_of(&pSave->uUART2[0]+0x10)	= HwUART2->MCR;  //0x10
	word_of(&pSave->uUART2[0]+0x1C)	= HwUART2->SCR;  //0x1C
	BITSET(HwUART2->LCR, Hw7);
	word_of(&pSave->uUART2[0]+0x20)	= HwUART2->DLL;  //0x00
	word_of(&pSave->uUART2[0]+0x24)	= HwUART2->DLM;  //0x04
	word_of(&pSave->uUART2[0]+0x28)	= HwUART2->AFT;  //0x20
	word_of(&pSave->uUART2[0]+0x2C)	= HwUART2->UCR;  //0x24
	word_of(&pSave->uUART2[0]+0x30)	= HwUART2->SCCR; //0x60
	word_of(&pSave->uUART2[0]+0x34)	= HwUART2->STC;  //0x64
	word_of(&pSave->uUART2[0]+0x38)	= HwUART2->IRCFG;//0x80

	// UART3
	word_of(&pSave->uUART3[0]+0x04)	= HwUART3->IER;  //0x04
	word_of(&pSave->uUART3[0]+0x0C)	= HwUART3->LCR;  //0x0C
	word_of(&pSave->uUART3[0]+0x10)	= HwUART3->MCR;  //0x10
	word_of(&pSave->uUART3[0]+0x1C)	= HwUART3->SCR;  //0x1C
	BITSET(HwUART3->LCR, Hw7);
	word_of(&pSave->uUART3[0]+0x20)	= HwUART3->DLL;  //0x00
	word_of(&pSave->uUART3[0]+0x24)	= HwUART3->DLM;  //0x04
	word_of(&pSave->uUART3[0]+0x28)	= HwUART3->AFT;  //0x20
	word_of(&pSave->uUART3[0]+0x2C)	= HwUART3->UCR;  //0x24
	word_of(&pSave->uUART3[0]+0x30)	= HwUART3->SCCR; //0x60
	word_of(&pSave->uUART3[0]+0x34)	= HwUART3->STC;  //0x64
	word_of(&pSave->uUART3[0]+0x38)	= HwUART3->IRCFG;//0x80

	// UART4
	word_of(&pSave->uUART4[0]+0x04)	= HwUART4->IER;  //0x04
	word_of(&pSave->uUART4[0]+0x0C)	= HwUART4->LCR;  //0x0C
	word_of(&pSave->uUART4[0]+0x10)	= HwUART4->MCR;  //0x10
	word_of(&pSave->uUART4[0]+0x1C)	= HwUART4->SCR;  //0x1C
	BITSET(HwUART4->LCR, Hw7);
	word_of(&pSave->uUART4[0]+0x20)	= HwUART4->DLL;  //0x00
	word_of(&pSave->uUART4[0]+0x24)	= HwUART4->DLM;  //0x04
	word_of(&pSave->uUART4[0]+0x28)	= HwUART4->AFT;  //0x20
	word_of(&pSave->uUART4[0]+0x2C)	= HwUART4->UCR;  //0x24
	word_of(&pSave->uUART4[0]+0x30)	= HwUART4->SCCR; //0x60
	word_of(&pSave->uUART4[0]+0x34)	= HwUART4->STC;  //0x64
	word_of(&pSave->uUART4[0]+0x38)	= HwUART4->IRCFG;//0x80

	// UART5
	word_of(&pSave->uUART5[0]+0x04)	= HwUART5->IER;  //0x04
	word_of(&pSave->uUART5[0]+0x0C)	= HwUART5->LCR;  //0x0C
	word_of(&pSave->uUART5[0]+0x10)	= HwUART5->MCR;  //0x10
	word_of(&pSave->uUART5[0]+0x1C)	= HwUART5->SCR;  //0x1C
	BITSET(HwUART5->LCR, Hw7);
	word_of(&pSave->uUART5[0]+0x20)	= HwUART5->DLL;  //0x00
	word_of(&pSave->uUART5[0]+0x24)	= HwUART5->DLM;  //0x04
	word_of(&pSave->uUART5[0]+0x28)	= HwUART5->AFT;  //0x20
	word_of(&pSave->uUART5[0]+0x2C)	= HwUART5->UCR;  //0x24
	word_of(&pSave->uUART5[0]+0x30)	= HwUART5->SCCR; //0x60
	word_of(&pSave->uUART5[0]+0x34)	= HwUART5->STC;  //0x64
	word_of(&pSave->uUART5[0]+0x38)	= HwUART5->IRCFG;//0x80

	memcpy(pSave->uUARTPORTMUX, (char*)&HwUARTPORTMUX_BASE, sizeof(UARTPORTMUX));

	//DMA Controller
	memcpy(pSave->uGDMA0, (char*)&HwGDMA0_BASE, sizeof(GDMACTRL));
	memcpy(pSave->uGDMA1, (char*)&HwGDMA1_BASE, sizeof(GDMACTRL));
	memcpy(pSave->uGDMA2, (char*)&HwGDMA2_BASE, sizeof(GDMACTRL));
	memcpy(pSave->uGDMA3, (char*)&HwGDMA3_BASE, sizeof(GDMACTRL));
	
	//Real Time Clock
	//memcpy(pSave->uRTC, &HwRTC_BASE, sizeof(RTC));
	
	//TouchScreen ADC
	memcpy(pSave->uTSADC, (char*)&HwTSADC_BASE, sizeof(TSADC));
	BITCLR(HwPMU->CONTROL, Hw18); // Not Isolated

	//Bruce, 090528, to reduce current leakage
	HwTSADC->ADCCON = 0x00014344;
	HwTSADC->ADCTSC = 0x00000050;
	
	//Error Correction Code
	//memcpy(pSave->uECC, (char*)&HwECC_BASE, sizeof(SLCECC));
	//Multi-Protocla Encapsulation Forward Error Correction
	memcpy(pSave->uMPEFEC, (char*)&HwMPEFEC_BASE, sizeof(MPEFEC));
	
	//IOBUS Configuration Register
	//memcpy(pSave->uIOBUSCFG, &HwIOBUSCFG_BASE, sizeof(IOBUSCFG));

#ifdef FEATURE_DDI_GRP_BACKUP
	/*--------------------------------------------------------------
	 PART6 - DDI_BUS
	--------------------------------------------------------------*/
	//LCD
	memcpy(pSave->uLCD, (char*)&HwLCDC0_BASE, sizeof(LCDC));
	memcpy(pSave->uLCD1, (char*)&HwLCDC1_BASE, sizeof(LCDC));
	memcpy(pSave->uLCDSI, (char*)&HwLCDSI_BASE, sizeof(LCDSI));
	
	//Memory To Memory Scaler
	memcpy(pSave->uM2MSCALER0, (char*)&HwM2MSCALER0_BASE, sizeof(M2MSCALER));
	memcpy(pSave->uM2MSCALER1, (char*)&HwM2MSCALER1_BASE, sizeof(M2MSCALER));
	
	//NTSCPAL
	memcpy(pSave->uNTSCPAL, (char*)&HwTVE_BASE, sizeof(NTSCPAL));
	memcpy(pSave->uNTSCPALOP, (char*)&HwTVE_VEN_BASE, sizeof(NTSCPALOP));
	//HwTVE->DACPD = 1; //Bruce, 090527, to reduce current leakage

	//HDMI
	//memcpy(pSave->uHDMICTRL, (char*)&HwHDMICTRL_BASE, sizeof(HDMICTRL));
	//memcpy(pSave->uHDMICORE, (char*)&HwHDMICORE_BASE, sizeof(HDMICORE));
	//memcpy(pSave->uHDMIAES, (char*)&HwHDMIAES_BASE, sizeof(HDMIAES));
	//memcpy(pSave->uHDMISPDIF, (char*)&HwHDMISPDIF_BASE, sizeof(HDMISPDIF));
	//memcpy(pSave->uHDMII2S, (char*)&HwHDMII2S_BASE, sizeof(HDMII2S));
	//memcpy(pSave->uHDMICEC, (char*)&HwHDMICEC_BASE, sizeof(HDMICEC));
	
	//CIF
	memcpy(pSave->uCIF, (char*)&HwCIF_BASE, sizeof(CIF));
	//Effect
	memcpy(pSave->uEFFECT, (char*)&HwCEM_BASE, sizeof(EFFECT));	
	//Scaler
	memcpy(pSave->uCIFSACLER, (char*)&HwCSC_BASE, sizeof(CIFSACLER));	
	//Video & Image Quality Enhancer
	memcpy(pSave->uVIQE, (char*)&HwVIQE_BASE, sizeof(VIQE));	
	//DDI_CACHE
	memcpy(pSave->uDDICACHE, (char*)&HwDDI_CACHE_BASE, sizeof(DDICACHE));

	//DDI_CONFIG
	//memcpy(pSave->uDDI_CONFIG, (char*)&HwLVDS_BASE, sizeof(DDICONFIG));	

	/*--------------------------------------------------------------
	 PART7 - VIDEO BUS
	--------------------------------------------------------------*/
	//Video Codec
	
	//Jpeg Codec
	//memcpy(pSave->uJPEGDECODER, (char*)&HwJPEGDECODER_BASE, sizeof(JPEGDECODER));
	//memcpy(pSave->uJPEGENCODER, (char*)&HwJPEGENCODER_BASE, sizeof(JPEGENCODER));
	
	/*--------------------------------------------------------------
	 PART8 - GRAPHIC BUS
	--------------------------------------------------------------*/
	//Overlay Mixer
	//memcpy(pSave->uOVERLAYMIXER, (char*)&HwOVERLAYMIXER_BASE, sizeof(OVERLAYMIXER));
	
	// 2D/3D GPU
	memcpy(pSave->uGPUPIXELPROCESSOR, (char*)&HwPIXELPROCESSOR_BASE, sizeof(GPUPIXELPROCESSOR));
	memcpy(pSave->uGPUGEOMETRYPROCESSOR, (char*)&HwGEOMETRYPROCESSOR_BASE, sizeof(GPUGEOMETRYPROCESSOR));
	
	//GRPBUS Configuration
	memcpy(pSave->uGPUGRPBUSBWRAP, (char*)&HwGRPBUSBWRAP_BASE, sizeof(GPUGRPBUSBWRAP));
	//memcpy(pSave->uGPUGRPBUSCONFIG, (char*)&HwGRPBUS_BASE, sizeof(GPUGRPBUSCONFIG));
#endif

	if (machine_is_tcc9200s()) {
		//Bruce, 090528, Sleep 상태에서는 USB 전원을 OFF 해야 한다. (H/W 유동석 대리)
		BITCLR(HwGPIOD->GPDAT, Hw25);
	}

	if (machine_is_tcc9200s()) {
		// change dai port to input port to reduce current leakage
		BITCLR(HwGPIOD->GPFN0, Hw20 - Hw0);
		BITCLR(HwGPIOD->GPEN, 0x1F);
	}

	if (machine_is_m57te() || machine_is_m801()) {
		// HDMI
		BITCLR(HwGPIOD->GPDAT, Hw10);

		// LCD, BL
		BITCLR(HwGPIOA->GPDAT, Hw4); // LCD_BLCTL
		BITCLR(HwGPIOD->GPDAT, Hw21); // LCD_PWREN

		//BITCLR(HwGPIOA->GPDAT, Hw2); // SHDN
	}
	
	//Enter Power Off Mode
	pSave->uMask = 0xc818c818;

	*pVaddr = pSave;
	*pPaddr = DRAM_DATA_ADDR;
	
	//jckim 101202 move to IO_ARM_SaveREG
	//local_flush_tlb_all();
	//flush_cache_all();

	IO_ARM_SaveREG(0x10000000, pSave, Awake_address);

	//Wake-up Power Off Mode
	pSave->uMask = 0;
	*pVaddr = 0; 
	*pPaddr = 0;

	// Bus Clock Enable
	HwIOBUSCFG->HCLKEN0 = 0xFFFFFFFF;
	HwIOBUSCFG->HCLKEN1 = 0xFFFFFFFF;
	HwIOBUSCFG->HRSTEN0 = 0xFFFFFFFF;
	HwIOBUSCFG->HRSTEN1 = 0xFFFFFFFF;

	//Don't move!! to reduce DAC-current leakage....
	HwTVE->DACPD = 1;

#ifdef FEATURE_DDI_GRP_BACKUP
	//DDI BUS - All Enable
	HwLVDS->PWDN		= 0x0;
	
	//GRAPHIC BUS - All Enable
	HwGRPBUS->GRPBUS_PWRDOWN = 0x0;
#endif

	//PLL
	HwCKC->PLL0CFG = ((PCKC)(pSave->uCKC))->PLL0CFG;
	HwCKC->PLL1CFG = ((PCKC)(pSave->uCKC))->PLL1CFG;
	HwCKC->PLL2CFG = ((PCKC)(pSave->uCKC))->PLL2CFG;
	#ifndef MDDR_CLOCK_SETTINT_USED
	HwCKC->PLL3CFG = ((PCKC)(pSave->uCKC))->PLL3CFG; //Bruce, mDDR에서 사용하는 PLL이므로 mDDR Device Driver 함수를 통해서 설정되어야 한다. 
	#endif

	//Divider
	HwCKC->CLKDIVC = ((PCKC)(pSave->uCKC))->CLKDIVC;
	HwCKC->CLKDIVC1 = ((PCKC)(pSave->uCKC))->CLKDIVC1;
	//HwCKC->CLKDIVC2 = ((PCKC)(pSave->uCKC))->CLKDIVC2;
	//HwCKC->CLKDIVC3 = ((PCKC)(pSave->uCKC))->CLKDIVC3;

	NopDelay();

	//Clock (Source = PLL0)
	#if 0
	HwCKC->CLKCTRL0 = ((0x1<<21) + (0x0<<20) + (0xFFFF<<4) + 0); // enable + normal mode + config(0xFFFF) + source(PLL0) = PLL0
	HwCKC->CLKCTRL1 = ((0x1<<21) + (0x0<<20) + (0x0001<<4) + 0); // enable + normal mode + divisor(0x1) + source(PLL0) = PLL0/2
	HwCKC->CLKCTRL2 = ((0x1<<21) + (0x0<<20) + (0x0001<<4) + 0); // enable + normal mode + divisor(0x1) + source(PLL0) = PLL0/2
	HwCKC->CLKCTRL3 = ((0x1<<21) + (0x0<<20) + (0x0001<<4) + 0); // enable + normal mode + divisor(0x1) + source(PLL0) = PLL0/2
	HwCKC->CLKCTRL4 = ((0x1<<21) + (0x0<<20) + (0x0001<<4) + 0); // enable + normal mode + divisor(0x1) + source(PLL0) = PLL0/2
	HwCKC->CLKCTRL5 = ((0x1<<21) + (0x0<<20) + (0x0001<<4) + 0); // enable + normal mode + divisor(0x1) + source(PLL0) = PLL0/2
	HwCKC->CLKCTRL6 = ((0x1<<21) + (0x0<<20) + (0x0001<<4) + 0); // enable + normal mode + divisor(0x1) + source(PLL0) = PLL0/2
	HwCKC->CLKCTRL7 = ((0x1<<21) + (0x0<<20) + (0x0001<<4) + 0); // enable + normal mode + divisor(0x1) + source(PLL0) = PLL0/2
	#else
	HwCKC->CLK0CTRL = ((PCKC)(pSave->uCKC))->CLK0CTRL;
	HwCKC->CLK1CTRL = ((PCKC)(pSave->uCKC))->CLK1CTRL;
	#ifndef MDDR_CLOCK_SETTINT_USED
	HwCKC->CLK2CTRL = ((PCKC)(pSave->uCKC))->CLK2CTRL; //Bruce, mDDR Clock을 Restore하기 위해서, SRAM상에서 Code가 수행되므로 SRAM Restore이후에 설정한다.
	#endif
	HwCKC->CLK3CTRL = ((PCKC)(pSave->uCKC))->CLK3CTRL;
	HwCKC->CLK4CTRL = ((PCKC)(pSave->uCKC))->CLK4CTRL;
	HwCKC->CLK5CTRL = ((PCKC)(pSave->uCKC))->CLK5CTRL;
	HwCKC->CLK6CTRL = ((PCKC)(pSave->uCKC))->CLK6CTRL;
	HwCKC->CLK7CTRL = ((PCKC)(pSave->uCKC))->CLK7CTRL;
	#endif

	//Peripheral clock
	// - this should be done by memcpy() because memcpy() reside in ITCM area.
	memcpy((char*)&(HwCKC->PCLK_TCX), pSave->uCKC+0x80, 0x114-0x80);
	
	//Boot_InitCACHE(); To Do


	//Restore Memory & system registers
	/*--------------------------------------------------------------
	 Internal Memory ( TCM & SRAM)
	--------------------------------------------------------------*/
	// ITCM
	// - this should be done by memcpy() because memcpy() reside in ITCM area.
	#ifndef _SLEEP_WITH_ITCM_BOOT_
	//memcpy(phys_to_virt(ITCM_BASE), pSave->uITCM, ITCM_SIZE);
	#endif
	// DTCM
	//memcpy(phys_to_virt(DTCM_BASE), pSave->uDTCM, DTCM_SIZE);
	// SRAM
	//memcpy(phys_to_virt(SRAM_BASE), pSave->uSRAM, SRAM_SIZE);

	#ifdef MDDR_CLOCK_SETTINT_USED
	/*--------------------------------------------------------------
	 mDDR Clock - //Bruce, mDDR Clock을 Restore하기 위해서, SRAM상에서 Code가 수행되므로 SRAM Restore이후에 설정한다.
	--------------------------------------------------------------*/
	//DEV_mDDR_SetFreq(uIO_CKC_Fclk[HwCKC_SYS_MBUS]/10000, &pll_value, &clk_value); TO DO
	#endif

	/*--------------------------------------------------------------
	 PART3 - GPIO
	--------------------------------------------------------------*/
	memcpy((char*)&HwGPIO_BASE, pSave->uGPIO, sizeof(GPIO));
	//Disable "I/O Retention"
	//After this, the port can be controlled by the processor.
	BITCLR(HwPMU->CONTROL, HwPMU_CONTROL_IOR);

	if (machine_is_tcc9200s()) {
		//Bruce, 090528, Sleep 상태에서는 USB 전원을 OFF 해야 한다. (H/W 유동석 대리)
		BITSET(HwGPIOD->GPDAT, Hw25);
	}

	/*--------------------------------------------------------------
	 PART2 - SMU & PMU
	--------------------------------------------------------------*/
	//CKC
	//memcpy((char*)&HwCKC_BASE, pSave->uCKC, sizeof(CKC));
	
	//VPIC (VECTORED PRIORITY INTERRUPT CONTROLLER)
	memcpy((char*)&HwPIC_BASE, pSave->uPIC, sizeof(PIC));
	memcpy((char*)&HwVIC_BASE, pSave->uVIC, sizeof(VIC));
	//TIMER / COUNTER
	memcpy((char*)&HwTMR_BASE, pSave->uTMR, sizeof(TIMER));
	//PMU (POWER MANAGEMENT UNIT)
	memcpy((char*)&HwPMU_BASE, pSave->uPMU, sizeof(PMU));
	//SMU_I2C
	memcpy((char*)&HwSMUI2C_MASTER0_BASE, pSave->uSMUI2C_MASTER0, sizeof(SMUI2CMASTER));
	memcpy((char*)&HwSMUI2C_MASTER1_BASE, pSave->uSMUI2C_MASTER1, sizeof(SMUI2CMASTER));
	memcpy((char*)&HwSMUI2C_COMMON_BASE, pSave->uSMUI2C_COMMON, sizeof(SMUI2CICLK));
	
	/*--------------------------------------------------------------
	 PART4 - CORE & MEMORY BUS
	--------------------------------------------------------------*/
	//SDR/DDR SDRAM Controller Registers
	//memcpy((char*)&HwDRAMM0_BASE, pSave->uDRAM, sizeof(DRAM));
	
	//DDR2 SDRAM Controller Registers
	//memcpy((char*)&HwDRAMM1_BASE, pSave->uDRAMMX, sizeof(DRAM));
	
	//SDRAM PHY Control Registers
	//memcpy((char*)&HwDRAMPHY_BASE, pSave->uDRAMPHY, sizeof(DRAMPHY));
	
	//Miscellaneous Configuration Registers
	//memcpy((char*)&HwDRAMMISC_BASE, pSave->uDRAMMISC, sizeof(DRAMMISC));
	
	//Memory Bus Configuration Registers
	//memcpy((char*)&HwDRAMMEMBUS_BASE, pSave->uDRAMMEMBUS, sizeof(DRAMMEMBUS));
	
	//Core Bus Configuration Registers
	memcpy((char*)&HwCORECFG_BASE, pSave->uCORECFG, sizeof(MISCCOREBUS));
	//Virtual MMU Table Registers
	memcpy((char*)&HwREGION_BASE, pSave->uVMTREGION, sizeof(VMTREGION));
	
	/*--------------------------------------------------------------
	 PART5 - IO BUS
	--------------------------------------------------------------*/
	//Memory Stick Host Controller
	memcpy((char*)&HwSMSHC_BASE, pSave->uSMSHC, sizeof(SMSHC));
	memcpy((char*)&HwSMSHCPORTCFG_BASE, pSave->uSMSHCPORTCFG, sizeof(SMSHCPORTCFG));
	
	//SD/SDIO/MMC/CE_ATA Host Controller	
	//memcpy((char*)&HwSDCORE0SLOT0_BASE, pSave->uSDCORE0SLOT0, sizeof(SDHOST));
	//memcpy((char*)&HwSDCORE0SLOT1_BASE, pSave->uSDCORE0SLOT1, sizeof(SDHOST));
	//memcpy((char*)&HwSDCORE1SLOT0_BASE, pSave->uSDCORE1SLOT0, sizeof(SDHOST));
	//memcpy((char*)&HwSDCORE1SLOT1_BASE, pSave->uSDCORE1SLOT1, sizeof(SDHOST));
	//memcpy((char*)&HwSDCHCTRL_BASE, pSave->uSDCHCTRL, sizeof(SDCHCTRL));
	
	//NAND Flash Controller
	//memcpy((char*)&HwNFC_BASE, pSave->uNFC, sizeof(NFC));
	tcc_nfc_resume((PNFC)tcc_p2v(HwNFC_BASE),(PNFC)pSave->uNFC);
	//Static Memory Controller
	memcpy((char*)&HwSMC_BASE, pSave->uSMC, sizeof(SMC));
	//External Device Interface
	memcpy((char*)&HwEDI_BASE, pSave->uEDI, sizeof(EDI));
	//IDE Controller
	memcpy((char*)&HwIDE_BASE, pSave->uIDE, sizeof(IDE));

	//Audio DMA
	memcpy((char*)&HwADMA_BASE, pSave->uADMA, sizeof(ADMA));
	memcpy((char*)&HwADMA_DAIBASE, pSave->uADMA_DAI, sizeof(ADMADAI));
	memcpy((char*)&HwADMA_CDIFBASE, pSave->uADMA_CDIF, sizeof(ADMACDIF));
//	memcpy((char*)&HwADMA_SPDIFTXBASE, pSave->uADMA_SPDIFTX, sizeof(ADMASPDIFTX));
//	memcpy((char*)&HwSPDIF_BASE, pSave->uADMA_SPDIFRX, sizeof(ADMASPDIFRX));
#if 0
	//DAI Interface Register
	memcpy((char*)&HwDAI_BASE, pSave->uDAI, sizeof(DAI));
	memcpy((char*)&HwCDIF_BASE, pSave->uCDIF, sizeof(CDIF));
#endif
	
	//SPDIF TRANSMITTER
	memcpy((char*)&HwSPDIF_BASE, pSave->uSPDIF, sizeof(SPDIF));
	
	//USB 2.0 OTG Controller
	memcpy((char*)&HwUSB20OTG_BASE, pSave->uUSB20OTG, sizeof(USB20OTG));
	memcpy((char*)&HwUSBOTGCFG_BASE, pSave->uUSBOTGCFG, sizeof(USBOTGCFG));
	memcpy((char*)&HwUSBPHYCFG_BASE, pSave->uUSBPHYCFG, sizeof(USBPHYCFG));
	
	//External Host Interface
	memcpy((char*)&HwEHICS0_BASE, pSave->uEHI0, sizeof(EHI));
	memcpy((char*)&HwEHICS1_BASE, pSave->uEHI1, sizeof(EHI));
	
	//General Purpose Serial Bus
	//*
	memcpy((char*)&HwGPSBCH0_BASE, pSave->uGPSB0, sizeof(GPSB));
	memcpy((char*)&HwGPSBCH1_BASE, pSave->uGPSB1, sizeof(GPSB));
	memcpy((char*)&HwGPSBCH2_BASE, pSave->uGPSB2, sizeof(GPSB));
	memcpy((char*)&HwGPSBCH3_BASE, pSave->uGPSB3, sizeof(GPSB));
	memcpy((char*)&HwGPSBCH4_BASE, pSave->uGPSB4, sizeof(GPSB));
	memcpy((char*)&HwGPSBCH5_BASE, pSave->uGPSB5, sizeof(GPSB));
	memcpy((char*)&HwGPSBPORTCFG_BASE, pSave->uGPSBPORTCFG, sizeof(GPSBPORTCFG));
	memcpy((char*)&HwGPSBPIDTABLE_BASE, pSave->uGPSBPIDTABLE, sizeof(GPSBPIDTABLE));
	
	//Transport Stream Interface
	//memcpy((char*)&HwTSIF_BASE, pSave->uTSIF, sizeof(TSIF));
	//memcpy((char*)&HwTSIFPORTSEL_BASE, pSave->uTSIFPORTSEL, sizeof(TSIFPORTSEL));
	//*/
	
	//GPS
	//memcpy((char*)&HwGPS_BASE, pSave->uGPS, sizeof(GPS));
	
	//Remote Control Interface
	memcpy((char*)&HwREMOTE_BASE, pSave->uREMOCON, sizeof(REMOTECON));
	
	//I2C
	memcpy((char*)&HwI2CMASTER0_BASE, pSave->uI2CMASTER0, sizeof(I2CMASTER));
	memcpy((char*)&HwI2CMASTER1_BASE, pSave->uI2CMASTER1, sizeof(I2CMASTER));
	memcpy((char*)&HwI2CSLAVE_BASE, pSave->uI2CSLAVE, sizeof(I2CSLAVE));
	memcpy((char*)&HwI2CSTATUS_BASE, pSave->uI2CSTATUS, sizeof(I2CSTATUS));

	//UART
	memcpy((char*)&HwUARTPORTMUX_BASE, pSave->uUARTPORTMUX, sizeof(UARTPORTMUX));

	// UART0 (Debug UART)
	BITCLR(HwUART0->LCR, Hw7);
	HwUART0->IER	= word_of(&pSave->uUART0[0]+0x04);
	HwUART0->FCR	= Hw2 + Hw1 + Hw0;
	HwUART0->LCR	= word_of(&pSave->uUART0[0]+0x0C);
	HwUART0->MCR	= word_of(&pSave->uUART0[0]+0x10);
	HwUART0->SCR	= word_of(&pSave->uUART0[0]+0x1C);
	BITSET(HwUART0->LCR, Hw7);
	HwUART0->DLL	= word_of(&pSave->uUART0[0]+0x20);
	HwUART0->DLM	= word_of(&pSave->uUART0[0]+0x24);
	HwUART0->AFT	= word_of(&pSave->uUART0[0]+0x28);
	HwUART0->UCR	= word_of(&pSave->uUART0[0]+0x2C);
	HwUART0->SCCR	= word_of(&pSave->uUART0[0]+0x30);
	HwUART0->STC	= word_of(&pSave->uUART0[0]+0x34);
	HwUART0->IRCFG	= word_of(&pSave->uUART0[0]+0x38);
	BITCLR(HwUART0->LCR, Hw7);

	// UART1 (BT UART)
	BITCLR(HwUART1->LCR, Hw7);
	HwUART1->IER	= word_of(&pSave->uUART1[0]+0x04);
	HwUART1->FCR	= Hw3 + Hw2 + Hw1 + Hw0;
	HwUART1->LCR	= word_of(&pSave->uUART1[0]+0x0C);
	HwUART1->MCR	= word_of(&pSave->uUART1[0]+0x10);
	HwUART1->SCR	= word_of(&pSave->uUART1[0]+0x1C);
	BITSET(HwUART1->LCR, Hw7);
	HwUART1->DLL	= word_of(&pSave->uUART1[0]+0x20);
	HwUART1->DLM	= word_of(&pSave->uUART1[0]+0x24);
	HwUART1->AFT	= word_of(&pSave->uUART1[0]+0x28);
	HwUART1->UCR	= word_of(&pSave->uUART1[0]+0x2C);
	HwUART1->SCCR	= word_of(&pSave->uUART1[0]+0x30);
	HwUART1->STC	= word_of(&pSave->uUART1[0]+0x34);
	HwUART1->IRCFG	= word_of(&pSave->uUART1[0]+0x38);
	BITCLR(HwUART1->LCR, Hw7);

	// UART2
	BITCLR(HwUART2->LCR, Hw7);
	HwUART2->IER	= word_of(&pSave->uUART2[0]+0x04);
	HwUART2->FCR	= Hw2 + Hw1 + Hw0;
	HwUART2->LCR	= word_of(&pSave->uUART2[0]+0x0C);
	HwUART2->MCR	= word_of(&pSave->uUART2[0]+0x10);
	HwUART2->SCR	= word_of(&pSave->uUART2[0]+0x1C);
	BITSET(HwUART2->LCR, Hw7);
	HwUART2->DLL	= word_of(&pSave->uUART2[0]+0x20);
	HwUART2->DLM	= word_of(&pSave->uUART2[0]+0x24);
	HwUART2->AFT	= word_of(&pSave->uUART2[0]+0x28);
	HwUART2->UCR	= word_of(&pSave->uUART2[0]+0x2C);
	HwUART2->SCCR	= word_of(&pSave->uUART2[0]+0x30);
	HwUART2->STC	= word_of(&pSave->uUART2[0]+0x34);
	HwUART2->IRCFG	= word_of(&pSave->uUART2[0]+0x38);
	BITCLR(HwUART2->LCR, Hw7);

	// UART3
	BITCLR(HwUART3->LCR, Hw7);
	HwUART3->IER	= word_of(&pSave->uUART3[0]+0x04);
	HwUART3->FCR	= Hw2 + Hw1 + Hw0;
	HwUART3->LCR	= word_of(&pSave->uUART3[0]+0x0C);
	HwUART3->MCR	= word_of(&pSave->uUART3[0]+0x10);
	HwUART3->SCR	= word_of(&pSave->uUART3[0]+0x1C);
	BITSET(HwUART3->LCR, Hw7);
	HwUART3->DLL	= word_of(&pSave->uUART3[0]+0x20);
	HwUART3->DLM	= word_of(&pSave->uUART3[0]+0x24);
	HwUART3->AFT	= word_of(&pSave->uUART3[0]+0x28);
	HwUART3->UCR	= word_of(&pSave->uUART3[0]+0x2C);
	HwUART3->SCCR	= word_of(&pSave->uUART3[0]+0x30);
	HwUART3->STC	= word_of(&pSave->uUART3[0]+0x34);
	HwUART3->IRCFG	= word_of(&pSave->uUART3[0]+0x38);
	BITCLR(HwUART3->LCR, Hw7);

	// UART4
	BITCLR(HwUART4->LCR, Hw7);
	HwUART4->IER	= word_of(&pSave->uUART4[0]+0x04);
	HwUART4->FCR	= Hw2 + Hw1 + Hw0;
	HwUART4->LCR	= word_of(&pSave->uUART4[0]+0x0C);
	HwUART4->MCR	= word_of(&pSave->uUART4[0]+0x10);
	HwUART4->SCR	= word_of(&pSave->uUART4[0]+0x1C);
	BITSET(HwUART4->LCR, Hw7);
	HwUART4->DLL	= word_of(&pSave->uUART4[0]+0x20);
	HwUART4->DLM	= word_of(&pSave->uUART4[0]+0x24);
	HwUART4->AFT	= word_of(&pSave->uUART4[0]+0x28);
	HwUART4->UCR	= word_of(&pSave->uUART4[0]+0x2C);
	HwUART4->SCCR	= word_of(&pSave->uUART4[0]+0x30);
	HwUART4->STC	= word_of(&pSave->uUART4[0]+0x34);
	HwUART4->IRCFG	= word_of(&pSave->uUART4[0]+0x38);
	BITCLR(HwUART4->LCR, Hw7);

	// UART5
	BITCLR(HwUART5->LCR, Hw7);
	HwUART5->IER	= word_of(&pSave->uUART5[0]+0x04);
	HwUART5->FCR	= Hw2 + Hw1 + Hw0;
	HwUART5->LCR	= word_of(&pSave->uUART5[0]+0x0C);
	HwUART5->MCR	= word_of(&pSave->uUART5[0]+0x10);
	HwUART5->SCR	= word_of(&pSave->uUART5[0]+0x1C);
	BITSET(HwUART5->LCR, Hw7);
	HwUART5->DLL	= word_of(&pSave->uUART5[0]+0x20);
	HwUART5->DLM	= word_of(&pSave->uUART5[0]+0x24);
	HwUART5->AFT	= word_of(&pSave->uUART5[0]+0x28);
	HwUART5->UCR	= word_of(&pSave->uUART5[0]+0x2C);
	HwUART5->SCCR	= word_of(&pSave->uUART5[0]+0x30);
	HwUART5->STC	= word_of(&pSave->uUART5[0]+0x34);
	HwUART5->IRCFG	= word_of(&pSave->uUART5[0]+0x38);
	BITCLR(HwUART5->LCR, Hw7);
			
	pmdrv_dbg("Wake up !!\n");
	
	//DMA Controller
	memcpy((char*)&HwGDMA0_BASE, pSave->uGDMA0, sizeof(GDMACTRL));
	memcpy((char*)&HwGDMA1_BASE, pSave->uGDMA1, sizeof(GDMACTRL));
	memcpy((char*)&HwGDMA2_BASE, pSave->uGDMA2, sizeof(GDMACTRL));
	memcpy((char*)&HwGDMA3_BASE, pSave->uGDMA3, sizeof(GDMACTRL));
	
	//Real Time Clock
	//memcpy((char*)&HwRTC_BASE, pSave->uRTC, sizeof(RTC));
	
	//TouchScreen ADC
	memcpy((char*)&HwTSADC_BASE, pSave->uTSADC, sizeof(TSADC));
	
	//Error Correction Code
	//memcpy((char*)&HwECC_BASE, pSave->uECC, sizeof(SLCECC));
	
	//Multi-Protocla Encapsulation Forward Error Correction
	memcpy((char*)&HwMPEFEC_BASE, pSave->uMPEFEC, sizeof(MPEFEC));
	
	//IOBUS Configuration Register
	//memcpy((char*)&HwIOBUSCFG_BASE, pSave->uIOBUSCFG, sizeof(IOBUSCFG));

#ifdef FEATURE_DDI_GRP_BACKUP
	/*--------------------------------------------------------------
	 PART6 - DDI_BUS
	--------------------------------------------------------------*/
	//LCD
	memcpy((char*)&HwLCDC0_BASE, pSave->uLCD, sizeof(LCDC));
	memcpy((char*)&HwLCDC1_BASE, pSave->uLCD1, sizeof(LCDC));
	memcpy((char*)&HwLCDSI_BASE, pSave->uLCDSI, sizeof(LCDSI));
	
	//Memory To Memory Scaler
	memcpy((char*)&HwM2MSCALER0_BASE, pSave->uM2MSCALER0, sizeof(M2MSCALER));
	memcpy((char*)&HwM2MSCALER1_BASE, pSave->uM2MSCALER1, sizeof(M2MSCALER));
	
	//NTSCPAL
	memcpy((char*)&HwTVE_BASE, pSave->uNTSCPAL, sizeof(NTSCPAL));
	memcpy((char*)&HwTVE_VEN_BASE, pSave->uNTSCPALOP, sizeof(NTSCPALOP));

	//HDMI
	//memcpy((char*)&HwHDMICTRL_BASE, pSave->uHDMICTRL, sizeof(HDMICTRL));
	//memcpy((char*)&HwHDMICORE_BASE, pSave->uHDMICORE, sizeof(HDMICORE));
	//memcpy((char*)&HwHDMIAES_BASE, pSave->uHDMIAES, sizeof(HDMIAES));
	//memcpy((char*)&HwHDMISPDIF_BASE, pSave->uHDMISPDIF, sizeof(HDMISPDIF));
	//memcpy((char*)&HwHDMII2S_BASE, pSave->uHDMII2S, sizeof(HDMII2S));
	//memcpy((char*)&HwHDMICEC_BASE, pSave->uHDMICEC, sizeof(HDMICEC));
	
	//CIF
	memcpy((char*)&HwCIF_BASE, pSave->uCIF, sizeof(CIF));
	
	//Effect
	memcpy((char*)&HwCEM_BASE, pSave->uEFFECT, sizeof(EFFECT)); 
	
	//Scaler
	memcpy((char*)&HwCSC_BASE, pSave->uCIFSACLER, sizeof(CIFSACLER));	
	
	//Video & Image Quality Enhancer
	memcpy((char*)&HwVIQE_BASE, pSave->uVIQE, sizeof(VIQE));	
	
	//DDI_CACHE
	memcpy((char*)&HwDDI_CACHE_BASE, pSave->uDDICACHE, sizeof(DDICACHE));

	//DDI_CONFIG
	//memcpy((char*)&HwLVDS_BASE, pSave->uDDI_CONFIG, sizeof(LVDS));	TO DO???
	
	/*--------------------------------------------------------------
	 PART7 - VIDEO BUS
	--------------------------------------------------------------*/
	//Video Codec
	
	//Jpeg Codec
	//memcpy((char*)&HwJPEGDECODER_BASE, pSave->uJPEGDECODER, sizeof(JPEGDECODER));
	//memcpy((char*)&HwJPEGENCODER_BASE, pSave->uJPEGENCODER, sizeof(JPEGENCODER));
	
	/*--------------------------------------------------------------
	 PART8 - GRAPHIC BUS
	--------------------------------------------------------------*/
	//Overlay Mixer
	//memcpy((char*)&HwOVERLAYMIXER_BASE, pSave->uOVERLAYMIXER, sizeof(OVERLAYMIXER));
	
	// 2D/3D GPU
	memcpy((char*)&HwPIXELPROCESSOR_BASE, pSave->uGPUPIXELPROCESSOR, sizeof(GPUPIXELPROCESSOR));
	memcpy((char*)&HwGEOMETRYPROCESSOR_BASE, pSave->uGPUGEOMETRYPROCESSOR, sizeof(GPUGEOMETRYPROCESSOR));
	
	//GRPBUS Configuration
	memcpy((char*)&HwGRPBUSBWRAP_BASE, pSave->uGPUGRPBUSBWRAP, sizeof(GPUGRPBUSBWRAP));
	
	memcpy((char*)&HwGRPBUS_BASE, pSave->uGPUGRPBUSCONFIG, sizeof(GPUGRPBUSCONFIG));
	//HwGRPBUS->GRPBUS_MALI_IDLE = ((PGPUGRPBUSCONFIG)pSave->uGPUGRPBUSCONFIG)->GRPBUS_MALI_IDLE;
	//HwGRPBUS->GRPBUS_PWRDOWN = ((PGPUGRPBUSCONFIG)pSave->uGPUGRPBUSCONFIG)->GRPBUS_PWRDOWN;
#endif

	//IOBUS Configuration Register - Restore Bus Clock Enable
	memcpy((char*)&HwIOBUSCFG_BASE, pSave->uIOBUSCFG, sizeof(IOBUSCFG));

	//Unlock ARM Interrupt
	IO_ARM_SetCPSR(pSave->lock);

//-------------------------------------------------------------------------------
// Initialize pripheral-device
#if 0 //TO DO
#ifdef TRIFLASH_INCLUDE
	DISK_Ioctl( DISK_DEVICE_TRIFLASH, DEV_INITIALIZE, NULL);
#endif
#ifdef MMC_INCLUDE
	DISK_Ioctl( DISK_DEVICE_MMC, DEV_INITIALIZE, NULL);
#endif
#endif
	// CODEC ?
	// LCD ?
	// T-Flash ?
	return	0;
}
EXPORT_SYMBOL(suspend_mode_on);

volatile static void tca_off_ddr2selfrefresh(void)
{
	// SDRAM into Self Refresh
	register unsigned int nCount = 0;

	*(volatile unsigned long *)0xF0102004 |= Hw2; // GPIOADAT == corebus
	nCount = *(volatile unsigned long *)0xF0102000; 			


	//Enter Self-Refresh Mode
	*(volatile unsigned long *)0xF0302004 = 0x00000003; // PL341_PAUSE
	while (((*(volatile unsigned long *)0xF0302000) & 0x3)!=2); //Wait PL34X_STATUS_PAUSED

	*(volatile unsigned long *)0xF0302004 = 0x00000001; // PL341_SLEEP
	while (((*(volatile unsigned long *)0xF0302000) & 0x3)!=3); // Wait PL34X_STATUS_LOWPOWER

	// To prevent input leakage
	*(volatile unsigned long *)0xF0304400 |= 0x00000004; 
	// DLL OFF
	*(volatile unsigned long *)0xF0304404 &=  ~(0x00000003);	 // DLL-0FF,DLL-Stop running
	*(volatile unsigned long *)0xF0304428 &= ~(0x00000003); // Calibration Start,Update Calibration
	*(volatile unsigned long *)0xF030302c &=  ~(0x00004000);  //SDRAM IO Control Register Gatein Signal Power Down
		
	nCount = ((*(volatile unsigned long *)0xF030200C) & ~(0x00004000));
	*(volatile unsigned long *)0xF030200C = nCount| (1<<14);		// Stop-MCLK Enter Self-refresh mode


	//DRAM controller power down
		*(volatile unsigned long *)0xF030302C =0x3fff;
		*(volatile unsigned long *)0xF030302C &= ~Hw14;
		for (nCount=0; nCount<10; nCount++);
		*(volatile unsigned long *)0xF0304400 = 0x2;
		for (nCount=0; nCount<10; nCount++);
		*(volatile unsigned long *)0xF0304404 &= ~(Hw0|Hw1);
		*(volatile unsigned long *)0xF0304428 &= ~(Hw0|Hw1);
		*(volatile unsigned long *)0xF0304428 |= Hw12;
		*(volatile unsigned long *)0xF0304428 |= Hw0;

		*(volatile unsigned long *)0xF0304400 = 0x6;

		nCount = 800;
		for ( ; nCount > 0 ; nCount --);		// Wait
		nCount = 800;
		for ( ; nCount > 0 ; nCount --);		// Wait


	*(volatile unsigned long *)0xF0400000 = 0x002ffff4; // CKC-CLKCTRL0 - set cpu clk to XIN
	*(volatile unsigned long *)0xF0400004 = 0x00200014; // CKC-CLKCTRL1 - set display clk to XIN
	*(volatile unsigned long *)0xF0400008 = 0x00200014; // CKC-CLKCTRL2 - set memory clk to XIN
	*(volatile unsigned long *)0xF040000c = 0x00200014; // CKC-CLKCTRL3 - set graphic clk to XIN
	*(volatile unsigned long *)0xF0400010 = 0x00200014; // CKC-CLKCTRL4 - set io clk to XIN

	*(volatile unsigned long *)0xF0400014 = 0x00200014; // CKC-CLKCTRL5 - set video bus clk to XIN
	*(volatile unsigned long *)0xF0400018 = 0x00200014; // CKC-CLKCTRL6 - set video core clk to XIN
	*(volatile unsigned long *)0xF040001c = 0x00200014; // CKC-CLKCTRL7 - set SMU clk to XIN

//Bruce, 100108, set sleep mode.
#ifdef CONFIG_SLEEP_WITH_ITCM
	*(volatile unsigned long *)0xF0404000 |= Hw4; // PMU-CONTROL - INITR : ITCM Booting
	*(volatile unsigned long *)0xF0404000 |= Hw3; // PMU-CONTROL - DPDN : Deep Power Down mode
#else
	*(volatile unsigned long *)0xF0404000 &= ~Hw4; // PMU-CONTROL - INITR : BOOT-ROM Booting
	*(volatile unsigned long *)0xF0404000 &= ~Hw3; // PMU-CONTROL - DPDN : Normal mode
#endif

	//Bruce, 100108, have to set i/o retension ,before sleep in.
	*(volatile unsigned long *)0xF0404000 |= Hw31; // PMU-CONTROL - i/o retension

    // XXX
	*(volatile unsigned long *)0xF0404000 |= 0x00000002; // PMU-CONTROL - Power Off

	*(volatile unsigned long *)0xF0400020 &= ~0x80000000; // CKC-PLL0CFG - PLL disable
	*(volatile unsigned long *)0xF0400024 &= ~0x80000000; // CKC-PLL1CFG - PLL disable
	*(volatile unsigned long *)0xF0400028 &= ~0x80000000; // CKC-PLL2CFG - PLL disable
	*(volatile unsigned long *)0xF040002c &= ~0x80000000; // CKC-PLL3CFG - PLL disable
	
	*(volatile unsigned long *)0xF0404001 = 0x00000000;
	*(volatile unsigned long *)0xF0404002 = 0x00000000;
	*(volatile unsigned long *)0xF0404008 = 0x00000000;
	*(volatile unsigned long *)0xF0404020 = 0x00000000;

	*(volatile unsigned long *)0xF0404000 |= 0x00000002; // PMU-CONTROL - Power Off

	
	 while(1)
	 {
		*(volatile unsigned long *)0xF0102000 &= ~Hw2; 			
		*(volatile unsigned long *)0xF0102000 &= ~Hw2; 			
	 }	

}

//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//-------------------------------------------------------------------------------
#if defined(CONFIG_MMC_TCC_DEMO_BOARD_SD_WAKEUP)
extern int sd_detect_flag;
#endif
volatile void copy_func_to_sram(void)
{
    volatile unsigned int *fPtr;
    volatile unsigned int *p;
    int i;

	volatile unsigned int *ptr;
	//CKC
	memcpy(uCKC, (char*)&HwCLK_BASE, sizeof(CKC));
	//VPIC (VECTORED PRIORITY INTERRUPT CONTROLLER)
	memcpy(uPIC, (char*)&HwPIC_BASE, sizeof(PIC));
	memcpy(uVIC, (char*)&HwVIC_BASE, sizeof(VIC));
	//TIMER / COUNTER
	memcpy(uTMR, (char*)&HwTMR_BASE, sizeof(TIMER));	
	memcpy(uPMU, (char*)&HwPMU_BASE, sizeof(PMU));

	if (machine_is_tcc9200s()) {
		HwPMU->PWROFF &= ~(HwPMU_PWROFF_DAC|
				   HwPMU_PWROFF_HD |
				   HwPMU_PWROFF_UP |
				   HwPMU_PWROFF_MB |
				   HwPMU_PWROFF_VB |
				   HwPMU_PWROFF_DB |
				   HwPMU_PWROFF_GB |
				   HwPMU_PWROFF_IOB);

		/* wakeup polarity: 0 = active high, 1 = active low */
		HwPMU->WKUPPOL = 0
			| HwPMU_WKUP_GPIOA02 //key0
			| HwPMU_WKUP_GPIOA03 //key1
			| HwPMU_WKUP_GPIOA12 //key2
			;
#if 1
		/* XXX: this is board specific and should be moved */
		BITCSET(HwGPIOA->GPEN, Hw5|Hw6|Hw7, Hw5|Hw6|Hw7);
		BITCSET(HwGPIOA->GPDAT, Hw5|Hw6|Hw7, 0);
#endif

#if defined(CONFIG_MMC_TCC_DEMO_BOARD_SD_WAKEUP)
		if(sd_detect_flag == 0)
			HwPMU->WKUPPOL |= HwPMU_WKUP_GPIOA13;
#endif

		HwPMU->WKUPEN = 0
			| HwPMU_WKUP_GPIOA02 //key0
			| HwPMU_WKUP_GPIOA03 //key1
			| HwPMU_WKUP_GPIOA12 //key2
#if defined(CONFIG_MMC_TCC_DEMO_BOARD_SD_WAKEUP)
			| HwPMU_WKUP_GPIOA13
#endif
			| HwPMU_WKUP_RTCWKUP
			;
		HwPMU->PWROFF &= ~(HwPMU_PWROFF_DAC|
				   HwPMU_PWROFF_HD |
				   HwPMU_PWROFF_UP |
				   HwPMU_PWROFF_MB |
				   HwPMU_PWROFF_VB |
				   HwPMU_PWROFF_DB |
				   HwPMU_PWROFF_GB |
				   HwPMU_PWROFF_IOB);
		// jmlee, USB Current leakage patch
		HwPMU->PWROFF = 0
			| HwPMU_PWROFF_UP;
	} else {
		/* TCC8900/TCC9201 */
		/* set wakeup enable */
		if (machine_is_m57te() || machine_is_m801()) {
			HwPMU->WKUPEN = 0
				| HwPMU_WKUP_GPIOA03
#if defined(CONFIG_MMC_TCC_DEMO_BOARD_SD_WAKEUP)
				| HwPMU_WKUP_GPIOA10
#endif
				| HwPMU_WKUP_RTCWKUP
				;
		}
		else {
			HwPMU->WKUPEN = 0
				| HwPMU_WKUP_GPIOA03
#if defined(CONFIG_MMC_TCC_DEMO_BOARD_SD_WAKEUP)
				| HwPMU_WKUP_GPIOA06
#endif
				| HwPMU_WKUP_RTCWKUP
				;
		}
		
		/* wakeup polarity: 0 = active high, 1 = active low */
		HwPMU->WKUPPOL = 0
			| HwPMU_WKUP_GPIOA03
			;

#if defined(CONFIG_MMC_TCC_DEMO_BOARD_SD_WAKEUP)
		if( sd_detect_flag == 0 ) {
			if (machine_is_m57te() || machine_is_m801())
				HwPMU->WKUPPOL |= HwPMU_WKUP_GPIOA10;
			else
				HwPMU->WKUPPOL |= HwPMU_WKUP_GPIOA06;
		}
#endif
	
		//Bruce, 090718, Before registers of each block are accessed, PMU.PowerOff must be released,
		HwPMU->PWROFF &= ~(HwPMU_PWROFF_DAC|
				   HwPMU_PWROFF_HD |
				   HwPMU_PWROFF_UP |
				   HwPMU_PWROFF_MB |
				   HwPMU_PWROFF_VB |
				   HwPMU_PWROFF_DB |
				   HwPMU_PWROFF_GB |
				   HwPMU_PWROFF_IOB);
		// jmlee, USB Current leakage patch
		HwPMU->PWROFF = 0
			| HwPMU_PWROFF_UP;
	}

	if (machine_is_tcc9200s()) {
		fPtr = (volatile unsigned int *)IO_CKC_EnterSRFInt;
	
		p = (volatile unsigned int *)SRAM_ADDR_STANDBY;

		for (i = 0; i < (SRAM_FUNC_SIZE+0x100); i++) {
			*(p++) = *(fPtr++);
		}

		while (--i)
			;

		suspend_mode_on();
	} else {
		/* TCC8900/TCC9201 */
#if 0
		tcc_pca953x_setup(PCA9539_U3_SLAVE_ADDR, PWRGP4, OUTPUT, LOW, SET_DIRECTION | SET_VALUE);	msleep(50);
//		tcc_pca953x_setup(PCA9539_U3_SLAVE_ADDR, SD0, OUTPUT, HIGH, SET_DIRECTION | SET_VALUE);	msleep(50);
//		tcc_pca953x_setup(PCA9539_U3_SLAVE_ADDR, SD1, OUTPUT, H, SET_DIRECTION | SET_VALUE);	msleep(50);

		tcc_pca953x_setup(PCA9538_U4_SLAVE_ADDR, PWR_GP0, OUTPUT, HIGH, SET_DIRECTION | SET_VALUE);	msleep(50);
		tcc_pca953x_setup(PCA9538_U4_SLAVE_ADDR, PWR_GP1, OUTPUT, LOW, SET_DIRECTION | SET_VALUE);	msleep(50);
		tcc_pca953x_setup(PCA9538_U4_SLAVE_ADDR, PWR_GP2, OUTPUT, HIGH, SET_DIRECTION | SET_VALUE);	msleep(50);
		tcc_pca953x_setup(PCA9538_U4_SLAVE_ADDR, PWR_GP3, OUTPUT, HIGH, SET_DIRECTION | SET_VALUE);	msleep(50);
#endif
		fPtr = (volatile unsigned int *)tca_off_ddr2selfrefresh;
	
		p = (volatile unsigned int *)SRAM_ADDR_STANDBY;

		for (i = 0; i < (SRAM_FUNC_SIZE+0x100); i++) {
			*(p++) = *(fPtr++);
		}
	
		while (--i)
		;

#ifdef CONFIG_MACH_TCC8900
		//101202 removed by jckim
		//IO_ARM_CleanCACHE(1);
#endif
		suspend_mode_on(); 
	}
}

static int tcc_pm_enter(suspend_state_t state)
{
	if (state != PM_SUSPEND_MEM) {
		return -EINVAL;
	}
	pmdrv_dbg("[%s] Start func\n", __func__);
	copy_func_to_sram();
	pmdrv_dbg("[%s] End func\n", __func__);
	return 0;
}

static int tcc_pm_begin(suspend_state_t state)
{
	HwCKC->CLK0CTRL |= HwCLKCTRL_EN;
	HwCKC->CLK1CTRL |= HwCLKCTRL_EN;
	HwCKC->CLK2CTRL |= HwCLKCTRL_EN;
	HwCKC->CLK3CTRL |= HwCLKCTRL_EN;
	HwCKC->CLK4CTRL |= HwCLKCTRL_EN;
	HwCKC->CLK5CTRL |= HwCLKCTRL_EN;
	HwCKC->CLK6CTRL |= HwCLKCTRL_EN;
	HwCKC->CLK7CTRL |= HwCLKCTRL_EN;

	HwPMU->PWROFF &= ~(Hw9 | Hw8 | Hw7 | Hw6 | Hw5 | Hw3 | Hw1  | Hw0);

	mdelay(10);	
	HwCKC->SWRESET &= ~(HwSWRESET_SMU_ON | HwSWRESET_VCC_ON | HwSWRESET_VB_ON
						| HwSWRESET_IO_ON | HwSWRESET_GB_ON | HwSWRESET_MB_ON
						| HwSWRESET_DDIB_ON | HwSWRESET_CPU_ON);
	
	HwCKC->PCLK_TCX |= Hw28;
	HwCKC->PCLK_TCT |= Hw28;
	HwCKC->PCLK_TCZ |= Hw28;
	HwCKC->PCLK_LCD0 |= Hw28;
	HwCKC->PCLK_LCD1 |= Hw28;
	HwCKC->PCLK_LCDSI |= Hw28;
	HwCKC->PCLK_CIFMC |= Hw28;
	HwCKC->PCLK_CIFSC |= Hw28;
	HwCKC->PCLK_OUT0 |= Hw28;
	HwCKC->PCLK_OUT1 |= Hw28;
	HwCKC->PCLK_USB11H |= Hw28;
	HwCKC->PCLK_UART0 |= Hw28;
	HwCKC->PCLK_UART1 |= Hw28;
	HwCKC->PCLK_UART2 |= Hw28;
	HwCKC->PCLK_UART3 |= Hw28;
	HwCKC->PCLK_UART4 |= Hw28;
	HwCKC->PCLK_UART5 |= Hw28;
	HwCKC->PCLK_GPSB0 |= Hw28;
	HwCKC->PCLK_GPSB1 |= Hw28;
	HwCKC->PCLK_GPSB2 |= Hw28;
	HwCKC->PCLK_GPSB3 |= Hw28;
	HwCKC->PCLK_GPSB4 |= Hw28;
	HwCKC->PCLK_GPSB5 |= Hw28;
	HwCKC->PCLK_ADC |= Hw28;
	HwCKC->PCLK_SPDIF |= Hw28;
	HwCKC->PCLK_EHI0 |= Hw28;
	HwCKC->PCLK_EHI1 |= Hw28;
	HwCKC->PCLK_AUD |= Hw28;
	HwCKC->PCLK_CAN |= Hw28;
	HwCKC->PCLK_DAI |= Hw28;

	if (machine_is_tcc9200s()) {
		HwPMU->WKUPEN = 0
			| HwPMU_WKUP_GPIOA02 //key0
			| HwPMU_WKUP_GPIOA03 //key1
			| HwPMU_WKUP_GPIOA12 //key2
#if defined(CONFIG_MMC_TCC_DEMO_BOARD_SD_WAKEUP)
			| HwPMU_WKUP_GPIOA13
#endif
			| HwPMU_WKUP_RTCWKUP
			;
	}

	pPaddr = ioremap_nocache(DRAM_PHSY_ADDR, PAGE_ALIGN(sizeof(unsigned int)));
	pVaddr = ioremap_nocache(DRAM_VIRT_ADDR, PAGE_ALIGN(sizeof(unsigned int)));
	pSave  = ioremap_nocache(DRAM_DATA_ADDR, PAGE_ALIGN(sizeof(TCC92X_REG)));

	return 0;
}

static int tcc_pm_prepare(void)
{
	//BITCSET(HwGPIOE->GPFN0, HwPORTCFG_GPFN0_MASK|HwPORTCFG_GPFN1_MASK, HwPORTCFG_GPFN0(0)|HwPORTCFG_GPFN1(0));	// functions : GPIO
	return 0;
}

static void tcc_pm_finish(void)
{
	//BITCSET(HwGPIOE->GPFN0, HwPORTCFG_GPFN0_MASK|HwPORTCFG_GPFN1_MASK, HwPORTCFG_GPFN0(1)|HwPORTCFG_GPFN1(1));	// functions : GPIO
	//Free pSave
	if (pSave != NULL) {
		iounmap(pSave);
		pSave = NULL;
	}

	if (pPaddr != NULL) {
		iounmap(pPaddr);
		pPaddr = NULL;
	}

	if (pVaddr != NULL) {
		iounmap(pVaddr);
		pVaddr = NULL;
	}

	return 0;
}


static void tcc_pm_recover(void)
{

	return 0;
}

static void tcc_pm_end(void)
{
	
	clk_disable_unused();
	return 0;
}

static struct platform_suspend_ops tcc_pm_ops = {
	.valid	= suspend_valid_only_mem,
	.begin	= tcc_pm_begin,
	.prepare = tcc_pm_prepare,	
	.enter	= tcc_pm_enter,
	.finish = tcc_pm_finish,
	.end	= tcc_pm_end,
	.recover = tcc_pm_recover,

};

static uint32_t restart_reason = 0x776655AA;

static void tcc_pm_restart(char str)
{
	volatile uint32_t *usersts = (volatile uint32_t *) PMU_USERSTS;

	/* store restart_reason to USERSTS register */
	if (restart_reason != 0x776655AA)
		*usersts = restart_reason;

	printk(KERN_DEBUG "reboot: reason=0x%x\n", restart_reason);

	arch_reset(str, NULL);
}

static int tcc_reboot_call(struct notifier_block *this, unsigned long code, void *cmd)
{
	if (code == SYS_RESTART) {
		if (cmd) {
			if (!strcmp(cmd, "bootloader")) {
				restart_reason = FASTBOOT_MODE;
			} else if (!strcmp(cmd, "recovery")) {
				restart_reason = RECOVERY_MODE;
			} else {
				restart_reason = 0x77665501;
			}
		} else {
			restart_reason = 0x77665501;
		}
	}
	return NOTIFY_DONE;
}

static struct notifier_block tcc_reboot_notifier = {
	.notifier_call = tcc_reboot_call,
};

static int __init tcc_pm_init(void)
{
	pm_power_off = tcc_pm_power_off;
	arm_pm_restart = tcc_pm_restart;

	register_reboot_notifier(&tcc_reboot_notifier);

	suspend_set_ops(&tcc_pm_ops);
	return 0;
}

__initcall(tcc_pm_init);
