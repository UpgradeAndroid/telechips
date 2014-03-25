/*
 * linux/arch/arm/mach-tcc92x/tcc_lpddr.c
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

#include <mach/bsp.h>
#include "ddr.h"

#define PMS(P, M, S)	((S) << 24 | (M) << 8 | (P))

//#define TCC_LPDDR_USE_DYNAMIC_PLL_FREQ

#ifdef TCC_LPDDR_USE_DYNAMIC_PLL_FREQ
static ddr_config_t lpddr_mem_config[] = {
	{
		.nFreq = 18000,
		.nClockSource = 3,
		.nPMS = PMS(1, 48, 3),
		.nDiv = 4,
		.nPeriod = 55556,
	},
	{
		.nFreq = 27000,
		.nClockSource = 3,
		.nPMS = PMS(1, 36, 1),
		.nDiv = 8,
		.nPeriod = 37037,
	},
	{
		.nFreq = 36000,
		.nClockSource = 3,
		.nPMS = PMS(1, 48, 3),
		.nDiv = 2,
		.nPeriod = 27778,
	},
	{
		.nFreq = 46000,
		.nClockSource = 3,
		.nPMS = PMS(3, 92, 1),
		.nDiv = 4,
		.nPeriod = 21739,
	},
	{
		.nFreq = 54000,
		.nClockSource = 3,
		.nPMS = PMS(1, 36, 1),
		.nDiv = 4,
		.nPeriod = 18519,
	},
	{
		.nFreq = 66000,
		.nClockSource = 3,
		.nPMS = PMS(1, 44, 2),
		.nDiv = 2,
		.nPeriod = 15152,
	},
	{
		.nFreq = 80000,
		.nClockSource = 3,
		.nPMS = PMS(3, 80, 0),
		.nDiv = 4,
		.nPeriod = 12500,
	},
	{
		.nFreq = 83000,
		.nClockSource = 3,
		.nPMS = PMS(3, 83, 0),
		.nDiv = 4,
		.nPeriod = 12048,
	},
	{
		.nFreq = 92000,
		.nClockSource = 3,
		.nPMS = PMS(3, 92, 1),
		.nDiv = 2,
		.nPeriod = 10870,
	},
	{
		.nFreq = 96000,
		.nClockSource = 3,
		.nPMS = PMS(1, 32, 1),
		.nDiv = 2,
		.nPeriod = 10417,
	},
	{
		.nFreq = 100000,
		.nClockSource = 3,
		.nPMS = PMS(3, 100, 0),
		.nDiv = 4,
		.nPeriod = 10000,
	},
	{
		.nFreq = 111000,
		.nClockSource = 3,
		.nPMS = PMS(1, 37, 1),
		.nDiv = 2,
		.nPeriod = 9091,
	},
	{
		.nFreq = 122000,
		.nClockSource = 3,
		.nPMS = PMS(3, 122, 1),
		.nDiv = 2,
		.nPeriod = 8197,
	},
	{
		.nFreq = 130000,
		.nClockSource = 3,
		.nPMS = PMS(3, 130, 1),
		.nDiv = 2,
		.nPeriod = 7693,
	},
	{
		.nFreq = 141000,
		.nClockSource = 3,
		.nPMS = PMS(1, 47, 1),
		.nDiv = 2,
		.nPeriod = 7093,
	},
	{
		.nFreq = 152000,
		.nClockSource = 3,
		.nPMS = PMS(3, 76, 0),
		.nDiv = 2,
		.nPeriod = 6579,
	},
	{
		.nFreq = 160000,
		.nClockSource = 3,
		.nPMS = PMS(3, 80, 0),
		.nDiv = 2,
		.nPeriod = 6250,
	},
	{
		.nFreq = 166000,
		.nClockSource = 3,
		.nPMS = PMS(3, 83, 0),
		.nDiv = 2,
		.nPeriod = 6025,
	},
#if defined(CONFIG_HIGH_PERFORMANCE)
	{
		.nFreq = 170000,
		.nClockSource = 3,
		.nPMS = PMS(3, 85, 0),
		.nDiv = 2,
		.nPeriod = 5883,
	},
	{
		.nFreq = 180000,
		.nClockSource = 3,
		.nPMS = PMS(1, 30, 0),
		.nDiv = 2,
		.nPeriod = 5556,
	},
	{
		.nFreq = 190000,
		.nClockSource = 3,
		.nPMS = PMS(3, 95, 0),
		.nDiv = 2,
		.nPeriod = 5263,
	},
	{
		.nFreq = 200000,
		.nClockSource = 3,
		.nPMS = PMS(3, 100, 0),
		.nDiv = 2,
		.nPeriod = 5000,
	},
#endif
};
#else
static ddr_config_t lpddr_mem_config[] = {
	{
		.nFreq = 19200,
		.nClockSource = 1,
		.nPMS = PMS(1, 32, 0),
		.nDiv = 20,
		.nPeriod = 52083,
	},
	{
		.nFreq = 24000,
		.nClockSource = 1,
		.nPMS = PMS(1, 32, 0),
		.nDiv = 16,
		.nPeriod = 41667,
	},
	{
		.nFreq = 32000,
		.nClockSource = 1,
		.nPMS = PMS(1, 32, 0),
		.nDiv = 12,
		.nPeriod = 31250,
	},
	{
		.nFreq = 40000,
		.nClockSource = 3,
		.nPMS = PMS(3, 100, 0),
		.nDiv = 10,
		.nPeriod = 25000,
	},
#if 0	// Not Use..   cause lock-up in MP3 plaing..
	{
		.nFreq = 48000,
		.nClockSource = 1,
		.nPMS = PMS(1, 32, 0),
		.nDiv = 8,
		.nPeriod = 20833,
	},
#endif
	{
		.nFreq = 55000,
		.nClockSource = 2,
		.nPMS = PMS(3, 83, 0),
		.nDiv = 6,
		.nPeriod = 18072,
	},
	{
		.nFreq = 64000,
		.nClockSource = 1,
		.nPMS = PMS(1, 32, 0),
		.nDiv = 6,
		.nPeriod = 15625,
	},
	{
		.nFreq = 83000,
		.nClockSource = 2,
		.nPMS = PMS(3, 83, 0),
		.nDiv = 4,
		.nPeriod = 12048,
	},
	{
		.nFreq = 96000,
		.nClockSource = 1,
		.nPMS = PMS(1, 32, 0),
		.nDiv = 4,
		.nPeriod = 10417,
	},
	{
		.nFreq = 100000,
		.nClockSource = 3,
		.nPMS = PMS(3, 100, 0),
		.nDiv = 4,
		.nPeriod = 10000,
	},
	{
		.nFreq = 166000,
		.nClockSource = 2,
		.nPMS = PMS(3, 83, 0),
		.nDiv = 2,
		.nPeriod = 6025,
	},
#if defined(CONFIG_HIGH_PERFORMANCE)
	{
		.nFreq = 192000,
		.nClockSource = 1,
		.nPMS = PMS(1, 32, 0),
		.nDiv = 2,
		.nPeriod = 5208,
	},
	{
		.nFreq = 200000,
		.nClockSource = 3,
		.nPMS = PMS(3, 100, 0),
		.nDiv = 2,
		.nPeriod = 5000,
	},
#endif
};
#endif

extern unsigned int IO_ARM_ChangeStackSRAM(void);
extern void IO_ARM_RestoreStackSRAM(unsigned int);

typedef void (*lpfunc)(ddr_config_t *);
static lpfunc lpSelfRefresh;

#define SRAM_COPY_ADDR				0xF0800000
#define SRAM_COPY_FUNC_SIZE			0x600

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

static void tcc_mem_set_clock(ddr_config_t *mem_cfg)
{
	ddr_config_t cfg;
//	int i;

	// it should be copied from SDRAM to SRAM because SDRAM will be unavailable
	_memcpy(&cfg, mem_cfg, sizeof(ddr_config_t));

	*(volatile unsigned long *)0xF0301004 = 0x00000003; 		// PL341_PAUSE
	while (((*(volatile unsigned long *)0xF0301000) & 0x3)!=2); //Wait PL34X_STATUS_PAUSED

	*(volatile unsigned long *)0xF0301004 = 0x00000004; 		// PL341_Configure
	while (((*(volatile unsigned long *)0xF0301000) & 0x3)!=0); //Wait PL34X_STATUS_CONFIG

// DLL OFF
	*(volatile unsigned long *)0xF0304404 &=  ~(0x00000003);	// DLL-0FF,DLL-Stop running
	*(volatile unsigned long *)0xF0304428 &= ~(0x00000003); 	// Calibration Start,Update Calibration
	*(volatile unsigned long *)0xF030302C &=  ~(0x00004000);	//SDRAM IO Control Register Gatein Signal Power Down

#ifdef TCC_LPDDR_USE_DYNAMIC_PLL_FREQ
	// mddr wait time
//	for (i = 10 ; i > 0 ; i --);

//	for (i=0;i<100;i++)	HwPL340->direct_cmd = (1<<18);	// refresh command

// 1. Set temporally memory clock
	*(volatile unsigned long *)0xF0400008 = (0x00200000 | (3 << 4) | 2); // CKC-CLKCTRL2 - Mem

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
	*(volatile unsigned long *)0xF0400008 = (0x00200000 | ((cfg.nDiv-1) << 4) | 3); // CKC-CLKCTRL2 - Mem

//	for (i=0;i<100;i++)	HwPL340->direct_cmd = (1<<18);	// refresh command
#else
	*(volatile unsigned long *)0xF0400008 = (0x00200000 | ((cfg.nDiv-1) << 4) | cfg.nClockSource); // CKC-CLKCTRL2 - Mem
#endif /* TCC_LPDDR_USE_DYNAMIC_PLL_FREQ */

//Init DDR2 
#if defined(CONFIG_MDDR_EHD013091MA60)
	*(volatile unsigned long *)0xF030100C = 0x00210012; 		// config0 cas 10bit, ras 13bit , AP bit 10, Burst 4, 2chips
#else
	*(volatile unsigned long *)0xF030100C = 0x0021001a;//0x00210012; 		// config0 cas 10bit, ras 13bit , AP bit 10, Burst 4, 2chips
#endif

	*(volatile unsigned long *) 0xF0303000 |= 0x00800000;		// bit23 enable -synopt enable
	*(volatile unsigned long *) 0xF0303010 |= 0x00800000;		// bit23 enable -synopt enable

#if defined(CONFIG_MDDR_EHD013091MA60)
	*(volatile unsigned long *)0xF030104C= 0x000006D1;
#else
	*(volatile unsigned long *)0xF030104C= 0x000002D1;
#endif

	*(volatile unsigned long *)0xF0301010 = 0x000003E8; // refresh_prd = 1000

	HwPL340->cas_latency = cfg.nCL;

	HwPL340->tMRD = cfg.tMRD;
	HwPL340->tRAS = cfg.tRAS;
	HwPL340->tRC  = cfg.tRC;
	HwPL340->tRCD = ((cfg.tRCD - 3) << 3) | cfg.tRCD;
	HwPL340->tRFC = ((cfg.tRFC - 3) << 5) | cfg.tRFC;
	HwPL340->tRP  = ((cfg.tRP - 3) << 3) | cfg.tRP;
	HwPL340->tRRD = cfg.tRRD;
	HwPL340->tWR  = cfg.tWR;
	HwPL340->tWTR = cfg.tWTR;
	HwPL340->tXP  = cfg.tXP;
	HwPL340->tXSR = cfg.tXSR;
	HwPL340->tESR = cfg.tESR;

#if defined(CONFIG_TCC_MEM_128M)
	HwPL340->uCHIP0.nReg = 0
		| (0xf8 << 0)		/* Address Mask */
		| (0x40 << 8)		/* Address Match */
		| (0 << 16)		/* brc_n_rbc */
		;
#else
	HwPL340->uCHIP0.nReg = 0
		| (0xf0 << 0)		/* Address Mask */
		| (0x40 << 8)		/* Address Match */
		| (0 << 16)		/* brc_n_rbc */
		;
#endif

	// add by SangWon
//	*(volatile unsigned long *)0xF030302C = 0x3FFF;

	*(volatile unsigned long *)0xF0304428 =  (3 << 17)	// PRD_CAL
						| (0 << 16)	// PRD_CEN
#ifdef CONFIG_MDDR_EHD013091MA60
						| (3 << 13)	// DRV_STR
#else
						| (7 << 13)	// DRV_STR
#endif
						| (1 << 12)	// TERM_DIS
						| (2 << 9)	// ODT(PHY) value
						| (5 << 6)	// PULL UP
						| (2 << 3)	// PULL DOWN
						| (0 << 2)	// ZQ
						| (0 << 1)	// UPDATE
						| (1 << 0);	// CAL_START
	while (!((*(volatile unsigned long *)0xF030442c) & (1)));	// Wait until Calibration completion without error
	*(volatile unsigned long *)0xF0304428 =  (3 << 17)	// PRD_CAL
						| (0 << 16)	// PRD_CEN
#ifdef CONFIG_MDDR_EHD013091MA60
						| (3 << 13)	// DRV_STR
#else
						| (7 << 13)	// DRV_STR
#endif
						| (1 << 12)	// TERM_DIS
						| (2 << 9)	// ODT(PHY) value
						| (5 << 6)	// PULL UP
						| (2 << 3)	// PULL DOWN
						| (0 << 2)	// ZQ
						| (1 << 1)	// UPDATE
						| (1 << 0);	// CAL_START
	*(volatile unsigned long *)0xF0304428 =  (3 << 17)	// PRD_CAL
						| (0 << 16)	// PRD_CEN
#ifdef CONFIG_MDDR_EHD013091MA60
						| (3 << 13)	// DRV_STR
#else
						| (7 << 13)	// DRV_STR
#endif
						| (1 << 12)	// TERM_DIS
						| (2 << 9)	// ODT(PHY) value
						| (5 << 6)	// PULL UP
						| (2 << 3)	// PULL DOWN
						| (0 << 2)	// ZQ
						| (0 << 1)	// UPDATE
						| (1 << 0);	// CAL_START

	*(volatile unsigned long *)0xF0304404 = 0x00000001;	// DLL ON
	*(volatile unsigned long *)0xF0304404 = 0x00000003;

	*(volatile unsigned long *)0xF0301008 = 0x00000032; //MRS
	*(volatile unsigned long *)0xF0301008 = 0x000a0000; //EMRS
	*(volatile unsigned long *)0xF0301008 = 0x00080032;
	*(volatile unsigned long *)0xF0301008 = 0x00040032;	
	*(volatile unsigned long *)0xF0301008 = 0x00040032;	
	*(volatile unsigned long *)0xF0301008 = 0x00040032;	
	*(volatile unsigned long *)0xF0301008 = 0x00040032;	
	*(volatile unsigned long *)0xF0301008 = 0x00040032;	
	*(volatile unsigned long *)0xF0301008 = 0x00040032;	
//Change MEM Source 
	//*(volatile unsigned long *)0xF0400008 = (0x00200000 | ((lmem_div-1) << 4)|lmem_source); // CKC-CLKCTRL2 - Mem

	*(volatile unsigned long *) 0xF0301004=0x00000000; // PL341_GO
	while (((*(volatile unsigned long *)0xF0301000) & (0x03)) != 1);	// Wait until READY
}

static ddr_config_t *get_mem_config(unsigned int freq)
{
	int i;
	ddr_config_t *p = &lpddr_mem_config[0];

	for (i = 0; i < ARRAY_SIZE(lpddr_mem_config); i++) {
		if (p->nFreq >= freq) {
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

#include <mach/tca_lcdc.h>
void tcc_mddr_set_clock(unsigned int freq)
{
	int lcdc0_on = 0, lcdc1_on = 0;
	volatile PLCDC	pLCDC_BASE0 = (volatile PLCDC)tcc_p2v(HwLCDC0_BASE);
	volatile PLCDC	pLCDC_BASE1 = (volatile PLCDC)tcc_p2v(HwLCDC1_BASE);
#if defined(CONFIG_GENERIC_TIME)
	volatile PPIC	pPIC	= (volatile PPIC)tcc_p2v(HwPIC_BASE);
#else
	volatile PTIMER	pTIMER	= (volatile PTIMER)tcc_p2v(HwTMR_BASE);
#endif

#ifdef TCC_LPDDR_USE_DYNAMIC_PLL_FREQ
	lcdc0_on = DEV_LCDC_Wait_signal(0);
	lcdc1_on = DEV_LCDC_Wait_signal(1);
#endif

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
EXPORT_SYMBOL(tcc_mddr_set_clock);

static int __init tcc_mddr_init(void)
{	
	int i;
	unsigned int period;
	ddr_config_t *mem_cfg;

	for (i = 0; i < ARRAY_SIZE(lpddr_mem_config); i++) {
		mem_cfg = &lpddr_mem_config[i];
		period = mem_cfg->nPeriod;

#if defined(CONFIG_MDDR_K4X1G163PE)
		mem_cfg->nCW = 2;
		mem_cfg->nRW = 3;
		mem_cfg->nCL = 3;	/* CAS latency */
		mem_cfg->nBurstLength = 2;
		mem_cfg->nGateDelay = 3;
		mem_cfg->nReadDelay = 3;
		mem_cfg->tMRD = 2;	/* cycle number (min. 2 cycles) */
		mem_cfg->tWTR = 1;	/* cycle number (min. 3+BL/2, BL=4) */
		mem_cfg->tXP = 2;	/* cycle number (min. 1 cycles) */
		mem_cfg->tESR = 100;	/* cycle number (min. 200 cycles) */
		mem_cfg->tRAS = 40000;	/* pico second (42 - 120,000ns) */
		mem_cfg->tRC = 55000;	/* pico second (min. 60ns) */
		mem_cfg->tRCD = 20000;	/* pico second (min. 16.2ns) */
		mem_cfg->tRFC = 80000;	/* pico second (min. 96ns) */
		mem_cfg->tRP = 15000;	/* pico second (min. 18ns) */
		mem_cfg->tRRD = 10000;	/* pico second (min. 12ns) */
		mem_cfg->tWR = 12000;	/* pico second (min. 15ns) */
		mem_cfg->tXSR = 120000;	/* pico second (min. 120ns) */
		mem_cfg->tREF_CYC = 2000;
#elif defined(CONFIG_MDDR_H5MS1G62MFP)
		mem_cfg->nCW = 2;
		mem_cfg->nRW = 3;
		mem_cfg->nCL = 3;	/* CAS latency */
		mem_cfg->nBurstLength = 2;
		mem_cfg->nGateDelay = 3;
		mem_cfg->nReadDelay = 1;
		mem_cfg->tMRD = 2;	/* cycle number (min. 2 cycles) */
		mem_cfg->tWTR = 1;	/* cycle number (min. 3+BL/2, BL=4) */
		mem_cfg->tXP = 2;	/* cycle number (min. 1 cycles) */
		mem_cfg->tESR = 50;	/* cycle number (min. 200 cycles) */
		mem_cfg->tRAS = 42000;	/* pico second (42 - 120,000ns) */
		mem_cfg->tRC = 60000;	/* pico second (min. 60ns) */
		mem_cfg->tRCD = 18000;	/* pico second (min. 16.2ns) */
		mem_cfg->tRFC = 110000;	/* pico second (min. 96ns) */
		mem_cfg->tRP = 18000;	/* pico second (min. 18ns) */
		mem_cfg->tRRD = 12000;	/* pico second (min. 12ns) */
		mem_cfg->tWR = 15000;	/* pico second (min. 15ns) */
		mem_cfg->tXSR = 140000;	/* pico second (min. 120ns) */
		mem_cfg->tREF_CYC = 1000;
#elif defined(CONFIG_MDDR_EHD013091MA60)
		mem_cfg->nCW = 2;
		mem_cfg->nRW = 3;
		mem_cfg->nCL = 3;	/* CAS latency */
		mem_cfg->nBurstLength = 2;
		mem_cfg->nGateDelay = 3;
		mem_cfg->nReadDelay = 1;
		mem_cfg->tMRD = 2;	/* cycle number (min. 2 cycles) */
		mem_cfg->tWTR = 1;	/* cycle number (min. 3+BL/2, BL=4) */
		mem_cfg->tXP = 2;	/* cycle number (min. 1 cycles) */
		mem_cfg->tESR = 0xC8;	/* cycle number (min. 200 cycles) */
		mem_cfg->tRAS = 42000;	/* pico second (42 - 120,000ns) */
		mem_cfg->tRC = 60000;	/* pico second (min. 60ns) */
		mem_cfg->tRCD = 18000;	/* pico second (min. 16.2ns) */
		mem_cfg->tRFC = 96000;	/* pico second (min. 96ns) */
		mem_cfg->tRP = 18000;	/* pico second (min. 18ns) */
		mem_cfg->tRRD = 12000;	/* pico second (min. 12ns) */
		mem_cfg->tWR = 15000;	/* pico second (min. 15ns) */
		mem_cfg->tXSR = 120000;	/* pico second (min. 120ns) */
		mem_cfg->tREF_CYC = 1000;
#elif defined(CONFIG_MDDR_MT46H64M32LF)
                mem_cfg->nCW = 2;
                mem_cfg->nRW = 3;
                mem_cfg->nCL = 3;       /* CAS latency */
                mem_cfg->nBurstLength = 2;
                mem_cfg->nGateDelay = 3;
                mem_cfg->nReadDelay = 1;
                mem_cfg->tMRD = 3;      /* cycle number (min. 2 cycles) */
                mem_cfg->tWTR = 2;      /* cycle number (min. 3+BL/2, BL=4) */
                mem_cfg->tXP = 3;       /* cycle number (min. 1 cycles) */
                mem_cfg->tESR = 0x33;   /* cycle number (min. 200 cycles) */
                mem_cfg->tRAS = 42000;  /* pico second (42 - 120,000ns) */
                mem_cfg->tRC = 60000;   /* pico second (min. 60ns) */
                mem_cfg->tRCD = 18000;  /* pico second (min. 16.2ns) */
                mem_cfg->tRFC = 96000;  /* pico second (min. 96ns) */
                mem_cfg->tRP = 18000;   /* pico second (min. 18ns) */
                mem_cfg->tRRD = 12000;  /* pico second (min. 12ns) */
                mem_cfg->tWR = 15000;   /* pico second (min. 15ns) */
                mem_cfg->tXSR = 120000; /* pico second (min. 120ns) */
                mem_cfg->tREF_CYC = 2000;
#else
#error Unknown memory type!
#endif

		mem_cfg->nCL  = (mem_cfg->nCL > 0x7) ? 0x7 : mem_cfg->nCL;
        mem_cfg->nCL <<= 1;
		mem_cfg->tMRD = (mem_cfg->tMRD > 0x7f) ? 0x7f : mem_cfg->tMRD;
		mem_cfg->tRAS = ddr_time_to_cycle(period, mem_cfg->tRAS, 0x0, 0xf);
		mem_cfg->tRC  = ddr_time_to_cycle(period, mem_cfg->tRC, 0x0, 0xf);
		mem_cfg->tRCD = ddr_time_to_cycle(period, mem_cfg->tRCD, 0x4, 0x7);
		mem_cfg->tRFC = ddr_time_to_cycle(period, mem_cfg->tRFC, 0x4, 0x1f);
		mem_cfg->tRP  = ddr_time_to_cycle(period, mem_cfg->tRP, 0x4, 0x7);
		mem_cfg->tRRD = ddr_time_to_cycle(period, mem_cfg->tRRD, 0x0, 0xf);
		mem_cfg->tWR  = ddr_time_to_cycle(period, mem_cfg->tWR, 0x0, 0x7);
		mem_cfg->tWTR = (mem_cfg->tWTR > 0x7) ? 0x7 : mem_cfg->tWTR;
		mem_cfg->tXP  = (mem_cfg->tXP > 0xff) ? 0xff : mem_cfg->tXP;
		mem_cfg->tXSR = ddr_time_to_cycle(period, mem_cfg->tXSR, 0, 0xff);
		mem_cfg->tESR = (mem_cfg->tESR > 0xff) ? 0xff : mem_cfg->tESR;
		mem_cfg->tREF_CYC = (mem_cfg->tREF_CYC > 0x7fff) ? 0x7fff : mem_cfg->tREF_CYC;
	}
	return 0;
}

__initcall(tcc_mddr_init);
