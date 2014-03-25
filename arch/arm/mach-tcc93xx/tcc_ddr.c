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
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <mach/hardware.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <linux/spinlock.h>
#include <linux/ioctl.h>
#include <linux/sched.h>
#include <linux/tick.h>
#include <linux/time.h>
#include <mach/bsp.h>
#include <linux/tcc_ioctl.h>
#include <mach/tcc_ddr.h>



/*===========================================================================

                  DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/

extern unsigned int IO_ARM_ChangeStackSRAM(void);
extern void IO_ARM_RestoreStackSRAM(unsigned int);
static unsigned int get_cycle(unsigned int tCK, unsigned int ps, unsigned int ck);

#define time2cycle(time, tCK)		((int)((time + tCK - 1) / tCK))

#define iomap_p2v(x)            io_p2v(x)

#define addr(x) (0xF0000000+x)

typedef void (*FuncPtr)(unsigned int arg);




/*===========================================================================

                                 MAP

===========================================================================*/

/*---------------------------------------------------------------------------
 1) TCC93xx
     << sram1 >>
     0xD0000000(0xEFE00000) ------------------
                           | ckc change func  | 0x500
                     0x500  ------------------
                           | ckc change arg   | 0x80
                     0x580  ------------------
                           |      Stack       | 0x80
                     0x600  ------------------

---------------------------------------------------------------------------*/

#define CKC_CHANGE_FUNC_ADDR       0xEFE00000

#define CKC_CHANGE_FUNC_SIZE       0x500

#define CKC_CHANGE_ARG_BASE        0xEFE00500



/*===========================================================================

                          Clock Register Value

===========================================================================*/

typedef struct {
	unsigned int	pll;
	unsigned int	clkctrl;   
	unsigned int	freq;
} MEMCLK;

#ifdef CONFIG_MEM_CLK_SYNC_MODE
#define PMS(P, M, S, V)	((V)<<30 | (S)<<24 | (M)<<8 | (P))
#endif

MEMCLK pIO_CKC_PLL[]	=
{
#ifdef CONFIG_MEM_CLK_SYNC_MODE
	{PMS(3, 252, 2, 0), 0x002FFFF0, 1260000},
	{PMS(3, 260, 2, 0), 0x002FFFF0, 1300000},
	{PMS(3, 276, 2, 0), 0x002FFFF0, 1380000},
	{PMS(3, 282, 2, 0), 0x002FFFF0, 1410000},
	{PMS(3, 290, 2, 0), 0x002FFFF0, 1450000},
	{PMS(3, 304, 2, 0), 0x002FFFF0, 1520000},
	{PMS(3, 320, 2, 0), 0x002FFFF0, 1600000},
	{PMS(3, 340, 2, 0), 0x002FFFF0, 1700000},
	{PMS(3, 360, 2, 1), 0x002FFFF0, 1800000},
	{PMS(3, 380, 2, 1), 0x002FFFF0, 1900000},
	{PMS(3, 400, 2, 1), 0x002FFFF0, 2000000},
	{PMS(3, 420, 2, 1), 0x002FFFF0, 2100000},
	{PMS(3, 440, 2, 1), 0x002FFFF0, 2200000},
	{PMS(3, 460, 2, 1), 0x002FFFF0, 2300000},
	{PMS(3, 480, 2, 1), 0x002FFFF0, 2400000},
	{PMS(3, 500, 2, 1), 0x002FFFF0, 2500000},
	{PMS(3, 260, 1, 0), 0x002FFFF0, 2600000},
	{PMS(3, 270, 1, 0), 0x002FFFF0, 2700000},
	{PMS(3, 280, 1, 0), 0x002FFFF0, 2800000},
	{PMS(3, 290, 1, 0), 0x002FFFF0, 2900000},
	{PMS(3, 300, 1, 0), 0x002FFFF0, 3000000},
	{PMS(3, 310, 1, 0), 0x002FFFF0, 3100000},
	{PMS(3, 320, 1, 0), 0x002FFFF0, 3200000},
	{PMS(3, 330, 1, 0), 0x002FFFF0, 3300000},
	{PMS(3, 340, 1, 0), 0x002FFFF0, 3400000},
	{PMS(3, 350, 1, 0), 0x002FFFF0, 3500000},
	{PMS(3, 360, 1, 1), 0x002FFFF0, 3600000},
	{PMS(3, 370, 1, 1), 0x002FFFF0, 3700000},
	{PMS(3, 380, 1, 1), 0x002FFFF0, 3800000},
	{PMS(3, 390, 1, 1), 0x002FFFF0, 3900000},
	{PMS(3, 400, 1, 1), 0x002FFFF0, 4000000},
#else
	{0x41007E03, 0x00200013, 1260000},
	{0x41008203, 0x00200013, 1300000},
	{0x41008A03, 0x00200013, 1380000},
	{0x41008D03, 0x00200013, 1410000},
	{0x41009103, 0x00200013, 1450000},
	{0x41009803, 0x00200013, 1520000},
	{0x4100A003, 0x00200013, 1600000},
	{0x00005503, 0x00200013, 1700000},
	{0x00005A03, 0x00200013, 1800000},
	{0x00005F03, 0x00200013, 1900000},
	{0x00006403, 0x00200013, 2000000},
	{0x00006903, 0x00200013, 2100000},
	{0x00006E03, 0x00200013, 2200000},
	{0x00007303, 0x00200013, 2300000},
	{0x40007803, 0x00200013, 2400000},
	{0x40007D03, 0x00200013, 2500000},
	{0x40008203, 0x00200013, 2600000},
	{0x40008703, 0x00200013, 2700000},
	{0x40008C03, 0x00200013, 2800000},
	{0x40009103, 0x00200013, 2900000},
	{0x40009603, 0x00200013, 3000000},
	{0x40009B03, 0x00200013, 3100000},
	{0x4000A003, 0x00200013, 3200000},
	{0x4000A503, 0x00200013, 3300000},
	{0x4000AA03, 0x00200013, 3400000},
	{0x4000AF03, 0x00200013, 3500000},
	{0x4000B403, 0x00200013, 3600000},
	{0x4000B903, 0x00200013, 3700000},
	{0x4000BE03, 0x00200013, 3800000},
	{0x4000C303, 0x00200013, 3900000},
	{0x4000C803, 0x00200013, 4000000},
#endif
};



/*===========================================================================

                          Clock Change Argument

===========================================================================*/

#define CKC_CHANGE_ARG(x)   (*(volatile unsigned long *)(CKC_CHANGE_ARG_BASE + (4*(x))))

#if defined(CONFIG_DRAM_DDR3)
	enum {
		//Clock
		PLL_VALUE = 0,
		CKC_CTRL_VALUE,
		T_REFI, /* 0 <= t_refi < 2^17 */
		T_RFC, /* 0 <= t_rfc < 2^8 */
		T_RRD, /* 0 <= t_rrd < 2^4 */
		T_RP, /* 0 <= t_rp < 2^4 */
		T_RCD, /* 0 <= t_rcd < 2^4 */ 
		T_RC, /* 0 <= t_rc < 2^6 */ 
		T_RAS, /* 0 <= t_ras < 2^6 */ 
		T_WTR, /* 0 <= t_wtr < 2^4 */ 
		T_WR, /* 0 <= t_wr < 2^4 */ 
		T_RTP, /* 0 <= t_rtp < 2^4 */ 
		CL, /* 0 <= cl < 2^4 */ 
		WL, /* 0 <= wl < 2^4 */ 	
		RL, /* 0 <= rl < 2^4 */ 	
		T_FAW, /* 0 <= t_faw < 2^6 */ 	 
		T_XSR, /* 0 <= t_xsr < 2^11 */ 	 
		T_XP, /* 0 <= t_xp < 2^11 */ 	 
		T_CKE, /* 0 <= t_cke < 2^4 */
		T_MRD, /* 0 <= t_mrd < 2^4 */
		MR0_DDR3,
		MR1_DDR3,
		MR2_DDR3,
		MR3_DDR3,
		#ifndef MRS_ALWAYS_SETTING
		MRS_SET
		#endif
	};
#elif defined(CONFIG_DRAM_DDR2)
	enum {
		PLL_VALUE = 0,
		CKC_CTRL_VALUE,
		T_REFI,
		TROW,
		TDATA,
		TPOWER,
		T_WR,
		CL,
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
	unsigned int nRFC, nRRD, nRP, nRCD, nRC, nRAS;
	unsigned int nWTR, nWR, nRTP, nCL, nWL, nRL;
	unsigned int nFAW, nXSR, nXP, nCKE, nMRD;


	tCK = 1000000/mem_freq; // tCK is pico sec unit

	//nCL = nRCD = nRP
	nCL = DDR2_CL;//((DDR2_CL > 5) ? 5 : DDR2_CL);

	CKC_CHANGE_ARG(CL) = nCL;
	CKC_CHANGE_ARG(T_REFI) = get_cycle(tCK, DDR2_tREFI_ps, 1);

	nRFC = get_cycle(tCK, DDR2_tRFC_ps, 1);

	nRRD = get_cycle(tCK, DDR2_tRRD_ps, DDR2_tRRD_ck);
	nRP  = nCL; //get_cycle(tCK, DDR2_tRP_ps,  DDR2_tRP_ck);
	nRCD = nCL; //get_cycle(tCK, DDR2_tRCD_ps, DDR2_tRCD_ck);
	nRC  = get_cycle(tCK, DDR2_tRC_ps,  DDR2_tRC_ck);
	nRAS = get_cycle(tCK, DDR2_tRAS_ps, DDR2_tRAS_ck);

	CKC_CHANGE_ARG(TROW) = (nRFC<<24)|(nRRD<<20)|(nRP<<16)|(nRCD<<12)|(nRC<<6)|(nRAS);

	nWTR = get_cycle(tCK, DDR2_tWTR_ps, DDR2_tWTR_ck);
	nWR  = get_cycle(tCK, DDR2_tWR_ps, DDR2_tWR_ck);
	nRTP = get_cycle(tCK, DDR2_tRTP_ps, DDR2_tRTP_ck);
	nWL  = nCL -1;
	nRL  = nCL;
	CKC_CHANGE_ARG(TDATA) = (nWTR<<28)|(nWR<<24)|(nRTP<<20)|(nCL<<16)|(nWL<<8)|(nRL);

	nFAW = get_cycle(tCK, DDR2_tFAW_ps, DDR2_tFAW_ck);
	nXSR = DDR2_tXSR_ck;
	nXP  = DDR2_tXP_ck;
	nCKE = DDR2_tCKE_ck;
	nMRD = DDR2_tMRD_ck;
	CKC_CHANGE_ARG(TPOWER) = (nFAW<<24)|(nXSR<<16)|(nXP<<8)|(nCKE<<4)|(nMRD);

	CKC_CHANGE_ARG(T_WR) = nWR;

//	printk("T_REFI:0x%x, TROW:0x%x, TDATA:0x%x, TPOWER:0x%x, T_WR:0x%x\n",CKC_CHANGE_ARG(T_REFI),CKC_CHANGE_ARG(TROW),CKC_CHANGE_ARG(TDATA),CKC_CHANGE_ARG(TPOWER),CKC_CHANGE_ARG(T_WR));

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

	*(volatile unsigned long *)addr(0x303020 ) &= ~Hw17;

	if(DDR2_BANK_NUM == 8)
		while (((*(volatile unsigned long *)addr(0x305048)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
	else//DDR2_BANK_NUM == 4
		while (((*(volatile unsigned long *)addr(0x305048)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED

	if(DDR2_LOGICAL_CHIP_NUM == 2){
		if(DDR2_BANK_NUM == 8)
			while (((*(volatile unsigned long *)addr(0x30504C)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
		else//DDR2_BANK_NUM == 4
			while (((*(volatile unsigned long *)addr(0x30504C)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED
	}

//--------------------------------------------------------------------------
// Clock setting..

#ifdef CONFIG_MEM_CLK_SYNC_MODE
	*(volatile unsigned long *)addr(0x500000) = 0x002FFFF1; //cpu - DirectPLL1 : 384MHz
	*(volatile unsigned long *)addr(0x500008) = 0x00200021; //mem bus - DirectPLL1 : 384MHz/3
	*(volatile unsigned long *)addr(0x50001C) = 0x00200021; //smu
	*(volatile unsigned long *)addr(0x50002C) = 0xFFFF0100; //MBUSCTRL - set synchronous clock mode! cpubus/2

	//set pll1 value
	*(volatile unsigned long *)addr(0x500030) = CKC_CHANGE_ARG(PLL_VALUE); // PLL_PWR_OFF & SET PMS
	*(volatile unsigned long *)addr(0x500030) |= 0x80000000; // PLL_PWR_ON

	for (i = 100; i > 0; i --);	// Wait, PLL 안정화 시간이 50이 한계라서 100으로 설정.

	*(volatile unsigned long *)addr(0x500000) = CKC_CHANGE_ARG(CKC_CTRL_VALUE);
	*(volatile unsigned long *)addr(0x50001C) = 0x00200030; //smu
#else
	//mem bus - DirectPLL2/3(It is for asynchronous clock mode)
	*(volatile unsigned long *)addr(0x500008)  = 0x00200021;

	//MBUSCTRL - set asynchronous clock mode! cpubus/2
	*(volatile unsigned long *)addr(0x50002C)  = 0xFFFF0101;

	//set pll1 value
	*(volatile unsigned long *)addr(0x50003C)  = CKC_CHANGE_ARG(PLL_VALUE); // PLL_PWR_OFF & SET PMS
	*(volatile unsigned long *)addr(0x50003C) |= 0x80000000;			// PLL_PWR_ON

	//*(volatile unsigned long *)addr(0x500008)  = 0x00200013;
	*(volatile unsigned long *)addr(0x500008) = CKC_CHANGE_ARG(CKC_CTRL_VALUE);
#endif

//--------------------------------------------------------------------------
// Controller setting

	//phy configuration
	*(volatile unsigned long *)addr(0x303024 ) = 0x200;//PHYCFG

	//PhyZQControl
	if (mem_freq >= 200) {
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL ;
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL | Hw1 ;//zq start
	} else {
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL | Hw0;
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL | Hw1 | Hw0 ;//zq start
	}
	while (((*(volatile unsigned long *)addr(0x305040)) & (0x10000)) != 0x10000);	// Wait until ZQ End

	if (mem_freq >= 200) {
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

	*(volatile unsigned long *)addr(0x305030 ) = CKC_CHANGE_ARG(T_REFI);
	*(volatile unsigned long *)addr(0x305034 ) = CKC_CHANGE_ARG(TROW);
	*(volatile unsigned long *)addr(0x305038 ) = CKC_CHANGE_ARG(TDATA);
	*(volatile unsigned long *)addr(0x30503C ) = CKC_CHANGE_ARG(TPOWER);

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
	*(volatile unsigned long *)addr(0x305010 ) = (DDR2_BURST_LEN|(DDR2_READ_BURST_TYPE<<3)|(CKC_CHANGE_ARG(CL)<<4)|((CKC_CHANGE_ARG(T_WR)-1)<<9));	// BurstLength 4, CL , WR 
	i = 20; while(i--);
	*(volatile unsigned long *)addr(0x305010 ) = 0x00010380;	// OCD Calibration default
	i = 20; while(i--);
	*(volatile unsigned long *)addr(0x305010 ) = 0x00010004 | DDR2_DRIVE_STRENGTH;	// OCD Calibration default

	if(DDR2_LOGICAL_CHIP_NUM == 2){
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
		*(volatile unsigned long *)addr(0x305010 ) = (DDR2_BURST_LEN|(DDR2_READ_BURST_TYPE<<3)|(CKC_CHANGE_ARG(CL)<<4)|((CKC_CHANGE_ARG(T_WR)-1)<<9)) | Hw20;	// BurstLength 4, CL , WR 
		i = 20; while(i--);
		*(volatile unsigned long *)addr(0x305010 ) = 0x00010380 | Hw20;	// OCD Calibration default
		i = 20; while(i--);
		*(volatile unsigned long *)addr(0x305010 ) = 0x00010004 | Hw20 | DDR2_DRIVE_STRENGTH;	// OCD Calibration default
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
}

#elif defined(CONFIG_DRAM_DDR3)

/*===========================================================================

                             DDR3 Setting

===========================================================================*/

/*===========================================================================
VARIABLE
===========================================================================*/

#ifndef MRS_ALWAYS_SETTING
static unsigned int gWR = 0;
static unsigned int gCL = 0;
#endif

/*===========================================================================
FUNCTION
 - mem_freq : MHz unit
===========================================================================*/
static void get_ddr_param(unsigned int mem_freq)
{
	unsigned int tCK = 0, nCL=0, nWR=0, nCWL=0, nAL=0;
	unsigned int arrMRS[4];

//--------------------------------------------------------------------------
// Calculate timing parameters.

	tCK = 1000000/mem_freq; // tCK is pico sec unit

	/* tRP = tRCD = CL */
	//CL, RP, RCD
	if(mem_freq < 400)
		nCL = DDR3_CL*400*2/DDR3_MAX_SPEED;
	else
		nCL = DDR3_CL*mem_freq*2/DDR3_MAX_SPEED;
	if(DDR3_CL*mem_freq%DDR3_MAX_SPEED != 0)
		nCL++;

	nWR = get_cycle(tCK, DDR3_tWR_ps, 1);

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

	if(DDR3_AL == AL_DISABLED)
		nAL = 0;
	else if(DDR3_AL == AL_CL_MINUS_ONE)
		nAL = nCL - 1;
	else if(DDR3_AL == AL_CL_MINUS_TWO)
		nAL = nCL - 2;

	// Set timing parameters.
	CKC_CHANGE_ARG(T_REFI) = get_cycle(tCK, DDR3_tREFI_ps, 1);
	CKC_CHANGE_ARG(T_RFC) = get_cycle(tCK, DDR3_tRFC_ps, 1);
	CKC_CHANGE_ARG(T_RRD) = get_cycle(tCK, DDR3_tRRD_ps, DDR3_tRRD_ck);
	CKC_CHANGE_ARG(T_RP) = nCL;//get_cycle(tCK, DDR3_tRP_ps, DDR3_tRP_ck);
	CKC_CHANGE_ARG(T_RCD) = nCL;//get_cycle(tCK, DDR3_tRCD_ps, DDR3_tRCD_ck);
	CKC_CHANGE_ARG(T_RC) = get_cycle(tCK, DDR3_tRC_ps, DDR3_tRC_ck);
	CKC_CHANGE_ARG(T_RAS) = get_cycle(tCK, DDR3_tRAS_ps, DDR3_tRAS_ck);
	CKC_CHANGE_ARG(T_WTR) = get_cycle(tCK, DDR3_tWTR_ps, DDR3_tWTR_ck);
	CKC_CHANGE_ARG(T_WR) = nWR;
	CKC_CHANGE_ARG(T_RTP) = get_cycle(tCK, DDR3_tRTP_ps, DDR3_tRTP_ck);
	CKC_CHANGE_ARG(CL) = nCL; 
	CKC_CHANGE_ARG(WL) = nAL + nCWL; 
	CKC_CHANGE_ARG(RL) = nAL + nCL;
	CKC_CHANGE_ARG(T_FAW) = get_cycle(tCK, DDR3_tFAW_ps, DDR3_tFAW_ck); 
	CKC_CHANGE_ARG(T_XSR) = get_cycle(tCK, DDR3_tXS_ps, DDR3_tXS_ck);
	CKC_CHANGE_ARG(T_XP) = get_cycle(tCK, DDR3_tXP_ps, DDR3_tXP_ck);
	CKC_CHANGE_ARG(T_CKE) = get_cycle(tCK, DDR3_tCKE_ps, DDR3_tCKE_ck);
	CKC_CHANGE_ARG(T_MRD) = DDR3_tMRD_ck;

#ifndef MRS_ALWAYS_SETTING
	if((nWR == gWR) && (nCL == gCL))
		CKC_CHANGE_ARG(MRS_SET) = 0;
	else
		CKC_CHANGE_ARG(MRS_SET) = 1;
	gWR = nWR; gCL = nCL;
#endif

//--------------------------------------------------------------------------
// Set mode registers

	/* MR0 */
	arrMRS[0] = 0;
	arrMRS[0] |= DDR3_BURST_LEN; // Burst Len
	arrMRS[0] |= DDR3_READ_BURST_TYPE*Hw3; // Read Burst Type

	if(nCL < 5) // CAS Latency
		arrMRS[0] |= (5-4)*Hw4;
	else if(nCL > 11)
		arrMRS[0] |= (11-4)*Hw4;
	else
		arrMRS[0] |= (nCL-4)*Hw4;

	if(nWR <= 5) // Write Recovery for autoprecharge
		arrMRS[0] |= WR_5*Hw9;
	else if(nWR == 6)
		arrMRS[0] |= WR_6*Hw9;
	else if(nWR == 7)
		arrMRS[0] |= WR_7*Hw9;
	else if(nWR == 8)
		arrMRS[0] |= WR_8*Hw9;
	else if(nWR == 9 || nWR == 10)
		arrMRS[0] |= WR_10*Hw9;
	else if(nWR >= 11)
		arrMRS[0] |= WR_12*Hw9;

	arrMRS[0] |= FAST_EXIT*Hw12; // DLL On

	/* MR1 */
	arrMRS[1] = Hw16;//MR1
	
	arrMRS[1] |= DDR3_AL*Hw3;
	//arrMRS[1] |= (Hw12 | Hw11 | Hw6); //Rtt_Nom is RZQ/2, Don't enable "Write leveling enable", tDQS enable
	arrMRS[1] |= (Hw2 | Hw1); //Rtt_Nom is RZQ/4, Output Driver Impedance Control RZQ/7

	/* MR2 */
	arrMRS[2] = Hw17;//MR2
	if(mem_freq*2 <= DDR3_800)
		arrMRS[2] |= 0;
	else if(mem_freq*2 <= DDR3_1066)
		arrMRS[2] |= 1*Hw3;
	else if(mem_freq*2 <= DDR3_1333)
		arrMRS[2] |= 2*Hw3;
	else if(mem_freq*2 <= DDR3_1600)
		arrMRS[2] |= 3*Hw3;	

	//Bruce, 101029, modify according to soc guide 
	//arrMRS[2] |= Hw10; //Rtt_WR is RZQ/2

	/* MR3 */
	arrMRS[3] = Hw17|Hw16;//MR2

	// Set Mode Registers
	CKC_CHANGE_ARG(MR0_DDR3) = arrMRS[0];
	CKC_CHANGE_ARG(MR1_DDR3) = arrMRS[1];
	CKC_CHANGE_ARG(MR2_DDR3) = arrMRS[2];
	CKC_CHANGE_ARG(MR3_DDR3) = arrMRS[3];

//	printk("T_REFI:0x%x, T_RFC:0x%x, T_RRD:0x%x, T_RP:0x%x, T_RCD:0x%x\n",CKC_CHANGE_ARG(T_REFI),CKC_CHANGE_ARG(T_RFC),CKC_CHANGE_ARG(T_RRD),CKC_CHANGE_ARG(T_RP),CKC_CHANGE_ARG(T_RCD));
//	printk("T_RC:0x%x, T_RAS:0x%x, T_WTR:0x%x, T_WR:0x%x, T_RTP:0x%x\n",CKC_CHANGE_ARG(T_RC),CKC_CHANGE_ARG(T_RAS),CKC_CHANGE_ARG(T_WTR),CKC_CHANGE_ARG(T_WR),CKC_CHANGE_ARG(T_RTP));
//	printk("CL:0x%x, WL:0x%x, RL:0x%x\n",CKC_CHANGE_ARG(CL),CKC_CHANGE_ARG(WL),CKC_CHANGE_ARG(RL));
//	printk("T_FAW:0x%x, T_XSR:0x%x, T_XP:0x%x, T_CKE:0x%x, T_MRD:0x%x\n",CKC_CHANGE_ARG(T_FAW),CKC_CHANGE_ARG(T_XSR),CKC_CHANGE_ARG(T_XP),CKC_CHANGE_ARG(T_CKE),CKC_CHANGE_ARG(T_MRD));
//	printk("MR0:0x%x, MR1:0x%x, MR2:0x%x, MR3:0x%x\n", CKC_CHANGE_ARG(MR0_DDR3), CKC_CHANGE_ARG(MR1_DDR3), CKC_CHANGE_ARG(MR2_DDR3), CKC_CHANGE_ARG(MR3_DDR3));
}

/*===========================================================================
FUNCTION
 - mem_freq : MHz unit
===========================================================================*/
static void change_clock(unsigned int mem_freq)
{
	volatile int i;

//--------------------------------------------------------------------------
// Change to config mode

	*(volatile unsigned long *)addr(0x303020 ) = 0x0003010b ;//EMCCFG

	if(DDR3_BANK_NUM == 8)
		while (((*(volatile unsigned long *)addr(0x30c208)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
	else//DDR3_BANK_NUM is 4
		while (((*(volatile unsigned long *)addr(0x30c208)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED

	if(DDR3_LOGICAL_CHIP_NUM == 2){
		if(DDR3_BANK_NUM == 8)
			while (((*(volatile unsigned long *)addr(0x30c20C)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
		else//DDR3_BANK_NUM is 4
			while (((*(volatile unsigned long *)addr(0x30c20C)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED
	}

//--------------------------------------------------------------------------
// Clock setting..

#ifdef CONFIG_MEM_CLK_SYNC_MODE
	*(volatile unsigned long *)addr(0x500000) = 0x002FFFF1; //cpu - DirectPLL1 : 384MHz
	*(volatile unsigned long *)addr(0x500008) = 0x00200021; //mem bus - DirectPLL1 : 384MHz/3
	*(volatile unsigned long *)addr(0x50001C) = 0x00200021; //smu

	*(volatile unsigned long *)addr(0x50002C) = 0xFFFF0100; //MBUSCTRL - set synchronous clock mode! cpubus/2

	//set pll1 value
	*(volatile unsigned long *)addr(0x500030) = CKC_CHANGE_ARG(PLL_VALUE); // PLL_PWR_OFF & SET PMS
	*(volatile unsigned long *)addr(0x500030) |= 0x80000000; // PLL_PWR_ON

	for (i = 100; i > 0; i --);	// Wait, PLL 안정화 시간이 50이 한계라서 100으로 설정.

	*(volatile unsigned long *)addr(0x500000) = CKC_CHANGE_ARG(CKC_CTRL_VALUE);
	*(volatile unsigned long *)addr(0x50001C) = 0x00200030; //smu
#else
	//mem bus - DirectPLL2/3(It is for asynchronous clock mode)
	*(volatile unsigned long *)addr(0x500008)  = 0x00200021;

	//MBUSCTRL - set asynchronous clock mode! cpubus/2
	*(volatile unsigned long *)addr(0x50002C)  = 0xFFFF0101;

	//set pll1 value
	*(volatile unsigned long *)addr(0x50003C)  = CKC_CHANGE_ARG(PLL_VALUE); // PLL_PWR_OFF & SET PMS
	*(volatile unsigned long *)addr(0x50003C) |= 0x80000000;			// PLL_PWR_ON

	//*(volatile unsigned long *)addr(0x500008)  = 0x00200013;
	*(volatile unsigned long *)addr(0x500008) = CKC_CHANGE_ARG(CKC_CTRL_VALUE);
#endif

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
	if(mem_freq >= 333)
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

	*(volatile unsigned long *)addr(0x30C100) = CKC_CHANGE_ARG(T_REFI);
	*(volatile unsigned long *)addr(0x30C104) = CKC_CHANGE_ARG(T_RFC);
	*(volatile unsigned long *)addr(0x30C108) = CKC_CHANGE_ARG(T_RRD);
	*(volatile unsigned long *)addr(0x30C10c) = CKC_CHANGE_ARG(T_RP);
	*(volatile unsigned long *)addr(0x30C110) = CKC_CHANGE_ARG(T_RCD);
	*(volatile unsigned long *)addr(0x30C114) = CKC_CHANGE_ARG(T_RC);
	*(volatile unsigned long *)addr(0x30C118) = CKC_CHANGE_ARG(T_RAS);
	*(volatile unsigned long *)addr(0x30C11c) = CKC_CHANGE_ARG(T_WTR);
	*(volatile unsigned long *)addr(0x30C120) = CKC_CHANGE_ARG(T_WR);
	*(volatile unsigned long *)addr(0x30C124) = CKC_CHANGE_ARG(T_RTP);
	*(volatile unsigned long *)addr(0x30C128) = CKC_CHANGE_ARG(CL);
	*(volatile unsigned long *)addr(0x30C12c) = CKC_CHANGE_ARG(WL);
	*(volatile unsigned long *)addr(0x30C130) = CKC_CHANGE_ARG(RL);
	*(volatile unsigned long *)addr(0x30C134) = CKC_CHANGE_ARG(T_FAW);
	*(volatile unsigned long *)addr(0x30C138) = CKC_CHANGE_ARG(T_XSR);
	*(volatile unsigned long *)addr(0x30C13c) = CKC_CHANGE_ARG(T_XP);
	*(volatile unsigned long *)addr(0x30C140) = CKC_CHANGE_ARG(T_CKE);
	*(volatile unsigned long *)addr(0x30C144) = CKC_CHANGE_ARG(T_MRD);

//--------------------------------------------------------------------------
// MRS Setting

	*(volatile unsigned long *)addr(0x30C010) = 0x08000000 ;//DirectCmd - XSR

	if(DDR3_LOGICAL_CHIP_NUM == 2)
		*(volatile unsigned long *)addr(0x30C010) = 0x08000000 | Hw20;//DirectCmd - XSR

	i = 50; while(i--);

#ifndef MRS_ALWAYS_SETTING
	if(CKC_CHANGE_ARG(MRS_SET))
#endif
	{
		*(volatile unsigned long *)addr(0x30C010) = 0x07000000 ;//DirectCmd - NOP

		*(volatile unsigned long *)addr(0x30C010) = CKC_CHANGE_ARG(MR2_DDR3);//DirectCmd - MRS : MR2
		*(volatile unsigned long *)addr(0x30C010) = CKC_CHANGE_ARG(MR3_DDR3);//DirectCmd - MRS : MR3
		*(volatile unsigned long *)addr(0x30C010) = CKC_CHANGE_ARG(MR1_DDR3);//DirectCmd - MRS : MR1 : AL(0),Rtt_Nom(disable),OIC(RZQ/6) ,DLL(enable)

		//Bruce, 101102, for DLL Reset.
		//*(volatile unsigned long *)addr(0x30C010 ) = CKC_CHANGE_ARG(MR0_DDR3) ;//DirectCmd - MRS : MR0 : DLLPRE(off), WR(5), DLL Reset(Yes), MODE(0), CL(6), BL(8)
		{
			*(volatile unsigned long *)addr(0x30C010) = (CKC_CHANGE_ARG(MR0_DDR3))|0x100;//DirectCmd - MRS : MR0 : DLLPRE(off), WR(), DLL Reset(Yes), MODE(0), CL(), BL(8)
			*(volatile unsigned long *)addr(0x30C010) = 0x01000000;//precharge all
			*(volatile unsigned long *)addr(0x30C010) = 0x05000000;//AREF
			*(volatile unsigned long *)addr(0x30C010) = 0x05000000;//AREF
			*(volatile unsigned long *)addr(0x30C010) = 0x05000000;//AREF
			*(volatile unsigned long *)addr(0x30C010) = CKC_CHANGE_ARG(MR0_DDR3);	// DLL reset release.
			i = 50;	while(i--);
		}
		
		*(volatile unsigned long *)addr(0x30C010) = 0x0a000400 ;//DirectCmd - ZQCL

		if(DDR3_LOGICAL_CHIP_NUM == 2){
			*(volatile unsigned long *)addr(0x30C010) = 0x07000000 | Hw20;//DirectCmd - NOP

			*(volatile unsigned long *)addr(0x30C010) = CKC_CHANGE_ARG(MR2_DDR3) | Hw20;//DirectCmd - MRS : MR2
			*(volatile unsigned long *)addr(0x30C010) = CKC_CHANGE_ARG(MR3_DDR3) | Hw20;//DirectCmd - MRS : MR3
			*(volatile unsigned long *)addr(0x30C010) = CKC_CHANGE_ARG(MR1_DDR3) | Hw20;//DirectCmd - MRS : MR1 : AL(0),Rtt_Nom(disable),OIC(RZQ/6) ,DLL(enable)

			//Bruce, 101102, for DLL Reset.
			//*(volatile unsigned long *)addr(0x30C010 ) = CKC_CHANGE_ARG(MR0_DDR3) | Hw20;//DirectCmd - MRS : MR0 : DLLPRE(off), WR(5), DLL Reset(Yes), MODE(0), CL(6), BL(8)
			{
				*(volatile unsigned long *)addr(0x30C010) = (CKC_CHANGE_ARG(MR0_DDR3))|0x100 | Hw20;//DirectCmd - MRS : MR0 : DLLPRE(off), WR(), DLL Reset(Yes), MODE(0), CL(), BL(8)
				*(volatile unsigned long *)addr(0x30C010) = 0x01000000 | Hw20;//precharge all
				*(volatile unsigned long *)addr(0x30C010) = 0x05000000 | Hw20;//AREF
				*(volatile unsigned long *)addr(0x30C010) = 0x05000000 | Hw20;//AREF
				*(volatile unsigned long *)addr(0x30C010) = 0x05000000 | Hw20;//AREF
				*(volatile unsigned long *)addr(0x30C010) = CKC_CHANGE_ARG(MR0_DDR3) | Hw20;	// DLL reset release.
				i = 50; while(i--);
			}
			
			*(volatile unsigned long *)addr(0x30C010) = 0x0a000400 | Hw20;//DirectCmd - ZQCL
		}
	}

//--------------------------------------------------------------------------

	*(volatile unsigned long *)addr(0x30C000) = 0x60FF3030;//ConControl
	*(volatile unsigned long *)addr(0x303020) = 0x0007010b;//EMCCFG

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
static unsigned int get_cycle(unsigned int tCK, unsigned int ps, unsigned int ck)
{
	unsigned int ret;

	ret = time2cycle(ps, tCK);

	if(ret > ck)
		return ret;
	else
		return ck;
}

/*===========================================================================
FUNCTION
 - index : 0 = lcdc0, 1 = lcdc1
===========================================================================*/
static unsigned char mem_access_disable(unsigned index)
{
	unsigned int loop = 0;
	volatile PLCDC	uiPLCD;
	PPMU ppmu; 
	PCKC pckc;
	PDDICONFIG pddiconfig;

	ppmu = (PPMU)(iomap_p2v((unsigned int)&HwPMU_BASE));
	pckc = (PCKC)(iomap_p2v((unsigned int)&HwCLK_BASE)); 
	pddiconfig = (PDDICONFIG)(iomap_p2v((unsigned int)&HwDDI_CONFIG_BASE));

	if(ISSET(ppmu->PWROFF, PMU_DDIBUS) || !(pckc->CLK1CTRL & 0x00200000))
		return FALSE;

	if(index == 0)
	{
		uiPLCD = (volatile PLCDC)tcc_p2v(HwLCDC0_BASE);

		if(ISSET(pddiconfig->PWDN, HwDDIC_PWDN_LCDC0))
			return FALSE;
	}
	else
	{
		uiPLCD = (volatile PLCDC)tcc_p2v(HwLCDC1_BASE);

		if(ISSET(pddiconfig->PWDN, HwDDIC_PWDN_LCDC1))
			return FALSE;
	}

	if(ISZERO(uiPLCD->LCTRL, HwLCTRL_LEN))
		return FALSE;

	while(TRUE && (loop < 100000)) //check wheather is lcd on
	{
		if((ISSET(uiPLCD->LSTATUS, HwLSTATUS_DEOF))
/*			&& (ISSET(uiPLCD->LSTATUS, HwLSTATUS_DD)) */
		)
			break;
		loop++;
	}

	return TRUE;
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void mem_access_enable(unsigned index)
{
	volatile PLCDC	uiPLCD;

	if(index == 0)
		uiPLCD = (volatile PLCDC)tcc_p2v(HwLCDC0_BASE);
	else
		uiPLCD = (volatile PLCDC)tcc_p2v(HwLCDC1_BASE);

	uiPLCD->LCTRL |= HwLCTRL_LEN;
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
	CKC_CHANGE_ARG(CKC_CTRL_VALUE) = pIO_CKC_PLL[i].clkctrl;

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
void tcc_ddr_set_clock(unsigned int freq)
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

#ifdef CONFIG_MEM_CLK_SYNC_MODE
//	printk("CPU/MEM freq change - freq:%d , pll:0x%x, cpu: 0x%x\n", mem_freq, CKC_CHANGE_ARG(PLL_VALUE), CKC_CHANGE_ARG(CKC_CTRL_VALUE));
#else
//	printk("memory bus freq change - freq:%d , pll:0x%x, ckc:0x%x\n", mem_freq, CKC_CHANGE_ARG(PLL_VALUE), CKC_CHANGE_ARG(CKC_CTRL_VALUE));
#endif

	local_irq_save(flags);
	local_irq_disable();
	local_flush_tlb_all();
	flush_cache_all();
#if defined(CONFIG_GENERIC_TIME)
	pPIC->IEN0 &= ~Hw1;		/* Disable Timer0 interrupt */
#else
	pTIMER->TC32EN &= ~Hw24;
#endif

	lcdc0_on = mem_access_disable(0);
	lcdc1_on = mem_access_disable(1);

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

	if(lcdc0_on) mem_access_enable(0);
	if(lcdc0_on) mem_access_enable(1);
}
/*=========================================================================*/

EXPORT_SYMBOL(tcc_ddr_set_clock);

/*=========================================================================*/

