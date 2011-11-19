/*
 * IR driver for remote controller : tca_remocon.c
 *
 * Copyright (C) 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <asm/io.h>
#include <mach/bsp.h>
#include <asm/mach-types.h>
#include "tcc_remocon.h"

//#define USE_PBUS_CLOCK

//======================================================
// REMOCON initialize
//======================================================
void    RemoconCommandOpen (void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	BITCLR(pRcu->CMD, Hw0);
	BITSET(pRcu->CMD, Hw1|Hw2|(20*Hw3)|Hw12|Hw13|Hw14);
}

void    RemoconCommandReset (void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);
	
	if(machine_is_tcc8900() || machine_is_tcc9200() || machine_is_tcc9201() || machine_is_tcc9200s()) {
		pRcu->CMD = Hw1|Hw2|(14*Hw3)|Hw13|Hw14;
	}
	else {
#ifdef USE_PBUS_CLOCK
		pRcu->CMD = Hw1|Hw2|(14*Hw3)|Hw13|Hw14;
#else
		BITCLR(pRcu->CMD, Hw0);
		BITCLR(pRcu->CMD, Hw12);
		BITSET(pRcu->CMD, Hw1|Hw2|(20*Hw3)|Hw13|Hw14);
#endif
	}
}

//======================================================
// configure address
//======================================================
void    RemoconConfigure (void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	if(machine_is_tcc8900() || machine_is_tcc9200() || machine_is_tcc9201() || machine_is_tcc9200s()) {
		BITSET(pRcu->INPOL, Hw1);             // polarity enable
	}
	else {
#ifdef USE_PBUS_CLOCK
		BITCLR(pRcu->INPOL, Hw8);
#else
		BITSET(pRcu->INPOL, Hw8);
#endif
		BITSET(pRcu->INPOL, Hw0|Hw9|Hw10|Hw11);
		//BITCLR(pRcu->INPOL, (Hw9|Hw10|Hw11));

		tcc_remocon_set_io();
	}
}

//======================================================
// REMOCON STATUS : irq enacle and fifo overflow (active low)
//======================================================
void    RemoconStatus (void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	BITCSET(pRcu->STA, 0x0, Hw12);
}

//======================================================
// REMOCON DIVIDE enable & ir clk & end count setting
// (end count use remocon clk)
//======================================================
void    RemoconDivide (void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	if(machine_is_tcc8900() || machine_is_tcc9200() || machine_is_tcc9201() || machine_is_tcc9200s()) {
		pRcu->CLKDIV = ((500*24)*Hw14)|(1*Hw0);
	}
	else{
#ifdef USE_PBUS_CLOCK
		pRcu->CLKDIV = ((500*36)*Hw14)|(1*Hw0);
#else
		pRcu->CLKDIV = (1*Hw0);
#endif
	}

	pRcu->CLKDIV = BITCSET(pRcu->CLKDIV, 0x000000FF, 100);	// Set END_CNT as 100
}

//======================================================
// REMOCON diable 
//======================================================
void    DisableRemocon (void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	BITSET(pRcu->CMD, Hw0);
	BITCLR(pRcu->CMD, Hw1|Hw2|Hw13|Hw14);
}

//======================================================
// REMOCON functions
//======================================================
void    RemoconInit (void)
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	RemoconDivide();
	RemoconIrqClear();
	RemoconIntClear();

	pRcu->CLKDIV = BITCSET(pRcu->CLKDIV, 0x000000FF, 100);	// Set END_CNT as 100
}

void    RemoconIntClear ()
{
	PREMOTECON      pRcu = (volatile PREMOTECON)tcc_p2v(HwREMOTE_BASE);

	BITCLR(pRcu->CMD, Hw2);
	BITSET(pRcu->CMD, Hw0);

	udelay(120);

	BITSET(pRcu->CMD, Hw2);
	BITCLR(pRcu->CMD, Hw0);
}

void    RemoconIntWait ()
{
	PPIC pPic = (volatile PPIC)tcc_p2v(HwPIC_BASE);
	uint rx_data;

	while(1)
	{
		rx_data = pPic->STS1;
		if((rx_data&0x400) == 0x400)
			break;
	}
	pPic->CLR1 = 0x400;     //RMT bit set to 1 which means rmt interrupt clear      
}

void    RemoconIrqClear ()
{
	uint idx;
	PPIC pPic = (volatile PPIC)tcc_p2v(HwPIC_BASE);

	for(idx=1; idx<3000; idx ++)
		pPic->CLR1 = 0x400;                     // Clear VIC Interrupt Clear Register 
}

