#ifndef __TCC_CLK_H
#define __TCC_CLK_H

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/spinlock.h>

extern spinlock_t tcc_ckc_lock;

static inline struct clk *tcc_clk_fixed(const char *name, int rate)
{
	return clk_register_fixed_rate(NULL, name, NULL, CLK_IS_ROOT, rate);
}

extern struct clk *tcc_clk_pll(const char *name, const char *parent_name, void __iomem *reg,
	int enable_bit, int vco_bit, uint32_t s_shift, uint32_t s_mask, uint32_t m_shift, uint32_t m_mask,
	uint32_t p_shift, uint32_t p_mask);
extern struct clk *tcc_clk_per(const char *name, const char **parent_names, int num_parents, void __iomem *reg,
	uint32_t enable_bit, uint32_t div_shift, uint32_t div_mask, uint32_t src_shift, uint32_t src_mask);

#endif
