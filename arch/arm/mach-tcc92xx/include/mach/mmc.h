/* linux/arch/arm/mach-tcc92xx/include/mach/mmc.h
 *
 * Copyright (C) 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __TCC92XX_MMC_H
#define __TCC92XX_MMC_H

#include <linux/types.h>
#include <linux/device.h>
#include <linux/mmc/host.h>

struct tcc_mmc_platform_data {
	int slot;
	unsigned int caps;
	unsigned int f_min;
	unsigned int f_max;
	u32 ocr_mask;

	unsigned int cd_int_num;

	int (*init)(struct device *dev, int id);

	int (*card_detect)(struct device *dev, int id);

	int (*suspend)(struct device *dev, int id);
	int (*resume)(struct device *dev, int id);

	int (*set_power)(struct device *dev, int id, int on);

	int (*set_bus_width)(struct device *dev, int id, int width);
};

#endif
