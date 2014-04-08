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
#define PLL4CFG		0x0050
#define PLL5CFG		0x0054
#define PCK_OFFS(pclk)	ckc + (0x0080+(pclk)*4)

enum tcc88xx_clk {
	/* core clocks */
	pll0, pll1, pll2, pll3, pll4, pll5, xin, xtin, extclkin, hdmi_tmds, hdmi_pclk, hdmi_osc,
	sata_osc, usb0_phy,
	pll0_div, pll1_div, pll2_div, pll3_div, pll4_div, pll5_div, xin_div, xtin_div,

	/* peripheral clocks */
	tcx, tct, tcz, lcd0, lcd1, lcdsi0, lcdsi1, hdmia, dsi, hdmi, usb11h, sdmmc0, mstick,
	i2c0, uart0, uart1, uart2, uart3, uart4, uart5, gpsb0, gpsb1, gpsb2, gpsb3, gpsb4, gpsb5,
	adc, spdif, ehi0, ehi1, pdm, sdmmc1, sdmmc2, sdmmc3, dai1, aud0, dai2, i2c1, sata_ref0,
	sata_ref1, sata_oob0, sata_oob1, usb20h_phy, gmac, cifmc, cifsc, ispj, isps, filter,
	out0, out1,

	clk_max
};

static void __iomem *ckc;

static const char *sel_pck[] = {
	"pll0", "pll1", "pll2", "pll3", "xin", "pll0_div", "pll1_div", "pll2_div", "pll3_div",
	"xtin", "extclkin", "hdmi_tmds", "hdmi_pclk", "hdmi_osc", "sata_osc", "usb0_phy",
	"xin_div", "xtin_div", "pll4", "pll5", "pll4_div", "pll5_div",
};

static struct clk *tcc_clk_div(const char *name, const char *parent, unsigned regoff, unsigned bitoff)
{
	return clk_register_divider(NULL, name, parent, 0, ckc + regoff, bitoff, 6, 0, &tcc_ckc_lock);
}

static struct clk *tcc88xx_clk_pll(const char *name, const char *parent, void __iomem *reg)
{
	/* XXX PLL{1,2,3}CFG M=8:16, PLL{0,4,5}CFG M=8:17 */
	return tcc_clk_pll(name, parent, reg, BIT(31), BIT(30), /* s */ 24, 0x7, /* m */8, 0x3ff, /* p */0, 0x3f);
}

static struct clk *tcc88xx_clk_per(const char *name, const char **parents, uint32_t nparents, void __iomem *reg)
{
	return tcc_clk_per(name, parents, nparents, reg, BIT(28), /* div */ 0, 0xfff, /* src */ 24, 0xf);
}

static struct clk *clks[clk_max];
static struct clk_onecell_data clk_data;

int __init tcc88xx_clocks_init(void)
{
	struct device_node *np;
	int i;

	np = of_find_compatible_node(NULL, NULL, "tcc,tcc88xx-ckc");
	ckc = of_iomap(np, 0);
	BUG_ON(!ckc);

	clks[xin] = tcc_clk_fixed("xin", 12000000);
	clks[xtin] = tcc_clk_fixed("xtin", 12000000);
	clks[pll0] = tcc88xx_clk_pll("pll0", "xin", ckc + PLL0CFG);
	clks[pll1] = tcc88xx_clk_pll("pll1", "xin", ckc + PLL1CFG);
	clks[pll2] = tcc88xx_clk_pll("pll2", "xin", ckc + PLL2CFG);
	clks[pll3] = tcc88xx_clk_pll("pll3", "xin", ckc + PLL3CFG);
	clks[pll4] = tcc88xx_clk_pll("pll4", "xin", ckc + PLL4CFG);
	clks[pll5] = tcc88xx_clk_pll("pll5", "xin", ckc + PLL5CFG);
	clks[pll0_div] = tcc_clk_div("pll0_div", "pll0", CLKDIVC0, 24);
	clks[pll1_div] = tcc_clk_div("pll1_div", "pll1", CLKDIVC0, 16);
	clks[pll2_div] = tcc_clk_div("pll2_div", "pll2", CLKDIVC0, 8);
	clks[pll3_div] = tcc_clk_div("pll3_div", "pll3", CLKDIVC0, 0);
	clks[pll4_div] = tcc_clk_div("pll4_div", "pll4", CLKDIVC1, 24);
	clks[pll5_div] = tcc_clk_div("pll5_div", "pll5", CLKDIVC1, 16);
	clks[xin_div] = tcc_clk_div("xin_div", "xin", CLKDIVC1, 8);
	clks[xtin_div] = tcc_clk_div("xtin_div", "xtin", CLKDIVC1, 0);

	clks[extclkin] = tcc_clk_fixed("extclkin", 0); // XXX ?????
	clks[hdmi_osc] = tcc_clk_fixed("hdmi_osc", 27000000);
	clks[hdmi_pclk] = tcc_clk_fixed("hdmi_pclk", 0); // XXX actually variable
	clks[hdmi_tmds] = tcc_clk_fixed("hdmi_tmds", 0); // XXX ?????
	clks[sata_osc] = tcc_clk_fixed("sata_osc", 25000000);
	clks[usb0_phy] = tcc_clk_fixed("usb0_phy", 48000000);

	clks[tcx] = tcc88xx_clk_per("tcx", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(0));
	clks[tct] = tcc88xx_clk_per("tct", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(1));
	clks[tcz] = tcc88xx_clk_per("tcz", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(2));
	clks[lcd0] = tcc88xx_clk_per("lcd0", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(3));
	clks[lcd1] = tcc88xx_clk_per("lcd1", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(4));
	clks[lcdsi0] = tcc88xx_clk_per("lcdsi0", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(5));
	clks[lcdsi1] = tcc88xx_clk_per("lcdsi1", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(6));
	clks[cifmc] = tcc88xx_clk_per("hdmia", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(7));	// YYY
	clks[cifsc] = tcc88xx_clk_per("dsi", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(8));
	clks[out0] = tcc88xx_clk_per("hdmi", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(10));
	clks[out1] = tcc88xx_clk_per("usb11h", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(11));
	clks[hdmi] = tcc88xx_clk_per("sdmcc0", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(12));
	clks[usb11h] = tcc88xx_clk_per("mstick", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(13));
	clks[sdmmc0] = tcc88xx_clk_per("i2c0", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(14));
	clks[uart0] = tcc88xx_clk_per("uart0", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(15));
	clks[uart1] = tcc88xx_clk_per("uart1", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(16));
	clks[uart2] = tcc88xx_clk_per("uart2", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(17));
	clks[uart3] = tcc88xx_clk_per("uart3", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(18));
	clks[uart4] = tcc88xx_clk_per("uart4", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(19));
	clks[uart5] = tcc88xx_clk_per("uart5", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(20));
	clks[gpsb0] = tcc88xx_clk_per("gpsb0", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(21));
	clks[gpsb1] = tcc88xx_clk_per("gpsb1", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(22));
	clks[gpsb2] = tcc88xx_clk_per("gpsb2", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(23));
	clks[gpsb3] = tcc88xx_clk_per("gpsb3", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(24));
	clks[gpsb4] = tcc88xx_clk_per("gpsb4", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(25));
	clks[gpsb5] = tcc88xx_clk_per("gpsb5", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(26));
	clks[adc] = tcc88xx_clk_per("adc", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(27));	// YYY
	clks[spdif] = tcc88xx_clk_per("spdif", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(28));	// YYY
	clks[ehi0] = tcc88xx_clk_per("ehi0", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(29));
	clks[ehi1] = tcc88xx_clk_per("ehi1", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(30));
	clks[pdm] = tcc88xx_clk_per("pdm", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(32));
	clks[sdmmc1] = tcc88xx_clk_per("sdmmc1", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(33));
	clks[sdmmc2] = tcc88xx_clk_per("sdmmc2", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(34));
	clks[sdmmc3] = tcc88xx_clk_per("sdmmc3", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(35));
	clks[dai1] = tcc88xx_clk_per("dai1", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(36));	// YYY
	clks[aud0] = tcc88xx_clk_per("aud0", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(37));	// YYY
	clks[dai2] = tcc88xx_clk_per("dai2", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(38));	// YYY
	clks[i2c1] = tcc88xx_clk_per("i2c1", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(39));
	clks[sata_ref0] = tcc88xx_clk_per("sata_ref0", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(40));
	clks[sata_ref1] = tcc88xx_clk_per("sata_ref1", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(41));
	clks[sata_oob0] = tcc88xx_clk_per("sata_oob0", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(42));
	clks[sata_oob1] = tcc88xx_clk_per("sata_oob1", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(43));
	clks[usb20h_phy] = tcc88xx_clk_per("usb20h_phy", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(44));
	clks[gmac] = tcc88xx_clk_per("gmac", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(45));
	clks[cifmc] = tcc88xx_clk_per("cifmc", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(46));
	clks[cifsc] = tcc88xx_clk_per("cifsc", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(47));
	clks[ispj] = tcc88xx_clk_per("ispj", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(48));
	clks[isps] = tcc88xx_clk_per("isps", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(49));
	clks[filter] = tcc88xx_clk_per("filter", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(50));
	clks[out0] = tcc88xx_clk_per("out0", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(54));
	clks[out1] = tcc88xx_clk_per("out1", sel_pck, ARRAY_SIZE(sel_pck), PCK_OFFS(55));

	for (i = 0; i < ARRAY_SIZE(clks); i++)
	if (IS_ERR(clks[i])) {
		pr_err("tcc88xx clk %d: register failed with %ld\n",
			i, PTR_ERR(clks[i]));
		return PTR_ERR(clks[i]);
	}

	clk_data.clks = clks;
	clk_data.clk_num = ARRAY_SIZE(clks);
	of_clk_add_provider(np, of_clk_src_onecell_get, &clk_data);

	return 0;
}
