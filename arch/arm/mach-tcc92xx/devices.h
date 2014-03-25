/* linux/arch/arm/mach-tcc92xx/devices.h
 *
 * Copyright (C) 2010 Telechips, Inc.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __ARCH_ARM_MACH_TCC92XX_DEVICES_H
#define __ARCH_ARM_MACH_TCC92XX_DEVICES_H

extern struct platform_device tcc9200_i2c_device;
extern struct platform_device tcc9200_smu_device;
extern int tcc_panel_id;
extern struct platform_device tcc_lcd_device;
extern struct platform_device tcc_otg_device;
#if !defined(CONFIG_MMC_TCC_4SD_SLOT)
extern struct platform_device tcc_mmc_core0_device;
extern struct platform_device tcc_mmc_core1_device;
#else
extern struct platform_device tcc_mmc_core0_slot0_device;
extern struct platform_device tcc_mmc_core0_slot1_device;
extern struct platform_device tcc_mmc_core1_slot2_device;
extern struct platform_device tcc_mmc_core1_slot3_device;
#endif

extern struct platform_device tcc9200_uart0_device;
extern struct platform_device tcc9200_uart1_device;
extern struct platform_device tcc9200_uart2_device;
extern struct platform_device tcc9200_uart3_device;
extern struct platform_device tcc9200_uart4_device;
extern struct platform_device tcc9200_uart5_device;

#endif
