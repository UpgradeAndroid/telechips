/*
 * linux/arch/arm/mach-tcc93xx/include/mach/hardware.h
 *
 * Rewritten by:    <linux@telechips.com>
 * Modifiedd: August 30, 2010
 * Description: Hardware definitions for TCC9300 processors and boards
 * Author: RidgeRun, Inc. Greg Lonnon <glonnon@ridgerun.com>
 * Reorganized for Linux-2.6 by Tony Lindgren <tony@atomide.com>
 *                          and Dirk Behme <dirk.behme@de.bosch.com>
 *
 * Copyright (C) 2001 RidgeRun, Inc.
 * Copyright (C) 2008-2010 Telechips, Inc.
 *
 * NOTE: Please put device driver specific defines into a separate header
 *	 file for each driver.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __ASM_ARCH_TCC9300_HARDWARE_H
#define __ASM_ARCH_TCC9300_HARDWARE_H

#include <asm/sizes.h>
#ifndef __ASSEMBLER__
#include <asm/types.h>
#endif
#include <mach/io.h>

#define TCC_GPA_PHYS_BASE	0xB010A000
#define TCC_GPB_PHYS_BASE	0xB010A040
#define TCC_GPC_PHYS_BASE	0xB010A080
#define TCC_GPD_PHYS_BASE	0xB010A0C0
#define TCC_GPE_PHYS_BASE	0xB010A100
#define TCC_GPF_PHYS_BASE	0xB010A140
#define TCC_GPG_PHYS_BASE	0xB010A180

#define TCC_I2C_CORE0_PHYS_BASE	0xB0109000
#define TCC_I2C_CORE1_PHYS_BASE	0xB0109100
#define TCC_I2C_CORE2_PHYS_BASE	0xB0109200
#define TCC_SMUI2C_PHYS_BASE	0xB0505000

#define TCC_SDMMC0_PHYS_BASE	0xB0020000
#define TCC_SDMMC1_PHYS_BASE	0xB0020200
#define TCC_SDMMC2_PHYS_BASE	0xB0020400
#define TCC_SDMMC3_PHYS_BASE	0xB0020600

#define TCC_SPI0_BASE	0xB0107000
#define TCC_SPI1_BASE	0xB0107100

#define TCC_TSADC_BASE      (0xB0100000)
#define TCC_RTC_BASE        (0xF0502000)
#define TCC_I2C_BASE        (0xF0109000)
#define TCC_UART0_BASE      (0xF0102000)
#define TCC_UART1_BASE      (0xF0102100)
#define TCC_UART2_BASE      (0xF0102200)
#define TCC_UART3_BASE      (0xF0102300)
#define TCC_UART4_BASE      (0xF0102400)
#define TCC_UART5_BASE      (0xF0102500)
#define TCC_USBOTG0_BASE    (0xF0040000)

#define TCC_GMAC_BASE		(0xF0820000)
#define TCC_GMAC_DMA_BASE	(0xF0821000)
/*
 * ----------------------------------------------------------------------------
 * Clocks
 * ----------------------------------------------------------------------------
 */
#define CLKGEN_REG_BASE		(0xfffece00)
#define ARM_CKCTL		(CLKGEN_REG_BASE + 0x0)
#define ARM_IDLECT1		(CLKGEN_REG_BASE + 0x4)
#define ARM_IDLECT2		(CLKGEN_REG_BASE + 0x8)
#define ARM_EWUPCT		(CLKGEN_REG_BASE + 0xC)
#define ARM_RSTCT1		(CLKGEN_REG_BASE + 0x10)
#define ARM_RSTCT2		(CLKGEN_REG_BASE + 0x14)
#define ARM_SYSST		(CLKGEN_REG_BASE + 0x18)
#define ARM_IDLECT3		(CLKGEN_REG_BASE + 0x24)

/* DPLL control registers */
#define DPLL_CTL		(0xfffecf00)

#endif	/* __ASM_ARCH_TCC8900_HARDWARE_H */
