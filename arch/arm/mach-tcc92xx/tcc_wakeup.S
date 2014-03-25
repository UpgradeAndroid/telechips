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


#define uMark        0x00
#define uRestoreFUNC 0x04
#define pSave        0x08
#define MODE_USR_32	0x10
#define MODE_FIQ_32	0x11
#define MODE_IRQ_32	0x12
#define MODE_SVC_32	0x13
#define MODE_ABT_32	0x17
#define MODE_UND_32	0x1B
#define MODE_SYS_32	0x1F
#define MODE_SEC_32	0x16

#define MODE_BITS		0x1F


 #if 0
//-------------------------------------------------------------------------------
//
//	FUNCTION :
//
//	DESCRIPTION :
//
//------------------------------------------------------------------------------- 
ENTRY(IO_ARM_RestoreREG)
	@ Restore return address
	ldr	r14, [r0, #uRestoreFUNC]

	@ set r0 to pSave in sPStop structure (pSave is equal to uProcState in sPTemporary structure)
	ldr	r0, [r0, #pSave]

	mov	r1, r0
	add	r0, r0, #4

	@ restore SVC r13 & r4~r12 of system mode registers.
	ldmia	r0!, {r4-r13}

	@ restore System mode registers
	mrs	r3, cpsr
	orr	r3, r3, #MODE_SYS_32
	msr	CPSR_cxfs, r3
	ldmia	r0!, {r13-r14}

	@ restore Undef mode registers
	bic	r3, r3, #MODE_BITS
	orr	r3, r3, #MODE_UND_32
	msr	CPSR_cxfs, r3
	ldmia	r0!, {r13-r14}

	@ restore Abort mode registers
	bic	r3, r3, #MODE_BITS
	orr	r3, r3, #MODE_ABT_32
	msr	CPSR_cxfs, r3
	ldmia	r0!, {r13-r14}

	@ restore IRQ mode registers
	bic	r3, r3, #MODE_BITS
	orr	r3, r3, #MODE_IRQ_32
	msr	CPSR_cxfs, r3
	ldmia	r0!, {r13-r14}

	@ restore FIQ mode registers
	bic	r3, r3, #MODE_BITS
	orr	r3, r3, #MODE_FIQ_32
	msr	CPSR_cxfs, r3
	ldmia	r0!, {r8-r14}

	@ restore CPSR
	ldr	r3, [r1]
	msr	CPSR_cxfs, r3

	bx	lr				@mov	pc, lr
#endif

