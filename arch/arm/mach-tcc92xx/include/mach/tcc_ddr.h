/*
 * linux/arch/arm/mach-tccxxxx/include/mach/tcc_ddr.c
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
#ifndef __TCC_DDR__H__
#define __TCC_DDR__H__

#if defined(CONFIG_DRAM_DDR2)

/*===========================================================================

                           DDR2 Attributes Type

===========================================================================*/

/*---------------------------------------------------------------------------
 Clock Speed Type
---------------------------------------------------------------------------*/

#define DDR2_400	(400)
#define DDR2_533	(533)
#define DDR2_667	(667)
#define DDR2_800	(800)

/*---------------------------------------------------------------------------
 Page Size Type
---------------------------------------------------------------------------*/
enum 
{
	PAGE_1KB = 0,
	PAGE_2KB,
};

/*---------------------------------------------------------------------------
 Burst Length Type
---------------------------------------------------------------------------*/
enum
{
	BL_4 = 2,
	BL_8 = 3
};

/*---------------------------------------------------------------------------
 Read Burst Type
---------------------------------------------------------------------------*/
enum
{
	RBT_SEQUENTIAL = 0,
	RBT_INTERLEAVE,
};

#if defined(TCC_DDR2_LOWER_IDS_USE)
#define DDR2_AUTO_PD	0x1
#define DDR2_PD_PRD		0x7

#define DDR2_GATECTRL	( (0x0<<10) \	// ADB_EN
						| (0x0<<3) \	// OFFSETC
						| (0x7))		// SHIFTC (360)

#define DDR2_PHYZQCTRL	( (0x3 << 17) \	// PRD_CAL
						| (0x0 << 16) \	// PRD_CEN
						| (0x7 << 13) \	// DRV_STR
						| (0x0 << 12) \	// TERM_DIS
						| (0x2 << 9) \	// ODT(PHY) value 75 Ohm
						| (0x0 << 6) \	// PULL UP
						| (0x0 << 3) \	// PULL DOWN
						| (0x0 << 2) \	// ZQ
						| (0x0 << 1) \	// UPDATE
						| (0x1 << 0))

#define DDR2_READDELAY	0x6
#else
#define DDR2_AUTO_PD	0x0
#define DDR2_PD_PRD		0x0

#define DDR2_GATECTRL	( (0x0<<10) \
						| (0x0<<3) \
						| (0x6))

#define DDR2_PHYZQCTRL	( (0x3 << 17) \
						| (0x0 << 16) \
						| (0x7 << 13) \
						| (0x0 << 12) \
						| (0x2 << 9) \
						| (0x5 << 6) \
						| (0x2 << 3) \
						| (0x0 << 2) \
						| (0x0 << 1) \
						| (0x1 << 0))

#define DDR2_READDELAY	0x4
#endif


#if defined(CONFIG_DDR2_HXB18T2G160AF) /*==================================*/

/*---------------------------------------------------------------------------
 DDR2 Configuation
---------------------------------------------------------------------------*/
#define DDR2_PHYSICAL_CHIP_NUM           2
#define DDR2_LOGICAL_CHIP_NUM            1
#define DDR2_MAX_SPEED            DDR2_800
#define DDR2_CL                          5
#define DDR2_PAGE_SIZE            PAGE_2KB
#define DDR2_BURST_LEN                BL_4
#define DDR2_READ_BURST_TYPE RBT_SEQUENTIAL
#define DDR2_EA_MB_SIZE                256
#define DDR2_TOTAL_MB_SIZE             (DDR2_EA_MB_SIZE*DDR2_PHYSICAL_CHIP_NUM)
#define DDR2_ROWBITS                    14
#define DDR2_COLBITS                    10
#define DDR2_BANK_BITS                   3
#define DDR2_BANK_NUM                    8
#define DDR2_PHYSICAL_DATA_BITS         16
#define DDR2_LOGICAL_DATA_BITS          32

 /*---------------------------------------------------------------------------
 DDR2 Access Timing Parameters
---------------------------------------------------------------------------*/
#define DDR2_tRFC_ps                175000
#define DDR2_tREFI_ps              7800000
#define DDR2_tRRD_ps                 10000
#define DDR2_tRRD_ck                     1
#define DDR2_tRAS_ps                 45000
#define DDR2_tRAS_ck                     1
#define DDR2_tRC_ps                  57500
#define DDR2_tRC_ck                      1
#define DDR2_tRCD_ps                 12500
#define DDR2_tRCD_ck                     1
#define DDR2_tRP_ps                  12500
#define DDR2_tRP_ck                      1
#define DDR2_tWTR_ps                  7500
#define DDR2_tWTR_ck                     2
#define DDR2_tWR_ps                  15000
#define DDR2_tWR_ck                      1
#define DDR2_tESR_ck                   200
#define DDR2_tFAW_ps                 45000
#define DDR2_tFAW_ck                     1
#define DDR2_tXSR_ps  (DDR2_tRFC_ps+10000)
#define DDR2_tXSR_ck                   200
#define DDR2_tXP_ck                      2
#define DDR2_tMRD_ck                     2

#elif defined(CONFIG_DDR2_HY5PS1G1631CFPS6) /*==============================*/

/*---------------------------------------------------------------------------
 DDR2 Configuation
---------------------------------------------------------------------------*/
#define DDR2_PHYSICAL_CHIP_NUM           2
#define DDR2_LOGICAL_CHIP_NUM            1
#define DDR2_MAX_SPEED            DDR2_800
#define DDR2_CL                          6
#define DDR2_PAGE_SIZE            PAGE_2KB
#define DDR2_BURST_LEN                BL_4
#define DDR2_READ_BURST_TYPE RBT_SEQUENTIAL
#define DDR2_EA_MB_SIZE                128
#define DDR2_TOTAL_MB_SIZE             (DDR2_EA_MB_SIZE*DDR2_PHYSICAL_CHIP_NUM)
#define DDR2_ROWBITS                    13
#define DDR2_COLBITS                    10
#define DDR2_BANK_BITS                   3
#define DDR2_BANK_NUM                    8
#define DDR2_PHYSICAL_DATA_BITS         16
#define DDR2_LOGICAL_DATA_BITS          32

/*---------------------------------------------------------------------------
 DDR2 Access Timing Parameters
---------------------------------------------------------------------------*/
#define DDR2_tRFC_ps                127500
#define DDR2_tREFI_ps              7800000
#define DDR2_tRRD_ps                 10000
#define DDR2_tRRD_ck                     1
#define DDR2_tRAS_ps                 45000
#define DDR2_tRAS_ck                     1
#define DDR2_tRC_ps                  60000
#define DDR2_tRC_ck                      1
#define DDR2_tRCD_ps                 15000
#define DDR2_tRCD_ck                     1
#define DDR2_tRP_ps                  15000
#define DDR2_tRP_ck                      1
#define DDR2_tWTR_ps                  7500
#define DDR2_tWTR_ck                     2
#define DDR2_tWR_ps                  15000
#define DDR2_tWR_ck                      1
#define DDR2_tESR_ck                   200
#define DDR2_tFAW_ps                 45000
#define DDR2_tFAW_ck                     1
#define DDR2_tXSR_ps  (DDR2_tRFC_ps+10000)
#define DDR2_tXSR_ck                   200
#define DDR2_tXP_ck                      2
#define DDR2_tMRD_ck                     2

#elif defined(CONFIG_DDR2_HY5PS1G831CFPS6) /*==============================*/

/*---------------------------------------------------------------------------
 DDR2 Configuation
---------------------------------------------------------------------------*/
#define DDR2_PHYSICAL_CHIP_NUM           4
#define DDR2_LOGICAL_CHIP_NUM            1
#define DDR2_MAX_SPEED            DDR2_800
#define DDR2_CL                          6
#define DDR2_PAGE_SIZE            PAGE_1KB
#define DDR2_BURST_LEN                BL_4
#define DDR2_READ_BURST_TYPE RBT_SEQUENTIAL
#define DDR2_EA_MB_SIZE                128
#define DDR2_TOTAL_MB_SIZE             (DDR2_EA_MB_SIZE*DDR2_PHYSICAL_CHIP_NUM)
#define DDR2_ROWBITS                    14
#define DDR2_COLBITS                    10
#define DDR2_BANK_BITS                   3
#define DDR2_BANK_NUM                    8
#define DDR2_PHYSICAL_DATA_BITS         16
#define DDR2_LOGICAL_DATA_BITS          32

/*---------------------------------------------------------------------------
 DDR2 Access Timing Parameters
---------------------------------------------------------------------------*/
#define DDR2_tRFC_ps                127500
#define DDR2_tREFI_ps              7800000
#define DDR2_tRRD_ps                  7500
#define DDR2_tRRD_ck                     1
#define DDR2_tRAS_ps                 45000
#define DDR2_tRAS_ck                     1
#define DDR2_tRC_ps                  60000
#define DDR2_tRC_ck                      1
#define DDR2_tRCD_ps                 15000
#define DDR2_tRCD_ck                     1
#define DDR2_tRP_ps                  15000
#define DDR2_tRP_ck                      1
#define DDR2_tWTR_ps                  7500
#define DDR2_tWTR_ck                     2
#define DDR2_tWR_ps                  15000
#define DDR2_tWR_ck                      1
#define DDR2_tESR_ck                   200
#define DDR2_tFAW_ps                 35000
#define DDR2_tFAW_ck                     1
#define DDR2_tXSR_ps  (DDR2_tRFC_ps+10000)
#define DDR2_tXSR_ck                   200
#define DDR2_tXP_ck                      2
#define DDR2_tMRD_ck                     2

#elif defined(CONFIG_DDR2_E2116ABSE ) /*==================================*/

/*---------------------------------------------------------------------------
 DDR2 Configuation
---------------------------------------------------------------------------*/
#define DDR2_PHYSICAL_CHIP_NUM           2
#define DDR2_LOGICAL_CHIP_NUM            1
#define DDR2_MAX_SPEED            DDR2_800
#define DDR2_CL                          6
#define DDR2_PAGE_SIZE            PAGE_2KB
#define DDR2_BURST_LEN                BL_4
#define DDR2_READ_BURST_TYPE RBT_SEQUENTIAL
#define DDR2_EA_MB_SIZE                256
#define DDR2_TOTAL_MB_SIZE             (DDR2_EA_MB_SIZE*DDR2_PHYSICAL_CHIP_NUM)
#define DDR2_ROWBITS                    14
#define DDR2_COLBITS                    10
#define DDR2_BANK_BITS                   3
#define DDR2_BANK_NUM                    8
#define DDR2_PHYSICAL_DATA_BITS         16
#define DDR2_LOGICAL_DATA_BITS          32

 /*---------------------------------------------------------------------------
 DDR2 Access Timing Parameters
---------------------------------------------------------------------------*/
#define DDR2_tRFC_ps                195000
#define DDR2_tREFI_ps              7800000
#define DDR2_tRRD_ps                 10000
#define DDR2_tRRD_ck                     1
#define DDR2_tRAS_ps                 45000
#define DDR2_tRAS_ck                     1
#define DDR2_tRC_ps                  60000
#define DDR2_tRC_ck                      1
#define DDR2_tRCD_ps                 18000
#define DDR2_tRCD_ck                     1
#define DDR2_tRP_ps                  18000
#define DDR2_tRP_ck                      1
#define DDR2_tWTR_ps                  7500
#define DDR2_tWTR_ck                     2
#define DDR2_tWR_ps                  15000
#define DDR2_tWR_ck                      1
#define DDR2_tESR_ck                   200
#define DDR2_tFAW_ps                 45000
#define DDR2_tFAW_ck                     1
#define DDR2_tXSR_ps  (DDR2_tRFC_ps+10000)
#define DDR2_tXSR_ck                   200
#define DDR2_tXP_ck                      2
#define DDR2_tMRD_ck                     2

#else
	#error Not Selected DDR2 Memory Type
#endif

#else
	#error Not Selected Memory Type LPCON/DDR2..
#endif

#endif	/* __TCC_DDR__H__ */
/************* end of file *************************************************************/
