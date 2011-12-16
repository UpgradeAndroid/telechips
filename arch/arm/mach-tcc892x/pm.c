/*
 * arch/arm/mach-tcc892x/pm.c
 *
 * Author:  <linux@telechips.com>
 * Created: November, 2011
 * Description: LINUX POWER MANAGEMENT FUNCTIONS
 *
 * Copyright (C) 2011 Telechips
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
#include <linux/gpio.h>
#include <asm/io.h>
#include <asm/tlbflush.h>
#include <linux/syscalls.h>		// sys_sync()
#include <asm/cacheflush.h>		// local_flush_tlb_all(), flush_cache_all();
#include <mach/globals.h>
#include <mach/system.h>
#include <mach/pm.h>
#include <mach/tcc_ddr.h>
#ifdef CONFIG_CACHE_L2X0
#include <asm/hardware/cache-l2x0.h>
#endif

extern void SRAM_Boot(void);
extern void save_cpu_reg(int sram_addr, unsigned int pReg, void *);
extern void resore_cpu_reg(void);
extern void __cpu_early_init(void);

/*===========================================================================

                  DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/

#define nop_delay(x) for(cnt=0 ; cnt<x ; cnt++){ \
					__asm__ __volatile__ ("nop\n"); }

#define addr_clk(b) (0x74000000+b)
#define addr_mem(b) (0x73000000+b)
#define time2cycle(time, tCK)		((int)((time + tCK - 1) / tCK))

#define denali_ctl(x) (*(volatile unsigned long *)addr_mem(0x500000+(x*4)))
#define denali_phy(x) (*(volatile unsigned long *)addr_mem(0x600000+(x)))
#define ddr_phy(x) (*(volatile unsigned long *)addr_mem(0x420400+(x)))

#define L2CACHE_BASE   0xFA000000

/*===========================================================================

                                 Shut-down

===========================================================================*/

/*===========================================================================
VARIABLE
===========================================================================*/

/*===========================================================================
FUNCTION
===========================================================================*/
static void shutdown(void)
{
	volatile unsigned int cnt = 0;

// -------------------------------------------------------------------------
// mmu & cache off

	__asm__ volatile (
	"mrc p15, 0, r0, c1, c0, 0 \n"
	"bic r0, r0, #(1<<12) \n" //ICache
	"bic r0, r0, #(1<<2) \n" //DCache
	"bic r0, r0, #(1<<0) \n" //MMU
	"mcr p15, 0, r0, c1, c0, 0 \n"
	"nop \n"
	);

// -------------------------------------------------------------------------
// SDRAM Self-refresh

#if defined(CONFIG_DRAM_DDR3)
	#if (0)
		BITCSET(denali_ctl(20), 0xFF000000, ((2<<2)|(1<<1)|(0))<<24);
		BITSET(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x3
	#else
		while(denali_ctl(45)&(1<<8)); //CONTROLLER_BUSY
		BITCSET(denali_ctl(20), 0xFF000000, ((2<<2)|(1<<1)|(0))<<24);
		while(denali_ctl(45)&(1<<8)); //CONTROLLER_BUSY
		while(!(denali_ctl(46)&(0x40)));
		BITSET(denali_ctl(47), 0x40);
		BITSET(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x3
		BITCLR(denali_ctl(0),0x1); //START[0] = 0
		BITSET(denali_phy(0x0C), 0x1<<22); //ctrl_cmosrcv[22] = 0x1
		BITCSET(denali_phy(0x0C), 0x001F0000, 0x1F<<16); //ctrl_pd[20:16] = 0x1f
		BITCSET(denali_phy(0x20), 0x000000FF, 0xF<<4|0xF); //ctrl_pulld_dq[7:4] = 0xf, ctrl_pulld_dqs[3:0] = 0xf
	#endif
#elif defined(CONFIG_DRAM_DDR2)
	//Miscellaneous Configuration : COMMON
	*(volatile unsigned long *)addr_mem(0x410020) &= ~Hw17; //creq2=0 : enter low power
	while((*(volatile unsigned long *)addr_mem(0x410020))&Hw24); //cack2==0 : wait for acknowledgement to request..

	//MEMCTRL
	*(volatile unsigned long *)addr_mem(0x430004) |= 0x00000001; //clk_stop_en=1 //Bruce_temp_8920
#elif defined(CONFIG_DRAM_LPDDR2)
	#error "LPDDR2 is not implemented"
#else
	#error "not selected"
#endif

	nop_delay(5);

// -------------------------------------------------------------------------
// Clock <- XIN, PLL <- OFF

//Bruce_temp..
	((PCKC)HwCKC_BASE)->CLKCTRL0.nREG = 0x002ffff4; //CPU
	((PCKC)HwCKC_BASE)->CLKCTRL1.nREG = 0x00200014; //Memory Bus
	((PCKC)HwCKC_BASE)->CLKCTRL2.nREG = 0x00000014; //DDI Bus
	((PCKC)HwCKC_BASE)->CLKCTRL3.nREG = 0x00000014; //Graphic Bus
	((PCKC)HwCKC_BASE)->CLKCTRL4.nREG = 0x00200014; //I/O Bus
	((PCKC)HwCKC_BASE)->CLKCTRL5.nREG = 0x00000014; //Video Bus
	((PCKC)HwCKC_BASE)->CLKCTRL6.nREG = 0x00000014; //Video core
	((PCKC)HwCKC_BASE)->CLKCTRL7.nREG = 0x00000014; //HSIO BUS
	((PCKC)HwCKC_BASE)->CLKCTRL8.nREG = 0x00200014; //SMU Bus

	((PCKC)HwCKC_BASE)->PLL0CFG.nREG &= ~0x80000000;
	((PCKC)HwCKC_BASE)->PLL1CFG.nREG &= ~0x80000000;
	((PCKC)HwCKC_BASE)->PLL2CFG.nREG &= ~0x80000000;
	((PCKC)HwCKC_BASE)->PLL3CFG.nREG &= ~0x80000000;
	((PCKC)HwCKC_BASE)->PLL4CFG.nREG &= ~0x80000000;
	((PCKC)HwCKC_BASE)->PLL5CFG.nREG &= ~0x80000000;

// -------------------------------------------------------------------------
// ZQ/VDDQ Power OFF
	#if defined(CONFIG_MACH_M805_892X)
	BITCLR(((PGPIO)HwGPIO_BASE)->GPDDAT.nREG, 1<<15); //GPIO D 15
	#else
	BITCLR(((PGPIO)HwGPIO_BASE)->GPCDAT.nREG, 1<<22); //GPIO C 22
	#endif

// -------------------------------------------------------------------------
// SRAM Retention

	BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<16); //PD_RETN : SRAM retention mode=0

// -------------------------------------------------------------------------
// Remap

	BITCSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, 0x30000000, 0x10000000); //remap : 0x00000000 <- sram(0x10000000)
	BITSET(((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSCFG.nREG, 1<<31); //RMIR : remap for top area : 0xF0000000 <- sram(0x10000000)

// -------------------------------------------------------------------------
// BUS Power Down

#if defined(TCC_PM_PMU_CTRL)
	//IP isolation off
	((PPMU)HwPMU_BASE)->PMU_ISOL.nREG = 0xFFFFFFFF;

	//High Speed I/O Bus power down
	((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.HSIOBUS = 0;
	while (((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.HSIOBUS == 1);
	((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.HSB = 0;
	((PPMU)HwPMU_BASE)->PWRDN_HSBUS.bREG.DATA = 1;
	while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PD_HSB == 0);

	//DDI Bus power down
	((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.DBUS = 0;
	while ((((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.DBUS0 | ((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.DBUS1) == 1);
	((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.DB = 0;
	((PPMU)HwPMU_BASE)->PWRDN_DBUS.bREG.DATA = 1;
	while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PD_DB == 0);

	//Graphic Bus power down
	((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.GBUS = 0;
	while (((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.GBUS == 1);
	((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.GB = 0;
	((PPMU)HwPMU_BASE)->PWRDN_GBUS.bREG.DATA = 1;
	while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PD_GB == 0);

	//Video Bus power down
	((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.VBUS = 0;
	while ((((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.VBUS0 | ((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.VBUS1) == 1);
	((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.VB = 0;
	((PPMU)HwPMU_BASE)->PWRDN_VBUS.bREG.DATA = 1;
	while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PD_VB == 0);
#endif

// -------------------------------------------------------------------------
// SSTL & IO Retention

	while(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8)){
		BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode =0
	}
	while(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<4)){
		BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode =0
	}

// -------------------------------------------------------------------------
// set wake-up
//Bruce, wake-up source should be surely set here !!!!

	//set wake-up trigger mode : level
	((PPMU)HwPMU_BASE)->PMU_WKMOD0.nREG = 0xFFFFFFFF;
	((PPMU)HwPMU_BASE)->PMU_WKMOD1.nREG = 0xFFFFFFFF;

	/* Power Key */
#if defined(CONFIG_MACH_M805_892X)
	//set wake-up polarity
	((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.GPIO_D09 = 1; //power key - Active Low
	//set wake-up source
	((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.GPIO_D09 = 1; //power key
#elif defined(CONFIG_MACH_TCC8920ST)
	//set wake-up polarity
	((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.GPIO_D14 = 1; //power key - Active Low
	//set wake-up source
	((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.GPIO_D14 = 1; //power key
#else
	//set wake-up polarity
	((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.GPIO_G16 = 1; //power key - Active Low
	//set wake-up source
	((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.GPIO_G16 = 1; //power key
#endif

	/* RTC Alarm Wake Up */
	//set wake-up polarity
	((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.RTC_WAKEUP = 0; //RTC_PMWKUP - Active High
	//set wake-up source
	((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.RTC_WAKEUP = 1; //RTC_PMWKUP - PMU WakeUp Enable

// -------------------------------------------------------------------------
// Enter Shutdown !!

	while(1){
		((PPMU)HwPMU_BASE)->PWRDN_TOP.nREG = 1;
		nop_delay(100);
	};
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void wakeup(void)
{
	volatile unsigned int cnt;
	volatile unsigned *src;
	volatile unsigned *dest;

// -------------------------------------------------------------------------
// GPIO Restore

	src = (unsigned*)GPIO_REPOSITORY_ADDR;
	dest = (unsigned*)HwGPIO_BASE;

	for(cnt=0 ; cnt<(sizeof(GPIO)/sizeof(unsigned)) ; cnt++)
		dest[cnt] = src[cnt];

// -------------------------------------------------------------------------
// set wake-up
	//disable all for accessing PMU Reg. !!!
	((PPMU)HwPMU_BASE)->PMU_WKUP0.nREG = 0x00000000;
	((PPMU)HwPMU_BASE)->PMU_WKUP1.nREG = 0x00000000;

// -------------------------------------------------------------------------
// SSTL & IO Retention Disable

	while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8))){
		BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode disable=1
	}
	while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<4))){
		BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode disable=1
	}

// -------------------------------------------------------------------------
// ZQ/VDDQ Power ON

	#if defined(CONFIG_MACH_M805_892X)
	BITSET(((PGPIO)HwGPIO_BASE)->GPDDAT.nREG, 1<<15); //GPIO D 15
	#else
	BITSET(((PGPIO)HwGPIO_BASE)->GPCDAT.nREG, 1<<22); //GPIO C 22
	#endif

// -------------------------------------------------------------------------
// BUS Power On

#if defined(TCC_PM_PMU_CTRL)
	//IP isolation on
	((PPMU)HwPMU_BASE)->PMU_ISOL.nREG = 0x00000000;
#endif
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void sdram_init(void)
{
#if defined(CONFIG_DRAM_DDR3)
	volatile int i;
	register unsigned int tmp;
	uint nCL, nCWL;

	#define PLL5_VALUE      0x01012C03	// PLL5 : 600MHz for CPU
#ifdef DDR3_AXI0_FIFO_1to2_ENABLE
//	#define PLL4_VALUE      0x00012C03	// PLL4 : 1200MHz for MEM
//	#define MEM_CLK         600 // = PLL4/2
//	#define PLL4_VALUE      0x00011303	// PLL4 : 1068MHz for MEM
//	#define MEM_CLK         550 // = PLL4/2
//	#define PLL4_VALUE      0x00010B03	// PLL4 : 1068MHz for MEM
//	#define MEM_CLK         533 // = PLL4/2
	#define PLL4_VALUE      0x01012C03	// PLL4 : 600MHz for MEM
	#define MEM_CLK         300 // = PLL4/2
#else
//	#define PLL4_VALUE      0x41016803 // PLL4 : 720MHz for MEM
//	#define MEM_CLK         360// = PLL4/2
	#define PLL4_VALUE      0x01012C03	// PLL4 : 600MHz for MEM
	#define MEM_CLK         300 // = PLL4/2
#endif
	#define DDR3_CLK        MEM_CLK
	#define tCK (1000000/DDR3_CLK)

//--------------------------------------------------------------------------
//for debugging by j-tag
	*(volatile unsigned long *)0x77000018 = 0xf0000000;

//--------------------------------------------------------------------------
//Change to config mode

//--------------------------------------------------------------------------
//Clock setting..

	//Memory BUS Configuration : MBUSCFG
	#ifdef DDR3_AXI0_FIFO_1to2_ENABLE
	*(volatile unsigned long *)addr_mem(0x810010) |= Hw15; //dclkr=1 : this is used for DDR3 only.
	#else
	*(volatile unsigned long *)addr_mem(0x810010) &= ~Hw15; //(d)not used just for ddr3 clk div
	#endif
	i=3200; while(i--);

	*(volatile unsigned long *)addr_clk(0x000000) = 0x002ffff4; //cpu bus : XIN
	*(volatile unsigned long *)addr_clk(0x000004) = 0x00200014; //mem bus : XIN/2 
	*(volatile unsigned long *)addr_clk(0x000010) = 0x00200014; //io bus : XIN/2
	*(volatile unsigned long *)addr_clk(0x000020) = 0x00200014; //smu bus : XIN/2
	*(volatile unsigned long *)addr_clk(0x000044) = PLL5_VALUE; //pll5 for cpu
	*(volatile unsigned long *)addr_clk(0x000044) = 0x80000000 | PLL5_VALUE;
	*(volatile unsigned long *)addr_clk(0x000040) = PLL4_VALUE; //pll4 for mem
	*(volatile unsigned long *)addr_clk(0x000040) = 0x80000000 | PLL4_VALUE;
	i=3200; while(i--);
	*(volatile unsigned long *)addr_clk(0x000000) = 0x002FFFF9;  // cpu
	*(volatile unsigned long *)addr_clk(0x000004) = 0x00200018;  // mem bus
	*(volatile unsigned long *)addr_clk(0x000010) = 0x00200039;  // io bus
	*(volatile unsigned long *)addr_clk(0x000020) = 0x00200039;  // smu bus
	i=3200; while(i--);

//--------------------------------------------------------------------------
// Timing Parameter

	if(DDR3_CLK < 400)
		nCL = DDR3_CL*400*2/DDR3_MAX_SPEED;
	else
		nCL = DDR3_CL*DDR3_CLK*2/DDR3_MAX_SPEED;
	if(DDR3_CL*DDR3_CLK%DDR3_MAX_SPEED != 0)
		nCL++;

	if(tCK >= 2500 /* 2.5 ns */)
		nCWL = 5;
	else if(tCK >= 1875 /* 1.875 ns */)
		nCWL = 6;
	else if(tCK >= 1500 /* 1.5 ns */)
		nCWL = 7;
	else if(tCK >= 1250 /* 1.25 ns */)
		nCWL = 8;
	else if(tCK >= 1070 /* 1.07 ns */)
		nCWL = 9;
	else if(tCK >= 935 /* 0.935 ns */)
		nCWL = 10;
	else if(tCK >= 833 /* 0.833 ns */)
		nCWL = 11;
	else if(tCK >= 750 /* 0.75 ns */)
		nCWL = 12;

	denali_ctl(0) = 0x20410600; //DRAM_CLASS[11:8] = 6:DDR3, 4:DDR2
	denali_ctl(2) = 0x00000004; //TINIT[23:0] = 0x4
	denali_ctl(3) = time2cycle(20000,tCK); //TRST_PWRON = 200us, 7 //Bruce_temp.. ns ?
	denali_ctl(4) = time2cycle(50000,tCK); //CKE_INACTIVE = 500us, 10 //Bruce_temp.. ns ?

	//TBST_INT_INTERVAL[26:24] = 0x1, WL[20:16], CASLAT_LIN[13:8], CL half-cycle increment = 0, INITAREF[3:0] = 0x8
	if(DDR3_AL == AL_DISABLED){ //nAL = 0;
		denali_ctl(5) = (0x1<<24 | (nCWL)<<16 | ((nCL<<1)|0)<<8 | 0x8);
	}
	else if(DDR3_AL == AL_CL_MINUS_ONE){ //nAL = nCL - 1;
		denali_ctl(5) = (0x1<<24 | (nCWL+nCL-1)<<16 | ((nCL<<1)|0)<<8 | 0x8);
	}	
	else if(DDR3_AL == AL_CL_MINUS_TWO){ //nAL = nCL - 2;
		denali_ctl(5) = (0x1<<24 | (nCWL+nCL-2)<<16 | ((nCL<<1)|0)<<8 | 0x8);
	}

	denali_ctl(6) = (time2cycle(DDR3_tRAS_ps,tCK)<<24 | time2cycle(DDR3_tRC_ps,tCK)<<16 | time2cycle(DDR3_tRRD_ps,tCK)<<8 | DDR3_tCCD_ck);
	denali_ctl(7) = (DDR3_tMRD_ck<<24 | time2cycle(DDR3_tRTP_ps,tCK)<<16 | /*time2cycle(DDR3_tRP_ps,tCK)*/nCL<<8 | (time2cycle(DDR3_tWTR_ps,tCK)+1));
	denali_ctl(8) = (time2cycle(DDR3_tRAS_MAX_ps,tCK)<<8 | time2cycle(DDR3_tMOD_ps,tCK));
	denali_ctl(9) = ((time2cycle(DDR3_tCKE_ps,tCK)+1)<<8 | time2cycle(DDR3_tCKE_ps,tCK));
	denali_ctl(10) = (time2cycle(DDR3_tWR_ps,tCK)<<24 | /*time2cycle(DDR3_tRCD_ps,tCK)*/nCL<<16 | 1<<8 | 1);
	denali_ctl(11) = (1<<24 | DDR3_tDLLK_ck<<8 | (time2cycle(DDR3_tWR_ps,tCK)+nCL));
	denali_ctl(12) = (1<<16 | time2cycle(DDR3_tFAW_ps,tCK)<<8 | 3);
	denali_ctl(13) = /*time2cycle(DDR3_tRP_ps,tCK)*/nCL;
	BITCSET(denali_ctl(14), 0x03FF0100, (time2cycle(DDR3_tRFC_ps,tCK)<<16 | 1<<8));
	denali_ctl(15) = time2cycle(DDR3_tREFI_ps,tCK);
	denali_ctl(16) = (time2cycle(DDR3_tXPDLL_ps,tCK)<<16 | time2cycle(DDR3_tXP_ps,tCK)); // DDR3 Only
	denali_ctl(17) = (6<<16 | 2); //TXARD[-0] = 0x2, TXARDS[-16] = 0x6 // DDR2 only
	denali_ctl(18) = (time2cycle(DDR3_tXS_ps,tCK)<<16 | DDR3_tXSDLL_ck);
	denali_ctl(19) |= (1<<16); //ENABLE_QUICK_SREFRESH = 0x1
	BITCSET(denali_ctl(20), 0x000F0F00, time2cycle(DDR3_tCKSRX_ps,tCK)<<16 | time2cycle(DDR3_tCKSRE_ps,tCK)<<8);

//--------------------------------------------------------------------------
// MRS Setting

	// MRS0
#if (1) //Bruce_temp_mrs0 FAST_EXIT 맞나?
	tmp = DDR3_BURST_LEN | (DDR3_READ_BURST_TYPE<<3);
#else
	tmp = DDR3_BURST_LEN | (DDR3_READ_BURST_TYPE<<3) | (FAST_EXIT<<12);
#endif
	if(nCL < 5)
		tmp |= ((5-4)<<4);
	else if(nCL > 11)
		tmp |= ((11-4)<<4);
	else
		tmp |= ((nCL-4)<<4);

	if(time2cycle(DDR3_tWR_ps,tCK) <= 5) // Write Recovery for autoprecharge
		tmp |= WR_5<<9;
	else if(time2cycle(DDR3_tWR_ps,tCK) == 6)
		tmp |= WR_6<<9;
	else if(time2cycle(DDR3_tWR_ps,tCK) == 7)
		tmp |= WR_7<<9;
	else if(time2cycle(DDR3_tWR_ps,tCK) == 8)
		tmp |= WR_8<<9;
	else if((time2cycle(DDR3_tWR_ps,tCK) == 9) || (time2cycle(DDR3_tWR_ps,tCK) == 10))
		tmp |= WR_10<<9;
	else if(time2cycle(DDR3_tWR_ps,tCK) >= 11)
		tmp |= WR_12<<9;

	tmp	|= (1<<8); // DLL Reset

	denali_ctl(27) = tmp<<8;
	BITCSET(denali_ctl(30), 0x0000FFFF, tmp);

	// MRS1
	BITCSET(denali_ctl(28), 0x0000FFFF, (DDR3_AL<<3) | (DDR3_ODT<<2) | (DDR3_DIC<<1));
	BITCSET(denali_ctl(30), 0xFFFF0000, ((DDR3_AL<<3) | (DDR3_ODT<<2) | (DDR3_DIC<<1))<<16);

	// MRS2
	tmp = 0;
	if(nCWL <= 5)
		tmp |= 0;
	else if(nCWL == 6)
		tmp |= 1<<3;
	else if(nCWL == 7)
		tmp |= 2<<3;
	else if(nCWL >= 8)
		tmp |= 3<<3;

	BITCSET(denali_ctl(28), 0xFFFF0000, tmp<<16);
	BITCSET(denali_ctl(31), 0x0000FFFF, tmp);

	// MRS3
	BITCSET(denali_ctl(29), 0xFFFF0000, 0<<16);
	BITCSET(denali_ctl(32), 0x0000FFFF, 0);

//--------------------------------------------------------------------------

	//BIST Start
	BITCLR(denali_ctl(32), 1<<16); //BIST_GO = 0x0
	denali_ctl(33) = (1<<16)|(1<<8); //BIST_ADDR_CHECK[-16] = 0x1, BIST_DATA_CHECK[-8] = 0x1
	denali_ctl(34) = 0; denali_ctl(35) = 0; denali_ctl(36) = 0; denali_ctl(37) = 0;
	//BIST End

	denali_ctl(38) = (DDR3_tZQOPER_ck<<16 | DDR3_tZQINIT_ck);
	denali_ctl(39) = (0x2<<24 | DDR3_tZQCS_ck); //ZQCS, ZQ_ON_SREF_EXIT
	denali_ctl(40) = (0x1<<16 | 0x40); //ZQCS_ROTATE=0x1, REFRESH_PER_ZQ=0x40

	// DRAM Size Setting
	if(DDR3_BANK_NUM == 8) //EIGHT_BANK_MODE
		BITSET(denali_ctl(40), 0x1<<24);
	else
		BITCLR(denali_ctl(40), 0x1<<24);
	denali_ctl(41) = 0xFF<<24|DDR3_APBIT<<16|(11-DDR3_COLBITS)<<8|(16-DDR3_ROWBITS); //ADDR_PINS, COLUMN_SIZE, APREBIT, AGE_COUNT = 0xFF
	denali_ctl(42) = 0x1<<24|0x1<<16|0x1<<8|0xFF; //COMMAND_AGE_CCOUNT = 0xff, ADDR_CMP_EN = 0x1, BANK_SPLIT_EN = 0x1 PLACEMENT_EN = 0x1
	denali_ctl(43) = 0x1<<24|0x1<<16|0x1<<8|0x1; //PRIORITY_EN = 0x1, RW_SAME_EN = 0x1,SWAP_EN = 0x1, SWAP_PORT_RW_SAME_EN = 0x1
#if defined(CONFIG_DRAM_16BIT_USED)
	if(DDR3_LOGICAL_CHIP_NUM == 2)
		denali_ctl(44) = (0x1<<24|0xc<<16|0x3<<8); //REDUC[24] = DQ 0:32BIT, 1:16BIT
	else
		denali_ctl(44) = (0x1<<24|0xc<<16|0x1<<8); //REDUC[24] = DQ 0:32BIT, 1:16BIT
#else
	if(DDR3_LOGICAL_CHIP_NUM == 2)
		denali_ctl(44) = (0x0<<24|0xc<<16|0x3<<8); //REDUC[24] = DQ 0:32BIT, 1:16BIT
	else
		denali_ctl(44) = (0x0<<24|0xc<<16|0x1<<8); //REDUC[24] = DQ 0:32BIT, 1:16BIT
#endif
	denali_ctl(45) = 0x1<<24; //RESYNC_DLL_PER_AREF_EN = 0x1 // Automatic Controller-Initiated Update

	denali_ctl(47) = 0x00020000;

//--------------------------------------------------------------------------
// ODT Setting

	BITCSET(denali_ctl(64), 0x00030000, 0x0<<16); //ODT_RD_MAP_CS0
	if(DDR3_LOGICAL_CHIP_NUM == 2)
		BITCSET(denali_ctl(65), 0x00000003, 0x0<<0); //ODT_RD_MAP_CS1

	if(DDR3_PINMAP_TYPE == 0 || DDR3_PINMAP_TYPE == 1){
		BITCSET(denali_ctl(64), 0x03000000, 0x1<<24); //ODT_WR_MAP_CS0
		if(DDR3_LOGICAL_CHIP_NUM == 2)
			BITCSET(denali_ctl(65), 0x00000300, 0x1<<8); //ODT_WR_MAP_CS1
	} else {
		BITCSET(denali_ctl(64), 0x03000000, 0x3<<24); //ODT_WR_MAP_CS0
		if(DDR3_LOGICAL_CHIP_NUM == 2)
			BITCSET(denali_ctl(65), 0x00000300, 0x3<<8); //ODT_WR_MAP_CS1
	}
	BITCSET(denali_ctl(65), 0x000F0000, 0x2<<16); //ADD_ODT_CLK_R2W_SAMECS = 0x2
	denali_ctl(66) = 0x1<<24|0x1<<16|0x2<<8|0x2; //ADD_ODT_CLK_DIFFTYPE_DIFFCS = 0x2, ADD_ODT_CLK_SAMETYPE_DIFFCS = 0x2, R2R_DIFFCS_DLY = 0x1, R2W_DIFFCS_DLY = 0x1
	denali_ctl(67) = 0x2<<24|0x0<<16|0x1<<8|0x1; //W2R_DIFFCS_DLY = 0x1, W2W_DIFFCS_DLY = 0x1, R2R_SAMECS_DLY = 0x0, R2W_SAMECS_DLY = 0x2
	BITCSET(denali_ctl(68), 0x00000707, 0x0<<8|0x0); //W2R_SAMECS_DLY = 0x0, W2W_SAMECS_DLY = 0x0
	denali_ctl(72) = 0x0<<16|0x28<<8|0x19; //WLDQSEN = 0x19, WLMRD = 0x28, WRLVL_EN = 0x0
	denali_ctl(81) = 0x21<<16|0x21; //RDLVL_DELAY_0 = 0x21, RDLVL_GATE_DELAY_0 = 0x21
	denali_ctl(84) = 0x2121<<8; //RDLVL_DELAY_1 = 0x2121
	denali_ctl(88) = 0x2121; //RDLVL_DELAY_2 = 0x2121
	denali_ctl(91) = 0x2121<<8; //RDLVL_DELAY_3 = 0x2121
	denali_ctl(92) = 0xFFFF<<16; //AXI0_EN_SIZE_LT_WIDTH_INSTR = 0xffff
	denali_ctl(93) = 0x8<<8|0x8; //AXI0_R_PRIORITY = 0x8. AXI0_W_PRIORITY = 0x8
	#ifdef DDR3_AXI0_FIFO_1to2_ENABLE // AXI : MEM Rating Setting, 0 = Async, 1 = 2:1 ( Port : Core ), 2 = 1:2 ( Port : Core ), 3 = Sync.
		BITCSET(denali_ctl(93), 0x00030000, 0x3<<16); //AXI0_FIFO_TYPE_REG[17:16] = 0x3
	#else
		BITCSET(denali_ctl(93), 0x00030000, 0x1<<16); //AXI0_FIFO_TYPE_REG[17:16] = 0x1
	#endif

//--------------------------------------------------------------------------
// DFI Timing

	// tPHY_RDLAT     : DFI_RDDATA_EN <=> DFI_RDDATA_VALID 
	// tPHY_WRLAT(RO) : Write Command <=> DFI_WRDATA_EN    : WRLAT_ADJ(min. 1) + REG_DIMM_ENABLE - 1
	// tRDDATA_EN     : Read Command  <=> DFI_RDDATA_EN    : RDLAT_ADJ(min. 2) + REG_DIMM_ENABLE - 1
	// tPHY_WRDATA    : DFI_WRDATA    <=> DFI_WRDATA_EN
	BITCSET(denali_ctl(95), 0x3F000000, 0x2<<24); //TDFI_PHY_RDLAT = 0x2
	BITCLR(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x0
	denali_ctl(97) = 0x200<<16|0x50E; //TDFI_CTRLUPD_MAX = 0x50e, TDFI_PHYUPD_TYPE0 = 0x200
	denali_ctl(98) = 0x200<<16|0x200; //TDFI_PHYUPD_TYPE1 = 0x200, TDFI_PHYUPD_TYPE2 = 0x200
	denali_ctl(99) = 0x50E<<16|0x200; //TDFI_PHYUPD_TYPE3 = 0x200, TDFI_PHYUPD_RESP = 0x50e
	denali_ctl(100) = 0x1946; //TDFI_CTRLUPD_INTERVAL = 0x1946
	if(DDR3_AL == AL_DISABLED){ //nAL = 0;
		denali_ctl(101) = (0x1<<24|0x1<<16|nCWL<<8|(nCL+4));
	}
	else if(DDR3_AL == AL_CL_MINUS_ONE){ //nAL = nCL - 1;
		denali_ctl(101) = (0x1<<24|0x1<<16|(nCWL+nCL-1)<<8|(nCL+4));
	}	
	else if(DDR3_AL == AL_CL_MINUS_TWO){ //nAL = nCL - 2;
		denali_ctl(101) = (0x1<<24|0x1<<16|(nCWL+nCL-2)<<8|(nCL+4));
	}
	denali_ctl(102) = 0x3<<24|0x8000<<8|0x1; //TDFI_DRAM_CLK_ENABLE = 0x1, DFI_WRLVL_MAX_DELAY = 0x8000, TDFI_WRLVL_EN = 0x3
	denali_ctl(103) = 0x4<<16|0x7<<8|0x3; //TDFI_WRLVL_DLL = 0x3, TDFI_WRLVL_LOAD = 0x7, TDFI_WRLVL_RESPLAT = 0x4
	denali_ctl(104) = 0xA; //TDFI_WRLVL_WW = 0xa
	denali_ctl(107) = 0x10<<16|0xFFFF; //RDLVL_MAX_DELAY = 0xffff, RDLVL_GATE_MAX_DELAY = 0x10
	denali_ctl(108) = 0x12<<24|0x7<<16|0x3<<8|0x3; //TDFI_RDLVL_EN = 0x3, TDFI_RDLVL_DLL = 0x3, TDFI_RDLVL_LOAD = 0x7, TDFI_RDLVL_RESPLAT = 0x12 
	denali_ctl(109) = 0xF; //TDFI_RDLVL_RR = 0xf
	denali_ctl(115) = 0x2<<8|0x4; //RDLVL_DQ_ZERO_COUNT = 0x4, RDLVL_GATE_DQ_ZERO_COUNT = 0x2
	denali_ctl(118) = 1; //ODT_ALT_EN = 0x1

//--------------------------------------------------------------------------
// Denali phy setting

	denali_phy(0x10) = 0x10680000; //Bruce_temp, 현재 ZQ Cal 동작 안되서 강제로 focing한 값을 사용함
	denali_phy(0x0C) = 0x01801010;
	denali_phy(0x1C) = 0x00300000|(MEMCTRL_TERM<<8)|(MEMCTRL_DDS<<4); //Bruce_temp, zq forcing valu (일단 이값을 사용해야함)

//--------------------------------------------------------------------------
// DDR phy setting

	//PHY Mode contrl register (PHYMDCFG0)
	ddr_phy(0x0000) = (0 << 21) |    //ctrl_lat                  //Bruce_temp_8920 ??
	                  (0x1f << 16) | //ctrl_wake_up
	                  (DDR3_PINMAP_TYPE << 14) |    //pin_map
	                  ((DDR3_LOGICAL_CHIP_NUM-1) << 13) | //cs2s
	                  (0 << 4) |     //mode                        //Bruce_temp_8920 ??
	                  (0 << 3) |     //c_type : denali=0, lpcon=1
	                  (1);           //dram_type : ddr2=0, ddr3=1, lpddr=2, lpddr2=3

//--------------------------------------------------------------------------
// Start denali MC init
	BITSET(denali_ctl(0), 1); //START = 1
	if(denali_ctl(46)&(1<<5))
	{
		BITSET(denali_ctl(47),(1<<5));
		while(denali_ctl(46)&(1<<5));
	}
	BITCLR(denali_phy(0x08),1<<1); //ctrl_dll_on_rst = 0
	for (i=0; i<5; i++);
	BITSET(denali_phy(0x08),1<<1); //ctrl_dll_on_rst = 1
	while(!(denali_phy(0x28)&(1<<12))); //ctrl_zq_end, ZQ End ...
	while(!(denali_phy(0x24)&(1<<10))); //ctrl_locked, DLL Locked ...
	while(!(denali_ctl(46)&0x20));

	//denali_ctl(47) |= 0x20;
	denali_phy(0x1C) = 0x00316001|(MEMCTRL_TERM<<8)|(MEMCTRL_DDS<<4);

//--------------------------------------------------------------------------

#elif defined(CONFIG_DRAM_DDR2)

	volatile int i;
	register unsigned int tmp;

	#define PLL5_VALUE      0x01012C03	// PLL5 : 600MHz for CPU
	//#define PLL4_VALUE      0x4201A403 //for 210 //0x01012C03	// PLL4 : 600MHz for MEM
	//#define MEM_CLK         210//300 // = PLL4/2
	#define PLL4_VALUE      0x01012C03	// PLL4 : 600MHz for MEM
	#define MEM_CLK         300 // = PLL4/2

	#define DDR2_CLK        MEM_CLK
	#define tCK (1000000/DDR2_CLK)

//--------------------------------------------------------------------------
//for debugging by j-tag
	*(volatile unsigned long *)0x77000018 = 0xf0000000;

//--------------------------------------------------------------------------
//Change to config mode

	//Miscellaneous Configuration : COMMON
	*(volatile unsigned long *)addr_mem(0x410020) &= ~Hw17; //creq2=0 : enter low power
	while((*(volatile unsigned long *)addr_mem(0x410020))&Hw24); //cack2==0 : wait for acknowledgement to request..

#if (0)
	//MEMCTRL
	//*(volatile unsigned long *)addr_mem(0x430004) |= 0x00000001; //clk_stop_en=1

	if(DDR2_BANK_NUM == 8)
		while (((*(volatile unsigned long *)addr_mem(0x430048)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
	else//DDR2_BANK_NUM == 4
		while (((*(volatile unsigned long *)addr_mem(0x430048)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED

	if(DDR2_LOGICAL_CHIP_NUM == 2){
		if(DDR2_BANK_NUM == 8)
			while (((*(volatile unsigned long *)addr_mem(0x43004C)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
		else//DDR2_BANK_NUM == 4
			while (((*(volatile unsigned long *)addr_mem(0x43004C)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED
	}
#endif

//--------------------------------------------------------------------------
//Clock setting..

	//Memory BUS Configuration : MBUSCFG
	*(volatile unsigned long *)addr_mem(0x810010) &= ~Hw15; //dclkr=0 : this is used for DDR3 only.
	i=3200; while(i--);

	*(volatile unsigned long *)addr_clk(0x000000) = 0x002ffff4; //cpu bus : XIN
	*(volatile unsigned long *)addr_clk(0x000004) = 0x00200014; //mem bus : XIN/2 
	*(volatile unsigned long *)addr_clk(0x000010) = 0x00200014; //io bus : XIN/2
	*(volatile unsigned long *)addr_clk(0x000020) = 0x00200014; //smu bus : XIN/2
	*(volatile unsigned long *)addr_clk(0x000044) = PLL5_VALUE; //pll5 for cpu
	*(volatile unsigned long *)addr_clk(0x000044) = 0x80000000 | PLL5_VALUE;
	*(volatile unsigned long *)addr_clk(0x000040) = PLL4_VALUE; //pll4 for mem
	*(volatile unsigned long *)addr_clk(0x000040) = 0x80000000 | PLL4_VALUE;
	i=3200; while(i--);
	*(volatile unsigned long *)addr_clk(0x000000) = 0x002FFFF9;  // cpu
	*(volatile unsigned long *)addr_clk(0x000004) = 0x00200018;  // mem bus
	*(volatile unsigned long *)addr_clk(0x000010) = 0x00200039;  // io bus
	*(volatile unsigned long *)addr_clk(0x000020) = 0x00200039;  // smu bus
	i=3200; while(i--);


//--------------------------------------------------------------------------
// Controller setting

	//PHY Mode contrl register (PHYMDCFG0)
	*(volatile unsigned long *)addr_mem(0x420400) = (0 << 21) | //ctrl_lat                  //Bruce_temp_8920 ??
	                                             (0x1f << 16) | //ctrl_wake_up
	                                             (DDR2_PINMAP_TYPE << 14) | //pin_map
	                                             ((DDR2_LOGICAL_CHIP_NUM-1) << 13) | //cs2s
	                                             (0 << 4) | //mode                        //Bruce_temp_8920 ??
	                                             (1 << 3) | //c_type : denali=0, lpcon=1
	                                             (0); //dram_type : ddr2=0, ddr3=1, lpddr=2, lpddr2=3



	//PhyZQControl
	if (DDR2_CLK >= DDR2_ODT_ENABLE_MIN_FREQ) {
		*(volatile unsigned long *)addr_mem(0x430044 ) = PHYZQCTRL ;
		*(volatile unsigned long *)addr_mem(0x430044 ) = PHYZQCTRL | Hw1 ;//zq start
	} else {
		*(volatile unsigned long *)addr_mem(0x430044 ) = PHYZQCTRL | Hw0;
		*(volatile unsigned long *)addr_mem(0x430044 ) = PHYZQCTRL | Hw1 | Hw0 ;//zq start
	}
	while (((*(volatile unsigned long *)addr_mem(0x430040)) & (0x10000)) != 0x10000);	// Wait until ZQ End

	if (DDR2_CLK >= DDR2_ODT_ENABLE_MIN_FREQ) {
		*(volatile unsigned long *)addr_mem(0x430044 ) = PHYZQCTRL ;
	} else {
		*(volatile unsigned long *)addr_mem(0x430044 ) = PHYZQCTRL | Hw0;
	}

	*(volatile unsigned long *)addr_mem(0x430018 ) = 0x0010100A; //PHY Control0
	*(volatile unsigned long *)addr_mem(0x43001C ) = 0xE0000086; //PHY Control1 // modify by crony : [31:30] : ODT Enable for Write and Read
	*(volatile unsigned long *)addr_mem(0x430020 ) = 0x00000000; //PHY Control2
	*(volatile unsigned long *)addr_mem(0x430024 ) = 0x00000000; //PHY Control3
	*(volatile unsigned long *)addr_mem(0x430018 ) = 0x0010100B; //PHY Control0

	while (((*(volatile unsigned long *)addr_mem(0x430040)) & (0x7)) != 0x7);// Wait until FLOCK == 1

	//PHY Control1
	*(volatile unsigned long *)addr_mem(0x43001C) = 0xE000008E; //resync = 1
	*(volatile unsigned long *)addr_mem(0x43001C) = 0xE0000086; //resync = 0

//--------------------------------------------------------------------------
// Memory config

	//Enable Out of order scheduling
	*(volatile unsigned long *)addr_mem(0x430000 ) = 0x30FF2018;

	//MEMCTRL
	*(volatile unsigned long *)addr_mem(0x430004 ) = (0x2 << 20) |
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
	*(volatile unsigned long *)addr_mem(0x430008 ) = (0x80<<24) |
	                                             ((0xFF - (DDR2_TOTAL_MB_SIZE/(DDR2_LOGICAL_CHIP_NUM*0x10)-1))<<16) |
	                                             (0x1 << 12) |
	                                             ((DDR2_COLBITS - 7)<<8) |
	                                             ((DDR2_ROWBITS - 12)<<4) |
	                                             DDR2_BANK_BITS;

	//MEMCHIP1
	if(DDR2_LOGICAL_CHIP_NUM == 2)
	*(volatile unsigned long *)addr_mem(0x43000C ) = ((0x80 + DDR2_TOTAL_MB_SIZE/(DDR2_LOGICAL_CHIP_NUM*0x10))<<24) |
		                                         ((0xFF - (DDR2_TOTAL_MB_SIZE/(DDR2_LOGICAL_CHIP_NUM*0x10)-1))<<16) |
		                                         (0x1 << 12) |
		                                         ((DDR2_COLBITS - 7)<<8) |
		                                         ((DDR2_ROWBITS - 12)<<4) |
		                                         DDR2_BANK_BITS;

//--------------------------------------------------------------------------

	*(volatile unsigned long *)addr_mem(0x430014 ) = 0x0; //PRECONFIG
	*(volatile unsigned long *)addr_mem(0x430028 ) = 0xFFFF00FF; //PRECONFIG

//--------------------------------------------------------------------------
// Timing parameter setting.

	//T_REFI
	*(volatile unsigned long *)addr_mem(0x430030 ) = time2cycle(DDR2_tREFI_ps, tCK);

	//TROW
	*(volatile unsigned long *)addr_mem(0x430034 ) = time2cycle(DDR2_tRAS_ps, tCK); //tRAS
	*(volatile unsigned long *)addr_mem(0x430034 ) |= (time2cycle(DDR2_tRC_ps, tCK)<<6); //tRC
	*(volatile unsigned long *)addr_mem(0x430034 ) |= (DDR2_CL<<12); //tRCD
	*(volatile unsigned long *)addr_mem(0x430034 ) |= (DDR2_CL<<16); //tRP
	*(volatile unsigned long *)addr_mem(0x430034 ) |= (time2cycle(DDR2_tRRD_ps, tCK)<<20); //tRRD
	*(volatile unsigned long *)addr_mem(0x430034 ) |= (time2cycle(DDR2_tRFC_ps, tCK)<<24); //tRFC

	//TDATA
	*(volatile unsigned long *)addr_mem(0x430038 ) = DDR2_CL; //tRL
	*(volatile unsigned long *)addr_mem(0x430038 ) |= ((DDR2_CL-1)<<8); //tWL
	*(volatile unsigned long *)addr_mem(0x430038 ) |= (DDR2_CL<<16); //tCL
	tmp = time2cycle(DDR2_tRTP_ps, tCK);
	if(tmp<DDR2_tRTP_ck) tmp=DDR2_tRTP_ck;
	*(volatile unsigned long *)addr_mem(0x430038 ) |= (tmp<<20); //tRTP
	*(volatile unsigned long *)addr_mem(0x430038 ) |= (time2cycle(DDR2_tWR_ps, tCK)<<24); //tWR
	tmp = time2cycle(DDR2_tWTR_ps, tCK);
	if(tmp<DDR2_tWTR_ck) tmp=DDR2_tWTR_ck;
	*(volatile unsigned long *)addr_mem(0x430038 ) |= (tmp<<28); //tWTR

	//TPOWER
	*(volatile unsigned long *)addr_mem(0x43003C ) = DDR2_tMRD_ck; //tMRD
	*(volatile unsigned long *)addr_mem(0x43003C ) |= (DDR2_tCKE_ck<<4); //tCKE
	*(volatile unsigned long *)addr_mem(0x43003C ) |= (DDR2_tXP_ck<<8); //tXP
	*(volatile unsigned long *)addr_mem(0x43003C ) |= (DDR2_tXSR_ck<<16); //tXSR
	*(volatile unsigned long *)addr_mem(0x43003C ) |= (time2cycle(DDR2_tFAW_ps, tCK)<<24); //tFAW

//--------------------------------------------------------------------------
// MRS Setting

	//Direct Command
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x07000000;//NOP
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x01000000;//precharge all
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00020000;
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00030000;
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00010000;
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00000100;
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x01000000;//precharge all
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x05000000;//AREF
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x05000000;//AREF
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x05000000;//AREF
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00000000;	// DLL reset release.
	*(volatile unsigned long *)addr_mem(0x430010 ) = (DDR2_BURST_LEN|(DDR2_READ_BURST_TYPE<<3)|(DDR2_CL<<4)|((time2cycle(DDR2_tWR_ps, tCK)-1)<<9));
	i = 100; while(i--);
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00010380; // OCD Calibration default
	i = 100; while(i--);
	*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00010000 | (DDR2_ODT<<2) | (DDR2_DIC<<1);

	if(DDR2_LOGICAL_CHIP_NUM == 2)
	{
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x07000000 | Hw20;//NOP
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x01000000 | Hw20;//precharge all
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00020000 | Hw20;
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00030000 | Hw20;
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00010000 | Hw20;
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00000100 | Hw20;
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x01000000 | Hw20;//precharge all
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x05000000 | Hw20;//AREF
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x05000000 | Hw20;//AREF
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x05000000 | Hw20;//AREF
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00000000 | Hw20;	// DLL reset release.
		*(volatile unsigned long *)addr_mem(0x430010 ) = (DDR2_BURST_LEN|(DDR2_READ_BURST_TYPE<<3)|(DDR2_CL<<4)|((time2cycle(DDR2_tWR_ps, tCK)-1)<<9)) | Hw20;
		i = 100; while(i--);
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00010380 | Hw20; // OCD Calibration default
		i = 100; while(i--);
		*(volatile unsigned long *)addr_mem(0x430010 ) = 0x00010000 | (DDR2_ODT<<2) | (DDR2_DIC<<1) | Hw20;
	}

//--------------------------------------------------------------------------

	//Miscellaneous Configuration : COMMON
	*(volatile unsigned long *)addr_mem(0x410020) |= Hw17; //creq2=1 : exit low power
	while(!((*(volatile unsigned long *)addr_mem(0x410020))&Hw24)); //cack2==1 : wait for acknowledgement to request..

	*(volatile unsigned long *)addr_mem(0x430000 ) |= 0x00000020; //CONCONTROL : aref_en=1

	if(DDR2_BANK_NUM == 8)
		while (((*(volatile unsigned long *)addr_mem(0x430048)) & 0xFFFFFFFF)!= 0x44444444); //Wait PAUSED
	else//DDR2_BANK_NUM == 4
		while (((*(volatile unsigned long *)addr_mem(0x430048)) & 0x0000FFFF)!= 0x00004444); //Wait PAUSED

	if(DDR2_LOGICAL_CHIP_NUM == 2){
		if(DDR2_BANK_NUM == 8)
			while (((*(volatile unsigned long *)addr_mem(0x43004C)) & 0xFFFFFFFF)!= 0x44444444); //Wait PAUSED
		else//DDR2_BANK_NUM == 4
			while (((*(volatile unsigned long *)addr_mem(0x43004C)) & 0x0000FFFF)!= 0x00004444); //Wait PAUSED
	}

//--------------------------------------------------------------------------
#endif
}

static TCC_REG RegRepo;

/*===========================================================================
FUNCTION
===========================================================================*/
static void shutdown_mode(void)
{
	volatile unsigned int cnt = 0;
	unsigned way_mask;

	/*--------------------------------------------------------------
	 BUS Config
	--------------------------------------------------------------*/
	memcpy(&RegRepo.membuscfg, (PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE), sizeof(MEMBUSCFG));
	memcpy(&RegRepo.iobuscfg, (PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE), sizeof(IOBUSCFG));

	/* all peri io bus on */
	((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN0.nREG = 0xFFFFFFFF;
	((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN1.nREG = 0xFFFFFFFF;

	/*--------------------------------------------------------------
	 SMU & PMU
	--------------------------------------------------------------*/
	memcpy(&RegRepo.smuconfig, (PSMUCONFIG)tcc_p2v(HwSMUCONFIG_BASE), sizeof(SMUCONFIG));
#if defined(TCC_PM_PMU_CTRL)
	memcpy(&RegRepo.pmu, (PPMU)tcc_p2v(HwPMU_BASE), sizeof(PMU));
#endif
	memcpy(&RegRepo.timer, (PTIMER *)tcc_p2v(HwTMR_BASE), sizeof(TIMER));
	memcpy(&RegRepo.vic, (PVIC)tcc_p2v(HwVIC_BASE), sizeof(VIC));
	memcpy(&RegRepo.pic, (PPIC)tcc_p2v(HwPIC_BASE), sizeof(PIC));
	memcpy(&RegRepo.ckc, (PCKC)tcc_p2v(HwCKC_BASE), sizeof(CKC));

	/*--------------------------------------------------------------
	 GPIO
	--------------------------------------------------------------*/
	memcpy((void*)GPIO_REPOSITORY_ADDR, (PGPIO)tcc_p2v(HwGPIO_BASE), sizeof(GPIO));

#ifdef CONFIG_CACHE_L2X0
	/*--------------------------------------------------------------
	 L2 cache
	--------------------------------------------------------------*/
	RegRepo.L2_aux = *(volatile unsigned int *)(L2CACHE_BASE+L2X0_AUX_CTRL); //save aux
	if (RegRepo.L2_aux & (1 << 16))
		way_mask = (1 << 16) - 1;
	else
		way_mask = (1 << 8) - 1;
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CLEAN_WAY) = way_mask; //clean all
	while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CLEAN_WAY)&way_mask); //wait for clean
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC) = 0; //sync
	while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC)&1); //wait for sync
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 0; //cache off
#endif

	/*--------------------------------------------------------------
	 flush tlb & cache
	--------------------------------------------------------------*/
	local_flush_tlb_all();
	flush_cache_all();

	/////////////////////////////////////////////////////////////////
	save_cpu_reg(SHUTDOWN_FUNC_ADDR, CPU_DATA_REPOSITORY_ADDR, resore_cpu_reg);
	/////////////////////////////////////////////////////////////////

	__asm__ __volatile__ ("nop\n");

	/* all peri io bus on */
	((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN0.nREG = 0xFFFFFFFF;
	((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN1.nREG = 0xFFFFFFFF;

	/*--------------------------------------------------------------
	 SMU & PMU
	--------------------------------------------------------------*/
#if defined(TCC_PM_PMU_CTRL)
	//memcpy((PPMU)tcc_p2v(HwPMU_BASE), &RegRepo.pmu, sizeof(PMU));
	{
		while(((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.MAIN_STATE);

		if(RegRepo.pmu.PMU_PWRSTS.bREG.PU_HSB){ //if High Speed I/O Bus has been turn on.
			((PPMU)tcc_p2v(HwPMU_BASE))->PWRUP_HSBUS.bREG.DATA = 1;
			while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PU_HSB == 0);
			((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.HSB = 1;
			((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.HSIOBUS = 1;
			while (((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.HSIOBUS == 0);
		}
		if(RegRepo.pmu.PMU_PWRSTS.bREG.PU_DB){ //if DDI Bus has been turn on.
			((PPMU)tcc_p2v(HwPMU_BASE))->PWRUP_DBUS.bREG.DATA = 1;
			while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PU_DB == 0);
			((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.DB = 1;
			((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.DBUS = 1;
			while ((((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.DBUS0 & ((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.DBUS1) == 0);
		}
		if(RegRepo.pmu.PMU_PWRSTS.bREG.PU_GB){ //if Graphic Bus has been turn on.
			((PPMU)tcc_p2v(HwPMU_BASE))->PWRUP_GBUS.bREG.DATA = 1;
			while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PU_GB == 0);
			((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.GB = 1;
			((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.GBUS = 1;
			while (((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.GBUS == 0);
		}
		if(RegRepo.pmu.PMU_PWRSTS.bREG.PU_VB){ //if Video Bus has been turn on.
			((PPMU)tcc_p2v(HwPMU_BASE))->PWRUP_VBUS.bREG.DATA = 1;
			while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PU_VB == 0);
			((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.VB = 1;
			((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.VBUS = 1;
			while ((((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.VBUS0 & ((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.VBUS1) == 0);
		}

		//IP isolation restore
		//Bruce, PMU_ISOL is only write-only.. so it can't be set by back-up data..
		//((PPMU)tcc_p2v(HwPMU_BASE))->PMU_ISOL.nREG = RegRepo.PMU_ISOL.nREG;
		((PPMU)tcc_p2v(HwPMU_BASE))->PMU_ISOL.nREG = 0x00000000;
	}
#endif

	//memcpy((PCKC)tcc_p2v(HwCKC_BASE), &RegRepo.ckc, sizeof(CKC));
	{
		//PLL
		((PCKC)tcc_p2v(HwCKC_BASE))->PLL0CFG.nREG = RegRepo.ckc.PLL0CFG.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->PLL1CFG.nREG = RegRepo.ckc.PLL1CFG.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->PLL2CFG.nREG = RegRepo.ckc.PLL2CFG.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->PLL3CFG.nREG = RegRepo.ckc.PLL3CFG.nREG;
		//((PCKC)tcc_p2v(HwCKC_BASE))->PLL4CFG.nREG = RegRepo.ckc.PLL4CFG.nREG; //PLL4 is used as a source of memory bus clock
		//((PCKC)tcc_p2v(HwCKC_BASE))->PLL5CFG.nREG = RegRepo.ckc.PLL5CFG.nREG; //PLL5 is used as a source of CPU bus clock
		nop_delay(1000);

		//Divider
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKDIVC0.nREG = RegRepo.ckc.CLKDIVC0.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKDIVC1.nREG = RegRepo.ckc.CLKDIVC1.nREG;
		nop_delay(100);

		//((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL0.nREG = RegRepo.ckc.CLKCTRL0.nREG; //CPU Clock can't be adjusted freely.
		//((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL1.nREG = RegRepo.ckc.CLKCTRL1.nREG; //Memory Clock can't be adjusted freely.
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL2.nREG = RegRepo.ckc.CLKCTRL2.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL3.nREG = RegRepo.ckc.CLKCTRL3.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL4.nREG = RegRepo.ckc.CLKCTRL4.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL5.nREG = RegRepo.ckc.CLKCTRL5.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL6.nREG = RegRepo.ckc.CLKCTRL6.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL7.nREG = RegRepo.ckc.CLKCTRL7.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL8.nREG = RegRepo.ckc.CLKCTRL8.nREG;

		((PCKC)tcc_p2v(HwCKC_BASE))->SWRESET.nREG = ~(RegRepo.ckc.SWRESET.nREG);

		//Peripheral clock
		memcpy((void*)&(((PCKC)tcc_p2v(HwCKC_BASE))->PCLKCTRL00.nREG), (void*)&(RegRepo.ckc.PCLKCTRL00.nREG), 0x150-0x80);
	}
	nop_delay(100);

	memcpy((PPIC)tcc_p2v(HwPIC_BASE), &RegRepo.pic, sizeof(PIC));
	memcpy((PVIC)tcc_p2v(HwVIC_BASE), &RegRepo.vic, sizeof(VIC));
	memcpy((PTIMER *)tcc_p2v(HwTMR_BASE), &RegRepo.timer, sizeof(TIMER));

#if (0)
	//memcpy((PPMU)tcc_p2v(HwPMU_BASE), &RegRepo.pmu, sizeof(PMU));
	{
		if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_HSB){ //if High Speed I/O Bus has been turn off.
			((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.HSIOBUS = 0;
			while (((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.HSIOBUS == 1);
			((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.HSB = 0;
			((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_HSBUS.bREG.DATA = 1;
			while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PD_HSB == 0);
		}
		if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_DB){ //if DDI Bus has been turn off.
			((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.DBUS = 0;
			while ((((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.DBUS0 | ((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.DBUS1) == 1);
			((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.DB = 0;
			((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_DBUS.bREG.DATA = 1;
			while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PD_DB == 0);
		}
		if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_GB){ //if Graphic Bus has been turn off.
			((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.GBUS = 0;
			while (((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.GBUS == 1);
			((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.GB = 0;
			((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_GBUS.bREG.DATA = 1;
			while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PD_GB == 0);
		}
		if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_VB){ //if Video Bus has been turn off.
			((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.VBUS = 0;
			while ((((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.VBUS0 | ((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.VBUS1) == 1);
			((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.VB = 0;
			((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_VBUS.bREG.DATA = 1;
			while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PD_VB == 0);
		}
		//if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_MB) //Memory bus should be always turn on.

		//IP isolation off
		//Bruce, PMU_ISOL is only write-only.. so it can't be set by back-up data..
		//((PPMU)tcc_p2v(HwPMU_BASE))->PMU_ISOL.nREG = RegRepo.PMU_ISOL.nREG;
		((PPMU)tcc_p2v(HwPMU_BASE))->PMU_ISOL.nREG = 0x00000000;
	}
#endif

	memcpy((PSMUCONFIG)tcc_p2v(HwSMUCONFIG_BASE), &RegRepo.smuconfig, sizeof(SMUCONFIG));

	/*--------------------------------------------------------------
	 BUS Config
	--------------------------------------------------------------*/
	memcpy((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE), &RegRepo.iobuscfg, sizeof(IOBUSCFG));
	memcpy((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE), &RegRepo.membuscfg, sizeof(MEMBUSCFG));

	/*--------------------------------------------------------------
	 cpu re-init for VFP
	--------------------------------------------------------------*/
	__cpu_early_init();

#ifdef CONFIG_CACHE_L2X0
	/*--------------------------------------------------------------
	 L2 cache enable
	--------------------------------------------------------------*/
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 0; //cache off
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_AUX_CTRL) = RegRepo.L2_aux; //restore aux
	if (RegRepo.L2_aux & (1 << 16))
		way_mask = (1 << 16) - 1;
	else
		way_mask = (1 << 8) - 1;
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_INV_WAY) = way_mask; //invalidate all
	while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_INV_WAY)&way_mask); //wait for invalidate
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC) = 0; //sync
	while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC)&1); //wait for sync
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 1; //cache on
#endif

}
/*=========================================================================*/



/*===========================================================================

                        Power Management Driver

===========================================================================*/

/*===========================================================================
FUNCTION
	mode 0: sleep mode, mode 1: shut down mode
===========================================================================*/
static int tcc_pm_enter(suspend_state_t state)
{
	unsigned long flags;
//	unsigned reg_backup[20];

// -------------------------------------------------------------------------
// disable interrupt
	local_irq_save(flags);
	local_irq_disable();

// -------------------------------------------------------------------------
// replace pm functions
	memcpy((void*)SRAM_BOOT_ADDR,       (void*)SRAM_Boot,  SRAM_BOOT_SIZE);
	memcpy((void*)SHUTDOWN_FUNC_ADDR,   (void*)shutdown,   SHUTDOWN_FUNC_SIZE);
	memcpy((void*)WAKEUP_FUNC_ADDR,     (void*)wakeup,     WAKEUP_FUNC_SIZE);
	memcpy((void*)SDRAM_INIT_FUNC_ADDR, (void*)sdram_init, SDRAM_INIT_FUNC_SIZE);

// -------------------------------------------------------------------------
// enter shutdown mode
	shutdown_mode();

// -------------------------------------------------------------------------
// enable interrupt
	local_irq_restore(flags);

	return 0;
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void tcc_pm_power_off(void)
{
#ifdef CONFIG_RTC_DISABLE_ALARM_FOR_PWROFF_STATE		//Disable the RTC Alarm during the power off state
	extern volatile void tca_alarm_disable(unsigned int rtcbaseaddress);

	tca_alarm_disable(tcc_p2v(HwRTC_BASE));
#endif
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
static uint32_t restart_reason = 0x776655AA;

/*===========================================================================
FUNCTION
===========================================================================*/
static void tcc_pm_restart(char str)
{
	/* store restart_reason to USTS register */
	if (restart_reason != 0x776655AA)
		((PPMU)tcc_p2v(HwPMU_BASE))->PMU_USSTATUS.nREG = restart_reason;

	//printk(KERN_DEBUG "reboot: reason=0x%x\n", restart_reason);

	arch_reset(str, NULL);
}

/*===========================================================================
FUNCTION
===========================================================================*/
static int tcc_reboot_call(struct notifier_block *this, unsigned long code, void *cmd)
{
	/* XXX: convert reboot mode value because USTS register
	 * hold only 8-bit value
	 */
	if (code == SYS_RESTART) {
		if (cmd) {
			if (!strcmp(cmd, "bootloader")) {
				restart_reason = 1;	/* fastboot mode */
			} else if (!strcmp(cmd, "recovery")) {
				restart_reason = 2;	/* recovery mode */
			} else {
				restart_reason = 0;
			}
		} else {
			restart_reason = 0;
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

