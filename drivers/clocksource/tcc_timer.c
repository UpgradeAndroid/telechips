/*
 * TCC8000 system timer setup
 *
 * (C) 2009 Hans J. Koch <hjk@linutronix.de>
 * (C) 2014 Ithamar R. Adema <ithamar@upgrade-android.com>
 *
 * Licensed under the terms of the GPL version 2.
 *
 */

#include <linux/clk.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqreturn.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

/* Timer registers */
#define TC32EN_OFFS		0x80
#define TC32LDV_OFFS		0x84
#define TC32CMP0_OFFS		0x88
#define TC32CMP1_OFFS		0x8c
#define TC32PCNT_OFFS		0x90
#define TC32MCNT_OFFS		0x94
#define TC32IRQ_OFFS		0x98

/* Bits in TC32EN */
#define TC32EN_PRESCALE_MASK	0x00ffffff
#define TC32EN_ENABLE		(1 << 24)
#define TC32EN_LOADZERO		(1 << 25)
#define TC32EN_STOPMODE		(1 << 26)
#define TC32EN_LDM0		(1 << 28)
#define TC32EN_LDM1		(1 << 29)

/* Bits in TC32IRQ */
#define TC32IRQ_MSTAT_MASK	0x0000001f
#define TC32IRQ_RSTAT_MASK	(0x1f << 8)
#define TC32IRQ_IRQEN0		(1 << 16)
#define TC32IRQ_IRQEN1		(1 << 17)
#define TC32IRQ_IRQEN2		(1 << 18)
#define TC32IRQ_IRQEN3		(1 << 19)
#define TC32IRQ_IRQEN4		(1 << 20)
#define TC32IRQ_RSYNC		(1 << 30)
#define TC32IRQ_IRQCLR		(1 << 31)


static void __iomem *timer_base;

static int tcc_set_next_event(unsigned long evt,
			      struct clock_event_device *unused)
{
	unsigned long reg = __raw_readl(timer_base + TC32MCNT_OFFS);

	__raw_writel(reg + evt, timer_base + TC32CMP0_OFFS);
	return 0;
}

static void tcc_set_mode(enum clock_event_mode mode,
				struct clock_event_device *evt)
{
	unsigned long tc32irq;

	switch (mode) {
	case CLOCK_EVT_MODE_ONESHOT:
		tc32irq = __raw_readl(timer_base + TC32IRQ_OFFS);
		tc32irq |= TC32IRQ_IRQEN0;
		__raw_writel(tc32irq, timer_base + TC32IRQ_OFFS);
		break;
	case CLOCK_EVT_MODE_SHUTDOWN:
	case CLOCK_EVT_MODE_UNUSED:
		tc32irq = __raw_readl(timer_base + TC32IRQ_OFFS);
		tc32irq &= ~TC32IRQ_IRQEN0;
		__raw_writel(tc32irq, timer_base + TC32IRQ_OFFS);
		break;
	case CLOCK_EVT_MODE_PERIODIC:
	case CLOCK_EVT_MODE_RESUME:
		break;
	}
}

static irqreturn_t tcc_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = dev_id;

	/* Acknowledge TC32 interrupt by reading TC32IRQ */
	__raw_readl(timer_base + TC32IRQ_OFFS);

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static struct clock_event_device clockevent_tcc = {
	.name		= "tcc_timer1",
	.features	= CLOCK_EVT_FEAT_ONESHOT,
	.shift		= 32,
	.set_mode	= tcc_set_mode,
	.set_next_event	= tcc_set_next_event,
	.rating		= 200,
};

static struct irqaction tcc_timer_irq = {
	.name		= "TC32_timer",
	.flags		= IRQF_DISABLED | IRQF_TIMER,
	.handler	= tcc_timer_interrupt,
	.dev_id		= &clockevent_tcc,
};

static int __init tcc_clockevent_init(struct clk *clock)
{
	unsigned int c = clk_get_rate(clock);

	clocksource_mmio_init(timer_base + TC32MCNT_OFFS, "tcc_tc32", c,
		200, 32, clocksource_mmio_readl_up);

	clockevent_tcc.mult = div_sc(c, NSEC_PER_SEC,
					clockevent_tcc.shift);
	clockevent_tcc.max_delta_ns =
			clockevent_delta2ns(0xfffffffe, &clockevent_tcc);
	clockevent_tcc.min_delta_ns =
			clockevent_delta2ns(0xff, &clockevent_tcc);

	clockevent_tcc.cpumask = cpumask_of(0);

	clockevents_register_device(&clockevent_tcc);

	return 0;
}

void __init tcc_timer_init(struct device_node *node)
{
	struct clk *clk = NULL;
	int irq;
	u32 reg;

	timer_base = of_iomap(node, 0);
	if (!timer_base)
		panic("Can't map registers");

	irq = irq_of_parse_and_map(node, 1 /* TC1 */);
	if (irq <= 0)
		panic("Can't parse IRQ");

	clk = of_clk_get(node, 2);
	if (IS_ERR(clk))
		panic("Can't get timer clock");

	clk_prepare_enable(clk); //XXX needed?
	tcc_timer_irq.irq = irq;

	/* Initialize 32-bit timer */
	reg = __raw_readl(timer_base + TC32EN_OFFS);
	reg &= ~TC32EN_ENABLE; /* Disable timer */
	__raw_writel(reg, timer_base + TC32EN_OFFS);
	/* Free running timer, counting from 0 to 0xffffffff */
	__raw_writel(0, timer_base + TC32EN_OFFS);
	__raw_writel(0, timer_base + TC32LDV_OFFS);
	reg = __raw_readl(timer_base + TC32IRQ_OFFS);
	reg |= TC32IRQ_IRQEN0; /* irq at match with CMP0 */
	__raw_writel(reg, timer_base + TC32IRQ_OFFS);

	__raw_writel(TC32EN_ENABLE, timer_base + TC32EN_OFFS);

	tcc_clockevent_init(clk);
	setup_irq(irq, &tcc_timer_irq);
}
CLOCKSOURCE_OF_DECLARE(tcc, "tcc,timers", tcc_timer_init);
