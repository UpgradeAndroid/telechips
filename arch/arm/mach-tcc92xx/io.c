/*
 * linux/arch/arm/mach-tcc92xx/io.c
 *
 * Author:  <linux@telechips.com>
 * Created: 10th February, 2008
 * Description: TCC92xx/TCC890x mapping code
 *
 * Copyright (C) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/bootmem.h>
#include <linux/init.h>

#include <asm/tlb.h>
#include <asm/mach/map.h>
#include <asm/io.h>

#include <mach/bsp.h>
#include <plat/pmap.h>

/*
 * The machine specific code may provide the extra mapping besides the
 * default mapping provided here.
 */
static struct map_desc tcc9200_io_desc[] __initdata = {
    {
        .virtual    = 0xF0000000,
        .pfn        = __phys_to_pfn(0xF0000000),   
        .length     = 0x100000,                   
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF0100000,
        .pfn        = __phys_to_pfn(0xF0100000),
        .length     = 0x100000,               
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF0200000,
        .pfn        = __phys_to_pfn(0xF0200000),
        .length     = 0x100000,                
        .type       = MT_DEVICE
    },
#if 1    
    {
        .virtual    = 0xF0300000,
        .pfn        = __phys_to_pfn(0xF0300000),
        .length     = 0x100000,                
        .type       = MT_DEVICE_NONSHARED
    },
#endif    
    {
        .virtual    = 0xF0400000,
        .pfn        = __phys_to_pfn(0xF0400000),
        .length     = 0x100000,                
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF0500000,
        .pfn        = __phys_to_pfn(0xF0500000),
        .length     = 0x100000,                
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF0600000,
        .pfn        = __phys_to_pfn(0xF0600000),
        .length     = 0x100000,                
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF0700000,
        .pfn        = __phys_to_pfn(0xF0700000),
        .length     = 0x100000,                
        .type       = MT_DEVICE
    },    
#if 1
    {
        .virtual    = 0xF0800000,
        .pfn        = __phys_to_pfn(0x10000000),
        .length     = 0x100000,                
        .type       = MT_MEMORY_TCC
    },
#endif    
};

#if 0
static struct map_desc tcc92x_sram_desc[] __initdata = {
	{
		.virtual	= 0xF0300000,
		.pfn		= __phys_to_pfn(0xF0300000),
		.length		= 0x100000,
		.type		= MT_DEVICE,
	}, 
};
#endif

static void tcc_reserve_sdram(void)
{
	pmap_t pmap;

	if (pmap_get_info("total", &pmap)) {
		if (reserve_bootmem(pmap.base, pmap.size, BOOTMEM_EXCLUSIVE) == 0) {
			printk(KERN_DEBUG "Total reserved memory: base=0x%x, size=0x%x\n", pmap.base, pmap.size);
		} else {
			printk(KERN_ERR "Can't reserve memory (base=0x%x, size=0x%x)\n", pmap.base, pmap.size);
		}
	}
}

/*
 *  Maps common IO regions for TCC92xx and TCC890x.
 */
void __init tcc9200_map_common_io(void)
{
	iotable_init(tcc9200_io_desc, ARRAY_SIZE(tcc9200_io_desc));

	/* Normally devicemaps_init() would flush caches and tlb after
	 * mdesc->map_io(), but we must also do it here because of the CPU
	 * revision check below.
	 */
	local_flush_tlb_all();
	flush_cache_all();

	tcc_reserve_sdram();
}
