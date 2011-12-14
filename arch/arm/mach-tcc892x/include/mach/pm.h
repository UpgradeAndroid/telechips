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


//#define TCC_PM_PMU_CTRL


/*===========================================================================

                              Shut-down MAP

===========================================================================*/

/*---------------------------------------------------------------------------
 1) Shut-down (shut down + sram boot + sdram self-refresh)

     << sram >>
     0xF0000000(0x10000000) ------------------
                           |    Boot code     | 0x700
                     0x700  ------------------
                           |      Stack       | 0x100
                     0x800  ------------------
                           |     Shutdown     | 0x400
                     0xC00  ------------------
                           |     Wake-up      | 0x400
                    0x1000  ------------------
                           |    SDRAM Init    | 0x700
                    0x1700  ------------------
                           |   GPIO Storage   | 0x300
                    0x1A00  ------------------
                           |   cpu_reg/mmu data | 0x100
                    0x1B00  ------------------

---------------------------------------------------------------------------*/

#define SRAM_BOOT_ADDR           0xF0000000
#define SRAM_BOOT_SIZE           0x00000700

#define SRAM_STACK_ADDR          0xF0000700
#define SRAM_STACK_SIZE          0x00000100

#define SHUTDOWN_FUNC_ADDR       0xF0000800
#define SHUTDOWN_FUNC_SIZE       0x00000400

#define WAKEUP_FUNC_ADDR         0xF0000C00
#define WAKEUP_FUNC_SIZE         0x00000400

#define SDRAM_INIT_FUNC_ADDR     0xF0001000
#define SDRAM_INIT_FUNC_SIZE     0x00000700

#define GPIO_REPOSITORY_ADDR     0xF0001700
#define GPIO_REPOSITORY_SIZE     0x00000300

#define CPU_DATA_REPOSITORY_ADDR 0xF0001A00
#define CPU_DATA_REPOSITORY_SIZE 0x00000100

/*-------------------------------------------------------------------------*/


/*===========================================================================

                        Shut-down Backup Registers

===========================================================================*/

typedef struct _TCC_REG_{
	CKC ckc;
	PIC pic;
	VIC vic;
	TIMER timer;
#if defined(TCC_PM_PMU_CTRL)
	PMU pmu;
#endif
	SMUCONFIG smuconfig;
	IOBUSCFG iobuscfg;
	MEMBUSCFG membuscfg;
	unsigned L2_aux;
} TCC_REG, *PTCC_REG;

/*-------------------------------------------------------------------------*/

#endif  /*__TCC_PM_H__*/
