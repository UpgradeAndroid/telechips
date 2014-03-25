/*
 * linux/arch/arm/mach-tcc9300/tcc_ckc_ctrl.c
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

#if defined(__WINCE__)
#include "windows.h"
#include "bsp.h"
#include "tca_ckc.h"
#else
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/spinlock.h>
#include <asm/cacheflush.h>
#include <asm/tlbflush.h>
#include <mach/clock.h>
#include <mach/globals.h>
#include <mach/tca_ckc.h>
#include <linux/delay.h>
#include <bsp.h>
#include <asm/io.h> 
#include <linux/tcc_ioctl.h>
#include <mach/tcc_ckc_ctrl.h>
//Bruce, 100930, Porting into the Android Kernel
//#include "../../../../../bootloader/tcboot/include/ddr.h"
#include <mach/ddr.h>
//Bruce, 100930, Porting into the Android Kernel
#undef _WINCE_
#endif

#define FBUS_STEP_NUM 33
#define FCORE_STEP_NUM 69


#if defined(_LINUX_)
	#define dbg(x...) do {} while(0)
	#define addr(b) (0xf0000000+b)
	//Bruce, 100930, Porting into the Android Kernel
	#if (1)
	#define SRAM_COPY_ADDR				0xEFE00000
	#define CKC_CHANGE_ARG_BASE         0xEFE00400
	#else
	#define SRAM_COPY_ADDR				0xEFF00000
	#define CKC_CHANGE_ARG_BASE         0xEFF01800
	#endif
#else
	#define addr(b) (0xB0000000+b)
	#define SRAM_COPY_ADDR				addr(0xA00000)
	#define CKC_CHANGE_ARG_BASE			addr(0xA01800)
#endif

#if defined(_LINUX_)
extern unsigned int g_org_tcc_clk; /* backup cpu clock value */
#endif
typedef void (*lpfunc)(unsigned int lbusvalue);
static lpfunc lpSelfRefresh;
#define SRAM_COPY_FUNC_SIZE			0x400

///////////////jy defines for ckc_ioctl
// data array
#define CKC_CHANGE_ARG(x)           (*(volatile unsigned long *)(CKC_CHANGE_ARG_BASE + (4 * (x))))
extern int arm_changestack(void);
extern void arm_restorestack(unsigned int rst);
extern int arm_disableInt(void);
extern void arm_restoreInt(int rint);

#if defined(__WINCE__)
static unsigned int _gCPSRInt = 0;
#else
static unsigned long flags;
#endif
volatile unsigned int retstack = 0;

unsigned int FbusStepValue[FBUS_STEP_NUM] = {
	2640000, // PLL3
	2340000,
	1760000, // PLL3
	1560000,
	1320000, // PLL3
	1170000,
	1056000, // PLL3
	936000,
	880000, // PLL3
	780000,
	754285, // PLL3
	668571,
	660000, // PLL3
	586666, // PLL3
	585000,
	528000, // PLL3
	520000,
	480000, // PLL3
	468000,
	440000, // PLL3
	425454,
	406153, // PLL3
	390000,
	377142, // PLL3
	360000,
	352000, // PLL3
	334285,
	330000, // PLL3
	312000,
	292500, 
	60000, // XIN
	30000, // XIN
	10000, // XIN
};

unsigned int FcoreStepValue[FCORE_STEP_NUM] = {
	8000000, 7200000, 6000000, 5062500, 4860000,
	4725000, 4556250, 4387500, 4252500, 4050000,
	3948750, 3780000, 3712500, 3645000, 3510000,
	3375000, 3240000, 3037500, 2970000, 2835000,
	2700000, 2632500, 2430000, 2362500, 2227500,
	2160000, 2025000, 1890000, 1822500, 1750000,
	1687500, 1620000, 1518750, 1485000, 1417500,
	1350000, 1215000, 1125000, 1080000, 1012500, 
	960000,  945000,  907500,  892500,  840000, 
	810000,  787500,  765000,  730000,  712500,
	660000,  637500,  607500,  577500,  547500, 
	495000,  450000,  405000,  382500,  365000,
	330000,  270000,  247500,  240000,  225000, 
	202500,  182500,  135000,  120000,
};

unsigned int FcorePllValue[FCORE_STEP_NUM] = {
	8000000, 7200000, 6000000, 5400000,	4860000,	
	5400000, 4860000, 5400000, 4860000,	4320000,	
	4860000, 4320000, 5400000, 4860000,	4320000,	
	5400000, 3240000, 3240000, 4320000,	3240000,	
	5400000, 3240000, 3240000, 5400000, 3240000, 
	2160000, 2160000, 2160000, 3240000,	2160000,	
	5400000, 2160000, 4860000, 2160000, 3240000, 
	2160000, 2160000, 1800000, 2160000,	3240000,	
	1920000, 2160000, 1320000, 2040000,	1920000,	
	2160000, 1800000, 2040000, 1460000,	1900000,	
	1320000, 2040000, 3240000, 1320000,	1460000, 
	1320000, 5400000, 2160000, 2040000, 1460000,    
	1320000, 4320000, 1320000, 1920000, 1800000,    
	3240000, 1460000, 2160000, 1920000, 
};

unsigned int FcoreDividerValue[FCORE_STEP_NUM] = {
	16, 16, 16, 15,	16,	
	14,	15,	13,	14,	15,	
	13,	14,	11, 12,	13,	
	10,	16,	15,	11,	14,	
	 8,	13,	12,  7,	11,	
	16,	15,	14,	 9,	13,	
	 5,	12,  5, 11,	 7,	
	10,	 9,	10,	 8,	 5,	
	 8,	 7,	11,  7,	 7,	
	 6,	 7,	 6,	 8,	 6,	
	 8,	 5,	 3,  7,	 6,	
	 6,	 5,	 4,  3,  4, 
	 4,  1,  3,  2,  2,  
	 1,  2,  1,  1, 
};


#if defined(DRAM_DDR2) || defined(DRAM_DDR3)
typedef struct {
    unsigned int        pll1config_reg;
    unsigned int        clkctrl2_reg;   

	//Bruce, 100930, Porting into the Android Kernel
	unsigned int		freq_value;
} MEMCLK;
#elif defined(DRAM_MDDR)
typedef struct {
    unsigned int        pll1config_reg;
    char lchange_source;
    char lchange_div; 
    char lmem_source; 
    char lmem_div;  
} MEMCLK1;
#endif

#if defined(DRAM_DDR3)
	enum {
		T_REFI = 0, /* 0 <= t_refi < 2^17 */
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

		//Clock
		PLLVALUE_DIS_DDR3,
		CLK_CTRL2_DDR3,
		MR0_DDR3,
		MR1_DDR3,
		MR2_DDR3,
		MR3_DDR3,
		DLLONOFF_DDR3,
		//Bruce, 101102, WR과 CL의 변경사항이 없으면 MRS Setting을 다시 하지 않는다.
		MRS_USE
	};
#else//DDR2, MDDR
	enum {
#if defined(DDR2CONTROLLER_LPCON)
		TAREF = 0,
		TROW,
		TDATA,
		TPOWER,
		PLLVALUE_DIS,
		CLK_CTRL2,
		TWR,
#else	
		TCAS = 0,
		TRCD,
		TRP ,
		TRAS,
		TRC ,
		TRFC,
		TESR,
		TRRD,
		TWR ,
		TWTR,
		TXP ,
		TXSR,
		TFAW,
		TREFRESH,
		PLLVALUE_DIS,
		CLK_CTRL2,
		EMR1
#endif		
	};

	//////////////user configuration start//////////////////////////
	//Physical DRAM description
	#if defined(DRAM_PHY_MAX_FREQ_400)
	#define DRAM_PHY_MAX_FREQ   (400)
	#elif defined(DRAM_PHY_MAX_FREQ_330)
	#define DRAM_PHY_MAX_FREQ   (330)
	#elif defined(DRAM_PHY_MAX_FREQ_200)
	#define DRAM_PHY_MAX_FREQ   (200)
	#elif defined(DRAM_PHY_MAX_FREQ_185)
	#define DRAM_PHY_MAX_FREQ   (185)
	#elif defined(DRAM_PHY_MAX_FREQ_166)
	#define DRAM_PHY_MAX_FREQ   (166)
	#else
	#define DRAM_PHY_MAX_FREQ   (330)
	#endif

	#if defined(DRAM_PHY_CAS_3)
	#define DRAM_PHY_CAS        (3)
	#elif defined(DRAM_PHY_CAS_4)
	#define DRAM_PHY_CAS        (4)
	#elif defined(DRAM_PHY_CAS_5)
	#define DRAM_PHY_CAS        (5)
	#elif defined(DRAM_PHY_CAS_6)
	#define DRAM_PHY_CAS        (6)
	#else
	#define DRAM_PHY_CAS   (5)
	#endif

	#if defined(DRAM_PHY_BANKBIT_2)
	#define DRAM_PHY_BANKBIT    (2)
	#elif defined(DRAM_PHY_BANKBIT_3)
	#define DRAM_PHY_BANKBIT    (3)
	#else
	#define DRAM_PHY_BANKBIT    (2)
	#endif
	
	#if defined(DRAM_PHY_ROWBITS_13)
	#define DRAM_PHY_ROWBITS    (13)
	#elif defined(DRAM_PHY_ROWBITS_14)
	#define DRAM_PHY_ROWBITS    (14)
	#else
	#define DRAM_PHY_ROWBITS    (14)
	#endif
	
	#if defined(DRAM_PHY_COLBITS_9)
	#define DRAM_PHY_COLBITS    (9)
	#elif defined(DRAM_PHY_COLBITS_10)
	#define DRAM_PHY_COLBITS    (10)
	#else
	#define DRAM_PHY_COLBITS    (10)
	#endif

	#if defined(DRAM_PHY_DATABITS_16)
	#define DRAM_PHY_DATABITS   (16)
	#elif defined(DRAM_PHY_DATABITS_32)
	#define DRAM_PHY_DATABITS   (32)
	#else
	#define DRAM_PHY_DATABITS   (32)
	#endif

	#if defined(DRAM_PHY_SIZE_256)
	#define DRAM_PHY_SIZE       (256) //Mbit
	#elif defined(DRAM_PHY_SIZE_512)
	#define DRAM_PHY_SIZE       (512) //Mbit
	#elif defined(DRAM_PHY_SIZE_1024)
	#define DRAM_PHY_SIZE       (1024) //Mbit
	#else
	#define DRAM_PHY_SIZE		(2048) //Mbit
	#endif

	#if defined(BOARD_DRAM_PHYSICAL_NUM_1)
	#define BOARD_DRAM_PHYSICAL_NUM		(1) //실제 상판에 붙은 DRAM 갯수 - size 계산에 필요
	#elif defined(BOARD_DRAM_PHYSICAL_NUM_2)
	#define BOARD_DRAM_PHYSICAL_NUM		(2) //실제 상판에 붙은 DRAM 갯수 - size 계산에 필요
	#else
	#define BOARD_DRAM_PHYSICAL_NUM		(2) //실제 상판에 붙은 DRAM 갯수 - size 계산에 필요
	#endif


	#if defined(BOARD_DRAM_LOGICAL_NUM_1)
	#define BOARD_DRAM_LOGICAL_NUM		(1) //컨트롤러가 인식할 DRAM수 - 16*2로 32bit처럼 사용하므로 1
	#elif defined(BOARD_DRAM_LOGICAL_NUM_2)
	//for test 2CS - only test
	#define BOARD_DRAM_LOGICAL_NUM		(2) //컨트롤러가 인식할 DRAM수 - 16*2로 16bit 사용하므로 2개
	#else
	#define BOARD_DRAM_LOGICAL_NUM		(1) //컨트롤러가 인식할 DRAM수 - 16*2로 32bit처럼 사용하므로 1개로함
	#endif

	#if defined(BOARD_DRAM_DATABITS_16)
	#define BOARD_DRAM_DATABITS (16)
	#elif defined(BOARD_DRAM_DATABITS_32)
	#define BOARD_DRAM_DATABITS (32)
	#else
	#define BOARD_DRAM_DATABITS (32)
	#endif

	//////////////user configuration end//////////////////////////
	#define BOARD_DRAM_TOTAL    (DRAM_PHY_SIZE * BOARD_DRAM_PHYSICAL_NUM) / 8// MByte
	#define DRAM_AUTOPD_ENABLE (Hw13)
	#if defined(DRAM_DDR2)
	#define DRAM_AUTOPD_PERIOD (7<<7) // must larger than CAS latency
	#elif defined(DRAM_MDDR)
	#define DRAM_AUTOPD_PERIOD (4<<7) // must larger than CAS latency
	#endif


	#define DRAM_SET_AUTOPD DRAM_AUTOPD_ENABLE|DRAM_AUTOPD_PERIOD
	#define DDR_DELAY   (1)
	#define PLLEN (Hw31)

#if defined(DRAM_DDR2)
	#if defined(DDR2CONTROLLER_LPCON)
		
		/*
		//description of 0xB0305004
		#define MEMCTRL ( 
			(0x2 << 20)//4                  // memory burst length is 4
			| ((BOARD_DRAM_LOGICAL_NUM-1)<<16) //num of chips
			| ((BOARD_DRAM_DATABITS/16-1)<<12) //width of memory data bus
			| (0x4 << 8) //ddr type is ddr2 (lpddr: 0x1, lpddr2: 0x2, ddr2=0x4)
			| (0x0 << 6) //addtionanl latency for PALL is 0 cyle
			| (0x0 << 5) //Dynamic Self Refresh is not used
			| (0x0 << 4) //Timeout Precharge is not used
			| (0x0 << 2) //Type of Dynamic Power Down is "Active/precharge power down"
			| (0x0 << 1) //Dynamic Power Down is not used 
			| (0x0) //Dynamic Clock Control is not used (Always running)
			)
		*/
		#define MEMCTRL ( (0x2 << 20) \ 
			|((BOARD_DRAM_LOGICAL_NUM-1)<<16) \
			|((BOARD_DRAM_DATABITS/16)<<12) \
			| (0x4 << 8) \
			| (0x0 << 6) \
			| (0x0 << 5) \
			| (0x0 << 4) \
			| (0x0 << 2) \
			| (0x0 << 1) \
			| (0x0) )

		/*
		//description of 0xB0305008	
		#define MEMCHIP0CFG (
			(0x40<<24) //AXI Base Address
			|((0xFF - ((DRAM_PHY_SIZE*BOARD_DRAM_PHYSICAL_NUM/8)/(BOARD_DRAM_LOGICAL_NUM*0x10)-1))<<16)//AXI Base Address Mask
			|(0x1 << 12) //Address Map Method. (linear : 0x0, interleaved: 0x1)
			|((DRAM_PHY_COLBITS - 7)<<8)
			|((DRAM_PHY_ROWBITS - 12)<<4)//Num of Row Address Bits
			| DRAM_PHY_BANKBIT //Num of Banks
			)
		*/
		#define MEMCHIP0CFG ( (0x40<<24) \
			|((0xFF - ((DRAM_PHY_SIZE*BOARD_DRAM_PHYSICAL_NUM/8)/(BOARD_DRAM_LOGICAL_NUM*0x10)-1))<<16) \
			|(0x1 << 12) \
			|((DRAM_PHY_COLBITS - 7)<<8) \
			|((DRAM_PHY_ROWBITS - 12)<<4) \
			| DRAM_PHY_BANKBIT )

		/*
		//Description of 0xB030500C	
		#define MEMCHIP1CFG (
			((0x40 - (DRAM_PHY_SIZE*BOARD_DRAM_PHYSICAL_NUM/8)/(BOARD_DRAM_LOGICAL_NUM*0x10))<<24) //AXI Base Address
			|((0xFF - ((DRAM_PHY_SIZE*BOARD_DRAM_PHYSICAL_NUM/8)/(BOARD_DRAM_LOGICAL_NUM*0x10)-1))<<16)//AXI Base Address Mask
			|(0x1 << 12) //Address Map Method. (linear : 0x0, interleaved: 0x1)
			|((DRAM_PHY_COLBITS - 7)<<8)
			|((DRAM_PHY_ROWBITS - 12)<<4)//Num of Row Address Bits
			| DRAM_PHY_BANKBIT //Num of Banks
			)
		*/
		#define MEMCHIP1CFG ( \
			((0x40 - (DRAM_PHY_SIZE*BOARD_DRAM_PHYSICAL_NUM/8)/(BOARD_DRAM_LOGICAL_NUM*0x10))<<24) \
			|((0xFF - ((DRAM_PHY_SIZE*BOARD_DRAM_PHYSICAL_NUM/8)/(BOARD_DRAM_LOGICAL_NUM*0x10)-1))<<16) \
			|(0x1 << 12) \
			|((DRAM_PHY_COLBITS - 7)<<8) \ 
			|((DRAM_PHY_ROWBITS - 12)<<4) \
			| DRAM_PHY_BANKBIT )
		
		/*
		//Description of 0xB0305044
		#define PHYZQCTRL	(
			(0xE38<<20)
			| (0x2<<17) //Immediate Control Code for Pull-up
			| (0x5<<14) //Immediate Control Code for Pull-down
			| (0x2<<11) //ODT Resister value
			| (0x7<<8) //Driver Strength
			| (0x2<<4) //Calibration I/O Clock (0x0=mclk/2, 0x1=mclk/4, 0x2=mclk/8, 0x3=mclk/16, 0x4=mclk/32)
			| (0x0<<2) //Force Calibration
			| (0x0<<1) //Auto Calibration Start
			| (0x0<<0) //Termination Disable(0x0=Termination enble, 0x1=Termination diable)
			)
		*/

	#if defined(CONFIG_DDR2_HXB18T2G160AF)
		#define PHYZQCTRL	( \
			  (0xE38<<20) \
			| (0x2<<17) \
			| (0x5<<14) \
			| (0x2<<11) \
			| (0x7<<8) \
			| (0x3<<4) \
			| (0x0<<2) \
			| (0x0<<1) \
			| (0x0<<0) \
			)
	#else
		#if 1
		#define PHYZQCTRL	( \
			  (0x0<<20) \
			| (0x2<<17) \
			| (0x5<<14) \
			| (0x2<<11) \
			| (0x7<<8) \
			| (0x3<<4) \
			| (0x0<<2) \
			| (0x0<<1) \
			| (0x0<<0) \
			)
		#else
		#define PHYZQCTRL	( \
			(0x0<<20) \
			| (0x0<<17) \
			| (0x0<<14) \
			| (0x0<<11) \
			| (0x7<<8) \
			| (0x2<<4) \
			| (0x0<<2) \
			| (0x0<<1) \
			| (0x0<<0) \
			)			
		#endif
	#endif

	#if (defined(CONFIG_DDR2_HXB18T2G160AF))
		#define RFC(x) (17500)
	#else
		#define RFC(x) ( x == 256 ? 7500 :(x ==  512 ? 10500 :(x == 1024 ? 12750:12750)))
	#endif
		#define DRAM_PHY_RFC        RFC(DRAM_PHY_SIZE)
	#else
		//common macros..
		#define DDR2_SETRCD(x)	((x)> 3? ((((x)-3)<<8)| (x) ) : ((1<<8) | (x)))
		#define DDR2_SETRP(x)	((x)> 3? ((((x)-3)<<8)| (x) ) : ((1<<8) | (x)))
		#define DDR2_SETRFC(x)	((x)> 3? ((((x)-3)<<8)| (x) ) : ((1<<8) | (x)))
		#define DDR2_SETFAW(x)	((x)> 3? ((((x)-3)<<8)| (x) ) : ((1<<8) | (x)))

		//memory controller configuration
		#define MDE_1CS				((DRAM_PHY_ROWBITS - 13)<<2)
		#define COMMONREG           (0x00070103 | MDE_1CS)
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
		#define RFC(x) ( x == 256 ? 7500 :(x ==  512 ? 10500 :(x == 1024 ? 12750:19500)))
		#define DRAM_PHY_RFC        RFC(DRAM_PHY_SIZE)

		//ZQ option update
		//if auto calibration result is bad, you can use below options to setting right ZQ pullup, pulldown value 
		#define DRV_STR		(7) 
		#define TERM_DIS	(0) //ODT function control 0: enable 1: disable
		#define TERM_VAL	(2) //ODT value setting 1:75ohm 2:150ohm 3: 50ohm 4: 35(spec out)

		#define CAL_START	(1)//calibration control 0: manual(with ZQ = 1) 1:auto
		#define ZQ		(0)//ZQ FORCING enable 1:overwrite pullup,pulldown value
		#define PULL_DOWN	(6) //5 or 6 is very good
		#define PULL_UP		(0) //0 or 1 is very good.  over 2 cause aging fail.(we do not guarantee.)

		#define ZQ_DRV_STR			(DRV_STR<<13)
		#define ZQ_TERM_DIS			(TERM_DIS<<12)
		#define ZQ_TERM_VAL			(TERM_VAL<<9)
		#define ZQ_PULL_DOWN		(PULL_DOWN<<6)
		#define ZQ_PULL_UP			(PULL_UP<<3)
		#define ZQ_ZQ				(ZQ<<2)
		#define ZQ_UPDATE			(1<<1)
		#define ZQ_CAL_START		(CAL_START)
	#endif
#elif defined(DRAM_MDDR)
	#define MDDR_SETRP(x)  ((x)>3? ((((int)x-3)<<3)| (int)(x) ) : ((0<<3) | (int)(x)))
	#define MDDR_SETRCD(x) ((x)>3? ((((int)x-3)<<3)| (int)(x) ) : ((0<<3) | (int)(x)))
	#define MDDR_SETRFC(x) ((x)>3? ((((int)x-3)<<5)| (int)(x) ) : ((0<<5) | (int)(x)))
	#define XSR  ( DRAM_PHY_SIZE == 512 ? 120 :140)
	#define WTR  ( DRAM_PHY_MAX_FREQ == 200 ? 2 :1)
	/*
	#define RRD  (DRAM_PHY_MAX_FREQ == 200 ? 10 :(DRAM_PHY_MAX_FREQ ==  185 ? 10.8 : 12))
	#define RCD  (DRAM_PHY_MAX_FREQ == 200 ? 15 :(DRAM_PHY_MAX_FREQ ==  185 ? 16.2 : 18))
	#define RC   (DRAM_PHY_MAX_FREQ == 200 ? 55 :(DRAM_PHY_MAX_FREQ ==  185 ? 59.4 : 60))
	*/
	#define RRD  (DRAM_PHY_MAX_FREQ == 200 ? 1000 :(DRAM_PHY_MAX_FREQ ==  185 ? 1080 : 1200))
	#define RCD  (DRAM_PHY_MAX_FREQ == 200 ? 1500 :(DRAM_PHY_MAX_FREQ ==  185 ? 1620 : 1800))
	#define RC   (DRAM_PHY_MAX_FREQ == 200 ? 5500 :(DRAM_PHY_MAX_FREQ ==  185 ? 5940 : 6000))
	#define RAS  ( DRAM_PHY_MAX_FREQ == 200 ? 40 :42)

	#define ACTCH       1
	#define QOS_MBITS   0
	#define MBURST      1
	#define CONFIGREG1          ( (ACTCH<<21) | (QOS_MBITS<<18) | (MBURST << 16) | DRAM_SET_AUTOPD\
                                    | ((DRAM_PHY_ROWBITS-11)<<3) |(DRAM_PHY_COLBITS-8) )
    
	#define RD_DLY  1 << 9
	#define TYPE    3 << 6
	#define MEMCFG2REG          ( RD_DLY | TYPE | ((BOARD_DRAM_DATABITS /16-1)<<4) | 1)
#endif
#endif

#if defined(DRAM_DDR3) || defined(DRAM_DDR2)
//Bruce, 100930, Porting into the Android Kernel
#if (1)
MEMCLK pIO_CKC_PLL3[]	=
{
	{0x41007E03,0x00200013, 1260000},
	{0x41008203,0x00200013, 1300000},
	{0x41008A03,0x00200013, 1380000},
	{0x41008D03,0x00200013, 1410000},
	{0x41009103,0x00200013, 1450000},
	{0x41009803,0x00200013, 1520000},
	{0x4100A003,0x00200013, 1600000},
	{0x00005503,0x00200013, 1700000},
	{0x00005A03,0x00200013, 1800000},
	{0x00005F03,0x00200013, 1900000},
	{0x00006403,0x00200013, 2000000},
	{0x00006903,0x00200013, 2100000},
	{0x00006E03,0x00200013, 2200000},
	{0x00007303,0x00200013, 2300000},
	{0x40007803,0x00200013, 2400000},
	{0x40007D03,0x00200013, 2500000},
	{0x40008203,0x00200013, 2600000},
	{0x40008703,0x00200013, 2700000},
	{0x40008C03,0x00200013, 2800000},
	{0x40009103,0x00200013, 2900000},
	{0x40009603,0x00200013, 3000000},
	{0x40009B03,0x00200013, 3100000},
	{0x4000A003,0x00200013, 3200000},
	{0x4000A503,0x00200013, 3300000},
	{0x4000AA03,0x00200013, 3400000},
	{0x4000AF03,0x00200013, 3500000},
	{0x4000B403,0x00200013, 3600000},
	{0x4000B903,0x00200013, 3700000},
	{0x4000BE03,0x00200013, 3800000},
	{0x4000C303,0x00200013, 3900000},
	{0x4000C803,0x00200013, 4000000},
};
#else
MEMCLK pIO_CKC_PLL1[]	=
{
	{0x0200FC03,0x00200011},//L"FMBUS_126Mhz", 
	{0x02010403,0x00200011},//L"FMBUS_130Mhz", 
	{0x02011403,0x00200011},//L"FMBUS_138Mhz",
	{0x02011A03,0x00200011},//L"FMBUS_141Mhz", 
	{0x02012203,0x00200011},//L"FMBUS_145Mhz",
	{0x02013003,0x00200011},//L"FMBUS_152Mhz", 
	{0x02014003,0x00200011},//L"FMBUS_160Mhz",
	{0x02015403,0x00200011},//L"FMBUS_170Mhz", 
	{0x42016803,0x00200011},//L"FMBUS_180Mhz",
	{0x42017C03,0x00200011},//L"FMBUS_190Mhz", 
	{0x42019003,0x00200011},//L"FMBUS_200Mhz",
	{0x4201A403,0x00200011},//L"FMBUS_210Mhz", 
	{0x4201B803,0x00200011},//L"FMBUS_220Mhz",
	{0x4201CC03,0x00200011},//L"FMBUS_230Mhz", 
	{0x4201E003,0x00200011},//L"FMBUS_240Mhz",
	{0x4201F403,0x00200011},//L"FMBUS_250Mhz", 
	{0x01010403,0x00200011},//L"FMBUS_260Mhz", 
	{0x01010E03,0x00200011},//L"FMBUS_270Mhz",
	{0x01011803,0x00200011},//L"FMBUS_280Mhz", 
	{0x01012203,0x00200011},//L"FMBUS_290Mhz",
	{0x01012C03,0x00200011},//L"FMBUS_300Mhz", 
	{0x01013803,0x00200011},//L"FMBUS_312Mhz", 
	{0x01014003,0x00200011},//L"FMBUS_320Mhz",
	{0x01014A03,0x00200011},//L"FMBUS_330Mhz", 
	{0x41016803,0x00200011},//L"FMBUS_360Mhz",

	{0x41017203,0x00200011},//L"FMBUS_370Mhz",
	{0x41017C03,0x00200011},//L"FMBUS_380Mhz",
	{0x41018603,0x00200011},//L"FMBUS_390Mhz",	
	
	{0x41019003,0x00200011},//L"FMBUS_400Mhz",
#if 0
	{0x41019A03,0x00200011},//L"FMBUS_410Mhz",
	{0x4101A403,0x00200011},//L"FMBUS_420Mhz",
	{0x4101AE03,0x00200011},//L"FMBUS_430Mhz",
	{0x4101B803,0x00200011},//L"FMBUS_440Mhz",	
	{0x4101C203,0x00200011},//L"FMBUS_450Mhz",
	{0x4101D603,0x00200011},//L"FMBUS_470Mhz",
	{0x4101EA03,0x00200011},//L"FMBUS_490Mhz",
	{0x0000FF03,0x00200011},//L"FMBUS_510Mhz",
#endif
};
#endif
#elif (0) //defined(DRAM_DDR2)
MEMCLK pIO_CKC_PLL1[]	=
{
	{0x0200FC03,0x00200011},//L"FMBUS_126Mhz", 
	{0x02010403,0x00200011},//L"FMBUS_130Mhz", 
	{0x02011403,0x00200011},//L"FMBUS_138Mhz",
	{0x02011A03,0x00200011},//L"FMBUS_141Mhz", 
	{0x02012203,0x00200011},//L"FMBUS_145Mhz",
	{0x02013003,0x00200011},//L"FMBUS_152Mhz", 
	{0x02014003,0x00200011},//L"FMBUS_160Mhz",
	{0x02015403,0x00200011},//L"FMBUS_170Mhz", 
	{0x42016803,0x00200011},//L"FMBUS_180Mhz",
	{0x42017C03,0x00200011},//L"FMBUS_190Mhz", 
	{0x42019003,0x00200011},//L"FMBUS_200Mhz",
	{0x4201A403,0x00200011},//L"FMBUS_210Mhz", 
	{0x4201B803,0x00200011},//L"FMBUS_220Mhz",
	{0x4201CC03,0x00200011},//L"FMBUS_230Mhz", 
	{0x4201E003,0x00200011},//L"FMBUS_240Mhz",
	{0x4201F403,0x00200011},//L"FMBUS_250Mhz", 
	{0x01010403,0x00200011},//L"FMBUS_260Mhz", 
	{0x01010E03,0x00200011},//L"FMBUS_270Mhz",
	{0x01011803,0x00200011},//L"FMBUS_280Mhz", 
	{0x01012203,0x00200011},//L"FMBUS_290Mhz",
	{0x01012C03,0x00200011},//L"FMBUS_300Mhz", 
	{0x01013803,0x00200011},//L"FMBUS_312Mhz", 
	{0x01014003,0x00200011},//L"FMBUS_320Mhz",
	{0x01014A03,0x00200011},//L"FMBUS_330Mhz", 
	{0x41016803,0x00200011},//L"FMBUS_360Mhz",
	{0x41019003,0x00200011},//L"FMBUS_400Mhz",
};	
#elif defined(DRAM_MDDR)
/*
	pll lchange_source	lchange_div lmem_source lmem_div;	
*/
MEMCLK1 pIO_CKC_PLL1[]	=
{
	{0x04011803, 2, 4, 1, 10},//L"FMBUS_7Mhz"
	{0x44019403, 2, 4, 1, 12},//L"FMBUS_8(8.42)Mhz",
	{0x04011803, 2, 4, 1, 8},//L"FMBUS_9(8.75)Mhz",
	{0x44019403, 2, 4, 1, 10},//L"FMBUS_10(10.1)Mhz",
	{0x03010803, 2, 4, 1, 12},//L"FMBUS_11Mhz",
	{0x03012403, 2, 4, 1, 12},//L"FMBUS_12(12.16)Mhz",
	{0x43018003, 2, 4, 1, 6},//L"FMBUS_14(13.71)Mhz",
	{0x4301B003, 2, 4, 1, 12},//L"FMBUS_18Mhz",
	{0x43019803, 2, 4, 1, 10},//L"FMBUS_20(20.4)Mhz",
	{0x03010803, 2, 4, 1, 6},//L"FMBUS_22Mhz",
	{0x44019403, 2, 4, 1, 4},//L"FMBUS_25(25.25)Mhz",
	{0x4301E803, 2, 4, 1, 8},//L"FMBUS_30(30.5)Mhz",,,
	{0x02011A03, 2, 4, 1, 8},//L"FMBUS_35(35.25)Mhz",,, 
	{0x02014003, 2, 4, 1, 8},//L"FMBUS_40Mhz",,,
	{0x43016803, 2, 4, 1, 4},//L"FMBUS_45Mhz",,, 
	{0x42019003, 2, 4, 1, 8},//L"FMBUS_50Mhz",,,
	{0x4201B803, 2, 4, 1, 8},//L"FMBUS_55Mhz",,, 
	{0x4201E003, 2, 4, 1, 8},//L"FMBUS_60Mhz",,, 
	{0x01010403, 2, 4, 1, 8},//L"FMBUS_65Mhz",,,
	{0x4201A403, 2, 4, 1, 6},//L"FMBUS_70Mhz",, 
	{0x4201C203, 2, 4, 1, 6},//L"FMBUS_75Mhz",,
	{0x02014003, 2, 4, 1, 4},//L"FMBUS_80Mhz", , 
	{0x02015403, 2, 4, 1, 4},//L"FMBUS_85Mhz",
	{0x42016803, 2, 4, 1, 4},//L"FMBUS_90Mhz",, 
	{0x42017C03, 2, 4, 1, 4},//L"FMBUS_95Mhz", 
	{0x42019003, 2, 4, 1, 4},//L"FMBUS_100Mhz" 
	{0x4201A403, 2, 4, 1, 4},//L"FMBUS_105Mhz"
	{0x4201B803, 2, 4, 1, 4},//L"FMBUS_110Mhz"
	{0x4201CC03, 2, 4, 1, 4},//L"FMBUS_115Mhz" 
	{0x4201E003, 2, 4, 1, 4},//L"FMBUS_120Mhz" 
	{0x4201F403, 2, 4, 1, 4},//L"FMBUS_125Mhz" 
	{0x02010403, 2, 4, 1, 2},//L"FMBUS_130Mhz" 
	{0x01010E03, 2, 4, 1, 4},//L"FMBUS_135Mhz"
	{0x01011803, 2, 4, 1, 4},//L"FMBUS_140Mhz",, 
	{0x01012203, 2, 4, 1, 4},//L"FMBUS_145Mhz",,
	{0x01012C03, 2, 4, 1, 4},//L"FMBUS_150Mhz",, 
	{0x02013803, 2, 4, 1, 2},//L"FMBUS_156Mhz",,
	{0x02014003, 2, 4, 1, 2},//L"FMBUS_160Mhz",, 
	{0x02014A03, 2, 4, 1, 2},//L"FMBUS_165Mhz",,
	{0x02015403, 2, 4, 1, 2},//L"FMBUS_170Mhz",, 
	{0x0200FC03, 2, 4, 1, 2},//L"FMBUS_176Mhz",,
	{0x42016803, 2, 4, 1, 2},//L"FMBUS_180Mhz",, 
	{0x42017203, 2, 4, 1, 2},//L"FMBUS_185Mhz",, 
};
#endif
	
static PGPIO pgpio;
#if defined(DRAM_DDR3)
void init_clockchange_ddr3(unsigned int lbusvalue)
{
	// TCC9300 Memory setting....
	// CPU: 600Mhz , BUS: 300Mhz, ddr3 256
	volatile int i;

	//ddr3
	*(volatile unsigned long *)addr(0x303020 ) = 0x0003010b ;//EMCCFG

	if(CHIP_BANK == 8)
		while (((*(volatile unsigned long *)addr(0x30c208)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
	else//CHIP_BANK is 4
		while (((*(volatile unsigned long *)addr(0x30c208)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED

//Clock setting..

	//mem bus - DirectPLL2/3(It is for asynchronous clock mode)
	*(volatile unsigned long *)addr(0x500008)  = 0x00200021;

	//MBUSCTRL - set asynchronous clock mode! cpubus/2
	*(volatile unsigned long *)addr(0x50002C)  = 0xFFFF0101;

	//set pll1 value
	*(volatile unsigned long *)addr(0x50003C)  = CKC_CHANGE_ARG(PLLVALUE_DIS_DDR3); // PLL_PWR_OFF & SET PMS
	*(volatile unsigned long *)addr(0x50003C) |= 0x80000000;			// PLL_PWR_ON

	//*(volatile unsigned long *)addr(0x500008)  = 0x00200013;
	*(volatile unsigned long *)addr(0x500008) = CKC_CHANGE_ARG(CLK_CTRL2_DDR3);

//Memory setting..

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
	if(CKC_CHANGE_ARG(DLLONOFF_DDR3))
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

    *(volatile unsigned long *)addr(0x30C004 ) = 0x0000018a  ;//MemControl
    if(BURST_LEN == BL_8)
        BITCSET(*(volatile unsigned long *)addr(0x30C004), 0x7<<7, 0x3<<7);
    else
        BITCSET(*(volatile unsigned long *)addr(0x30C004), 0x7<<7, 0x2<<7);

    if(CHIP_LOGICAL_NUM == 1)//1 chip
        BITCSET(*(volatile unsigned long *)addr(0x30C004), 0x3<<5, 0x0);
    else
        BITCSET(*(volatile unsigned long *)addr(0x30C004), 0x3<<5, 0x1<<5);

    //Chip0 Configuration
	//address mapping method - interleaved
	*(volatile unsigned long *)addr(0x30C008) = 0x40f01313  ;//MemConfig0i
    //set chip mask
    BITCSET(*(volatile unsigned long *)addr(0x30C008), 0xFF<<16, (0xFF - ((CHIP_LOGICAL_SIZE)/(CHIP_LOGICAL_NUM*0x10)-1))<<16);

    BITCSET(*(volatile unsigned long *)addr(0x30C008), 0xF<<8, (CHIP_COLBITS - 7)<<8);//set column bits
    BITCSET(*(volatile unsigned long *)addr(0x30C008), 0xF<<4, (CHIP_ROWBITS - 12)<<4);//set row bits
    if(CHIP_BANK == 8)//8 banks
        BITCSET(*(volatile unsigned long *)addr(0x30C008), 0xF, 0x3);
    else//4 banks
        BITCSET(*(volatile unsigned long *)addr(0x30C008), 0xF, 0x2);

    //Chip1 Configuration
    if(CHIP_LOGICAL_NUM == 2)
    {
		//address mapping method - interleaved
	    *(volatile unsigned long *)addr(0x30C00C) = 0x40f01313  ;//MemConfig1
        //set chip base
        BITCSET(*(volatile unsigned long *)addr(0x30C00C), 0xFF<<24, (0x40 - (CHIP_LOGICAL_SIZE)/(CHIP_LOGICAL_NUM*0x10))<<24);
        //set chip mask
        BITCSET(*(volatile unsigned long *)addr(0x30C00C), 0xFF<<16, (0xFF - ((CHIP_LOGICAL_SIZE)/(CHIP_LOGICAL_NUM*0x10)-1))<<16);

        BITCSET(*(volatile unsigned long *)addr(0x30C00C), 0xF<<8, (CHIP_COLBITS - 7)<<8);//set column bits
        BITCSET(*(volatile unsigned long *)addr(0x30C00C), 0xF<<4, (CHIP_ROWBITS - 12)<<4);//set row bits
        if(CHIP_BANK == 8)//8 banks
            BITCSET(*(volatile unsigned long *)addr(0x30C00C), 0xF, 0x3);
        else//4 banks
            BITCSET(*(volatile unsigned long *)addr(0x30C00C), 0xF, 0x2);
    }
	*(volatile unsigned long *)addr(0x30C000 ) = 0x40ff3010  ;//ConControl
	*(volatile unsigned long *)addr(0x30C014 ) = 0x01000000  ;//PrechConfig

	#if 1
	*(volatile unsigned long *)addr(0x30C100 ) = CKC_CHANGE_ARG(T_REFI);//AREF
	*(volatile unsigned long *)addr(0x30C104 ) = CKC_CHANGE_ARG(T_RFC);//TRFC
	*(volatile unsigned long *)addr(0x30C108 ) = CKC_CHANGE_ARG(T_RRD)	;//TRRD
	*(volatile unsigned long *)addr(0x30C10c ) = CKC_CHANGE_ARG(T_RP)	;//TRP 
	*(volatile unsigned long *)addr(0x30C110 ) = CKC_CHANGE_ARG(T_RCD)	;//TRCD
	*(volatile unsigned long *)addr(0x30C114 ) = CKC_CHANGE_ARG(T_RC)  ;//TRC 
	*(volatile unsigned long *)addr(0x30C118 ) = CKC_CHANGE_ARG(T_RAS)	;//TRAS
	*(volatile unsigned long *)addr(0x30C11c ) = CKC_CHANGE_ARG(T_WTR)	;//TWTR
	*(volatile unsigned long *)addr(0x30C120 ) = CKC_CHANGE_ARG(T_WR)	;//TWR 
	*(volatile unsigned long *)addr(0x30C124 ) = CKC_CHANGE_ARG(T_RTP)	;//TRTP
	*(volatile unsigned long *)addr(0x30C128 ) = CKC_CHANGE_ARG(CL) ;//CL
	*(volatile unsigned long *)addr(0x30C12c ) = CKC_CHANGE_ARG(WL) ;//WL
	*(volatile unsigned long *)addr(0x30C130 ) = CKC_CHANGE_ARG(RL) ;//RL
	*(volatile unsigned long *)addr(0x30C134 ) = CKC_CHANGE_ARG(T_FAW);//FAW 
	*(volatile unsigned long *)addr(0x30C138 ) = CKC_CHANGE_ARG(T_XSR) ;//TXS
	*(volatile unsigned long *)addr(0x30C13c ) = CKC_CHANGE_ARG(T_XP)	;//TXP 
	*(volatile unsigned long *)addr(0x30C140 ) = CKC_CHANGE_ARG(T_CKE)	;//TCKE
	*(volatile unsigned long *)addr(0x30C144 ) = CKC_CHANGE_ARG(T_MRD)	;//TMRD
	#else
	*(volatile unsigned long *)addr(0x30C100 ) = 0xC30 ;//AREF
	*(volatile unsigned long *)addr(0x30C104 ) = 0x2C	;//TRFC
	*(volatile unsigned long *)addr(0x30C108 ) = 0x4	;//TRRD
	*(volatile unsigned long *)addr(0x30C10c ) = 0x6	;//TRP 
	*(volatile unsigned long *)addr(0x30C110 ) = 0x6	;//TRCD
	*(volatile unsigned long *)addr(0x30C114 ) = 0x12  ;//TRC 
	*(volatile unsigned long *)addr(0x30C118 ) = 0xF  ;//TRAS
	*(volatile unsigned long *)addr(0x30C11c ) = 0x4	;//TWTR
	*(volatile unsigned long *)addr(0x30C120 ) = 0x6	;//TWR 
	*(volatile unsigned long *)addr(0x30C124 ) = 0x4	;//TRTP
	*(volatile unsigned long *)addr(0x30C128 ) = 0x6	;//CL
	*(volatile unsigned long *)addr(0x30C12c ) = 0x5	;//WL
	*(volatile unsigned long *)addr(0x30C130 ) = 0x6	;//RL
	*(volatile unsigned long *)addr(0x30C134 ) = 0x12;//FAW 
	*(volatile unsigned long *)addr(0x30C138 ) = 0x30 ;//TXS
	*(volatile unsigned long *)addr(0x30C13c ) = 0x3	;//TXP 
	*(volatile unsigned long *)addr(0x30C140 ) = 0x3	;//TCKE
	*(volatile unsigned long *)addr(0x30C144 ) = 0x4	;//TMRD
	#endif

	*(volatile unsigned long *)addr(0x30C010 ) = 0x08000000 ;//DirectCmd - XSR
	i = 50;
	while(i--);

	//Bruce, 101102, WR과 CL의 변경사항이 없으면 MRS Setting을 다시 하지 않는다.
	if(CKC_CHANGE_ARG(MRS_USE))
	{
		//after 500 us
		*(volatile unsigned long *)addr(0x30C010 ) = 0x07000000 ;//DirectCmd - NOP

		*(volatile unsigned long *)addr(0x30C010 ) = CKC_CHANGE_ARG(MR2_DDR3) ;//DirectCmd - MRS : MR2
		*(volatile unsigned long *)addr(0x30C010 ) = CKC_CHANGE_ARG(MR3_DDR3) ;//DirectCmd - MRS : MR3
		*(volatile unsigned long *)addr(0x30C010 ) = CKC_CHANGE_ARG(MR1_DDR3) ;//DirectCmd - MRS : MR1 : AL(0),Rtt_Nom(disable),OIC(RZQ/6) ,DLL(enable)

		//Bruce, 101102, for DLL Reset.
		//*(volatile unsigned long *)addr(0x30C010 ) = CKC_CHANGE_ARG(MR0_DDR3) ;//DirectCmd - MRS : MR0 : DLLPRE(off), WR(5), DLL Reset(Yes), MODE(0), CL(6), BL(8)
		{
			*(volatile unsigned long *)addr(0x30C010 ) = (CKC_CHANGE_ARG(MR0_DDR3))|0x100;//DirectCmd - MRS : MR0 : DLLPRE(off), WR(5), DLL Reset(Yes), MODE(0), CL(6), BL(8)
			*(volatile unsigned long *)addr(0x30C010 ) = 0x01000000;//precharge all
			*(volatile unsigned long *)addr(0x30C010 ) = 0x05000000;//AREF
			*(volatile unsigned long *)addr(0x30C010 ) = 0x05000000;//AREF
			*(volatile unsigned long *)addr(0x30C010 ) = 0x05000000;//AREF
			*(volatile unsigned long *)addr(0x30C010 ) = CKC_CHANGE_ARG(MR0_DDR3);	// DLL reset release.
			i = 50;
			while(i--);
		}
		
		*(volatile unsigned long *)addr(0x30C010 ) = 0x0a000400 ;//DirectCmd - ZQCL
	}

	*(volatile unsigned long *)addr(0x30C000 ) = 0x60ff3030  ;//ConControl
	*(volatile unsigned long *)addr(0x303020 ) = 0x0007010b ;//EMCCFG

	if(CHIP_BANK == 8)
		while (((*(volatile unsigned long *)addr(0x30c208)) & 0xFFFFFFFF)!= 0x44444444); //Wait PAUSED
	else//CHIP_BANK is 4
		while (((*(volatile unsigned long *)addr(0x30c208)) & 0x0000FFFF)!= 0x00004444); //Wait PAUSED
}
#elif defined(DRAM_DDR2)
void init_clockchange_ddr2(unsigned int lbusvalue)
{
#if defined(DDR2CONTROLLER_LPCON)
	volatile unsigned int i = 0;			

	*(volatile unsigned long *)addr(0x303020 ) &= ~Hw17;

	if(DRAM_PHY_BANKBIT == 2)
	{
		while (((*(volatile unsigned long *)addr(0x305048)) & 0x0000FFFF)!= 0x00003333); //Wait PAUSED
	}
	else//DRAM_PHY_BANKBIT == 3
	{
		while (((*(volatile unsigned long *)addr(0x305048)) & 0xFFFFFFFF)!= 0x33333333); //Wait PAUSED
	}

//SangWon, 101014, Porting into the Android Kernel
#if (1)
	{
		//mem bus - DirectPLL2/3(It is for asynchronous clock mode)
		*(volatile unsigned long *)addr(0x500008)  = 0x00200021;

		//MBUSCTRL - set asynchronous clock mode! cpubus/2
		*(volatile unsigned long *)addr(0x50002C)  = 0xFFFF0101;

		//set pll1 value
		*(volatile unsigned long *)addr(0x50003C)  = CKC_CHANGE_ARG(PLLVALUE_DIS); // PLL_PWR_OFF & SET PMS
		*(volatile unsigned long *)addr(0x50003C) |= 0x80000000;			// PLL_PWR_ON

		//*(volatile unsigned long *)addr(0x500008)  = 0x00200013;
		*(volatile unsigned long *)addr(0x500008) = CKC_CHANGE_ARG(CLK_CTRL2);
	}
#else
	#if defined(FCPU_800MHZ_ALL_BUS_400MHZ_INCLUDE)
		
	#else
		*(volatile unsigned long *)addr(0x500008) = 0x00200014;
		*(volatile unsigned long *)addr(0x500034)  &= ~0x80000000  ; // PLL_PWR_ON
		*(volatile unsigned long *)addr(0x500034)  = CKC_CHANGE_ARG(PLLVALUE_DIS)  ; // PMS_600M
		*(volatile unsigned long *)addr(0x500034)  |= 0x80000000  ; // PLL_PWR_ON
		//*(volatile unsigned long *)addr(0x500008) = 0x00200011;
		
		if( (*(volatile unsigned long *)addr(0x50002C) & 1) == 0 )
		{
		  	//Synchronous clock mode -> Asynchronous clock mode
			#if 1
			*(volatile unsigned long *)addr(0x600004) |= Hw7;
			*(volatile unsigned long *)addr(0x60000C) = Hw7;
			
			*(volatile unsigned long *)addr(0x50002C) &= 0x000FFFFF;
			*(volatile unsigned long *)addr(0x50002C) |= 0x80*Hw20;
			*(volatile unsigned long *)addr(0x50002C) |= 1;
			
			__asm__ __volatile__ ("mov r1, #0\n"
														"mcr p15, 0, r1, c7, c0, 4\n");
														
			*(volatile unsigned long *)addr(0x60000C) = Hw7;
			*(volatile unsigned long *)addr(0x600004) &= ~Hw7;
			#else
			*(volatile unsigned long *)addr(0x600004) |= Hw7;
			
			*(volatile unsigned long *)addr(0x50002C) &= 0x000FFFFF;
			*(volatile unsigned long *)addr(0x50002C) |= 0x80*Hw20;
			*(volatile unsigned long *)addr(0x50002C) |= 1;
			
			i = 128;
			while(i)
			{
				i--;
			}
			#endif
		}
		//membus src is PLL1
		*(volatile unsigned long *)addr(0x500008) = CKC_CHANGE_ARG(CLK_CTRL2);
	#endif
#endif

	//phy configuration
	*(volatile unsigned long *)addr(0x303024 ) = 0x200;//PHYCFG

	//PhyZQControl
	if (lbusvalue >= 2000000) {
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL ;
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL | Hw1 ;//zq start
	}
	else {
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL | Hw0;
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL | Hw1 | Hw0 ;//zq start
	}
	while (((*(volatile unsigned long *)addr(0x305040)) & (0x10000)) != 0x10000);	// Wait until ZQ End

	if (lbusvalue >= 2000000) {
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL ;
	}
	else {
		*(volatile unsigned long *)addr(0x305044 ) = PHYZQCTRL | Hw0;
	}

	//PHY Control0
	*(volatile unsigned long *)addr(0x305018 ) = 0x0010100A;

	// modify by crony : [31:30] : ODT Enable for Write and Read
	//PHY Control1
	*(volatile unsigned long *)addr(0x30501C ) = 0xE0000086;

	//PHY Control2
	*(volatile unsigned long *)addr(0x305020 ) = 0x0;
	//PHY Control3
	*(volatile unsigned long *)addr(0x305024 ) = 0x0;
	//PHY Control0
	*(volatile unsigned long *)addr(0x305018 ) = 0x0010100B;

	while (((*(volatile unsigned long *)addr(0x305040)) & (0x7)) != 0x7);// Wait until FLOCK == 1

	//PHY Control1
	*(volatile unsigned long *)addr(0x30501C) = 0xE000008E; //resync = 1

	*(volatile unsigned long *)addr(0x30501C) = 0xE0000086; //resync = 0

	//Enable Out of order scheduling
#if (defined(CONFIG_DDR2_HXB18T2G160AF) || defined(CONFIG_DDR2_HY5PS1G831CFPS6))
	*(volatile unsigned long *)addr(0x305000 ) = 0x30FF2018;
#else
	if(*(volatile unsigned long *)addr(0x305000 ) != (0x30FF2088 | Hw4))
		*(volatile unsigned long *)addr(0x305000 ) = 0x30FF2088 | Hw4;
#endif
	
	//MEMCTRL
	*(volatile unsigned long *)addr(0x305004 ) = MEMCTRL;
	//MEMCHIP0
	*(volatile unsigned long *)addr(0x305008 ) = MEMCHIP0CFG;
	//MEMCHIP1
	if(BOARD_DRAM_LOGICAL_NUM == 2)
		*(volatile unsigned long *)addr(0x30500C ) = MEMCHIP1CFG;
	//PRECONFIG
	*(volatile unsigned long *)addr(0x305014 ) = 0x0;

	//PRECONFIG
	*(volatile unsigned long *)addr(0x305028 ) = 0xFFFF00FF;

	//tAREF
	//*(volatile unsigned long *)addr(0x305030 ) = 0x507;
	*(volatile unsigned long *)addr(0x305030 ) = CKC_CHANGE_ARG(TAREF);
	//tROW
	*(volatile unsigned long *)addr(0x305034 ) = CKC_CHANGE_ARG(TROW);
	//tDATA
	*(volatile unsigned long *)addr(0x305038 ) = CKC_CHANGE_ARG(TDATA);
	//tPWR
	*(volatile unsigned long *)addr(0x30503C ) = CKC_CHANGE_ARG(TPOWER);

	//Direct Command
	*(volatile unsigned long *)addr(0x305010 ) = 0x07000000;//NOP
	if(BOARD_DRAM_LOGICAL_NUM == 2)
		*(volatile unsigned long *)addr(0x305010 ) = (0x07000000 | Hw20);	

	*(volatile unsigned long *)addr(0x305010 ) = 0x01000000;//precharge all
	if(BOARD_DRAM_LOGICAL_NUM == 2)
		*(volatile unsigned long *)addr(0x305010 ) = (0x01000000 | Hw20);	

#if (defined(CONFIG_DDR2_HXB18T2G160AF) || defined(CONFIG_DDR2_HY5PS1G831CFPS6))
	*(volatile unsigned long *)addr(0x305010 ) = 0x00020000;
	
	*(volatile unsigned long *)addr(0x305010 ) = 0x00030000;

	*(volatile unsigned long *)addr(0x305010 ) = 0x00010000;

	*(volatile unsigned long *)addr(0x305010 ) = 0x00000100;

	*(volatile unsigned long *)addr(0x305010 ) = 0x01000000;//precharge all

	i = 3;
	while(i)
	{
		*(volatile unsigned long *)addr(0x305010 ) = 0x05000000;//AREF
		i--;
	}// repeat end
	
	*(volatile unsigned long *)addr(0x305010 ) = 0x00000000;	// DLL reset release.
	// wait until 200 mclk
	*(volatile unsigned long *)addr(0x305010 ) = (0x00000002 | (5<<4) | (CKC_CHANGE_ARG(TWR)<<9));	// BurstLength 4, CL 5, WR 5

	i = 100;
	while(i)
	{
		i--;
	}

	*(volatile unsigned long *)addr(0x305010 ) = 0x00010004;	// OCD Calibration default
#else
	i = 2;
	while(i)
	{
		*(volatile unsigned long *)addr(0x305010 ) = 0x05000000;//AREF
		if(BOARD_DRAM_LOGICAL_NUM == 2)
			*(volatile unsigned long *)addr(0x305010 ) = (0x05000000 | Hw20);	
		i--;
	}		// repeat end

	*(volatile unsigned long *)addr(0x305010 ) = 0x07000000;//NOP
		if(BOARD_DRAM_LOGICAL_NUM == 2)
			*(volatile unsigned long *)addr(0x305010 ) = (0x07000000 | Hw20);	

	*(volatile unsigned long *)addr(0x305010 ) = 0x00020000;
	if(BOARD_DRAM_LOGICAL_NUM == 2)
		*(volatile unsigned long *)addr(0x305010 ) = (0x00020000 | Hw20);	
	
	*(volatile unsigned long *)addr(0x305010 ) = 0x00030000;
	if(BOARD_DRAM_LOGICAL_NUM == 2)
		*(volatile unsigned long *)addr(0x305010 ) = (0x00030000 | Hw20);	

	*(volatile unsigned long *)addr(0x305010 ) = 0x00010000 | (Hw6 /*| Hw3*/); //ODT 150ohm
	if(BOARD_DRAM_LOGICAL_NUM == 2)
		*(volatile unsigned long *)addr(0x305010 ) = (0x00010000 | (Hw6 /*| Hw3*/) | Hw20);	//ODT 150ohm	

	// wait until 200 mclk
	if(DRAM_PHY_CAS >= 5)
	{
		*(volatile unsigned long *)addr(0x305010 ) = (0x00000002 | (5<<4) | (CKC_CHANGE_ARG(TWR)<<9) | (1<<8));	
		if(BOARD_DRAM_LOGICAL_NUM == 2)
			*(volatile unsigned long *)addr(0x305010 ) = (0x00000002 | Hw20 | (5<<4) | (CKC_CHANGE_ARG(TWR)<<9) | (1<<8));	
	}
	else
	{
		*(volatile unsigned long *)addr(0x305010 ) = (0x00000002 | (DRAM_PHY_CAS<<4) | (CKC_CHANGE_ARG(TWR)<<9) | (1<<8));	
		if(BOARD_DRAM_LOGICAL_NUM == 2)
			*(volatile unsigned long *)addr(0x305010 ) = (0x00000002 | Hw20 | (DRAM_PHY_CAS<<4) | (CKC_CHANGE_ARG(TWR)<<9) | (1<<8));	
	}
#endif

//	*(volatile unsigned long *)addr(0x303020 ) |=  Hw17;//EMCCFG
	*(volatile unsigned long *)addr(0x303020 ) =  0x0007010A;//EMCCFG
	*(volatile unsigned long *)addr(0x305000 ) |= 0x20;

#if 1
	if(DRAM_PHY_BANKBIT == 2)
		while (((*(volatile unsigned long *)addr(0x305048)) & 0x0000FFFF)!= 0x00004444); //Wait PAUSED
	else//DRAM_PHY_BANKBIT == 3
		while (((*(volatile unsigned long *)addr(0x305048)) & 0xFFFFFFFF)!= 0x44444444); //Wait PAUSED
#else
	i = 10;
	while(i--);
#endif

#else//below is PL341	
	volatile unsigned int i = 0;	

	//memory setting start	
	*(volatile unsigned long *)addr(0x302004)   = 0x00000003  ; // PL341_PAUSE	
	while (((*(volatile unsigned long *)addr(0x302000)) & 0x3)!= 2); //Wait PL34X_STATUS_PAUSE
	*(volatile unsigned long *)addr(0x302004)   = 0x00000004  ; // PL341_CONFIGURE
	while (((*(volatile unsigned long *)addr(0x302000)) & 0x3)!= 0); //Wait PL34X_STATUS_ONFIGURE

//SangWon, 101014, Porting into the Android Kernel
#if (1)
	{
		//mem bus - DirectPLL2/3(It is for asynchronous clock mode)
		*(volatile unsigned long *)addr(0x500008)  = 0x00200021;

		//MBUSCTRL - set asynchronous clock mode! cpubus/2
		*(volatile unsigned long *)addr(0x50002C)  = 0xFFFF0101;

		//set pll1 value
		*(volatile unsigned long *)addr(0x50003C)  = CKC_CHANGE_ARG(PLLVALUE_DIS_DDR3); // PLL_PWR_OFF & SET PMS
		*(volatile unsigned long *)addr(0x50003C) |= 0x80000000;			// PLL_PWR_ON

		//*(volatile unsigned long *)addr(0x500008)  = 0x00200013;
		*(volatile unsigned long *)addr(0x500008) = CKC_CHANGE_ARG(CLK_CTRL2_DDR3);
	}
#else	
	#if defined(FCPU_800MHZ_ALL_BUS_400MHZ_INCLUDE)
	//cpu bus - DirectXIN, Synchronous clock mode 
	*(volatile unsigned long *)0xB0500000 = 0x002ffff4;
	//io bus - DirectXIN/2
	*(volatile unsigned long *)0xB0500010 = 0x00200014;
	*(volatile unsigned long *)addr(0x500010) = 0x00200024;
	i = 3200; while(i) i--;
	
	//Set PLL0
	//PLL0 - power off : 800MHz, P(3), M(400), S(1), VSEL(1)
	*(volatile unsigned long *)addr(0x500030)= 0x41019003;
	//PLL0 - power on
	*(volatile unsigned long *)addr(0x500030)= 0xC1019003;		
	
	//cpu bus - DirectPLL0, 800Mhz
	*(volatile unsigned long *)addr(0x500000) = 0x002FFFF0;
	*(volatile unsigned long *)addr(0x500010) = 0x00200010;
	//io bus - DirectPLL2, 800MHz/2 = 400MHz
	*(volatile unsigned long *)addr(0x500010) = 0x00200010;
	i = 3200; while(i) i--;
	#else
 	*(volatile unsigned long *)addr(0x500008) = 0x00200014;
 	*(volatile unsigned long *)addr(0x500034)  &= ~0x80000000  ; // PLL_PWR_ON
	*(volatile unsigned long *)addr(0x500034)  = CKC_CHANGE_ARG(PLLVALUE_DIS)  ; // PMS_600M
	*(volatile unsigned long *)addr(0x500034)  |= 0x80000000  ; // PLL_PWR_ON
	//*(volatile unsigned long *)addr(0x500008) = 0x00200011;
	
	if( (*(volatile unsigned long *)addr(0x50002C) & 1) == 0 )
	{
			#if 1//Synchronous clock mode -> Asynchronous clock mode
			*(volatile unsigned long *)addr(0x600004) |= Hw7;
			*(volatile unsigned long *)addr(0x60000C) = Hw7;
			
			*(volatile unsigned long *)addr(0x50002C) &= 0x000FFFFF;
			*(volatile unsigned long *)addr(0x50002C) |= 0x80*Hw20;
			*(volatile unsigned long *)addr(0x50002C) |= 1;
			
			__asm__ __volatile__ ("mov r1, #0\n"
														"mcr p15, 0, r1, c7, c0, 4\n");
														
			*(volatile unsigned long *)addr(0x60000C) = Hw7;
			*(volatile unsigned long *)addr(0x600004) &= ~Hw7;
			
			#else
			*(volatile unsigned long *)addr(0x600004) |= Hw7;
			
			*(volatile unsigned long *)addr(0x50002C) &= 0x000FFFFF;
			*(volatile unsigned long *)addr(0x50002C) |= 0x80*Hw20;
			*(volatile unsigned long *)addr(0x50002C) |= 1;
			
			i = 128;
			while(i)
			{
				i--;
			}
			#endif
	}
	//membus src is PLL1
	*(volatile unsigned long *)addr(0x500008) = CKC_CHANGE_ARG(CLK_CTRL2);
	#endif
#endif

	/*
	*(volatile unsigned long *)addr(0x304428)    = 0x00500000   ; // ZQCAL
	*(volatile unsigned long *)addr(0x304428)    = 0x00500038   ; // ZQCAL
	*(volatile unsigned long *)addr(0x304428)    = 0x005001F8   ; // ZQCAL
	*(volatile unsigned long *)addr(0x304428)    = 0x005001F9   ; // ZQCAL
	while (((*(volatile unsigned long *)addr(0x30442C)) & 0x1)!=1); //Wait flock
	*(volatile unsigned long *)addr(0x304428)    = 0x005001F8   ; // ZQCAL
	*/
	
	// timing setting
	*(volatile unsigned long *)addr(0x302014)     =  CKC_CHANGE_ARG(TCAS);
	*(volatile unsigned long *)addr(0x30201c)     =  3  								;
	*(volatile unsigned long *)addr(0x302020)     =  CKC_CHANGE_ARG(TRAS)  ; 
	*(volatile unsigned long *)addr(0x302024)     =  CKC_CHANGE_ARG(TRC); 
	*(volatile unsigned long *)addr(0x302028)     =  CKC_CHANGE_ARG(TRCD);
	*(volatile unsigned long *)addr(0x30202c)     =  CKC_CHANGE_ARG(TRFC);
	*(volatile unsigned long *)addr(0x302030)     =  CKC_CHANGE_ARG(TRP); 
	*(volatile unsigned long *)addr(0x302034)     =  CKC_CHANGE_ARG(TRRD);
	*(volatile unsigned long *)addr(0x302038)     =  CKC_CHANGE_ARG(TWR); 
	*(volatile unsigned long *)addr(0x30203c)     =  CKC_CHANGE_ARG(TWTR);
	*(volatile unsigned long *)addr(0x302040)     =  CKC_CHANGE_ARG(TXP); 
	*(volatile unsigned long *)addr(0x302044)	  	=	 CKC_CHANGE_ARG(TXSR);
	*(volatile unsigned long *)addr(0x302048)     =	 CKC_CHANGE_ARG(TESR);
	*(volatile unsigned long *)addr(0x302054)	  	=	 CKC_CHANGE_ARG(TFAW);
		
	*(volatile unsigned long *)addr(0x30200C) 		|= 0x00140000; 
	*(volatile unsigned long *)addr(0x30200c)     =  CONFIGREG2   ; // CFG0
	
	*(volatile unsigned long *)addr(0x302010)     =  CKC_CHANGE_ARG(TREFRESH)   ; // REFRESH

	*(volatile unsigned long *)addr(0x30204c)     =  MEMCFG2REG   ; //CFG2
		
	if(BOARD_DRAM_TOTAL == 128)//using 27bit address
		*(volatile unsigned long *)addr(0x302200)=0x000040F8; // config_chip0 - CS0 - 0x40000000~0x47ffffff
	else if(BOARD_DRAM_TOTAL == 256) //using 28bit address
		*(volatile unsigned long *)addr(0x302200)=0x000040F0; // config_chip0 - CS0 - 0x40000000~0x47ffffff
	else
	*(volatile unsigned long *)addr(0x302200)=0x000040E0; // config_chip0 -
	
	*(volatile unsigned long *)addr(0x303024) 	   =  0x00000000 ;//PHYCTRL
	//*(volatile unsigned long *)addr(0x30302c)      =  0x00004000 ;//SSTL
	*(volatile unsigned long *)addr(0x30302c)      |=  0x00007fff ;//SSTL
	
	*(volatile unsigned long *)addr(0x303020)= COMMONREG; // emccfg_config0
	
			
	*(volatile unsigned long *)addr(0x304400)     =  0x00000808; //ddr2, differential DQS, gate siganl for ddr2, differential receiver
	*(volatile unsigned long *)addr(0x304404) 	  =	 0x00080001;//  DLL-On
		
	*(volatile unsigned long *)addr(0x304408) = CKC_CHANGE_ARG(EMR1); // dll phase detect 
	
	*(volatile unsigned long *)addr(0x304404) 	  =	 0x00080003;// dll-on|start
	
	while (((*(volatile unsigned long *)addr(0x304404)) & 0x18)!= 0x18); //Wait PL34X_STATUS_PAUSED
	
	*(volatile unsigned long *)addr(0x304424)  	  =	 0x00000035 ;// DLLFORCELOCK
	*(volatile unsigned long *)addr(0x30440c)     =  0x00000006 ;//  GATECTRLt 
	*(volatile unsigned long *)addr(0x304430)  	  =	 DRAM_PHY_CAS;// RDDELAY
	
	
	//calibration start  with forcing option (added by jykim)
	if(ZQ_CAL_START)
	{
		if(CKC_CHANGE_ARG(EMR1+1) == 0)
		{
				*(volatile unsigned long *)addr(0x304428) =  0x0056f581;	// term_disable
		}
		else
		{
			*(volatile unsigned long *)addr(0x304428) =  (0x00560000|
															ZQ_DRV_STR|
															ZQ_TERM_DIS|
															ZQ_TERM_VAL|
															ZQ_PULL_DOWN|
															ZQ_PULL_UP|
															ZQ_ZQ|
															//ZQ_UPDATE|
															ZQ_CAL_START);	
		}
	
		while (!((*(volatile unsigned long *)addr(0x30442c)) & (1)));	// Wait until Calibration completion without error
	
		if(CKC_CHANGE_ARG(EMR1+1) == 0)
		{
			*(volatile unsigned long *)addr(0x304428) =  0x0056f583;	// CAL_START
			*(volatile unsigned long *)addr(0x304428) =  0x0056f581;	// CAL_START
		}
		else
		{
			*(volatile unsigned long *)addr(0x304428) =  (0x00560000|
															ZQ_DRV_STR|
															ZQ_TERM_DIS|
															ZQ_TERM_VAL|
															ZQ_PULL_DOWN|
															ZQ_PULL_UP|
															ZQ_ZQ|
															ZQ_UPDATE|
															ZQ_CAL_START);	
			
			*(volatile unsigned long *)addr(0x304428) =  (0x00560000|
															ZQ_DRV_STR|
															ZQ_TERM_DIS|
															ZQ_TERM_VAL|
															ZQ_PULL_DOWN|
															ZQ_PULL_UP|
															ZQ_ZQ|
															//ZQ_UPDATE|
															ZQ_CAL_START);	
		}
	}
	else
	{
		if(CKC_CHANGE_ARG(EMR1+1) == 0)
		{
			*(volatile unsigned long *)addr(0x304428) =  0x0056f581;	// term_disable
		}
		else
		{
			*(volatile unsigned long *)addr(0x304428) =  (0x00560000|
															ZQ_DRV_STR|
															ZQ_TERM_DIS|
															ZQ_TERM_VAL|
															ZQ_PULL_DOWN|
															ZQ_PULL_UP|
															ZQ_ZQ|
															//ZQ_UPDATE|
															ZQ_CAL_START);	
		}
	}

	*(volatile unsigned long *)addr(0x302008)     =	 0x000c0000   ;//NOP
	
	// repeat 400ns
	i = 100;
	while(i)
	{
		i--;
	}
	
	*(volatile unsigned long *)addr(0x302008)     =	 0x00000000   ;// pre-charge all
	*(volatile unsigned long *)addr(0x302008)     =	 0x00040000   ;// auto-refresh
	*(volatile unsigned long *)addr(0x302008)     =	 0x00040000   ;// auto-refresh
	
	//MR
	//update write recovery time to avoid dram buffer overwrite
	*(volatile unsigned long *)addr(0x302008) = (0x00080002|(DRAM_PHY_CAS<<4)|((CKC_CHANGE_ARG(TWR))<<9)|(1<<8)); //EMR1	
	*(volatile unsigned long *)addr(0x302008) &= ~(1<<8);
	
	//EMR1
	*(volatile unsigned long *)addr(0x302008) = 0x00090000; 		// Direct COmmnad Register 
	*(volatile unsigned long *)addr(0x302008) = 0x00090380; 		// Direct COmmnad Register 
	if(CKC_CHANGE_ARG(EMR1+1) == 0)
		*(volatile unsigned long *)addr(0x302008) = 0x00090000; 		// Direct COmmnad Register //soc1-3
	else
		*(volatile unsigned long *)addr(0x302008) = 0x00090004; 		// Direct COmmnad Register //soc1-3
		
	*(volatile unsigned long *)addr(0x302008)     =	 0x000a0000   ;// EMRS2
	*(volatile unsigned long *)addr(0x302008)     =	 0x000b0000   ;/// EMRS3
	
	// repeat 100
	i = 100;
	while(i)
	{
		*(volatile unsigned long *) addr(0x302008) =0x00040000 ; //auto-refresh
		i--;
	}
	*(volatile unsigned long *)addr(0x302004)     =	 0x00000000    ;// PL341_GO
#endif	
}
#elif defined(DRAM_MDDR) || defined(CONFIG_DRAM_MDDR)
void init_clockchange_mddr(unsigned int lbusvalue)
{
#if 0
	volatile unsigned int i = CKC_CHANGE_ARG(TCAS);
	*(volatile unsigned long *)addr(0x301004) = 0x00000003; 		// PL341_PAUSE
	while (((*(volatile unsigned long *)addr(0x301000)) & 0x3)!=2); //Wait PL34X_STATUS_PAUSED

	*(volatile unsigned long *)addr(0x301004) = 0x00000004; 		// PL341_Configure
	while (((*(volatile unsigned long *)addr(0x301000)) & 0x3)!=0); //Wait PL34X_STATUS_CONFIG

// DLL OFF
	*(volatile unsigned long *)addr(0x304404) &=  ~(0x00000003);	// DLL-0FF,DLL-Stop running
	*(volatile unsigned long *)addr(0x304428) &= ~(0x00000003); 	// Calibration Start,Update Calibration
	*(volatile unsigned long *)addr(0x30302C) &=  ~(0x00004000);	//SDRAM IO Control Register Gatein Signal Power Down

	//Clock Change	
	//mem bus - DirectPLL2/3(It is for asynchronous clock mode) 
	*(volatile unsigned long *)addr(0x500008) = 0x00200022;
	//MBUSCTRL - set asynchronous clock mode! cpubus/2
	*(volatile unsigned long *)addr(0x500008) = 0x00800101;
	
	//PLL1
	*(volatile unsigned long *)addr(0x500034)= 0x0000fa03;		// pll pwr off
	*(volatile unsigned long *)addr(0x500034)= CKC_CHANGE_ARG(PLLVALUE_DIS);
	*(volatile unsigned long *)addr(0x500034)|= 0x80000000;
	*(volatile unsigned long *)addr(0x500008)= CKC_CHANGE_ARG(CLK_CTRL2);

	*(volatile unsigned long *)addr(0x30104C)= MEMCFG2REG;

	*(volatile unsigned long *)addr(0x30100C) = CONFIGREG1;
	*(volatile unsigned long *)addr(0x303000) |= 0x00800000;		// bit23 enable -synopt enable
	*(volatile unsigned long *)addr(0x303010) |= 0x00800000;		// bit23 enable -synopt enable

	//*(volatile unsigned long *)addr(0x301010) = CKC_CHANGE_ARG(TREFRESH);; // refresh_prd = 1000
	*(volatile unsigned long *)addr(0x301010) = 0x000003E8; // refresh_prd = 1000

	*(volatile unsigned long *)addr(0x301014) = i; // cas_latency = 3
	*(volatile unsigned long *)addr(0x30101C) = 0x00000002; // tMRD 2tck 

	*(volatile unsigned long *)addr(0x301020) = CKC_CHANGE_ARG(TRAS); // tRAS 
	*(volatile unsigned long *)addr(0x301024) = CKC_CHANGE_ARG(TRC); // tRC 
	*(volatile unsigned long *)addr(0x301028) = CKC_CHANGE_ARG(TRCD); // tRCD 
	*(volatile unsigned long *)addr(0x30102c) = CKC_CHANGE_ARG(TRFC); // tRFC 
	*(volatile unsigned long *)addr(0x301030) = CKC_CHANGE_ARG(TRP); // tRP 
	*(volatile unsigned long *)addr(0x301034) = CKC_CHANGE_ARG(TRRD); // tRRD 
	*(volatile unsigned long *)addr(0x301038) = CKC_CHANGE_ARG(TWR); // tWR 
	*(volatile unsigned long *)addr(0x30103c) = CKC_CHANGE_ARG(TWTR); // tWTR 
	
	//no data on sheet
	*(volatile unsigned long *)addr(0x301040) = CKC_CHANGE_ARG(TXP); // tXP
	*(volatile unsigned long *)addr(0x301044) = CKC_CHANGE_ARG(TXSR); // tXSR 
	*(volatile unsigned long *)addr(0x301048) = CKC_CHANGE_ARG(TESR); // tESR


	*(volatile unsigned long *)addr(0x304404) = 0x00000001; 		// DLLCTRL
//	*(volatile unsigned long *)addr(0x304404) = 0x00000003; 		// DLLCTRL
//	while (((*(volatile unsigned long *)addr(0x304404)) & (0x00000018)) != (0x00000018)); // Wait DLL Lock
	
	
	// 1CS
	if(BOARD_DRAM_TOTAL == 64)
		*(volatile unsigned long *)addr(0x301200)=0x000040FC; // config_chip0 - CS0 - 0x40000000~0x47ffffff
	if(BOARD_DRAM_TOTAL == 128)
		*(volatile unsigned long *)addr(0x301200)=0x000040F8; // config_chip0 - CS0 - 0x40000000~0x47ffffff
	if(BOARD_DRAM_TOTAL == 256)
		*(volatile unsigned long *)addr(0x301200)=0x000040F0; // config_chip0 - CS0 - 0x40000000~0x47ffffff
	
	*(volatile unsigned long *)addr(0x304428) = 0x6f551;
	while (!((*(volatile unsigned long *)addr(0x30442c)) & (1)));	// Wait until Calibration completion without error
	*(volatile unsigned long *)addr(0x304428) = 0x6f553;
	*(volatile unsigned long *)addr(0x304428) = 0x6f551;

	if( (i>>1) == 3){
		*(volatile unsigned long *)addr(0x301008) = 0x00000032; //MRS
		*(volatile unsigned long *)addr(0x301008) = 0x000a0000;//EMRS
		*(volatile unsigned long *)addr(0x301008) = 0x00080032;
		*(volatile unsigned long *)addr(0x301008) = 0x00040032;	
		*(volatile unsigned long *)addr(0x301008) = 0x00040032;	
		*(volatile unsigned long *)addr(0x301008) = 0x00040032;	
		*(volatile unsigned long *)addr(0x301008) = 0x00040032;	
	}
	else{
		*(volatile unsigned long *)addr(0x301008) = 0x00000022; //MRS
		*(volatile unsigned long *)addr(0x301008) = 0x000a0000;//EMRS
		*(volatile unsigned long *)addr(0x301008) = 0x00080022;
		*(volatile unsigned long *)addr(0x301008) = 0x00040022;
		*(volatile unsigned long *)addr(0x301008) = 0x00040022;
		*(volatile unsigned long *)addr(0x301008) = 0x00040022;
	}

	*(volatile unsigned long *)addr(0x301004)=0x00000000; // PL341_GO
	while (((*(volatile unsigned long *)addr(0x301000)) & (0x03)) != 1);	// Wait until READY
#endif
}
#endif


void ckc_delay(unsigned int cnt)
{
	volatile unsigned int count;
	count = cnt*10000;
	while(count--);
}

void ckc_etcblock(unsigned int lMask)
{
#if defined(__WINCE__)
	PUSBOTGCFG  pOTGCFG  = (PUSBOTGCFG)(OALPAtoVA((unsigned int)&HwUSB20OTG_BASE,FALSE));
	PGRPBUSCONFIG  pGPUGRPBUSCONFIG  = (PGRPBUSCONFIG)(OALPAtoVA((unsigned int)&HwGRPBUSCONFIG_BASE,FALSE));
#else
	volatile PUSBOTGCFG  pOTGCFG  = (volatile PUSBOTGCFG)tcc_p2v(HwUSB20OTG_BASE);
	volatile PGRPBUSCONFIG  pGPUGRPBUSCONFIG  = (volatile PGRPBUSCONFIG)tcc_p2v(HwGRPBUSCONFIG_BASE);
#endif	

// Disable
	if(lMask & ETCMASK_USBPHYOFF)
	{
#if defined(_WINCE_)
		OALMSG(TC_LOG_LEVEL(TC_DEBUG), (L"[KERNEL      ]ETCMASK_USBPHYOFF\r\n"));
#else
        dbg("%s: ETC_USBPHYOFF\n", __func__);
#endif
		BITCSET(pOTGCFG->UPCR2,Hw10|Hw9,Hw9);
		pOTGCFG->UPCR0 = 0x4840;
		pOTGCFG->UPCR0 = 0x6940;
	}

#if !defined(MAX_FGRP_EN)
	if(lMask & ETCMASK_3DGPUOFF)
	{
#if defined(_WINCE_)
		OALMSG(TC_LOG_LEVEL(TC_DEBUG), (L"[KERNEL      ]ETCMASK_3DGPUOFF\r\n"));
#else
        dbg("%s: ETCMASK_3DGPUOFF\n", __func__);
#endif
		pGPUGRPBUSCONFIG->GRPBUS_PWRDOWN |= Hw0;
	}
	
	if(lMask & ETCMASK_OVERLAYMIXEROFF)
	{
#if defined(_WINCE_)
		OALMSG(TC_LOG_LEVEL(TC_DEBUG), (L"[KERNEL      ]ETCMASK_OVERLAYMIXEROFF\r\n"));
#else
        dbg("%s: ETCMASK_OVERLAYMIXEROFF\n", __func__);
#endif
		pGPUGRPBUSCONFIG->GRPBUS_PWRDOWN |= Hw1;
	}
#endif
//Enable 
	if(lMask & ETCMASK_OVERLAYMIXERON)
	{
#if defined(_WINCE_)
		OALMSG(TC_LOG_LEVEL(TC_DEBUG), (L"[KERNEL      ]ETCMASK_OVERLAYMIXEROFF\r\n"));
#else
        dbg("%s: ETCMASK_OVERLAYMIXEROFF\n", __func__);
#endif
		pGPUGRPBUSCONFIG->GRPBUS_PWRDOWN  &= ~Hw1;
	}

	if(lMask & ETCMASK_3DGPUON)
	{
#if defined(_WINCE_)
		OALMSG(TC_LOG_LEVEL(TC_DEBUG), (L"[KERNEL      ]ETCMASK_3DGPUON\r\n"));
#else
        dbg("%s: ETCMASK_3DGPUON\n", __func__);
#endif
		pGPUGRPBUSCONFIG->GRPBUS_PWRDOWN &= ~Hw0;
	}
	
	if(lMask & ETCMASK_USBPHYON)
	{
#if defined(_WINCE_)
		OALMSG(TC_LOG_LEVEL(TC_DEBUG), (L"[KERNEL      ]ETC_USBPHYON\r\n"));
#else
        dbg("%s: ETC_USBPHYON\n", __func__);
#endif
		BITCSET(pOTGCFG->UPCR2,Hw10|Hw9,0);
		pOTGCFG->UPCR0 = 0x2842;
	}

}

void int_alldisable(void)
{
	/*
	volatile unsigned int count;
	count = 1;
	while(count--);
	*/
#if defined(__WINCE__)
	_gCPSRInt = arm_disableInt();
#else
    local_irq_save(flags);
    local_irq_disable();
#endif
}

void int_restore(void)
{
    /*
	volatile unsigned int count;
	count = 1;
	while(count--);
	*/
#if defined(__WINCE__)
	arm_restoreInt(_gCPSRInt);
#else
    local_irq_restore(flags);
#endif
}

#if defined(DRAM_DDR3)
//Bruce, 101102, WR과 CL의 변경사항이 없으면 MRS Setting을 다시 하지 않는다.
static unsigned int gWR = 0;
static unsigned int gCL = 0;

void ddr3_set_param(ddr3_setting_value ddr3, unsigned int lbusvalue)
{
	unsigned int tRFC=0, tCK_min=0, tRAS=0, tFAW=0, tXP=0, tCKE=0, tRRD=0;
	unsigned int nCL=0, nREFI=0, nRFC=0, nRRD=0, nRC=0, nRAS=0, nWTR=0, nWR=0, nAL=0, nCWL=0, nWL=0, nXS=0, nRTP=0, nXP=0, nRL=0, nFAW=0, nCKE=0, nMRD=0;
	unsigned int cur_cas=0;	/* cas latency for current clock */
	unsigned int cur_max_clk=0;
	unsigned int arrMRS[4];

	if(ddr3.clk*2 <= DDR3_800)
		cur_max_clk = DDR3_800;
	else if(ddr3.clk*2 <= DDR3_1066)
		cur_max_clk = DDR3_1066;
	else if(ddr3.clk*2 <= DDR3_1333)
		cur_max_clk = DDR3_1333;
	else if(ddr3.clk*2 <= DDR3_1600)
		cur_max_clk = DDR3_1600;
	else if(ddr3.clk*2 <= DDR3_1866)
		cur_max_clk = DDR3_1866;
	else if(ddr3.clk*2 <= DDR3_2133)
		cur_max_clk = DDR3_2133;

	/* tRP = tRCD = tCAS */
	if(ddr3.clk < 400)
		cur_cas = ddr3.cl*400*2/(ddr3.max_speed);
	else
		cur_cas = ddr3.cl*ddr3.clk*2/(ddr3.max_speed);
	if(ddr3.cl*ddr3.clk%ddr3.max_speed != 0)
		cur_cas++;
	nCL = cur_cas;

	/* tREFI = 78us */
	nREFI  = 7800*ddr3.clk/1000;
	if(7800*ddr3.clk%1000 != 0)
		nREFI++;

	/*
		tRFC
		512MBIT = 90ns, 1GBIT = 110ns, 2GBIT = 160ns, 4GBIT = 300ns, 8GBIT = 350ns
	*/
	switch (ddr3.size)
	{
	case SIZE_512MBIT:
		tRFC = 90;
		break;
	case SIZE_1GBIT:
		tRFC = 110;
		break;
	case SIZE_2GBIT:
		tRFC = 160;
		break;
	case SIZE_4GBIT:
		tRFC = 300;
		break;
	case SIZE_8GBIT:
		tRFC = 350;
		break;
	default :
		break;
	}
	nRFC = tRFC*ddr3.clk/1000;
	if(tRFC*ddr3.clk%1000 != 0)
		nRFC++;
	
	/* tXS =tXPR = max(5nCK, tRFC(min) +10ns) */
	nXS = (tRFC+10)*ddr3.clk/1000;
	if((tRFC+10)*ddr3.clk%1000 != 0)
		nXS++;
	if(nXS < 5)
		nXS = 5;

	/*
		tRC = tRAS + tRP
		tRAS is 37.5(DDR3-800, DDR3-1066), 36(DDR3-1333), 35(DDR3-1600, DDR3-1866, DDR3-2133)
		tRP = cur_cas*cur_max_clk/ddr3.max_speed
		
		tXP is 7.5(DDR3-800, DDR3-1066), 6(DDR3-1333, ,DDR3-1600, DDR3-1866, DDR3-2133)
		tXP = max(3*tCK, tXP);		

		tCKE is 7.5(DDR3-800), 5.625(DDR3-1066, DDR3-1333), 5(DDR3-1600, DDR3-1866, DDR3-2133)
		tCKE = max(3*tCK, tCKE);		
	*/
	switch(ddr3.max_speed)
	{
	case DDR3_800:
		tRAS = 37500 /* 37.5 ns */;
		tCK_min = 2500 /* 2.5 ns */;
		if(ddr3.pagesize == PAGE_1KB)
		{
			tRRD = 10000 /* 10 ns */;
			tFAW = 40000 /* 40 ns */;
		}
		else
		{
			tRRD = 10000 /* 10 ns */;
			tFAW = 50000 /* 50 ns */;
		}
		tXP = 7599 /* 7.5 ns */;
		tCKE = 7599 /* 7.5 ns */;
		break;
	case DDR3_1066:
		tRAS = 37500 /* 37.5 ns */;
		tCK_min = 1875 /*1.875 ns */;
		if(ddr3.pagesize == PAGE_1KB)
		{
			tRRD = 7500 /* 7.5 ns */;
			tFAW = 37500 /* 37.5 ns */;
		}
		else
		{
			tRRD = 10000 /* 10 ns */;
			tFAW = 50000 /* 50 ns */;
		}
		tXP = 7599 /* 7.5 ns */; 
		tCKE = 5625 /* 5.625 */;
		break;
	case DDR3_1333:
		tRAS = 36000 /* 36 ns */;
		tCK_min = 1500 /* 1.5 ns */;
		if(ddr3.pagesize == PAGE_1KB)
		{
			tRRD = 6000 /* 6 ns */;
			tFAW = 30000 /* 30 ns */;
		}
		else
		{
			tRRD = 7500 /* 7.5 ns */;
			tFAW = 45000 /* 45 ns */;
		}
		tXP = 6000 /* 6 ns */; 
		tCKE = 5625 /* 5.625 ns */;
		break;
	case DDR3_1600:
		tRAS = 35000 /* 35 ns */;
		tCK_min = 1250 /* 1.25 ns */;
		if(ddr3.pagesize == PAGE_1KB)
		{
			tRRD = 6000 /* 6 ns */;
			tFAW = 30000 /* 30 ns */;
		}
		else
		{
			tRRD = 7500 /* 7.5 ns */;
			tFAW = 40000 /* 40 ns */;
		}
		tXP = 6000 /* 6 ns */; 
		tCKE = 5000 /* 5 ns */; 
		break;
	case DDR3_1866:
		tRAS = 35000 /* 35 ns */;
		tCK_min = 1070 /* 1.07 ns */;
		/*TBD(To bo determined) value. This is temp value refer DDR3-1600 */
		if(ddr3.pagesize == PAGE_1KB)
		{
			tRRD = 6000 /* 6 ns */;
			tFAW = 30000 /* 30 ns */;
		}
		else
		{
			tRRD = 7500 /* 7.5 ns */;
			tFAW = 40000 /* 40 ns */;
		}
		tXP = 6000 /* 6 ns */; 
		tCKE = 5000 /* 5 ns */; 
		break;
	case DDR3_2133:
		tRAS = 35000 /* 35 ns */;
		tCK_min = 935 /* 0.935 ns */;
		/*TBD(To bo determined) value. This is temp value refer DDR3-1600 */
		if(ddr3.pagesize == PAGE_1KB)
		{
			tRRD = 6000 /* 6 ns */;
			tFAW = 30000 /* 30 ns */;
		}
		else
		{
			tRRD = 7500 /* 7.5 ns */;
			tFAW = 40000 /* 40 ns */;
		}
		tXP = 6000 /* 6 ns */; 
		tCKE = 5000 /* 5 ns */; 
		break;
	default:
		dbg("%s : error!! ddr3.max_speed is not valid!!!\r\n", __FUNCTION__);
		break;
	}

	/* tRRD = max(4*tCK, tRRD) */
	nRRD = tRRD*ddr3.clk/1000000;
	if(tRRD*ddr3.clk%1000000)
		nRRD++;
	if(nRRD < 4)
		nRRD = 4;

	nRAS = tRAS*ddr3.clk/1000000;
	if(tRAS*ddr3.clk%1000000 != 0)
		nRAS++;

	nRC = (tRAS + tCK_min*cur_cas)*ddr3.clk/1000000;
	if( (tRAS + tCK_min*cur_cas)*ddr3.clk%1000000 != 0)
		nRC++;

	/* tWTR = max(4nCK, 7.5ns) */
	nWTR = 7500*ddr3.clk/1000000;
	if( 7500*ddr3.clk%1000000 != 0)
		nWTR++;
	if(nWTR < 4)
		nWTR = 4;
	
	/* tWR = 15ns */
	nWR = 15*ddr3.clk/1000;
	if( 15*ddr3.clk%1000 != 0 )
		nWR++;

	/* tWTR = max(4nCK, 7.5ns) */
	nRTP = nWTR;

	/* tXP = max(3*tCK, tXP); */
	nXP = tXP*ddr3.clk/1000000;
	if(tXP*ddr3.clk%1000000 != 0)
		nXP++;
	if(nXP < 3)
		nXP = 3;

	/* tCKE = max(3*tCK, tCKE); */
	nCKE = tCKE*ddr3.clk/1000000;
	if(tCKE*ddr3.clk%1000000 != 0)
		nCKE++;
	if(nCKE < 3)
		nCKE = 3;

	if(1000000/ddr3.clk >= 2500 /* 2.5 ns */)
		nCWL = 5;
	else if(1000000/ddr3.clk >= 1875 /* 1.875 ns */)
		nCWL = 6;
	else if(1000000/ddr3.clk >= 1500 /* 1.5 ns */)
		nCWL = 7;
	else if(1000000/ddr3.clk >= 1250 /* 1.25 ns */)
		nCWL = 8;
	else if(1000000/ddr3.clk >= 1070 /* 1.07 ns */)
		nCWL = 9;
	else if(1000000/ddr3.clk >= 935 /* 0.935 ns */)
		nCWL = 10;
	else if(1000000/ddr3.clk >= 833 /* 0.833 ns */)
		nCWL = 11;
	else if(1000000/ddr3.clk >= 750 /* 0.75 ns */)
		nCWL = 12;

	switch(ddr3.al)
	{
	case AL_DISABLED:
		nAL = 0;
		break;
	case AL_CL_MINUS_ONE:
		nAL = cur_cas - 1;
		break;
	case AL_CL_MINUS_TWO:
		nAL = cur_cas - 2;
		break;
	default:
		dbg("%s : al type is error!! not valid!!\r\n", __FUNCTION__);
		break;
	}

	nWL = nAL + nCWL;
	nRL = nAL + cur_cas;
	nFAW = tFAW*ddr3.clk/1000000;
	if(tFAW*ddr3.clk%1000000 != 0)
		nFAW++;
	
	nMRD = 4;

	/* Setting Mode Register */
	/* MR0 */
	arrMRS[0] = 0;
	arrMRS[0] |= BURST_LEN;
	arrMRS[0] |= READ_BURST_TYPE*Hw3;

	if(nCL < 5)
		arrMRS[0] |= (5-4)*Hw4;
	else if(nCL > 11)
		arrMRS[0] |= (11-4)*Hw4;
	else
		arrMRS[0] |= (nCL-4)*Hw4;
	
	if(nWR <= 5) 
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

	//Bruce, 101102, WR과 CL의 변경사항이 없으면 MRS Setting을 다시 하지 않는다.
	{
		CKC_CHANGE_ARG(MRS_USE) = 0;

		if(nWR != gWR)
			CKC_CHANGE_ARG(MRS_USE) = 1;
		gWR = nWR;

		if(nCL != gCL)
			CKC_CHANGE_ARG(MRS_USE) = 1;
		gCL = nCL;
	}

	//Bruce, 101029, modify according to soc guide, ddr2는 0이 fast exit 라고 표기되어 있음.
	//arrMRS[0] |= FAST_EXIT*Hw12;

	//Bruce, 101029, modify according to soc guide
	if(lbusvalue >= 3330000)
		CKC_CHANGE_ARG(DLLONOFF_DDR3) = 1;
	else
		CKC_CHANGE_ARG(DLLONOFF_DDR3) = 0;

	/* MR1 */
	arrMRS[1] = Hw16;//MR1

	
	arrMRS[1] |= CUR_AL*Hw3;
	//arrMRS[1] |= (Hw12 | Hw11 | Hw6); //Rtt_Nom is RZQ/2, Don't enable "Write leveling enable", tDQS enable
	arrMRS[1] |= (Hw2 | Hw1); //Rtt_Nom is RZQ/4, Output Driver Impedance Control RZQ/7

	/* MR2 */
	arrMRS[2] = Hw17;//MR2
	if(ddr3.clk*2 <= DDR3_800)
		arrMRS[2] |= 0;
	else if(ddr3.clk*2 <= DDR3_1066)
		arrMRS[2] |= 1*Hw3;
	else if(ddr3.clk*2 <= DDR3_1333)
		arrMRS[2] |= 2*Hw3;
	else if(ddr3.clk*2 <= DDR3_1600)
		arrMRS[2] |= 3*Hw3;	

	//Bruce, 101029, modify according to soc guide 
	//arrMRS[2] |= Hw10; //Rtt_WR is RZQ/2

	/* MR3 */
	arrMRS[3] = Hw17|Hw16;//MR2

	CKC_CHANGE_ARG(T_REFI) = nREFI;
	CKC_CHANGE_ARG(T_RFC) = nRFC;
	CKC_CHANGE_ARG(T_RRD) = nRRD;
	CKC_CHANGE_ARG(T_RP) = nCL;
	CKC_CHANGE_ARG(T_RCD) = nCL;
	CKC_CHANGE_ARG(T_RC) = nRC;
	CKC_CHANGE_ARG(T_RAS) = nRAS;
	CKC_CHANGE_ARG(T_WTR) = nWTR;
	CKC_CHANGE_ARG(T_WR) = nWR;
	CKC_CHANGE_ARG(T_RTP) = nRTP; 
	CKC_CHANGE_ARG(CL) = nCL; 
	CKC_CHANGE_ARG(WL) = nWL; 
	CKC_CHANGE_ARG(RL) = nRL;
	CKC_CHANGE_ARG(T_FAW) = nFAW; 
	CKC_CHANGE_ARG(T_XSR) = nXS;
	CKC_CHANGE_ARG(T_XP) = nXP; 
	CKC_CHANGE_ARG(T_CKE) = nCKE; 
	CKC_CHANGE_ARG(T_MRD) = nMRD;

	CKC_CHANGE_ARG(MR0_DDR3) = arrMRS[0];
	CKC_CHANGE_ARG(MR1_DDR3) = arrMRS[1];
	CKC_CHANGE_ARG(MR2_DDR3) = arrMRS[2];
	CKC_CHANGE_ARG(MR3_DDR3) = arrMRS[3];
	//dbg("dec: nREFI=%d, nRFC=%d, nRRD=%d, nRC=%d, nRAS=%d, nWTR=%d, nWR=%d, nRTP=%d, nCL=%d, nWL=%d,nRL=%d, nFAW=%d, nXS=%d, nXP=%d, nCKE=%d, nMRD=%d\r\n", nREFI, nRFC, nRRD, nRC, nRAS, nWTR, nWR, nRTP, nCL, nWL,nRL, nFAW, nXS, nXP, nCKE, nMRD);
	//dbg("hex: nREFI=0x%x, nRFC=0x%x, nRRD=0x%x, nRC=0x%x, nRAS=0x%x, nWTR=0x%x, nWR=0x%x, nRTP=0x%x, nCL=0x%x, nWL=0x%x,nRL=0x%x, nFAW=0x%x, nXS=0x%x, nXP=0x%x, nCKE=0x%x, nMRD=0x%x\r\n", nREFI, nRFC, nRRD, nRC, nRAS, nWTR, nWR, nRTP, nCL, nWL,nRL, nFAW, nXS, nXP, nCKE, nMRD);
}

static unsigned int set_clockchangeparam(unsigned int lbusvalue)
{
	int 					i = 0;
	unsigned int DRAM_BUS_CLOCK = 0;
	ddr3_setting_value ddr3;

	for (i=0 ; i<ARRAY_SIZE(pIO_CKC_PLL3) ; i++) {
		if (pIO_CKC_PLL3[i].freq_value >= lbusvalue)
			break;
	}

	if (i >= ARRAY_SIZE(pIO_CKC_PLL3))
		i = (ARRAY_SIZE(pIO_CKC_PLL3) - 1);;

	lbusvalue = pIO_CKC_PLL3[i].freq_value;

	CKC_CHANGE_ARG(PLLVALUE_DIS_DDR3) = pIO_CKC_PLL3[i].pll1config_reg;
	CKC_CHANGE_ARG(CLK_CTRL2_DDR3) = 	pIO_CKC_PLL3[i].clkctrl2_reg;

	ddr3.max_speed = CUR_MAX_SPEED;
	ddr3.size =  CUR_SIZE;
	ddr3.cl = CUR_CL;
	ddr3.al = CUR_AL;
	ddr3.pagesize = CUR_PAGE_SIZE;
	ddr3.clk = pIO_CKC_PLL3[i].freq_value/10000; //lbusvalue/10000

	//printk("DDR3- clk:%d , clk:%d, pms:0x%x, clk:0x%x, mrs:%d\n", lbusvalue/10000, ddr3.clk, pIO_CKC_PLL3[i].pll1config_reg, pIO_CKC_PLL3[i].clkctrl2_reg, CKC_CHANGE_ARG(MRS_USE));
	//printk("      mr0:0x%x, mr1:0x%x, mr2:0x%x, mr3:0x%x\n", CKC_CHANGE_ARG(MR0_DDR3), CKC_CHANGE_ARG(MR1_DDR3), CKC_CHANGE_ARG(MR2_DDR3), CKC_CHANGE_ARG(MR3_DDR3));

	ddr3_set_param(ddr3, ddr3.clk*10000);

	return lbusvalue;
}

static void init_copychangeclock(unsigned int lbusvalue)
{
	lpSelfRefresh = (lpfunc)(SRAM_COPY_ADDR);

	memcpy((void *) SRAM_COPY_ADDR, (void*)init_clockchange_ddr3, SRAM_COPY_FUNC_SIZE);

	// Jump to Function Start Point
	lpSelfRefresh(lbusvalue);
	
}
#elif defined(DRAM_DDR2)
#define time2cycle(time, tCK)		((int)((time + tCK - 1) / tCK))
static unsigned int set_clockchangeparam(unsigned int lbusvalue)
{
	int 					i = 0;
	unsigned int temp;
	unsigned int DRAM_BUS_CLOCK = 0;
	int tCK = 0;

	for (i=0 ; i<ARRAY_SIZE(pIO_CKC_PLL3) ; i++) {
		if (pIO_CKC_PLL3[i].freq_value >= lbusvalue)
			break;
	}

	if (i >= ARRAY_SIZE(pIO_CKC_PLL3))
		i = (ARRAY_SIZE(pIO_CKC_PLL3) - 1);;

	lbusvalue = pIO_CKC_PLL3[i].freq_value;
	DRAM_BUS_CLOCK = lbusvalue/10000;//DRAM_BUS_CLOCK;

	CKC_CHANGE_ARG(PLLVALUE_DIS) = pIO_CKC_PLL3[i].pll1config_reg;
	CKC_CHANGE_ARG(CLK_CTRL2)	 = pIO_CKC_PLL3[i].clkctrl2_reg;

	// for linux compile(x100)
	tCK = 100000/DRAM_BUS_CLOCK;
		
	#if defined(DDR2CONTROLLER_LPCON)
		CKC_CHANGE_ARG(TAREF) = (int)(time2cycle(780000, tCK));//t_aref
		#if (defined(CONFIG_TCC_MEM_512MB))
		CKC_CHANGE_ARG(TROW) = 
			( time2cycle(DRAM_PHY_RFC, tCK) << 24 )//t_rfc
			| ( time2cycle(((BOARD_DRAM_DATABITS == 16) ? 750 : 1000), tCK) << 20 )//t_rrd
			| ( time2cycle(1250, tCK) << 16 )//t_rp
			| ( time2cycle(1250, tCK) << 12 )//t_rcd
			| ( time2cycle(5750, tCK) << 6)//t_rc
			| ( time2cycle(4500, tCK) << 0)//t_ras
			;

		CKC_CHANGE_ARG(TDATA) = 
			(3 << 28)//t_wtr
			| ( time2cycle(1500, tCK) << 24)//t_wr
			| ( 3 << 20 )//t_rtp
			| ( ((DRAM_PHY_CAS > 5) ? 5 : DRAM_PHY_CAS) << 16 )//cl
			| ( ((DRAM_PHY_CAS > 5) ? 4 : (DRAM_PHY_CAS-1)) << 8 )//wl
			| ( ((DRAM_PHY_CAS > 5) ? 5 : DRAM_PHY_CAS) << 16 )//rl - it is only for LPDDR2
			;
		
		CKC_CHANGE_ARG(TPOWER) = 
			( time2cycle(4500, tCK) << 24)//t_faw
			| ( 200 << 16 )//t_xsr
			| ( 3 << 8 )//t_xp
			| ( 3 << 4 )//t_cke
			| 2//t_mrd
			;//TFAW
		//for EMR
		CKC_CHANGE_ARG(TWR) = time2cycle(1500, tCK);//TWR
		#else
		CKC_CHANGE_ARG(TROW) = 
			( (int)(DRAM_PHY_RFC/tCK +DDR_DELAY) << 24 )//t_rfc
			| ( (int)( ((BOARD_DRAM_DATABITS == 16) ? 750 : 1000)/tCK +DDR_DELAY) << 20 )//t_rrd
			| ( (int)( 1800/tCK + DDR_DELAY ) << 16 )//t_rp
			| ( (int)( 1800/tCK + DDR_DELAY ) << 12 )//t_rcd
			| ( (int)( 6000/tCK + DDR_DELAY ) << 6)//t_rc
			| (int)( 4500/tCK + DDR_DELAY )//t_ras
			;
		
		CKC_CHANGE_ARG(TDATA) = 
			(3 << 28)//t_wtr
			| ( (int)( 1500/tCK + DDR_DELAY ) << 24)//t_wr
			| ( 3 << 20 )//t_rtp
			| ( ((DRAM_PHY_CAS > 5) ? 5 : DRAM_PHY_CAS) << 16 )//cl
			| ( ((DRAM_PHY_CAS > 5) ? 4 : (DRAM_PHY_CAS-1)) << 8 )//wl
			| ( ((DRAM_PHY_CAS > 5) ? 5 : DRAM_PHY_CAS) << 16 )//rl - it is only for LPDDR2
			//| ( (DRAM_PHY_CAS) << 16 )//cl
			//| ( (DRAM_PHY_CAS-1) << 8 )//wl
			//| ( DRAM_PHY_CAS )//rl - it is only for LPDDR2
			;
		
		CKC_CHANGE_ARG(TPOWER) = 
			( (int)(4500/tCK + DDR_DELAY) << 24)//t_faw
			| ( 200 << 16 )//t_xsr
			| ( 3 << 8 )//t_xp
			| ( 3 << 4 )//t_cke
			| 2//t_mrd
			;//TFAW
		//for EMR
		CKC_CHANGE_ARG(TWR) = (int)(1500/tCK +DDR_DELAY);//TWR
		#endif
	#else
		CKC_CHANGE_ARG(TCAS) =	(DRAM_PHY_CAS<<1)										;//TCAS
		//CKC_CHANGE_ARG(TRCD) =	DDR2_SETRCD((int) (DRAM_PHY_CAS*(DRAM_PHY_MAX_FREQ == 330 ? 3 :2.5 )/tCK)  )							;//TRCD
		//CKC_CHANGE_ARG(TRP) =	DDR2_SETRP((int)(DRAM_PHY_CAS*(DRAM_PHY_MAX_FREQ == 330 ? 3 :2.5 )/tCK))							;//TRP		
		CKC_CHANGE_ARG(TRCD) =	DDR2_SETRCD((int) (1800/tCK) +DDR_DELAY )							;//TRCD
		CKC_CHANGE_ARG(TRP) =	DDR2_SETRP((int) (1800/tCK) +DDR_DELAY )							;//TRP	
		CKC_CHANGE_ARG(TRAS) =	(int)(4500/tCK+DDR_DELAY)									;//TRAS
		CKC_CHANGE_ARG(TRC) =	(int)(6000/tCK+DDR_DELAY) 	;//TRC	
		CKC_CHANGE_ARG(TRFC) =	DDR2_SETRFC((int)(DRAM_PHY_RFC/tCK +DDR_DELAY))				;//TRFC
		CKC_CHANGE_ARG(TESR) =	(int)((DRAM_PHY_RFC+1000)/tCK+DDR_DELAY)						;//TESR
		CKC_CHANGE_ARG(TRRD) =	(int)( ((BOARD_DRAM_DATABITS == 16) ? 750 : 1000)/tCK +DDR_DELAY);//TRRD
		CKC_CHANGE_ARG(TWR) =	 (int)(1500/tCK +DDR_DELAY)								;//TWR	
		CKC_CHANGE_ARG(TWTR) =	(int)(3)									;//TWTR
		CKC_CHANGE_ARG(TXP) =	(int)(3)									;// TXP	
		CKC_CHANGE_ARG(TXSR) =	 (int)(200)									;//TXSR
		CKC_CHANGE_ARG(TFAW) =	DDR2_SETFAW( (int)(4500/tCK) +DDR_DELAY)									;//TFAW
		CKC_CHANGE_ARG(TREFRESH) =	(int)(780000/tCK +DDR_DELAY)								;//TREFRESH

		if(i>23)
			CKC_CHANGE_ARG(EMR1) =	0x1212;//EMR1REG
		else if(i< 3)
			CKC_CHANGE_ARG(EMR1) =	0x2525;//EMR1REG
		else
			CKC_CHANGE_ARG(EMR1) =	0x1717;//EMR1REG
		if(i < 10)
			CKC_CHANGE_ARG(EMR1+1) =	0 ;//ODT_CTRL_OFF
		else
			CKC_CHANGE_ARG(EMR1+1) =	1 ;//ODT_CTRL_ON
	#endif

	#if defined(DDR2CONTROLLER_LPCON)
	temp = CKC_CHANGE_ARG(TAREF);
	temp = CKC_CHANGE_ARG(TROW);
	temp = CKC_CHANGE_ARG(TDATA);
	temp = CKC_CHANGE_ARG(TPOWER);
	temp = CKC_CHANGE_ARG(PLLVALUE_DIS);
	temp = CKC_CHANGE_ARG(CLK_CTRL2);

//	printk(" TROW :0x%x, TDATA:0x%x, TPOWER:0x%x\n", CKC_CHANGE_ARG(TROW), CKC_CHANGE_ARG(TDATA), CKC_CHANGE_ARG(TPOWER));
	#else//PL341
//Prevent Page Table Work. 
	temp = CKC_CHANGE_ARG(TCAS);
	temp = CKC_CHANGE_ARG(TRCD);
	temp = CKC_CHANGE_ARG(TRP) ;
	temp = CKC_CHANGE_ARG(TRAS);
	temp = CKC_CHANGE_ARG(TRC) ;
	temp = CKC_CHANGE_ARG(TRFC);
	temp = CKC_CHANGE_ARG(TESR);
	temp = CKC_CHANGE_ARG(TRRD);
	temp = CKC_CHANGE_ARG(TWR) ;
	temp = CKC_CHANGE_ARG(TWTR);
	temp = CKC_CHANGE_ARG(TXP) ;
	temp = CKC_CHANGE_ARG(TXSR);
	temp = CKC_CHANGE_ARG(TFAW);
	temp = CKC_CHANGE_ARG(TREFRESH) ;
	temp = CKC_CHANGE_ARG(PLLVALUE_DIS);
	temp = CKC_CHANGE_ARG(CLK_CTRL2);	
	temp = CKC_CHANGE_ARG(EMR1); 
	temp = CKC_CHANGE_ARG(EMR1+1);
	#endif

	
	temp = *(volatile unsigned int*)addr(0x500008);
	temp = *(volatile unsigned int*)addr(0x500038);
	temp = *(volatile unsigned int*)addr(0x50002c);
	temp = *(volatile unsigned int*)addr(0x600004);

	return lbusvalue;
}

static void init_copychangeclock(unsigned int lbusvalue)
{
	lpSelfRefresh = (lpfunc)(SRAM_COPY_ADDR);

	memcpy((void *) SRAM_COPY_ADDR, (void*)init_clockchange_ddr2, SRAM_COPY_FUNC_SIZE);

	// Jump to Function Start Point
	lpSelfRefresh(lbusvalue);
}
#else
static void init_copychangeclock(unsigned int lbusvalue)
{

	volatile unsigned int	*fptr = 0;
	volatile unsigned int	*p = 0;
	int 					i = 0;
	unsigned int temp;
	unsigned int DRAM_BUS_CLOCK = 0;
	int tCK = 0;

	DRAM_BUS_CLOCK = lbusvalue/10000;//DRAM_BUS_CLOCK;

#if defined(DRAM_DDR2)	
	switch(lbusvalue){
	case			1260000: i = 0; break;
	case 			1300000: i = 1; break;
	case 			1380000: i = 2; break;
	case 			1410000: i = 3; break;
	case 			1450000: i = 4; break;
	case 			1520000: i = 5; break;
	case 			1600000: i = 6; break;
	case 			1700000: i = 7; break; 
	case 			1800000: i = 8; break;
	case 			1900000: i = 9;  break;
	case 			2000000: i = 10; break;
	case 			2100000: i = 11; break;
	case 			2200000: i = 12; break;
	case 			2300000: i = 13; break; 
	case 			2400000: i = 14; break;
	case 			2500000: i = 15; break;
	case 			2600000: i = 16; break;
	case 			2700000: i = 17; break;
	case 			2800000: i = 18; break;
	case 			2900000: i = 19; break;
	case 			3000000: i = 20; break;
	case 			3120000: i = 21; break;
	case 			3200000: i = 22; break;
	case    		3300000: i = 23; break;	
	case    		3600000: i = 24; break;
	case    		4000000: i = 25; break;
	default: i=18; break;
	}	
	

	CKC_CHANGE_ARG(PLLVALUE_DIS) =	pIO_CKC_PLL3[i].pll1config_reg;
	CKC_CHANGE_ARG(CLK_CTRL2) =		pIO_CKC_PLL3[i].clkctrl2_reg;

	// for linux compile(x100)
	tCK = 100000/DRAM_BUS_CLOCK;
		
	#if defined(DDR2CONTROLLER_LPCON)
		CKC_CHANGE_ARG(TAREF) = (int)( 780000/tCK + DDR_DELAY );//t_aref
		CKC_CHANGE_ARG(TROW) = 
			( (int)(DRAM_PHY_RFC/tCK +DDR_DELAY) << 24 )//t_rfc
			| ( (int)( ((BOARD_DRAM_DATABITS == 16) ? 750 : 1000)/tCK +DDR_DELAY) << 20 )//t_rrd
			| ( (int)( 1800/tCK + DDR_DELAY ) << 16 )//t_rp
			| ( (int)( 1800/tCK + DDR_DELAY ) << 12 )//t_rcd
			| ( (int)( 6000/tCK + DDR_DELAY ) << 6)//t_rc
			| (int)( 4500/tCK + DDR_DELAY )//t_ras
			;
		CKC_CHANGE_ARG(TDATA) = 
			(3 << 28)//t_wtr
			| ( (int)( 1500/tCK + DDR_DELAY ) << 24)//t_wr
			| ( 3 << 20 )//t_rtp
			| ( ((DRAM_PHY_CAS > 5) ? 5 : DRAM_PHY_CAS) << 16 )//cl
			| ( ((DRAM_PHY_CAS > 5) ? 4 : (DRAM_PHY_CAS-1)) << 8 )//wl
			| ( ((DRAM_PHY_CAS > 5) ? 5 : DRAM_PHY_CAS) << 16 )//rl - it is only for LPDDR2
			//| ( (DRAM_PHY_CAS) << 16 )//cl
			//| ( (DRAM_PHY_CAS-1) << 8 )//wl
			//| ( DRAM_PHY_CAS )//rl - it is only for LPDDR2
			;
		
		CKC_CHANGE_ARG(TPOWER) = 
			( (int)(4500/tCK + DDR_DELAY) << 24)//t_faw
			| ( 200 << 16 )//t_xsr
			| ( 3 << 8 )//t_xp
			| ( 3 << 4 )//t_cke
			| 2//t_mrd
			;//TFAW		

		//for EMR
		CKC_CHANGE_ARG(TWR) = (int)(1500/tCK +DDR_DELAY);//TWR
	#else
		CKC_CHANGE_ARG(TCAS) =	(DRAM_PHY_CAS<<1)										;//TCAS
		//CKC_CHANGE_ARG(TRCD) =	DDR2_SETRCD((int) (DRAM_PHY_CAS*(DRAM_PHY_MAX_FREQ == 330 ? 3 :2.5 )/tCK)  )							;//TRCD
		//CKC_CHANGE_ARG(TRP) =	DDR2_SETRP((int)(DRAM_PHY_CAS*(DRAM_PHY_MAX_FREQ == 330 ? 3 :2.5 )/tCK))							;//TRP		
		CKC_CHANGE_ARG(TRCD) =	DDR2_SETRCD((int) (1800/tCK) +DDR_DELAY )							;//TRCD
		CKC_CHANGE_ARG(TRP) =	DDR2_SETRP((int) (1800/tCK) +DDR_DELAY )							;//TRP	
		CKC_CHANGE_ARG(TRAS) =	(int)(4500/tCK+DDR_DELAY)									;//TRAS
		CKC_CHANGE_ARG(TRC) =	(int)(6000/tCK+DDR_DELAY) 	;//TRC	
		CKC_CHANGE_ARG(TRFC) =	DDR2_SETRFC((int)(DRAM_PHY_RFC/tCK +DDR_DELAY))				;//TRFC
		CKC_CHANGE_ARG(TESR) =	(int)((DRAM_PHY_RFC+1000)/tCK+DDR_DELAY)						;//TESR
		CKC_CHANGE_ARG(TRRD) =	(int)( ((BOARD_DRAM_DATABITS == 16) ? 750 : 1000)/tCK +DDR_DELAY);//TRRD
		CKC_CHANGE_ARG(TWR) =	 (int)(1500/tCK +DDR_DELAY)								;//TWR	
		CKC_CHANGE_ARG(TWTR) =	(int)(3)									;//TWTR
		CKC_CHANGE_ARG(TXP) =	(int)(3)									;// TXP	
		CKC_CHANGE_ARG(TXSR) =	 (int)(200)									;//TXSR
		CKC_CHANGE_ARG(TFAW) =	DDR2_SETFAW( (int)(4500/tCK) +DDR_DELAY)									;//TFAW
		CKC_CHANGE_ARG(TREFRESH) =	(int)(780000/tCK +DDR_DELAY)								;//TREFRESH

		if(i>23)
			CKC_CHANGE_ARG(EMR1) =	0x1212;//EMR1REG
		else if(i< 3)
			CKC_CHANGE_ARG(EMR1) =	0x2525;//EMR1REG
		else
			CKC_CHANGE_ARG(EMR1) =	0x1717;//EMR1REG
		if(i < 10)
			CKC_CHANGE_ARG(EMR1+1) =	0 ;//ODT_CTRL_OFF
		else
			CKC_CHANGE_ARG(EMR1+1) =	1 ;//ODT_CTRL_ON
	#endif


#endif

#if defined(DRAM_MDDR)
switch(lbusvalue){
	case		70000	: i = 0; break;
	case 		80000	: i = 1; break;
	case 		90000	: i = 2; break;
	case 		100000	: i = 3; break;
	case 		110000	: i = 4; break;
	case 		120000	: i = 5; break;
	case 		140000	: i = 6; break;
	case 		180000	: i = 7; break; 
	case 		200000	: i = 8; break;
	case 		220000	: i = 9;  break;
	case 		250000	: i = 10; break;
	case 		305000	: i = 11; break;
	case 		352500	: i = 12; break;
	case 		400000	: i = 13; break; 
	case 		450000	: i = 14; break;
	case 		500000	: i = 15; break;
	case 		550000	: i = 16; break;
	case 		600000	: i = 17; break;
	case 		650000	: i = 18; break;
	case 		705000	: i = 19; break;
	case 		750000	: i = 20; break;
	case 		800000	: i = 21; break;
	case 		850000	: i = 22; break;
	case    	900000	: i = 23; break;	
	case 		950000	: i = 24; break;
	case 		1000000	: i = 25; break;
	case 		1050000	: i = 26; break;
	case 		1100000	: i = 27; break; 
	case 		1150000	: i = 28; break;
	case 		1200000	: i = 29; break;
	case 		1250000	: i = 30; break;
	case 		1300000	: i = 31; break;
	case 		1330000	: i = 32; break;
	case 		1400000	: i = 33; break;
	case 		1450000	: i = 34; break;
	case 		1500000	: i = 35; break;
	case 		1560000	: i = 36; break;
	case    	1600000	: i = 37; break;
	case    	1650000	: i = 38; break;
	case    	1700000	: i = 39; break;
	case    	1760000	: i = 40; break;
	case    	1800000	: i = 41; break;
	case    	1850000	: i = 42; break;
	default: i=33; break;
	}	

	CKC_CHANGE_ARG(PLLVALUE_DIS) =	pIO_CKC_PLL1[i].pll1config_reg;
	CKC_CHANGE_ARG(CLK_CTRL2) =		0x00200000 |pIO_CKC_PLL1[i].lmem_source | ((pIO_CKC_PLL1[i].lmem_div-1) << 4);
	
	// for linux compile(x100)
	tCK = 100000/DRAM_BUS_CLOCK;

	CKC_CHANGE_ARG(TREFRESH) =	(int)(780000/tCK)								;//TREFRESH
	
	if(i <= 21) {//MDDR 80Mhz under..
		CKC_CHANGE_ARG(TCAS) =	((DRAM_PHY_CAS-1)<<1)										;//TCAS
		CKC_CHANGE_ARG(TFAW) = DRAM_PHY_CAS-1;
	}
	else{
		CKC_CHANGE_ARG(TCAS) =	(DRAM_PHY_CAS<<1)										;//TCAS
		CKC_CHANGE_ARG(TFAW) = DRAM_PHY_CAS;
	}
		
	
	CKC_CHANGE_ARG(TRAS) =	(int)(RAS*100/tCK +1)									;//TRAS
	CKC_CHANGE_ARG(TRC) =	(int)(RC/tCK + 1)                 	;//TRC	
	CKC_CHANGE_ARG(TRCD) =	MDDR_SETRCD((int)( RCD/tCK + 1))							;//TRCD
	CKC_CHANGE_ARG(TRFC) =	MDDR_SETRFC((int)( 10000/tCK + 1))				;//TRFC
	CKC_CHANGE_ARG(TRP) =	MDDR_SETRP ((int)( RCD/tCK + 1))							;//TRP	

	CKC_CHANGE_ARG(TRRD) =	(int)( RRD/tCK + 1);//TRRD
	CKC_CHANGE_ARG(TWR) =	(int)(1500/tCK +  1)								;//TWR	
	CKC_CHANGE_ARG(TWTR) =	(int)(WTR +1)									;//TWTR
	CKC_CHANGE_ARG(TXP) =	(int)(2)									;// TXP	
	CKC_CHANGE_ARG(TXSR) =	 (int)(XSR*100/tCK+1)									;//TXSR
	CKC_CHANGE_ARG(TESR) =	(int)(50)						;//TESR why??

#endif

	#if defined(DDR2CONTROLLER_LPCON)
	temp = CKC_CHANGE_ARG(TAREF);
	temp = CKC_CHANGE_ARG(TROW) ;
	temp = CKC_CHANGE_ARG(TDATA);
	temp = CKC_CHANGE_ARG(TPOWER);
	temp = CKC_CHANGE_ARG(PLLVALUE_DIS);
	temp = CKC_CHANGE_ARG(CLK_CTRL2);	
	#else//PL341
//Prevent Page Table Work. 
	temp = CKC_CHANGE_ARG(TCAS);
	temp = CKC_CHANGE_ARG(TRCD);
	temp = CKC_CHANGE_ARG(TRP) ;
	temp = CKC_CHANGE_ARG(TRAS);
	temp = CKC_CHANGE_ARG(TRC) ;
	temp = CKC_CHANGE_ARG(TRFC);
	temp = CKC_CHANGE_ARG(TESR);
	temp = CKC_CHANGE_ARG(TRRD);
	temp = CKC_CHANGE_ARG(TWR) ;
	temp = CKC_CHANGE_ARG(TWTR);
	temp = CKC_CHANGE_ARG(TXP) ;
	temp = CKC_CHANGE_ARG(TXSR);
	temp = CKC_CHANGE_ARG(TFAW);
	temp = CKC_CHANGE_ARG(TREFRESH) ;
	temp = CKC_CHANGE_ARG(PLLVALUE_DIS);
	temp = CKC_CHANGE_ARG(CLK_CTRL2);	
	temp = CKC_CHANGE_ARG(EMR1); 
	temp = CKC_CHANGE_ARG(EMR1+1);
	#endif

	
	temp = *(volatile unsigned int*)addr(0x500008);
	temp = *(volatile unsigned int*)addr(0x500038);
	temp = *(volatile unsigned int*)addr(0x50002c);
	temp = *(volatile unsigned int*)addr(0x600004);

#if defined(DRAM_DDR2)
	fptr = (volatile unsigned int*)init_clockchange_ddr2;	
#endif
#if defined(DRAM_MDDR)
	fptr = (volatile unsigned int*)init_clockchange_mddr;	
#endif
	
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
	lpSelfRefresh(lbusvalue);
	
}
#endif

#define iomap_p2v(x)            io_p2v(x)
extern unsigned int IO_ARM_ChangeStackSRAM(void);
extern void IO_ARM_RestoreStackSRAM(unsigned int);

static unsigned char DEV_LCDC_Wait_signal_disable(char lcdc)
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

	if(lcdc == 0)
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

void ckc_set_membusclock(unsigned int freq)
{
	unsigned int lbusvalue = freq*10;
	volatile unsigned int	*fptr = 0;
	volatile unsigned int	*p = 0;
	int 					i = 0;
	unsigned int			sel_pllfreq = 0;
	unsigned int temp;
	unsigned int DRAM_BUS_CLOCK = 0;
	int tCK = 0;

	unsigned int lcdc0_on = 0, lcdc1_on = 0;
	volatile PLCDC	pLCDC_BASE0 = (volatile PLCDC)tcc_p2v(HwLCDC0_BASE);
	volatile PLCDC	pLCDC_BASE1 = (volatile PLCDC)tcc_p2v(HwLCDC1_BASE);
	
#if defined(CONFIG_GENERIC_TIME)
	volatile PPIC	pPIC	= (volatile PPIC)tcc_p2v(HwPIC_BASE);
#else
	volatile PTIMER	pTIMER	= (volatile PTIMER)tcc_p2v(HwTMR_BASE);
#endif

#if defined(DRAM_DDR3) || defined(DRAM_DDR2)
	lbusvalue = set_clockchangeparam(lbusvalue);
#endif

	int_alldisable();
	local_flush_tlb_all();
	flush_cache_all();

#if defined(CONFIG_GENERIC_TIME)
	pPIC->IEN0 &= ~Hw1;		/* Disable Timer0 interrupt */
#else
	pTIMER->TC32EN &= ~Hw24;
#endif

	lcdc0_on = DEV_LCDC_Wait_signal_disable(0);
	lcdc1_on = DEV_LCDC_Wait_signal_disable(1);

	retstack = IO_ARM_ChangeStackSRAM();

	init_copychangeclock(lbusvalue);

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

EXPORT_SYMBOL(ckc_set_membusclock);
