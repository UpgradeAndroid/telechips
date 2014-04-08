#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include "clk.h"

/* Clock controller register offsets */
#define PLL0CFG		0x0020
#define PLL1CFG		0x0024
#define PLL2CFG		0x0028
#define PLL3CFG		0x002c
#define CLKDIVC0	0x0030
#define CLKDIVC1	0x0034
#define PCK_OFFS(pclk)	ckc + (0x0080+(pclk)*4)

enum tcc92xx_clk {
	/* main clock sources */
	pll0, pll1, pll2, pll3, xin, xtin,
	pll0_div, pll1_div, pll2_div, pll3_div, xin_div, xtin_div,
	extclkin, hdmi_osc, hdmi_pclk, hdmi_tmds, sata_osc, usb_osc,
	/* peripheral clocks (starts at 18) */
	tcx, tct, tcz, lcd0, lcd1, lcdsi, cifmc, cifsc, out0, out1,
	hdmi, usb11h, sdmmc0, mstick, i2c, uart0, uart1, uart2, uart3,
	uart4, uart5, gpsb0, gpsb1, gpsb2, gpsb3, gpsb4, gpsb5, adc,
	spdif, ehi0, ehi1, can, sdmmc1, dai,

	clk_max
};

static void __iomem *ckc;

static const char *sel_pck_xxx[] = {
	"pll0", "pll1", "pll2", "pll3", "xin", "pll0_div", "pll1_div", "pll2_div", "pll3_div",
	"xtin", "extclkin", "xin_div", "xtin_div", "hdmi_osc", "sata_osc", "usb_osc"
};
static const char *sel_pck_xxx2[] = {
	"pll0", "pll1", "pll2", "pll3", "xin", "pll0_div", "pll1_div", "pll2_div", "pll3_div",
	"xtin", "extclkin", "hdmi_tmds", "hdmi_pclk", "hdmi_osc", "sata_osc", "usb_osc"
};
static const char *sel_pck_yyy[] = {
	"pll0", "pll1", "pll2", "pll3", "xin", "pll0_div", "pll1_div", "pll2_div", "pll3_div",
	"xtin", "extclkin", "xin_div", "xtin_div", "hdmi_osc", "sata_osc", "usb_osc"
};

static struct clk *tcc92xx_clk_div(const char *name, const char *parent, unsigned regoff, unsigned bitoff)
{
	return clk_register_divider(NULL, name, parent, 0, ckc + regoff, bitoff, 6, 0, &tcc_ckc_lock);
}

static struct clk *tcc92xx_clk_pll(const char *name, const char *parent, void __iomem *reg)
{
	return tcc_clk_pll(name, parent, reg, BIT(31), -1, /* s */ 24, 0x7, /* m */8, 0x7ff, /* p */0, 0x3f);
}

static struct clk *tcc92xx_clk_per(const char *name, const char **parents, uint32_t nparents, void __iomem *reg)
{
	return tcc_clk_per(name, parents, nparents, reg, BIT(28), /* div */ 0, 0xfff, /* src */ 24, 0xf);
}

static struct clk *clks[clk_max];
static struct clk_onecell_data clk_data;

int __init tcc92xx_clocks_init(void)
{
	struct device_node *np;
	int i;

	np = of_find_compatible_node(NULL, NULL, "tcc,tcc92xx-ckc");
	ckc = of_iomap(np, 0);
	BUG_ON(!ckc);

	/* XXX patch SDMMC controllers to have USB_PHY(48MHz) as parent */
	writel_relaxed(15 << 24, PCK_OFFS(12));
	writel_relaxed(15 << 24, PCK_OFFS(34));

	clks[xin] = tcc_clk_fixed("xin", 12000000);
	clks[xtin] = tcc_clk_fixed("xtin", 12000000);
	clks[pll0] = tcc92xx_clk_pll("pll0", "xin", ckc + PLL0CFG);
	clks[pll1] = tcc92xx_clk_pll("pll1", "xin", ckc + PLL1CFG);
	clks[pll2] = tcc92xx_clk_pll("pll2", "xin", ckc + PLL2CFG);
	clks[pll3] = tcc92xx_clk_pll("pll3", "xin", ckc + PLL3CFG);
	clks[pll0_div] = tcc92xx_clk_div("pll0_div", "pll0", CLKDIVC0, 24);
	clks[pll1_div] = tcc92xx_clk_div("pll1_div", "pll1", CLKDIVC0, 16);
	clks[pll2_div] = tcc92xx_clk_div("pll2_div", "pll2", CLKDIVC0, 8);
	clks[pll3_div] = tcc92xx_clk_div("pll3_div", "pll3", CLKDIVC0, 0);
	clks[xin_div] = tcc92xx_clk_div("xin_div", "xin", CLKDIVC1, 8);
	clks[xtin_div] = tcc92xx_clk_div("xtin_div", "xtin", CLKDIVC1, 0);

	clks[extclkin] = tcc_clk_fixed("extclkin", 0); // XXX ?????
	clks[hdmi_osc] = tcc_clk_fixed("hdmi_osc", 27000000);
	clks[hdmi_pclk] = tcc_clk_fixed("hdmi_pclk", 0); // XXX actually variable
	clks[hdmi_tmds] = tcc_clk_fixed("hdmi_tmds", 0); // XXX ?????
	clks[sata_osc] = tcc_clk_fixed("sata_osc", 25000000);
	clks[usb_osc] = tcc_clk_fixed("usb_osc", 48000000);

	clks[tcx] = tcc92xx_clk_per("tcx", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(0));
	clks[tct] = tcc92xx_clk_per("tct", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(1));
	clks[tcz] = tcc92xx_clk_per("tcz", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(2));
	clks[lcd0] = tcc92xx_clk_per("lcd0", sel_pck_xxx2, ARRAY_SIZE(sel_pck_xxx2), PCK_OFFS(3));
	clks[lcd1] = tcc92xx_clk_per("lcd1", sel_pck_xxx2, ARRAY_SIZE(sel_pck_xxx2), PCK_OFFS(4));
	clks[lcdsi] = tcc92xx_clk_per("lcdsi", sel_pck_xxx2, ARRAY_SIZE(sel_pck_xxx2), PCK_OFFS(5));
	clks[cifmc] = tcc92xx_clk_per("cifmc", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(6));
	clks[cifsc] = tcc92xx_clk_per("cifsc", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(7));
	clks[out0] = tcc92xx_clk_per("out0", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(8));
	clks[out1] = tcc92xx_clk_per("out1", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(9));
	clks[hdmi] = tcc92xx_clk_per("hdmi", sel_pck_xxx2, ARRAY_SIZE(sel_pck_xxx2), PCK_OFFS(10));
	clks[usb11h] = tcc92xx_clk_per("usb11h", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(11));
	clks[sdmmc0] = tcc92xx_clk_per("sdmmc0", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(12));
	clks[mstick] = tcc92xx_clk_per("mstick", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(13));
	clks[i2c] = tcc92xx_clk_per("i2c", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(14));
	clks[uart0] = tcc92xx_clk_per("uart0", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(15));
	clks[uart1] = tcc92xx_clk_per("uart1", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(16));
	clks[uart2] = tcc92xx_clk_per("uart2", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(17));
	clks[uart3] = tcc92xx_clk_per("uart3", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(18));
	clks[uart4] = tcc92xx_clk_per("uart4", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(19));
	clks[uart5] = tcc92xx_clk_per("uart5", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(20));
	clks[gpsb0] = tcc92xx_clk_per("gpsb0", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(21));
	clks[gpsb1] = tcc92xx_clk_per("gpsb1", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(22));
	clks[gpsb2] = tcc92xx_clk_per("gpsb2", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(23));
	clks[gpsb3] = tcc92xx_clk_per("gpsb3", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(24));
	clks[gpsb4] = tcc92xx_clk_per("gpsb4", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(25));
	clks[gpsb5] = tcc92xx_clk_per("gpsb5", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(26));
	clks[adc] = tcc92xx_clk_per("adc", sel_pck_yyy, ARRAY_SIZE(sel_pck_yyy), PCK_OFFS(27));
	clks[spdif] = tcc92xx_clk_per("spdif", sel_pck_yyy, ARRAY_SIZE(sel_pck_yyy), PCK_OFFS(28));
	clks[ehi0] = tcc92xx_clk_per("ehi0", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(29));
	clks[ehi1] = tcc92xx_clk_per("ehi1", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(30));
	clks[can] = tcc92xx_clk_per("can", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(32));
	clks[sdmmc1] = tcc92xx_clk_per("sdmmc1", sel_pck_xxx, ARRAY_SIZE(sel_pck_xxx), PCK_OFFS(34));
	clks[dai] = tcc92xx_clk_per("dai", sel_pck_yyy, ARRAY_SIZE(sel_pck_yyy), PCK_OFFS(36));

	for (i = 0; i < ARRAY_SIZE(clks); i++)
	if (IS_ERR(clks[i])) {
		pr_err("tcc92xx clk %d: register failed with %ld\n",
			i, PTR_ERR(clks[i]));
		return PTR_ERR(clks[i]);
	}

	clk_data.clks = clks;
	clk_data.clk_num = ARRAY_SIZE(clks);
	of_clk_add_provider(np, of_clk_src_onecell_get, &clk_data);

	return 0;
}
