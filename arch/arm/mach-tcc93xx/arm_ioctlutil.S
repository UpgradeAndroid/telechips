/***************************************************************************************
*	FileName    : arm_ioctlutil.s 
****************************************************************************************
*
*	TCC Board Support Package
*	Copyright (c) Telechips, Inc.
*	ALL RIGHTS RESERVED
*
****************************************************************************************/
    .global arm_changestack
arm_changestack:
	mov		r0, r13
//	ldr		r13, =0xF0A03FA0  			//	0x10000000 ~ 0x10003FFF SRAM(16KB)    
//	ldr		r13, =0xEFF03FFC    //	0x10000000 ~ 0x10003FFF SRAM(16KB)    
	ldr		r13, =0xEFF03FDC    //	0x10000000 ~ 0x10003FFF SRAM(16KB)    
    mov		pc, lr    
    
    .global arm_restorestack
arm_restorestack:
    mov		r13, r0    
    mov		pc, lr


/************* end of file *************************************************************/
