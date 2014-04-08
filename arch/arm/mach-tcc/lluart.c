/*
 * Static memory mapping for DEBUG_LL
 *
 * Copyright (c) 2014 Upgrade Android.
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/kernel.h>
#include <asm/page.h>
#include <asm/mach/map.h>

#define TCC_UART0_PA_BASE	0xf0532000
#define TCC_UART0_VA_BASE	0xf0532000

void __init tcc_map_lluart(void)
{
	struct map_desc lluart_map = {
		.virtual        = TCC_UART0_VA_BASE,
		.pfn            = __phys_to_pfn(TCC_UART0_PA_BASE),
		.length         = SZ_4K,
		.type           = MT_DEVICE,
	};

	iotable_init(&lluart_map, 1);
}
