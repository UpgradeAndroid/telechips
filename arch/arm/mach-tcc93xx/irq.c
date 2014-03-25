/*
 * linux/arch/arm/mach-tcc93xx/irq.c
 *
 * Author:  <linux@telechips.com>
 * Created: August, 2010
 * Description: Interrupt handler for Telechips TCC9300 chipset
 *
 * Copyright (C) Telechips, Inc.
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
static volatile PUARTPORTMUX pUARTPORTMUX;
static volatile PGDMACTRL pPGDMACTRL0, pPGDMACTRL1, pPGDMACTRL2, pPGDMACTRL3;
static volatile PTIMER pTIMER;
#define SD0_SLOT0_INT 0xB00200FC
#define SD0_SLOT1_INT 0xB00201FC
#define SD1_SLOT2_INT 0xB00202FC
#define SD1_SLOT3_INT 0xB00203FC


/******************************************
 * Disable IRQ
 *
 * If mask_ack exist, this is not called.
 *****************************************/
static void tcc9300_mask_irq(unsigned int irq)
{
    if (irq < 32) {
        BITCLR(pPIC->INTMSK0,   (1 << irq));
    } else {
        BITCLR(pPIC->INTMSK1,   (1 << (irq - 32)));
    }
}

static void tcc9300_mask_irq_uart(unsigned int irq)
{
    if (irq != INT_UART) {
        BITCLR(pPIC->INTMSK1, Hw15);
    }
}

static void tcc9300_mask_irq_gpsb(unsigned int irq)
{
    if (irq != INT_GPSB) {
        BITCLR(pPIC->INTMSK1, Hw4);
    }
}

static void tcc9300_mask_irq_dma(unsigned int irq)
{
    if (irq != INT_GDMA0) {
        BITCLR(pPIC->INTMSK0, Hw29);
    }
}

static void tcc9300_mask_irq_tc0(unsigned int irq)
{
    if (irq != INT_TC0) {
        BITCLR(pPIC->INTMSK0, Hw0);
    }
}

/******************************************
 * Enable IRQ
 *****************************************/
static void tcc9300_irq_enable(unsigned int irq)
{
    if (irq < 32) {
        BITSET(pPIC->CLR0,      (1 << irq));
        BITSET(pPIC->IEN0,      (1 << irq));
        BITSET(pPIC->INTMSK0,   (1 << irq));
    } else {
        BITSET(pPIC->CLR1,      (1 << (irq - 32)));
        BITSET(pPIC->IEN1,      (1 << (irq - 32)));
        BITSET(pPIC->INTMSK1,   (1 << (irq - 32)));
    }
}

static void tcc9300_unmask_irq(unsigned int irq)
{
    if (irq < 32) {
        BITSET(pPIC->INTMSK0,   (1 << irq));
        BITSET(pPIC->CLR0,      (1 << irq));
    } else {
        BITSET(pPIC->INTMSK1,   (1 << (irq - 32)));
        BITSET(pPIC->CLR1,      (1 << (irq - 32)));
    }
}
static void tcc9300_unmask_irq_uart(unsigned int irq)
{
    if (irq != INT_UART) {
        BITSET(pPIC->INTMSK1, Hw15);
    }
}
static void tcc9300_unmask_irq_gpsb(unsigned int irq)
{
    if (irq != INT_GPSB) {
        BITSET(pPIC->INTMSK1, Hw4);
    }
}
static void tcc9300_unmask_irq_dma(unsigned int irq)
{
    if (irq != INT_GDMA0) {
        BITSET(pPIC->INTMSK0, Hw29);
    }
}

static void tcc9300_unmask_irq_tc0(unsigned int irq)
{
    if (irq != INT_TC0) {
        BITSET(pPIC->INTMSK0, Hw0);
    }
}

/******************************************
 * Ack IRQ (Disable IRQ)
 *****************************************/

static void tcc9300_irq_disable(unsigned int irq)
{
    if (irq < 32){
        BITCLR(pPIC->IEN0,      (1 << irq));
        BITCLR(pPIC->INTMSK0,   (1 << irq));
    } else {
        BITCLR(pPIC->IEN1,      (1 << (irq - 32)));
        BITCLR(pPIC->INTMSK1,   (1 << (irq - 32)));
    }
}


static void tcc9300_mask_ack_irq(unsigned int irq)
{
    if (irq < 32){
        BITCLR(pPIC->INTMSK0,   (1 << irq));
    } else {
        BITCLR(pPIC->INTMSK1,   (1 << (irq - 32)));
    }
}

static void tcc9300_mask_ack_irq_uart(unsigned int irq)
{
    if (irq != INT_UART) {
        BITCLR(pPIC->INTMSK1, Hw15);
    }
}

static void tcc9300_mask_ack_irq_gpsb(unsigned int irq)
{
    if (irq != INT_GPSB) {
        BITCLR(pPIC->INTMSK1, Hw4);
    }
}

static void tcc9300_mask_ack_irq_dma(unsigned int irq)
{
    if (irq != INT_GDMA0) {
        BITCLR(pPIC->INTMSK0, Hw29);
    }
}

static void tcc9300_mask_ack_irq_tc0(unsigned int irq)
{
    if (irq != INT_TC0) {
        BITCLR(pPIC->INTMSK0, Hw0);
    }
}

/******************************************
 * wake IRQ
 *****************************************/
static int tcc9300_wake_irq(unsigned int irq, unsigned int enable)
{
    return 0;
}

static int tcc9300_wake_irq_uart(unsigned int irq, unsigned int enable)
{
    return 0;
}

static int tcc9300_wake_irq_gpsb(unsigned int irq, unsigned int enable)
{
    return 0;
}

static int tcc9300_wake_irq_dma(unsigned int irq, unsigned int enable)
{
    return 0;
}

static int tcc9300_wake_irq_tc0(unsigned int irq, unsigned int enable)
{
    return 0;
}

static int tcc9300_wake_irq_sd0(unsigned int irq, unsigned int enable)
{
    return 0;
}
static int tcc9300_wake_irq_sd1(unsigned int irq, unsigned int enable)
{
    return 0;
}

static void tcc9300_irq_dummy(unsigned int irq)
{
}

static void tcc9300_irq_uart_handler(unsigned irq, struct irq_desc *desc)
{
    if (pUARTPORTMUX->CHST & Hw0) {
        irq = INT_UART0;
    } else if (pUARTPORTMUX->CHST & Hw1) {
        irq = INT_UART1;
    } else if (pUARTPORTMUX->CHST & Hw2) {
        irq = INT_UART2;
    } else if (pUARTPORTMUX->CHST & Hw3) {
        irq = INT_UART3;
    } else if (pUARTPORTMUX->CHST & Hw4) {
        irq = INT_UART4;
    } else if (pUARTPORTMUX->CHST & Hw5) {
        irq = INT_UART5;
    } else {
	    //BITSET(pPIC->INTMSK1 , Hw15); // using INTMSK
        BITSET(pPIC->CLR1, Hw15);
        goto out;
    }

    desc = irq_desc + irq;
    desc_handle_irq(irq, desc);
out:
    return;
}

static void tcc9300_irq_gpsb_handler(unsigned irq, struct irq_desc *desc)
{
    if (pPGPSBPORTCFG->CIRQST & Hw3) {
        irq = INT_GPSB1_DMA;
    } else if (pPGPSBPORTCFG->CIRQST & Hw1) {
        irq = INT_GPSB0_DMA;
    } else if (pPGPSBPORTCFG->CIRQST & Hw5) {
        irq = INT_GPSB2_DMA;
    } else if (pPGPSBPORTCFG->CIRQST & Hw0) {
        irq = INT_GPSB0_CORE;
    } else if (pPGPSBPORTCFG->CIRQST & Hw2) {
        irq = INT_GPSB1_CORE;
    } else if (pPGPSBPORTCFG->CIRQST & Hw4) {
        irq = INT_GPSB2_CORE;
	} else if (pPGPSBPORTCFG->CIRQST & Hw6) {
        irq = INT_GPSB3_CORE;
	} else if (pPGPSBPORTCFG->CIRQST & Hw8) {
        irq = INT_GPSB4_CORE;
	} else if (pPGPSBPORTCFG->CIRQST & Hw10) {
        irq = INT_GPSB5_CORE;
    } else {
	    //BITSET(pPIC->INTMSK1 , Hw4);	// using INTMSK
        BITSET(pPIC->CLR1, Hw4);
        goto out;
    }

    desc = irq_desc + irq;
    desc_handle_irq(irq, desc);
out:
    return;
}

static void tcc9300_irq_dma_handler(unsigned irq, struct irq_desc *desc)
{
	if (pPGDMACTRL0->CHCONFIG & (Hw18|Hw17|Hw16)) {
		if (pPGDMACTRL0->CHCONFIG & Hw16) {
			irq = INT_DMA0_CH0;
		} else if (pPGDMACTRL0->CHCONFIG & Hw17) {
			irq = INT_DMA0_CH1;
		} else if (pPGDMACTRL0->CHCONFIG & Hw18) {
			irq = INT_DMA0_CH2;
		} else {
			goto out1;
		}
	} else if (pPGDMACTRL1->CHCONFIG & (Hw18|Hw17|Hw16)) {
		if (pPGDMACTRL1->CHCONFIG & Hw16) {
			irq = INT_DMA1_CH0;
		} else if (pPGDMACTRL1->CHCONFIG & Hw17) {
			irq = INT_DMA1_CH1;
		} else if (pPGDMACTRL1->CHCONFIG & Hw18) {
			irq = INT_DMA1_CH2;
		} else {
			goto out1;
		}

	} else if (pPGDMACTRL2->CHCONFIG & (Hw18|Hw17|Hw16)) {
		if (pPGDMACTRL2->CHCONFIG & Hw16) {
			irq = INT_DMA2_CH0;
		} else if (pPGDMACTRL2->CHCONFIG & Hw17) {
			irq = INT_DMA2_CH1;
		} else if (pPGDMACTRL2->CHCONFIG & Hw18) {
			irq = INT_DMA2_CH2;
		} else {
			goto out1;
		}

	} else if (pPGDMACTRL3->CHCONFIG & (Hw18|Hw17|Hw16)) {
		if (pPGDMACTRL3->CHCONFIG & Hw16) {
			irq = INT_DMA3_CH0;
		} else if (pPGDMACTRL3->CHCONFIG & Hw17) {
			irq = INT_DMA3_CH1;
		} else if (pPGDMACTRL3->CHCONFIG & Hw18) {
			irq = INT_DMA3_CH2;
		} else {
			goto out1;
		}

	} else {
out1:
		BITSET(pPIC->CLR0, Hw29);
		goto out2;
	}
	
    desc = irq_desc + irq;
    desc_handle_irq(irq, desc);
out2:
    return;
}

static void tcc9300_irq_tc0_handler(unsigned irq, struct irq_desc *desc)
{
	if (pTIMER->TIREQ & Hw0) {
		irq = INT_TC0_TI0;
	} else if (pTIMER->TIREQ & Hw1) {
		irq = INT_TC0_TI1;
	} else if (pTIMER->TIREQ & Hw2) {
		irq = INT_TC0_TI2;
	} else if (pTIMER->TIREQ & Hw3) {
		irq = INT_TC0_TI3;
	} else if (pTIMER->TIREQ & Hw4) {
		irq = INT_TC0_TI4;
	} else if (pTIMER->TIREQ & Hw5) {
		irq = INT_TC0_TI5;
	} else {
		//BITSET(pPIC->INTMSK1 , Hw0);	// using INTMSK
		BITSET(pPIC->CLR0, Hw0);
		goto out;
	}

	desc = irq_desc + irq;
	desc_handle_irq(irq, desc);
out:
	return;
}


static struct irq_chip tcc9300_irq_chip = {
    .name       = "IRQ",
    .enable     = tcc9300_irq_enable,
    .disable    = tcc9300_irq_disable,
    .ack        = tcc9300_mask_ack_irq,
    .mask_ack   = tcc9300_mask_ack_irq,
    .mask       = tcc9300_mask_irq,
    .unmask     = tcc9300_unmask_irq,
    .set_wake   = tcc9300_wake_irq,
};

static struct irq_chip tcc9300_irq_uart_chip = {
    .name       = "IRQ_UART",
    .enable     = tcc9300_irq_dummy,
    .disable    = tcc9300_irq_dummy,
    .ack        = tcc9300_mask_ack_irq_uart,
    .mask_ack   = tcc9300_mask_ack_irq_uart,
    .mask       = tcc9300_mask_irq_uart,
    .unmask     = tcc9300_unmask_irq_uart,
    .set_wake   = tcc9300_wake_irq_uart,
};

static struct irq_chip tcc9300_irq_gpsb_chip = {
    .name       = "IRQ_GPSB",
    .enable     = tcc9300_irq_dummy,
    .disable    = tcc9300_irq_dummy,
    .ack        = tcc9300_mask_ack_irq_gpsb,
    .mask_ack   = tcc9300_mask_ack_irq_gpsb,
    .mask       = tcc9300_mask_irq_gpsb,
    .unmask     = tcc9300_unmask_irq_gpsb,
    .set_wake   = tcc9300_wake_irq_gpsb,
};

static struct irq_chip tcc9300_irq_dma_chip = {
    .name       = "IRQ_DMA",
    .enable     = tcc9300_irq_dummy,
    .disable    = tcc9300_irq_dummy,
    .ack        = tcc9300_mask_ack_irq_dma,
    .mask_ack   = tcc9300_mask_ack_irq_dma,
    .mask       = tcc9300_mask_irq_dma,
    .unmask     = tcc9300_unmask_irq_dma,
    .set_wake   = tcc9300_wake_irq_dma,
};

static struct irq_chip tcc9300_irq_tc0_chip = {
    .name       = "IRQ_TC0",
    .enable     = tcc9300_irq_dummy,
    .disable    = tcc9300_irq_dummy,
    .ack        = tcc9300_mask_ack_irq_tc0,
    .mask_ack   = tcc9300_mask_ack_irq_tc0,
    .mask       = tcc9300_mask_irq_tc0,
    .unmask     = tcc9300_unmask_irq_tc0,
    .set_wake   = tcc9300_wake_irq_tc0,
};

void __init tcc9300_irq_init(void)
{
	int irqno;

	printk("%s\n", __func__);

	//reset interrupt 
	pPIC = (volatile PPIC)tcc_p2v(HwPRIOINTRCTR_BASE);
	pVIC = (volatile PVIC)tcc_p2v(HwVECTINTRCTR_BASE);
	pPGPSBPORTCFG = (volatile PGPSBPORTCFG)tcc_p2v(HwGPSBPORTCFG_BASE);
	pUARTPORTMUX = (volatile PUARTPORTMUX)tcc_p2v(HwUARTPORTMUX_BASE);
	pPGDMACTRL0 = (volatile PGDMACTRL)tcc_p2v(HwGDMA0_BASE);
	pPGDMACTRL1 = (volatile PGDMACTRL)tcc_p2v(HwGDMA1_BASE);
	pPGDMACTRL2 = (volatile PGDMACTRL)tcc_p2v(HwGDMA2_BASE);
	pPGDMACTRL3 = (volatile PGDMACTRL)tcc_p2v(HwGDMA3_BASE);
	pTIMER = (volatile PTIMER)tcc_p2v(HwTMR_BASE);


	/* ADD IOREMAP */

	//clear IEN Field
	BITCLR(pPIC->IEN0 , 0xFFFFFFFF); // All Interrupt Disable
	BITCLR(pPIC->IEN1 , 0xFFFFFFFF); // All Interrupt Disable

	//clear SEL Field
	BITSET(pPIC->SEL0 , 0xFFFFFFFF); //using IRQ
	BITSET(pPIC->SEL1 , 0xFFFFFFFF); //using IRQ

	//clear TIG Field
	BITCLR(pPIC->TIG0 , 0xFFFFFFFF); //Test Interrupt Disable
	BITCLR(pPIC->TIG1 , 0xFFFFFFFF); //Test Interrupt Disable

	//clear POL Field
	BITCLR(pPIC->POL0 , 0xFFFFFFFF); //Default ACTIVE Low
	BITCLR(pPIC->POL1 , 0xFFFFFFFF); //Default ACTIVE Low

	//clear MODE Field
	BITSET(pPIC->MODE0 , 0xFFFFFFFF); //Trigger Mode - Level Trigger Mode
	BITSET(pPIC->MODE1 , 0xFFFFFFFF); //Trigger Mode - Level Trigger Mode

	//clear SYNC Field
	BITSET(pPIC->SYNC0 , 0xFFFFFFFF); //SYNC Enable
	BITSET(pPIC->SYNC1 , 0xFFFFFFFF); //SYNC Enable

	//clear WKEN Field
	BITCLR(pPIC->WKEN0 , 0xFFFFFFFF); //Wakeup all disable
	BITCLR(pPIC->WKEN1 , 0xFFFFFFFF); //Wakeup all disable

	//celar MODEA Field
	BITCLR(pPIC->MODEA0 , 0xFFFFFFFF); //both edge - all disable
	BITCLR(pPIC->MODEA1 , 0xFFFFFFFF); //both edge - all disable

	//clear INTMSK Field
	BITCLR(pPIC->INTMSK0 , 0xFFFFFFFF); //not using INTMSK
	BITCLR(pPIC->INTMSK1 , 0xFFFFFFFF); //not using INTMSK

	//clear ALLMSK Field
	BITCSET(pPIC->ALLMSK , 0xFFFFFFFF, 0x1); //using only IRQ

	/* Install the interrupt handlers */
	for(irqno = INT_TC0; irqno <= INT_ISP1; irqno++)
	{
		if (irqno == INT_UART) {
			set_irq_chip(INT_UART, &tcc9300_irq_uart_chip);
			set_irq_chained_handler(INT_UART, tcc9300_irq_uart_handler);
		} else if (irqno == INT_GPSB) {
			set_irq_chip(INT_GPSB, &tcc9300_irq_gpsb_chip);
			set_irq_chained_handler(INT_GPSB, tcc9300_irq_gpsb_handler);
		} else if (irqno == INT_GDMA0) {
			set_irq_chip(INT_GDMA0, &tcc9300_irq_dma_chip);
			set_irq_chained_handler(INT_GDMA0, tcc9300_irq_dma_handler);
		} else if (irqno == INT_TC0) {
			set_irq_chip(INT_TC0, &tcc9300_irq_tc0_chip);
			set_irq_chained_handler(INT_TC0, tcc9300_irq_tc0_handler);
		} else {
			set_irq_chip(irqno, &tcc9300_irq_chip);
			set_irq_handler(irqno, handle_level_irq);
			set_irq_flags(irqno, IRQF_VALID);
		}
	}

	/* Install the interrupt UART Group handlers */
	for (irqno = INT_UART0; irqno <= INT_UART5; irqno++) {
		set_irq_chip(irqno, &tcc9300_irq_uart_chip);
		set_irq_handler(irqno, handle_level_irq);
		set_irq_flags(irqno, IRQF_VALID);
	}

	/* Install the interrupt GPSB Group handlers */
	for (irqno = INT_GPSB0_DMA; irqno <= INT_GPSB5_CORE; irqno++) {
		set_irq_chip(irqno, &tcc9300_irq_gpsb_chip);
		set_irq_handler(irqno, handle_level_irq);
		set_irq_flags(irqno, IRQF_VALID);
	}

	/* Install the interrupt DMA Group handlers */
	for (irqno = INT_DMA0_CH0; irqno <= INT_DMA3_CH2; irqno++) {
		set_irq_chip(irqno, &tcc9300_irq_dma_chip);
		set_irq_handler(irqno, handle_level_irq);
		set_irq_flags(irqno, IRQF_VALID);
	}

	/* Install the interrupt TC0 Group handlers */
	for (irqno = INT_TC0_TI0; irqno <= INT_TC0_TI5; irqno++) {
		set_irq_chip(irqno, &tcc9300_irq_tc0_chip);
		set_irq_handler(irqno, handle_level_irq);
		set_irq_flags(irqno, IRQF_VALID);
	}

    /* IEN SET */
	BITSET(pPIC->IEN1,      Hw15);	/* UART */
	BITSET(pPIC->INTMSK1,   Hw15);
	BITSET(pPIC->IEN1,      Hw4);	/* GPSB */
	BITSET(pPIC->INTMSK1,   Hw4);
	BITSET(pPIC->IEN0,      Hw29);	/* DMA */
	BITSET(pPIC->INTMSK0,   Hw29);
	BITSET(pPIC->IEN0,		Hw0);	/* TC0 */
	BITSET(pPIC->INTMSK0,	Hw0);
}

/* end of file */
