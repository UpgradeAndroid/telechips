/*
 * interrupt controller support for Telechips SoCs
 *
 * Copyright (c) 2014 Upgrade Android
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/init.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/irqdomain.h>
#include <linux/syscore_ops.h>
#include <asm/mach/irq.h>
#include <asm/exception.h>
#include "irqchip.h"

/* Controller register offsets */
#define VPIC_IEN	0x0000
#define VPIC_CLR	0x0008
#define VPIC_STS	0x0010
#define VPIC_SEL	0x0018
#define VPIC_SRC	0x0020
#define VPIC_MSTS	0x0028
#define VPIC_TIG	0x0030
#define VPIC_POL	0x0038
#define VPIC_IRQ	0x0040
#define VPIC_FIQ	0x0048
#define VPIC_MIRQ	0x0050
#define VPIC_MFIQ	0x0058
#define VPIC_MODE	0x0060
#define VPIC_SYNC	0x0068
#define VPIC_WKEN	0x0070
#define VPIC_MODEA	0x0078
#define VPIC_INTMSK	0x0100
#define VPIC_ALLMSK	0x0108	/* Only in first bank! */
#define VPIC_EI37SEL	0x0080	/* Only in first bank! */


static struct irq_domain *vpic_irqdomain;

static int
vpic_set_type(struct irq_data *data, unsigned int flow_type)
{
	struct irq_chip_generic *gc = irq_data_get_irq_chip_data(data);
	unsigned int irq = data->irq;
	u32 mode, modea, pol;

	flow_type &= IRQ_TYPE_SENSE_MASK;

	if (flow_type == IRQ_TYPE_NONE)
		return 0;

	irq_gc_lock(gc);

	mode = readl_relaxed(gc->reg_base + VPIC_MODE);
	modea = readl_relaxed(gc->reg_base + VPIC_MODEA);
	pol = readl_relaxed(gc->reg_base + VPIC_POL);

	switch(flow_type) {
		case IRQ_TYPE_EDGE_BOTH:
			mode &= ~BIT(irq);
			modea |= BIT(irq);
			break;
		case IRQ_TYPE_EDGE_RISING:
			mode &= ~BIT(irq);
			modea &= ~BIT(irq);
			pol &= ~BIT(irq);
			break;
		case IRQ_TYPE_EDGE_FALLING:
			mode &= ~BIT(irq);
			modea &= ~BIT(irq);
			pol |= BIT(irq);
			break;
		case IRQ_TYPE_LEVEL_HIGH:
			mode |= BIT(irq);
			pol &= ~BIT(irq);
			break;
		default:
			mode |= BIT(irq);
			pol |= BIT(irq);
			break;
	}

	writel_relaxed(mode, gc->reg_base + VPIC_MODE);
	writel_relaxed(modea, gc->reg_base + VPIC_MODEA);
	writel_relaxed(pol, gc->reg_base + VPIC_POL);

	irq_gc_unlock(gc);

	return 0;
}

static int
vpic_set_wake(struct irq_data *data, unsigned int on)
{
	struct irq_chip_generic *gc = irq_data_get_irq_chip_data(data);
	unsigned int irq = data->irq;
	u32 reg;

	irq_gc_lock(gc);

	reg = readl_relaxed(gc->reg_base + VPIC_WKEN);
	if (on)
		reg |= BIT(irq);
	else
		reg &= ~BIT(irq);
	writel_relaxed(reg, gc->reg_base + VPIC_WKEN);

	irq_gc_unlock(gc);

	return 0;
}

static __init void
vpic_alloc_gc(void __iomem *base, unsigned int irq_start, unsigned int num)
{
	struct irq_chip_generic *gc;
	struct irq_chip_type *ct;

	/* default all interrupts to IRQs */
	writel_relaxed(~0, base + VPIC_SEL);
	/* unmask all interrupts... */
	writel_relaxed(~0, base + VPIC_INTMSK);
	/* ... but keep them disabled */
	writel_relaxed(0, base + VPIC_IEN);
	/* disable all wakeup sources */
	writel_relaxed(0, base + VPIC_WKEN);

	gc = irq_alloc_generic_chip("VPIC", 1, irq_start, base, handle_level_irq);
	ct = gc->chip_types;

	ct->chip.irq_set_type = vpic_set_type;
	ct->chip.irq_set_wake = vpic_set_wake;
	ct->chip.irq_mask = irq_gc_mask_clr_bit;
	ct->chip.irq_unmask = irq_gc_mask_set_bit;
	ct->regs.mask = VPIC_IEN;
	ct->chip.irq_ack = irq_gc_ack_set_bit;
	ct->regs.ack = VPIC_CLR;

	irq_setup_generic_chip(gc, IRQ_MSK(num), IRQ_GC_INIT_MASK_CACHE, IRQ_NOREQUEST, 0);
}

static asmlinkage void __exception_irq_entry vpic_handle_irq(struct pt_regs *regs)
{
	void __iomem *base = vpic_irqdomain->host_data;
	u32 irqstat, irqnr = 0;

	irqstat = readl_relaxed(base + VPIC_MIRQ);
	if (!irqstat) {
		irqstat = readl_relaxed(base + VPIC_MIRQ + 4);
		irqnr = 32;
	}
	irqnr = irq_find_mapping(vpic_irqdomain, irqnr + ffs(irqstat) - 1);
	handle_IRQ(irqnr, regs);
}

static 
int __init tcc92xx_vpic_of_init(struct device_node *node, struct device_node *parent)
{
	void __iomem *regs = of_iomap(node, 0);
	if (WARN_ON(!regs))
		return -EIO;

	/* using legacy because irqchip_generic does not work with linear */
	vpic_irqdomain = irq_domain_add_legacy(node, 64, 0, 0,
				 &irq_domain_simple_ops, regs);

	vpic_alloc_gc(regs, 0, 32);
	vpic_alloc_gc(regs + 4, 32, 32);

	set_handle_irq(vpic_handle_irq);

	return 0;
}
IRQCHIP_DECLARE(tcc92xx_vpic, "tcc,tcc92xx-vpic", tcc92xx_vpic_of_init);

static
int __init tcc88xx_vpic_of_init(struct device_node *node, struct device_node *parent)
{
	u32 val = 0;
	int ret;

	ret = tcc92xx_vpic_of_init(node, parent);
	if (ret)
		return ret;

	ret = of_property_read_u32(node, "tcc,ei37sel", &val);
	if (!ret && (val & ~0xf8) != 0)
		pr_err("%s: invalid tcc,ei37sel property specified: %x\n", node->full_name, val);
	else
		writel_relaxed(val, vpic_irqdomain->host_data + VPIC_EI37SEL);

	return 0;
}
IRQCHIP_DECLARE(tcc88xx_vpic, "tcc,tcc88xx-vpic", tcc88xx_vpic_of_init);
