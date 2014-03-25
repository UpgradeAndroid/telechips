/*
 * arch/arm/mach-tcc9300/pm.c  
 *
 * Author:  <linux@telechips.com>
 * Created: October, 2009
 * Description: LINUX POWER MANAGEMENT FUNCTIONS
 *
 * Copyright (C) 2008-2009 Telechips 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 * ChangeLog:
 *
 *
 */

/*===========================================================================

                         INCLUDE FILES FOR MODULE

===========================================================================*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/io.h>
#include <linux/reboot.h>

#include <asm/tlbflush.h>
#include <linux/syscalls.h>		// sys_sync()
#include <asm/cacheflush.h>		// local_flush_tlb_all(), flush_cache_all();

//#include <bsp.h>
//#include <mach/tcc_ckc_ctrl.h>		// arm_changestack(), arm_restorestack()
#include <mach/system.h>
#include <mach/pm.h>

#include <mach/tcc_ddr.h>



/*===========================================================================

                  DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/

#if !defined(CONFIG_TCC93XX_AX)
#define TCC_BUS_SWRESET_USED
#endif

//Bruce, Shut-down, Sleep, WFI mode에서 Bus Off 할 때에, MEM Bus도 OFF하는 option.
#define TCC_MEMBUS_PWROFF_USED

#ifdef TCC_PM_SHUTDOWN_MODE
extern void BackupRAM_Boot(void);
extern void save_cpu_reg(int sram_addr, unsigned int pReg, void *);
extern void resore_cpu_reg(void);
#define addr(x) (0xB0000000+x)
#endif

#ifdef TCC_PM_SLEEP_MODE
extern unsigned int IO_ARM_ChangeStackSRAM(void);
extern void IO_ARM_RestoreStackSRAM(unsigned int);

typedef void (*FuncPtr)(void);

#define addr(x) (0xF0000000+x)
#endif

#define time2cycle(time, tCK)		((int)((time + tCK - 1) / tCK))

#define RECOVERY_MODE  0x02
#define FASTBOOT_MODE  0x01

#ifdef CONFIG_PM_VERBOSE
#define pmdrv_dbg(fmt, arg...)     printk(fmt, ##arg)
#else
#define pmdrv_dbg(arg...)
#endif

#define NopDelay30() {\
	__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n");\
	__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n");\
	__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n");\
	__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n");\
	__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n");\
	__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n"); __asm__ volatile ("nop\n");__asm__ volatile ("nop\n");\
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void port_arrange(void)
{

	//arrange GPIO for reduce sleep current
	if (0) {//machine_is_tcc9300st()) {
		// GPIO_A
		*(volatile unsigned long *)0xF010A024 = 0x00100000;		//GPIOA Config - [5] IRDI, oherwise gpio
		*(volatile unsigned long *)0xF010A028 = 0x00000000;		//GPIOA Config - gpio
		*(volatile unsigned long *)0xF010A02C = 0x00000000;		//GPIOA Config - gpio
		*(volatile unsigned long *)0xF010A030 = 0x00000000;		//GPIOA Config - gpio
		*(volatile unsigned long *)0xF010A01C = 0x00000000;		//GPIOA Pull-up/down disable
		*(volatile unsigned long *)0xF010A020 = 0x00000000;		//GPIOA Pull-up/down disable
		*(volatile unsigned long *)0xF010A004 = 0xFFFFBFE7;		//GPIOA [3],[4],[14] input, otherwise output
		*(volatile unsigned long *)0xF010A000 = 0x00000000;		//GPIOA Low

		// GPIO_B
		*(volatile unsigned long *)0xF010A064 = 0x00000000;		//GPIOB config - gpio
		*(volatile unsigned long *)0xF010A068 = 0x00000000;		//GPIOB config - gpio
		*(volatile unsigned long *)0xF010A06C = 0x00111010;		//GPIOB [17],[19~21] NAND, otherwise gpio
		*(volatile unsigned long *)0xF010A070 = 0x11101010;		//GPIOB [25], [27], [29~31] NAND, otherwise gpio	
		*(volatile unsigned long *)0xF010A05C = 0x00000000;		//GPIOB Pull-up/down disable
		*(volatile unsigned long *)0xF010A060 = 0x00000000;		//GPIOB [31] Pull-down, otherwise Pull-up/down disable
		*(volatile unsigned long *)0xF010A044 = 0xFFFFFFFF;		//GPIOB all output
		*(volatile unsigned long *)0xF010A040 = 0x00000000 | 0xff;		//GPIOB [0~7] High, otherwize Low

		// GPIO_C
		*(volatile unsigned long *)0xF010A0A4 = 0x00000000;		//GPIOC Config - gpio
		*(volatile unsigned long *)0xF010A0A8 = 0x00000000;		//GPIOC Config - gpio
		*(volatile unsigned long *)0xF010A0AC = 0x00000000;		//GPIOC Config - gpio
		*(volatile unsigned long *)0xF010A0B0 = 0x00000000;		//GPIOC Config - gpio
		*(volatile unsigned long *)0xF010A09C = 0x00000000;		//GPIOC Pull-up/down disable
		*(volatile unsigned long *)0xF010A0A0 = 0x00000000;		//GPIOC Pull-up/down disable
		*(volatile unsigned long *)0xF010A084 = 0xFFFFFFFF;		//GPIOC output
		*(volatile unsigned long *)0xF010A080 = 0x00000000;		//GPIOC Low	

		// GPIO_D
		*(volatile unsigned long *)0xF010A0E4 = 0x00000000;		//GPIOD Config - gpio
		*(volatile unsigned long *)0xF010A0E8 = 0x00000000;		//GPIOD Config - gpio
		*(volatile unsigned long *)0xF010A0EC = 0x00000000;		//GPIOD Config - gpio
		*(volatile unsigned long *)0xF010A0F0 = 0x00000000;		//GPIOD Config - gpio
		*(volatile unsigned long *)0xF010A0DC = 0x00000000;		//GPIOD Pull-up/down disable
		*(volatile unsigned long *)0xF010A0E0 = 0x00000000;		//GPIOD Pull-up/down disable
		*(volatile unsigned long *)0xF010A0C4 = 0xFFFFFFFF;		//GPIOD output
		*(volatile unsigned long *)0xF010A0C0 = 0x00000000 | Hw6;		//GPIOD Low : SLEEP_MODF_CTL(D6)

		// GPIO_E
		*(volatile unsigned long *)0xF010A124 = 0x00000000;		//GPIOE Config - gpio
		*(volatile unsigned long *)0xF010A128 = 0x00000000;		//GPIOE Config - gpio
		*(volatile unsigned long *)0xF010A12C = 0x00000000;		//GPIOE Config - gpio
		*(volatile unsigned long *)0xF010A130 = 0x00000000;		//GPIOE Config - gpio
		*(volatile unsigned long *)0xF010A11C = 0x00000000;		//GPIOE Pull-up/down disable
		*(volatile unsigned long *)0xF010A120 = 0x00000000;		//GPIOE Pull-up/down disable
		*(volatile unsigned long *)0xF010A104 = 0x8EFDFFFF;		//GPIOE [17],[24],[28],[29],[30] input, otherwise output
		*(volatile unsigned long *)0xF010A100 = 0x00000000;

		// GPIO_F
		*(volatile unsigned long *)0xF010A164 = 0x00000000;		//GPIOF Config - gpio
		*(volatile unsigned long *)0xF010A168 = 0x00000000;		//GPIOF Config - gpio
		*(volatile unsigned long *)0xF010A16C = 0x00000000;		//GPIOF Config - gpio
		*(volatile unsigned long *)0xF010A170 = 0x00000000;		//GPIOF Config - gpio
		*(volatile unsigned long *)0xF010A15C = 0x00000000;		//GPIOF Pull-up/down disable
		*(volatile unsigned long *)0xF010A160 = 0x00000000;		//GPIOF Pull-up/down disable
		*(volatile unsigned long *)0xF010A144 = 0xFFFFFFDF;		//GPIOF [5]input, otherwise output	
		#ifdef TCC_MEMBUS_PWR_CTRL_USED
		*(volatile unsigned long *)0xF010A140 = 0x00000000 | Hw7;		//GPIOF Low : VDDQ_MEM_ON(F7)
		#else
		*(volatile unsigned long *)0xF010A140 = 0x00000000 | Hw7;		//GPIOF Low
		#endif

		// GPIO_G
		*(volatile unsigned long *)0xF010A1A4 = 0x00000000;		//GPIOG Config - gpio
		*(volatile unsigned long *)0xF010A1A8 = 0x00000000;		//GPIOG Config - gpio
		*(volatile unsigned long *)0xF010A1AC = 0x00000000;		//GPIOG Config - gpio
		*(volatile unsigned long *)0xF010A1B0 = 0x00000000;		//GPIOG Config - gpio
		*(volatile unsigned long *)0xF010A19C = 0x00000000;		//GPIOG Pull-up/down disable
		*(volatile unsigned long *)0xF010A1A0 = 0x00000000;		//GPIOG Pull-up/down disable
		*(volatile unsigned long *)0xF010A184 = 0xF5FFFFFF;		//GPIOG [25], [27] input, outhersize output
		*(volatile unsigned long *)0xF010A180 = 0x00000000 | Hw11 | Hw12;		//GPIOG Low : CORE_CTL0(G11), CORE_CTL1(G12)
 	}
}


#ifdef TCC_PM_SHUTDOWN_MODE
/*===========================================================================

                                 Shut-down

===========================================================================*/

/*===========================================================================
VARIABLE
===========================================================================*/
static TCC_REG *p93reg;

static TCC_REG reg_backup;

/*===========================================================================
FUNCTION
===========================================================================*/
static void shutdown(void)
{
	volatile unsigned int nCount = 0;

#if defined(CONFIG_DRAM_DDR3)
	*(volatile unsigned int *)addr(0x303020) &= ~(0x1<<18); // CSYSREQ3
	if(DDR3_BANK_NUM == 8)
		while (((*(volatile unsigned long *)addr(0x30c208)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
	else//DDR3_BANK_NUM is 4
		while (((*(volatile unsigned long *)addr(0x30c208)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED
#elif defined(CONFIG_DRAM_DDR2)// && defined(CONFIG_DDR2_LPCON)
	*(volatile unsigned int *)addr(0x303020) &= ~(0x1<<17); // CSYSREQ2
	if(DDR2_BANK_NUM == 8)
		while (((*(volatile unsigned long *)addr(0x305048)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
	else//DDR2_BANK_NUM == 4
		while (((*(volatile unsigned long *)addr(0x305048)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED
#else
 #error "not selected"
#endif

	for (nCount = 20000; nCount > 0; nCount --);	// Wait

	*(volatile unsigned long *)addr(0x500000) = 0x002ffff4; // CKC-CLKCTRL0 - set cpu clk to XIN
	*(volatile unsigned long *)addr(0x500004) = 0x00200014; // CKC-CLKCTRL1 - set display clk to XIN
	*(volatile unsigned long *)addr(0x500008) = 0x00200014; // CKC-CLKCTRL2 - set memory clk to XIN
	*(volatile unsigned long *)addr(0x50000C) = 0x00200014; // CKC-CLKCTRL3 - set graphic clk to XIN
	*(volatile unsigned long *)addr(0x500010) = 0x00200014; // CKC-CLKCTRL4 - set io clk to XIN
	*(volatile unsigned long *)addr(0x500014) = 0x00200014; // CKC-CLKCTRL5 - set video bus clk to XIN
	*(volatile unsigned long *)addr(0x500018) = 0x00200014; // CKC-CLKCTRL6 - set video core clk to XIN
	*(volatile unsigned long *)addr(0x50001C) = 0x00200014; // CKC-CLKCTRL7 - set SMU clk to XIN
	*(volatile unsigned long *)addr(0x500020) = 0x00200014; // CKC-CLKCTRL8 - set HSIObus clk to XIN
	*(volatile unsigned long *)addr(0x500024) = 0x00200014; // CKC-CLKCTRL9 - set Camera busclk to XIN
	*(volatile unsigned long *)addr(0x500028) = 0x00200014; // CKC-CLKCTRL10 - set display sub bus clk to XIN
	*(volatile unsigned long *)addr(0x500030) &= ~0x80000000; // CKC-PLL0CFG - PLL disable
	*(volatile unsigned long *)addr(0x500034) &= ~0x80000000; // CKC-PLL1CFG - PLL disable
	*(volatile unsigned long *)addr(0x500038) &= ~0x80000000; // CKC-PLL2CFG - PLL disable
	*(volatile unsigned long *)addr(0x50003C) &= ~0x80000000; // CKC-PLL3CFG - PLL disable
	*(volatile unsigned long *)addr(0x500040) &= ~0x80000000; // CKC-PLL4CFG - PLL disable
	*(volatile unsigned long *)addr(0x500044) &= ~0x80000000; // CKC-PLL5CFG - PLL disable

	//MEM_PDN
#if (0)
	*(volatile unsigned long *)addr(0x50303C) |= (Hw0|Hw1|Hw2|Hw3); //SD
	*(volatile unsigned long *)addr(0x50303C) &= ~(Hw4|Hw5); //EHI
	*(volatile unsigned long *)addr(0x50303C) &= ~(0x3<<6); //USB
	*(volatile unsigned long *)addr(0x50303C) &= ~(0x3F<<8); //Overlay Mixer
	*(volatile unsigned long *)addr(0x50303C) &= ~(Hw15); //NFC
	*(volatile unsigned long *)addr(0x50303C) |= (0x3<<16); //IRAM0
	//*(volatile unsigned long *)addr(0x50303C) |= (0x3<<18); //IRAM1
	*(volatile unsigned long *)addr(0x50303C) &= ~(Hw20); //CPU Prefetch Buffer
	*(volatile unsigned long *)addr(0x50303C) &= ~(Hw21); //IO Prefetch Buffer
	*(volatile unsigned long *)addr(0x50303C) &= ~(Hw22); //HSIO Prefetch Buffer
#endif

#ifdef CONFIG_MACH_TCC9300ST
	/* SLEEP_MODE_CTRL : GPIO_D6 */
	*(volatile unsigned long *)addr(0x10A0C4) |= Hw6;	// GPIO_D6
	*(volatile unsigned long *)addr(0x10A0C0) &= ~Hw6;	// low GPIO_D6

	/*
		CORE_CTL0 : GPIO_G11
		CORE_CTL1 : GPIO_G12

		CORE_CTL0 CORE_CTL1 : Core Voltage
		   Low       Low    :     0.95 V
		   Low       High   :     1.35 V
		   High      Low    :     0.97 V
		   High      High   :     1.45 V		
	*/

	// 0.95V
	*(volatile unsigned long *)addr(0x10a184) |= Hw11 | Hw12; // set output GPIO_G11 & GPIO_G12
	*(volatile unsigned long *)addr(0x10a180) &= ~Hw11;	// set GPIO_G11 to low
	*(volatile unsigned long *)addr(0x10a180) &= ~Hw12;	// set GPIO_G12 to low
#endif

#ifdef TCC_MEMBUS_PWR_CTRL_USED
	#ifdef CONFIG_MACH_TCC9300ST
	*(volatile unsigned long *)addr(0x10A144) |= Hw7;		//GPIO_F7
	*(volatile unsigned long *)addr(0x10A140) &= ~Hw7;		//GPIO_F7	
	#else
	//*(volatile unsigned long *)addr(0x10A044) |= Hw31;
	//*(volatile unsigned long *)addr(0x10A040) &= ~Hw31;
#endif
#endif

	/* Power Down */
	//*(volatile unsigned long *)addr(0x50303C) |= (Hw16|Hw17|Hw18|Hw19); // PWRCFG.MEM_PDN : IRAM0/1 Power Down
	*(volatile unsigned long *)addr(0x50301C) |= Hw0; // PWRCFG.BSRC : Reboot from Backup RAM
	*(volatile unsigned long *)addr(0x503014) |= Hw12; // CONFIG1.BBEN : Backup RAM Boot Enable
	*(volatile unsigned long *)addr(0x503000) |= (Hw28|Hw31); // CONTROL.IOR_M|IOR : I/O Retention

	//BUS Power Off
	*(volatile unsigned long *)addr(0x50005C) |= (Hw1|Hw3|Hw5|Hw6|Hw8|Hw9|Hw10); //reset on
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw0|Hw1|Hw2|Hw3|Hw4|Hw26); //ip power-off
#ifdef TCC_MEMBUS_PWROFF_USED
	*(volatile unsigned long *)addr(0x30B004) |= (Hw0); //Memory Bus Config . SW Reset
	for (nCount = 20; nCount > 0; nCount --);	// Wait
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw5|Hw8|Hw11|Hw14|Hw17|Hw20|Hw23); //iso : 0
	*(volatile unsigned long *)addr(0x503018) |= (Hw6|Hw9|Hw12|Hw15|Hw18|Hw21|Hw24); //pre-off : 1
	*(volatile unsigned long *)addr(0x503018) |= (Hw7|Hw10|Hw13|Hw16|Hw19|Hw22|Hw25); //off : 1
#else
	for (nCount = 20; nCount > 0; nCount --);	// Wait
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw5|Hw8|Hw11|Hw14|Hw17|Hw20); //iso : 0
	*(volatile unsigned long *)addr(0x503018) |= (Hw6|Hw9|Hw12|Hw15|Hw18|Hw21); //pre-off : 1
	*(volatile unsigned long *)addr(0x503018) |= (Hw7|Hw10|Hw13|Hw16|Hw19|Hw22); //off : 1
#endif

	*(volatile unsigned long *)addr(0x503000) |= Hw2; // CONTROL.SHTDN : Shutdown.!!
	
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void io_restore(void)
{
	volatile int i;
	volatile unsigned *src;
	volatile unsigned *dest;

	src = (unsigned*)GPIO_BUF_PHY;
	dest = (unsigned*)addr(0x10A000);//&HwGPIO_BASE;

	for(i=0 ; i<(sizeof(GPIO)/sizeof(unsigned)) ; i++)
		dest[i] = src[i];

    *(volatile unsigned int *)addr(0x503000) &= ~(0x1<<28); // IO Retention Dis
    *(volatile unsigned int *)addr(0x503000) &= ~(0x1<<31); // IO Retention Dis

#ifdef TCC_MEMBUS_PWR_CTRL_USED
	#ifdef CONFIG_MACH_TCC9300ST
	*(volatile unsigned long *)addr(0x10A144) |= Hw7;		//GPIO_F7
	*(volatile unsigned long *)addr(0x10A140) |= Hw7;		//GPIO_F7	
	#else
	//*(volatile unsigned long *)addr(0x10A044) |= Hw31;
	//*(volatile unsigned long *)addr(0x10A040) |= Hw31;
	#endif
#endif

#ifdef CONFIG_MACH_TCC9300ST
 	/* SLEEP_MODE_CTRL : GPIO_D6 */
	*(volatile unsigned long *)addr(0x10A0C4) |= Hw6;	// GPIO_D6
	*(volatile unsigned long *)addr(0x10A0C0) |= Hw6;	// high GPIO_D6

	/*
		CORE_CTL0 : GPIO_G11
		CORE_CTL1 : GPIO_G12

		CORE_CTL0 CORE_CTL1 : Core Voltage
		   Low       Low    :     0.95 V
		   Low       High   :     1.35 V
		   High      Low    :     0.97 V
		   High      High   :     1.45 V		
	*/
	// 1.35V
	*(volatile unsigned long *)addr(0x10a184) |= Hw11 | Hw12; // set output GPIO_G11 & GPIO_G12
	*(volatile unsigned long *)addr(0x10a180) &= ~Hw11;	// set GPIO_G11 to low
	*(volatile unsigned long *)addr(0x10a180) |= Hw12;	// set GPIO_G12 to high
#endif

	*(volatile unsigned long *)addr(0x50005C) |= (Hw1|Hw3|Hw5|Hw6|Hw8|Hw9|Hw10); //reset on
	*(volatile unsigned long *)addr(0x30B004) |= (Hw0); //Memory Bus Config . SW Reset

	//BUS Power On
#ifdef TCC_MEMBUS_PWROFF_USED
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw6|Hw9|Hw12|Hw15|Hw18|Hw21|Hw24); //pre-off : 0
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw7|Hw10|Hw13|Hw16|Hw19|Hw22|Hw25); //off : 0
	*(volatile unsigned long *)addr(0x503018) |= (Hw5|Hw8|Hw11|Hw14|Hw17|Hw20|Hw23); //iso : 1
	for (i = 20; i > 0; i --);	// Wait
	*(volatile unsigned long *)addr(0x30B004) &= ~(Hw0); //Memory Bus Config . SW Reset
#else
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw6|Hw9|Hw12|Hw15|Hw18|Hw21); //pre-off : 0
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw7|Hw10|Hw13|Hw16|Hw19|Hw22); //off : 0
	*(volatile unsigned long *)addr(0x503018) |= (Hw5|Hw8|Hw11|Hw14|Hw17|Hw20); //iso : 1
	for (i = 20; i > 0; i --);	// Wait
#endif
	*(volatile unsigned long *)addr(0x50005C) &= ~(Hw1|Hw3|Hw5|Hw6|Hw8|Hw9|Hw10); //reset off
	*(volatile unsigned long *)addr(0x503018) |= (Hw0|Hw1|Hw2|Hw3|Hw4|Hw26); //ip power-on

	//MEM_PDN
#if (0)
	*(volatile unsigned long *)addr(0x50303C) &= ~(Hw0|Hw1|Hw2|Hw3); //SD
	*(volatile unsigned long *)addr(0x50303C) |= (Hw4|Hw5); //EHI
	*(volatile unsigned long *)addr(0x50303C) |= (0x3<<6); //USB
	*(volatile unsigned long *)addr(0x50303C) |= (0x3F<<8); //Overlay Mixer
	*(volatile unsigned long *)addr(0x50303C) |= (Hw15); //NFC
	*(volatile unsigned long *)addr(0x50303C) &= ~(0x3<<16); //IRAM0
	//*(volatile unsigned long *)addr(0x50303C) &= ~(0x3<<18); //IRAM1
	*(volatile unsigned long *)addr(0x50303C) |= (Hw20); //CPU Prefetch Buffer
	*(volatile unsigned long *)addr(0x50303C) |= (Hw21); //IO Prefetch Buffer
	*(volatile unsigned long *)addr(0x50303C) |= (Hw22); //HSIO Prefetch Buffer
#endif
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void tcc_nfc_suspend(PNFC pBackupNFC, PNFC pHwNFC)
{
	pBackupNFC->NFC_CACYC = pHwNFC->NFC_CACYC;
	pBackupNFC->NFC_WRCYC = pHwNFC->NFC_WRCYC;
	pBackupNFC->NFC_RECYC = pHwNFC->NFC_RECYC;
	pBackupNFC->NFC_CTRL = pHwNFC->NFC_CTRL;
	pBackupNFC->NFC_IRQCFG = pHwNFC->NFC_IRQCFG;
	pBackupNFC->NFC_RFWBASE = pHwNFC->NFC_RFWBASE;
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void tcc_nfc_resume(PNFC pHwNFC, PNFC pBackupNFC)
{
	pHwNFC->NFC_CACYC = pBackupNFC->NFC_CACYC;
	pHwNFC->NFC_WRCYC = pBackupNFC->NFC_WRCYC;
	pHwNFC->NFC_RECYC = pBackupNFC->NFC_RECYC;
	pHwNFC->NFC_CTRL = pBackupNFC->NFC_CTRL;
	pHwNFC->NFC_IRQCFG = pBackupNFC->NFC_IRQCFG;
	pHwNFC->NFC_IRQ = 0xFFFFFFFF;
	pHwNFC->NFC_RFWBASE = pBackupNFC->NFC_RFWBASE;
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void shutdown_mode(void)
{
	volatile unsigned int nCount = 0;
	CKC *ckc = (CKC *)tcc_p2v(HwCLK_BASE); 
	PIC *pic = (PIC *)tcc_p2v(HwPRIOINTRCTR_BASE); 
	VIC *vic = (VIC *)tcc_p2v(HwVECTINTRCTR_BASE); 
	TIMER *timer = (TIMER *)tcc_p2v(HwTMR_BASE); 
	PMU *pmu = (PMU *)tcc_p2v(HwPMU_BASE); 
	SMUCONFIG *smuconfig = (SMUCONFIG*)tcc_p2v(HwSMUCONFIG_BASE);
	GPIO *gpio = (GPIO *)tcc_p2v(HwGPIO_BASE);

	I2CMASTER *i2c0_0 = (I2CMASTER *)tcc_p2v(HwI2C_CORE0_BASE); 
	I2CMASTER *i2c0_1 = (I2CMASTER *)tcc_p2v(HwI2C_CORE0_CH1_BASE); 
	I2CSLAVE *i2c0_slave = (I2CSLAVE *)tcc_p2v(HwI2C_CORE0_SLAVE_BASE);
	
	I2CMASTER *i2c1_0 = (I2CMASTER *)tcc_p2v(HwI2C_CORE1_BASE); 
	I2CMASTER *i2c1_1 = (I2CMASTER *)tcc_p2v(HwI2C_CORE1_CH1_BASE); 
	I2CSLAVE *i2c1_slave = (I2CSLAVE *)tcc_p2v(HwI2C_CORE1_SLAVE_BASE);
	
	I2CMASTER *i2c2_0 = (I2CMASTER *)tcc_p2v(HwI2C_CORE2_BASE); 
	I2CMASTER *i2c2_1 = (I2CMASTER *)tcc_p2v(HwI2C_CORE2_CH1_BASE); 
	I2CSLAVE *i2c2_slave = (I2CSLAVE *)tcc_p2v(HwI2C_CORE2_SLAVE_BASE);
	
	SMUI2CMASTER *smui2c = (SMUI2CMASTER *)tcc_p2v(HwSMUI2C_BASE); 
	SMUI2CMASTER *smui2c_ch1 = (SMUI2CMASTER *)tcc_p2v(HwSMUI2C_CH1_BASE); 	
	SMUI2CICLK *smui2cclk = (SMUI2CICLK *)tcc_p2v(HwSMU_I2CICLK_BASE); 

	ADMA *adma0 = (ADMA *)tcc_p2v(HwADMA0_BASE);
	ADMADAI *adma0_dai = (ADMADAI *)tcc_p2v(HwDAI0_BASE);
	ADMASPDIFTX *adma0_spdiftx = (ADMASPDIFTX *)tcc_p2v(HwSPDIF0TX_BASE);

	IOBUSCFG *iobuscfg = (IOBUSCFG *)tcc_p2v(HwIOBUSCFG_BASE);
	TSADC *tsadc = (TSADC *)tcc_p2v(HwTSADC_BASE);

#ifdef CONFIG_MACH_TCC9300ST
	UART *uartport0 = (UART *)tcc_p2v(HwUARTCH1_BASE);
#else
	UART *uartport0 = (UART *)tcc_p2v(HwUARTCH0_BASE);
#endif
	UARTPORTMUX *uartportmux = (UARTPORTMUX *)tcc_p2v(HwUARTPORTMUX_BASE);
	NFC *nfc = (NFC *)tcc_p2v(HwNFC_BASE);

/*
 * BACKUP REGISTER
 */

	p93reg = &reg_backup;

	/* backup iobus state */	
	p93reg->backup_peri_iobus0 = iobuscfg->HCLKMASK0;
	p93reg->backup_peri_iobus1 = iobuscfg->HCLKMASK1;

	/* all peri io bus on */
	iobuscfg->HCLKMASK0 = 0x0;
	iobuscfg->HCLKMASK1 = 0x0;

	/*--------------------------------------------------------------
	 IO BUS
	--------------------------------------------------------------*/
	//i2c
	memcpy(&p93reg->smui2c,smui2c,sizeof(SMUI2CMASTER));
	memcpy(&p93reg->smui2c_ch1,smui2c_ch1,sizeof(SMUI2CMASTER));
	memcpy(&p93reg->smui2cclk,smui2cclk,sizeof(SMUI2CICLK));
	
	memcpy(&p93reg->i2c0_0,i2c0_0,sizeof(I2CMASTER));
	memcpy(&p93reg->i2c0_1,i2c0_1,sizeof(I2CMASTER));
	p93reg->i2c0_slave.CTL = i2c0_slave->CTL;
	memcpy(&p93reg->i2c1_0,i2c1_0,sizeof(I2CMASTER));
	memcpy(&p93reg->i2c1_1,i2c1_1,sizeof(I2CMASTER));
	p93reg->i2c1_slave.CTL = i2c1_slave->CTL;
	memcpy(&p93reg->i2c2_0,i2c2_0,sizeof(I2CMASTER));
	memcpy(&p93reg->i2c2_1,i2c2_1,sizeof(I2CMASTER));
	p93reg->i2c2_slave.CTL = i2c2_slave->CTL;
	
	// Audio
	p93reg->adma0.TransCtrl = adma0->TransCtrl;
	p93reg->adma0.RptCtrl   = adma0->RptCtrl;
	p93reg->adma0.ChCtrl    = adma0->ChCtrl;
	p93reg->adma0.GIntReq   = adma0->GIntReq;

	p93reg->adma0_dai.DAMR  = adma0_dai->DAMR;
	p93reg->adma0_dai.DAVC  = adma0_dai->DAVC;
	p93reg->adma0_dai.MCCR0 = adma0_dai->MCCR0;
	p93reg->adma0_dai.MCCR1 = adma0_dai->MCCR1;

	p93reg->adma0_spdiftx.TxConfig  = adma0_spdiftx->TxConfig;
	p93reg->adma0_spdiftx.TxChStat  = adma0_spdiftx->TxChStat;
	p93reg->adma0_spdiftx.TxIntMask = adma0_spdiftx->TxIntMask;
	p93reg->adma0_spdiftx.DMACFG    = adma0_spdiftx->DMACFG;
    
	memcpy(&p93reg->tsadc, tsadc, sizeof(TSADC));
	tcc_nfc_suspend(&p93reg->nfc, nfc);

	pmdrv_dbg("Enter Suspend_mode !!\n");
	// UART
	word_of(&p93reg->UART0[0]+0x04)	= uartport0->REG2.IER;  //0x04
	BITCLR(uartport0->REG2.IER, Hw2);	//disable interrupt	
	word_of(&p93reg->UART0[0]+0x0C)	= uartport0->LCR;  //0x0C			
	BITSET(uartport0->LCR, Hw7);	// DLAB = 1
	word_of(&p93reg->UART0[0]+0x20)	= uartport0->REG1.DLL;  //0x00
	word_of(&p93reg->UART0[0]+0x24)	= uartport0->REG2.DLM;  //0x04
	word_of(&p93reg->UART0[0]+0x10)	= uartport0->MCR;  //0x10
	word_of(&p93reg->UART0[0]+0x28)	= uartport0->AFT;  //0x20
	word_of(&p93reg->UART0[0]+0x2C)	= uartport0->UCR;  //0x24
	memcpy(&p93reg->uartportmux,uartportmux,sizeof(UARTPORTMUX));
	
	memcpy(&p93reg->iobuscfg, iobuscfg, sizeof(IOBUSCFG));	

	/*--------------------------------------------------------------
	 SMU & PMU
	--------------------------------------------------------------*/
	memcpy(&p93reg->smuconfig, smuconfig, sizeof(SMUCONFIG));
	memcpy(&p93reg->pmu, pmu, sizeof(PMU));
	memcpy(&p93reg->timer, timer, sizeof(TIMER));
	memcpy(&p93reg->vic, vic, sizeof(VIC));
	memcpy(&p93reg->pic, pic, sizeof(PIC));	
	memcpy(&p93reg->ckc, ckc, sizeof(CKC));

	/*--------------------------------------------------------------
	 GPIO
	--------------------------------------------------------------*/
	memcpy((void*)GPIO_BUF_ADDR, gpio, sizeof(GPIO));
	port_arrange();	//for saving current

	/////////////////////////////////////////////////////////////////
	save_cpu_reg(SHUTDOWN_FUNC_PHY, REG_MMU_DATA_ADDR, resore_cpu_reg);
	/////////////////////////////////////////////////////////////////

	__asm__ __volatile__ ("nop\n");

	/* all peri io bus on */
	iobuscfg->HCLKMASK0 = 0x0;
	iobuscfg->HCLKMASK1 = 0x0;

	/*--------------------------------------------------------------
	 SMU & PMU
	--------------------------------------------------------------*/
	//memcpy(ckc, &p93reg->ckc, sizeof(CKC));
	{
		//PLL
		//ckc->PLL0CFG = p93reg->ckc.PLL0CFG;
		ckc->PLL1CFG = p93reg->ckc.PLL1CFG;
		ckc->PLL2CFG = p93reg->ckc.PLL2CFG;
#if defined(CONFIG_MEM_CLK_SYNC_MODE)
		ckc->PLL3CFG = p93reg->ckc.PLL3CFG; //PLL3 is used as Memory Bus Clock
#endif
		ckc->PLL4CFG = p93reg->ckc.PLL4CFG;
		ckc->PLL5CFG = p93reg->ckc.PLL5CFG;

		nCount = 1000; while(nCount--) NopDelay30();

		//Divider
		ckc->CLKDIVC = p93reg->ckc.CLKDIVC;
		ckc->CLKDIVC1 = p93reg->ckc.CLKDIVC1;
		ckc->CLKDIVC2 = p93reg->ckc.CLKDIVC2;
		NopDelay30();

		//ckc->CLK0CTRL = p93reg->ckc.CLK0CTRL;
		ckc->CLK1CTRL = p93reg->ckc.CLK1CTRL;
		//ckc->CLK2CTRL = p93reg->ckc.CLK2CTRL; //Memory Clock can't be adjusted freely.
		ckc->CLK3CTRL = p93reg->ckc.CLK3CTRL;
		ckc->CLK4CTRL = p93reg->ckc.CLK4CTRL;
		ckc->CLK5CTRL = p93reg->ckc.CLK5CTRL;
		ckc->CLK6CTRL = p93reg->ckc.CLK6CTRL;
		//ckc->CLK7CTRL = p93reg->ckc.CLK7CTRL;
		ckc->CLK8CTRL = p93reg->ckc.CLK8CTRL;
		ckc->CLK9CTRL = p93reg->ckc.CLK9CTRL;
		ckc->CLK10CTRL = p93reg->ckc.CLK10CTRL;
		#ifdef TCC_BUS_SWRESET_USED
		ckc->SWRESET = p93reg->ckc.SWRESET;
		#endif

		//Peripheral clock
		memcpy((void*)&(ckc->PCLK_TCX), (void*)&(p93reg->ckc.PCLK_TCX), 0x138-0x80);
	}
	NopDelay30();

	memcpy(pic, &p93reg->pic, sizeof(PIC));
	memcpy(vic, &p93reg->vic, sizeof(VIC));
	memcpy(timer, &p93reg->timer, sizeof(TIMER));

	//memcpy(pmu, &p93reg->pmu, sizeof(PMU));
	{
		//pmu->PWROFF = p93reg->pmu.PWROFF;

		//ip power
		pmu->PWROFF &= ~((~(p93reg->pmu.PWROFF & (Hw0|Hw1|Hw2|Hw3|Hw4|Hw26)))&(Hw0|Hw1|Hw2|Hw3|Hw4|Hw26));
		
		if(p93reg->pmu.PWROFF&Hw7) //if DDI Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw5); //iso : 0
			pmu->PWROFF |= (Hw6); //pre-off : 1
			pmu->PWROFF |= (Hw7); //off : 1
		}
		if(p93reg->pmu.PWROFF&Hw10) //if DDI Sub Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw8); //iso : 0
			pmu->PWROFF |= (Hw9); //pre-off : 1
			pmu->PWROFF |= (Hw10); //off : 1
		}
		if(p93reg->pmu.PWROFF&Hw13) //if Video Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw11); //iso : 0
			pmu->PWROFF |= (Hw12); //pre-off : 1
			pmu->PWROFF |= (Hw13); //off : 1
		}
		if(p93reg->pmu.PWROFF&Hw16) //if Graphic Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw14); //iso : 0
			pmu->PWROFF |= (Hw15); //pre-off : 1
			pmu->PWROFF |= (Hw16); //off : 1
		}
		if(p93reg->pmu.PWROFF&Hw19) //if Camera Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw17); //iso : 0
			pmu->PWROFF |= (Hw18); //pre-off : 1
			pmu->PWROFF |= (Hw19); //off : 1
		}
		if(p93reg->pmu.PWROFF&Hw22) //if High Speed I/O Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw20); //iso : 0
			pmu->PWROFF |= (Hw21); //pre-off : 1
			pmu->PWROFF |= (Hw22); //off : 1
		}
		if(p93reg->pmu.PWROFF&Hw25) //if Memory Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw23); //iso : 0
			pmu->PWROFF |= (Hw24); //pre-off : 1
			pmu->PWROFF |= (Hw25); //off : 1
		}
	}

	memcpy(smuconfig, &p93reg->smuconfig, sizeof(SMUCONFIG));

	/*--------------------------------------------------------------
	 IO BUS
	--------------------------------------------------------------*/
	memcpy(iobuscfg, &p93reg->iobuscfg, sizeof(IOBUSCFG));

	// UART
	memcpy(uartportmux, &p93reg->uartportmux, sizeof(UARTPORTMUX));
	BITCLR(uartport0->REG2.IER, Hw2);	// disable interrupt	
	BITSET(uartport0->LCR, Hw7);	// DLAB = 1
	uartport0->REG3.FCR	= Hw2 + Hw1 + Hw0;
	uartport0->REG1.DLL	= word_of(&p93reg->UART0[0]+0x20);
	uartport0->REG2.DLM	= word_of(&p93reg->UART0[0]+0x24);
	uartport0->MCR	= word_of(&p93reg->UART0[0]+0x10);
	uartport0->AFT	= word_of(&p93reg->UART0[0]+0x28);
	uartport0->UCR	= word_of(&p93reg->UART0[0]+0x2C);
	uartport0->LCR	= word_of(&p93reg->UART0[0]+0x0C);	
	uartport0->REG2.IER	= word_of(&p93reg->UART0[0]+0x04);

	pmdrv_dbg("Wake up !!\n");
	tcc_nfc_resume(nfc, &p93reg->nfc);
	memcpy(tsadc, &p93reg->tsadc, sizeof(TSADC));	

	// Audio
	adma0->TransCtrl = p93reg->adma0.TransCtrl;
	adma0->RptCtrl   = p93reg->adma0.RptCtrl;
	adma0->ChCtrl    = p93reg->adma0.ChCtrl;
	adma0->GIntReq   = p93reg->adma0.GIntReq;

	adma0_dai->DAMR  = p93reg->adma0_dai.DAMR;
	adma0_dai->DAVC  = p93reg->adma0_dai.DAVC;
	adma0_dai->MCCR0  = p93reg->adma0_dai.MCCR0;
	adma0_dai->MCCR1  = p93reg->adma0_dai.MCCR1;

	adma0_spdiftx->TxConfig  = p93reg->adma0_spdiftx.TxConfig;
	adma0_spdiftx->TxChStat  = p93reg->adma0_spdiftx.TxChStat;
	adma0_spdiftx->TxIntMask = p93reg->adma0_spdiftx.TxIntMask;
	adma0_spdiftx->DMACFG    = p93reg->adma0_spdiftx.DMACFG;
    
	//i2c	
	memcpy(i2c0_0,&p93reg->i2c0_0,sizeof(I2CMASTER));
	memcpy(i2c0_1,&p93reg->i2c0_1,sizeof(I2CMASTER));
	i2c0_slave->CTL = p93reg->i2c0_slave.CTL;	
	memcpy(i2c1_0,&p93reg->i2c1_0,sizeof(I2CMASTER));
	memcpy(i2c1_1,&p93reg->i2c1_1,sizeof(I2CMASTER));
	i2c1_slave->CTL = p93reg->i2c1_slave.CTL;	
	memcpy(i2c2_0,&p93reg->i2c2_0,sizeof(I2CMASTER));
	memcpy(i2c2_1,&p93reg->i2c2_1,sizeof(I2CMASTER));
	i2c2_slave->CTL = p93reg->i2c2_slave.CTL;

	memcpy(smui2c,&p93reg->smui2c,sizeof(SMUI2CMASTER));
	memcpy(smui2c_ch1,&p93reg->smui2c_ch1,sizeof(SMUI2CMASTER));
	memcpy(smui2cclk,&p93reg->smui2cclk,sizeof(SMUI2CICLK));
	
	//all peri io bus restore
	iobuscfg->HCLKMASK0 = p93reg->backup_peri_iobus0;
	iobuscfg->HCLKMASK1 = p93reg->backup_peri_iobus1;

}
/*=========================================================================*/
#endif /* TCC_PM_SHUTDOWN_MODE */



#ifdef TCC_PM_SLEEP_MODE
/*===========================================================================

                                 SLEEP

===========================================================================*/

/*===========================================================================
FUNCTION
===========================================================================*/
static void sleep(void)
{
	volatile unsigned int nCount = 0;
	//CKC *ckc = (CKC *)tcc_p2v(HwCLK_BASE);

// Enter SDRAM Self-refresh ------------------------------------------------------------

#if defined(CONFIG_DRAM_DDR3)
	*(volatile unsigned int *)addr(0x303020) &= ~(0x1<<18); // CSYSREQ3
	if(DDR3_BANK_NUM == 8)
		while (((*(volatile unsigned long *)addr(0x30c208)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
	else//DDR3_BANK_NUM is 4
		while (((*(volatile unsigned long *)addr(0x30c208)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED
#elif defined(CONFIG_DRAM_DDR2)// && defined(CONFIG_DDR2_LPCON)
	*(volatile unsigned int *)addr(0x303020) &= ~(0x1<<17); // CSYSREQ2
	if(DDR2_BANK_NUM == 8)
		while (((*(volatile unsigned long *)addr(0x305048)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
	else//DDR2_BANK_NUM == 4
		while (((*(volatile unsigned long *)addr(0x305048)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED
#endif

	for (nCount = 20000; nCount > 0; nCount --);	// Wait

// Clock Disable ------------------------------------------------------------

#ifdef TCC_PM_SLEEP_WFI_USED
	*(volatile unsigned long *)addr(0x50004C) = 0x00008a00;

	for (nCount = 20000; nCount > 0; nCount --);	// Wait

#ifdef CONFIG_MACH_TCC9300ST
	*(volatile unsigned long *)addr(0x500000) = 0x002ffffe; // CKC-CLKCTRL0 - set cpu clk to XTIN
#else
	*(volatile unsigned long *)addr(0x500000) = 0x002ffff7; // CKC-CLKCTRL0 - set cpu clk to XTIN
#endif
	if(*(volatile unsigned long *)addr(0x500004) & 0x00200000)
		*(volatile unsigned long *)addr(0x500004) = 0x00200017; // CKC-CLKCTRL1 - set display clk to XTIN
#ifdef CONFIG_MACH_TCC9300ST
	*(volatile unsigned long *)addr(0x500008) = 0x0020001e; // CKC-CLKCTRL2 - set memory clk to XTIN
#else
	*(volatile unsigned long *)addr(0x500008) = 0x00200017; // CKC-CLKCTRL2 - set memory clk to XTIN
#endif
	if(*(volatile unsigned long *)addr(0x50000C) & 0x00200000)
		*(volatile unsigned long *)addr(0x50000C) = 0x00200017; // CKC-CLKCTRL3 - set graphic clk to XTIN
#ifdef CONFIG_MACH_TCC9300ST
	*(volatile unsigned long *)addr(0x500010) = 0x0020001e; // CKC-CLKCTRL4 - set io clk to XTIN
#else
	*(volatile unsigned long *)addr(0x500010) = 0x00200017; // CKC-CLKCTRL4 - set io clk to XTIN
#endif
	if(*(volatile unsigned long *)addr(0x500014) & 0x00200000)
		*(volatile unsigned long *)addr(0x500014) = 0x00200017; // CKC-CLKCTRL5 - set video bus clk to XTIN
	if(*(volatile unsigned long *)addr(0x500018) & 0x00200000)
		*(volatile unsigned long *)addr(0x500018) = 0x00200017; // CKC-CLKCTRL6 - set video core clk to XTIN
#ifdef CONFIG_MACH_TCC9300ST
	*(volatile unsigned long *)addr(0x50001C) = 0x0020001e; // CKC-CLKCTRL7 - set SMU clk to XTIN
#else
	*(volatile unsigned long *)addr(0x50001C) = 0x00200017; // CKC-CLKCTRL7 - set SMU clk to XTIN
#endif
	if(*(volatile unsigned long *)addr(0x500020) & 0x00200000)
		*(volatile unsigned long *)addr(0x500020) = 0x00200017; // CKC-CLKCTRL8 - set HSIObus clk to XTIN
	if(*(volatile unsigned long *)addr(0x500024) & 0x00200000)
		*(volatile unsigned long *)addr(0x500024) = 0x00200017; // CKC-CLKCTRL9 - set Camera busclk to XTIN
	if(*(volatile unsigned long *)addr(0x500028) & 0x00200000)
		*(volatile unsigned long *)addr(0x500028) = 0x00200017; // CKC-CLKCTRL10 - set display sub bus clk to XTIN
#else
	*(volatile unsigned long *)addr(0x500000) = 0x002ffff7; // CKC-CLKCTRL0 - set cpu clk to XTIN
	if(*(volatile unsigned long *)addr(0x500004) & 0x00200000)
		*(volatile unsigned long *)addr(0x500004) = 0x00200017; // CKC-CLKCTRL1 - set display clk to XTIN
	*(volatile unsigned long *)addr(0x500008) = 0x00200017; // CKC-CLKCTRL2 - set memory clk to XTIN
	if(*(volatile unsigned long *)addr(0x50000C) & 0x00200000)
		*(volatile unsigned long *)addr(0x50000C) = 0x00200017; // CKC-CLKCTRL3 - set graphic clk to XTIN
	*(volatile unsigned long *)addr(0x500010) = 0x00200017; // CKC-CLKCTRL4 - set io clk to XTIN
	if(*(volatile unsigned long *)addr(0x500014) & 0x00200000)
		*(volatile unsigned long *)addr(0x500014) = 0x00200017; // CKC-CLKCTRL5 - set video bus clk to XTIN
	if(*(volatile unsigned long *)addr(0x500018) & 0x00200000)
		*(volatile unsigned long *)addr(0x500018) = 0x00200017; // CKC-CLKCTRL6 - set video core clk to XTIN
	*(volatile unsigned long *)addr(0x50001C) = 0x00200017; // CKC-CLKCTRL7 - set SMU clk to XTIN
	if(*(volatile unsigned long *)addr(0x500020) & 0x00200000)
		*(volatile unsigned long *)addr(0x500020) = 0x00200017; // CKC-CLKCTRL8 - set HSIObus clk to XTIN
	if(*(volatile unsigned long *)addr(0x500024) & 0x00200000)
		*(volatile unsigned long *)addr(0x500024) = 0x00200017; // CKC-CLKCTRL9 - set Camera busclk to XTIN
	if(*(volatile unsigned long *)addr(0x500028) & 0x00200000)
		*(volatile unsigned long *)addr(0x500028) = 0x00200017; // CKC-CLKCTRL10 - set display sub bus clk to XTIN
#endif

	*(volatile unsigned long *)addr(0x500030) &= ~0x80000000; // CKC-PLL0CFG - PLL disable
	*(volatile unsigned long *)addr(0x500034) &= ~0x80000000; // CKC-PLL1CFG - PLL disable
	*(volatile unsigned long *)addr(0x500038) &= ~0x80000000; // CKC-PLL2CFG - PLL disable
	*(volatile unsigned long *)addr(0x50003C) &= ~0x80000000; // CKC-PLL3CFG - PLL disable
	*(volatile unsigned long *)addr(0x500040) &= ~0x80000000; // CKC-PLL4CFG - PLL disable
	*(volatile unsigned long *)addr(0x500044) &= ~0x80000000; // CKC-PLL5CFG - PLL disable

// Enable MEM_PWDN --------------------------------------------------------------------
#if (0)
	*(volatile unsigned long *)addr(0x50303C) |= (Hw0|Hw1|Hw2|Hw3); //SD
	*(volatile unsigned long *)addr(0x50303C) &= ~(Hw4|Hw5); //EHI
	*(volatile unsigned long *)addr(0x50303C) &= ~(0x3<<6); //USB
	*(volatile unsigned long *)addr(0x50303C) &= ~(0x3F<<8); //Overlay Mixer
	*(volatile unsigned long *)addr(0x50303C) &= ~(Hw15); //NFC
	*(volatile unsigned long *)addr(0x50303C) |= (0x3<<16); //IRAM0
	//*(volatile unsigned long *)addr(0x50303C) |= (0x3<<18); //IRAM1
	*(volatile unsigned long *)addr(0x50303C) &= ~(Hw20); //CPU Prefetch Buffer
	*(volatile unsigned long *)addr(0x50303C) &= ~(Hw21); //IO Prefetch Buffer
	*(volatile unsigned long *)addr(0x50303C) &= ~(Hw22); //HSIO Prefetch Buffer
#endif

#ifdef CONFIG_MACH_TCC9300ST
	/* SLEEP_MODE_CTRL : GPIO_D6 */
	*(volatile unsigned long *)addr(0x10A0C4) |= Hw6;	// GPIO_D6
	*(volatile unsigned long *)addr(0x10A0C0) &= ~Hw6;	// low GPIO_D6

	/*
		CORE_CTL0 : GPIO_G11
		CORE_CTL1 : GPIO_G12

		CORE_CTL0 CORE_CTL1 : Core Voltage
		   Low       Low    :     0.95 V
		   Low       High   :     1.35 V
		   High      Low    :     0.97 V
		   High      High   :     1.45 V		
	*/

	// 0.95V
	*(volatile unsigned long *)addr(0x10a184) |= Hw11 | Hw12; // set output GPIO_G11 & GPIO_G12
	*(volatile unsigned long *)addr(0x10a180) &= ~Hw11;	// set GPIO_G11 to low
	*(volatile unsigned long *)addr(0x10a180) &= ~Hw12;	// set GPIO_G12 to low
#endif

#ifdef TCC_MEMBUS_PWR_CTRL_USED
// ZQ/VDDQ Power OFF ------------------------------------------------------------------
	#ifdef CONFIG_MACH_TCC9300ST
	*(volatile unsigned long *)addr(0x10A144) |= Hw7;		//GPIO_F7
	*(volatile unsigned long *)addr(0x10A140) &= ~Hw7;		//GPIO_F7	
	#else
	//*(volatile unsigned long *)addr(0x10A044) |= Hw31;
	//*(volatile unsigned long *)addr(0x10A040) &= ~Hw31;
#endif
#endif

// Enable I/O Retention ---------------------------------------------------------------
	*(volatile unsigned long *)addr(0x503000) |= (Hw28|Hw31); // CONTROL.IOR_M|IOR : I/O Retention

#ifdef TCC_BUS_SWRESET_USED
// BUS Power Off ----------------------------------------------------------------------
	*(volatile unsigned long *)addr(0x50005C) |= (Hw1|Hw3|Hw5|Hw6|Hw8|Hw9|Hw10); //reset on
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw0|Hw1|Hw2|Hw3|Hw4|Hw26); //ip power-off
#ifdef TCC_MEMBUS_PWROFF_USED
	*(volatile unsigned long *)addr(0x30B004) |= (Hw0); //Memory Bus Config . SW Reset
	for (nCount = 20; nCount > 0; nCount --);	// Wait
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw5|Hw8|Hw11|Hw14|Hw17|Hw20|Hw23); //iso : 0
	*(volatile unsigned long *)addr(0x503018) |= (Hw6|Hw9|Hw12|Hw15|Hw18|Hw21|Hw24); //pre-off : 1
	*(volatile unsigned long *)addr(0x503018) |= (Hw7|Hw10|Hw13|Hw16|Hw19|Hw22|Hw25); //off : 1
#else
	for (nCount = 20; nCount > 0; nCount --);	// Wait
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw5|Hw8|Hw11|Hw14|Hw17|Hw20); //iso : 0
	*(volatile unsigned long *)addr(0x503018) |= (Hw6|Hw9|Hw12|Hw15|Hw18|Hw21); //pre-off : 1
	*(volatile unsigned long *)addr(0x503018) |= (Hw7|Hw10|Hw13|Hw16|Hw19|Hw22); //off : 1
#endif
#endif

// Sleep mode -------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////
#ifdef TCC_PM_SLEEP_WFI_USED
	asm("mcr p15, 0, r0, c7, c0, 4");   /* wait for interrupt */
#else
	*(volatile unsigned long *)addr(0x503000) |= Hw1; // CONTROL.SLEEP : Sleep.!!
#endif
///////////////////////////////////////////////////////////////////////////////////////

// Disable I/O Retention --------------------------------------------------------------
	*(volatile unsigned long *)addr(0x503000) &= ~(Hw28|Hw31); // CONTROL.IOR_M|IOR : I/O Retention

#ifdef TCC_MEMBUS_PWR_CTRL_USED
// ZQ/VDDQ Power ON -------------------------------------------------------------------
	#ifdef CONFIG_MACH_TCC9300ST
	*(volatile unsigned long *)addr(0x10A144) |= Hw7;		//GPIO_F7
	*(volatile unsigned long *)addr(0x10A140) |= Hw7;		//GPIO_F7	
	#else
	//*(volatile unsigned long *)addr(0x10A044) |= Hw31;
	//*(volatile unsigned long *)addr(0x10A040) |= Hw31;
#endif
#endif

#ifdef CONFIG_MACH_TCC9300ST
 	/* SLEEP_MODE_CTRL : GPIO_D6 */
	*(volatile unsigned long *)addr(0x10A0C4) |= Hw6;	// GPIO_D6
	*(volatile unsigned long *)addr(0x10A0C0) |= Hw6;	// high GPIO_D6

	/*
		CORE_CTL0 : GPIO_G11
		CORE_CTL1 : GPIO_G12

		CORE_CTL0 CORE_CTL1 : Core Voltage
		   Low       Low    :     0.95 V
		   Low       High   :     1.35 V
		   High      Low    :     0.97 V
		   High      High   :     1.45 V		
	*/

	//1.35V
	*(volatile unsigned long *)addr(0x10a184) |= Hw11 | Hw12; // set output GPIO_G11 & GPIO_G12
	*(volatile unsigned long *)addr(0x10a180) &= ~Hw11;	// set GPIO_G11 to low
	*(volatile unsigned long *)addr(0x10a180) |= Hw12;	// set GPIO_G12 to high
#endif

#ifdef TCC_BUS_SWRESET_USED
// BUS Power On -----------------------------------------------------------------------
#ifdef TCC_MEMBUS_PWROFF_USED
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw6|Hw9|Hw12|Hw15|Hw18|Hw21|Hw24); //pre-off : 0
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw7|Hw10|Hw13|Hw16|Hw19|Hw22|Hw25); //off : 0
	*(volatile unsigned long *)addr(0x503018) |= (Hw5|Hw8|Hw11|Hw14|Hw17|Hw20|Hw23); //iso : 1
	for (nCount = 20; nCount > 0; nCount --);	// Wait
	*(volatile unsigned long *)addr(0x30B004) &= ~(Hw0); //Memory Bus Config . SW Reset
#else
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw6|Hw9|Hw12|Hw15|Hw18|Hw21); //pre-off : 0
	*(volatile unsigned long *)addr(0x503018) &= ~(Hw7|Hw10|Hw13|Hw16|Hw19|Hw22); //off : 0
	*(volatile unsigned long *)addr(0x503018) |= (Hw5|Hw8|Hw11|Hw14|Hw17|Hw20); //iso : 1
	for (nCount = 20; nCount > 0; nCount --);	// Wait
#endif
	*(volatile unsigned long *)addr(0x50005C) &= ~(Hw1|Hw3|Hw5|Hw6|Hw8|Hw9|Hw10); //reset off
	*(volatile unsigned long *)addr(0x503018) |= (Hw0|Hw1|Hw2|Hw3|Hw4|Hw26); //ip power-on
#endif

// Disable MEM_PWDN -------------------------------------------------------------------
#if (0)
	*(volatile unsigned long *)addr(0x50303C) &= ~(Hw0|Hw1|Hw2|Hw3); //SD
	*(volatile unsigned long *)addr(0x50303C) |= (Hw4|Hw5); //EHI
	*(volatile unsigned long *)addr(0x50303C) |= (0x3<<6); //USB
	*(volatile unsigned long *)addr(0x50303C) |= (0x3F<<8); //Overlay Mixer
	*(volatile unsigned long *)addr(0x50303C) |= (Hw15); //NFC
	*(volatile unsigned long *)addr(0x50303C) &= ~(0x3<<16); //IRAM0
	//*(volatile unsigned long *)addr(0x50303C) &= ~(0x3<<18); //IRAM1
	*(volatile unsigned long *)addr(0x50303C) |= (Hw20); //CPU Prefetch Buffer
	*(volatile unsigned long *)addr(0x50303C) |= (Hw21); //IO Prefetch Buffer
	*(volatile unsigned long *)addr(0x50303C) |= (Hw22); //HSIO Prefetch Buffer
#endif

// Exit SDRAM Self-refresh ------------------------------------------------------------
	{
		FuncPtr pFunc = (FuncPtr)(SDRAM_INIT_FUNC_ADDR);
		pFunc();
	}
}

/*===========================================================================
VARIABLE
===========================================================================*/
static CKC ckc_backup; // 0x138 = 312 Bytes
static PMU pmu_backup;
static GPIO gpio_backup;
#ifdef TCC_PM_SLEEP_WFI_USED
static PIC pic_backup;
static unsigned eint_backup;
#endif

/*===========================================================================
FUNCTION
===========================================================================*/
static void sleep_mode(void)
{
	unsigned stack;
	volatile unsigned int nCount;
	FuncPtr  pFunc = (FuncPtr )SLEEP_FUNC_ADDR;
	CKC *ckc = (CKC *)tcc_p2v(HwCLK_BASE);
	PMU *pmu = (PMU *)tcc_p2v(HwPMU_BASE);
#ifdef TCC_PM_SLEEP_WFI_USED
	PIC *pic = (PIC *)tcc_p2v(HwPIC_BASE);
	GPIO *gpio = (GPIO *)tcc_p2v(HwGPIO_BASE);
#endif

	/*--------------------------------------------------------------
	 flush tlb & cache
	--------------------------------------------------------------*/
	local_flush_tlb_all();
	flush_cache_all();

	/*--------------------------------------------------------------
	 save Reg.
	--------------------------------------------------------------*/
	memcpy(&ckc_backup, ckc, sizeof(CKC));
	memcpy(&pmu_backup, pmu, sizeof(PMU));
	memcpy(&gpio_backup, gpio, sizeof(GPIO));
	port_arrange();	//for saving current
#ifdef TCC_PM_SLEEP_WFI_USED
	memcpy(&pic_backup, pic, sizeof(PIC));
	eint_backup = gpio->EINTSEL0;
#endif

	/*--------------------------------------------------------------
	 Set wake-up interrupt..
	--------------------------------------------------------------*/
#ifdef TCC_PM_SLEEP_WFI_USED
#ifdef CONFIG_MACH_TCC9300
	// 1. Power key. GPIO E29 -------------------
	pic->IEN0 = (Hw2|Hw3); //EI11 & RTC
	pic->IEN1 = Hw10;    //RMT
	gpio->EINTSEL0 = 0x34; //EXT Int   <= GPIO_E29
	pic->POL0 &= ~Hw3;     //EXT Int 0 : active high : 0
	pic->MODE0 |= Hw3;     //EXT Int 0 : level trigger : 1
	pic->INTMSK0 |= Hw3;   //EXT Int 0 : INTMSK0
#elif defined(CONFIG_MACH_TCC9300CM)
	// 1. Power key. GPIO A3 -------------------
	pic->IEN0 = (Hw2|Hw3); //EI11 & RTC
	pic->IEN1 = Hw10;    //RMT
	gpio->EINTSEL0 = 0x3; //EXT Int   <= GPIO_A3
	pic->POL0 |= Hw3;     //EXT Int 0 : active low : 1
	pic->MODE0 |= Hw3;     //EXT Int 0 : level trigger : 1
	pic->INTMSK0 |= Hw3;   //EXT Int 0 : INTMSK0
#elif defined(CONFIG_MACH_TCC9300ST)
	pic->IEN0 = Hw2;   //RTC
	pic->IEN1 = Hw10;   //RMT
#endif
#endif

	/*--------------------------------------------------------------
	 flush tlb & cache
	--------------------------------------------------------------*/
	//local_flush_tlb_all();
	//flush_cache_all();


	stack = IO_ARM_ChangeStackSRAM();
	/////////////////////////////////////////////////////////////////
	pFunc();
	/////////////////////////////////////////////////////////////////
	IO_ARM_RestoreStackSRAM(stack);


	/*--------------------------------------------------------------
	 restore CKC
	--------------------------------------------------------------*/
	//memcpy(ckc, &ckc_backup, sizeof(CKC));
	{
		//PLL
		//ckc->PLL0CFG = ckc_backup.PLL0CFG;
		ckc->PLL1CFG = ckc_backup.PLL1CFG;
		ckc->PLL2CFG = ckc_backup.PLL2CFG;
#if defined(CONFIG_MEM_CLK_SYNC_MODE)
		ckc->PLL3CFG = ckc_backup.PLL3CFG; //PLL5 is used as Memory Bus Clock
#endif
		ckc->PLL4CFG = ckc_backup.PLL4CFG;
		ckc->PLL5CFG = ckc_backup.PLL5CFG;

		nCount = 1000; while(nCount--) NopDelay30();

		//Divider
		ckc->CLKDIVC = ckc_backup.CLKDIVC;
		ckc->CLKDIVC1 = ckc_backup.CLKDIVC1;
		ckc->CLKDIVC2 = ckc_backup.CLKDIVC2;
		NopDelay30();

		//ckc->CLK0CTRL = ckc_backup.CLK0CTRL;
		ckc->CLK1CTRL = ckc_backup.CLK1CTRL;
		//ckc->CLK2CTRL = ckc_backup.CLK2CTRL; //Memory Clock can't be adjusted freely.
		ckc->CLK3CTRL = ckc_backup.CLK3CTRL;
		ckc->CLK4CTRL = ckc_backup.CLK4CTRL;
		ckc->CLK5CTRL = ckc_backup.CLK5CTRL;
		ckc->CLK6CTRL = ckc_backup.CLK6CTRL;
		//ckc->CLK7CTRL = ckc_backup.CLK7CTRL;
		ckc->CLK8CTRL = ckc_backup.CLK8CTRL;
		ckc->CLK9CTRL = ckc_backup.CLK9CTRL;
		ckc->CLK10CTRL = ckc_backup.CLK10CTRL;

		#ifdef TCC_BUS_SWRESET_USED
		ckc->SWRESET = ckc_backup.SWRESET;
		#endif

		//Peripheral clock
		memcpy((void*)&(ckc->PCLK_TCX), (void*)&(ckc_backup.PCLK_TCX), 0x138-0x80);
	}

	/*--------------------------------------------------------------
	 restore PMU
	--------------------------------------------------------------*/
	//memcpy(pmu, &pmu_backup, sizeof(PMU));
	{
		//pmu->PWROFF = pmu_backup.PWROFF;

		//ip power
		pmu->PWROFF &= ~((~(pmu_backup.PWROFF & (Hw0|Hw1|Hw2|Hw3|Hw4|Hw26)))&(Hw0|Hw1|Hw2|Hw3|Hw4|Hw26));

		if(pmu_backup.PWROFF&Hw7) //if DDI Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw5); //iso : 0
			pmu->PWROFF |= (Hw6); //pre-off : 1
			pmu->PWROFF |= (Hw7); //off : 1
		}
		if(pmu_backup.PWROFF&Hw10) //if DDI Sub Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw8); //iso : 0
			pmu->PWROFF |= (Hw9); //pre-off : 1
			pmu->PWROFF |= (Hw10); //off : 1
		}
		if(pmu_backup.PWROFF&Hw13) //if Video Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw11); //iso : 0
			pmu->PWROFF |= (Hw12); //pre-off : 1
			pmu->PWROFF |= (Hw13); //off : 1
		}
		if(pmu_backup.PWROFF&Hw16) //if Graphic Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw14); //iso : 0
			pmu->PWROFF |= (Hw15); //pre-off : 1
			pmu->PWROFF |= (Hw16); //off : 1
		}
		if(pmu_backup.PWROFF&Hw19) //if Camera Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw17); //iso : 0
			pmu->PWROFF |= (Hw18); //pre-off : 1
			pmu->PWROFF |= (Hw19); //off : 1
		}
		if(pmu_backup.PWROFF&Hw22) //if High Speed I/O Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw20); //iso : 0
			pmu->PWROFF |= (Hw21); //pre-off : 1
			pmu->PWROFF |= (Hw22); //off : 1
		}
		if(pmu_backup.PWROFF&Hw25) //if Memory Bus has been turn off.
		{
			pmu->PWROFF &= ~(Hw23); //iso : 0
			pmu->PWROFF |= (Hw24); //pre-off : 1
			pmu->PWROFF |= (Hw25); //off : 1
		}
	}

	/*--------------------------------------------------------------
	 restore GPIO
	--------------------------------------------------------------*/
	memcpy(gpio, &gpio_backup, sizeof(GPIO));

#ifdef TCC_PM_SLEEP_WFI_USED
	/*--------------------------------------------------------------
	 restore PIC
	--------------------------------------------------------------*/
	memcpy(pic, &pic_backup, sizeof(PIC));

	/*--------------------------------------------------------------
	 restore External interrupt selection reg 0
	--------------------------------------------------------------*/
	gpio->EINTSEL0 = eint_backup;
#endif

}
/*=========================================================================*/
#endif /* TCC_PM_SLEEP_MODE */



#if defined(TCC_PM_SHUTDOWN_MODE) || defined(TCC_PM_SLEEP_MODE)
/*===========================================================================

                               SDRAM Init

===========================================================================*/

/*===========================================================================
FUNCTION
===========================================================================*/
static void sdram_init(void)
{
	volatile unsigned int i;
#if defined(CONFIG_DRAM_DDR3)
	register unsigned int mr;

	#define DDR3_CLK      200
	//#define nCK (1000000/(DDR3_MAX_SPEED/2))
	#define tCK (1000000/DDR3_CLK)
#elif defined(CONFIG_DRAM_DDR2)// && defined(CONFIG_DDR2_LPCON)
	register unsigned int tmp;

	#define DDR2_CLK      180
	//#define nCK (1000000/(DDR2_MAX_SPEED/2))
	#define tCK (1000000/DDR2_CLK)
#endif


//--------------------------------------------------------------------------
// Change to config mode

#if defined(CONFIG_DRAM_DDR3)
	*(volatile unsigned long *)addr(0x303020 ) = 0x0003010b ;//EMCCFG

	if(DDR3_BANK_NUM == 8)
		while (((*(volatile unsigned long *)addr(0x30c208)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
	else//DDR3_BANK_NUM is 4
		while (((*(volatile unsigned long *)addr(0x30c208)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED
#if (0)
	if(DDR3_LOGICAL_CHIP_NUM == 2){
		if(DDR3_BANK_NUM == 8)
			while (((*(volatile unsigned long *)addr(0x30c20C)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
		else//DDR3_BANK_NUM is 4
			while (((*(volatile unsigned long *)addr(0x30c20C)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED
	}
#endif
#elif defined(CONFIG_DRAM_DDR2)// && defined(CONFIG_DDR2_LPCON)
	*(volatile unsigned long *)addr(0x303020 ) =  0x0005010A;

	if(DDR2_BANK_NUM == 8)
		while (((*(volatile unsigned long *)addr(0x305048)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
	else//DDR2_BANK_NUM == 4
		while (((*(volatile unsigned long *)addr(0x305048)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED

#if (0)
	if(DDR2_LOGICAL_CHIP_NUM == 2){
		if(DDR2_BANK_NUM == 8)
			while (((*(volatile unsigned long *)addr(0x30504C)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
		else//DDR2_BANK_NUM == 4
			while (((*(volatile unsigned long *)addr(0x30504C)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED
	}
#endif
#endif

//--------------------------------------------------------------------------
// Clock setting..

	//clock setting start
	//Set CLKDIVC0,CLKDIVC1. But it is diabled.
	*(volatile unsigned long *)addr(0x500048) = 0x01010101;
#ifndef TCC_PM_SLEEP_WFI_USED
	*(volatile unsigned long *)addr(0x50004C) = 0x01010101;
#endif
	*(volatile unsigned long *)addr(0x500050) = 0x01010101;

	//cpu bus - DirectXIN,
	*(volatile unsigned long *)addr(0x500000) = 0x002ffff4;
	//mem bus - DirectXIN/2
	*(volatile unsigned long *)addr(0x500008) = 0x002f1f14;
	//io bus - DirectXIN/2
	*(volatile unsigned long *)addr(0x500010) = 0x002f1f14;
	//smu bus
	*(volatile unsigned long *)addr(0x50001C) = 0x002f1f14;

#ifdef CONFIG_MEM_CLK_SYNC_MODE
	//MBUSCTRL - set synchronous clock mode!
	*(volatile unsigned long *)addr(0x50002C) = 0xffff0100;

	//PLL0 - 290MHz
	*(volatile unsigned long *)addr(0x500030) = 0x02011203;
	*(volatile unsigned long *)addr(0x500030) = 0x82011203;
	//PLL2 - 468MHz
	*(volatile unsigned long *)addr(0x500038) = 0x4201D403;
	*(volatile unsigned long *)addr(0x500038) = 0xC201D403;

	for (i = 3000; i > 0; i --);	// Wait

	//cpu bus - PLL0, 290MHz
	*(volatile unsigned long *)addr(0x500000) = 0x002FFFF0;
	//mem bus - PLL0, 290/2 = 140MHz
	*(volatile unsigned long *)addr(0x500008) = 0x00200010;
	//io bus - PLL2, 468MHz/3 = 156MHz
	*(volatile unsigned long *)addr(0x500010) = 0x00200022;
	//smu bus
	*(volatile unsigned long *)addr(0x50001C) = 0x00200020;
#else
	//MBUSCTRL - set asynchronous clock mode!
	*(volatile unsigned long *)addr(0x50002C) = 0xffff0101;

	//PLL0 - 600MHz
	*(volatile unsigned long *)addr(0x500030) = 0x01012C03;
	*(volatile unsigned long *)addr(0x500030) = 0x81012C03;
	//PLL3 - 290MHz
	*(volatile unsigned long *)addr(0x50003C) = 0x41009103;
	*(volatile unsigned long *)addr(0x50003C) = 0xC1009103;
	//PLL2 - 468MHz
	*(volatile unsigned long *)addr(0x500038) = 0x4201D403;
	*(volatile unsigned long *)addr(0x500038) = 0xC201D403;

	for (i = 3000; i > 0; i --);	// Wait

	//cpu bus - PLL0, 600/2 = 300MHz
	*(volatile unsigned long *)addr(0x500000) = 0x002aaaa0;
	//mem bus - PLL5, 290/2 = 140MHz
	*(volatile unsigned long *)addr(0x500008) = 0x00200013;
	//io bus - PLL2, 468MHz/3 = 156MHz
	*(volatile unsigned long *)addr(0x500010) = 0x00200022;
#endif

	for (i = 3000; i > 0; i --);	// Wait

#if defined(CONFIG_DRAM_DDR3)
//--------------------------------------------------------------------------
// Controller setting

	*(volatile unsigned long *)addr(0x303024 ) = 0x00000300 ;//PHYCFG
	*(volatile unsigned long *)addr(0x304400 ) = 0x0000000A ;//PHYMODE

	//Bruce, 101029, modify according to soc guide
	//*(volatile unsigned long *)addr(0x30C400 ) = 0x00101708  ;//PhyControl0
	*(volatile unsigned long *)addr(0x30C400 ) = 0x0110140A  ;//PhyControl0

	*(volatile unsigned long *)addr(0x30C404 ) = 0x00000086  ;//PhyControl1
	*(volatile unsigned long *)addr(0x30C408 ) = 0x00000000  ;//PhyControl2
	*(volatile unsigned long *)addr(0x30C40c ) = 0x00000000  ;//PhyControl3
	*(volatile unsigned long *)addr(0x30C410 ) = 0x201c7004  ;//PhyControl4

	//Bruce, 101029, modify according to soc guide
	//*(volatile unsigned long *)addr(0x30C400 ) = 0x0110170B  ;//PhyControl0
	//while (((*(volatile unsigned long *)addr(0x30C418)) & (0x04)) != 4);	// dll locked
	if(DDR3_CLK >= 333)
		*(volatile unsigned long *)addr(0x30C400 ) = 0x0110140B  ;//PhyControl0
	else
		*(volatile unsigned long *)addr(0x30C400 ) = 0x0110140F  ;//PhyControl0
	while (((*(volatile unsigned long *)addr(0x30C418)) & (0x04)) != 4);	// dll locked

	*(volatile unsigned long *)addr(0x30C404 ) = 0x0000008e  ;//PhyControl1
	*(volatile unsigned long *)addr(0x30C404 ) = 0x00000086  ;//PhyControl1

	//Bruce, 101029, modify according to soc guide
	//*(volatile unsigned long *)addr(0x30C414 ) = 0x00030003  ;//PhyControl5
	*(volatile unsigned long *)addr(0x30C414 ) = 0x00020003  ;//PhyControl5

	*(volatile unsigned long *)addr(0x30C414 ) = 0x0003000b  ;//PhyControl5

	while (((*(volatile unsigned long *)addr(0x30c420)) & (0x01)) != 1);	// zq end

//--------------------------------------------------------------------------
// Memory config

	*(volatile unsigned long *)addr(0x30C004 ) = 0x0000018A ; //MemControl

	if(DDR3_BURST_LEN == BL_8) // BL
		BITCSET(*(volatile unsigned long *)addr(0x30C004), 0x7<<7, 0x3<<7);
	else
		BITCSET(*(volatile unsigned long *)addr(0x30C004), 0x7<<7, 0x2<<7);

	if(DDR3_LOGICAL_CHIP_NUM == 1) // num_chip
		BITCSET(*(volatile unsigned long *)addr(0x30C004), 0x3<<5, 0x0);
	else
		BITCSET(*(volatile unsigned long *)addr(0x30C004), 0x3<<5, 0x1<<5);

    // Chip 0 Configuration ------------------------------------------------
    {
		*(volatile unsigned long *)addr(0x30C008) = 0x40F01313; //MemConfig0 //address mapping method - interleaved
		BITCSET(*(volatile unsigned long *)addr(0x30C008), 0xFF<<16, (0xFF - ((DDR3_TOTAL_MB_SIZE)/(DDR3_LOGICAL_CHIP_NUM*0x10)-1))<<16);//set chip mask
		BITCSET(*(volatile unsigned long *)addr(0x30C008), 0xF<<8, (DDR3_COLBITS - 7)<<8);//set column bits
		BITCSET(*(volatile unsigned long *)addr(0x30C008), 0xF<<4, (DDR3_ROWBITS - 12)<<4);//set row bits
		if(DDR3_BANK_NUM == 8)//8 banks
			BITCSET(*(volatile unsigned long *)addr(0x30C008), 0xF, 0x3);
		else // 4 banks
			BITCSET(*(volatile unsigned long *)addr(0x30C008), 0xF, 0x2);
    }

    // Chip 1 Configuration ------------------------------------------------
	if(DDR3_LOGICAL_CHIP_NUM == 2)
	{
		*(volatile unsigned long *)addr(0x30C00C) = 0x50E01313; //MemConfig1 //address mapping method - interleaved
		BITCSET(*(volatile unsigned long *)addr(0x30C00C), 0xFF<<24, (0x40 + (DDR3_TOTAL_MB_SIZE)/(DDR3_LOGICAL_CHIP_NUM*0x10))<<24);//set chip base
		BITCSET(*(volatile unsigned long *)addr(0x30C00C), 0xFF<<16, (0xFF - ((DDR3_TOTAL_MB_SIZE)/(DDR3_LOGICAL_CHIP_NUM*0x10)-1))<<16);//set chip mask
		BITCSET(*(volatile unsigned long *)addr(0x30C00C), 0xF<<8, (DDR3_COLBITS - 7)<<8);//set column bits
		BITCSET(*(volatile unsigned long *)addr(0x30C00C), 0xF<<4, (DDR3_ROWBITS - 12)<<4);//set row bits
		if(DDR3_BANK_NUM == 8)// 8 banks
			BITCSET(*(volatile unsigned long *)addr(0x30C00C), 0xF, 0x3);
		else // 4 banks
			BITCSET(*(volatile unsigned long *)addr(0x30C00C), 0xF, 0x2);
	}

//--------------------------------------------------------------------------

	*(volatile unsigned long *)addr(0x30C000) = 0x40FF3010  ;//ConControl
	*(volatile unsigned long *)addr(0x30C014) = 0x01000000  ;//PrechConfig

//--------------------------------------------------------------------------
// Timing parameter setting.

#if (1)
	*(volatile unsigned long *)addr(0x30C100) = time2cycle(DDR3_tREFI_ps, tCK); //T_REFI
	*(volatile unsigned long *)addr(0x30C104) = time2cycle(DDR3_tRFC_ps, tCK); //T_RFC
	*(volatile unsigned long *)addr(0x30C108) = time2cycle(DDR3_tRRD_ps, tCK); //T_RRD
	if(*(volatile unsigned long *)addr(0x30C108)<DDR3_tRRD_ck)
		*(volatile unsigned long *)addr(0x30C108) = DDR3_tRRD_ck;
	*(volatile unsigned long *)addr(0x30C10c) = DDR3_CL; //T_RP
	*(volatile unsigned long *)addr(0x30C110) = DDR3_CL; //T_RCD
	*(volatile unsigned long *)addr(0x30C114) = time2cycle(DDR3_tRC_ps, tCK); //T_RC
	*(volatile unsigned long *)addr(0x30C118) = time2cycle(DDR3_tRAS_ps, tCK); //T_RAS
	*(volatile unsigned long *)addr(0x30C11c) = time2cycle(DDR3_tWTR_ps, tCK); //T_WTR
	if(*(volatile unsigned long *)addr(0x30C11c)<DDR3_tWTR_ck)
		*(volatile unsigned long *)addr(0x30C11c) = DDR3_tWTR_ck;
	*(volatile unsigned long *)addr(0x30C120) = time2cycle(DDR3_tWR_ps, tCK); //T_WR
	*(volatile unsigned long *)addr(0x30C124) = time2cycle(DDR3_tRTP_ps, tCK); //T_RTP
	if(*(volatile unsigned long *)addr(0x30C124)<DDR3_tRTP_ck)
		*(volatile unsigned long *)addr(0x30C124) = DDR3_tRTP_ck;
	*(volatile unsigned long *)addr(0x30C128) = DDR3_CL; //CL

	if(tCK >= 2500 /* 2.5 ns */)
		*(volatile unsigned long *)addr(0x30C12c) = 5;
	else if(tCK >= 1875 /* 1.875 ns */)
		*(volatile unsigned long *)addr(0x30C12c) = 6;
	else if(tCK >= 1500 /* 1.5 ns */)
		*(volatile unsigned long *)addr(0x30C12c) = 7;
	else if(tCK >= 1250 /* 1.25 ns */)
		*(volatile unsigned long *)addr(0x30C12c) = 8;
	else if(tCK >= 1070 /* 1.07 ns */)
		*(volatile unsigned long *)addr(0x30C12c) = 9;
	else if(tCK >= 935 /* 0.935 ns */)
		*(volatile unsigned long *)addr(0x30C12c) = 10;
	else if(tCK >= 833 /* 0.833 ns */)
		*(volatile unsigned long *)addr(0x30C12c) = 11;
	else if(tCK >= 750 /* 0.75 ns */)
		*(volatile unsigned long *)addr(0x30C12c) = 12;

	*(volatile unsigned long *)addr(0x30C130) = DDR3_CL; //RL = AL+CL

	if(DDR3_AL == AL_CL_MINUS_ONE){ //nAL = nCL - 1;
		*(volatile unsigned long *)addr(0x30C12c) += (DDR3_CL-1); //WL = AL+CWL
		*(volatile unsigned long *)addr(0x30C130) += (DDR3_CL-1); //RL = AL+CL
	}else if(DDR3_AL == AL_CL_MINUS_TWO){ //	nAL = nCL - 2;
		*(volatile unsigned long *)addr(0x30C12c) += (DDR3_CL-2); //WL = AL+CWL
		*(volatile unsigned long *)addr(0x30C130) += (DDR3_CL-2); //RL = AL+CL
	}

	*(volatile unsigned long *)addr(0x30C134) = time2cycle(DDR3_tFAW_ps, tCK); //T_FAW
	*(volatile unsigned long *)addr(0x30C138) = time2cycle(DDR3_tXS_ps, tCK); //T_XSR
	*(volatile unsigned long *)addr(0x30C13c) = time2cycle(DDR3_tXP_ps, tCK); //T_XP
	if(*(volatile unsigned long *)addr(0x30C13c)<DDR3_tXP_ck)
		*(volatile unsigned long *)addr(0x30C13c) = DDR3_tXP_ck;
	*(volatile unsigned long *)addr(0x30C140) = time2cycle(DDR3_tCKE_ps, tCK); //T_CKE
	if(*(volatile unsigned long *)addr(0x30C140)<DDR3_tCKE_ck)
		*(volatile unsigned long *)addr(0x30C140) = DDR3_tCKE_ck;
	*(volatile unsigned long *)addr(0x30C144) = DDR3_tMRD_ck; //T_MRD
#else
	*(volatile unsigned long *)addr(0x30C100) = 0x618;
	*(volatile unsigned long *)addr(0x30C104) = 0x16;
	*(volatile unsigned long *)addr(0x30C108) = 0x4;
	*(volatile unsigned long *)addr(0x30C10c) = 0x6;
	*(volatile unsigned long *)addr(0x30C110) = 0x6;
	*(volatile unsigned long *)addr(0x30C114) = 0xA;
	*(volatile unsigned long *)addr(0x30C118) = 0x8;
	*(volatile unsigned long *)addr(0x30C11c) = 0x4;
	*(volatile unsigned long *)addr(0x30C120) = 0x3;
	*(volatile unsigned long *)addr(0x30C124) = 0x4;
	*(volatile unsigned long *)addr(0x30C128) = 0x6;
	*(volatile unsigned long *)addr(0x30C12c) = 0x5;
	*(volatile unsigned long *)addr(0x30C130) = 0x6;
	*(volatile unsigned long *)addr(0x30C134) = 0x9;
	*(volatile unsigned long *)addr(0x30C138) = 0x18;
	*(volatile unsigned long *)addr(0x30C13c) = 0x3;
	*(volatile unsigned long *)addr(0x30C140) = 0x3;
	*(volatile unsigned long *)addr(0x30C144) = 0x4;
#endif

//--------------------------------------------------------------------------
// MRS Setting

	*(volatile unsigned long *)addr(0x30C010) = 0x08000000 ;//DirectCmd - XSR
	
	//after 500 us
	*(volatile unsigned long *)addr(0x30C010) = 0x07000000;//DirectCmd - NOP
	*(volatile unsigned long *)addr(0x30C010) = 0x00020000;//DirectCmd - MRS : MR2
	*(volatile unsigned long *)addr(0x30C010) = 0x00030000;//DirectCmd - MRS : MR3
	*(volatile unsigned long *)addr(0x30C010) = 0x00010006;//DirectCmd - MRS : MR1 : AL(0),Rtt_Nom(disable),OIC(RZQ/6) ,DLL(enable)

	//*(volatile unsigned long *)addr(0x30C010) = 0x00001220;//DirectCmd - MRS : MR0 : DLLPRE(off), WR(), DLL Reset(Yes), MODE(0), CL(), BL(8)
	{
		mr = DDR3_BURST_LEN | (DDR3_READ_BURST_TYPE<<3) | (FAST_EXIT<<12);

		if(DDR3_CL < 5)
			mr |= ((5-4)<<4);
		else if(DDR3_CL > 11)
			mr |= ((11-4)<<4);
		else
			mr |= ((DDR3_CL-4)<<4);

		if(tCK >= 2500 /* 2.5 ns */)
			mr |= (WR_5<<9);
		else if(tCK >= 1875 /* 1.875 ns */)
			mr |= (WR_6<<9);
		else if(tCK >= 1500 /* 1.5 ns */)
			mr |= (WR_7<<9);
		else if(tCK >= 1250 /* 1.25 ns */)
			mr |= (WR_8<<9);
		else if(tCK >= 935 /* 0.935 ns */)
			mr |= (WR_10<<9);
		else
			mr |= (WR_12<<9);

		*(volatile unsigned long *)addr(0x30C010) = mr;
	}

	*(volatile unsigned long *)addr(0x30C010) = 0x0a000400 ;//DirectCmd - ZQCL

	if(DDR3_LOGICAL_CHIP_NUM == 2){
		*(volatile unsigned long *)addr(0x30C010) = 0x08000000 | Hw20;//DirectCmd - XSR
		
		//after 500 us
		*(volatile unsigned long *)addr(0x30C010) = 0x07000000 | Hw20;//DirectCmd - NOP
		*(volatile unsigned long *)addr(0x30C010) = 0x00020000 | Hw20;//DirectCmd - MRS : MR2
		*(volatile unsigned long *)addr(0x30C010) = 0x00030000 | Hw20;//DirectCmd - MRS : MR3
		*(volatile unsigned long *)addr(0x30C010) = 0x00010006 | Hw20;//DirectCmd - MRS : MR1 : AL(0),Rtt_Nom(disable),OIC(RZQ/6) ,DLL(enable)

		//*(volatile unsigned long *)addr(0x30C010) = 0x00001420 | Hw20;//DirectCmd - MRS : MR0 : DLLPRE(off), WR(), DLL Reset(Yes), MODE(0), CL(), BL(8)
		{
			mr = DDR3_BURST_LEN | (DDR3_READ_BURST_TYPE<<3) | (FAST_EXIT<<12);

			if(DDR3_CL < 5)
				mr |= ((5-4)<<4);
			else if(DDR3_CL > 11)
				mr |= ((11-4)<<4);
			else
				mr |= ((DDR3_CL-4)<<4);

			if(tCK >= 2500 /* 2.5 ns */)
				mr |= (WR_5<<9);
			else if(tCK >= 1875 /* 1.875 ns */)
				mr |= (WR_6<<9);
			else if(tCK >= 1500 /* 1.5 ns */)
				mr |= (WR_7<<9);
			else if(tCK >= 1250 /* 1.25 ns */)
				mr |= (WR_8<<9);
			else if(tCK >= 935 /* 0.935 ns */)
				mr |= (WR_10<<9);
			else
				mr |= (WR_12<<9);

			*(volatile unsigned long *)addr(0x30C010) = mr | Hw20;
		}

		*(volatile unsigned long *)addr(0x30C010) = 0x0a000400 | Hw20;//DirectCmd - ZQCL
	}

//--------------------------------------------------------------------------

	*(volatile unsigned long *)addr(0x30C000) = 0x60ff3030  ;//ConControl
	*(volatile unsigned long *)addr(0x303020) = 0x0007010b ;//EMCCFG

	if(DDR3_BANK_NUM == 8)
		while (((*(volatile unsigned long *)addr(0x30C208)) & 0xFFFFFFFF)!= 0x44444444); //Wait PAUSED
	else //DDR3_BANK_NUM is 4
		while (((*(volatile unsigned long *)addr(0x30C208)) & 0x0000FFFF)!= 0x00004444); //Wait PAUSED

	if(DDR3_LOGICAL_CHIP_NUM == 2){
		if(DDR3_BANK_NUM == 8)
			while (((*(volatile unsigned long *)addr(0x30C20C)) & 0xFFFFFFFF)!= 0x44444444); //Wait PAUSED
		else //DDR3_BANK_NUM is 4
			while (((*(volatile unsigned long *)addr(0x30C20C)) & 0x0000FFFF)!= 0x00004444); //Wait PAUSED
	}

//--------------------------------------------------------------------------
#elif defined(CONFIG_DRAM_DDR2)// && defined(CONFIG_DDR2_LPCON)

//--------------------------------------------------------------------------
// Controller setting

	//phy configuration
	*(volatile unsigned long *)addr(0x303024 ) = 0x200;//PHYCFG

	//PhyZQControl
	if (DDR2_CLK >= 200) {
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL ;
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL | Hw1 ;//zq start
	} else {
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL | Hw0;
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL | Hw1 | Hw0 ;//zq start
	}
	while (((*(volatile unsigned long *)addr(0x305040)) & (0x10000)) != 0x10000);	// Wait until ZQ End

	if (DDR2_CLK >= 200) {
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL ;
	} else {
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL | Hw0;
	}

	*(volatile unsigned long *)addr(0x305018 ) = 0x0010100A; //PHY Control0
	*(volatile unsigned long *)addr(0x30501C ) = 0xE0000086; //PHY Control1 // modify by crony : [31:30] : ODT Enable for Write and Read
	*(volatile unsigned long *)addr(0x305020 ) = 0x00000000; //PHY Control2
	*(volatile unsigned long *)addr(0x305024 ) = 0x00000000; //PHY Control3
	*(volatile unsigned long *)addr(0x305018 ) = 0x0010100B; //PHY Control0

	while (((*(volatile unsigned long *)addr(0x305040)) & (0x7)) != 0x7);// Wait until FLOCK == 1

	//PHY Control1
	*(volatile unsigned long *)addr(0x30501C) = 0xE000008E; //resync = 1
	*(volatile unsigned long *)addr(0x30501C) = 0xE0000086; //resync = 0

//--------------------------------------------------------------------------
// Memory config

	//Enable Out of order scheduling
	*(volatile unsigned long *)addr(0x305000 ) = 0x30FF2018;

	//MEMCTRL
	*(volatile unsigned long *)addr(0x305004 ) = (0x2 << 20) |
	                                             ((DDR2_LOGICAL_CHIP_NUM-1)<<16) |
	                                             ((DDR2_LOGICAL_DATA_BITS/16)<<12) |
	                                             (0x4 << 8) |
	                                             (0x0 << 6) |
	                                             (0x0 << 5) |
	                                             (0x0 << 4) |
	                                             (0x0 << 2) |
	                                             (0x0 << 1) |
	                                             (0x0);

	//MEMCHIP0
	*(volatile unsigned long *)addr(0x305008 ) = (0x40<<24) |
	                                             ((0xFF - (DDR2_TOTAL_MB_SIZE/(DDR2_LOGICAL_CHIP_NUM*0x10)-1))<<16) |
	                                             (0x1 << 12) |
	                                             ((DDR2_COLBITS - 7)<<8) |
	                                             ((DDR2_ROWBITS - 12)<<4) |
	                                             DDR2_BANK_BITS;

	//MEMCHIP1
	if(DDR2_LOGICAL_CHIP_NUM == 2)
	*(volatile unsigned long *)addr(0x30500C ) = ((0x40 + DDR2_TOTAL_MB_SIZE/(DDR2_LOGICAL_CHIP_NUM*0x10))<<24) |
		                                         ((0xFF - (DDR2_TOTAL_MB_SIZE/(DDR2_LOGICAL_CHIP_NUM*0x10)-1))<<16) |
		                                         (0x1 << 12) |
		                                         ((DDR2_COLBITS - 7)<<8) |
		                                         ((DDR2_ROWBITS - 12)<<4) |
		                                         DDR2_BANK_BITS;

//--------------------------------------------------------------------------

	*(volatile unsigned long *)addr(0x305014 ) = 0x0; //PRECONFIG
	*(volatile unsigned long *)addr(0x305028 ) = 0xFFFF00FF; //PRECONFIG

//--------------------------------------------------------------------------
// Timing parameter setting.

	//T_REFI
	*(volatile unsigned long *)addr(0x305030 ) = time2cycle(DDR2_tREFI_ps, tCK);

	//TROW
	*(volatile unsigned long *)addr(0x305034 ) = time2cycle(DDR2_tRAS_ps, tCK); //tRAS
	*(volatile unsigned long *)addr(0x305034 ) |= (time2cycle(DDR2_tRC_ps, tCK)<<6); //tRC
	*(volatile unsigned long *)addr(0x305034 ) |= (DDR2_CL<<12); //tRCD
	*(volatile unsigned long *)addr(0x305034 ) |= (DDR2_CL<<16); //tRP
	*(volatile unsigned long *)addr(0x305034 ) |= (time2cycle(DDR2_tRRD_ps, tCK)<<20); //tRRD
	*(volatile unsigned long *)addr(0x305034 ) |= (time2cycle(DDR2_tRFC_ps, tCK)<<24); //tRFC

	//TDATA
	*(volatile unsigned long *)addr(0x305038 ) = DDR2_CL; //tRL
	*(volatile unsigned long *)addr(0x305038 ) |= ((DDR2_CL-1)<<8); //tWL
	*(volatile unsigned long *)addr(0x305038 ) |= (DDR2_CL<<16); //tCL
	tmp = time2cycle(DDR2_tRTP_ps, tCK);
	if(tmp<DDR2_tRTP_ck) tmp=DDR2_tRTP_ck;
	*(volatile unsigned long *)addr(0x305038 ) |= (tmp<<20); //tRTP
	*(volatile unsigned long *)addr(0x305038 ) |= (time2cycle(DDR2_tWR_ps, tCK)<<24); //tWR
	tmp = time2cycle(DDR2_tWTR_ps, tCK);
	if(tmp<DDR2_tWTR_ck) tmp=DDR2_tWTR_ck;
	*(volatile unsigned long *)addr(0x305038 ) |= (tmp<<28); //tWTR

	//TPOWER
	*(volatile unsigned long *)addr(0x30503C ) = DDR2_tMRD_ck; //tMRD
	*(volatile unsigned long *)addr(0x30503C ) |= (DDR2_tCKE_ck<<4); //tCKE
	*(volatile unsigned long *)addr(0x30503C ) |= (DDR2_tXP_ck<<8); //tXP
	*(volatile unsigned long *)addr(0x30503C ) |= (DDR2_tXSR_ck<<16); //tXSR
	*(volatile unsigned long *)addr(0x30503C ) |= (time2cycle(DDR2_tFAW_ps, tCK)<<24); //tFAW

//--------------------------------------------------------------------------
// MRS Setting

	//Direct Command
	*(volatile unsigned long *)addr(0x305010 ) = 0x07000000;//NOP
	*(volatile unsigned long *)addr(0x305010 ) = 0x01000000;//precharge all
	*(volatile unsigned long *)addr(0x305010 ) = 0x00020000;
	*(volatile unsigned long *)addr(0x305010 ) = 0x00030000;
	*(volatile unsigned long *)addr(0x305010 ) = 0x00010000;
	*(volatile unsigned long *)addr(0x305010 ) = 0x00000100;
	*(volatile unsigned long *)addr(0x305010 ) = 0x01000000;//precharge all
	*(volatile unsigned long *)addr(0x305010 ) = 0x05000000;//AREF
	*(volatile unsigned long *)addr(0x305010 ) = 0x05000000;//AREF
	*(volatile unsigned long *)addr(0x305010 ) = 0x05000000;//AREF
	*(volatile unsigned long *)addr(0x305010 ) = 0x00000000;	// DLL reset release.
	*(volatile unsigned long *)addr(0x305010 ) = (DDR2_BURST_LEN|(DDR2_READ_BURST_TYPE<<3)|(DDR2_CL<<4)|((time2cycle(DDR2_tWR_ps, tCK)-1)<<9));
	i = 100; while(i--);
	*(volatile unsigned long *)addr(0x305010 ) = 0x00010380; // OCD Calibration default
	i = 100; while(i--);
	*(volatile unsigned long *)addr(0x305010 ) = 0x00010004;	// OCD Calibration default

	if(DDR2_LOGICAL_CHIP_NUM == 2)
	{
		*(volatile unsigned long *)addr(0x305010 ) = 0x07000000 | Hw20;//NOP
		*(volatile unsigned long *)addr(0x305010 ) = 0x01000000 | Hw20;//precharge all
		*(volatile unsigned long *)addr(0x305010 ) = 0x00020000 | Hw20;
		*(volatile unsigned long *)addr(0x305010 ) = 0x00030000 | Hw20;
		*(volatile unsigned long *)addr(0x305010 ) = 0x00010000 | Hw20;
		*(volatile unsigned long *)addr(0x305010 ) = 0x00000100 | Hw20;
		*(volatile unsigned long *)addr(0x305010 ) = 0x01000000 | Hw20;//precharge all
		*(volatile unsigned long *)addr(0x305010 ) = 0x05000000 | Hw20;//AREF
		*(volatile unsigned long *)addr(0x305010 ) = 0x05000000 | Hw20;//AREF
		*(volatile unsigned long *)addr(0x305010 ) = 0x05000000 | Hw20;//AREF
		*(volatile unsigned long *)addr(0x305010 ) = 0x00000000 | Hw20;	// DLL reset release.
		*(volatile unsigned long *)addr(0x305010 ) = (DDR2_BURST_LEN|(DDR2_READ_BURST_TYPE<<3)|(DDR2_CL<<4)|((time2cycle(DDR2_tWR_ps, tCK)-1)<<9)) | Hw20;
		i = 100; while(i--);
		*(volatile unsigned long *)addr(0x305010 ) = 0x00010380 | Hw20; // OCD Calibration default
		i = 100; while(i--);
		*(volatile unsigned long *)addr(0x305010 ) = 0x00010004 | Hw20;	// OCD Calibration default
	}

//--------------------------------------------------------------------------

	*(volatile unsigned long *)addr(0x303020 ) =  0x0007010A;//EMCCFG
	*(volatile unsigned long *)addr(0x305000 ) |= 0x20;

	if(DDR2_BANK_NUM == 8)
		while (((*(volatile unsigned long *)addr(0x305048)) & 0xFFFFFFFF)!= 0x44444444); //Wait PAUSED
	else//DDR2_BANK_NUM == 4
		while (((*(volatile unsigned long *)addr(0x305048)) & 0x0000FFFF)!= 0x00004444); //Wait PAUSED

	if(DDR2_LOGICAL_CHIP_NUM == 2){
		if(DDR2_BANK_NUM == 8)
			while (((*(volatile unsigned long *)addr(0x30504C)) & 0xFFFFFFFF)!= 0x44444444); //Wait PAUSED
		else//DDR2_BANK_NUM == 4
			while (((*(volatile unsigned long *)addr(0x30504C)) & 0x0000FFFF)!= 0x00004444); //Wait PAUSED
	}
#endif
}
/*=========================================================================*/
#endif /* defined(TCC_PM_SHUTDOWN_MODE) || defined(TCC_PM_SLEEP_MODE) */



/*===========================================================================

                        Power Management Driver

===========================================================================*/

/*===========================================================================
FUNCTION
	mode 0: sleep mode, mode 1: shut-down mode
===========================================================================*/
static int tcc_pm_enter(suspend_state_t state)
{
	unsigned long flags;
	unsigned reg_backup[20];
	pmdrv_dbg("[%s] Start func\n", __func__);

	if (machine_is_tcc9300st()) 
	{
		reg_backup[5] = *((volatile unsigned long *)0xf010a180);		// GPGDAT
		reg_backup[6] = *((volatile unsigned long *)0xf010a184);		// GPGEN
		reg_backup[7] = *((volatile unsigned long *)0xf010a1b4);		// GPGIEN
	}

// -------------------------------------------------------------------------
// disable interrupt
	local_irq_save(flags);
	local_irq_disable();

#ifdef CONFIG_MACH_TCC9300
	// save power key gpio -------------------------------------------------
	reg_backup[0] = *((volatile unsigned long *)0xf010a004);	//GPAEN
	reg_backup[1] = *((volatile unsigned long *)0xf010a000);	//GPADAT
	reg_backup[2] = *((volatile unsigned long *)0xf010a184);	//GPGEN
	reg_backup[3] = *((volatile unsigned long *)0xf010a180);	//GPGDAT
	reg_backup[4] = *((volatile unsigned long *)0xf010a104);	//GPEEN

	// set power key gpio for wake-up --------------------------------------
	*((volatile unsigned long *)0xf010a004) |= Hw8; //GPIO_A8 : output
	*((volatile unsigned long *)0xf010a000) &= ~Hw8; //GPIO_A8 : low
	*((volatile unsigned long *)0xf010a184) |= Hw22; //GPIO_G22 : output
	*((volatile unsigned long *)0xf010a180) |= Hw22; //GPIO_G22 : high
	*((volatile unsigned long *)0xf010a104) &= ~Hw29; //GPIO_E29 : input

	#ifndef TCC_PM_SLEEP_WFI_USED
	// set wake-up source --------------------------------------------------
	*((volatile unsigned long *)0xf050302C) |= Hw16; // WKUPEN1 (E29)	-> power key
	*((volatile unsigned long *)0xf0503030) &= ~Hw16; // WKUPPOL1 : active high
	*((volatile unsigned long *)0xf0503004) |= 0	// WKUPEN0
						//| HwPMU_WKUP_GPIOA07	// WKUPEN0 : Config Wake-up Event (A7)
						| HwPMU_WKUP_RTCWKUP; 	// WKUPEN0 : RTC Wake-up Event - enable
	*((volatile unsigned long *)0xf0503008) &= 0xFFFFFFFF
						//& ~HwPMU_WKUP_GPIOA07	// WKUPPOL0 : active high
						& ~HwPMU_WKUP_RTCWKUP; 	// WKUPEN0 : RTC Wake-up Event - active high
	#endif
#elif defined(CONFIG_MACH_TCC9300CM)
	// set power key gpio for wake-up --------------------------------------
	*((volatile unsigned long *)0xf010a004) &= ~Hw3; //GPIO_A3 : input

	#ifndef TCC_PM_SLEEP_WFI_USED
	// set wake-up source --------------------------------------------------
	*((volatile unsigned long *)0xf0503004) |= Hw15; // WKUPEN0 (A3)	-> power key
	*((volatile unsigned long *)0xf0503008) |= Hw15; // WKUPPOL0 : active low
	*((volatile unsigned long *)0xf0503004) |= HwPMU_WKUP_RTCWKUP; 	// WKUPEN0 : RTC Wake-up Event - enable
	*((volatile unsigned long *)0xf0503008) &= ~HwPMU_WKUP_RTCWKUP; 	// WKUPPOL0 : RTC Wake-up Event - active high
	#endif
#elif defined(CONFIG_MACH_TCC9300ST)

#ifdef TCC_PM_SHUTDOWN_MODE
	// save power key gpio -------------------------------------------------
	reg_backup[0] = *((volatile unsigned long *)0xf010a104);	//GPEEN

	// set power key gpio for wake-up --------------------------------------
	*((volatile unsigned long *)0xf010a104) &= ~Hw29; //GPIO_E29 : input

	// set wake-up source --------------------------------------------------
	*((volatile unsigned long *)0xf050302C) |= Hw16; // WKUPEN1 (E29)	-> power key
	*((volatile unsigned long *)0xf0503030) |= Hw16; // WKUPPOL1 : active low
	*((volatile unsigned long *)0xf0503004) |= 0	// WKUPEN0
						//| HwPMU_WKUP_GPIOA07	// WKUPEN0 : Config Wake-up Event (A7)
						| HwPMU_WKUP_RTCWKUP; 	// WKUPEN0 : RTC Wake-up Event - enable
	*((volatile unsigned long *)0xf0503008) &= 0xFFFFFFFF
						//& ~HwPMU_WKUP_GPIOA07	// WKUPPOL0 : active high
						& ~HwPMU_WKUP_RTCWKUP; 	// WKUPEN0 : RTC Wake-up Event - active high
#endif

#endif

// -------------------------------------------------------------------------
// Shut-down or Sleep

#ifdef TCC_PM_SHUTDOWN_MODE
	{
		memcpy((void*)BACKUP_RAM_BOOT_ADDR, (void*)BackupRAM_Boot, BACKUP_RAM_BOOT_SIZE); 
		memcpy((void*)IO_RESTORE_FUNC_ADDR, (void*)io_restore, IO_RESTORE_FUNC_SIZE); 
		memcpy((void*)SDRAM_INIT_FUNC_ADDR, (void*)sdram_init, SDRAM_INIT_FUNC_SIZE); 
		memcpy((void*)SHUTDOWN_FUNC_ADDR, (void*)shutdown, SHUTDOWN_FUNC_SIZE); 

		shutdown_mode();
	}
#endif

#ifdef TCC_PM_SLEEP_MODE
	{
		memcpy((void*)SLEEP_FUNC_ADDR, (void*)sleep, SLEEP_FUNC_SIZE); 
		memcpy((void*)SDRAM_INIT_FUNC_ADDR, (void*)sdram_init, SDRAM_INIT_FUNC_SIZE); 
		sleep_mode();
	}
#endif
// -------------------------------------------------------------------------

	// restore registers ---------------------------------------------------
#ifdef CONFIG_MACH_TCC9300
	*((volatile unsigned long *)0xf010a004) = reg_backup[0];
	*((volatile unsigned long *)0xf010a000) = reg_backup[1];
	*((volatile unsigned long *)0xf010a184) = reg_backup[2];
	*((volatile unsigned long *)0xf010a180) = reg_backup[3];
	*((volatile unsigned long *)0xf010a104) = reg_backup[4];
#endif
	if (machine_is_tcc9300st())
	{
		*((volatile unsigned long *)0xf010a180) = reg_backup[5];		// GPGDAT
		*((volatile unsigned long *)0xf010a184) = reg_backup[6];		// GPGEN
		*((volatile unsigned long *)0xf010a1b4) = reg_backup[7];		// GPGIEN
	}

// -------------------------------------------------------------------------
// enable interrupt
	local_irq_restore(flags);

	pmdrv_dbg("[%s] End func\n", __func__);

	return 0;
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void tcc_pm_power_off(void)
{
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

	while(1);
}

/*===========================================================================
VARIABLE
===========================================================================*/
static struct platform_suspend_ops tcc_pm_ops = {
	.valid   = suspend_valid_only_mem,
	.enter   = tcc_pm_enter,
};

/*===========================================================================
VARIABLE
===========================================================================*/
static uint32_t restart_reason = 0xAA;

/*===========================================================================
FUNCTION
===========================================================================*/
static void tcc_pm_restart(char str)
{
	volatile PPMU pPMU = (volatile PPMU)(tcc_p2v(HwPMU_BASE));

	/* store restart_reason to USERSTS register */
	if (restart_reason != 0xAA)
		pPMU->CONFIG1 = pPMU->CONFIG1 | (0x000000FF & restart_reason);

	printk(KERN_DEBUG "reboot: reason=0x%x\n", restart_reason);

	arch_reset(str, NULL);
}

/*===========================================================================
FUNCTION
===========================================================================*/
static int tcc_reboot_call(struct notifier_block *this, unsigned long code, void *cmd)
{
	if(code == SYS_RESTART)
	{
		if (cmd)
		{
			if (!strcmp(cmd, "bootloader"))
			{
				restart_reason = FASTBOOT_MODE;
			}
			else if (!strcmp(cmd, "recovery"))
			{
				restart_reason = RECOVERY_MODE;
			}
			else
			{
				restart_reason = 0x00;
			}
		}
		else
		{
			restart_reason = 0x00;
		}
	}
	return NOTIFY_DONE;
}

/*===========================================================================
VARIABLE
===========================================================================*/
static struct notifier_block tcc_reboot_notifier = {
	.notifier_call = tcc_reboot_call,
};

/*===========================================================================
FUNCTION
===========================================================================*/
static int __init tcc_pm_init(void)
{
	pm_power_off = tcc_pm_power_off;
	arm_pm_restart = tcc_pm_restart;

	register_reboot_notifier(&tcc_reboot_notifier);

	suspend_set_ops(&tcc_pm_ops);
	return 0;
}

__initcall(tcc_pm_init);
