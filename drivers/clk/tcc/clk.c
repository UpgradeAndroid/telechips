//#define DEBUG

#include <linux/io.h>
#include <linux/slab.h>
#include <linux/err.h>

#include "clk.h"

/* All clocks share the same lock as none can be changed concurrently */
DEFINE_SPINLOCK(tcc_ckc_lock);

/* PLL clock related functions */
#define to_clk_pll(_hw) container_of(_hw, struct clk_pll, hw)

struct clk_pll {
        struct clk_hw   hw;
        void __iomem    *reg;
        const char      *name;
	spinlock_t	*lock;
	int		enable_bit;
	int		vco_bit;
	uint32_t	s_shift;
	uint32_t	s_mask;
	uint32_t	m_shift;
	uint32_t	m_mask;
	uint32_t	p_shift;
	uint32_t	p_mask;
};

static unsigned long tcc_pll_recalc_rate(struct clk_hw *hw,
                                unsigned long parent_rate)
{
        struct clk_pll *pll = to_clk_pll(hw);
        u32 pll_val = readl(pll->reg);

        if (pll_val & BIT(pll->enable_bit)) {
                unsigned long rate;
                u32 s, m, p;
                s = (pll_val >> pll->s_shift) & pll->s_mask;
                m = (pll_val >> pll->m_shift) & pll->m_mask;
                p = (pll_val >> pll->p_shift) & pll->p_mask;

                rate = ((parent_rate * m) / p) >> s;
                pr_debug("%s: %s %lu %lu\n", __func__, pll->name, parent_rate, rate);

                return rate;
        }

        return 0;
}

const struct clk_ops tcc_pll_ops = {
//      .round_rate = tcc_pll_round_rate,
//      .set_rate = tcc_pll_set_rate,
        .recalc_rate = tcc_pll_recalc_rate,
};

struct clk *tcc_clk_pll(const char *name, const char *parent_name, void __iomem *reg,
	int enable_bit, int vco_bit, uint32_t s_shift, uint32_t s_mask,
	uint32_t m_shift, uint32_t m_mask, uint32_t p_shift, uint32_t p_mask)
{
	struct clk_pll *pll;
	struct clk *clk;
	struct clk_init_data init;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	init.name = name;
	init.ops = &tcc_pll_ops;
	init.flags = 0;
	init.parent_names = (parent_name ? &parent_name : NULL);
	init.num_parents = (parent_name ? 1 : 0);

	pll->lock = &tcc_ckc_lock;
	pll->name = name;
	pll->reg = reg;
	pll->enable_bit = enable_bit;
	pll->vco_bit = vco_bit;
	pll->s_shift = s_shift;
	pll->s_mask = s_mask;
	pll->m_shift = m_shift;
	pll->m_mask = m_mask;
	pll->p_shift = p_shift;
	pll->p_mask = p_mask;
	pll->hw.init = &init;

	clk = clk_register(NULL, &pll->hw);
	if (IS_ERR(clk))
		kfree(pll);

	return clk;
}

struct clk_device {
        struct clk_hw   hw;
        void __iomem    *reg;
	spinlock_t	*lock;
        const char      *name;
	uint32_t	enable_bit;
	uint32_t	div_shift;
	uint32_t	div_mask;
	uint32_t	src_shift;
	uint32_t	src_mask;
};

#define to_clk_device(_hw) container_of(_hw, struct clk_device, hw)

static int tcc_dclk_enable(struct clk_hw *hw)
{
        struct clk_device *cdev = to_clk_device(hw);
        u32 en_val;
        unsigned long flags = 0;

        pr_debug("%s: %s\n", __func__, cdev->name);

        spin_lock_irqsave(cdev->lock, flags);

        en_val = readl_relaxed(cdev->reg);
        en_val |= BIT(cdev->enable_bit);
        writel_relaxed(en_val, cdev->reg);

        spin_unlock_irqrestore(cdev->lock, flags);
        return 0;
}

static void tcc_dclk_disable(struct clk_hw *hw)
{
        struct clk_device *cdev = to_clk_device(hw);
        u32 en_val;
        unsigned long flags = 0;

        pr_debug("%s: %s\n", __func__, cdev->name);

        spin_lock_irqsave(cdev->lock, flags);

        en_val = readl_relaxed(cdev->reg);
        en_val &= ~BIT(cdev->enable_bit);
        writel_relaxed(en_val, cdev->reg);

        spin_unlock_irqrestore(cdev->lock, flags);
}

static int tcc_dclk_is_enabled(struct clk_hw *hw)
{
        struct clk_device *cdev = to_clk_device(hw);
        u32 en_val = (readl_relaxed(cdev->reg) & BIT(cdev->enable_bit));

        pr_debug("%s: %s => %d\n", __func__, cdev->name, en_val);

        return en_val ? 1 : 0;
}

static unsigned long tcc_dclk_recalc_rate(struct clk_hw *hw,
                                unsigned long parent_rate)
{
        struct clk_device *cdev = to_clk_device(hw);
        unsigned long rate = parent_rate;
        u32 val = readl(cdev->reg);

        if (val & BIT(cdev->enable_bit))
                rate /= ((val >> cdev->div_shift) & cdev->div_mask) + 1;

        pr_debug("%s: %s %lu %lu\n", __func__, cdev->name, parent_rate, rate);

        return rate;
}

static long tcc_dclk_round_rate(struct clk_hw *hw, unsigned long rate,
                                unsigned long *prate)
{
        u32 divisor = *prate / rate;

        pr_debug("%s: %p %lu %lu\n", __func__, hw, rate, *prate);

        return *prate / divisor;
}

static int tcc_dclk_set_rate(struct clk_hw *hw, unsigned long rate,
                                unsigned long parent_rate)
{
        struct clk_device *cdev = to_clk_device(hw);
        u32 divisor = parent_rate / rate;
        unsigned long flags = 0;
	u32 val;

        pr_debug("%s: %s %lu %lu\n", __func__, cdev->name, rate, parent_rate);

        if (divisor > cdev->div_mask +1 || divisor == 0) {
                pr_err("%s: invalid divisor for clock\n", __func__);
                return -EINVAL;
        }

        spin_lock_irqsave(cdev->lock, flags);
	val = readl_relaxed(cdev->reg);
	val &= ~(cdev->div_mask << cdev->div_shift);
	val |= (divisor -1) << cdev->div_shift;
        writel_relaxed(val, cdev->reg);
        spin_lock_irqsave(cdev->lock, flags);

        return 0;
}

static u8 tcc_dclk_get_parent(struct clk_hw *hw)
{
        struct clk_device *cdev = to_clk_device(hw);
        u8 src = (readl_relaxed(cdev->reg) >> cdev->src_shift) & cdev->src_mask;
        pr_debug("%s: %s %d\n", __func__, cdev->name, src);
        return src;
}

static const struct clk_ops tcc_device_clk_ops = {
        .enable = tcc_dclk_enable,
        .disable = tcc_dclk_disable,
        .is_enabled = tcc_dclk_is_enabled,
        .round_rate = tcc_dclk_round_rate,
        .set_rate = tcc_dclk_set_rate,
        .recalc_rate = tcc_dclk_recalc_rate,
        .get_parent = tcc_dclk_get_parent,
};

struct clk *tcc_clk_per(const char *name, const char **parent_names, int num_parents, void __iomem *reg,
	uint32_t enable_bit, uint32_t div_shift, uint32_t div_mask, uint32_t src_shift, uint32_t src_mask)
{
        struct clk *clk;
        struct clk_device *dev_clk;
        struct clk_init_data init;

        dev_clk = kzalloc(sizeof(*dev_clk), GFP_KERNEL);
        if (WARN_ON(!dev_clk))
                return ERR_PTR(-ENOMEM);

        dev_clk->name = name;
        dev_clk->reg = reg;
	dev_clk->lock = &tcc_ckc_lock;
	dev_clk->enable_bit = enable_bit;
	dev_clk->div_shift = div_shift;
	dev_clk->div_mask = div_mask;
	dev_clk->src_shift = src_shift;
	dev_clk->src_mask = src_mask;
        init.name = name;
        init.ops = &tcc_device_clk_ops;
        init.flags = 0;
        init.parent_names = parent_names;
        init.num_parents = num_parents;
        dev_clk->hw.init = &init;
        clk = clk_register(NULL, &dev_clk->hw);
        if (WARN_ON(IS_ERR(clk))) {
                kfree(dev_clk);
                return clk;
        }
        pr_debug("Registered clock %s with %d parents\n", name, num_parents);
	return clk;
}
