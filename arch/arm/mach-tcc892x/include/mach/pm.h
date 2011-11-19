/*
 * arch/arm/mach-tcc88xx/pm.h
 *
 * Author:  <linux@telechips.com>
 * Created: April 21, 2008
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
 */
#ifndef __TCC_PM_H__
#define __TCC_PM_H__

/*===========================================================================

                                  MODE

===========================================================================*/

/*===========================================================================

                              Shut-down MAP

===========================================================================*/

/*---------------------------------------------------------------------------
 1) Shut-down (shut down + sram boot + sdram self-refresh)

     << sram >>
     0x10000000(0xEFF00000) ------------------
                           |    Boot code     | 0x700
                     0x700  ------------------
                           |      Stack       | 0x100
                     0x800  ------------------
                           |   I/O restore    | 0x200
                     0xA00  ------------------
                           |   SDRAM Init     | 0x700
                    0x1100  ------------------
                           | GPIO Backup Data | 0x300
                    0x1400  ------------------
                           |  Enter Shutdown  | 0x400
                    0x1800  ------------------
                           |   mmu on code    | 0x50
                    0x1850  ------------------
                           | cpu_reg/mmu data | 0x100
                    0x1950  ------------------
#ifdef PORT_ARRANGE_USED
                           | GPIO Backup Data | 0x300
                    0x1C50  ------------------
#endif
                           |                  |
                             : : : : : : : : :
                           |                  |
                    0x2000  ------------------  
                           | core power ctrl. | 0x1000
                    0x3000  ------------------

---------------------------------------------------------------------------*/

#define SRAM_BOOT_ADDR           0xEFF00000
#define SRAM_BOOT_SIZE           0x00000700

#define SHUTDOWN_STACK_ADDR      0xEFF00700
#define SHUTDOWN_STACK_SIZE      0x00000100
#define SHUTDOWN_STACK_PHY       0x10000700

#define IO_RESTORE_FUNC_ADDR     0xEFF00800
#define IO_RESTORE_FUNC_SIZE     0x00000200

#define GPIO_BUF_ADDR            0xEFF01100
#define GPIO_BUF_PHY             0x10001100

#define SHUTDOWN_FUNC_ADDR       0xEFF01400
#define SHUTDOWN_FUNC_SIZE       0x00000400
#define SHUTDOWN_FUNC_PHY        0x10001400

#define MMU_SWITCH_FUNC_ADDR     0xEFF01800
#define MMU_SWITCH_FUNC_SIZE     0x00000020
#define MMU_SWITCH_FUNC_PHY      0x10001800

#define REG_MMU_DATA_ADDR        0xEFF01850
#define REG_MMU_DATA_PHY         0x10001850

#define MMU_SWITCH_EXEC_ADDR     0xF0700100

#define SDRAM_INIT_FUNC_ADDR     0xEFF00A00
#define SDRAM_INIT_FUNC_SIZE     0x00000700
#define SDRAM_INIT_FUNC_PHY      0x10000A00

#define COREPWR_FUNC_ADDR        0xEFF02000
#define COREPWR_FUNC_SIZE        0x00000FE0
#define COREPWR_FUNC_PHY         0x10002000

#define COREPWR_PARAM_ADDR       0xEFF02FE0
#define COREPWR_PARAM_SIZE       0x00000020
#define COREPWR_PARAM_PHY        0x10002FE0

/*-------------------------------------------------------------------------*/


/*===========================================================================

                        Shut-down Backup Registers

===========================================================================*/

typedef struct _TCC_REG_{
	CKC ckc;
	PIC pic;
	VIC vic;
	TIMER timer;
	PMU pmu;
	SMUCONFIG smuconfig;
	//GPIO gpio;

	IOBUSCFG iobuscfg;

	volatile unsigned int backup_peri_iobus0;
	volatile unsigned int backup_peri_iobus1;
} TCC_REG, *PTCC_REG;

/*-------------------------------------------------------------------------*/

#endif  /*__TCC_PM_H__*/
