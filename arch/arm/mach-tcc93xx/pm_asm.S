/*
 * linux/arch/arm/mach-tcc9300/pm_asm.S
 *
 * Author:  <linux@telechips.com>
 * Created: October, 2009
 * Description: LINUX POWER MANAGEMENT FUNCTIONS
 *
 * Copyright (C) 2008-2009 Telechips 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <linux/linkage.h>
#include <asm/ptrace.h>


#define Mode_USR		0x10
#define Mode_FIQ		0x11
#define Mode_IRQ		0x12
#define Mode_SVC		0x13
#define Mode_ABT		0x17
#define Mode_UND		0x1B
#define Mode_SYS		0x1F
#define Mode_MASK		0x1F
#define NOINT			0xC0
#define I_Bit			0x80
#define F_Bit			0x40

#define SYSCTL_SBZ_MASK		0xCC1A0000
#define SYSCTL_SBO_MASK		0x00000070
#define MMUTTB_SBZ_MASK		0x00001FE0		// for 8KB Boundary Size of TTB0

	.text

/*
 * IO_ARM_SaveREG
 * --------------
 *
 * Save ARM registers, MMU data and MODE data for suspend mode.
 *
 * input parameters:
 *	r0: SRAM_ADDR_STANDBY
 *	r1: TCC_REG
 *	r2: Awake_address
 */

ENTRY(IO_ARM_SaveREG)
	@ address of sdram_self_refresh() (r0)
	mov r12, r0

	@ Save register state
	stmdb sp!, {r4-r12}
	stmdb sp!, {lr}
	str r2, [r1, #0x4]				// save resume function address (virtual)
	mov r3, r1						// r1 (&p89reg) -> r3


@
@ Save MMU to DRAM
@

	@ CP15 System Control Register
	mrc p15, 0, r2, c1, c0, 0		// load r2 with System Control Register
	ldr r0, =SYSCTL_SBZ_MASK		// Should Be Zero Mask for System Control Register
	bic r2, r2, r0
	ldr r0, =SYSCTL_SBO_MASK		// Should Be One Mask for System Control Register
	orr r2, r2, r0
	str r2, [r3, #0x08]				// [SleepState_SYSCTL]

	@ CP15 TTB Register0
	mrc p15, 0, r2, c2, c0, 0		// load r2 with TTB Register0
	ldr r0, =MMUTTB_SBZ_MASK		// Should Be Zero Mask for TTB Register0
	bic r2, r2, r0
	str r2, [r3, #0x0C]				// [SleepState_MMUTTB0]

	@ CP15 TTB Register1
	mrc p15, 0, r2, c2, c0, 1		// load r2 with TTB Register1
//	ldr r0, =MMUTTB_SBZ_MASK		// Should Be Zero Mask for TTB Register1
//	bic r2, r2, r0
	str r2, [r3, #0x10]				// [SleepState_MMUTTB1]

	@ CP15 TTB Control Register
	mrc p15, 0, r2, c2, c0, 2		// load r2 with TTB Control Register
//	ldr r0, =MMUTTB_CTLSBZ_MASK		// Should Be Zero Mask for TTB Register0
//	bic r2, r2, r0
	str r2, [r3, #0x14]				// [SleepState_MMUTTBCTL]

	@ CP15 Domain Access Control Register
	mrc p15, 0, r2, c3, c0, 0		// load r2 with Domain Access Control Register
	str r2, [r3, #0x18]				// [SleepState_MMUDOMAIN]


@
@ Save CPU register to DRAM
@

	@ Supervisor mode CPU Register
	str sp, [r3, #0x1C]				// [SleepState_SVC_SP]
	mrs r2, spsr					// Status Register
	str r2, [r3, #0x20]				// [SleepState_SVC_SPSR]
	
	add r3, r1, #0x24

	@ FIQ mode CPU Registers
	mov r1, #Mode_FIQ | NOINT		// Enter FIQ mode, no interrupts
	msr cpsr, r1
	mrs r2, spsr					// Status Register
	stmia r3!, {r2, r8-r12, sp, lr}	// Store FIQ mode registers [SleepState_FIQ_SPSR~SleepState_FIQ_LR]

	@ Abort mode CPU Registers
	mov r1, #Mode_ABT | NOINT		// Enter ABT mode, no interrupts
	msr cpsr, r1
	mrs r0, spsr					// Status Register
	stmia r3!, {r0, sp, lr}			// Store ABT mode Registers [SleepState_ABT_SPSR~SleepState_ABT_LR]

	@ IRQ mode CPU Registers
	mov r1, #Mode_IRQ | NOINT		// Enter IRQ mode, no interrupts
	msr cpsr, r1
	mrs r0, spsr					// Status Register
	stmia r3!, {r0, sp, lr}			// Store the IRQ Mode Registers [SleepState_IRQ_SPSR~SleepState_IRQ_LR]

	@Undefined mode CPU Registers
	mov r1, #Mode_UND | NOINT		// Enter UND mode, no interrupts
	msr cpsr, r1
	mrs r0, spsr					// Status Register
	stmia r3!, {r0, sp, lr}			// Store the UND mode Registers [SleepState_UND_SPSR~SleepState_UND_LR]

	@ System(User) mode CPU Registers
	mov r1, #Mode_SYS | NOINT		// Enter SYS mode, no interrupts
	msr cpsr, r1
	stmia r3!, {sp, lr}				// Store the SYS mode Registers [SleepState_SYS_SP, SleepState_SYS_LR]

	@ Return to SVC mode
	mov r1, #Mode_SVC | NOINT		// Back to SVC mode, no interrupts
	msr cpsr, r1
	
	ldr r2, [sp]
	str r2, [r3], #4

	bl IO_ARM_CleanCACHE

	nop
	nop
	nop

#if 0
	ldr r0, =0x00050078
	mcr p15, 0, r0, c1, c0, 0		// Disable Cache/MMU Control
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
#endif

	@ jump to power-off code.
	mov pc, r12						// jump to self-refresh code
ENDPROC(IO_ARM_SaveREG)


/*
 * Awake_address
 * --------------
 *
 * Restore CPU Register from Sleep Data Area in DRAM
 * r3 : p89reg ioremap virtual address, r3 is assigned by IO_ARM_RestoreREG in bootloader
 */
ENTRY(Awake_address)
	@ FIQ mode CPU Registers
	mov r1, #Mode_FIQ | NOINT		// Enter FIQ mode, no interrupts
	msr cpsr, r1
	ldr r0,	[r3, #0x24]
	msr spsr, r0
	ldr r8,	[r3, #0x28]
	ldr r9,	[r3, #0x2C]
	ldr r10,[r3, #0x30]
	ldr r11,[r3, #0x34]
	ldr r12,[r3, #0x38]
	ldr sp,	[r3, #0x3C]
	ldr lr,	[r3, #0x40]

	@ Abort mode CPU Registers
	mov r1, #Mode_ABT | I_Bit		// Enter ABT mode, no IRQ - FIQ is available
	msr cpsr, r1
	ldr r0,	[r3, #0x44]
	msr spsr, r0
	ldr sp,	[r3, #0x48]
	ldr lr,	[r3, #0x4C]

	@ IRQ mode CPU Registers
	mov r1, #Mode_IRQ | I_Bit		// Enter IRQ mode, no IRQ - FIQ is available
	msr cpsr, r1
	ldr r0, [r3, #0x50]
	msr spsr, r0
	ldr sp, [r3, #0x54]
	ldr lr, [r3, #0x58]
		
	@ Undefined mode CPU Registers
	mov r1, #Mode_UND | I_Bit		// Enter UND mode, no IRQ - FIQ is available
	msr cpsr, r1
	ldr r0, [r3, #0x5C]
	msr spsr, r0
	ldr sp, [r3, #0x60]
	ldr lr, [r3, #0x64]

	@ System(User) mode CPU Registers
	mov r1, #Mode_SYS | I_Bit		// Enter SYS mode, no IRQ - FIQ is available
	msr cpsr, r1
	ldr sp, [r3, #0x68]
	ldr lr, [r3, #0x6C]
	
	@ Supervisor mode CPU Registers
	mov r1, #Mode_SVC | I_Bit		// Enter SVC mode, no IRQ - FIQ is available
	msr cpsr, r1
	ldr r0, [r3, #0x20]
	msr spsr, r0
	ldr sp, [r3, #0x1C]


	@ Pop SVC Register from our Stack
	ldr lr, [sp], #4
	ldmia sp!, {r4-r12}

	@ Return to PM functions
	mov pc, lr
ENDPROC(Awake_address)


/*
 * Clean & Flush Cache
 * -------------------
 */
ENTRY(IO_ARM_CleanCACHE)
	stmdb r13!, {lr}

	bl IO_ARM_CleanDCACHE

	cmp	r0, #0
	movne	r0, #0				@ Drain Write Buffer
	mcrne	p15, 0, r0, c7, c10, 4

	ldmia r13!, {pc}
ENDPROC(IO_ARM_CleanCACHE)


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
ENDPROC(IO_ARM_CleanDCACHE)
