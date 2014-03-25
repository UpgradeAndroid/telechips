/************************************************************************
 *  TCC77x Digital Audio Player
 *  ------------------------------------------------
 *
 *  FUNCTION    : Various purpose routines.
 *  MODEL       : TCC77x DAP
 *  CPU NAME    : TCC77x
 *  SOURCE      : util.c
 *
 *  START DATE  : 2005 JAN. 14
 *  MODIFY DATE :
 *  DIVISION    : DEPT. SYSTEM FT 3 TEAM
 *              : TELECHIPS, INC.
 *
 *  HISTORY     : 2005.3.28. shyfool@telechips
 *                - modify IO_UTIL_CheckCodes() for debugging and new 
 *                 boot scheme.
 *
 *                2005.3.22. shyfool@telechips
 *                - change the initialization sequence of SDRAM controller 
 * 
 *                2005.3.18. vizirdo@telechips
 *                - add test environment for TCC77x board.
 *
 *                2005.3.17. shyfool@telechips
 *                - porting to TCC75x version 
 *                - add dbg_* functions for debugging.
 *
 *  $SFID: util.c,v 1.3 2005/03/29 02:02:02 shyfool Exp  $
 *
 ************************************************************************/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <mach/bsp.h>

#define ECID_ENABLE  0x80000000

// ECID Code
// -------- 31 ------------- 15 ----------- 0 --------
// [0]     |****************|****************|    : '*' is valid
// [1]     |0000000000000000|****************|    : 
//

void
IO_UTIL_ReadECID (void)
{
  unsigned i;	
  unsigned int CS, SIGDEV, FSET , PRCHG, PROG, MCK;
  unsigned gECID[2] = {0,0};

  gECID[0] = gECID[1] = 0;

  CS = 0;
  SIGDEV = 0;
  FSET = 0;
  PRCHG = 0;
  PROG = 0;
  MCK = 0;


	HwGPIO->ECID0  = 0xe0010000;
	HwGPIO->ECID0  = 0xe8010000;
	HwGPIO->ECID0  = 0xe0010000;

#if 0
	HwGPIO->ECID0  = 0xe0000000;
	HwGPIO->ECID0  = 0xe8000000;
	HwGPIO->ECID0  = 0xe0000000;
#else 

    //start---
    HwGPIO->ECID0 = (ECID_ENABLE | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23));



    for(i=0; i<100; i++);
    PRCHG = 1;
    HwGPIO->ECID0 = (ECID_ENABLE | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23));



    for(i=0; i<100; i++);
    CS = 1;
    HwGPIO->ECID0 = (ECID_ENABLE | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23)); 
    PRCHG = 0; 
    HwGPIO->ECID0 = (ECID_ENABLE | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23)); 
    SIGDEV = 1;
    HwGPIO->ECID0 = (ECID_ENABLE | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23));



    for(i=0; i<100; i++);
    FSET = 1;
    HwGPIO->ECID0 = (ECID_ENABLE | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23)); 



    for(i=0; i<100; i++);
    SIGDEV = 0;
    HwGPIO->ECID0 = (ECID_ENABLE | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23)); 



    for(i=0; i<100; i++);
    PRCHG = 1;
    HwGPIO->ECID0 = (ECID_ENABLE | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23)); 

    for(i=0; i<100; i++);
    FSET = 0;
    HwGPIO->ECID0 = (ECID_ENABLE | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23)); 



    for(i=0; i<100; i++);
    CS = 0;
    HwGPIO->ECID0 = (ECID_ENABLE | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23)); 

#endif
    gECID[0] = HwGPIO->ECID2;
    gECID[1] = HwGPIO->ECID3;

	//delay50ns(10);

	HwGPIO->ECID0  = 0x00000000;  // ECID Closed

#ifdef __DEBUG__
  printk("ECID DATA[0] : 0x%x\n",gECID[0]);
  printk("ECID DATA[1] : 0x%x\n",gECID[1]);
#endif

	system_serial_low = gECID[0]; 
	system_serial_high = gECID[1]; 

	printk("Serial Number: %08x%08x\n", system_serial_high, system_serial_low);
}


