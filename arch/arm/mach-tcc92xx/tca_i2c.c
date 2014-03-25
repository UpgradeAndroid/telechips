/****************************************************************************
 *   FileName    : tca_i2c.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#include <linux/delay.h>
#include <mach/bsp.h>
#include <mach/tca_i2c.h>


/*****************************************************************************
* Function Name : tca_i2c_setgpio()
* Description: I2C port configuration
*              SCL0-GPA0, SDA0-GPA1
*              SCL1-GPA8, SDA1-GPA9
* input parameter:
*       int ch;   // I2C master channel
******************************************************************************/
void tca_i2c_setgpio(int core, int ch)
{
	volatile struct tcc_gpio *gpio;

	gpio = (volatile struct tcc_gpio *)GPA_BASE;

	if(core != 0)
		return ;

	switch (ch) {
	case 0:
		BITCSET(gpio->FN0, (Hw4-Hw0), Hw0);	// GPA[0] function set 1
		BITCSET(gpio->FN0, (Hw8-Hw4), Hw4);	// GPA[1] function set 1
		break;
	case 1:
		BITCSET(gpio->FN1, (Hw4-Hw0), Hw0);	// GPA[8] function set 1
		BITCSET(gpio->FN1, (Hw8-Hw4), Hw4);	// GPA[9] function set 1
		break;
	default:
		break;
	}
}

/* end of source */
