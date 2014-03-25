/*
 * linux/include/asm-arm/arch-tcc92x/system.h 
 *
 * Author: <linux@telechips.com>
 * Created: June 10, 2008
 * Description: LINUX SYSTEM FUNCTIONS for TCC92x
 *
 * Copyright (C) 2008-2009 Telechips 
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
	if (machine_is_tcc9200s()) {
		cpu_do_idle();
	} else {
		tcc_idle();
	}
}

static inline void arch_reset(char mode, const char *cmd)
{
#if defined(CONFIG_REGULATOR)
	struct regulator *vdd_core;
#endif
	volatile PPMU pPMU = (volatile PPMU)(tcc_p2v(HwPMU_BASE));	//0xF0404000
	volatile PIOBUSCFG pIOBUSCFG = (volatile PIOBUSCFG)(tcc_p2v(HwIOBUSCFG_BASE)); //0xF05F5000

#if defined(CONFIG_REGULATOR)
	vdd_core = regulator_get(NULL, "vdd_core");
	if (IS_ERR(vdd_core))
		vdd_core = NULL;
	else
	{
		regulator_set_voltage(vdd_core, 1200000, 1200000);
		regulator_put(vdd_core);
	}
#endif

	pIOBUSCFG->HCLKEN0 = -1;
	pIOBUSCFG->HCLKEN1 = -1;
	
	while (1) {
		pPMU->WATCHDOG = (Hw31 + 0x1);
	}
}

#endif
