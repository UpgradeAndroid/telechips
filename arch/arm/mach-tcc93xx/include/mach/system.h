/*
 * linux/arch/arm/mach-tcc93xx/include/mach 
 *
 * Author: <linux@telechips.com>
 * Created: August 30, 2010
 * Description: LINUX SYSTEM FUNCTIONS
 *
 * Copyright (C) 2010 Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H
#include <linux/clk.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <linux/regulator/consumer.h>

#include <mach/bsp.h>
extern void plat_tcc_reboot(void);

extern inline void tcc_idle(void);

static inline void arch_idle(void)
{
    cpu_do_idle();
	//tcc_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
#if defined(CONFIG_REGULATOR)
	struct regulator *vdd_core;
#endif
	volatile PPMU pPMU = (volatile PPMU)(tcc_p2v(HwPMU_BASE));
	volatile PIOBUSCFG pIOBUSCFG = (volatile PIOBUSCFG)(tcc_p2v(HwIOBUSCFG_BASE));

#if defined(CONFIG_REGULATOR)
	vdd_core = regulator_get(NULL, "vdd_coreA");
	if (IS_ERR(vdd_core))
		vdd_core = NULL;
	else
	{
		regulator_set_voltage(vdd_core, 1200000, 1200000);
		regulator_put(vdd_core);
	}
#endif

	pPMU->CONFIG1 = pPMU->CONFIG1 & 0xFFFFF8FF;				// Set REMAP field as 0b000	

	pPMU->PWROFF = 0x18924928;		// PMU is not reset with WATCHDOG. It must be properly initialized.

	pIOBUSCFG->HCLKMASK0 = 0;
	pIOBUSCFG->HCLKMASK1 = 0;
	
	while (1) {
		pPMU->WATCHDOG = (Hw31 + 0x1);
	}
}

#endif
