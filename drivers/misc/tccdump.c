#include <linux/module.h>
#include <linux/io.h>

#include <mach/bsp.h>

/* Compatibility hack for 2.3/4.x TCC release */
#ifndef HwCLK_BASE
#define HwCLK_BASE	HwCKC_BASE
#endif

static void
dump_regs(void *regs, long count, const char *header)
{
	u32 *r = (u32 *)regs;
	long i;

	pr_info("Dump of %s\n", header);
	for (i=0; i < count; i++)
		pr_info("[%04lx]=%08x\n", i*sizeof(u32), r[i]);
}

static int
tccdump_init(void)
{
	dump_regs((void*)tcc_p2v(HwCLK_BASE), 88, "CKC");
	dump_regs((void*)tcc_p2v(HwGPIOA_BASE), 128, "GPIO");
	dump_regs((void*)tcc_p2v(HwLCDC0_BASE), 88, "LCDC0");
	dump_regs((void*)tcc_p2v(HwLCDC1_BASE), 88, "LCDC1");
	dump_regs((void*)tcc_p2v(HwDDI_CONFIG_BASE), 24, "DDICONFIG");
	dump_regs((void*)tcc_p2v(HwDDICACHE_BASE), 128, "DDICACHE");
	return -ENODEV;
}
module_init(tccdump_init);
