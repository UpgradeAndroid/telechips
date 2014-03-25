/*
 * linux/arch/arm/mach-tccxxxx/tcc_ddr.c
 *
 * Author:  <linux@telechips.com>
 * Created: November, 2010
 * Description: to change memory bus clock for Telechips chipset
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

/*===========================================================================

                         INCLUDE FILES FOR MODULE

===========================================================================*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <mach/hardware.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <bsp.h>
#include <mach/tcc_ddr.h>


/*===========================================================================

                  DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/

//#define TCC_USE_MEM_360MHZ

extern unsigned int IO_ARM_ChangeStackSRAM(void);
extern void IO_ARM_RestoreStackSRAM(unsigned int);
static unsigned int get_cycle(unsigned int tCK, unsigned int ps, unsigned int ck, unsigned int mask);

#define time2cycle(time, tCK)		((int)((time + tCK - 1) / tCK))

#define addr(x) (0xF0000000+x)

typedef void (*FuncPtr)(unsigned int arg);


/*===========================================================================

                                 MAP

===========================================================================*/

/*---------------------------------------------------------------------------
 1) TCC92xx
     << sram1 >>
     0x10000000(0xF0800000) ------------------
                           | ckc change func  | 0x400
                     0x400  ------------------
                           | ckc change arg   | 0x80
                     0x480  ------------------
                           |      Stack       | 0x80
                     0x500  ------------------

---------------------------------------------------------------------------*/

#define CKC_CHANGE_FUNC_ADDR       0xF0800000
#define CKC_CHANGE_FUNC_SIZE       0x400
#define CKC_CHANGE_ARG_BASE        (CKC_CHANGE_FUNC_ADDR + CKC_CHANGE_FUNC_SIZE)


/*===========================================================================

                          Clock Register Value

===========================================================================*/

typedef struct {
	unsigned int	pll;
	unsigned int	clkctrl;   
	unsigned int	freq;
} MEMCLK;

MEMCLK pIO_CKC_PLL[]	=
{
	{0x01002A01, 0x00200013, 1260000},
	{0x00005303, 0x00200013, 1660000},
	{0x00002101, 0x00200013, 1980000},
	{0x0000C803, 0x00200013, 2000000},
	{0x00002801, 0x00200013, 2400000},
	{0x00009103, 0x00200013, 2900000},
#if defined(CONFIG_VBUS_280MHZ_USE)
	{0x00008C03, 0x00200010, 3250000},
#endif
	{0x00006E02, 0x00200013, 3300000},
#if defined(TCC_USE_MEM_360MHZ)
	{0x00007802, 0x00200013, 3600000},
#endif
};


/*===========================================================================

                          Clock Change Argument

===========================================================================*/

#define CKC_CHANGE_ARG(x)   (*(volatile unsigned long *)(CKC_CHANGE_ARG_BASE + (4*(x))))

#if defined(CONFIG_DRAM_DDR2)
	enum {
		PLL_VALUE = 0,
		CKC_CTRL_VALUE,
		T_REFI,
		N_CL,
		T_MRD,
		T_RAS,
		T_RC,
		T_RCD,
		T_RFC,
		T_RP,
		T_RRD,
		T_WR,
		T_WTR,
		T_XP,
		T_XSR,
		T_ESR,
		T_FAW
	};
#endif



#if defined(CONFIG_DRAM_DDR2)
/*===========================================================================

                             DDR2 Setting

===========================================================================*/

/*===========================================================================
FUNCTION
 - mem_freq : MHz unit
===========================================================================*/
static void get_ddr_param(unsigned int mem_freq)
{
	int tCK = 0;
	unsigned int nRCD, nRFC, nRP, nFAW;

	tCK = 1000000/mem_freq; // tCK is pico sec unit

	nRCD = (DDR2_CL&0x7);
	nRFC = get_cycle(tCK, DDR2_tRFC_ps, 1, 0x7F);
	nRP  = (DDR2_CL&0x7);
	nFAW = get_cycle(tCK, DDR2_tFAW_ps, DDR2_tFAW_ck, 0x1F);
		
	CKC_CHANGE_ARG(T_REFI) = get_cycle(tCK, DDR2_tREFI_ps, 1, 0x7FFF);
	CKC_CHANGE_ARG(N_CL)   = DDR2_CL<<1;
	CKC_CHANGE_ARG(T_MRD)  = DDR2_tMRD_ck;
	CKC_CHANGE_ARG(T_RAS)  = get_cycle(tCK, DDR2_tRAS_ps, DDR2_tRAS_ck, 0x1F);
	CKC_CHANGE_ARG(T_RC)   = get_cycle(tCK, DDR2_tRC_ps,  DDR2_tRC_ck, 0x1F);
	CKC_CHANGE_ARG(T_RCD)  = ((nRCD-3)<<8)|nRCD;
	CKC_CHANGE_ARG(T_RFC)  = ((nRFC-3)<<8)|nRFC;
	CKC_CHANGE_ARG(T_RP)   = ((nRP-3)<<8)|nRP;
	CKC_CHANGE_ARG(T_RRD)  = get_cycle(tCK, DDR2_tRRD_ps, DDR2_tRRD_ck, 0xF);
	CKC_CHANGE_ARG(T_WR)   = get_cycle(tCK, DDR2_tWR_ps, DDR2_tWR_ck, 0x7);
	CKC_CHANGE_ARG(T_WTR)  = get_cycle(tCK, DDR2_tWTR_ps, DDR2_tWTR_ck, 0x7);
	CKC_CHANGE_ARG(T_XP)   = DDR2_tXP_ck;
	CKC_CHANGE_ARG(T_XSR)  = get_cycle(tCK, DDR2_tXSR_ps, 1, 0xFF);;
	CKC_CHANGE_ARG(T_ESR)  = DDR2_tESR_ck;
	CKC_CHANGE_ARG(T_FAW)  = ((nFAW-3)<<8)|nFAW;
}

/*===========================================================================
FUNCTION
 - mem_freq : MHz unit
===========================================================================*/
static void change_clock(unsigned int mem_freq)
{
	volatile unsigned int i = 0;			

//--------------------------------------------------------------------------
// Change to config mode

	*(volatile unsigned long *)addr(0x302004)  = 0x00000003;					// PL341_PAUSE
	while (((*(volatile unsigned long *)addr(0x302000)) & 0x3)!=2);		//Wait PL34X_STATUS_PAUSED

	*(volatile unsigned long *)addr(0x302004)  = 0x00000004;					// PL341_Configure
	while (((*(volatile unsigned long *)addr(0x302000)) & 0x3)!=0);		//Wait PL34X_STATUS_CONFIG

	*(volatile unsigned long *)addr(0x304404) &= ~(0x00000003);				// DLL-0FF,DLL-Stop running
	*(volatile unsigned long *)addr(0x304428) &= ~(0x00000003);				// Calibration Start,Update Calibration
	*(volatile unsigned long *)addr(0x30302C) &= ~(0x00004000);				//SDRAM IO Control Register Gatein Signal Power Down

//--------------------------------------------------------------------------
// Clock setting..

	// Set temporally memory clock
	*(volatile unsigned long *)addr(0x400008) = (0x00200000|(3<<4)|1);		// CKC-CLKCTRL2 - Mem

	// DDI & GPU & VBUS & VPU clock check
	if (((*(volatile unsigned long *)addr(0x400004))&0x07) == 3)
		*(volatile unsigned long *)addr(0x400004) = (((*(volatile unsigned long *)addr(0x400004))&0x00200000)|(1<<4)|2);	// CKC-CLKCTRL1 - DDI
	if (((*(volatile unsigned long *)addr(0x40000C))&0x07) == 3)
		*(volatile unsigned long *)addr(0x40000C) = (((*(volatile unsigned long *)addr(0x40000C))&0x00200000)|(1<<4)|2);	// CKC-CLKCTRL1 - GPU
	if (((*(volatile unsigned long *)addr(0x400014))&0x07) == 3)
		*(volatile unsigned long *)addr(0x400014) = (((*(volatile unsigned long *)addr(0x400014))&0x00200000)|(1<<4)|2);	// CKC-CLKCTRL1 - VBUS
	if (((*(volatile unsigned long *)addr(0x400018))&0x07) == 3)
		*(volatile unsigned long *)addr(0x400018) = (((*(volatile unsigned long *)addr(0x400018))&0x00200000)|(1<<4)|2);	// CKC-CLKCTRL1 - VCOD

	//set pll1 value
	*(volatile unsigned long *)addr(0x40002C)  = CKC_CHANGE_ARG(PLL_VALUE);	// PLL_PWR_OFF & SET PMS
	*(volatile unsigned long *)addr(0x40002C) |= 0x80000000;				// PLL_PWR_ON

	*(volatile unsigned long *)addr(0x400008)  = CKC_CHANGE_ARG(CKC_CTRL_VALUE);

//--------------------------------------------------------------------------
// Controller setting

	//M1CFG
	*(volatile unsigned long *)addr(0x30200C)  = (0x0<<21) |	// ACTCH (1 Chip)
												 (0x5<<18) |	// QOS_MBITS (ARID[8:5]
												 (0x2<<15) |	// MBURST ( Burst 4)
												 (0x0<<14) |	// STOP_MCLK
												 (DDR2_AUTO_PD<<13) |
												 (DDR2_PD_PRD<<7) |
												 ((DDR2_ROWBITS-11)<<3) |
												 (DDR2_COLBITS-8);


	*(volatile unsigned long *)addr(0x302010)  = CKC_CHANGE_ARG(T_REFI);

	//M1CFG2
	*(volatile unsigned long *)addr(0x30204C)  = (0x5<<8) |
												 ((DDR2_PHYSICAL_DATA_BITS/16)<<6) |
												 (DDR2_BANK_BITS << 4) |
												 (0x0 << 3) |
												 (0x0 << 2) |
												 (0x1);	// CLK_CFG

	*(volatile unsigned long *)addr(0x302014)  = CKC_CHANGE_ARG(N_CL);
	*(volatile unsigned long *)addr(0x30201C)  = CKC_CHANGE_ARG(T_MRD);
	*(volatile unsigned long *)addr(0x302020)  = CKC_CHANGE_ARG(T_RAS);
	*(volatile unsigned long *)addr(0x302024)  = CKC_CHANGE_ARG(T_RC);
	*(volatile unsigned long *)addr(0x302028)  = CKC_CHANGE_ARG(T_RCD);
	*(volatile unsigned long *)addr(0x30202C)  = CKC_CHANGE_ARG(T_RFC);
	*(volatile unsigned long *)addr(0x302030)  = CKC_CHANGE_ARG(T_RP);
	*(volatile unsigned long *)addr(0x302034)  = CKC_CHANGE_ARG(T_RRD);
	*(volatile unsigned long *)addr(0x302038)  = CKC_CHANGE_ARG(T_WR);
	*(volatile unsigned long *)addr(0x30203C)  = CKC_CHANGE_ARG(T_WTR);
	*(volatile unsigned long *)addr(0x302040)  = CKC_CHANGE_ARG(T_XP);
	*(volatile unsigned long *)addr(0x302044)  = CKC_CHANGE_ARG(T_XSR);
	*(volatile unsigned long *)addr(0x302048)  = CKC_CHANGE_ARG(T_ESR);
	*(volatile unsigned long *)addr(0x302054)  = CKC_CHANGE_ARG(T_FAW);

	// Chip Configuration Register
	*(volatile unsigned long *)addr(0x302200)  = (0x0<<16) |	// BRC_N_RBC
												 (0x40<<8) |
												 (0xFF-(DDR2_TOTAL_MB_SIZE/(DDR2_LOGICAL_CHIP_NUM*0x10)-1));

	*(volatile unsigned long *)addr(0x30440C)  = DDR2_GATECTRL;				// Gate Control
	*(volatile unsigned long *)addr(0x304420)  = 0x00000000;				// CLK Delay

	// ZQ Calibration
	if (mem_freq < 200) {
		*(volatile unsigned long *)addr(0x304428)  = DDR2_PHYZQCTRL|(1<<12);
		while (!((*(volatile unsigned long *)addr(0x30442c)) & (1)));		// Wait until Calibration completion without error
		*(volatile unsigned long *)addr(0x304428) =  DDR2_PHYZQCTRL|(1<<12)|(1<<1);	// update
		*(volatile unsigned long *)addr(0x304428) =  DDR2_PHYZQCTRL|(1<<12);
	}
	else {
		*(volatile unsigned long *)addr(0x304428)  = DDR2_PHYZQCTRL;
		while (!((*(volatile unsigned long *)addr(0x30442c)) & (1)));		// Wait until Calibration completion without error
		*(volatile unsigned long *)addr(0x304428) =  DDR2_PHYZQCTRL | (1<<1);	// update
		*(volatile unsigned long *)addr(0x304428) =  DDR2_PHYZQCTRL;
	}

	*(volatile unsigned long *)addr(0x304430)  = DDR2_READDELAY;			// Read Delay
	*(volatile unsigned long *)addr(0x30302C) |= 0x00004000;				//SOC1-3	// Gatein signal Power down
	*(volatile unsigned long *)addr(0x304404)  = 0x00000001;		 		// DLLCTRL

#if defined(TCC_USE_MEM_360MHZ)
	if (mem_freq > 333)
		*(volatile unsigned long *)addr(0x304408)  = 0x00001212;				// DLLPDCFG
	else
#endif
	if (mem_freq > 266)
		*(volatile unsigned long *)addr(0x304408)  = 0x00001717;				// DLLPDCFG
#if 0
	else if (mem_freq > 166)
		*(volatile unsigned long *)addr(0x304408)  = 0x00002E2E;				// DLLPDCFG
	else
		*(volatile unsigned long *)addr(0x304408)  = 0x00003E3E;				// DLLPDCFG
#else
	else
		*(volatile unsigned long *)addr(0x304408)  = 0x00002020;				// DLLPDCFG
#endif

	*(volatile unsigned long *)addr(0x304404)  = 0x00000003;				// DLLCTRL
	while (((*(volatile unsigned long *)addr(0x304404)) & (0x00000018)) != (0x00000018));	// Wait DLL Lock





#if (defined(CONFIG_DDR2_MT47H128M8) || defined(CONFIG_DDR2_HXB18T2G160AF))
	*(volatile unsigned long *)addr(0x302008) = 0x000c0000;			// Direct COmmnad Register nop
	*(volatile unsigned long *)addr(0x302008) = 0x00000000;			// Direct COmmnad Register precharge all
	*(volatile unsigned long *)addr(0x302008) = 0x000a0000;			// Direct COmmnad Register EMR2
	*(volatile unsigned long *)addr(0x302008) = 0x000b0000;			// Direct COmmnad Register EMR3
	*(volatile unsigned long *)addr(0x302008) = 0x00090000;			// Direct COmmnad Register EMR1
	*(volatile unsigned long *)addr(0x302008) = 0x00080100;			// Direct Command Register MRS0 DLL Reset
	*(volatile unsigned long *)addr(0x302008) = 0x00000000;			// Direct COmmnad Register precharge all

	*(volatile unsigned long *)addr(0x302008) = 0x00040000;			// Direct COmmnad Register auto refresh
	*(volatile unsigned long *)addr(0x302008) = 0x00040000;			// Direct COmmnad Register 
	*(volatile unsigned long *)addr(0x302008) = 0x00040000;			// Direct COmmnad Register 

	*(volatile unsigned long *)addr(0x302008) = 0x00080000;			// Direct Command Register MRS0 DLL Reset Release
#else
	*(volatile unsigned long *)addr(0x302008) = 0x000c0000;			// Direct COmmnad Register 
	*(volatile unsigned long *)addr(0x302008) = 0x00000000;			// Direct COmmnad Register 
	*(volatile unsigned long *)addr(0x302008) = 0x00040000;			// Direct COmmnad Register 
	*(volatile unsigned long *)addr(0x302008) = 0x00040000;			// Direct COmmnad Register 
	*(volatile unsigned long *)addr(0x302008) = 0x000a0000;			// Direct COmmnad Register 
	*(volatile unsigned long *)addr(0x302008) = 0x000b0000;			// Direct COmmnad Register 
	*(volatile unsigned long *)addr(0x302008) = 0x00090000;			// Direct COmmnad Register	  
	*(volatile unsigned long *)addr(0x302008) = 0x00080902|(DDR2_CL<<4);			// Direct COmmnad Register 
	*(volatile unsigned long *)addr(0x302008) = 0x00000000; 		// Direct COmmnad Register 
	*(volatile unsigned long *)addr(0x302008) = 0x00040000;			// dir_cmd
#endif

	*(volatile unsigned long *)addr(0x302008) = (0x00080002|(DDR2_CL<<4)|((CKC_CHANGE_ARG(T_WR)-1)<<9)); //MRS

	i=100;
	while(i) { i--; }

//	if (mem_freq >= 200) {
		*(volatile unsigned long *)addr(0x302008)  = (0x00090000 | (0x7 << 7)); 	// Direct COmmnad Register EMR1 with Enable OCD
		*(volatile unsigned long *)addr(0x302008)  = (0x00090000 | (0x4)); 		// Direct COmmnad Register EMR1 with ODT 75Ohm
//	}
	
	*(volatile unsigned long *)addr(0x302004)  = 0x00000000;				// PL341_GO
	while (((*(volatile unsigned long *)addr(0x302000)) & 0x3)!=1);		//Wait PL34X_STATUS_GO
}

#else
	#error Not Selected ddr type
#endif



/*===========================================================================

                            Common Functions

===========================================================================*/

/*===========================================================================
FUNCTION
===========================================================================*/
static unsigned int get_cycle(unsigned int tCK, unsigned int ps, unsigned int ck, unsigned int mask)
{
	unsigned int ret;

	ret = time2cycle(ps, tCK);

	if(ret > ck)
		return (ret&mask);
	else
		return (ck&mask);
}


/*===========================================================================
FUNCTION
 - return value : MHz unit
===========================================================================*/
static unsigned int get_membus_ckc(unsigned int mem_freq)
{
	int i = 0;

	for (i=0 ; i<ARRAY_SIZE(pIO_CKC_PLL) ; i++) {
		if (pIO_CKC_PLL[i].freq >= mem_freq)
			break;
	}

	if (i >= ARRAY_SIZE(pIO_CKC_PLL))
		i = (ARRAY_SIZE(pIO_CKC_PLL) - 1);;

	CKC_CHANGE_ARG(PLL_VALUE) = pIO_CKC_PLL[i].pll;
	CKC_CHANGE_ARG(CKC_CTRL_VALUE) = 	pIO_CKC_PLL[i].clkctrl;

	return (pIO_CKC_PLL[i].freq/10000);
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void copy_change_clock(unsigned int mem_freq)
{
	FuncPtr pFunc = (FuncPtr)(CKC_CHANGE_FUNC_ADDR);

	memcpy((void *)CKC_CHANGE_FUNC_ADDR, (void*)change_clock, CKC_CHANGE_FUNC_SIZE);

	pFunc(mem_freq);
}

/*===========================================================================
FUNCTION
===========================================================================*/
#include <mach/tca_lcdc.h>

void tcc_ddr2_set_clock(unsigned int freq)
{
	unsigned long  flags;
	unsigned int   stack;
	unsigned int   mem_freq;
	unsigned int   lcdc0_on = 0, lcdc1_on = 0;

#if defined(CONFIG_GENERIC_TIME)
	volatile PPIC	pPIC	= (volatile PPIC)tcc_p2v(HwPIC_BASE);
#else
	volatile PTIMER	pTIMER	= (volatile PTIMER)tcc_p2v(HwTMR_BASE);
#endif

	mem_freq = get_membus_ckc(freq*10);
	get_ddr_param(mem_freq);

	lcdc0_on = DEV_LCDC_Wait_signal(0);
	lcdc1_on = DEV_LCDC_Wait_signal(1);

	local_irq_save(flags);
	local_irq_disable();
	local_flush_tlb_all();
//	flush_cache_all();	// it spend 1~2milisec. time
#if defined(CONFIG_GENERIC_TIME)
	pPIC->IEN0 &= ~Hw1;		/* Disable Timer0 interrupt */
#else
	pTIMER->TC32EN &= ~Hw24;
#endif


	stack = IO_ARM_ChangeStackSRAM();

	copy_change_clock(mem_freq);

	IO_ARM_RestoreStackSRAM(stack);

#if !defined(CONFIG_GENERIC_TIME)
	pTIMER->TC32EN |= Hw24;
#endif
	local_irq_restore(flags);
#if defined(CONFIG_GENERIC_TIME)
	pPIC->IEN0 |= Hw1;		/* Enable Timer0 interrupt */
#endif

}

EXPORT_SYMBOL(tcc_ddr2_set_clock);

