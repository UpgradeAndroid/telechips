/* linux/include/asm-arm/arch-tcc93xx/irqs.h
 *
 * Author: <linux@telechips.com>
 * Created: August 30, 2010
 * Copyright (C) 2010 Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __ASM_ARCH_TCC_IRQS_H
#define __ASM_ARCH_TCC_IRQS_H

/*
 * IRQ numbers for interrupt handler
 */

#define INT_TC0    	0
#define INT_TC32    1
#define INT_RTC     2
#define INT_EI0		3
#define INT_EI1		4
#define INT_EI2		5
#define INT_EI3		6
#define INT_EI4		7
#define INT_EI5		8
#define INT_EI6		9	
#define INT_EI7		10
#define INT_EI8		11
#define INT_EI9		12
#define INT_EI10	13
#define INT_EI11	14
#define INT_SC0		15
#define INT_SC1		16
#define INT_CIF		17
#define INT_LCD0	18
#define INT_LCD1	19
#define INT_VIPET	20
#define INT_JPGE	21
#define INT_ISP2    22
#define INT_VCDC	23
#define INT_3DPP	24
#define INT_3DGP	25
#define INT_3DMMU	26
#define INT_G2D		27
#define INT_TSADC	28
#define INT_GDMA0   29
#define INT_EHI0	31

#define INT_EHI1	32
#define INT_MIPID   33 
#define INT_HDMI	34
#define INT_SATAH	35
#define INT_GPSB	36
#define INT_SATAD   37
#define INT_I2C		38 
#define INT_CKC     39
#define INT_MS		40
#define INT_NFC		41
#define INT_RMT		42
#define INT_SMUI2C  43 
#define INT_SD0		44
#define INT_SD1		45
#define INT_SD3     46
#define INT_UART	47
#define INT_UOTG	48 
#define INT_UOTG1   49
#define INT_GDMA3   50
#define INT_ADMA1   51
#define INT_CIPHER  52
#define INT_TSIF0	53
#define INT_TSIF1	54
#define INT_GMAC    55
#define INT_ISP3    56
#define INT_AUDIO1  57
#define INT_ADMA0   58
#define INT_AUDIO0	59
#define INT_APMU	60
#define INT_SD2     61
#define INT_ISP0    62
#define INT_ISP1    63

/*
 * IRQ_UT numbers for UART[0:5]
 */
#define INT_UT_BASE         64
#define INT_UART0           (0 + INT_UT_BASE)
#define INT_UART1           (1 + INT_UT_BASE) 
#define INT_UART2           (2 + INT_UT_BASE) 
#define INT_UART3           (3 + INT_UT_BASE) 
#define INT_UART4           (4 + INT_UT_BASE) 
#define INT_UART5           (5 + INT_UT_BASE) 

/*
 * GPSB-IRQ numbers for GPSB0 & GPSB0
 */
#define INT_GPSB0_BASE      70          
#define INT_GPSB0_DMA       (0 + INT_GPSB0_BASE)          
#define INT_GPSB1_DMA       (1 + INT_GPSB0_BASE)
#define INT_GPSB2_DMA       (2 + INT_GPSB0_BASE)

#define INT_GPSB0_CORE      (3 + INT_GPSB0_BASE)
#define INT_GPSB1_CORE      (4 + INT_GPSB0_BASE) 
#define INT_GPSB2_CORE      (5 + INT_GPSB0_BASE) 
#define INT_GPSB3_CORE      (6 + INT_GPSB0_BASE) 
#define INT_GPSB4_CORE      (7 + INT_GPSB0_BASE) 
#define INT_GPSB5_CORE      (8 + INT_GPSB0_BASE) 

/*
 * DMA-IRQ numbers
 */
#define INT_DMA0_BASE       79 
#define INT_DMA0_CH0        (0 + INT_DMA0_BASE) 
#define INT_DMA0_CH1        (1 + INT_DMA0_BASE) 
#define INT_DMA0_CH2        (2 + INT_DMA0_BASE) 
#define INT_DMA1_CH0        (3 + INT_DMA0_BASE) 
#define INT_DMA1_CH1        (4 + INT_DMA0_BASE) 
#define INT_DMA1_CH2        (5 + INT_DMA0_BASE) 
#define INT_DMA2_CH0        (6 + INT_DMA0_BASE) 
#define INT_DMA2_CH1        (7 + INT_DMA0_BASE) 
#define INT_DMA2_CH2        (8 + INT_DMA0_BASE)
#define INT_DMA3_CH0        (9 + INT_DMA0_BASE) 
#define INT_DMA3_CH1        (10 + INT_DMA0_BASE) 
#define INT_DMA3_CH2        (11 + INT_DMA0_BASE) 

/*
 * TC0-IRQ numbers for Timer/Counter 0~5
 */
#define INT_TC0_BASE		91
#define INT_TC0_TI0		(0 + INT_TC0_BASE)
#define INT_TC0_TI1		(1 + INT_TC0_BASE)
#define INT_TC0_TI2		(2 + INT_TC0_BASE)
#define INT_TC0_TI3		(3 + INT_TC0_BASE)
#define INT_TC0_TI4		(4 + INT_TC0_BASE)
#define INT_TC0_TI5		(5 + INT_TC0_BASE)

/*
 * SD0/1-IRQ numbers for Slot[3:0]
 */
#define INT_SDHC0_BASE		97
#define INT_SDHC0		(0 + INT_SDHC0_BASE)
#define INT_SDHC1		(1 + INT_SDHC0_BASE)
#define INT_SDHC2		(2 + INT_SDHC0_BASE)
#define INT_SDHC3		(3 + INT_SDHC0_BASE)


/*
 * NR_IRQ:
 */
#define NR_IRQS ((INT_ISP1 + 1)\
                 + (INT_UART5 - INT_UT_BASE + 1)\
                 + (INT_GPSB5_CORE - INT_GPSB0_BASE + 1)\
                 + (INT_DMA3_CH2 - INT_DMA0_BASE + 1)\
                 + (INT_TC0_TI5 - INT_TC0_BASE + 1)\
                 + (INT_SDHC3 - INT_SDHC0_BASE + 1))

#endif  /* ASM_ARCH_TCC_IRQS_H */

