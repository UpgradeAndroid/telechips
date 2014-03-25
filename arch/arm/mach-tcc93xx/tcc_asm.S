/*
 * linux/arch/arm/plat-tcc/tcc_asm.S
 *
 * Author:  <linux@telechips.com>
 * Created: April 21, 2008
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
//#include <asm/hardware.h>
#include <mach/hardware.h>
#include <asm/ptrace.h>

#define uMark        	0x00
#define uRestoreFUNC 	0x04
#define pSave        	0x08
#define MODE_USR_32	0x10
#define MODE_FIQ_32	0x11
#define MODE_IRQ_32	0x12
#define MODE_SVC_32	0x13
#define MODE_ABT_32	0x17
#define MODE_UND_32	0x1B
#define MODE_SYS_32	0x1F
#define MODE_SEC_32	0x16
#define MODE_BITS		0x1F


#define SYSCTL_SBZ_MASK		0xCC1A0000
#define SYSCTL_SBO_MASK		0x00000070
#define MMUTTB_SBZ_MASK		0x00001FE0		// for 8KB Boundary Size of TTB0

//---------------------------
// CPSR Mode Bit Definition
//---------------------------
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
	.text

//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//-------------------------------------------------------------------------------
ENTRY(IO_ARM_SetCPSR)
	//EXPORT	IO_ARM_SetCPSR
	//EXPORT	IO_ARM_SetINT
//IO_ARM_SetCPSR
//IO_ARM_SetINT
	msr	cpsr_cf, r0
	bx	lr				@mov	pc, lr

//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//-------------------------------------------------------------------------------
ENTRY(IO_ARM_GetCPSR)
	//EXPORT	IO_ARM_GetCPSR
	//EXPORT	IO_ARM_GetINT
//IO_ARM_GetCPSR
//IO_ARM_GetINT
	mrs	r0, cpsr
	bx	lr				@mov	pc, lr


//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//-------------------------------------------------------------------------------
ENTRY(IO_ARM_ChangeStackSRAM)

	mov		r0, r13
//	ldr		r13, =0xF0803FA0  			@	0x10000000 ~ 0x10003FFF SRAM(16KB)    
//	ldr		r13, =0xEFF03FA0  			@	0x10000000 ~ 0x10003FFF SRAM(16KB)    
	ldr		r13, =0xEFE00600
	mov		pc, lr    


//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//-------------------------------------------------------------------------------  
ENTRY(IO_ARM_RestoreStackSRAM)

    mov		r13, r0    
    mov		pc, lr

    
//-------------------------------------------------------------------------------  
//	r0 : shut-down func phy address
//  r1 : mmu & cpu_reg backup area vir address
//  r2 : restore func vir address
//------------------------------------------------------------------------------- 
ENTRY(save_cpu_reg)

	//; shut-down func address
	mov	r12, r0	 

	//; save svc_reg (r4-r12,lr)
	stmdb	sp!, {r4-r12}
	stmdb	sp!, {lr}	

	//; mark shut down mode
	ldr     r3, =0x18C818C8
	str     r3, [r1, #0x00]

	//; save restore func address
	str     r2, [r1, #0x04]
	mov		r3, r1

	//; Save MMU & CPU Registers to RAM.
	mrc		p15, 0, r2, c1, c0, 0		// load r2 with System Control Register
	ldr		r0, =SYSCTL_SBZ_MASK		// Should Be Zero Mask for System Control Register
	bic		r2, r2, r0
	ldr		r0, =SYSCTL_SBO_MASK		// Should Be One Mask for System Control Register
	orr		r2, r2, r0
	str		r2, [r3, #0x08]				// [SleepState_SYSCTL]

	//; CP15 Translation Table Base Register0
	mrc     p15, 0, r2, c2, c0, 0		// load r2 with TTB Register0
	ldr		r0, =MMUTTB_SBZ_MASK		// Should Be Zero Mask for TTB Register0
	bic		r2, r2, r0
	str		r2, [r3, #0x0C]				// [SleepState_MMUTTB0]

	//; CP15 Translation Table Base Register1
	mrc		p15, 0, r2, c2, c0, 1		// load r2 with TTB Register1
	//ldr		r0, =MMUTTB_SBZ_MASK	// Should Be Zero Mask for TTB Register1
	//bic		r2, r2, r0
	str		r2, [r3, #0x10]				// [SleepState_MMUTTB1]

	//; CP15 Translation Table Base Control Register
	mrc     p15, 0, r2, c2, c0, 2		// load r2 with TTB Control Register
	//ldr		r0, =MMUTTB_CTLSBZ_MASK	// Should Be Zero Mask for TTB Register0
	//bic		r2, r2, r0
	str		r2, [r3, #0x14]				// [SleepState_MMUTTBCTL]

	//; CP15 Domain Access Control Register
	mrc		p15, 0, r2, c3, c0, 0		// load r2 with Domain Access Control Register
	str		r2, [r3, #0x18]				// [SleepState_MMUDOMAIN]

	//; Supervisor mode CPU Register
	str		sp, [r3, #0x1C]				// [SleepState_SVC_SP]
	mrs		r2, spsr					// Status Register
	str		r2, [r3, #0x20]				// [SleepState_SVC_SPSR]
	
	add 	r3, r1, #0x00000024			

	//; FIQ mode CPU Registers
	mov		r1, #Mode_FIQ | NOINT		// Enter FIQ mode, no interrupts
	msr		cpsr, r1
	mrs		r2, spsr					// Status Register
	stmia	r3!, {r2, r8-r12, sp, lr}		// Store FIQ mode registers [SleepState_FIQ_SPSR~SleepState_FIQ_LR]

	//; Abort mode CPU Registers
	mov		r1, #Mode_ABT | NOINT		// Enter ABT mode, no interrupts
	msr		cpsr, r1
	mrs		r0, spsr					// Status Register
	stmia	r3!, {r0, sp, lr}			// Store ABT mode Registers [SleepState_ABT_SPSR~SleepState_ABT_LR]


	//; IRQ mode CPU Registers
	mov		r1, #Mode_IRQ | NOINT		// Enter IRQ mode, no interrupts
	msr		cpsr, r1
	mrs		r0, spsr					// Status Register
	stmia	r3!, {r0, sp, lr}			// Store the IRQ Mode Registers [SleepState_IRQ_SPSR~SleepState_IRQ_LR]
	
	//; Undefined mode CPU Registers
	mov		r1, #Mode_UND | NOINT		// Enter UND mode, no interrupts
	msr		cpsr, r1
	mrs		r0, spsr					// Status Register
	stmia	r3!, {r0, sp, lr}			// Store the UND mode Registers [SleepState_UND_SPSR~SleepState_UND_LR]

	//; System(User) mode CPU Registers
	mov		r1, #Mode_SYS | NOINT		// Enter SYS mode, no interrupts
	msr		cpsr, r1
	stmia	r3!, {sp, lr}				// Store the SYS mode Registers [SleepState_SYS_SP, SleepState_SYS_LR]

	//; Return to SVC mode
	mov		r1, #Mode_SVC | NOINT		// Back to SVC mode, no interrupts
	msr		cpsr, r1
	
	ldr		r2, [sp]
	str		r2, [r3], #4	
	
	mov		r0, #0x1
	bl IO_ARM_CleanCACHE 

	mov		r0, #0x0
	mcr		p15, 0, r0, c8, c7, 0

	//ldr	sp, =0xEFE00600  //; stack : logical addr
	ldr	sp, =0xD0000600      //; stack : physical addr

	ldr r0, =0x00050078
	mcr	p15, 0, r0, c1, c0, 0			//; Disable Cache/MMU Control

	//; jump to Shut-down function.
	mov     pc, r12

//Bruce, 20101022, shut_down + backup_ram_boot
/*----------------------------------------------------------------------
 * Backup RAM Booting
 *--------------------------------------------------------------------*/
.globl BackupRAM_Boot
BackupRAM_Boot:

	ldr	pc,	=0xD8000004

	//; Mask IRQ/FIQ
	mrs	r0, cpsr
	orr	r0, r0, #0xC0
	msr	cpsr_cf, r0

	//; Set sp(stack point)
	ldr r13, =0x10010000

	//; Set Peripheral Port
	ldr r0, =0xB0000013	//;0xB0000000(Address) + 0x13(256MB)
 	mcr	p15, 0, r0, c15, c2, 4;

	//; REMAP - Internal SRAM
	ldr	r0, =0xB0503000
	ldr	r1, [r0, #0x14]
	bic	r1, r1, #0x700
	orr	r1, r1, #0x100
	str	r1, [r0, #0x14]

	//; CONFIG1.BBEN : Backup RAM Boot Enable //;*((volatile unsigned long *)0xB0503014) |= Hw12;
	bic	r1, r1, #0x1000
	str	r1, [r0, #0x14]

	//; Cache/TLB disable & Invalidate
	ldr	r0, =0x00050078
	mcr	p15, 0, r0, c1, c0, 0		// Disable Cache/MMU Control
	mov	r0, #0x0
	mcr	p15, 0, r0, c7, c7, 0		// Invalidate Both Caches.
	mcr	p15, 0, r0, c8, c7, 0		// Invalidate TLB

	//; I/O restore suspend_wakeup
	mov		lr, pc
	ldr		pc, =0xD8000400 //IO_RESTORE_FUNC_ADDR, io_restore()

	//; SDRAM Init - Jump InitRoutine_Start for SDRAM Initialize
	mov		lr, pc
	ldr		pc, =0xD8000600 //SDRAM_INIT_FUNC_PHY, sdram_init()

	//; Check whether Shutdown is enabled or not.
	//; cpu_reg/mmu data backup address
	ldr		r10, =0xD8000E00 // REG_MMU_DATA_PHY
	ldr		r1, [r10]
	ldr		r2, =0x18C818C8 // "TFNB" : BSP_SUSPEND_MASK
	cmp		r1, r2
	ldr r1, =0x0
	str r1, [r10]

resume_err_loop:
	bne resume_err_loop

	ldr		r6, [r10, #0x18]	// Domain Access Control Register
	ldr		r5, [r10, #0x14]	// TTB Control Register
	ldr		r4, [r10, #0x10]	// TTB Register1
	ldr		r3, [r10, #0x0C]	// TTB Register0
	ldr		r2, [r10, #0x08]	// System Control Register
	ldr		r1, [r10, #0x04]	// Return Address

	mov		r0, #0
	mcr		p15, 0, r0, c7, c5, 0	// Invalidate Entire Instruction Cache
	mcr		p15, 0, r0, c7, c14, 0	// Clean and Invalidate Entire Data Cache

	mrc		p15, 0, r0, c1, c0, 0
	bic		r0, r0, #0x0005		// Disable MMU and Data Cache
	mcr		p15, 0, r0, c1, c0, 0

	mov		r0, #0
	mcr		p15, 0, r0, c8, c7, 0	// Flush the TLB

	mcr		p15, 0, r6, c3, c0, 0	// Restore Domain Access Control Register
	mcr		p15, 0, r5, c2, c0, 2	// Restore TTB Control Register
	mcr		p15, 0, r4, c2, c0, 1	// Restore TTB Register1
	mcr		p15, 0, r3, c2, c0, 0	// Restore TTB Register0

	mcr		p15, 0, r2, c1, c0, 0	// Restore System Control Register (MMU Control)

	nop

	//; Return to WakeUp_Address
	mov		pc, r1			//; Jump to Virtual Return Address

//---------------------------------------------------------------------
ENTRY(resore_cpu_reg)

	//; cpu_reg/mmu data backup address
	ldr r3, =0xEFC00E00 //; REG_MMU_DATA_ADDR

	//; FIQ mode CPU Registers	
	mov		r1, #Mode_FIQ | NOINT	// Enter FIQ mode, no interrupts
	msr		cpsr, r1
	ldr		r0,	[r3, #0x24]
	msr		spsr, r0
	ldr		r8,	[r3, #0x28]
	ldr		r9,	[r3, #0x2C]
	ldr		r10,[r3, #0x30]
	ldr		r11,[r3, #0x34]
	ldr		r12,[r3, #0x38]
	ldr		sp,	[r3, #0x3C]
	ldr		lr,	[r3, #0x40]

	//; Abort mode CPU Registers
	mov		r1, #Mode_ABT | I_Bit	// Enter ABT mode, no IRQ - FIQ is available
	msr		cpsr, r1

	ldr		r0,	[r3, #0x44]
	msr		spsr, r0
	ldr		sp,	[r3, #0x48]
	ldr		lr,	[r3, #0x4C]

	//; IRQ mode CPU Registers
	mov		r1, #Mode_IRQ | I_Bit	// Enter IRQ mode, no IRQ - FIQ is available
	msr		cpsr, r1

	ldr		r0,	[r3, #0x50]
	msr		spsr, r0
	ldr		sp,	[r3, #0x54]
	ldr		lr,	[r3, #0x58]
		
	//; Undefined mode CPU Registers
	mov		r1, #Mode_UND | I_Bit	// Enter UND mode, no IRQ - FIQ is available
	msr		cpsr, r1

	ldr		r0,	[r3, #0x5C]
	msr		spsr, r0
	ldr		sp,	[r3, #0x60]
	ldr		lr,	[r3, #0x64]

	//; System(User) mode CPU Registers
	mov		r1, #Mode_SYS | I_Bit	// Enter SYS mode, no IRQ - FIQ is available
	msr		cpsr, r1

	ldr		sp,	[r3, #0x68]
	ldr		lr,	[r3, #0x6C]
	
	//; Supervisor mode CPU Registers
	mov		r1, #Mode_SVC | I_Bit	// Enter SVC mode, no IRQ - FIQ is available
	msr		cpsr, r1

	ldr		r0, [r3, #0x20]
	msr		spsr, r0
	ldr		sp, [r3, #0x1C]

	//;	2. Pop SVC Register from our Stack
	ldr		lr, [sp], #4
	ldmia	sp!, {r4-r12}

	//;	3. Return to Caller of GWESPowerOff()
	mov     pc, lr                         


