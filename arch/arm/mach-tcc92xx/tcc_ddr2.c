/*
 * linux/arch/arm/mach-tcc8900/tcc_ckc_ctrl.c
 *
 * Author:  <linux@telechips.com>
 * Created: 10th February, 2009
 * Description: Interrupt handler for Telechips TCC8900 chipset
 *
 * Copyright (C) Telechips, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <mach/hardware.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/tick.h>
#include <linux/time.h>

#include <bsp.h>
#include <linux/tcc_ioctl.h>
#include "ddr.h"

/* Auto_PD:On, Read-Dealy:0x06, Drive_Strengtn:0x07, Pull_down/up/ZQ/UPDATE:forced off,
 * PHY ODT:0x02, SHIFTC:0x06, OFFSETC:0x00 
 */
//#define TCC_DDR2_LOWER_IDS_USE
//#define TCC_USE_MEM_360MHZ

#if defined(CONFIG_DDR2_HY5PS1G1631CFPS6)
#define DRAM_CAS	6
#elif defined(CONFIG_DDR2_K4T1G164QQ)
#define DRAM_CAS	5
#elif defined(CONFIG_DDR2_E1108AEBG) || defined(CONFIG_DDR2_E2116ABSE)
#define DRAM_CAS	6
#elif defined(CONFIG_DDR2_HXB18T2G160AF)
#define DRAM_CAS	5
#elif defined(CONFIG_DDR2_K4T1G164QF_BCE7)
#define DRAM_CAS	5
#elif defined(CONFIG_DDR2_MT47H128M8)
#define DRAM_CAS	5
#else
#error Not Selected DDR2
#endif

#if defined(CONFIG_TCC_MEM_512MB)
#define DRAM_ROW14	Hw3
#else
#define DRAM_ROW14	0
#endif

#define PMS(P, M, S)	((S) << 24 | (M) << 8 | (P))

typedef struct {
	unsigned int freq;
	unsigned int pll_freq;
	unsigned int pll;
	unsigned int div;
	unsigned int tRAS;
	unsigned int tRC;
	unsigned int tRCD;
	unsigned int tRFC;
	unsigned int tRP;
	unsigned int tRRD;
	unsigned int tWR;
	unsigned int tWTR;
	unsigned int tXP;
	unsigned int tXSR;
	unsigned int tESR;
	unsigned int tFAW;
	unsigned int nPeriod;
} mem_cfg_t;

static ddr_config_t ddr_mem_config[] = {
#if 1
	{
		.nFreq = 126000,
		.nClockSource = 3,
		.nPMS = PMS(1, 42, 1),
		.nDiv = 2,
		.nPeriod = 7937,
		.tREF_CYC = 982,
	},
#endif
	{
		.nFreq = 166000,
		.nClockSource = 3,
		.nPMS = PMS(3, 83, 0),
		.nDiv = 2,
		.nPeriod = 6024,
		.tREF_CYC = 1294,
	},
	{
		.nFreq = 198000,
		.nClockSource = 3,
		.nPMS = PMS(1, 33, 0),
		.nDiv = 2,
		.nPeriod = 5051,
		.tREF_CYC = 1544,
	},
	{
		.nFreq = 200000,
		.nClockSource = 3,
		.nPMS = PMS(3, 100, 0),
		.nDiv = 2,
		.nPeriod = 5000,
		.tREF_CYC = 1560,
	},
	{
		.nFreq = 240000,
		.nClockSource = 3,
		.nPMS = PMS(1, 40, 0),
		.nDiv = 2,
		.nPeriod = 4167,
		.tREF_CYC = 1872,
	},
	{
		.nFreq = 290000,
		.nClockSource = 3,
		.nPMS = PMS(3, 145, 0),
		.nDiv = 2,
		.nPeriod = 3448,
		.tREF_CYC = 2262,
	},
#if defined(CONFIG_VBUS_280MHZ_USE)
	{
		.nFreq = 325000,
		.nClockSource = 3,
		.nPMS = PMS(3, 140, 0),	// dummy 280MHz
		.nDiv = 2,
		.nPeriod = 3030,
		.tREF_CYC = 2574,
	},
#endif
	{
		.nFreq = 330000,
		.nClockSource = 3,
		.nPMS = PMS(2, 110, 0),
		.nDiv = 2,
		.nPeriod = 3030,
		.tREF_CYC = 2574,
	},
#if defined(TCC_USE_MEM_360MHZ)
	{
		.nFreq = 360000,
		.nClockSource = 3,
		.nPMS = PMS(2, 120, 0),
		.nDiv = 2,
		.nPeriod = 2778,
		.tREF_CYC = 2808,
	},
#endif
};

extern unsigned int IO_ARM_ChangeStackSRAM(void);
extern void IO_ARM_RestoreStackSRAM(unsigned int);

typedef void (*lpfunc)(ddr_config_t *);

static lpfunc lpSelfRefresh;

#define SRAM_COPY_ADDR				0xF0800000
#define SRAM_COPY_FUNC_SIZE			0x420

static unsigned int retstack = 0;
static unsigned long flags;

static void int_alldisable(void)
{
	local_irq_save(flags);
}

static void int_restore(void)
{
	local_irq_restore(flags);
}

static inline void *_memcpy(void *dst, const void *src, size_t len)
{
	size_t i;

	if ((uintptr_t) dst % sizeof(long) == 0 &&
	    (uintptr_t) src % sizeof(long) == 0 &&
	    len % sizeof(long) == 0) {
		long *d = dst;
		const long *s = src;

		for (i = 0; i < len/sizeof(long); i++) {
			d[i] = s[i];
		}
	} else {
		char *d = dst;
		const char *s = src;

		for (i = 0; i < len; i++) {
			d[i] = s[i];
		}
	}
	return dst;
}
#define DDR2_SETRCD(x) (x>3? (((x-3)<<8)| x ) : ((1<<8) | x))
#define DDR2_SETRFC(x) (x>3? (((x-3)<<8)| x ) : ((0<<8) | x))
#define DDR2_SETRP(x)  (x>3? (((x-3)<<8)| x ) : ((1<<8) | x))

static void tcc_mem_set_clock(ddr_config_t *mem_cfg)
{
	unsigned int odt_flag;
	ddr_config_t cfg;
	volatile int i;

	// it should be copied from SDRAM to SRAM because SDRAM will be unavailable
	_memcpy(&cfg, mem_cfg, sizeof(ddr_config_t));

	if (mem_cfg->nFreq < 200000)
		odt_flag = 1;
	else
		odt_flag = 0;

	*(volatile unsigned long *)0xF0302004 = 0x00000003;				// PL341_PAUSE
	while (((*(volatile unsigned long *)0xF0302000) & 0x3)!=2);	//Wait PL34X_STATUS_PAUSED

	*(volatile unsigned long *)0xF0302004 = 0x00000004;				// PL341_Configure
	while (((*(volatile unsigned long *)0xF0302000) & 0x3)!=0);	//Wait PL34X_STATUS_CONFIG

// DLL OFF
	*(volatile unsigned long *)0xF0304404 &= ~(0x00000003);			// DLL-0FF,DLL-Stop running
	*(volatile unsigned long *)0xF0304428 &= ~(0x00000003); 		// Calibration Start,Update Calibration
	*(volatile unsigned long *)0xF030302C &= ~(0x00004000);			//SDRAM IO Control Register Gatein Signal Power Down

	// Set the memory bus clock source to pll0 when the value over the 300MHz
	// 1. Set temporally memory clock
	*(volatile unsigned long *)0xF0400008 = (0x00200000 | (3 << 4) | 1); // CKC-CLKCTRL2 - Mem

	// 1-1. DDI & GPU & VBUS & VPU clock check
	if (((*(volatile unsigned long *)0xF0400004) & 0x07) == 3)
		*(volatile unsigned long *)0xF0400004 = (((*(volatile unsigned long *)0xF0400004)&0x00200000) | (1 << 4) | 2); // CKC-CLKCTRL1 - DDI
	if (((*(volatile unsigned long *)0xF040000C) & 0x07) == 3)
		*(volatile unsigned long *)0xF040000C = (((*(volatile unsigned long *)0xF040000C)&0x00200000) | (1 << 4) | 2); // CKC-CLKCTRL1 - GPU
	if (((*(volatile unsigned long *)0xF0400014) & 0x07) == 3)
		*(volatile unsigned long *)0xF0400014 = (((*(volatile unsigned long *)0xF0400014)&0x00200000) | (1 << 4) | 2); // CKC-CLKCTRL1 - VBUS
	if (((*(volatile unsigned long *)0xF0400018) & 0x07) == 3)
		*(volatile unsigned long *)0xF0400018 = (((*(volatile unsigned long *)0xF0400018)&0x00200000) | (1 << 4) | 2); // CKC-CLKCTRL1 - VCOD

	// 2. Change PLL3 Frequencey.
	//PLL 3 OFF
	*(volatile unsigned long *)0xF040002C &= ~(1<<31);

	// Set PLL3 Frequencey
	*(volatile unsigned long *)0xF040002C = cfg.nPMS;

	// refresh command
	//	for (i=0;i<0x400;i++)  HwPL340->direct_cmd = (1<<18);

	//PLL 3 ON
	*(volatile unsigned long *)0xF040002C |= (1<<31);

	// 3. Set memory clock
#if defined(CONFIG_VBUS_280MHZ_USE)
	if (mem_cfg->nFreq == 325000)
		*(volatile unsigned long *)0xF0400008 = (0x00200000 | ((cfg.nDiv-1) << 4) | 0); // CKC-CLKCTRL2 - Mem
	else
#endif
		*(volatile unsigned long *)0xF0400008 = (0x00200000 | ((cfg.nDiv-1) << 4) | 3); // CKC-CLKCTRL2 - Mem

//Init DDR2 
#if defined(TCC_DDR2_LOWER_IDS_USE)
	*(volatile unsigned long *)0xF030200C |= (0x00150012 | 1<<13 | 7<<7 | DRAM_ROW14);	// config 0  cas 10bit, ras 13bit	// AUTO_PD
#else
	*(volatile unsigned long *)0xF030200C |= (0x00150012 | DRAM_ROW14);					// config 0 : column 10bit, row 14bit
#endif
	*(volatile unsigned long *)0xF0302010 = cfg.tREF_CYC;	
	*(volatile unsigned long *)0xF030204C |= 0x00000571;		// config 2

	*(volatile unsigned long *)0xF0302014 = cfg.nCL;
	*(volatile unsigned long *)0xF030201C = cfg.tMRD;
	*(volatile unsigned long *)0xF0302020 = cfg.tRAS; 			// tRAS - 45ns
	*(volatile unsigned long *)0xF0302024 = cfg.tRC; 			// tRC	- 60ns
	*(volatile unsigned long *)0xF0302028 = DDR2_SETRCD(cfg.tRCD); // tRCD - 15ns
	*(volatile unsigned long *)0xF030202c = DDR2_SETRFC(cfg.tRFC); // tRFC - 105ns
	*(volatile unsigned long *)0xF0302030 = DDR2_SETRP(cfg.tRP); // tRP	- 15ns
	*(volatile unsigned long *)0xF0302034 = cfg.tRRD; 			// tRRD - 10ns
	*(volatile unsigned long *)0xF0302038 = cfg.tWR; 			// tWR - 15ns
	*(volatile unsigned long *)0xF030203c = cfg.tWTR; 			// tWTR - 7.5ns
	*(volatile unsigned long *)0xF0302040 = cfg.tXP; 			// tXP - min 2tCK
	*(volatile unsigned long *)0xF0302044 = cfg.tXSR;		 	// tXSR
	*(volatile unsigned long *)0xF0302048 = cfg.tESR;		 	// tESR
	*(volatile unsigned long *)0xF0302054 = ((cfg.tFAW-3)<<8) | cfg.tFAW; 	// tFAW

#if defined(CONFIG_TCC_MEM_128MB)
	*(volatile unsigned long *)0xF0302200 = 0
										| (0xf8 << 0)		/* Address Mask */
										| (0x40 << 8)		/* Address Match */
										| (0 << 16)		/* brc_n_rbc */
										;
#elif defined(CONFIG_TCC_MEM_256MB)
	*(volatile unsigned long *)0xF0302200 = 0
										| (0xf0 << 0)		/* Address Mask */
										| (0x40 << 8)		/* Address Match */
										| (0 << 16)		/* brc_n_rbc */
										;
#elif defined(CONFIG_TCC_MEM_512MB)
	*(volatile unsigned long *)0xF0302200 = 0
										| (0xe0 << 0)		/* Address Mask */
										| (0x40 << 8)		/* Address Match */
										| (0 << 16)		/* brc_n_rbc */
										;
#else
	#error not defined memory size
#endif

#if defined(TCC_DDR2_LOWER_IDS_USE)
	*(volatile unsigned long *)0xF030440C |= 0x6;		// SHIFTC
	*(volatile unsigned long *)0xF030440C &= ~((0x7F << 3) | 0x01);		// OFFSETC
#else
#if (defined(CONFIG_DDR2_MT47H128M8) || defined(CONFIG_DDR2_HXB18T2G160AF))
	*(volatile unsigned long *)0xF030440C |= 0x6;			// SHIFTC
	*(volatile unsigned long *)0xF030440C &= ~(0x7F << 3);	// OFFSETC
	*(volatile unsigned long *)0xF0304420 = 0x0;			// OFFSETD
#endif
#endif

	*(volatile unsigned long *)0xF0304428 =  (3 << 17)	// PRD_CAL
						| (0 << 16)	// PRD_CEN
						| (7 << 13)	// DRV_STR
						| ((odt_flag&0x1) << 12)	// TERM_DIS
						| (2 << 9)	// ODT(PHY) value 75 Ohm
#if defined(TCC_DDR2_LOWER_IDS_USE)
						| (0 << 6)	// PULL UP
						| (0 << 3)	// PULL DOWN
#else
						| (5 << 6)	// PULL UP
						| (2 << 3)	// PULL DOWN
#endif
						| (0 << 2)	// ZQ
						| (0 << 1)	// UPDATE
						| (1 << 0);	// CAL_START
	
	while (!((*(volatile unsigned long *)0xF030442c) & (1)));	// Wait until Calibration completion without error
	*(volatile unsigned long *)0xF0304428 =  (3 << 17)	// PRD_CAL
						| (0 << 16)	// PRD_CEN
						| (7 << 13)	// DRV_STR
						| ((odt_flag&0x1) << 12)	// TERM_DIS
						| (2 << 9)	// ODT(PHY) value
#if defined(TCC_DDR2_LOWER_IDS_USE)
						| (0 << 6)	// PULL UP
						| (0 << 3)	// PULL DOWN
#else
						| (5 << 6)	// PULL UP
						| (2 << 3)	// PULL DOWN
#endif
						| (0 << 2)	// ZQ
						| (1 << 1)	// UPDATE
						| (1 << 0);	// CAL_START
	*(volatile unsigned long *)0xF0304428 =  (3 << 17)	// PRD_CAL
						| (0 << 16)	// PRD_CEN
						| (7 << 13)	// DRV_STR
						| ((odt_flag&0x1) << 12)	// TERM_DIS
						| (2 << 9)	// ODT(PHY) value
#if defined(TCC_DDR2_LOWER_IDS_USE)
						| (0 << 6)	// PULL UP
						| (0 << 3)	// PULL DOWN
#else
						| (5 << 6)	// PULL UP
						| (2 << 3)	// PULL DOWN
#endif
						| (0 << 2)	// ZQ
						| (0 << 1)	// UPDATE
						| (1 << 0);	// CAL_START

#if defined(TCC_DDR2_LOWER_IDS_USE)
	*(volatile unsigned long *)0xF0304430 |= 0x6;		// Read Delay
	*(volatile unsigned long *)0xF0304430 &= ~0x1;
#endif

	*(volatile unsigned long *)0xF030302C |=  (0x00004000);	//SOC1-3	// Gatein signal Power down
	*(volatile unsigned long *)0xF0304404 = 0x00000001; 		// DLLCTRL
#if defined(TCC_USE_MEM_360MHZ)
	if (mem_cfg->nFreq > 333000)
		*(volatile unsigned long *)0xF0304408 = 0x00001212; 	// DLLPDCFG
	else
#endif
	if (mem_cfg->nFreq > 266000)
		*(volatile unsigned long *)0xF0304408 = 0x00001717; 	// DLLPDCFG
#if 0
	else if (mem_cfg->nFreq > 166000)
		*(volatile unsigned long *)0xF0304408 = 0x00002E2E; 	// DLLPDCFG
	else
		*(volatile unsigned long *)0xF0304408 = 0x00003E3E; 	// DLLPDCFG
#else
	else
		*(volatile unsigned long *)0xF0304408 = 0x00002020; 	// DLLPDCFG
#endif
	*(volatile unsigned long *)0xF0304404 = 0x00000003; 		// DLLCTRL
	while (((*(volatile unsigned long *)0xF0304404) & (0x00000018)) != (0x00000018));	// Wait DLL Lock

#if (defined(CONFIG_DDR2_MT47H128M8) || defined(CONFIG_DDR2_HXB18T2G160AF))
	*(volatile unsigned long *)0xF0302008 = 0x000c0000;			// Direct COmmnad Register nop
	*(volatile unsigned long *)0xF0302008 = 0x00000000;			// Direct COmmnad Register precharge all
	*(volatile unsigned long *)0xF0302008 = 0x000a0000;			// Direct COmmnad Register EMR2
	*(volatile unsigned long *)0xF0302008 = 0x000b0000;			// Direct COmmnad Register EMR3
	*(volatile unsigned long *)0xF0302008 = 0x00090000;			// Direct COmmnad Register EMR1
	*(volatile unsigned long *)0xF0302008 = 0x00080100;			// Direct Command Register MRS0 DLL Reset
	*(volatile unsigned long *)0xF0302008 = 0x00000000;			// Direct COmmnad Register precharge all

	*(volatile unsigned long *)0xF0302008 = 0x00040000;			// Direct COmmnad Register auto refresh
	*(volatile unsigned long *)0xF0302008 = 0x00040000;			// Direct COmmnad Register 
	*(volatile unsigned long *)0xF0302008 = 0x00040000;			// Direct COmmnad Register 

	*(volatile unsigned long *)0xF0302008 = 0x00080000;			// Direct Command Register MRS0 DLL Reset Release
#else
	*(volatile unsigned long *)0xF0302008 = 0x000c0000;			// Direct COmmnad Register 
	*(volatile unsigned long *)0xF0302008 = 0x00000000;			// Direct COmmnad Register 
	*(volatile unsigned long *)0xF0302008 = 0x00040000;			// Direct COmmnad Register 
	*(volatile unsigned long *)0xF0302008 = 0x00040000;			// Direct COmmnad Register 
	*(volatile unsigned long *)0xF0302008 = 0x000a0000;			// Direct COmmnad Register 
	*(volatile unsigned long *)0xF0302008 = 0x000b0000;			// Direct COmmnad Register 
	*(volatile unsigned long *)0xF0302008 = 0x00090000;			// Direct COmmnad Register	  
	
#if defined(DRAM_CAS6)
	*(volatile unsigned long *)0xF0302008 = 0x00080962;			// Direct COmmnad Register 
#else
	*(volatile unsigned long *)0xF0302008 = 0x00080952;			// Direct COmmnad Register 
#endif
		
	*(volatile unsigned long *)0xF0302008 = 0x00000000; 		// Direct COmmnad Register 

	*(volatile unsigned long *)0xF0302008 = 0x00040000;			// dir_cmd
#endif

	*(volatile unsigned long *)0xF0302008 = (0x00080002|(DRAM_CAS<<4)|((cfg.tWR-1)<<9)); //MRS

	i=100;
	while(i) { i--; }

//	if (!odt_flag) {
		*(volatile unsigned long *)0xF0302008 = (0x00090000 | (0x7 << 7)); 	// Direct COmmnad Register EMR1 with Enable OCD
		*(volatile unsigned long *)0xF0302008 = (0x00090000 | (Hw2)); 		// Direct COmmnad Register EMR1 with ODT 75Ohm
//	}
	
	*(volatile unsigned long *)0xF0302004 = 0x00000000; 		// PL341_GO
	while (((*(volatile unsigned long *)0xF0302000) & 0x3)!=1); //Wait PL34X_STATUS_GO
}

static ddr_config_t *get_mem_config(unsigned int freq)
{
	int i;
	ddr_config_t *p = &ddr_mem_config[0];

	for (i = 0; i < ARRAY_SIZE(ddr_mem_config); i++) {
		if (p->nFreq >= freq) {
			//pr_err("ddr2: find memory clock: %d, (%d)\n", p->nFreq, freq);
#if defined(CONFIG_VBUS_280MHZ_USE)
			if (freq != 325000 && p->nFreq == 325000)
				;
			else
#endif
			break;
		}
		++p;
	}

	return p;
}

static void init_copychangeclock(unsigned int freq)
{
	lpSelfRefresh = (lpfunc)(SRAM_COPY_ADDR);

	_memcpy((void *) SRAM_COPY_ADDR, tcc_mem_set_clock, SRAM_COPY_FUNC_SIZE);

	// Jump to Function Start Point
	lpSelfRefresh(get_mem_config(freq));
}





#define TCC_VSYNC_INT_USE
#ifdef TCC_VSYNC_INT_USE
#define TCC_VSYNC_INT	HwINT0_EI6
#endif

#include <mach/tca_lcdc.h>
#include <mach/tcc_scaler_ctrl.h>

void tcc_ddr2_set_clock(unsigned int freq)
{
	unsigned int lcdc0_on = 0, lcdc1_on = 0;
	volatile PLCDC	pLCDC_BASE0 = (volatile PLCDC)tcc_p2v(HwLCDC0_BASE);
	volatile PLCDC	pLCDC_BASE1 = (volatile PLCDC)tcc_p2v(HwLCDC1_BASE);
	unsigned int scale0, scale1; 

#if defined(CONFIG_GENERIC_TIME)
	volatile PPIC	pPIC	= (volatile PPIC)tcc_p2v(HwPIC_BASE);
#else
	volatile PTIMER	pTIMER	= (volatile PTIMER)tcc_p2v(HwTMR_BASE);
#endif

#ifdef TCC_VSYNC_INT_USE
	HwPIC->INTMSK0 &= ~TCC_VSYNC_INT;
#endif

	scale0 = DEV_M2M_Wait_signal_disable(0);
	scale1 = DEV_M2M_Wait_signal_disable(1);

	lcdc0_on = DEV_LCDC_Wait_signal(0);
	lcdc1_on = DEV_LCDC_Wait_signal(1);

	int_alldisable();
	local_flush_tlb_all();
	flush_cache_all();

#if defined(CONFIG_GENERIC_TIME)
	pPIC->IEN0 &= ~Hw1;		/* Disable Timer0 interrupt */
#else
	pTIMER->TC32EN &= ~Hw24;
#endif

	retstack = IO_ARM_ChangeStackSRAM();

	init_copychangeclock(freq);

	IO_ARM_RestoreStackSRAM(retstack);

#if !defined(CONFIG_GENERIC_TIME)
	pTIMER->TC32EN |= Hw24;
#endif

	int_restore();				

#ifdef TCC_VSYNC_INT_USE
	HwPIC->INTMSK0 |= TCC_VSYNC_INT;
	HwPIC->CLR0    |= TCC_VSYNC_INT;
//	HwPIC->IEN0    |= TCC_VSYNC_INT;
#endif

#if defined(CONFIG_GENERIC_TIME)
	pPIC->IEN0 |= Hw1;		/* Enable Timer0 interrupt */
#endif

	//	LCDC Power Up
	if (lcdc0_on) {
		pLCDC_BASE0->LCTRL |= Hw0;
	}

	if (lcdc1_on) {		
		pLCDC_BASE1->LCTRL |= Hw0;
	}
	
}
EXPORT_SYMBOL(tcc_ddr2_set_clock);

static int __init tcc_ddr_init(void)
{	
	int i;
	unsigned int period;
	ddr_config_t *mem_cfg;

	for (i = 0; i < ARRAY_SIZE(ddr_mem_config); i++) {
		mem_cfg = &ddr_mem_config[i];
		period = mem_cfg->nPeriod;

#if defined(CONFIG_DDR2_K4T1G164QQ)
		mem_cfg->nCW = 2;
		mem_cfg->nRW = 3;
		mem_cfg->nCL = DRAM_CAS;	/* CAS latency */
		mem_cfg->nBurstLength = 4;
		mem_cfg->nGateDelay = 3;
		mem_cfg->nReadDelay = 3;

		mem_cfg->tMRD =      2;	/* cycle number (min. 2 cycles) */
		mem_cfg->tWTR =      5;	/* cycle number (min. 3+BL/2, BL=4) */
		mem_cfg->tXP  =      1;	/* cycle number (min. 1 cycles) */
		mem_cfg->tESR =    200;	/* cycle number (min. 200 cycles) */
		mem_cfg->tRAS =  45000;	/* pico second (45 - 70,000ns) */
		mem_cfg->tRC  =  60000;	/* pico second (min. 60ns) */
		mem_cfg->tRCD =  15000;	/* pico second (min. 15ns) */
		mem_cfg->tRFC = 105000;	/* pico second (min. 105ns) */
		mem_cfg->tRP  =  15000;	/* pico second (min. 15ns) */
		mem_cfg->tRRD =  10000;	/* pico second (min. 10ns) */
		mem_cfg->tWR  =  15000;	/* pico second (min. 15ns) */
		mem_cfg->tXSR =    200;	/* cycle number (min. 200 cycle) */
		
		mem_cfg->tFAW =  50000;	/* pico second (min. 50ns) */
#elif defined(CONFIG_DDR2_HY5PS1G1631CFPS6) || defined(CONFIG_DDR2_E1108AEBG)
		mem_cfg->nCW = 2;
		mem_cfg->nRW = 3;
		mem_cfg->nCL = DRAM_CAS;	/* CAS latency */
		mem_cfg->nBurstLength = 4;
		mem_cfg->nGateDelay = 3;
		mem_cfg->nReadDelay = 3;

		mem_cfg->tMRD =      2;	/* cycle number (min. 2 cycles) */
		mem_cfg->tWTR =      5;	/* cycle number (min. 3+BL/2, BL=4) */
		mem_cfg->tXP  =      2;	/* cycle number (min. 2 cycles) */
		mem_cfg->tESR =    200;	/* cycle number (min. 200 cycles) */
		mem_cfg->tRAS =  45000;	/* pico second (45 - 70,000ns) */
		mem_cfg->tRC  =  60000;	/* pico second (min. 60ns) */
		mem_cfg->tRCD =  15000;	/* pico second (min. 15ns) */
		mem_cfg->tRFC = 128000;	/* pico second (min. 127.5ns) */
		mem_cfg->tRP  =  15000;	/* pico second (min. 15ns) */
		mem_cfg->tRRD =  10000;	/* pico second (min. 10ns) */
		mem_cfg->tWR  =  15000;	/* pico second (min. 15ns) */

		//Bruce, 101027,HY5PS1G163CFPS6에 대해서 XSRD 대신 XSRN 으로 변경
		//mem_cfg->tXSR =    200;	/* cycle number (min. 200 cycle) */
		#ifdef CONFIG_DDR2_HY5PS1G1631CFPS6
		mem_cfg->tXSR = 138000;	/* pico second (min. 137.5ns  (RFC+10)) */
		#else
		mem_cfg->tXSR =    200;	/* cycle number (min. 200 cycle) */
		#endif

		mem_cfg->tFAW =  45000;	/* pico second (min. 45ns) */
#elif defined(CONFIG_DDR2_E2116ABSE)
		mem_cfg->nCW = 2;
		mem_cfg->nRW = 3;
		mem_cfg->nCL = DRAM_CAS;	/* CAS latency */
		mem_cfg->nBurstLength = 4;
		mem_cfg->nGateDelay = 3;
		mem_cfg->nReadDelay = 3;

		mem_cfg->tMRD =      2;	/* cycle number (min. 2 cycles) */
		mem_cfg->tWTR =      3;	/* cycle number (min.     3+BL/2, BL=4) */
		mem_cfg->tXP  =      3;	/* cycle number (min. 2 cycles) */
		mem_cfg->tESR =    200;	/* cycle number (min. 200 cycles) */
		mem_cfg->tRAS =  45000;	/* pico second (45 - 70,000ns) */
		mem_cfg->tRC  =  60000;	/* pico second (min. 60ns) */
		mem_cfg->tRCD =  18000;	/* pico second (min. 15ns) */
		mem_cfg->tRFC = 195000;	/* pico second (min. 127.5ns) */
		mem_cfg->tRP  =  18000;	/* pico second (min. 15ns) */
		mem_cfg->tRRD =  10000;	/* pico second (min. 10ns) */
		mem_cfg->tWR  =  15000;	/* pico second (min. 15ns) */
		mem_cfg->tXSR =    200;	/* cycle number (min. 200 cycle) */
		
		mem_cfg->tFAW =  45000;	/* pico second (min. 45ns) */
#elif defined(CONFIG_DDR2_HXB18T2G160AF)
		mem_cfg->nCW = 2;
		mem_cfg->nRW = 3;
		mem_cfg->nCL = DRAM_CAS;	/* CAS latency */
		mem_cfg->nBurstLength = 4;
		mem_cfg->nGateDelay = 3;
		mem_cfg->nReadDelay = 3;

		mem_cfg->tMRD =      2;	/* cycle number (min. 2 cycles) */
		mem_cfg->tWTR =      3;	/* cycle number (min.     3+BL/2, BL=4) */
		mem_cfg->tXP  =      3;	/* cycle number (min. 2 cycles) */
		mem_cfg->tESR =    200;	/* cycle number (min. 200 cycles) */
		mem_cfg->tRAS =  45000;	/* pico second (45 - 70,000ns) */
		mem_cfg->tRC  =  57500;	/* pico second (min. 60ns) */
		mem_cfg->tRCD =  12500;	/* pico second (min. 15ns) */
		mem_cfg->tRFC = 195000;	/* pico second (min. 127.5ns) */
		mem_cfg->tRP  =  12500;	/* pico second (min. 15ns) */
		mem_cfg->tRRD =  10000;	/* pico second (min. 10ns) */
		mem_cfg->tWR  =  15000;	/* pico second (min. 15ns) */
		mem_cfg->tXSR =    200;	/* cycle number (min. 200 cycle) */

		mem_cfg->tFAW =  45000;	/* pico second (min. 35ns) */
#elif defined(CONFIG_DDR2_MT47H128M8)
		mem_cfg->nCW = 2;
		mem_cfg->nRW = 3;
		mem_cfg->nCL = DRAM_CAS;	/* CAS latency */
		mem_cfg->nBurstLength = 4;
		mem_cfg->nGateDelay = 3;
		mem_cfg->nReadDelay = 3;

		mem_cfg->tMRD =      2;	/* cycle number (min. 2 cycles) */
		mem_cfg->tWTR =      3;	/* cycle number (min.     3+BL/2, BL=4) */
		mem_cfg->tXP  =      2;	/* cycle number (min. 2 cycles) */
		mem_cfg->tESR =    200;	/* cycle number (min. 200 cycles) */
		mem_cfg->tRAS =  40000;	/* pico second (40 - 70,000ns) */
		mem_cfg->tRC  =  55000;	/* pico second (min. 60ns) */
		mem_cfg->tRCD =  15000;	/* pico second (min. 15ns) */
		mem_cfg->tRFC = 127500;	/* pico second (min. 127.5ns) */
		mem_cfg->tRP  =  15000;	/* pico second (min. 15ns) */
		mem_cfg->tRRD =   7500;	/* pico second (min. 10ns) */
		mem_cfg->tWR  =  18000;	/* pico second (min. 15ns) */
		mem_cfg->tXSR =    200;	/* cycle number (min. 200 cycle) */

		mem_cfg->tFAW =  37500;	/* pico second (min. 35ns) */
#elif defined(CONFIG_DDR2_K4T1G164QF_BCE7)
		mem_cfg->nCW = 2;
		mem_cfg->nRW = 3;
		mem_cfg->nCL = DRAM_CAS;	/* CAS latency */
		mem_cfg->nBurstLength = 4;
		mem_cfg->nGateDelay = 3;
		mem_cfg->nReadDelay = 3;

		mem_cfg->tMRD =      2;	/* cycle number (min. 2 cycles) */
		mem_cfg->tWTR =      5;	/* cycle number (min. 3+BL/2, BL=4) */
		mem_cfg->tXP  =      2;	/* cycle number (min. 2 cycles)*/
		mem_cfg->tESR =    200;	/* cycle number (min. 200 cycles) */
		mem_cfg->tRAS =  45000;	/* pico second (45 - 70,000ns) */
		mem_cfg->tRC  =  57500;	/* pico second (min. 57.5ns) */
		mem_cfg->tRCD =  12500;	/* pico second (min. 12.5ns) */
		mem_cfg->tRFC = 127500;	/* pico second (min. 127.5ns) */
		mem_cfg->tRP  =  12500;	/* pico second (min. 12.5ns) */
		mem_cfg->tRRD =  10000;	/* pico second (min. 10ns) */
		mem_cfg->tWR  =  15000;	/* pico second (min. 15ns) */
		mem_cfg->tXSR =    200;	/* pico second (min. 200nCLK) */
		mem_cfg->tFAW =  50000;	/* pico second (min. 45ns) */
#else
#error Unknown memory type!
#endif

/*
// sangwon
cl :10, mrd:2, ras:12, rc :15
rcd:4, rfc:27, rp :4, rrd:3
wr :4, wtr:1, xp :2, xsr:29

// soc
cl :10, mrd:3, ras:13, rc :17
rcd:5, rfc:36, rp :5, rrd:3
wr :5, wtr:3, xp :3, xsr:39
*/
		mem_cfg->nCL  = (mem_cfg->nCL > 0x7) ? 0x7 : mem_cfg->nCL;
        mem_cfg->nCL <<= 1;
		mem_cfg->tMRD = (mem_cfg->tMRD > 0x7f) ? 0x7f : mem_cfg->tMRD;
		mem_cfg->tRAS = ddr_time_to_cycle(period, mem_cfg->tRAS, 0x0, 0x1f);
		mem_cfg->tRC  = ddr_time_to_cycle(period, mem_cfg->tRC, 0x0, 0x1f);
		mem_cfg->tRCD = ddr_time_to_cycle(period, mem_cfg->tRCD, 0x4, 0xF);
		mem_cfg->tRFC = ddr_time_to_cycle(period, mem_cfg->tRFC, 0x4, 0x7f);
		mem_cfg->tRP  = ddr_time_to_cycle(period, mem_cfg->tRP, 0x4, 0x7);
		mem_cfg->tRRD = ddr_time_to_cycle(period, mem_cfg->tRRD, 0x0, 0xf);
		mem_cfg->tWR  = ddr_time_to_cycle(period, mem_cfg->tWR, 0x0, 0x7);
		mem_cfg->tWTR = (mem_cfg->tWTR > 0x7) ? 0x7 : mem_cfg->tWTR;
		mem_cfg->tXP  = (mem_cfg->tXP > 0xff) ? 0xff : mem_cfg->tXP;

		//Bruce, 101027,HY5PS1G163CFPS6에 대해서 XSRD 대신 XSRN 으로 변경
		//mem_cfg->tXSR = (mem_cfg->tXSR > 0xff) ? 0xff : mem_cfg->tXSR;
		#ifdef CONFIG_DDR2_HY5PS1G1631CFPS6
		mem_cfg->tXSR = ddr_time_to_cycle(period, mem_cfg->tXSR, 0, 0xff);
		#else
		mem_cfg->tXSR = (mem_cfg->tXSR > 0xff) ? 0xff : mem_cfg->tXSR;
		#endif

		mem_cfg->tESR = (mem_cfg->tESR > 0xff) ? 0xff : mem_cfg->tESR;
		mem_cfg->tFAW  = ddr_time_to_cycle(period, mem_cfg->tFAW, 0x0, 0x1F);
		mem_cfg->tREF_CYC = (mem_cfg->tREF_CYC > 0x7fff) ? 0x7fff : mem_cfg->tREF_CYC;

#if 0
		printk("cl :0x%x, mrd:0x%x, ras:0x%x, rc :0x%x, rcd:0x%x\n", mem_cfg->nCL, mem_cfg->tMRD, mem_cfg->tRAS, mem_cfg->tRC, mem_cfg->tRCD);
		printk("rfc:0x%x, rp :0x%x, rrd:0x%x, wr :0x%x, wtr:0x%x\n", mem_cfg->tRFC, mem_cfg->tRP, mem_cfg->tRRD, mem_cfg->tWR, mem_cfg->tWTR);
		printk("xp :0x%x, xsr:0x%x, esr:0x%x, faw:0x%x, ref:0x%x\n", mem_cfg->tXP, mem_cfg->tXSR, mem_cfg->tESR, mem_cfg->tFAW, mem_cfg->tREF_CYC);
#endif

	}
	return 0;
}

__initcall(tcc_ddr_init);

