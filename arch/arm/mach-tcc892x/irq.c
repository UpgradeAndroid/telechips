/*
 * linux/arch/arm/mach-tcc892x/irq.c
 *
 * Author:  <linux@telechips.com>
 * Description: Interrupt handler for Telechips TCC892x chipset
 *
 * Copyright (C) 2011 Telechips, Inc.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>

#include <linux/agpgart.h>
#include <linux/types.h>

#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <asm/io.h>

#include <mach/bsp.h>

// Global
static volatile PPIC pPIC;
static volatile PVIC pVIC;
static volatile PGPSBPORTCFG pPGPSBPORTCFG;
static volatile PUARTPORTCFG pUARTPORTCFG;
static volatile PGDMACTRL pPGDMACTRL0, pPGDMACTRL1, pPGDMACTRL2, pPGDMACTRL3;
static volatile PTIMER pTIMER;

/******************************************
 * Disable IRQ
 *
 * If mask_ack exist, this is not called.
 *****************************************/
static void tcc8920_mask_irq(unsigned int irq)
{
	pPIC->INTMSK.nREG &= ~(1<<irq);
}

static void tcc8920_mask_irq_uart(unsigned int irq)
{
	if (irq != INT_UART) {
		pPIC->INTMSK.bREG.UART = 0;
	}
}

static void tcc8920_mask_irq_gpsb(unsigned int irq)
{
    if (irq != INT_GPSB) {
        pPIC->INTMSK.bREG.GPSB = 0;
    }
}

static void tcc8920_mask_irq_dma(unsigned int irq)
{
    if (irq != INT_GDMA) {
        pPIC->INTMSK.bREG.GDMA = 0;
    }
}

static void tcc8920_mask_irq_tc0(unsigned int irq)
{
    if (irq != INT_TC0) {
        pPIC->INTMSK.bREG.TC0 = 0;
    }
}

/******************************************
 * Enable IRQ
 *****************************************/
static void tcc8920_irq_enable(unsigned int irq)
{
	pPIC->CLR.nREG |= (1 << irq);
	pPIC->IEN.nREG |= (1 << irq);
	pPIC->INTMSK.nREG |= (1 << irq);
}

static void tcc8920_unmask_irq(unsigned int irq)
{
	pPIC->INTMSK.nREG |= (1 << irq);
	pPIC->CLR.nREG |= (1 << irq);
}
static void tcc8920_unmask_irq_uart(unsigned int irq)
{
    if (irq != INT_UART) {
		pPIC->INTMSK.bREG.UART = 1;
    }
}
static void tcc8920_unmask_irq_gpsb(unsigned int irq)
{
    if (irq != INT_GPSB) {
        pPIC->INTMSK.bREG.GPSB = 1;
    }
}
static void tcc8920_unmask_irq_dma(unsigned int irq)
{
    if (irq != INT_GDMA) {
        pPIC->INTMSK.bREG.GDMA = 1;
    }
}

static void tcc8920_unmask_irq_tc0(unsigned int irq)
{
    if (irq != INT_TC0) {
        pPIC->INTMSK.bREG.TC0 = 1;
    }
}

/******************************************
 * Ack IRQ (Disable IRQ)
 *****************************************/

static void tcc8920_irq_disable(unsigned int irq)
{
	pPIC->IEN.nREG &= ~(1 << irq);
	pPIC->INTMSK.nREG &= ~(1 << irq);
}


static void tcc8920_mask_ack_irq(unsigned int irq)
{
	pPIC->INTMSK.nREG &= ~(1 << irq);
}

static void tcc8920_mask_ack_irq_uart(unsigned int irq)
{
    if (irq != INT_UART) {
        pPIC->INTMSK.bREG.UART = 0;
    }
}

static void tcc8920_mask_ack_irq_gpsb(unsigned int irq)
{
    if (irq != INT_GPSB) {
        pPIC->INTMSK.bREG.GPSB = 0;
    }
}

static void tcc8920_mask_ack_irq_dma(unsigned int irq)
{
    if (irq != INT_GDMA) {
        pPIC->INTMSK.bREG.GDMA = 0;
    }
}

static void tcc8920_mask_ack_irq_tc0(unsigned int irq)
{
    if (irq != INT_TC0) {
        pPIC->INTMSK.bREG.TC0 = 0;
    }
}

/******************************************
 * wake IRQ
 *****************************************/
static int tcc8920_wake_irq(unsigned int irq, unsigned int enable)
{
    return 0;
}

static int tcc8920_wake_irq_uart(unsigned int irq, unsigned int enable)
{
    return 0;
}

static int tcc8920_wake_irq_gpsb(unsigned int irq, unsigned int enable)
{
    return 0;
}

static int tcc8920_wake_irq_dma(unsigned int irq, unsigned int enable)
{
    return 0;
}

static int tcc8920_wake_irq_tc0(unsigned int irq, unsigned int enable)
{
    return 0;
}

static void tcc8920_irq_dummy(unsigned int irq)
{
}


/******************************************
 * set type IRQ
 *****************************************/
static int tcc8920_irq_set_type(unsigned int irq, unsigned int type)
{
    type &= IRQ_TYPE_SENSE_MASK;

    if(type == IRQ_TYPE_NONE)
        return 0;

    /* Edge trigger mode */
    if(type == IRQ_TYPE_EDGE_BOTH) {
		pPIC->MODE.nREG &= ~(1 << irq);   // trigger
		pPIC->MODEA.nREG |= (1 << irq);   // both
    }
    else if(type == IRQ_TYPE_EDGE_RISING) {
		pPIC->MODE.nREG &= ~(1 << irq);
		pPIC->MODEA.nREG &= ~(1 << irq);
		pPIC->POL.nREG &= ~(1 << irq);
    }
    else if(type == IRQ_TYPE_EDGE_FALLING) {
		pPIC->MODE.nREG &= ~(1 << irq);
		pPIC->MODEA.nREG &= ~(1 << irq);
		pPIC->POL.nREG |= (1 << irq);
    }
    else if(type == IRQ_TYPE_LEVEL_HIGH) {  /* Edge trigger mode */
 		pPIC->MODE.nREG |= (1 << irq);   // level
 		pPIC->POL.nREG &= ~(1 << irq);
    }
    else {  /* Edge trigger mode */
		pPIC->MODE.nREG |= (1 << irq);   // level
		pPIC->POL.nREG |= (1 << irq);
    }

    return 0;
}


static void tcc8920_irq_uart_handler(unsigned irq, struct irq_desc *desc)
{
	if (pUARTPORTCFG->ISTS.bREG.U0)
		irq = INT_UART0;
	else if (pUARTPORTCFG->ISTS.bREG.U1)
		irq = INT_UART1;
	else if (pUARTPORTCFG->ISTS.bREG.U2)
		irq = INT_UART2;
	else if (pUARTPORTCFG->ISTS.bREG.U3)
		irq = INT_UART3;
	else if (pUARTPORTCFG->ISTS.bREG.U4)
		irq = INT_UART4;
	else if (pUARTPORTCFG->ISTS.bREG.U5)
		irq = INT_UART5;
	else if (pUARTPORTCFG->ISTS.bREG.U6)
		irq = INT_UART6;
	else if (pUARTPORTCFG->ISTS.bREG.U7)
		irq = INT_UART7;
	else {
		pPIC->CLR.bREG.UART = 1;
		goto out;
	}

	desc = irq_desc + irq;
	desc_handle_irq(irq, desc);
out:
	return;
}

static void tcc8920_irq_gpsb_handler(unsigned irq, struct irq_desc *desc)
{
	if (pPGPSBPORTCFG->CIRQST.bREG.ISTD0)
		irq = INT_GPSB0_DMA;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTD1)
		irq = INT_GPSB1_DMA;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTD2)
		irq = INT_GPSB2_DMA;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTC0)
		irq = INT_GPSB0_CORE;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTC1)
		irq = INT_GPSB1_CORE;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTC2)
		irq = INT_GPSB2_CORE;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTC3)
		irq = INT_GPSB3_CORE;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTC4)
		irq = INT_GPSB4_CORE;
	else if (pPGPSBPORTCFG->CIRQST.bREG.ISTC5)
		irq = INT_GPSB5_CORE;
	else {
		pPIC->CLR.bREG.GPSB = 1;
		goto out;
	}

	desc = irq_desc + irq;
	desc_handle_irq(irq, desc);
out:
	return;
}

static void tcc8920_irq_dma_handler(unsigned irq, struct irq_desc *desc)
{
	if (pPGDMACTRL0->CHCONFIG.bREG.MIS0)
		irq = INT_DMA0_CH0;
	else if (pPGDMACTRL0->CHCONFIG.bREG.MIS1)
		irq = INT_DMA0_CH1;
	else if (pPGDMACTRL0->CHCONFIG.bREG.MIS2)
		irq = INT_DMA0_CH2;
	else if (pPGDMACTRL1->CHCONFIG.bREG.MIS0)
		irq = INT_DMA1_CH0;
	else if (pPGDMACTRL1->CHCONFIG.bREG.MIS1)
		irq = INT_DMA1_CH1;
	else if (pPGDMACTRL1->CHCONFIG.bREG.MIS2)
		irq = INT_DMA1_CH2;
	else if (pPGDMACTRL2->CHCONFIG.bREG.MIS0)
		irq = INT_DMA2_CH0;
	else if (pPGDMACTRL2->CHCONFIG.bREG.MIS1)
		irq = INT_DMA2_CH1;
	else if (pPGDMACTRL2->CHCONFIG.bREG.MIS2)
		irq = INT_DMA2_CH2;
	else {
		pPIC->CLR.bREG.GDMA = 1;
		goto out;
	}

	desc = irq_desc + irq;
	desc_handle_irq(irq, desc);
out:
	return;
}

static void tcc8920_irq_tc_handler(unsigned irq, struct irq_desc *desc)
{
	if (pTIMER->TIREQ.bREG.TI0)
		irq = INT_TC_TI0;
	else if (pTIMER->TIREQ.bREG.TI1)
		irq = INT_TC_TI1;
	else if (pTIMER->TIREQ.bREG.TI2)
		irq = INT_TC_TI2;
	else if (pTIMER->TIREQ.bREG.TI3)
		irq = INT_TC_TI3;
	else if (pTIMER->TIREQ.bREG.TI4)
		irq = INT_TC_TI4;
	else if (pTIMER->TIREQ.bREG.TI5)
		irq = INT_TC_TI5;
	else {
		pPIC->CLR.bREG.TC0 = 1;
		goto out;
	}

	desc = irq_desc + irq;
	desc_handle_irq(irq, desc);
out:
	return;
}


static struct irq_chip tcc8920_irq_chip = {
    .name       = "IRQ",
    .enable     = tcc8920_irq_enable,
    .disable    = tcc8920_irq_disable,
    .ack        = tcc8920_mask_ack_irq,
    .mask_ack   = tcc8920_mask_ack_irq,
    .mask       = tcc8920_mask_irq,
    .unmask     = tcc8920_unmask_irq,
    .set_wake   = tcc8920_wake_irq,
    .set_type   = tcc8920_irq_set_type,
};

static struct irq_chip tcc8920_irq_uart_chip = {
    .name       = "IRQ_UART",
    .enable     = tcc8920_irq_dummy,
    .disable    = tcc8920_irq_dummy,
    .ack        = tcc8920_mask_ack_irq_uart,
    .mask_ack   = tcc8920_mask_ack_irq_uart,
    .mask       = tcc8920_mask_irq_uart,
    .unmask     = tcc8920_unmask_irq_uart,
    .set_wake   = tcc8920_wake_irq_uart,
};

static struct irq_chip tcc8920_irq_gpsb_chip = {
    .name       = "IRQ_GPSB",
    .enable     = tcc8920_irq_dummy,
    .disable    = tcc8920_irq_dummy,
    .ack        = tcc8920_mask_ack_irq_gpsb,
    .mask_ack   = tcc8920_mask_ack_irq_gpsb,
    .mask       = tcc8920_mask_irq_gpsb,
    .unmask     = tcc8920_unmask_irq_gpsb,
    .set_wake   = tcc8920_wake_irq_gpsb,
};

static struct irq_chip tcc8920_irq_dma_chip = {
    .name       = "IRQ_DMA",
    .enable     = tcc8920_irq_dummy,
    .disable    = tcc8920_irq_dummy,
    .ack        = tcc8920_mask_ack_irq_dma,
    .mask_ack   = tcc8920_mask_ack_irq_dma,
    .mask       = tcc8920_mask_irq_dma,
    .unmask     = tcc8920_unmask_irq_dma,
    .set_wake   = tcc8920_wake_irq_dma,
};

static struct irq_chip tcc8920_irq_tc_chip = {
    .name       = "IRQ_TC",
    .enable     = tcc8920_irq_dummy,
    .disable    = tcc8920_irq_dummy,
    .ack        = tcc8920_mask_ack_irq_tc0,
    .mask_ack   = tcc8920_mask_ack_irq_tc0,
    .mask       = tcc8920_mask_irq_tc0,
    .unmask     = tcc8920_unmask_irq_tc0,
    .set_wake   = tcc8920_wake_irq_tc0,
};

void __init tcc_init_irq(void)
{
	int irqno;

	printk(KERN_DEBUG "%s\n", __func__);

	//reset interrupt
	pPIC = (volatile PPIC)tcc_p2v(HwPIC_BASE);
	pVIC = (volatile PVIC)tcc_p2v(HwVIC_BASE);
	pPGPSBPORTCFG = (volatile PGPSBPORTCFG)tcc_p2v(HwGPSB_PORTCFG_BASE);
	pUARTPORTCFG = (volatile PUARTPORTCFG)tcc_p2v(HwUART_PORTCFG_BASE);
	pPGDMACTRL0 = (volatile PGDMACTRL)tcc_p2v(HwGDMA0_BASE);
	pPGDMACTRL1 = (volatile PGDMACTRL)tcc_p2v(HwGDMA1_BASE);
	pPGDMACTRL2 = (volatile PGDMACTRL)tcc_p2v(HwGDMA2_BASE);
	pTIMER = (volatile PTIMER)tcc_p2v(HwTMR_BASE);


	/* ADD IOREMAP */

	//clear IEN Field
	pPIC->IEN.nREG = (unsigned long long)0x0000000000000000LL; // All Interrupt Disable

	//clear SEL Field
	pPIC->SEL.nREG = (unsigned long long)0xFFFFFFFFFFFFFFFFLL; //using IRQ

	//clear TIG Field
	pPIC->TIG.nREG = (unsigned long long)0x0000000000000000LL; //Test Interrupt Disable

	//clear POL Field
	pPIC->POL.nREG = (unsigned long long)0x0000000000000000LL; //Default ACTIVE Low

	//clear MODE Field
	pPIC->MODE.nREG = (unsigned long long)0xFFFFFFFFFFFFFFFFLL; //Trigger Mode - Level Trigger Mode

	//clear SYNC Field
	pPIC->SYNC.nREG = (unsigned long long)0xFFFFFFFFFFFFFFFFLL; //SYNC Enable

	//clear WKEN Field
	pPIC->WKEN.nREG = (unsigned long long)0x0000000000000000LL; //Wakeup all disable

	//celar MODEA Field
	pPIC->MODEA.nREG = (unsigned long long)0x0000000000000000LL; //both edge - all disable

	//clear INTMSK Field
	pPIC->INTMSK.nREG = (unsigned long long)0x0000000000000000LL; //not using INTMSK

	//clear ALLMSK Field
	pPIC->ALLMSK.bREG.IRQ = 1; //using only IRQ
	pPIC->ALLMSK.bREG.FIQ = 0;

	/* Install the interrupt handlers */
	for(irqno = INT_TC0; irqno <= INT_NUM; irqno++)
	{
		if (irqno == INT_UART) {
			set_irq_chip(INT_UART, &tcc8920_irq_uart_chip);
			set_irq_chained_handler(INT_UART, tcc8920_irq_uart_handler);
		} else if (irqno == INT_GPSB) {
			set_irq_chip(INT_GPSB, &tcc8920_irq_gpsb_chip);
			set_irq_chained_handler(INT_GPSB, tcc8920_irq_gpsb_handler);
		} else if (irqno == INT_GDMA) {
			set_irq_chip(INT_GDMA, &tcc8920_irq_dma_chip);
			set_irq_chained_handler(INT_GDMA, tcc8920_irq_dma_handler);
		} else if (irqno == INT_TC0) {
			set_irq_chip(INT_TC0, &tcc8920_irq_tc_chip);
			set_irq_chained_handler(INT_TC0, tcc8920_irq_tc_handler);
		} else {
			set_irq_chip(irqno, &tcc8920_irq_chip);
			set_irq_handler(irqno, handle_level_irq);
			set_irq_flags(irqno, IRQF_VALID);
		}
	}

	/* Install the interrupt UART Group handlers */
	for (irqno = INT_UT_BASE; irqno <= INT_UART_NUM; irqno++) {
		set_irq_chip(irqno, &tcc8920_irq_uart_chip);
		set_irq_handler(irqno, handle_level_irq);
		set_irq_flags(irqno, IRQF_VALID);
	}

	/* Install the interrupt GPSB Group handlers */
	for (irqno = INT_GPSB_BASE; irqno <= INT_GPSB_NUM; irqno++) {
		set_irq_chip(irqno, &tcc8920_irq_gpsb_chip);
		set_irq_handler(irqno, handle_level_irq);
		set_irq_flags(irqno, IRQF_VALID);
	}

	/* Install the interrupt DMA Group handlers */
	for (irqno = INT_DMA_BASE; irqno <= INT_DMA_NUM; irqno++) {
		set_irq_chip(irqno, &tcc8920_irq_dma_chip);
		set_irq_handler(irqno, handle_level_irq);
		set_irq_flags(irqno, IRQF_VALID);
	}

	/* Install the interrupt TC0 Group handlers */
	for (irqno = INT_TC_BASE; irqno <= INT_TC_NUM; irqno++) {
		set_irq_chip(irqno, &tcc8920_irq_tc_chip);
		set_irq_handler(irqno, handle_level_irq);
		set_irq_flags(irqno, IRQF_VALID);
	}

    /* IEN SET */
	pPIC->IEN.bREG.UART = 1;
	pPIC->INTMSK.bREG.UART = 1;
	pPIC->IEN.bREG.GPSB = 1;
	pPIC->INTMSK.bREG.GPSB = 1;
	pPIC->IEN.bREG.GDMA = 1;
	pPIC->INTMSK.bREG.GDMA = 1;
	pPIC->IEN.bREG.TC0 = 1;
	pPIC->INTMSK.bREG.TC0 = 1;
	}

/* end of file */
