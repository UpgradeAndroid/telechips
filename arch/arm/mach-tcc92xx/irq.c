/*
 * linux/arch/arm/mach-tcc92x/irq.c
 *
 * Author:  <linux@telechips.com>
 * Created: 10th February, 2009
 * Description: Interrupt handler for Telechips TCC9200 chipset
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
#include <mach/bsp.h>

#include <asm/io.h>

// Global 
static volatile PPIC pPIC;
static volatile PVIC pVIC;
static volatile PGPSBPORTCFG pPGPSBPORTCFG;
static volatile PUARTPORTMUX pUARTPORTMUX;
static volatile PGDMACTRL pPGDMACTRL0, pPGDMACTRL1, pPGDMACTRL2, pPGDMACTRL3;

#if defined(CONFIG_MMC_TCC_4SD_SLOT)
#define SD0_SLOT0_INT 0xF05A00FC
#define SD0_SLOT1_INT 0xF05A01FC
#define SD1_SLOT2_INT 0xF05A02FC
#define SD1_SLOT3_INT 0xF05A03FC
#endif
/******************************************
 * Disable IRQ
 *
 * If mask_ack exist, this is not called.
 *****************************************/
static void tcc9200_mask_irq(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq < 32) {
        BITCLR(pPIC->INTMSK0,   (1 << irq));
    } else {
        BITCLR(pPIC->INTMSK1,   (1 << (irq - 32)));
    }
}

static void tcc9200_mask_irq_uart(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_UART) {
        BITCLR(pPIC->INTMSK1, Hw15);
    }
}
static void tcc9200_mask_irq_gpsb(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_GPSB) {
        BITCLR(pPIC->INTMSK1, Hw4);
    }
}
static void tcc9200_mask_irq_dma(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_DMA) {
        BITCLR(pPIC->INTMSK0, Hw29);
    }
}

#if defined(CONFIG_MMC_TCC_4SD_SLOT)
static void tcc9200_mask_irq_sd0(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_SD0) {
        BITCLR(pPIC->INTMSK1, Hw12);
    }
}
static void tcc9200_mask_irq_sd1(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_SD1) {
        BITCLR(pPIC->INTMSK1, Hw13);
    }
}
#endif
/******************************************
 * Enable IRQ
 *****************************************/
static void tcc9200_irq_enable(struct irq_data *data)
{
    unsigned int irq = data->irq;

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

static void tcc9200_unmask_irq(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq < 32) {
        BITSET(pPIC->INTMSK0,   (1 << irq));
        BITSET(pPIC->CLR0,      (1 << irq));
    } else {
        BITSET(pPIC->INTMSK1,   (1 << (irq - 32)));
        BITSET(pPIC->CLR1,      (1 << (irq - 32)));
    }
}
static void tcc9200_unmask_irq_uart(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_UART) {
        BITSET(pPIC->INTMSK1, Hw15);
    }
}
static void tcc9200_unmask_irq_gpsb(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_GPSB) {
        BITSET(pPIC->INTMSK1, Hw4);
    }
}
static void tcc9200_unmask_irq_dma(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_DMA) {
        BITSET(pPIC->INTMSK0, Hw29);
    }
}

#if defined(CONFIG_MMC_TCC_4SD_SLOT)
static void tcc9200_unmask_irq_sd0(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_SD0) {
        BITSET(pPIC->INTMSK1, Hw12);
    }
}
static void tcc9200_unmask_irq_sd1(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_SD1) {
        BITSET(pPIC->INTMSK1, Hw13);
    }
}
#endif

/******************************************
 * Ack IRQ (Disable IRQ)
 *****************************************/

static void tcc9200_irq_disable(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq < 32){
        BITCLR(pPIC->IEN0,      (1 << irq));
        BITCLR(pPIC->INTMSK0,   (1 << irq));
    } else {
        BITCLR(pPIC->IEN1,      (1 << (irq - 32)));
        BITCLR(pPIC->INTMSK1,   (1 << (irq - 32)));
    }
}


static void tcc9200_mask_ack_irq(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq < 32){
        BITCLR(pPIC->INTMSK0,   (1 << irq));
    } else {
        BITCLR(pPIC->INTMSK1,   (1 << (irq - 32)));
    }
}

static void tcc9200_mask_ack_irq_uart(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_UART) {
        BITCLR(pPIC->INTMSK1, Hw15);
    }
}

static void tcc9200_mask_ack_irq_gpsb(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_GPSB) {
        BITCLR(pPIC->INTMSK1, Hw4);
    }
}

static void tcc9200_mask_ack_irq_dma(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_DMA) {
        BITCLR(pPIC->INTMSK0, Hw29);
    }
}

#if defined(CONFIG_MMC_TCC_4SD_SLOT)
static void tcc9200_mask_ack_irq_sd0(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_SD0) {
        BITCLR(pPIC->INTMSK1, Hw12);
    }
}
static void tcc9200_mask_ack_irq_sd1(struct irq_data *data)
{
    unsigned int irq = data->irq;

    if (irq != INT_SD1) {
        BITCLR(pPIC->INTMSK1, Hw13);
    }
}
#endif

/******************************************
 * wake IRQ
 *****************************************/
static int tcc9200_wake_irq(struct irq_data *data, unsigned int enable)
{
    return 0;
}

static int tcc9200_wake_irq_uart(struct irq_data *data, unsigned int enable)
{
    return 0;
}

static int tcc9200_wake_irq_gpsb(struct irq_data *data, unsigned int enable)
{
    return 0;
}

static int tcc9200_wake_irq_dma(struct irq_data *data, unsigned int enable)
{
    return 0;
}

#if defined(CONFIG_MMC_TCC_4SD_SLOT)
static int tcc9200_wake_irq_sd0(struct irq_data *data, unsigned int enable)
{
    return 0;
}
static int tcc9200_wake_irq_sd1(struct irq_data *data, unsigned int enable)
{
    return 0;
}
#endif

static void tcc9200_irq_dummy(struct irq_data *data)
{
}


static void tcc9200_irq_uart_handler(unsigned irq, struct irq_desc *desc)
{
	
    if (pUARTPORTMUX->CHST & Hw5) {
        irq = INT_UART5;
    } else if (pUARTPORTMUX->CHST & Hw4) {
        irq = INT_UART4;
    } else if (pUARTPORTMUX->CHST & Hw3) {
        irq = INT_UART3;
    } else if (pUARTPORTMUX->CHST & Hw2) {
        irq = INT_UART2;
    } else if (pUARTPORTMUX->CHST & Hw1) {
        irq = INT_UART1;
    } else if (pUARTPORTMUX->CHST & Hw0) {
        irq = INT_UART0;
    } else {
	    //BITSET(pPIC->INTMSK1 , Hw15); // using INTMSK
        BITSET(pPIC->CLR1, Hw15);
        goto out;
    }

    desc = irq_desc + irq;
    desc->handle_irq(irq, desc);
out:
    return;
}

static void tcc9200_irq_gpsb_handler(unsigned irq, struct irq_desc *desc)
{

    if (pPGPSBPORTCFG->CIRQST & Hw3) {
        irq = INT_GPSB1_DMA;
    } else if (pPGPSBPORTCFG->CIRQST & Hw1) {
        irq = INT_GPSB0_DMA;
    } else if (pPGPSBPORTCFG->CIRQST & Hw5) {
        irq = INT_GPSB2_DMA;
    } else if (pPGPSBPORTCFG->CIRQST & Hw3) {
        irq = INT_GPSB0_CORE;
    } else if (pPGPSBPORTCFG->CIRQST & Hw0) {
        irq = INT_GPSB1_CORE;
    } else if (pPGPSBPORTCFG->CIRQST & Hw2) {
        irq = INT_GPSB2_CORE;
	} else if (pPGPSBPORTCFG->CIRQST & Hw6) {
        irq = INT_GPSB3_CORE;
	} else if (pPGPSBPORTCFG->CIRQST & Hw8) {
        irq = INT_GPSB4_CORE;
	} else if (pPGPSBPORTCFG->CIRQST & Hw10) {
        irq = INT_GPSB5_CORE;
    } else {
	    //BITSET(pPIC->INTMSK1 , Hw4); // using INTMSK
        BITSET(pPIC->CLR1, Hw4);
        goto out;
    }

    desc = irq_desc + irq;
    desc->handle_irq(irq, desc);
out:
    return;
}

static void tcc9200_irq_dma_handler(unsigned irq, struct irq_desc *desc)
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
    desc->handle_irq(irq, desc);
out2:
    return;
}

#if defined(CONFIG_MMC_TCC_4SD_SLOT)
static void tcc9200_irq_sd0_handler(unsigned irq, struct irq_desc *desc)
{
	unsigned short slot_intr;
#if defined(CONFIG_MMC_TCC_SDHC_CORE0_SLOT0)
	slot_intr = tcc_readw(SD0_SLOT0_INT);
#elif defined(CONFIG_MMC_TCC_SDHC_CORE0_SLOT1)
	slot_intr = tcc_readw(SD0_SLOT1_INT);
#else
	slot_intr = 0;
#endif

	if (slot_intr & Hw0) {
		irq = INT_SD0_SLOT0;
	} else if (slot_intr & Hw1) {
		irq = INT_SD0_SLOT1;
	} else {
		//BITSET(pPIC->INTMSK1, Hw12);	// using INTMSK
        BITSET(pPIC->CLR1, Hw12);
        goto out;
	}

    desc = irq_desc + irq;
    desc->handle_irq(irq, desc);
out:
    return;
}
static void tcc9200_irq_sd1_handler(unsigned irq, struct irq_desc *desc)
{
	unsigned short slot_intr;
#if defined(CONFIG_MMC_TCC_SDHC_CORE1_SLOT2)
	slot_intr = tcc_readw(SD1_SLOT2_INT);
#elif defined(CONFIG_MMC_TCC_SDHC_CORE1_SLOT3)
	slot_intr = tcc_readw(SD1_SLOT3_INT);
#else
	slot_intr = 0;
#endif

	if (slot_intr & Hw0) {
		irq = INT_SD1_SLOT2;
	} else if (slot_intr & Hw1) {
		irq = INT_SD1_SLOT3;
	} else {
		//BITSET(pPIC->INTMSK1, Hw13);	// using INTMSK
        BITSET(pPIC->CLR1, Hw13);
        goto out;
	}

    desc = irq_desc + irq;
    desc->handle_irq(irq, desc);
out:
    return;
}
#endif

static struct irq_chip tcc9200_irq_chip = {
    .name       = "IRQ",
    .irq_enable     = tcc9200_irq_enable,
    .irq_disable    = tcc9200_irq_disable,
    .irq_ack        = tcc9200_mask_ack_irq,
    .irq_mask_ack   = tcc9200_mask_ack_irq,
    .irq_mask       = tcc9200_mask_irq,
    .irq_unmask     = tcc9200_unmask_irq,
    .irq_set_wake   = tcc9200_wake_irq,
};

static struct irq_chip tcc9200_irq_uart_chip = {
    .name       = "IRQ_UART",
    .irq_enable     = tcc9200_irq_dummy,
    .irq_disable    = tcc9200_irq_dummy,
    .irq_ack        = tcc9200_mask_ack_irq_uart,
    .irq_mask_ack   = tcc9200_mask_ack_irq_uart,
    .irq_mask       = tcc9200_mask_irq_uart,
    .irq_unmask     = tcc9200_unmask_irq_uart,
    .irq_set_wake   = tcc9200_wake_irq_uart,
};

static struct irq_chip tcc9200_irq_gpsb_chip = {
    .name       = "IRQ_GPSB",
    .irq_enable     = tcc9200_irq_dummy,
    .irq_disable    = tcc9200_irq_dummy,
    .irq_ack        = tcc9200_mask_ack_irq_gpsb,
    .irq_mask_ack   = tcc9200_mask_ack_irq_gpsb,
    .irq_mask       = tcc9200_mask_irq_gpsb,
    .irq_unmask     = tcc9200_unmask_irq_gpsb,
    .irq_set_wake   = tcc9200_wake_irq_gpsb,
};

static struct irq_chip tcc9200_irq_dma_chip = {
    .name       = "IRQ_DMA",
    .irq_enable     = tcc9200_irq_dummy,
    .irq_disable    = tcc9200_irq_dummy,
    .irq_ack        = tcc9200_mask_ack_irq_dma,
    .irq_mask_ack   = tcc9200_mask_ack_irq_dma,
    .irq_mask       = tcc9200_mask_irq_dma,
    .irq_unmask     = tcc9200_unmask_irq_dma,
    .irq_set_wake   = tcc9200_wake_irq_dma,
};

#if defined(CONFIG_MMC_TCC_4SD_SLOT)
static struct irq_chip tcc9200_irq_sd0_chip = {
    .name       = "IRQ_SD0",
    .irq_enable     = tcc9200_irq_dummy,
    .irq_disable    = tcc9200_irq_dummy,
    .irq_ack        = tcc9200_mask_ack_irq_sd0,
    .irq_mask_ack   = tcc9200_mask_ack_irq_sd0,
    .irq_mask       = tcc9200_mask_irq_sd0,
    .irq_unmask     = tcc9200_unmask_irq_sd0,
    .irq_set_wake   = tcc9200_wake_irq_sd0,
};
static struct irq_chip tcc9200_irq_sd1_chip = {
    .name       = "IRQ_SD1",
    .irq_enable     = tcc9200_irq_dummy,
    .irq_disable    = tcc9200_irq_dummy,
    .irq_ack        = tcc9200_mask_ack_irq_sd1,
    .irq_mask_ack   = tcc9200_mask_ack_irq_sd1,
    .irq_mask       = tcc9200_mask_irq_sd1,
    .irq_unmask     = tcc9200_unmask_irq_sd1,
    .irq_set_wake   = tcc9200_wake_irq_sd1,
};
#endif

void __init tcc9200_irq_init(void)
{
    int irqno;

	printk("%s\n", __func__);

	//reset interrupt 
    pPIC = (volatile PPIC)tcc_p2v(HwPIC_BASE);
   	pVIC = (volatile PVIC)tcc_p2v(HwVIC_BASE);
	pPGPSBPORTCFG = (volatile PGPSBPORTCFG)tcc_p2v(HwGPSBPORTCFG_BASE);
	pUARTPORTMUX = (volatile PUARTPORTMUX)tcc_p2v(HwUARTPORTMUX_BASE);
	pPGDMACTRL0 = (volatile PGDMACTRL)tcc_p2v(HwGDMA0_BASE);
    pPGDMACTRL1 = (volatile PGDMACTRL)tcc_p2v(HwGDMA1_BASE);
    pPGDMACTRL2 = (volatile PGDMACTRL)tcc_p2v(HwGDMA2_BASE);
    pPGDMACTRL3 = (volatile PGDMACTRL)tcc_p2v(HwGDMA3_BASE);

    
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
	BITSET(pPIC->POL1 , 0x10000000); /* set ARM11 PMU interrupt signal to active-low */

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
    for(irqno = INT_TC0; irqno <= INT_AEIRQ; irqno++)
    {
        if (irqno == INT_UART) {
            irq_set_chip(INT_UART, &tcc9200_irq_uart_chip);
            irq_set_chained_handler(INT_UART, tcc9200_irq_uart_handler);
        } else if (irqno == INT_GPSB) {
            irq_set_chip(INT_GPSB, &tcc9200_irq_gpsb_chip);
            irq_set_chained_handler(INT_GPSB, tcc9200_irq_gpsb_handler);
        } else if (irqno == INT_DMA) {
            irq_set_chip(INT_DMA, &tcc9200_irq_dma_chip);
            irq_set_chained_handler(INT_DMA, tcc9200_irq_dma_handler);
#if defined(CONFIG_MMC_TCC_4SD_SLOT)			
		} else if (irqno == INT_SD0) {
			irq_set_chip(INT_SD0, &tcc9200_irq_sd0_chip);
			irq_set_chained_handler(INT_SD0, tcc9200_irq_sd0_handler);
		} else if (irqno == INT_SD1) {
			irq_set_chip(INT_SD1, &tcc9200_irq_sd1_chip);
			irq_set_chained_handler(INT_SD1, tcc9200_irq_sd1_handler);
#endif			
        } else {
            irq_set_chip(irqno, &tcc9200_irq_chip);
            irq_set_handler(irqno, handle_level_irq);
            set_irq_flags(irqno, IRQF_VALID);
        }
    }

    /* Install the interrupt UART Group handlers */
    for (irqno = INT_UART0; irqno <= INT_UART5; irqno++) {
        irq_set_chip(irqno, &tcc9200_irq_uart_chip);
        irq_set_handler(irqno, handle_level_irq);
        set_irq_flags(irqno, IRQF_VALID);
    }

    /* Install the interrupt GPSB Group handlers */
    for (irqno = INT_GPSB0_DMA; irqno <= INT_GPSB5_CORE; irqno++) {
        irq_set_chip(irqno, &tcc9200_irq_gpsb_chip);
        irq_set_handler(irqno, handle_level_irq);
        set_irq_flags(irqno, IRQF_VALID);
    }

    /* Install the interrupt DMA Group handlers */
    for (irqno = INT_DMA0_CH0; irqno <= INT_DMA3_CH2; irqno++) {
        irq_set_chip(irqno, &tcc9200_irq_dma_chip);
        irq_set_handler(irqno, handle_level_irq);
        set_irq_flags(irqno, IRQF_VALID);
    }

#if defined(CONFIG_MMC_TCC_4SD_SLOT)
	/* Install the interrupt SD0 Group handlers */
	for (irqno = INT_SD0_SLOT0; irqno <= INT_SD0_SLOT1; irqno++) {
		irq_set_chip(irqno, &tcc9200_irq_sd0_chip);
		irq_set_handler(irqno, handle_level_irq);
		set_irq_flags(irqno, IRQF_VALID);
	}
	/* Install the interrupt SD1 Group handlers */
	for (irqno = INT_SD1_SLOT2; irqno <= INT_SD1_SLOT3; irqno++) {
		irq_set_chip(irqno, &tcc9200_irq_sd1_chip);
		irq_set_handler(irqno, handle_level_irq);
		set_irq_flags(irqno, IRQF_VALID);
	}
#endif

    /* IEN SET */
    BITSET(pPIC->IEN1,      Hw15);   /* UART */
    BITSET(pPIC->INTMSK1,   Hw15);   /* UART */
    BITSET(pPIC->IEN1,      Hw4);    /* GPSB */
    BITSET(pPIC->INTMSK1,   Hw4);    /* GPSB */
    BITSET(pPIC->IEN0,      Hw29);   /* DMA */
    BITSET(pPIC->INTMSK0,   Hw29);   /* DMA */
#if defined(CONFIG_MMC_TCC_4SD_SLOT)	
	BITSET(pPIC->IEN1,      Hw12);	/* SD0 */
	BITSET(pPIC->INTMSK1,   Hw12);
	BITSET(pPIC->IEN1,      Hw13);	/* SD1 */
	BITSET(pPIC->INTMSK1,   Hw13);
#endif
}

/* end of file */
