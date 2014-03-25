/*
 * linux/arch/arm/plat-tcc/tcc_asm.S
 *
 * Author:  <linux@telechips.com>
 * Created: April 21, 2008
 * Description: LINUX POWER MANAGEMENT FUNCTIONS
 *
 * Copyright (C) 2008-2009 Telechips 
 *
 * This program is free software@ you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation@ either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY@ without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program@ if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <linux/linkage.h>
//#include <asm/hardware.h>
#include <mach/hardware.h>
#include <asm/ptrace.h>

#define	Hw31									0x80000000
#define	Hw30									0x40000000
#define	Hw29									0x20000000
#define	Hw28									0x10000000
#define	Hw27									0x08000000
#define	Hw26									0x04000000
#define	Hw25									0x02000000
#define	Hw24									0x01000000
#define	Hw23									0x00800000
#define	Hw22									0x00400000
#define	Hw21									0x00200000
#define	Hw20									0x00100000
#define	Hw19									0x00080000
#define	Hw18									0x00040000
#define	Hw17									0x00020000
#define	Hw16									0x00010000
#define	Hw15									0x00008000
#define	Hw14									0x00004000
#define	Hw13									0x00002000
#define	Hw12									0x00001000
#define	Hw11									0x00000800
#define	Hw10									0x00000400
#define	Hw9										0x00000200
#define	Hw8										0x00000100
#define	Hw7										0x00000080
#define	Hw6										0x00000040
#define	Hw5										0x00000020
#define	Hw4										0x00000010
#define	Hw3										0x00000008
#define	Hw2										0x00000004
#define	Hw1										0x00000002
#define	Hw0										0x00000001

// PMU Control
#define	HwPMU_CONTROL_IOR			Hw31										// I/O Retension Enable Register (0:Not-Retension, 1:Retension)
#define	HwPMU_CONTROL_FWKU			Hw30										// Fast Wakeup Enable Register (0:Normal, 1:Fast)
#define	HwPMU_CONTROL_FPO			Hw29										// Fast Power-Off (0:Normal, 1:Fast)
#define	HwPMU_CONTROL_AISO			Hw18										// Touch Screen ADC Isolation Enable Register (0:Isolated, 1:Not-Isolated)
#define	HwPMU_CONTROL_ASTM			Hw17										// Touch Screen ADC Stop Mode Register (0:Normal, 1:Stop Mode)
#define	HwPMU_CONTROL_APEN			Hw16										// Touch Screen ADC Power Enable Register (0:Disable, 1:Enable)
#define	HwPMU_CONTROL_INITR			Hw4											// Boot Memory Configuration Register with ITCM (0:Boot-ROM, 1:ITCM)
#define	HwPMU_CONTROL_DPDN			Hw3											// Deep Power Down Mode Register (0:Normal, 1:Deep Power-Down)
#define	HwPMU_CONTROL_PDN			Hw2											// Power Down Mode Register (0:Normal, 1:Power-Down)
#define	HwPMU_CONTROL_POFF			Hw1											// Power Off Mode Register (0:Normal, 1:Power-Off)
#define	HwPMU_CONTROL_XIEN			Hw0											// Main Oscillatro Enable Register (0:Disable, 1:Enable)

// Wake UP Control
#define	HwPMU_WKUP_GPIOE25			Hw31
#define	HwPMU_WKUP_GPIOE24			Hw30
#define	HwPMU_WKUP_GPIOE05			Hw29
#define	HwPMU_WKUP_GPIOE04			Hw28
#define	HwPMU_WKUP_GPIOB31			Hw27
#define	HwPMU_WKUP_GPIOB30			Hw26
#define	HwPMU_WKUP_GPIOA15			Hw25
#define	HwPMU_WKUP_GPIOA14			Hw24
#define	HwPMU_WKUP_GPIOA13			Hw23
#define	HwPMU_WKUP_GPIOA12			Hw22
#define	HwPMU_WKUP_GPIOA11			Hw21
#define	HwPMU_WKUP_GPIOA10			Hw20
#define	HwPMU_WKUP_GPIOA07			Hw19
#define	HwPMU_WKUP_GPIOA06			Hw18
#define	HwPMU_WKUP_GPIOA05			Hw17
#define	HwPMU_WKUP_GPIOA04			Hw16
#define	HwPMU_WKUP_GPIOA03			Hw15
#define	HwPMU_WKUP_GPIOA02			Hw14
#define	HwPMU_WKUP_TSC_UPDOWN		Hw13										// Touch Screen Up/Down Signal
#define	HwPMU_WKUP_TSC_STOP_WKU		Hw12										// Touch Screen Stop Wakeup Signal
#define	HwPMU_WKUP_GPIOD18			Hw11
#define	HwPMU_WKUP_TSC_WKU			Hw10										// Touch Screen Wake-Up Signal
#define	HwPMU_WKUP_GPIOF23			Hw9
#define	HwPMU_WKUP_GPIOF24			Hw8
#define	HwPMU_WKUP_GPIOF25			Hw7
#define	HwPMU_WKUP_GPIOF26			Hw6
#define	HwPMU_WKUP_GPIOF27			Hw5
#define	HwPMU_WKUP_GPIOC31			Hw4
#define	HwPMU_WKUP_GPIOC30			Hw3
#define	HwPMU_WKUP_GPIOC29			Hw2
#define	HwPMU_WKUP_GPIOC28			Hw1
#define	HwPMU_WKUP_RTCWKUP			Hw0											// RTC Wakeup Output Signal

// WatchDoc Control
#define	HwPMU_WATCHDOC_EN			Hw31										// Watchdog Enable Register (0:disable, 1:enable)
#define	HwPMU_WATCHDOC_CLR			Hw30										// Watchdog Clear Register

// Power Off Control
#define	HwPMU_PWROFF_IOB			Hw9											// I/O Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_GB				Hw8											// Graphic Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_DB				Hw7											// DDI Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_VB				Hw6											// Video Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_MB				Hw5											// Memory Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_RSVD2			Hw4											// Reserved
#define	HwPMU_PWROFF_UP				Hw3											// USB Nano Phy Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_RSVD1			Hw2											// Reserved
#define	HwPMU_PWROFF_HD				Hw1											// HDMI Phy Power-Off Control Register (0:Power-On, 1:Power-Off)
#define	HwPMU_PWROFF_DAC			Hw0											// Video DAC Power-Off Control Register (0:Power-On, 1:Power-Off)

#define HwPMU_CONTROL				0x00000000
#define HwDRAM_BASE               	0xF0301000
#define HwDRAM_PL340_BASE         	0xF0301000
#define HwDRAM_PL341_BASE         	0xF0302000
#define HwDRAM_CMD                	0x00000004
#define HwDRAM_DIRECT_CMD         	0x00000008
#define HwDRAM_CFG                	0x0000000C
#define HwDRAM_CFG_STOP_MCLK      	Hw14
#define HwDRAM_CFG_SREN           	Hw31

#define HwDRAM_MISC_BASE          	0xF0303000

#define HwDRAM_PHY_BASE           	0xF0304000
#define HwDRAM_PHY_CTRL           	0x00000400
#define HwDRAM_PHY_CTRL_PD        	Hw2
#define HwDRAM_PHY_DLL_CTRL       	0x00000404
#define HwDRAM_PHY_DLL_CTRL_ON    	Hw0
#define HwDRAM_PHY_DLL_CTRL_START 	Hw1
#define HwDRAM_PHY_ZQ_CTRL        	0x00000428
#define HwDRAM_PHY_ZQ_CTRL_START  	Hw0
#define HwDRAM_PHY_ZQ_CTRL_UPDATE 	Hw1

#define HwDRAM_MEMBUS_BASE        	0xF0305000
#define HwDRAM_MEMBUS_CKDOWN      	0x00000004
#define HwDRAM_MEMBUS_CKDOWN_MBUS 	Hw0
#define HwDRAM_MEMBUS_CKDOWN_SDDR 	Hw1
#define HwDRAM_MEMBUS_CKDOWN_DDR2 	Hw2

#define HwCLKCTRL0			0x00000000		// R/W, CPU Clock Control Register
#define HwCLKCTRL1			0x00000004		// R/W, BUS Clock Control Register for Display Devices
#define HwCLKCTRL2			0x00000008		// R/W, BUS Clock Control Register for Memory Interface
#define HwCLKCTRL3			0x0000000C		// R/W, BUS Clock Control Register for Graphic Devices
#define HwCLKCTRL4			0x00000010		// R/W, BUS Clock Control Register for I/O Interface Devices
#define HwCLKCTRL5			0x00000014		// R/W, BUS Clock Control Register for Video CODEC
#define HwCLKCTRL6			0x00000018		// R/W, Core Clock Control Register for Video CODEC
#define HwCLKCTRL7			0x0000001C		// R/W, BUS Clock Control Register for SMU Hardwares

#define HwPLL0CFG			0x00000020		// R/W, PLL0 Configuration Register
#define HwPLL1CFG			0x00000024		//R/W, PLL1 Configuration Register
#define HwPLL2CFG			0x00000028		//R/W, PLL2 Configuration Register
#define HwPLL3CFG			0x0000002C		//R/W, PLL3 Configuration Register

#define HwPMU_PWROFF			0x00000018   //R/W	Power-Off Control Register 
#define HwPMU_PWROFF_IOB		Hw9	//I/O Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define HwPMU_PWROFF_GB 		Hw8	//Graphic Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define HwPMU_PWROFF_DB 		Hw7	//DDI Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define HwPMU_PWROFF_VB 		Hw6	//Video Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define HwPMU_PWROFF_MB 		Hw5	//Memory Bus Power-Off Control Register (0:Power-On, 1:Power-Off)
#define HwPMU_PWROFF_RSVD2		Hw4	//Reserved
#define HwPMU_PWROFF_UP 		Hw3	//USB Nano Phy Power-Off Control Register (0:Power-On, 1:Power-Off)
#define HwPMU_PWROFF_RSVD1		Hw2	//Reserved
#define HwPMU_PWROFF_HD 		Hw1	//HDMI Phy Power-Off Control Register (0:Power-On, 1:Power-Off)
#define HwPMU_PWROFF_DAC		Hw0	//Video DAC Power-Off Control Register (0:Power-On, 1:Power-Off)
#define HwCKC_BASE              0xF0400000	// Clock Controller Base Register
#define HwPMU_BASE  			0xF0404000 	//R/W   PMU Control Register

#define INTERRUPT_BITS		0xC0

#define ENABLE_IRQ			0x0
#define ENABLE_FIQ			0x0
#define DISABLE_FIQ		0x40
#define DISABLE_IRQ		0x80


#define HwALLMSK_FIQ		Hw1										// FIQ mask register
#define HwALLMSK_IRQ		Hw0										// IRQ mask register

#define HwALLMSK			0x00000108		// R/W, All Mask Register
#define HwALLMSK_FIQ		Hw1				// FIQ mask register
#define HwALLMSK_IRQ		Hw0				// IRQ mask register
#define HwPIC_BASE			0xF0401000


	.text

//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//------------------------------------------------------------------------------- 
ENTRY(IO_ARM_DisableINT)

	mrs	r0, cpsr
	orr	r1, r0, #INTERRUPT_BITS
	msr	cpsr_cf, r1
	bx	lr				@mov	pc, lr

	
//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//------------------------------------------------------------------------------- 
ENTRY(IO_INT_EnableINT)

	ldr	r1, =HwPIC_BASE
	ldr	r0, [r1, #HwALLMSK]
	orr	r2, r0, #(HwALLMSK_FIQ + HwALLMSK_IRQ)
	str	r2, [r1, #HwALLMSK]
	bx	lr				@mov	pc, lr


//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//------------------------------------------------------------------------------- 
ENTRY(IO_ARM_CleanDCACHE)

		stmdb r13!, {r0-r2, lr}

		mov r1, #0                  @ r1 - way (0~3)
l01:
		mov r1, r1, lsl #30
		mov r2, #0                  @ r2 - set index (128 set : 0~0x1000)
l02:
		orr r0, r1, r2              @ set index (way & set)
		mcr p15, 0, r0, c7, c10, 2  @ clean cache (using index)
		add r2, r2, #0x20        @ cache block size : 32 Bytes
		cmp r2, #0x1000             @ cache set size : 4 KBytes
		bne l02                    @ loop until done
		
		mov r1, r1, lsr #30
		add r1, r1, #1
		cmp r1, #4                  @ cache way size : 4
		bne l01

		ldmia r13!, {r0-r2, pc}





//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//------------------------------------------------------------------------------- 
ENTRY(IO_ARM_FlushCACHE)
		mov	r0, #0
		mcr	p15, 0, r0, c7, c5, 0		@ ICACHE

		mov r1, #0                  @ r1 - way (0~3)
loop011:
		mov r1, r1, lsl #30
		mov r2, #0                  @ r2 - set index (128 set : 0~0x1000)
loop022:
		orr r0, r1, r2              @ set index (way & set)
		mcr p15, 0, r0, c7, c6, 2   @ invalidate cache (using index)
		//mov r3, #0x20
		add r2, r2, #0x20 //#1:shl:5        @ cache block size : 32 Bytes
		cmp r2, #0x1000             @ cache set size : 4 KBytes
		bne loop022                    @ loop until done
		
		mov r1, r1, lsr #30
		add r1, r1, #1
		cmp r1, #4                  @ cache way size : 4
		bne loop011

		bx	lr				@mov	pc, lr



//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//------------------------------------------------------------------------------- 

ENTRY(IO_ARM_CleanCACHE)

	stmdb r13!, {lr}

	bl IO_ARM_CleanDCACHE

	cmp	r0, #0
	movne	r0, #0				@ Drain Write Buffer
	mcrne	p15, 0, r0, c7, c10, 4

	ldmia r13!, {pc}



ENTRY(IO_CKC_EnterSRFInt)

	//@sdram.command = 3
	ldr r2, =HwDRAM_PL340_BASE
	ldr r1, =0x3 //@ pause

	str r1, [r2, #HwDRAM_CMD]

loop01:
	ldr r1, [r2]
	and r1, r1, #0x3
	cmp	r1, #0x2 //@pause
	bne loop01

	//@sdram.command = 1
	ldr r1, =0x1 //@ sleep
	str r1, [r2, #HwDRAM_CMD]

loop02:
	ldr r1, [r2]
	and r1, r1, #0x3
	cmp	r1, #0x3 //@low power
	bne loop02

	//@sdram_phy.phy_mode_ctrl.pd = 1 : To prevent input leakage
	ldr	r2, =HwDRAM_PHY_BASE
	ldr	r1, [r2,#HwDRAM_PHY_CTRL]
	orr	r1, r1, #HwDRAM_PHY_CTRL_PD
	str	r1, [r2,#HwDRAM_PHY_CTRL]

	//@sdram_phy.dll_ctrl.on = 0, sdram_phy.dll_ctrl.start = 0
	ldr	r1, [r2,#HwDRAM_PHY_DLL_CTRL]
	bic	r1, r1, #(HwDRAM_PHY_DLL_CTRL_ON+HwDRAM_PHY_DLL_CTRL_START)
	str	r1, [r2,#HwDRAM_PHY_DLL_CTRL]

	//@sdram_phy.zq_ctrl.start = 1, sdram_phy.zq_ctrl.update = 0
	ldr	r1, [r2,#HwDRAM_PHY_ZQ_CTRL]
	orr	r1, r1, #HwDRAM_PHY_ZQ_CTRL_START
	bic	r1, r1, #HwDRAM_PHY_ZQ_CTRL_UPDATE
	str	r1, [r2,#HwDRAM_PHY_ZQ_CTRL]
	
	//@Delay
	mov	r1, #0x5
loop03:
	nop
	sub	r1, r1, #0x1
	cmp	r1, #0x0
	bne	loop03

	//@sdram.command = 3
	ldr r2, =HwDRAM_PL341_BASE
	ldr r1, =0x3 //@ pause
	str r1, [r2, #HwDRAM_CMD]
loop04:
	ldr r1, [r2]
	and r1, r1, #0x3
	cmp	r1, #0x2 //@ pause
	bne loop04

	//@sdram.command = 1
	ldr r1, =0x1 //@ sleep
	str r1, [r2, #HwDRAM_CMD]
loop05:
	ldr r1, [r2]
	and r1, r1, #0x3
	cmp	r1, #0x3 //@low power
	bne loop05

	//@phy_ctrl.pl340_sel = 0	
	ldr	r2,	=HwDRAM_MISC_BASE
	ldr	r1,	[r2, #0x24]
	bic	r1,	r1, #0x100
	str	r1, [r2, #0x24]

	//@phy_ctrl.ioCFG0 = 0x00003fff
	ldr r1, =0x00003FFF
	str	r1, [r2, #0x2c]

	//@sdram.cfg.stop_mclk = 1
	ldr	r2,	=HwDRAM_PL341_BASE
	ldr	r1,	[r2, #HwDRAM_CFG]
	orr	r1,	r1, #HwDRAM_CFG_STOP_MCLK
	str	r1, [r2, #HwDRAM_CFG]

	//@sdram.cfg.stop_mclk = 1
	ldr	r2,	=HwDRAM_PL340_BASE
	ldr	r1,	[r2, #HwDRAM_CFG]
	orr	r1,	r1, #HwDRAM_CFG_STOP_MCLK
	str	r1, [r2, #HwDRAM_CFG]

	//@sdram.ckcdown.cd_sddr = 1
	ldr r2, =HwDRAM_MEMBUS_BASE
	ldr r1, [r2, #HwDRAM_MEMBUS_CKDOWN]
	orr r1, r1, #HwDRAM_MEMBUS_CKDOWN_SDDR
	str r1, [r2, #HwDRAM_MEMBUS_CKDOWN]

	//@Use XIN clock
	ldr r2, =HwCKC_BASE
	ldr r1, =((0x1<<21) + (0x0<<20) + (0xFFFF<<4) + 4)	@enable + normal mode + config(0xFFFF) + source(XIN) = XIN
	str r1, [r2, #HwCLKCTRL0]
	ldr r1, =((0x1<<21) + (0x0<<20) + (0x0001<<4) + 4)	@enable + normal mode + divisor(0x1) + source(XIN) = XIN/2
	str r1, [r2, #HwCLKCTRL1]
	ldr r1, =((0x1<<21) + (0x0<<20) + (0x0001<<4) + 4)	@enable + normal mode + divisor(0x1) + source(XIN) = XIN/2
	str r1, [r2, #HwCLKCTRL2]
	ldr r1, =((0x1<<21) + (0x0<<20) + (0x0001<<4) + 4)	@enable + normal mode + divisor(0x1) + source(XIN) = XIN/2
	str r1, [r2, #HwCLKCTRL3]
	ldr r1, =((0x1<<21) + (0x0<<20) + (0x0001<<4) + 4)	@enable + normal mode + divisor(0x1) + source(XIN) = XIN/2
	str r1, [r2, #HwCLKCTRL4]
	ldr r1, =((0x1<<21) + (0x0<<20) + (0x0001<<4) + 4)	@enable + normal mode + divisor(0x1) + source(XIN) = XIN/2
	str r1, [r2, #HwCLKCTRL5]
	ldr r1, =((0x1<<21) + (0x0<<20) + (0x0001<<4) + 4)	@enable + normal mode + divisor(0x1) + source(XIN) = XIN/2
	str r1, [r2, #HwCLKCTRL6]
	ldr r1, =((0x1<<21) + (0x0<<20) + (0x0001<<4) + 4)	@enable + normal mode + divisor(0x1) + source(XIN) = XIN/2
	str r1, [r2, #HwCLKCTRL7]
	nop
	nop

	//@PLL Off
	ldr r1, =0x00000000
	str r1, [r2, #HwPLL0CFG]
	str r1, [r2, #HwPLL1CFG]
	str r1, [r2, #HwPLL2CFG]
	str r1, [r2, #HwPLL3CFG]
	nop
	nop

	//@pmu.power_off
	ldr r2, =HwPMU_BASE
	ldr r1, [r2, #HwPMU_PWROFF]
	bic r1, r1, #HwPMU_PWROFF_MB
	orr r1, r1, #HwPMU_PWROFF_DAC
	orr r1, r1, #HwPMU_PWROFF_HD
	orr r1, r1, #HwPMU_PWROFF_RSVD1
	orr r1, r1, #HwPMU_PWROFF_UP
	orr r1, r1, #HwPMU_PWROFF_RSVD2
	str r1, [r2, #HwPMU_PWROFF]

	//@if argument == 1 ? self-refresh only : self-refresh + deep-powerdown
	//@mov r1, #0
	//@cmp r1, r0
	//@bxeq	lr

	//@------------------------------------------------------
	//@ DeepPowerDown Routine..
	//@------------------------------------------------------
	
	//@pmu.control.initr
	ldr r2, =HwPMU_BASE
	ldr r1, [r2, #HwPMU_CONTROL]

#ifdef CONFIG_SLEEP_WITH_ITCM
	orr r1, r1, #HwPMU_CONTROL_INITR @ 1 : for ITCM Booting
#else
	bic r1, r1, #HwPMU_CONTROL_INITR //@ 0 : for BOOT ROM Booting
#endif
	str r1, [r2, #HwPMU_CONTROL]

	//@pmu.control.dpdn = 1 : Set DeepPowerDown mode
	ldr r1, [r2, #HwPMU_CONTROL]

#ifdef CONFIG_SLEEP_WITH_ITCM
	orr r1, r1, #HwPMU_CONTROL_DPDN
#else
	bic r1, r1, #HwPMU_CONTROL_DPDN
#endif
	str r1, [r2, #HwPMU_CONTROL]

	//@pmu.control.lor = 1 : I/O Retention
	ldr r1, [r2, #HwPMU_CONTROL]
	orr r1, r1, #HwPMU_CONTROL_IOR
	str r1, [r2, #HwPMU_CONTROL]

	//@pmu.control.poff = 1 : Enter DeepPowerDown!!
	ldr r1, [r2, #HwPMU_CONTROL]
	orr r1, r1, #HwPMU_CONTROL_POFF
	str r1, [r2, #HwPMU_CONTROL]

loop06:
	b	loop06
	bx	lr				@mov	pc, lr

