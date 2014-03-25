/* linux/arch/arm/mach-tcc93xx/devices.h
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

#ifndef __ARCH_ARM_MACH_TCC93XX_DEVICES_H
#define __ARCH_ARM_MACH_TCC93XX_DEVICES_H

extern struct platform_device tcc_lcd_device;
extern struct platform_device tcc9300_i2c_core0_device;
extern struct platform_device tcc9300_i2c_core1_device;
extern struct platform_device tcc9300_i2c_core2_device;
extern struct platform_device tcc9300_i2c_smu_device;

extern struct platform_device tcc9300_sdhc0_device;
extern struct platform_device tcc9300_sdhc1_device;
extern struct platform_device tcc9300_sdhc2_device;
extern struct platform_device tcc9300_sdhc3_device;

extern int tcc_panel_id;

extern struct platform_device tcc9300_adc_device; // kch
#ifdef CONFIG_BATTERY_TCC
extern struct platform_device tcc_battery_device; //kch
#endif

extern struct platform_device tcc9300_uart1_device;

extern struct platform_device pmem_device;
extern struct android_pmem_platform_data pmem_pdata;

extern struct platform_device pmem_adsp_device;
extern struct android_pmem_platform_data pmem_adsp_pdata;


extern struct platform_device tcc9300_uart0_device;
extern struct platform_device tcc9300_uart1_device;
extern struct platform_device tcc9300_uart2_device;
extern struct platform_device tcc9300_uart3_device;
extern struct platform_device tcc9300_uart4_device;
extern struct platform_device tcc9300_uart5_device;

#endif
