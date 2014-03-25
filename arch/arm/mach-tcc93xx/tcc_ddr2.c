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

#include <asm/io.h> 

//////////////user configuration start//////////////////////////
#define DRAM_PHY_CAS        (6)
#define DRAM_PHY_BANKBIT    (3)
#define DRAM_PHY_ROWBITS    (13)
#define DRAM_PHY_COLBITS    (10)
#define DRAM_PHY_DATABITS   (16)
#define DRAM_PHY_SIZE       (1024) //Mbit
#define BOARD_DRAM_PHYSICAL_NUM		(2) //실제 상판에 붙은 DRAM 갯수 - size 계산에 필요
#define BOARD_DRAM_LOGICAL_NUM		(1) //컨트롤러가 인식할 DRAM수 - 16*2로 32bit처럼 사용하므로 1
#define BOARD_DRAM_DATABITS (32)
#define DDR_DELAY   (1)
//////////////user configuration end//////////////////////////


#define iomap_p2v(x)            io_p2v(x)
#define addr(b) (0xF0000000+b)

extern unsigned int IO_ARM_ChangeStackSRAM(void);
extern void IO_ARM_RestoreStackSRAM(unsigned int);

typedef void (*lpfunc)(unsigned int);

static lpfunc lpSelfRefresh;

#define SRAM_COPY_ADDR				0xEFF00000	//0xF0800000
#define SRAM_COPY_FUNC_SIZE			0x600

static unsigned int retstack = 0;
static unsigned long flags;

static void int_alldisable(void)
{
	local_irq_save(flags);
	local_irq_disable();
}

static void int_restore(void)
{
	local_irq_restore(flags);
}

#define DDR2_SETRCD(x) ((x)>3 ? (((x-3)<<8)| (x) ) : ((1<<8) | (x)))
#define DDR2_SETRFC(x) ((x)>3 ? (((x-3)<<8)| (x) ) : ((0<<8) | (x)))
#define DDR2_SETRP(x)  ((x)>3 ? (((x-3)<<8)| (x) ) : ((1<<8) | (x)))
#define DDR2_SETFAW(x) ((x)>3 ? (((x-3)<<8)| (x) ) : ((1<<8) | (x)))

#define CKC_CHANGE_ARG_BASE         (SRAM_COPY_ADDR + ( SRAM_COPY_FUNC_SIZE * 4 ))
#define CKC_CHANGE_ARG(x)           (*(volatile unsigned long *)(CKC_CHANGE_ARG_BASE + (4 * (x))))
typedef struct {
    unsigned int        pll1config_reg;
    unsigned int        clkctrl2_reg;
	unsigned int		freq_value;
} MEMCLK;
MEMCLK pIO_CKC_PLL3[]	=
{
	{0x41007E03,0x00200013, 1260000},//L"FMBUS_126Mhz", 
	{0x41008203,0x00200013, 1300000},//L"FMBUS_130Mhz", 
	{0x41008A03,0x00200013, 1380000},//L"FMBUS_138Mhz",
	{0x41008D03,0x00200013, 1410000},//L"FMBUS_141Mhz", 
	{0x41009103,0x00200013, 1450000},//L"FMBUS_145Mhz",
	{0x41009803,0x00200013, 1520000},//L"FMBUS_152Mhz", 
	{0x4100A003,0x00200013, 1600000},//L"FMBUS_160Mhz",
	{0x00005503,0x00200013, 1700000},//L"FMBUS_170Mhz", 
	{0x00005A03,0x00200013, 1800000},//L"FMBUS_180Mhz",
	{0x00005F03,0x00200013, 1900000},//L"FMBUS_190Mhz", 
	{0x00006403,0x00200013, 2000000},//L"FMBUS_200Mhz",
	{0x00006903,0x00200013, 2100000},//L"FMBUS_210Mhz", 
	{0x00006E03,0x00200013, 2200000},//L"FMBUS_220Mhz",
	{0x00007303,0x00200013, 2300000},//L"FMBUS_230Mhz", 
	{0x40007803,0x00200013, 2400000},//L"FMBUS_240Mhz",
	{0x40007D03,0x00200013, 2500000},//L"FMBUS_250Mhz", 
	{0x40008203,0x00200013, 2600000},//L"FMBUS_260Mhz", 
	{0x40008703,0x00200013, 2700000},//L"FMBUS_270Mhz",
	{0x40008C03,0x00200013, 2800000},//L"FMBUS_280Mhz", 
	{0x40009103,0x00200013, 2900000},//L"FMBUS_290Mhz",
	{0x40009603,0x00200013, 3000000},//L"FMBUS_300Mhz", 
	{0x40009C03,0x00200013, 3100000},//L"FMBUS_312Mhz", 
	{0x4000A003,0x00200013, 3200000},//L"FMBUS_320Mhz",
	{0x4000A503,0x00200013, 3300000},//L"FMBUS_330Mhz", 
	{0x4000B403,0x00200013, 3600000},//L"FMBUS_360Mhz",
	{0x4000C803,0x00200013, 4000000},//L"FMBUS_400Mhz",
};
#define NUM_MEMCLK	ARRAY_SIZE(pIO_CKC_PLL3)

enum {
	TCAS = 0,
	TRCD,
	TRP	,
	TRAS,
	TRC	,
	TRFC,
	TESR,
	TRRD,
	TWR	,
	TWTR,
	TXP	,
	TXSR,
	TFAW,
	TREFRESH,
	PLLVALUE_DIS,
	CLK_CTRL2,
	EMR1
};

#define BOARD_DRAM_TOTAL    (DRAM_PHY_SIZE * BOARD_DRAM_PHYSICAL_NUM) / 8// MByte
#define DRAM_AUTOPD_ENABLE (Hw13)
#define DRAM_AUTOPD_PERIOD (7<<7) // must larger than CAS latency
#define DRAM_SET_AUTOPD DRAM_AUTOPD_ENABLE|DRAM_AUTOPD_PERIOD
#define PLLEN (Hw31)

//memory controller configuration
#define MDE_1CS				((DRAM_PHY_ROWBITS - 13)<<2)
#define COMMONREG           (0x00010103 | MDE_1CS)
#define MEMCFG2REG          (((BOARD_DRAM_DATABITS /16-1) << 6) | ((DRAM_PHY_BANKBIT==2 ? 0:3) << 4) | 0x501)
#define QOS_MBITS   5
#define CONFIGREG1          ( ((BOARD_DRAM_LOGICAL_NUM-1)<<21) | (QOS_MBITS<<18) |  DRAM_SET_AUTOPD)
#define MBURST      2
#define CONFIGREG2          (((BOARD_DRAM_LOGICAL_NUM-1)<<21) | (QOS_MBITS<<18) | (MBURST<<15) | ((DRAM_PHY_ROWBITS-11)<<3) |(DRAM_PHY_COLBITS-8) |DRAM_SET_AUTOPD)

//Mode register setting value
#define EMR1REG             (0x00080902|(DRAM_PHY_CAS<<4))
#define EMR2REG             (0x00080802|(DRAM_PHY_CAS<<4))

//calc logic
//#define RFC(x) ( x == 256 ? 75 :(x ==  512 ? 105 :(x == 1024 ? 127.5:195)))
#define RFC(x) ( x == 256 ? 7500 :(x ==  512 ? 10500 :(x == 1024 ? 12750:12750)))
#define DRAM_PHY_RFC        RFC(DRAM_PHY_SIZE)

//ZQ option update
//if auto calibration result is bad, you can use below options to setting right ZQ pullup, pulldown value 
#define DRV_STR		(7) 
#define TERM_DIS	(0)	//ODT function control 0: enable 1: disable
#define TERM_VAL	(2)	//ODT value setting 1:75ohm 2:150ohm 3: 50ohm 4: 35(spec out)

#define CAL_START	(1)	//calibration control 0: manual(with ZQ = 1) 1:auto
#define ZQ			(0)	//ZQ FORCING enable 1:overwrite pullup,pulldown value
#define PULL_DOWN	(6)	//5 or 6 is very good
#define PULL_UP		(0)	//0 or 1 is very good.  over 2 cause aging fail.(we do not guarantee.)

 

#define ZQ_DRV_STR			(DRV_STR<<13)
#define ZQ_TERM_DIS			(TERM_DIS<<12)
#define ZQ_TERM_VAL			(TERM_VAL<<9)
#define ZQ_PULL_DOWN		(PULL_DOWN<<6)
#define ZQ_PULL_UP			(PULL_UP<<3)
#define ZQ_ZQ				(ZQ<<2)
#define ZQ_UPDATE			(1<<1)
#define ZQ_CAL_START		(CAL_START)

void init_clockchange_ddr2(unsigned int nFreq)
{
	volatile unsigned int i = 0;
	unsigned int odt_flag;

	if (nFreq > 200000)
		odt_flag = 0;
	else
		odt_flag = 1;
	
	//memory setting start
//	*(volatile unsigned long *)addr(0x303020)  = 0x00070103;	// EMCCFG

	*(volatile unsigned long *)addr(0x302004)  = 0x00000003;			// PL341_PAUSE
	while (((*(volatile unsigned long *)addr(0x302000)) & 0x3)!=2);	// Wait PL34X_STATUS_PAUSED

	*(volatile unsigned long *)addr(0x302004)  = 0x00000004;			// PL341_Configure
	while (((*(volatile unsigned long *)addr(0x302000)) & 0x3)!=0);	// Wait PL34X_STATUS_CONFIG

#if (0) //def USE_SYNCMODE	//400MHz
	//mem bus - DirectPLL2/3(It is for asynchronous clock mode)
	*(volatile unsigned long *)addr(0x500000)  = 0x002FFFF4;
	*(volatile unsigned long *)addr(0x500010)  = 0x00200014;

	//MBUSCTRL - set asynchronous clock mode! cpubus/2
	*(volatile unsigned long *)addr(0x50002C)  = 0x00800100;

	//set pll1 value
	*(volatile unsigned long *)addr(0x500030)  = 0x41019003;			// PLL_PWR_OFF
	*(volatile unsigned long *)addr(0x500030) |= 0x80000000;			// PLL_PWR_ON

	*(volatile unsigned long *)addr(0x500000)  = 0x002FFFF0;
	*(volatile unsigned long *)addr(0x500010)  = 0x00200030;

#else
	//mem bus - DirectPLL2/3(It is for asynchronous clock mode)
	*(volatile unsigned long *)addr(0x500008)  = 0x00200021;

	//MBUSCTRL - set asynchronous clock mode! cpubus/2
	*(volatile unsigned long *)addr(0x50002C)  = 0xFFFF0101;

	//set pll1 value
	*(volatile unsigned long *)addr(0x50003C)  = 0x40009603;			// PLL_PWR_OFF
	*(volatile unsigned long *)addr(0x50003C)  = CKC_CHANGE_ARG(PLLVALUE_DIS);	// PMS_600M
	*(volatile unsigned long *)addr(0x50003C) |= 0x80000000;			// PLL_PWR_ON

	*(volatile unsigned long *)addr(0x500008)  = 0x00200013;
#endif
	i = 200;
	while(i--);

#if 1
	// DLL Off
	*(volatile unsigned long *)addr(0x304404) &= ~(0x00000003);			// DLL-0FF,DLL-Stop running
	*(volatile unsigned long *)addr(0x304428) &= ~(0x00000003); 		// Calibration Start,Update Calibration
#else
	*(volatile unsigned long *)addr(0x304428)  = 0x00500000;			// ZQCAL
	*(volatile unsigned long *)addr(0x304428)  = 0x00500038;			// ZQCAL
	*(volatile unsigned long *)addr(0x304428)  = 0x005001F8;			// ZQCAL
	*(volatile unsigned long *)addr(0x304428)  = 0x005001F9;			// ZQCAL
	while (((*(volatile unsigned long *)addr(0x30442C)) & 0x1)!=1);		// Wait flock
	*(volatile unsigned long *)addr(0x304428)  = 0x005001F8;			// ZQCAL
#endif

	// timing setting
//	*(volatile unsigned long *)addr(0x302014)  = 0x0000000A;			// CAS
	*(volatile unsigned long *)addr(0x302014)  = CKC_CHANGE_ARG(TCAS);
	*(volatile unsigned long *)addr(0x30201c)  = 2;
	*(volatile unsigned long *)addr(0x302020)  = CKC_CHANGE_ARG(TRAS);
	*(volatile unsigned long *)addr(0x302024)  = CKC_CHANGE_ARG(TRC);
	*(volatile unsigned long *)addr(0x302028)  = CKC_CHANGE_ARG(TRCD);
	*(volatile unsigned long *)addr(0x30202c)  = CKC_CHANGE_ARG(TRFC);
	*(volatile unsigned long *)addr(0x302030)  = CKC_CHANGE_ARG(TRP);
	*(volatile unsigned long *)addr(0x302034)  = CKC_CHANGE_ARG(TRRD);
	*(volatile unsigned long *)addr(0x302038)  = CKC_CHANGE_ARG(TWR);
	*(volatile unsigned long *)addr(0x30203c)  = CKC_CHANGE_ARG(TWTR);
	*(volatile unsigned long *)addr(0x302040)  = CKC_CHANGE_ARG(TXP);
	*(volatile unsigned long *)addr(0x302044)  = CKC_CHANGE_ARG(TXSR);
	*(volatile unsigned long *)addr(0x302048)  = CKC_CHANGE_ARG(TESR);
	*(volatile unsigned long *)addr(0x302054)  = CKC_CHANGE_ARG(TFAW);

	*(volatile unsigned long *)addr(0x30200C) |= 0x00140000;
	*(volatile unsigned long *)addr(0x30200c)  = CONFIGREG2;				// CFG0
	*(volatile unsigned long *)addr(0x302010)  = CKC_CHANGE_ARG(TREFRESH);	// REFRESH
	*(volatile unsigned long *)addr(0x30204c)  = MEMCFG2REG;				// CFG2

	if(BOARD_DRAM_TOTAL == 128)			//using 27bit address
	*(volatile unsigned long *)addr(0x302200)  = 0x000040F8;	// config_chip0 - CS0 - 0x40000000~0x47ffffff
	else if(BOARD_DRAM_TOTAL == 256)	//using 28bit address
	*(volatile unsigned long *)addr(0x302200)  = 0x000040F0;	// config_chip0 - CS0 - 0x40000000~0x47ffffff
	else
	*(volatile unsigned long *)addr(0x302200)  = 0x000040E0;	// config_chip0 -

	*(volatile unsigned long *)addr(0x303024)  = 0x00000000;	// PHYCTRL
//	*(volatile unsigned long *)addr(0x30302c)  = 0x00004000;	// SSTL
	*(volatile unsigned long *)addr(0x30302c) |= 0x00007fff;	// SSTL

	*(volatile unsigned long *)addr(0x303020)  = COMMONREG;		// emccfg_config0

#if 1
	*(volatile unsigned long *)addr(0x304428)  =
	                      (3 << 17)					// PRD_CAL
						| (0 << 16)					// PRD_CEN
						| (7 << 13)					// DRV_STR
						| ((odt_flag&0x1) << 12)	// TERM_DIS
						| (2 << 9)					// ODT(PHY) value
						| (5 << 6)					// PULL UP
						| (2 << 3)					// PULL DOWN
						| (0 << 2)					// ZQ
						| (0 << 1)					// UPDATE
						| (1 << 0);					// CAL_START
	
	while (!((*(volatile unsigned long *)addr(0x30442c)) & (1)));	// Wait until Calibration completion without error
	*(volatile unsigned long *)addr(0x304428)  =
						  (3 << 17)					// PRD_CAL
						| (0 << 16)					// PRD_CEN
						| (7 << 13)					// DRV_STR
						| ((odt_flag&0x1) << 12)	// TERM_DIS
						| (2 << 9)					// ODT(PHY) value
						| (5 << 6)					// PULL UP
						| (2 << 3)					// PULL DOWN
						| (0 << 2)					// ZQ
						| (1 << 1)					// UPDATE
						| (1 << 0);					// CAL_START
	*(volatile unsigned long *)addr(0x304428)  =
						  (3 << 17)					// PRD_CAL
						| (0 << 16)					// PRD_CEN
						| (7 << 13)					// DRV_STR
						| ((odt_flag&0x1) << 12)	// TERM_DIS
						| (2 << 9)					// ODT(PHY) value
						| (5 << 6)					// PULL UP
						| (2 << 3)					// PULL DOWN
						| (0 << 2)					// ZQ
						| (0 << 1)					// UPDATE
						| (1 << 0);					// CAL_START

	*(volatile unsigned long *)addr(0x304400)  = 0x00000808;	// ddr2, differential DQS, gate siganl for ddr2, differential receiver
	*(volatile unsigned long *)addr(0x304404)  = 0x00080001;	//  DLL-On

	if (nFreq > 3330000)
	*(volatile unsigned long *)addr(0x304408)  = 0x00001212; 	// DLLPDCFG
	else if (nFreq > 2660000)
	*(volatile unsigned long *)addr(0x304408)  = 0x00001717; 	// DLLPDCFG
	else
	*(volatile unsigned long *)addr(0x304408)  = 0x00002020; 	// DLLPDCFG

	*(volatile unsigned long *)addr(0x304404)  = 0x00000003; 	// DLLCTRL
	while (((*(volatile unsigned long *)addr(0x304404)) & (0x00000018)) != (0x00000018));	// Wait DLL Lock

#else
	*(volatile unsigned long *)addr(0x304400)  = 0x00000808;	// ddr2, differential DQS, gate siganl for ddr2, differential receiver
	*(volatile unsigned long *)addr(0x304404)  = 0x00080001;	//  DLL-On

	if (nFreq > 3300000)
	*(volatile unsigned long *)addr(0x304408)  = 0x00001212;	// dll phase detect
	else if (nFreq > 2600000)
	*(volatile unsigned long *)addr(0x304408)  = 0x00001717;	// dll phase detect
	else
	*(volatile unsigned long *)addr(0x304408)  = 0x00002020;	// dll phase detect

	*(volatile unsigned long *)addr(0x304404)  = 0x00000003;	// dll-on|start

	while (((*(volatile unsigned long *)addr(0x304404)) & 0x18)!=0x18);	// Wait PL34X_STATUS_PAUSED

	*(volatile unsigned long *)addr(0x304424)  = 0x00000035;	// DLLFORCELOCK
	*(volatile unsigned long *)addr(0x30440c)  = 0x00000006;	// GATECTRLt
	*(volatile unsigned long *)addr(0x304430)  = DRAM_PHY_CAS;	// RDDELAY

	//calibration start  with forcing option (added by jykim)
#if (ZQ_CAL_START > 0)
	if(CKC_CHANGE_ARG(EMR1+1) == 0) {
		*(volatile unsigned long *)addr(0x304428)  = 0x0006f581;	// term_disable
//		*(volatile unsigned long *)addr(0x304428)  = 0x0056f581;	// term_disable
	}
	else{
		*(volatile unsigned long *)addr(0x304428)  = (0x00060000|
//		*(volatile unsigned long *)addr(0x304428)  = (0x00560000|
													ZQ_DRV_STR|
													ZQ_TERM_DIS|
													ZQ_TERM_VAL|
													ZQ_PULL_DOWN|
													ZQ_PULL_UP|
													ZQ_ZQ|
													//ZQ_UPDATE|
													ZQ_CAL_START);
	}
	while (!((*(volatile unsigned long *)addr(0x30442c)) & (0x01)));	// Wait until Calibration completion without error

	if(CKC_CHANGE_ARG(EMR1+1) == 0){
		*(volatile unsigned long *)addr(0x304428)  = 0x0006f583;	// CAL_START
		*(volatile unsigned long *)addr(0x304428)  = 0x0006f581;	// CAL_START
//		*(volatile unsigned long *)addr(0x304428)  = 0x0056f583;	// CAL_START
//		*(volatile unsigned long *)addr(0x304428)  = 0x0056f581;	// CAL_START
	}
	else{
		*(volatile unsigned long *)addr(0x304428)  = (0x00060000|
//		*(volatile unsigned long *)addr(0x304428)  = (0x00560000|
													ZQ_DRV_STR|
													ZQ_TERM_DIS|
													ZQ_TERM_VAL|
													ZQ_PULL_DOWN|
													ZQ_PULL_UP|
													ZQ_ZQ|
													ZQ_UPDATE|
													ZQ_CAL_START);

		*(volatile unsigned long *)addr(0x304428)  = (0x00060000|
//		*(volatile unsigned long *)addr(0x304428)  = (0x00560000|
													ZQ_DRV_STR|
													ZQ_TERM_DIS|
													ZQ_TERM_VAL|
													ZQ_PULL_DOWN|
													ZQ_PULL_UP|
													ZQ_ZQ|
													//ZQ_UPDATE|
													ZQ_CAL_START);
	}
#else
	if(CKC_CHANGE_ARG(EMR1+1) == 0){
		*(volatile unsigned long *)addr(0x304428)  = 0x0006f581;	// term_disable
//		*(volatile unsigned long *)addr(0x304428)  = 0x0056f581;	// term_disable
	}
	else{
		*(volatile unsigned long *)addr(0x304428)  = (0x00060000|
//		*(volatile unsigned long *)addr(0x304428)  = (0x00560000|
													ZQ_DRV_STR|
													ZQ_TERM_DIS|
													ZQ_TERM_VAL|
													ZQ_PULL_DOWN|
													ZQ_PULL_UP|
													ZQ_ZQ|
													//ZQ_UPDATE|
													ZQ_CAL_START);
	}
#endif
#endif

	*(volatile unsigned long *)addr(0x302008)  = 0x000c0000;	// CMD, BNKA

	// repeat 400ns
	i = 100;
	while(i) {
		i--;
	}

	*(volatile unsigned long *)addr(0x302008)  = 0x00000000;	// pre-charge all
	*(volatile unsigned long *)addr(0x302008)  = 0x00040000;	// auto-refresh
	*(volatile unsigned long *)addr(0x302008)  = 0x00040000;	// auto-refresh

#if 1
	//MR
	//update write recovery time to avoid dram buffer overwrite
	*(volatile unsigned long *)addr(0x302008)  = (0x00080002|(DRAM_PHY_CAS<<4)|((CKC_CHANGE_ARG(TWR))<<9)|(1<<8));	// EMR1	
	*(volatile unsigned long *)addr(0x302008) &= ~(1<<8);

	//EMR1
	*(volatile unsigned long *)addr(0x302008)  = 0x00090000;	// Direct COmmnad Register
	*(volatile unsigned long *)addr(0x302008)  = 0x00090380;	// Direct COmmnad Register
	if(CKC_CHANGE_ARG(EMR1+1) == 0)
	*(volatile unsigned long *)addr(0x302008)  = 0x00090000;	// Direct COmmnad Register //soc1-3
	else
	*(volatile unsigned long *)addr(0x302008)  = 0x00090004;	// Direct COmmnad Register //soc1-3

	*(volatile unsigned long *)addr(0x302008)  = 0x000a0000;	// EMRS2
	*(volatile unsigned long *)addr(0x302008)  = 0x000b0000;	// EMRS3

	// repeat 100
	i = 100;
	while(i) {
		*(volatile unsigned long *)addr(0x302008)  = 0x00040000;	// auto-refresh
		i--;
	}
#else
	*(volatile unsigned long *)addr(0x302008)  = 0x000a0000;	// EMRS1
	*(volatile unsigned long *)addr(0x302008)  = 0x000b0000;	// EMRS2
	*(volatile unsigned long *)addr(0x302008)  = 0x00090000;	// MRS

	//update write recovery time to avoid dram buffer overwrite
	*(volatile unsigned long *)addr(0x302008)  = (0x00080002|(DRAM_PHY_CAS<<4)|((CKC_CHANGE_ARG(TWR)-1)<<9)|(1<<8));	// CKC_CHANGE_ARG(EMR1); 		//EMR1
	
	*(volatile unsigned long *)addr(0x302008)  = 0x00000000;	// pre-charge all
		
	// repeat 100
	i = 100;
	while(i) {
		*(volatile unsigned long *)addr(0x302008)  = 0x00040000;	// dir_cmd
		i--;
	}

	*(volatile unsigned long *)addr(0x302008) &= ~(1<<8);

	if(CKC_CHANGE_ARG(EMR1+1) == 0){
	}
	else{
		*(volatile unsigned long *)addr(0x302008)  = 0x00090380;	// Direct COmmnad Register
		*(volatile unsigned long *)addr(0x302008)  = 0x00090004;	// Direct COmmnad Register //soc1-3
	}
#endif

	*(volatile unsigned long *)addr(0x302004)     =	 0x00000000    ;// PL341_GO

}

static void init_copychangeclock(unsigned int freq)
{
	unsigned int lbusvalue = freq*10;
	volatile unsigned int	*fptr = 0;
	volatile unsigned int	*p = 0;
	int 					i = 0;
	unsigned int temp;
	unsigned int DRAM_BUS_CLOCK = 0;
	int tCK = 0;

	DRAM_BUS_CLOCK = lbusvalue/10000;//DRAM_BUS_CLOCK;

	for (i=0 ; i<NUM_MEMCLK ; i++) {
		if (pIO_CKC_PLL3[i].freq_value >= lbusvalue)
			break;
	}

	if (i >= NUM_MEMCLK )
		i = (NUM_MEMCLK - 1);

	CKC_CHANGE_ARG(PLLVALUE_DIS) =	pIO_CKC_PLL3[i].pll1config_reg;
	CKC_CHANGE_ARG(CLK_CTRL2) =		pIO_CKC_PLL3[i].clkctrl2_reg;

#if 1
	// for linux compile(x100)
	tCK = 100000/DRAM_BUS_CLOCK;
	CKC_CHANGE_ARG(TCAS) = (DRAM_PHY_CAS<<1);												//TCAS
#if 1
	CKC_CHANGE_ARG(TRCD) = DDR2_SETRCD(ddr_time_to_cycle(tCK, 1500, 0x04, 0x0F));			//TRCD
	CKC_CHANGE_ARG(TRP)  = DDR2_SETRP(ddr_time_to_cycle(tCK, 1500, 0x04, 0x07));			//TRP	
	CKC_CHANGE_ARG(TRAS) = ddr_time_to_cycle(tCK, 4500, 0x00, 0x1F);						//TRAS
	CKC_CHANGE_ARG(TRC)  = ddr_time_to_cycle(tCK, 6000, 0x00, 0x1F);						//TRC	
	CKC_CHANGE_ARG(TRFC) = DDR2_SETRFC(ddr_time_to_cycle(tCK, DRAM_PHY_RFC, 0x0F, 0x7F));	//TRFC
	CKC_CHANGE_ARG(TESR) = (int)(200);
	CKC_CHANGE_ARG(TRRD) = ddr_time_to_cycle(tCK, ((BOARD_DRAM_DATABITS == 16) ? 750 : 1000), 0x00, 0x0F);	//TRRD
	CKC_CHANGE_ARG(TWR)  = ddr_time_to_cycle(tCK, 1500, 0x00, 0x07);						//TWR	
	CKC_CHANGE_ARG(TWTR) = ddr_time_to_cycle(tCK, 750, 0x00, 0x07);//(int)(3)									;//TWTR
	CKC_CHANGE_ARG(TXP)  = 2;//(int)(3)									;// TXP	
	CKC_CHANGE_ARG(TXSR) = DDR2_SETRFC(ddr_time_to_cycle(tCK, (DRAM_PHY_RFC+1000), 0x00, 0xFF));	//TXSR
	CKC_CHANGE_ARG(TFAW) = DDR2_SETFAW(ddr_time_to_cycle(tCK, 4500, 0x00, 0x1F));			//TFAW
	CKC_CHANGE_ARG(TREFRESH) = (int)(((780000/tCK +DDR_DELAY) > 0x7fff) ? 0x7fff : (780000/tCK +DDR_DELAY));//TREFRESH
#else
	CKC_CHANGE_ARG(TRCD) = DDR2_SETRCD(ddr_time_to_cycle(tCK, 1800, 0x04, 0x0F));			//TRCD
	CKC_CHANGE_ARG(TRP)  = DDR2_SETRP(ddr_time_to_cycle(tCK, 1800, 0x04, 0x07));			//TRP	
	CKC_CHANGE_ARG(TRAS) = ddr_time_to_cycle(tCK, 4500, 0x00, 0x1F);						//TRAS
	CKC_CHANGE_ARG(TRC)  = ddr_time_to_cycle(tCK, 6000, 0x00, 0x1F);						//TRC	
	CKC_CHANGE_ARG(TRFC) = DDR2_SETRFC(ddr_time_to_cycle(tCK, DRAM_PHY_RFC, 0x0F, 0x7F));	//TRFC
	CKC_CHANGE_ARG(TESR) = (int)(200);														//TESR
	CKC_CHANGE_ARG(TRRD) = ddr_time_to_cycle(tCK, ((BOARD_DRAM_DATABITS == 16) ? 750 : 1000), 0x00, 0x0F);	//TRRD
	CKC_CHANGE_ARG(TWR)  = ddr_time_to_cycle(tCK, 1500, 0x00, 0x07);						//TWR	
	CKC_CHANGE_ARG(TWTR) = (int)(3)									;//TWTR
	CKC_CHANGE_ARG(TXP)  = (int)(3)									;// TXP	
	CKC_CHANGE_ARG(TXSR) = DDR2_SETRFC(ddr_time_to_cycle(tCK, (DRAM_PHY_RFC+1000), 0x00, 0xFF));	//TXSR
	CKC_CHANGE_ARG(TFAW) = DDR2_SETFAW(ddr_time_to_cycle(tCK, 4500, 0x00, 0x1F));			//TFAW
	CKC_CHANGE_ARG(TREFRESH) = (int)(((780000/tCK +DDR_DELAY) > 0x7fff) ? 0x7fff : (780000/tCK +DDR_DELAY));//TREFRESH
#endif
#else
	// for linux compile(x100)
	tCK = 100000/DRAM_BUS_CLOCK;
	CKC_CHANGE_ARG(TCAS) =	(DRAM_PHY_CAS<<1)										;//TCAS
	//CKC_CHANGE_ARG(TRCD) =	DDR2_SETRCD((int) (DRAM_PHY_CAS*(DRAM_PHY_MAX_FREQ == 330 ? 3 :2.5 )/tCK)  )							;//TRCD
	//CKC_CHANGE_ARG(TRP) =	DDR2_SETRP((int)(DRAM_PHY_CAS*(DRAM_PHY_MAX_FREQ == 330 ? 3 :2.5 )/tCK))							;//TRP		
	CKC_CHANGE_ARG(TRCD) =	DDR2_SETRCD((int) (1800/tCK) +DDR_DELAY )							;//TRCD
	CKC_CHANGE_ARG(TRP) =	DDR2_SETRP((int) (1800/tCK) +DDR_DELAY )							;//TRP	
	CKC_CHANGE_ARG(TRAS) =	(int)(4500/tCK+DDR_DELAY)									;//TRAS
	CKC_CHANGE_ARG(TRC) =	(int)(6000/tCK+DDR_DELAY) 	;//TRC	
	CKC_CHANGE_ARG(TRFC) =	DDR2_SETRFC((int)(DRAM_PHY_RFC/tCK +DDR_DELAY))				;//TRFC
//	CKC_CHANGE_ARG(TESR) =	(int)((DRAM_PHY_RFC+1000)/tCK+DDR_DELAY)						;//TESR
	CKC_CHANGE_ARG(TESR) =	(int)(200)						;//TESR
	CKC_CHANGE_ARG(TRRD) =	(int)( ((BOARD_DRAM_DATABITS == 16) ? 750 : 1000)/tCK +DDR_DELAY);//TRRD
	CKC_CHANGE_ARG(TWR) =	 (int)(1500/tCK +DDR_DELAY)								;//TWR	
	CKC_CHANGE_ARG(TWTR) =	(int)(3)									;//TWTR
	CKC_CHANGE_ARG(TXP) =	(int)(3)									;// TXP	
//	CKC_CHANGE_ARG(TWTR) =	(int)(5)									;//TWTR
//	CKC_CHANGE_ARG(TXP) =	(int)(2)									;// TXP	
	CKC_CHANGE_ARG(TXSR) =	DDR2_SETRFC((int)((DRAM_PHY_RFC+1000)/tCK +DDR_DELAY))				;//TXSR
//	CKC_CHANGE_ARG(TXSR) =	 (int)(200)									;//TXSR
	CKC_CHANGE_ARG(TFAW) =	DDR2_SETFAW( (int)(4500/tCK) +DDR_DELAY)									;//TFAW
	CKC_CHANGE_ARG(TREFRESH) =	(int)(780000/tCK +DDR_DELAY)								;//TREFRESH
#endif

	if(pIO_CKC_PLL3[i].freq_value>3300000)
		CKC_CHANGE_ARG(EMR1) =	EMR1REG+0x300;//EMR1REG
	else
		CKC_CHANGE_ARG(EMR1) =	EMR1REG;//EMR1REG

	if(pIO_CKC_PLL3[i].freq_value < 2000000)
		CKC_CHANGE_ARG(EMR1+1) =	0 ;//ODT_CTRL_OFF
	else
		CKC_CHANGE_ARG(EMR1+1) =	1 ;//ODT_CTRL_ON


//Prevent Page Table Work. 
	temp = CKC_CHANGE_ARG(TCAS);
	//printk("TCAS : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TRCD);
	//printk("TRCD : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TRP) ;
	//printk("TRP  : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TRAS);
	//printk("TRAS : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TRC) ;
	//printk("TRC  : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TRFC);
	//printk("TRFC : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TESR);
	//printk("TESR : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TRRD);
	//printk("TRRD : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TWR) ;
	//printk("TWR  : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TWTR);
	//printk("TWTR : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TXP) ;
	//printk("TXP  : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TXSR);
	//printk("TXSR : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TFAW);
	//printk("TFAW : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(TREFRESH) ;
	//printk("TREF : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(PLLVALUE_DIS);
	//printk("TPLL : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(CLK_CTRL2);	
	//printk("TCLK : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(EMR1); 
	//printk("EMR1 : 0x%08x\n", temp);
	temp = CKC_CHANGE_ARG(EMR1+1);
	//printk("EMR1 : 0x%08x\n", temp);

	fptr = (volatile unsigned int*)init_clockchange_ddr2;	
	
	lpSelfRefresh = (lpfunc)(SRAM_COPY_ADDR);
	
	p = (volatile unsigned int*)SRAM_COPY_ADDR;
	for (i = 0;i < (SRAM_COPY_FUNC_SIZE);i++)
	{
		*p = *fptr;
		p++;
		fptr++;
	}
	
	while(--i);
	
	// Jump to Function Start Point
	
	lpSelfRefresh(pIO_CKC_PLL3[i].freq_value);
}

#include <mach/tca_lcdc.h>

void tcc_ddr2_set_clock(unsigned int freq)
{
	unsigned int lcdc0_on = 0, lcdc1_on = 0;
	volatile PLCDC	pLCDC_BASE0 = (volatile PLCDC)tcc_p2v(HwLCDC0_BASE);
	volatile PLCDC	pLCDC_BASE1 = (volatile PLCDC)tcc_p2v(HwLCDC1_BASE);
	
#if defined(CONFIG_GENERIC_TIME)
	volatile PPIC	pPIC	= (volatile PPIC)tcc_p2v(HwPIC_BASE);
#else
	volatile PTIMER	pTIMER	= (volatile PTIMER)tcc_p2v(HwTMR_BASE);
#endif
	int_alldisable();
	local_flush_tlb_all();
	flush_cache_all();

#if defined(CONFIG_GENERIC_TIME)
	pPIC->IEN0 &= ~Hw1;		/* Disable Timer0 interrupt */
#else
	pTIMER->TC32EN &= ~Hw24;
#endif

#if 1
	lcdc0_on = DEV_LCDC_Wait_signal(0);
	lcdc1_on = DEV_LCDC_Wait_signal(1);
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
EXPORT_SYMBOL(tcc_ddr2_set_clock);

static int __init tcc_ddr_init(void)
{	
	return 0;
}

__initcall(tcc_ddr_init);

