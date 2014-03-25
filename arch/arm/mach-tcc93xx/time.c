/*
 * linux/arch/arm/mach-tcc9300/time.c
 *
 * Author:  <linux@telechips.com>
 * Created: August 30, 2010
 *
 * Description: TCC Timers
 *
 * Copyright (C) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* 
 *  Returns elapsed usecs since last system timer interrupt
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/irq.h>	// for setup_irq()
#include <linux/mm.h>	// for PAGE_ALIGN
#ifdef CONFIG_GENERIC_TIME	/* sgbaek */
#include <linux/clk.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#endif

#include <asm/io.h>
#include <asm/leds.h>
#include <asm/mach/time.h>
#include <mach/bsp.h>

#define TCC_TIMER_FREQ (12000000L) /* 12M */ 
#define TCC_ENABLE_BIT(X) (1 << (X))

#if (TCC_TIMER_FREQ < (USEC_PER_SEC))
#   define PRESCALE_TO_MICROSEC(X) ((X) * ((USEC_PER_SEC) / (TCC_TIMER_FREQ)))
#else
#   define PRESCALE_TO_MICROSEC(X) ((X) / ((TCC_TIMER_FREQ) / (USEC_PER_SEC)))
#endif

// Global 
static volatile PTIMER pTIMER;
static volatile PPIC pPIC;
unsigned int g_org_tcc_clk = 0; /* backup cpu clock value */

#undef TICKLESS_DEBUG_TCC

#if defined(TICKLESS_DEBUG_TCC)/* debug code, sgbaek */
static unsigned int  gInt_cnt = 0;
static unsigned int  gTimer_cnt = 0;
static unsigned int  gEvent_cnt = 0;
static unsigned int  gEvent_over_cnt = 0;
extern unsigned long volatile __jiffy_data jiffies;
static unsigned long gdelta_min = 0xFFFFFF;
static unsigned long gdelta_max = 0;
#endif

#ifndef CONFIG_GENERIC_TIME     /* sgbaek */
/*
 * Returns elapsed usecs since last system timer interrupt
 */
static unsigned long tcc9300_timer_gettimeoffset(void)
{
    unsigned long ret = PRESCALE_TO_MICROSEC(pTIMER->TC32PCNT);
    if (pPIC->IRQ0 & Hw1) {
        /* Timer intrrupt occured. But the jiffies was not updated at now.  */
        /* So, it returns the time of a jiffies */
        return (USEC_PER_SEC / HZ);
    }
    return ret;
}

static irqreturn_t tcc9300_timer_interrupt(int irq, void *dev_id)
{
    timer_tick();

    BITSET(pPIC->CLR0, TCC_ENABLE_BIT(irq));
    if(pTIMER->TC32IRQ & Hw31) 
        BITSET(pTIMER->TC32IRQ, Hw31);

    return IRQ_HANDLED;
}

static struct irqaction tcc9300_timer_irq = {
    .name       = "TC0_timer",
    .flags      = IRQF_DISABLED | IRQF_TIMER,
    .handler    = tcc9300_timer_interrupt,
};

/*
 * Scheduler clock - returns current time in nanosec units.
 */
unsigned long long sched_clock(void)
{
    return ((unsigned long long)jiffies) * (1000000000llu / HZ);
}


/* 
 * Timer Initialization 
 */
static void __init tcc9300_timer_init(void)
{
    unsigned int cpu_clk = 0;
    unsigned int bus_clk = 0;

    //init_pwm_list();

	tca_ckc_init();
#if 0
	tca_ckc_init();

    g_org_tcc_clk = tca_ckc_getclkctrl0();

    cpu_clk = (unsigned int)tca_ckc_getcpu();
    bus_clk = (unsigned int)tca_ckc_getbus();
#endif

    pTIMER  = (volatile PTIMER)tcc_p2v(HwTMR_BASE);
   	pPIC    = (volatile PPIC)tcc_p2v(HwPRIOINTRCTR_BASE);

    printk(" ### CORE CLOCK (%u Hz), BUS CLOCK (%u Hz) ###\n", cpu_clk * 100, bus_clk * 100);

    BITCLR(pTIMER->TC32EN, Hw24);
    pTIMER->TC32EN = (TCC_TIMER_FREQ / HZ) - 1;
    pTIMER->TC32LDV = 0;
    BITSET(pTIMER->TC32IRQ, Hw19);
    BITSET(pTIMER->TC32EN, Hw24);

    BITSET(pPIC->SEL0, TCC_ENABLE_BIT(INT_TC32));
    BITSET(pPIC->IEN0, TCC_ENABLE_BIT(INT_TC32));
    BITSET(pPIC->INTMSK0, TCC_ENABLE_BIT(INT_TC32));
    BITSET(pPIC->MODEA0, TCC_ENABLE_BIT(INT_TC32));
    //BITCLR(pPIC->CLR0, TCC_ENABLE_BIT(INT_TC32));

    setup_irq(INT_TC32, &tcc9300_timer_irq);
}

struct sys_timer tcc9300_timer = {
    .init       = tcc9300_timer_init,
    .offset     = tcc9300_timer_gettimeoffset, 
};

#else   /* CONFIG_GENERIC_TIME, second version from sa1100 */
/*************************************************************************************************
 * Tickless Part
 *************************************************************************************************/
#define MIN_OSCR_DELTA 2
static irqreturn_t tcc9300_ost0_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *c = dev_id;

	/* Disarm the compare/match, signal the event. */
//	OIER &= ~OIER_E0;
//	OSSR = OSSR_M0;

//    BITCLR(pPIC->IEN0, TCC_ENABLE_BIT(irq));    /* Interrupt Disable */
    BITCLR(pTIMER->TC32IRQ, Hw16);              /* Disable interrupt when the counter value matched with CMP0 */
    BITSET(pPIC->CLR0, TCC_ENABLE_BIT(irq));    /* Interrupt clear */
    if(pTIMER->TC32IRQ & Hw31)                  /* IRQ clear */
        BITSET(pTIMER->TC32IRQ, Hw31);
//    BITCLR(pTIMER->TC32EN, Hw24);               /* Timer disable */
	c->event_handler(c);

//printk("%s(): %d \n", __func__, irq);

#if defined(TICKLESS_DEBUG_TCC)/* debug code, sgbaek */
    gInt_cnt++;
#endif

	return IRQ_HANDLED;
}

static int
tcc9300_osmr0_set_next_event(unsigned long delta, struct clock_event_device *c)
{
	unsigned long flags, next, oscr;

#if defined(TICKLESS_DEBUG_TCC) /* debug code, sgbaek */
    static unsigned long jiffies_old;
#endif

	raw_local_irq_save(flags);

//	OIER |= OIER_E0;
//	next = OSCR + delta;
//	OSMR0 = next;
//	oscr = OSCR;
    BITSET(pTIMER->TC32IRQ, Hw16);  /* Enable interrupt at the end of count */
    next = pTIMER->TC32MCNT + delta;
    pTIMER->TC32CMP0 = next;        /* Load counter value */
    oscr = pTIMER->TC32MCNT;

	raw_local_irq_restore(flags);

#if defined(TICKLESS_DEBUG_TCC) /* debug code, sgbaek */
	if (delta > 0xEA00)     /* 10ms == 0xEA60 */
		gEvent_over_cnt++;

    if (gdelta_min > delta)
        gdelta_min = delta;
    if (gdelta_max < delta)
        gdelta_max = delta;
    
    gEvent_cnt++;
    if (gInt_cnt >= 5000) {
        printk("\nMin Delta: %x \t Max Delta: %x\n", gdelta_min, gdelta_max);
        printk("%s(%d) .. jiffies[%d, %d], int[%4d] event[%4d] delta[%08x] next[%08x]oscr[%08x]\n", 
						__func__, 
						gEvent_over_cnt, 
						jiffies, 
						jiffies - jiffies_old, 
						gInt_cnt, 
						gEvent_cnt,
						delta, 
						next, 
						oscr);
        jiffies_old = jiffies;
        gEvent_cnt = 0;
        gInt_cnt = 0;
		gEvent_over_cnt = 0;
        gdelta_min = 0xffffff;
        gdelta_max = 0;
    }
#endif

//    return delta <= MIN_OSCR_DELTA ? -ETIME : 0;
	return (signed)(next - oscr) <= MIN_OSCR_DELTA ? -ETIME : 0;
}


static void
tcc9300_osmr0_set_mode(enum clock_event_mode mode, struct clock_event_device *c)
{
	unsigned long flags;

#if defined(TICKLESS_DEBUG_TCC)/* debug code, sgbaek */
    printk("%s: mode %s... %d\n", __func__
                                , mode == CLOCK_EVT_MODE_ONESHOT  ? "ONESHOT"   : 
                                  mode == CLOCK_EVT_MODE_UNUSED   ? "UNUSED"    :
                                  mode == CLOCK_EVT_MODE_SHUTDOWN ? "SHUTDOWN"  :
                                  mode == CLOCK_EVT_MODE_RESUME   ? "RESUME"    :
                                  mode == CLOCK_EVT_MODE_PERIODIC ? "PERIODIC"  : "non"
                                , gTimer_cnt++);
#endif

	switch (mode) {
	case CLOCK_EVT_MODE_ONESHOT:
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		raw_local_irq_save(flags);
//		OIER &= ~OIER_E0;
//		OSSR = OSSR_M0;
        BITCLR(pTIMER->TC32IRQ, Hw16);                  /* Disable interrupt when the counter value matched with CMP0 */
        BITSET(pPIC->CLR0, TCC_ENABLE_BIT(INT_TC32));    /* PIC Interrupt clear */
        if(pTIMER->TC32IRQ & Hw31)                      /* IRQ clear */
            BITSET(pTIMER->TC32IRQ, Hw31);
		raw_local_irq_restore(flags);
		break;

    case CLOCK_EVT_MODE_RESUME:
	case CLOCK_EVT_MODE_PERIODIC:
        break;
	}
}

static struct clock_event_device ckevt_tcc9300_osmr0 = {
	.name		= "osmr0",
	.features	= CLOCK_EVT_FEAT_ONESHOT,
	.shift		= 32,
	.rating		= 250,
	.set_next_event	= tcc9300_osmr0_set_next_event,
	.set_mode	= tcc9300_osmr0_set_mode,
};

static cycle_t tcc9300_read_oscr(void)
{
//	return OSCR;
    return pTIMER->TC32MCNT;
}

static struct clocksource cksrc_tcc9300_oscr = {
	.name		= "oscr",
	.rating		= 300,
	.read		= tcc9300_read_oscr,
	.mask		= CLOCKSOURCE_MASK(32),
	.shift		= 20,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static struct irqaction tcc9300_timer_irq = {
	.name		= "ost0",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= tcc9300_ost0_interrupt,
	.dev_id		= &ckevt_tcc9300_osmr0,
};

static void __init tcc9300_timer_init(void)
{
	unsigned long	rate;
    unsigned int cpu_clk = 0;
    unsigned int bus_clk = 0;

#if defined(CONFIG_SATA_TCC)
    init_pwm_list();
#endif
	tca_ckc_init();

#if 0
	tca_ckc_init();
    g_org_tcc_clk = tca_ckc_getclkctrl0();

    cpu_clk = (unsigned int)tca_ckc_getcpu();
    bus_clk = (unsigned int)tca_ckc_getbus();
#endif

    printk(" ### CORE CLOCK (%u Hz), BUS CLOCK (%u Hz) ###\n", cpu_clk * 100, bus_clk * 100);

    rate = TCC_TIMER_FREQ;

    pTIMER  = (volatile PTIMER)tcc_p2v(HwTMR_BASE);
   	pPIC    = (volatile PPIC)tcc_p2v(HwPIC_BASE);

//	OIER = 0;		/* disable any timer interrupts */
//	OSSR = 0xf;		/* clear status on all timers */

    pTIMER->TC32EN = 1;                 /* Timer disable, Prescale is one */
    BITSET(pTIMER->TC32EN, Hw24);       /* Timer Enable */
    if(pTIMER->TC32IRQ & Hw31)          /* IRQ clear */
        BITSET(pTIMER->TC32IRQ, Hw31);

	ckevt_tcc9300_osmr0.mult =
		div_sc(6000000, NSEC_PER_SEC   , ckevt_tcc9300_osmr0.shift);
	ckevt_tcc9300_osmr0.max_delta_ns =
		clockevent_delta2ns(0x7fffffff  , &ckevt_tcc9300_osmr0);
	ckevt_tcc9300_osmr0.min_delta_ns =
		clockevent_delta2ns(4         , &ckevt_tcc9300_osmr0) + 1;
	ckevt_tcc9300_osmr0.cpumask = cpumask_of(0);

	cksrc_tcc9300_oscr.mult =
		clocksource_khz2mult(6000, cksrc_tcc9300_oscr.shift);


    BITSET(pPIC->SEL0, TCC_ENABLE_BIT(INT_TC32));
    BITSET(pPIC->IEN0, TCC_ENABLE_BIT(INT_TC32));
    BITSET(pPIC->INTMSK0, TCC_ENABLE_BIT(INT_TC32));
    BITSET(pPIC->MODEA0, TCC_ENABLE_BIT(INT_TC32));
    //BITCLR(pPIC->CLR0, TCC_ENABLE_BIT(INT_TC32));

	setup_irq(INT_TC32, &tcc9300_timer_irq);

    /* start clocksource timer */
//    BITSET(pTIMER->TCFG5, Hw8);     /* TCNT5 is cleared to zero */
//    BITSET(pTIMER->TCFG5, Hw0);     /* Timer Enable */

	clocksource_register(&cksrc_tcc9300_oscr);
	clockevents_register_device(&ckevt_tcc9300_osmr0);
}

struct sys_timer tcc9300_timer = {
	.init		= tcc9300_timer_init,
};

#endif  /* #ifndef CONFIG_GENERIC_TIME  */
