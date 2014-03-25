/*
 * arch/arm/mach-tcc93xx/clock.c
 *
 * Clock driver for TCC93xx
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

/*===========================================================================

                         INCLUDE FILES FOR MODULE

===========================================================================*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/spinlock.h>

#include <mach/clock.h>
#include <mach/globals.h>
#include <mach/tca_ckc.h>
#include <linux/delay.h>

#include <mach/bsp.h>
#include <asm/io.h> 

/*===========================================================================

                 DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/
#define dbg(msg...)	if (0) { printk( "CLOCK: " msg); }

#if !defined(CONFIG_TCC93XX_AX)
#define USE_CLOCK_PMU_CTRL
#endif

static LIST_HEAD(clock_list);
static DEFINE_SPINLOCK(clock_lock);
static DEFINE_MUTEX(clock_mutex);

struct freqclk_table {
	unsigned int freq;
	int clk_src;
};

extern int android_system_booting_finished;
static int _clk_set_rate(struct clk *clk, unsigned long rate);

#define NUM_FREQS		ARRAY_SIZE(clk_table)
#define iomap_p2v(x)	io_p2v(x)

#define SYS_CLK_CH		0
#define MEM_CLK_CH		3
#define SYS_CLK_SRC		DIRECTPLL0

#define MAX_TCC_PLL		6
static unsigned int stPLLValue[MAX_TCC_PLL];

/*===========================================================================

                                Clock Sources

===========================================================================*/

/*------------------------------ xin ------------------------------*/
static struct clk clk_xin = {
	.name = "xin",
	.rate = 12000000,
};

/*----------------------------- PLL0 ------------------------------*/
static int pll0_clk_set_rate(struct clk *clk, unsigned long rate)
{
#if defined(CONFIG_MEM_CLK_SYNC_MODE)
#if defined(CONFIG_CPU_HIGHSPEED)
	if (android_system_booting_finished)
#endif
		return 0;
#endif

	if (clk->real_rate != rate) {
		tca_ckc_setpll(rate/100, SYS_CLK_CH);
		clk->real_rate = rate;

		dbg("%s: clkname:%s, cur_rate:%ld, rate:%ld\n", __func__, clk->name, clk->rate, clk->real_rate);
	}
	return 0;
}

static struct clk clk_pll0 = {
	.name = "pll0",
	.flags = CLK_ALWAYS_ENABLED,
	.set_rate = pll0_clk_set_rate,
};

/*----------------------------- PLL1 ------------------------------*/
static struct clk clk_pll1 = {
	.name = "pll1",
	.flags = CLK_ALWAYS_ENABLED,
};

/*----------------------------- PLL2 ------------------------------*/
static struct clk clk_pll2 = {
	.name = "pll2",
	.flags = CLK_ALWAYS_ENABLED,
};

/*----------------------------- PLL3 ------------------------------*/
static int pll3_clk_set_rate(struct clk *clk, unsigned long rate)
{
#if defined(CONFIG_MEM_CLK_SYNC_MODE)

	if (clk->real_rate != rate) {
		tca_ckc_setpll(rate/100, MEM_CLK_CH);
		clk->real_rate = rate;

		dbg("%s: clkname:%s, cur_rate:%ld, rate:%ld\n", __func__, clk->name, clk->rate, clk->real_rate);
	}

	stPLLValue[3] = rate/100;
#endif
	return 0;
}

static struct clk clk_pll3 = {
	.name = "pll3",
	.flags = CLK_ALWAYS_ENABLED,
	.set_rate = pll3_clk_set_rate,
};

/*----------------------------- PLL4 ------------------------------*/
static struct clk clk_pll4 = {
	.name = "pll4",
	.flags = CLK_ALWAYS_ENABLED,
};

/*----------------------------- PLL5 ------------------------------*/
static struct clk clk_pll5 = {
	.name = "pll5",
	.flags = CLK_ALWAYS_ENABLED,
};


/*===========================================================================

                               System Clocks

===========================================================================*/

/*------------------------------ CPU ------------------------------*/
static unsigned long cpu_clk_get_rate(struct clk *clk)
{
	return tca_ckc_getcpu() * 100;
}

static int cpu_clk_set_rate(struct clk *clk, unsigned long rate)
{
	int n;
	unsigned long pll;

#if defined(CONFIG_MEM_CLK_SYNC_MODE)
#if defined(CONFIG_CPU_HIGHSPEED)
	if (android_system_booting_finished)
#endif
		return 0;
#endif

	if (clk->real_rate != rate) {
		pll = tca_ckc_getpll(SYS_CLK_CH);
		n = ((rate/100)*16) / pll;
		if (!n) ++n;	// n must be greater than 0
		tca_ckc_setcpu(n);
		clk->real_rate = (pll/16)*n*100;

		dbg("%s: clkname:%s, cur_rate:%ld, rate:%ld->%ld\n", __func__, clk->name, clk->rate, rate, clk->real_rate);
	}
	return 0;
}

static struct clk clk_cpu = {
	.name = "cpu",
	.get_rate = cpu_clk_get_rate,
	.set_rate = cpu_clk_set_rate,
};

/*-------------------------- Memory Bus ---------------------------*/
static int membus_clk_set_rate(struct clk *clk, unsigned long rate)
{
#if 0
	struct clk *pclk;

	list_for_each_entry(pclk, &clock_list, list) {
		if (pclk->parent == clk && pclk->rate > rate)
			rate = pclk->rate;
	}
#endif
	if (clk->real_rate != rate) {
		tca_ckc_set_mem_freq(rate / 1000);
		clk->real_rate = rate;
		dbg("%s: clkname:%s, cur_rate:%ld, rate:%ld\n", __func__, clk->name, clk->rate, rate);
	}
	return 0;
}

static struct clk clk_membus = {
	.name = "membus",
	.set_rate = membus_clk_set_rate,
};

/*------------------------------ SMU ------------------------------*/
static int smubus_clk_set_rate(struct clk *clk, unsigned long rate)
{
#if defined(CONFIG_MEM_CLK_SYNC_MODE)
#if defined(CONFIG_CPU_HIGHSPEED)
	if (android_system_booting_finished)
#endif
		return 0;
#endif

	tca_ckc_setfbusctrl(CLKCTRL7,  ENABLE, NORMAL_MD, rate, SYS_CLK_SRC); /*FBUS_SMU*/
	clk->rate = rate;
	return 0;
}

static struct clk clk_smu = {
	.name = "smu",
	.flags = CLK_ALWAYS_ENABLED,
	.set_rate = smubus_clk_set_rate,
};


/*===========================================================================
          Clock Control Functions
===========================================================================*/

static int fclk_enable(struct clk *clk)
{
	dbg("%s: clk:%s\n", __func__, clk->name);
#if defined(USE_CLOCK_PMU_CTRL)
	if((tca_ckc_getpmupwroff(clk->pwdn_idx+2) == 1) && clk->pwdn) {
		if (clk->pwdn) {
			clk->pwdn(clk->pwdn_idx+1, 0);	// BUSPRE
			clk->pwdn(clk->pwdn_idx+2, 0);	// BUS
			clk->pwdn(clk->pwdn_idx, 1);	// UBSISOLATION
			ndelay(100);
		}
		if (clk->swreset) {
			clk->swreset(clk->swreset_idx, 0);
			if (clk->swreset_idx == RESET_VIDEOBUS)
				clk->swreset(RESET_VIDEOCORE, 0);
		}
	}
#endif
	if (clk->clock)
		clk->clock(clk->clock_idx, 1);
	return 0;
}

static void fclk_disable(struct clk *clk)
{
	dbg("%s: clk:%s\n", __func__, clk->name);
	if (clk->clock)
		clk->clock(clk->clock_idx, 0);
#if defined(USE_CLOCK_PMU_CTRL)
	if (clk->swreset) {
		if (clk->swreset_idx == RESET_VIDEOBUS)
			clk->swreset(RESET_VIDEOCORE, 0);
		clk->swreset(clk->swreset_idx, 0);
	}
	if (clk->pwdn) {
		ndelay(100);
		clk->pwdn(clk->pwdn_idx, 0);	// UBSISOLATION
		clk->pwdn(clk->pwdn_idx+1, 1);	// BUSPRE
		clk->pwdn(clk->pwdn_idx+2, 1);	// BUS
	}
#endif
}

static int fclk_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned int div[MAX_TCC_PLL], div_100[MAX_TCC_PLL], i, clksrc, searchsrc, overclksrc;
	unsigned long clkrate;

	searchsrc = 0xFFFFFFFF;
	overclksrc = 0xFFFFFFFF;

	rate /= 100;

	if (rate == 0) {
		clksrc = DIRECTXIN;
		clkrate = 60000;
	}
	else {
		for (i=0 ; i<MAX_TCC_PLL ; i++) {
			if (stPLLValue[i] == 0 || i == SYS_CLK_CH || i == MEM_CLK_CH)
				continue;
			div_100[i] = stPLLValue[i]/(rate/100);
			/* find maximum frequency pll source */
			if (div_100[i] <= 100) {
				if (overclksrc == 0xFFFFFFFF)
					overclksrc = i;
				else if (div_100[i] < div_100[overclksrc])
					overclksrc = i;
				continue;
			}

			div[i]= div_100[i]/100;
			if (div_100[i]%100)
				div[i] += 1;

			if (div[i] < 2)
				div[i] = 2;

			div_100[i] = rate - stPLLValue[i]/div[i];

			if (searchsrc == 0xFFFFFFFF)
				searchsrc = i;
			else {
				/* find similar clock */
				if (div_100[i] < div_100[searchsrc])
					searchsrc = i;
				/* find even division vlaue */
				else if(div_100[i] == div_100[searchsrc]) {
					if (div[searchsrc]%2)
						searchsrc = i;
				}
			}
		}
		if (searchsrc == 0xFFFFFFFF) {
			searchsrc = overclksrc;
			div[searchsrc] = 2;
		}		
		switch(searchsrc) {
			case 0: clksrc = DIRECTPLL0; break;
			case 1: clksrc = DIRECTPLL1; break;
			case 2: clksrc = DIRECTPLL2; break;
			case 3: clksrc = DIRECTPLL3; break;
			case 4: clksrc = DIRECTPLL4; break;
			case 5: clksrc = DIRECTPLL5; break;
			default: printk("%s: failed by invalid clock source\n", __func__); return -1;
		}
		clkrate = stPLLValue[searchsrc]/((div[searchsrc]>16)?16:div[searchsrc]);
	}
	dbg("%s: clkname:%s, cur_rate:%ld, rate:%ld->%ld\n", __func__, clk->name, clk->rate, rate*100, clkrate*100);
	if (clk->real_rate != (clkrate*100)) {
		tca_ckc_setfbusctrl(clk->clock_idx, NOCHANGE, NORMAL_MD, clkrate, clksrc);
		clk->real_rate = clkrate*100;
	}
	return 0;
}

static unsigned long fclk_get_rate(struct clk *clk)
{
	return (unsigned long)(clk->real_rate);
}

static int phy_enable(struct clk *clk)
{
	dbg("%s: clk:%s\n", __func__, clk->name);
	tca_ckc_setpmupwroff(clk->pwdn_idx, 1);
	return 0;
}

static void phy_disable(struct clk *clk)
{
	dbg("%s: clk:%s\n", __func__, clk->name);
	tca_ckc_setpmupwroff(clk->pwdn_idx, 0);
}

static int pclk_enable(struct clk *clk)
{
	dbg("%s: clk:%s\n", __func__, clk->name);
	if (clk->pwdn)
		clk->pwdn(clk->pwdn_idx, 1);
	if (clk->clock)
		clk->clock(clk->clock_idx, 1);
	if (clk->swreset)
		clk->swreset(clk->pwdn_idx, 0);
	return 0;
}

static void pclk_disable(struct clk *clk)
{
	dbg("%s: clk:%s\n", __func__, clk->name);
	if (clk->swreset)
		clk->swreset(clk->pwdn_idx, 1);
	if (clk->clock)
		clk->clock(clk->clock_idx, 0);
	if (clk->pwdn)
		clk->pwdn(clk->pwdn_idx, 0);
}

static int pclk_set_rate(struct clk *clk, unsigned long rate)
{
	volatile unsigned long *pPERI;
	unsigned int div[MAX_TCC_PLL], div_100[MAX_TCC_PLL], i, clksrc, searchsrc, dco_shift, md;
	unsigned long clkrate;
	PCKC pCKC = (PCKC)(iomap_p2v((unsigned int)&HwCLK_BASE));

	clkrate = rate/100;

	if (clk->clock_idx == PERI_ADC)
		md = 1;
	else
		md = 0;

	if (clk->clock_idx == PERI_AUD || clk->clock_idx == PERI_AUD1) {
		if (clkrate < 65536)
			dco_shift = 0;
		else if (clkrate < 65536*2)  //  13.1072 MHz
			dco_shift = 1;
		else if (clkrate < 65536*4)  //  26.2144 MHz
			dco_shift = 2;
		else if (clkrate < 65536*8)  //  52.4288 MHz
			dco_shift = 3;
		else if (clkrate < 65536*16) // 104.8596 MHz
			dco_shift = 4;
		else                         // 209.7152 MHz
			dco_shift = 5;

		div_100[3] = ((clkrate*(65536>>dco_shift))/(stPLLValue[3]/100))<<dco_shift;
		if ((div_100[3]%100) > 50) {
			div[3] = div_100[3]/100 + 1;
			div_100[3] = 100 - (div_100[3]%100);
		}
		else {
			div[3] = div_100[3]/100;
			div_100[3] %= 100;
		}

		clksrc = PCDIRECTPLL3;
		searchsrc = 3;

		if ((div[searchsrc]) > (32768))
			clkrate = ((stPLLValue[searchsrc]>>dco_shift)*(65536-div[searchsrc]))/(65536>>dco_shift);
		else
			clkrate = ((stPLLValue[searchsrc]>>dco_shift)*div[searchsrc])/(65536>>dco_shift);
	}
#if defined(CONFIG_MEM_CLK_SYNC_MODE)
	else if (clk->clock_idx == PERI_SPDIF || clk->clock_idx == PERI_SPDIF1) {
		div_100[3] = stPLLValue[3]/(clkrate/100);
		if ((div_100[3]%100) > 50) {
			div[3] = div_100[3]/100 + 1;
			div_100[3] = 100 - (div_100[3]%100);
		}
		else {
			div[3] = div_100[3]/100;
			div_100[3] %= 100;
		}
		clksrc = PCDIRECTPLL3;
		clkrate = stPLLValue[3]/div[3];
		searchsrc = 3;

		if (div[searchsrc] > 2)
			div[searchsrc] -= 1;
		if (div[searchsrc] == 0)
			div[searchsrc] = 1;
	}
#endif
	else if (clk->clock_idx == PERI_I2C0 || clk->clock_idx == PERI_I2C1 || clk->clock_idx == PERI_I2C2
		  || clk->clock_idx == PERI_ADC || clk->clock_idx == PERI_PDM) {
		clksrc = PCDIRECTXIN;
		searchsrc = 0;
		div[searchsrc] = 120000/clkrate;
//		clkrate = rate;

		if (div[searchsrc] > 2)
			div[searchsrc] -= 1;
		if (div[searchsrc] == 0)
			div[searchsrc] = 1;
	}
	else {
		searchsrc = 0xFFFFFFFF;
		for (i=0 ; i<MAX_TCC_PLL ; i++) {
			if (stPLLValue[i] == 0 || i == SYS_CLK_CH || i == MEM_CLK_CH)
				continue;
			div_100[i] = stPLLValue[i]/(clkrate/100);
			if ((div_100[i]%100) > 50) {
				div[i] = div_100[i]/100 + 1;
				div_100[i] = 100 - (div_100[i]%100);
			}
			else {
				div[i] = div_100[i]/100;
				div_100[i] %= 100;
			}
			if (searchsrc == 0xFFFFFFFF)
				searchsrc = i;
			else {
				/* find similar clock */
				if (div_100[i] < div_100[searchsrc])
					searchsrc = i;
				/* find even division vlaue */
				else if(div_100[i] == div_100[searchsrc]) {
					if (div[searchsrc]%2)
						searchsrc = i;
				}
			}
		}
		switch(searchsrc) {
			case 0: clksrc = PCDIRECTPLL0; break;
			case 1: clksrc = PCDIRECTPLL1; break;
			case 2: clksrc = PCDIRECTPLL2; break;
			case 3: clksrc = PCDIRECTPLL3; break;
			case 4: clksrc = PCDIRECTPLL4; break;
			case 5: clksrc = PCDIRECTPLL5; break;
			default: clksrc = PCDIRECTPLL1; break;
		}

		clkrate = stPLLValue[searchsrc]/div[searchsrc];

		if (div[searchsrc] > 2)
			div[searchsrc] -= 1;
		if (div[searchsrc] == 0)
			div[searchsrc] = 1;
	}

	dbg("%s: clkname:%s, clkrate:%ld, rate:%ld->rate:%ld\n", __func__, clk->name, clk->rate, rate, clkrate*100);
#if 1
	pPERI =(volatile unsigned long*)((&pCKC->PCLK_TCX)+(clk->clock_idx));

	if (clk->rate != clkrate*100) {
		*pPERI = (md<<31)|(((clk->usecount)?1:0)<<29)|(clksrc<<24)|(div[searchsrc]<<0);
		clk->rate = clkrate*100;
	}
#else
	if (clk->rate != clkrate*100) {
		if (clk->usecount)
			tca_ckc_setperi(clk->clock_idx, ENABLE, clkrate, clksrc);
		else
			tca_ckc_setperi(clk->clock_idx, DISABLE, clkrate, clksrc);
		clk->rate = clkrate*100;
	}
#endif
	return 0;
}

static unsigned long pclk_get_rate(struct clk *clk)
{
	return (unsigned long)(tca_ckc_getperi(clk->clock_idx) * 100);
}


static struct clk clk_ddi = {
	.name         = "ddi",
	.flags        = CLK_AUTO_OFF,
	.enable       = fclk_enable,
	.disable      = fclk_disable,
	.set_rate     = fclk_set_rate,
	.pwdn_idx     = PMU_DDIBUSISOLATION,
	.pwdn         = tca_ckc_setpmupwroff,
	.swreset_idx  = RESET_DDIBUS,
	.swreset      = tca_ckc_setswreset,
	.clock_idx    = CLKCTRL1,
	.clock        = tca_ckc_enable,
};

static struct clk clk_gpu = {
	.name         = "gpu",
	.flags        = CLK_AUTO_OFF,
	.enable       = fclk_enable,
	.disable      = fclk_disable,
	.set_rate     = fclk_set_rate,
	.pwdn_idx     = PMU_GRAPHICBUSISOLATION,
	.pwdn         = tca_ckc_setpmupwroff,
	.swreset_idx  = RESET_GRAPBUS,
	.swreset      = tca_ckc_setswreset,
	.clock_idx    = CLKCTRL3,
	.clock        = tca_ckc_enable,
};

static struct clk clk_io = {
	.name         = "iobus",
	.flags        = CLK_ALWAYS_ENABLED,
	.enable       = fclk_enable,
	.disable      = fclk_disable,
	.set_rate     = fclk_set_rate,
	.clock_idx    = CLKCTRL4,
};

static struct clk clk_vbus = {
	.name         = "vbus",
	.flags        = CLK_AUTO_OFF,
	.enable       = fclk_enable,
	.disable      = fclk_disable,
	.set_rate     = fclk_set_rate,
	.pwdn_idx     = PMU_VIDEOBUSISOLATION,
	.pwdn         = tca_ckc_setpmupwroff,
	.swreset_idx  = RESET_VIDEOBUS,
	.swreset      = tca_ckc_setswreset,
	.clock_idx    = CLKCTRL5,
	.clock        = tca_ckc_enable,
};

static struct clk clk_vcore = {
	.name         = "vcore",
	.flags        = CLK_AUTO_OFF,
	.enable       = fclk_enable,
	.disable      = fclk_disable,
	.set_rate     = fclk_set_rate,
	.clock_idx    = CLKCTRL6,
	.clock        = tca_ckc_enable,
};

static struct clk clk_hsio = {
	.name         = "hsio",
	.flags        = CLK_AUTO_OFF,
	.enable       = fclk_enable,
	.disable      = fclk_disable,
	.set_rate     = fclk_set_rate,
	.pwdn_idx     = PMU_HSBUSISOLATION,
	.pwdn         = tca_ckc_setpmupwroff,
	.swreset_idx  = RESET_HSBUS,
	.swreset      = tca_ckc_setswreset,
	.clock_idx    = CLKCTRL8,
	.clock        = tca_ckc_enable,
};

static struct clk clk_cam = {
	.name         = "cam",
	.flags        = CLK_AUTO_OFF,
	.enable       = fclk_enable,
	.disable      = fclk_disable,
	.set_rate     = fclk_set_rate,
	.pwdn_idx     = PMU_CAMBUSISOLATION,
	.pwdn         = tca_ckc_setpmupwroff,
	.swreset_idx  = RESET_CAMBUS,
	.swreset      = tca_ckc_setswreset,
	.clock_idx    = CLKCTRL9,
	.clock        = tca_ckc_enable,
};

static struct clk clk_ddisub = {
	.name         = "ddisub",
	.flags        = CLK_AUTO_OFF,
	.enable       = fclk_enable,
	.disable      = fclk_disable,
	.set_rate     = fclk_set_rate,
	.pwdn_idx     = PMU_DDISUBBUSISOLATION,
	.pwdn         = tca_ckc_setpmupwroff,
	.swreset_idx  = RESET_DDISUBBUS,
	.swreset      = tca_ckc_setswreset,
	.clock_idx    = CLKCTRL10,
	.clock        = tca_ckc_enable,
};


/**************************************/
/* Display Bus Clock */

static struct clk clk_viqe = {
	.name      = "viqe",
	.parent    = &clk_ddi,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = DDIPWDN_VIQE,
	.pwdn      = tca_ckc_setddipwdn,
	.swreset   = tca_ckc_setddiswreset,
};

static struct clk clk_lcdc0 = {
	.name      = "lcdc0",
	.parent    = &clk_ddi,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = DDIPWDN_LCDC0,
	.pwdn      = tca_ckc_setddipwdn,
	.swreset   = tca_ckc_setddiswreset,
	.clock_idx = PERI_LCD0,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_lcdc1 = {
	.name      = "lcdc1",
	.parent    = &clk_ddi,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = DDIPWDN_LCDC1,
	.pwdn      = tca_ckc_setddipwdn,
	.swreset   = tca_ckc_setddiswreset,
	.clock_idx = PERI_LCD1,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_lcdsi0 = {
	.name      = "lcdsi0",
	.parent    = &clk_ddi,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = DDIPWDN_LCDSI0,
	.pwdn      = tca_ckc_setddipwdn,
	.swreset   = tca_ckc_setddiswreset,
	.clock_idx = PERI_LCDSI0,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_lcdsi1 = {
	.name      = "lcdsi1",
	.parent    = &clk_ddi,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = DDIPWDN_LCDSI1,
	.pwdn      = tca_ckc_setddipwdn,
	.swreset   = tca_ckc_setddiswreset,
	.clock_idx = PERI_LCDSI1,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_m2m0 = {
	.name      = "m2m0",
	.parent    = &clk_ddi,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = DDIPWDN_MSCL0,
	.pwdn      = tca_ckc_setddipwdn,
	.swreset   = tca_ckc_setddiswreset,
};

static struct clk clk_m2m1 = {
	.name      = "m2m1",
	.parent    = &clk_ddi,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = DDIPWDN_MSCL1,
	.pwdn      = tca_ckc_setddipwdn,
	.swreset   = tca_ckc_setddiswreset,
};

static struct clk clk_ddi_cache = {
	.name      = "ddi_cache",
	.parent    = &clk_ddi,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = DDIPWDN_DDIC,
	.pwdn      = tca_ckc_setddipwdn,
	.swreset   = tca_ckc_setddiswreset,
};

static struct clk clk_hdmi_phy = {
	.name      = "hdmi_phy",
	.parent    = &clk_ddi,
	.flags     = CLK_AUTO_OFF,
	.enable    = phy_enable,
	.disable   = phy_disable,
	.pwdn_idx  = PMU_HDMIPHY,
};

static struct clk clk_hdmi = {
	.name      = "hdmi",
	.parent    = &clk_hdmi_phy,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = DDIPWDN_HDMI,
	.pwdn      = tca_ckc_setddipwdn,
	.swreset   = tca_ckc_setddiswreset,
	.clock_idx = PERI_HDMI,
	.clock     = tca_ckc_pclk_enable,
};


/**************************************/
/* Graphic Bus Clock */

static struct clk clk_mali = {
	.name      = "mali_clk",
	.parent    = &clk_gpu,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = GRPPWDN_MALI,
	.pwdn      = tca_ckc_setgrppwdn,
	.swreset   = tca_ckc_setgrpswreset,
};


/**************************************/
/* I/O Bus Clock */

static struct clk clk_pwb = {
	.name      = "prefetch",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_PREFETCHWRITEBUFFER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_ehi0 = {
	.name      = "ehi0",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_EHI0,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_EHI0,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_ehi1 = {
	.name      = "ehi1",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_EHI1,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_EHI1,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_usb_otg = {
	.name      = "usb_otg",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_USB20OTG,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_nfc = {
	.name      = "nfc",
	.parent    = &clk_io,
	.flags     = CLK_ALWAYS_ENABLED,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_NFCCONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_NFC,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_sdhc = {
	.name      = "sdhc",
	.parent    = &clk_io,
	.flags     = CLK_ALWAYS_ENABLED,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_SDMMCPORTMUXCONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_sdhc0 = {
	.name      = "sdhc0",
	.parent    = &clk_sdhc,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_SDMMC0CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_SDMMC0,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_sdhc1 = {
	.name      = "sdhc1",
	.parent    = &clk_sdhc,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_SDMMC1CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_SDMMC1,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_sdhc2 = {
	.name      = "sdhc2",
	.parent    = &clk_sdhc,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_SDMMC2CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_SDMMC2,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_gdma0 = {
	.name      = "gdma0",
	.parent    = &clk_io,
	.flags     = CLK_ALWAYS_ENABLED,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_GDMA0CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_gdma1 = {
	.name      = "gdma1",
	.parent    = &clk_io,
	.flags     = CLK_ALWAYS_ENABLED,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_GDMA1CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_gdma2 = {
	.name      = "gdma2",
	.parent    = &clk_io,
	.flags     = CLK_ALWAYS_ENABLED,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_GDMA2CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_pwm = {
	.name      = "pwm",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_PWMCONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_PDM,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_overlay = {
	.name      = "overlay",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_2DCONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_sdhc3 = {
	.name      = "sdhc3",
	.parent    = &clk_sdhc,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_SDMMC3CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_SDMMC3,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_remote = {
	.name      = "remocon",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_REMOTECONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_tsif0 = {
	.name      = "tsif0",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_TSIF0CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_tsif1 = {
	.name      = "tsif1",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_TSIF1CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_adma0 = {
	.name      = "adma0",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_ADMA0CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_dai0 = {
	.name      = "dai0",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_DAI0CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_AUD,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_adma1 = {
	.name      = "adma1",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_ADMA1CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_dai1 = {
	.name      = "dai1",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_DAI1CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_AUD1,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_uart0 = {
	.name      = "uart0",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_UARTCONTROLLER0,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_UART0,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_uart1 = {
	.name      = "uart1",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_UARTCONTROLLER1,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_UART1,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_uart2 = {
	.name      = "uart2",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_UARTCONTROLLER2,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_UART2,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_uart3 = {
	.name      = "uart3",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_UARTCONTROLLER3,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_UART3,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_uart4 = {
	.name      = "uart4",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_UARTCONTROLLER4,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_UART4,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_uart5 = {
	.name      = "uart5",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_UARTCONTROLLER5,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_UART5,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_i2c0 = {
	.name      = "i2c0",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_I2CCONTROLLER0,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_I2C0,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_i2c1 = {
	.name      = "i2c1",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_I2CCONTROLLER1,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_I2C1,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_i2c2 = {
	.name      = "i2c2",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_I2CCONTROLLER2,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_I2C2,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_gpsb0 = {
	.name      = "gpsb0",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_GPSBCONTROLLER0,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_GPSB0,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_gpsb1 = {
	.name      = "gpsb1",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_GPSBCONTROLLER1,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_GPSB1,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_gpsb2 = {
	.name      = "gpsb2",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_GPSBCONTROLLER2,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_GPSB2,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_gpsb3 = {
	.name      = "gpsb3",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_GPSBCONTROLLER3,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_GPSB3,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_gpsb4 = {
	.name      = "gpsb4",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_GPSBCONTROLLER4,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_GPSB4,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_gpsb5 = {
	.name      = "gpsb5",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_GPSBCONTROLLER5,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_GPSB5,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_adc = {
	.name      = "adc",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_TSADCCONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_ADC,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_gpio = {
	.name      = "gpio",
	.parent    = &clk_io,
	.flags     = CLK_ALWAYS_ENABLED,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_GPIOCONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_spdif0 = {
	.name      = "spdif0",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_SPDIF0CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_SPDIF,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_protect = {
	.name      = "protect",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = RB_PROTECTCONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
};

static struct clk clk_spdif1 = {
	.name      = "spdif1",
	.parent    = &clk_io,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = RB_SPDIF1CONTROLLER,
	.pwdn      = tca_ckc_setiobus,
	.swreset   = tca_ckc_setioswreset,
	.clock_idx = PERI_SPDIF1,
	.clock     = tca_ckc_pclk_enable,
};


/**************************************/
/* Video Bus Clock */

static struct clk clk_vcache = {
	.name      = "vcache",
    .parent    = &clk_vbus,
    .flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = VIDEOBUSCFG_PWDNVIDEOCACHE,
	.pwdn      = tca_ckc_setvideobuscfgpwdn,
	.swreset   = tca_ckc_setvideobuscfgswreset,
};

static struct clk clk_vcodec = {
	.name      = "vcodec",
    .parent    = &clk_vbus,
    .flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = VIDEOBUSCFG_PWDNVIDEOCODEC,
	.pwdn      = tca_ckc_setvideobuscfgpwdn,
	.swreset   = tca_ckc_setvideobuscfgswreset,
};

static struct clk clk_jpege = {
	.name      = "jpege",
    .parent    = &clk_vbus,
    .flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = VIDEOBUSCFG_PWDNJPEGENC,
	.pwdn      = tca_ckc_setvideobuscfgpwdn,
	.swreset   = tca_ckc_setvideobuscfgswreset,
};

static struct clk clk_videodac = {
	.name      = "videodac",
	.flags     = CLK_AUTO_OFF,
	.enable    = phy_enable,
	.disable   = phy_disable,
	.pwdn_idx  = PMU_VIDEODAC,
};


/**************************************/
/* High speed I/O Bus Clock */

static struct clk clk_hs_pwb = {
	.name      = "hs_pwb",
	.parent    = &clk_hsio,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = HSIO_PWB,
	.pwdn      = tca_ckc_sethsiobus,
	.swreset   = tca_ckc_sethsiobusswreset,
};

static struct clk clk_sata_ref = {
	.name      = "sata_ref",
	.parent    = &clk_hsio,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.clock_idx = PERI_SATA_REF,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_satad = {
	.name      = "satad",
	.parent    = &clk_hsio,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = HSIO_SATAD,
	.pwdn      = tca_ckc_sethsiobus,
	.swreset   = tca_ckc_sethsiobusswreset,
};

static struct clk clk_gmac = {
	.name      = "gmac",
	.parent    = &clk_hsio,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = HSIO_GMAC,
	.pwdn      = tca_ckc_sethsiobus,
	.swreset   = tca_ckc_sethsiobusswreset,
	.clock_idx = PERI_GMAC,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_hs_usb_otg = {
	.name      = "hs_usb_otg",
	.parent    = &clk_hsio,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = HSIO_USBOTG,
	.pwdn      = tca_ckc_sethsiobus,
	.swreset   = tca_ckc_sethsiobusswreset,
};

static struct clk clk_hs_gdma = {
	.name      = "hs_gdma",
	.parent    = &clk_hsio,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = HSIO_GDMA,
	.pwdn      = tca_ckc_sethsiobus,
	.swreset   = tca_ckc_sethsiobusswreset,
};

static struct clk clk_mstick = {
	.name      = "mstick",
	.parent    = &clk_hsio,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = HSIO_MSTICK,
	.pwdn      = tca_ckc_sethsiobus,
	.swreset   = tca_ckc_sethsiobusswreset,
	.clock_idx = PERI_MSTICK,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_cipher = {
	.name      = "cipher",
	.parent    = &clk_hsio,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = HSIO_CIPHER,
	.pwdn      = tca_ckc_sethsiobus,
	.swreset   = tca_ckc_sethsiobusswreset,
};

static struct clk clk_satah = {
	.name      = "satah",
	.parent    = &clk_hsio,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.pwdn_idx  = HSIO_SATAH,
	.pwdn      = tca_ckc_sethsiobus,
	.swreset   = tca_ckc_sethsiobusswreset,
	.clock_idx = PERI_SATA,
	.clock     = tca_ckc_pclk_enable,
};


/**************************************/
/* Camera Bus Clock */

static struct clk clk_cif = {
	.name      = "cif",
	.parent    = &clk_cam,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = CAMBUS_CIF,
	.pwdn      = tca_ckc_setcambuspwdn,
	.swreset   = tca_ckc_setcambusswreset,
};

static struct clk clk_isp = {
	.name      = "isp",
	.parent    = &clk_cam,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = CAMBUS_ISP,
	.pwdn      = tca_ckc_setcambuspwdn,
	.swreset   = tca_ckc_setcambusswreset,
};

static struct clk clk_cifmc = {
	.name      = "cifmc",
	.parent    = &clk_cif,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.clock_idx = PERI_CIFMC,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_cifsc = {
	.name      = "cifsc",
	.parent    = &clk_cif,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.clock_idx = PERI_CIFSC,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_isps = {
	.name      = "isps",
	.parent    = &clk_isp,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.clock_idx = PERI_ISPS,
	.clock     = tca_ckc_pclk_enable,
};

static struct clk clk_ispj = {
	.name      = "ispj",
	.parent    = &clk_isp,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.set_rate  = pclk_set_rate,
	.get_rate  = pclk_get_rate,
	.clock_idx = PERI_ISPJ,
	.clock     = tca_ckc_pclk_enable,
};


/**************************************/
/* Display Sub Bus Clock */

static struct clk clk_mipi = {
	.name      = "mipi",
	.parent    = &clk_ddisub,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = DDISUBBUS_HCLK_MIPIDSI,
	.pwdn      = tca_ckc_setddisubbus,
	.swreset   = tca_ckc_setddisubbusswreset,
};

static struct clk clk_mipi_ehi = {
	.name      = "mipi_ehi",
	.parent    = &clk_ddisub,
	.flags     = CLK_AUTO_OFF,
	.enable    = pclk_enable,
	.disable   = pclk_disable,
	.pwdn_idx  = DDISUBBUS_HCLK_EHI,
	.pwdn      = tca_ckc_setddisubbus,
	.swreset   = tca_ckc_setddisubbusswreset,
};


/*===========================================================================

                          Register Clock Driver

===========================================================================*/

static struct clk *onchip_clocks[] = {
	// clock source
	&clk_xin,
//	&clk_xtin,
	&clk_pll0,
	&clk_pll1,
	&clk_pll2,
	&clk_pll3,
	&clk_pll4,
	&clk_pll5,

	// System clocks
	&clk_cpu,
	&clk_membus,
	&clk_smu,

	// Display Bus
	&clk_viqe,			// bit 1
	&clk_lcdc0,			// bit 2
	&clk_lcdc1,			// bit 3
	&clk_lcdsi0,		// bit 4
	&clk_m2m0,			// bit 5
	&clk_m2m1,			// bit 6
	&clk_ddi_cache,		// bit 7
	&clk_hdmi,			// bit 8
	&clk_lcdsi1,		// bit 9
	&clk_hdmi_phy,
	&clk_ddi,

	// Graphic Bus
	&clk_mali,			// bit 0
	&clk_gpu,

	// IO Bus
//	&clk_pwb,			// bit 0
	&clk_ehi0,			// bit 1
	&clk_ehi1,			// bit 2
	&clk_usb_otg,		// bit 3
//	&clk_nfc,			// bit 4
	&clk_sdhc,			// bit 5
	&clk_sdhc0,			// bit 6
	&clk_sdhc1,			// bit 7
	&clk_sdhc2,			// bit 8
//	&clk_gdma0,			// bit 9
//	&clk_gdma1,			// bit 10
//	&clk_gdma2,			// bit 11
//	&clk_pwm,			// bit 12
	&clk_overlay,		// bit 13
	&clk_sdhc3,			// bit 14
	&clk_remote,		// bit 15
//	&clk_tsif0,			// bit 16
//	&clk_tsif1,			// bit 17
	&clk_adma0,			// bit 18
	&clk_dai0,			// bit 19
//	&clk_adma1,			// bit 20
//	&clk_dai1,			// bit 21
	&clk_uart0,			// bit 22
	&clk_uart1,			// bit 23
	&clk_uart2,			// bit 24
	&clk_uart3,			// bit 25
	&clk_uart4,			// bit 26
	&clk_uart5,			// bit 27
	&clk_i2c0,			// bit 28
	&clk_i2c1,			// bit 29
	&clk_i2c2,			// bit 30
	&clk_gpsb0,			// bit 31
	&clk_gpsb1,			// bit 32
	&clk_gpsb2,			// bit 33
	&clk_gpsb3,			// bit 34
	&clk_gpsb4,			// bit 35
	&clk_gpsb5,			// bit 36
	&clk_adc,			// bit 37
//	&clk_gpio,			// bit 38
	&clk_spdif0,		// bit 39
//	&clk_protect,		// bit 40
	&clk_spdif1,		// bit 41
	&clk_io,

	// Video Bus
	&clk_jpege,			// bit 0
	&clk_vcodec,		// bit 2
	&clk_vcache,		// bit 3
	&clk_videodac,
	&clk_vcore,
	&clk_vbus,

	// HSIO Bus
	&clk_sata_ref,
//	&clk_hs_pwb,		// bit 0
	&clk_satad,			// bit 1
	&clk_gmac,			// bit 2
	&clk_hs_usb_otg,	// bit 3
	&clk_hs_gdma,		// bit 4
	&clk_mstick,		// bit 5
	&clk_cipher,		// bit 6
	&clk_satah,			// bit 7
	&clk_hsio,

	// Camera Bus
	&clk_cif,			// bit 0
	&clk_isp,			// bit 1
	&clk_cifmc,
	&clk_cifsc,
	&clk_isps,
	&clk_ispj,
	&clk_cam,

	// Display Sub Bus
	&clk_mipi_ehi,		// bit 0
	&clk_mipi,			// bit 1
	&clk_ddisub,
};

struct clk *clk_get(struct device *dev, const char *id)
{
	struct clk *clk;
	mutex_lock(&clock_mutex);
	list_for_each_entry(clk, &clock_list, list) {
		if (!strcmp(id, clk->name) && clk->dev == dev)
			goto found;
		if (!strcmp(id, clk->name) && clk->dev == NULL)
			goto found;
	}
	clk = ERR_PTR(-ENOENT);
found:
	mutex_unlock(&clock_mutex);
	return clk;
}
EXPORT_SYMBOL(clk_get);

void clk_put(struct clk *clk)
{
	/* do nothing */
}
EXPORT_SYMBOL(clk_put);

static int _clk_enable(struct clk *clk)
{
	int ret = 0;
	clk->initialized = 1;
	clk->usecount++;
	if (clk->usecount == 1) {
		if (clk->parent)
			ret = _clk_enable(clk->parent);
		if (clk->enable)
			ret = clk->enable(clk);
		else
			pr_warning("clock: %s: unimplemented function 'clk_enable' called\n", clk->name);
	}
	return ret;
}

int clk_enable(struct clk *clk)
{
	unsigned long flags;
	int ret = 0;
	BUG_ON(clk == 0);
	spin_lock_irqsave(&clock_lock, flags);
	ret = _clk_enable(clk);
	spin_unlock_irqrestore(&clock_lock, flags);
	return ret;
}
EXPORT_SYMBOL(clk_enable);

static void _clk_disable(struct clk *clk)
{
	clk->initialized = 1;
	if (clk->flags & CLK_ALWAYS_ENABLED)
		return;
	clk->usecount--;
	if (clk->usecount == 0) {
		if (clk->disable)
			(*clk->disable)(clk);
		else
			pr_warning("clock: %s: unimplemented function 'clk_disable' called\n", clk->name);
		if (clk->parent)
			_clk_disable(clk->parent);
	}
}

void clk_disable(struct clk *clk)
{
	unsigned long flags;
	BUG_ON(clk->usecount == 0);

	spin_lock_irqsave(&clock_lock, flags);
	_clk_disable(clk);
	spin_unlock_irqrestore(&clock_lock, flags);
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	unsigned long rate;
	mutex_lock(&clock_mutex);
	if (clk->get_rate)
		rate = (*clk->get_rate)(clk);
	else
		rate = clk->rate;
	mutex_unlock(&clock_mutex);
	return rate;
}
EXPORT_SYMBOL(clk_get_rate);

/*
 * Set the clock rate for a clock source
 */
static int _clk_set_rate(struct clk *clk, unsigned long rate)
{
	int ret = 0;
	unsigned long flags;
	if (clk->flags & CLK_FIXED_RATE)
		return 0;
	spin_lock_irqsave(&clock_lock, flags);
	if (clk->set_rate)
		ret = (*clk->set_rate)(clk, rate);
	else
		pr_warning("clock: unimplemented function 'clk_set_rate' called\n");
	spin_unlock_irqrestore(&clock_lock, flags);
	return ret;
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	int ret = 0;
	if (clk->rate == rate)
		return 0;
	ret = _clk_set_rate(clk, rate);
	if (ret == 0)
		clk->rate = rate;
	return ret;
}
EXPORT_SYMBOL(clk_set_rate);

/*
 * Adjust a rate to the exact rate a clock can provide
 */
long clk_round_rate(struct clk *clk, unsigned long rate)
{
	//TODO
	return rate;
}
EXPORT_SYMBOL(clk_round_rate);

/*
 * Set the parent clock source for the clock
 */
int clk_set_parent(struct clk *clk, struct clk *parent)
{
	unsigned long flags;
	spin_lock_irqsave(&clock_lock, flags);
	clk->parent = parent;
	spin_unlock_irqrestore(&clock_lock, flags);
	return 0;
}
EXPORT_SYMBOL(clk_set_parent);

/*
 * Get the parent clock source for the clock
 */
struct clk *clk_get_parent(struct clk *clk)
{
	BUG_ON(clk == 0);
	return clk->parent;
}
EXPORT_SYMBOL(clk_get_parent);

int __init clk_init(void)
{
	struct clk **clk;
	int i;


	spin_lock_init(&clock_lock);
	mutex_lock(&clock_mutex);

	tca_ckc_init();

	/* Read fixed pll values */
	for (i=0 ; i<MAX_TCC_PLL ; i++) {
#if defined(CONFIG_MEM_CLK_SYNC_MODE)
		if (i == SYS_CLK_CH) {
			stPLLValue[i] = 0;
			pr_info("pll_%d:  cpu/memory clcok source\n", i);
		}
#else
		if (i == SYS_CLK_CH) {
			stPLLValue[i] = 0;
			pr_info("pll_%d:  cpu clcok source\n", i);
		}
		else if (i == MEM_CLK_CH) {
			stPLLValue[i] = 0;
			pr_info("pll_%d:  memory clcok source\n", i);
		}
#endif
		else {
			stPLLValue[i] = tca_ckc_getpll(i);
			pr_info("pll_%d:  %d kHz (Fixed)\n", i, stPLLValue[i]/10);
		}
	}
	
	for (clk = onchip_clocks; clk < onchip_clocks + ARRAY_SIZE(onchip_clocks); clk++) {
		(*clk)->rate = 0xFFFFFFFF;
		list_add_tail(&(*clk)->list, &clock_list);
	}

	mutex_unlock(&clock_mutex);

	pr_info("TCC clock driver initialized\n");
	return 0;
}
arch_initcall(clk_init);

void clk_disable_unused(void)
{
	struct clk *clk;
	int count = 0;
	list_for_each_entry(clk, &clock_list, list) {
		if (clk->flags & CLK_AUTO_OFF && !clk->usecount && clk->disable && !clk->initialized) {
			clk->disable(clk);
			count++;
			pr_debug("clock: clock '%s' disabled\n", clk->name);
		}
	}
	pr_debug("clock: disabled %d unused clocks\n", count);
}
EXPORT_SYMBOL(clk_disable_unused);

static int __init clock_late_init(void)
{
	clk_disable_unused();
	return 0;
}

late_initcall(clock_late_init);

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static void *tcc93xx_ck_start(struct seq_file *m, loff_t *pos)
{
	return *pos < 1 ? (void *)1 : NULL;
}

static void *tcc93xx_ck_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void tcc93xx_ck_stop(struct seq_file *m, void *v)
{
}

static int tcc93xx_ck_show(struct seq_file *m, void *v)
{
#if 0
	struct clk *cp;
	
	list_for_each_entry(cp, &clock_list, list)
		if(!strcmp(cp->name,"cpu") || !strcmp(cp->name,"membus") ||
				!strcmp(cp->name,"ddi") || !strcmp(cp->name,"gpu") || !strcmp(cp->name,"iobus") ||
				!strcmp(cp->name,"vbus") || !strcmp(cp->name,"vcore") || !strcmp(cp->name,"smu"))
			seq_printf(m,"%ld ", clk_get_rate(cp));
#else
	// cpu
	seq_printf(m, "%ld", tca_ckc_getcpu()/10000);
	// mem bus
	seq_printf(m, "\t%ld ", clk_get_rate(&clk_membus)/1000000);
	// smu
	seq_printf(m, "\t%ld ", tca_ckc_getfbusctrl(CLKCTRL7)/10000);
	// ddi
	seq_printf(m, "\t%ld ", tca_ckc_getfbusctrl(clk_ddi.clock_idx)/10000);
	// gpu
	seq_printf(m, "\t%ld ", tca_ckc_getfbusctrl(clk_gpu.clock_idx)/10000);
	// io
	seq_printf(m, "\t%ld ", tca_ckc_getfbusctrl(clk_io.clock_idx)/10000);
	// vcore
	seq_printf(m, "\t%ld ", tca_ckc_getfbusctrl(clk_vcore.clock_idx)/10000);
	// vbus
	seq_printf(m, "\t%ld ", tca_ckc_getfbusctrl(clk_vbus.clock_idx)/10000);
#endif
	seq_printf(m,"\n");

	return 0;
}

static const struct seq_operations tcc93xx_ck_op = {
	.start	= tcc93xx_ck_start,
	.next	= tcc93xx_ck_next,
	.stop	= tcc93xx_ck_stop,
	.show	= tcc93xx_ck_show
};

static int tcc93xx_ck_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &tcc93xx_ck_op);
}

static const struct file_operations proc_tcc93xx_ck_operations = {
	.open		= tcc93xx_ck_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static int __init tcc93xx_ck_proc_init(void)
{
	proc_create("clocks", 0, NULL, &proc_tcc93xx_ck_operations);
	return 0;

}
__initcall(tcc93xx_ck_proc_init);
