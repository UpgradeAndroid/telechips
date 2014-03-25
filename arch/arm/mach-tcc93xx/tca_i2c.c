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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <mach/bsp.h>
#include <mach/tca_ckc.h>
#include <mach/tca_i2c.h>


/*****************************************************************************
* Function Name : tca_i2c_setgpio(int ch)
* Description: I2C port configuration
* input parameter:
* 		int core; 	// I2C Core
*       int ch;   	// I2C master channel
******************************************************************************/
void tca_i2c_setgpio(int core, int ch)
{
	PGPIO gpio = (PGPIO)tcc_p2v(HwGPIO_BASE);

	switch (core) {
		case 0:
		{
			if (ch == 0) {
				BITCSET(gpio->GPAFN0, (Hw8-Hw0), Hw4|Hw0);			/* GPIO_A[1:0] */
				BITSET(gpio->GPAEN, Hw1|Hw0);
				BITCLR(gpio->GPADAT, Hw1|Hw0);
			} else if (ch == 1) {
				#if defined(CONFIG_MACH_TCC9300CM) || defined(CONFIG_MACH_TCC9300ST)
                                BITCSET(gpio->GPAFN1, (Hw8-Hw0), Hw4|Hw0);          /* GPIO_A[9:8] */
				BITSET(gpio->GPAEN, Hw9|Hw8);
				BITCLR(gpio->GPADAT, Hw9|Hw8);
				#else
				BITCSET(gpio->GPEFN3, (Hw32-Hw24), Hw28|Hw24);		/* GPIO_E[31:30] */
				BITSET(gpio->GPEEN, Hw31|Hw30);
				BITCLR(gpio->GPEDAT, Hw31|Hw30);
				#endif
			}
			break;
		}
		case 1:
		{
			if (ch == 0) {
				#if defined(CONFIG_MACH_TCC9300) 
				BITCSET(gpio->GPAFN0, (Hw16-Hw8), Hw13|Hw9);		/* GPIO_A[3:2] */
				BITSET(gpio->GPAEN, Hw3|Hw2);
				BITCLR(gpio->GPADAT, Hw3|Hw2);
                		#elif defined(CONFIG_MACH_TCC9300ST)
				BITCSET(gpio->GPDFN2, (Hw32-Hw24), Hw30|Hw26); 	/* GPIO_D[23:22] */
				BITSET(gpio->GPDEN, Hw23|Hw22);
				BITCLR(gpio->GPDDAT, Hw23|Hw22);
				#elif defined(CONFIG_MACH_TCC9300CM)
				/* NC */
				#endif
			} else if (ch == 1) {
				BITCSET(gpio->GPAFN1, (Hw16-Hw8), Hw12|Hw8);		/* GPIO_A[11:10] */
				BITSET(gpio->GPAEN, Hw11|Hw10);
				BITCLR(gpio->GPADAT, Hw11|Hw10);
			}
			break;
		}
		case 2:
		{
			if (ch == 0) {
				BITCSET(gpio->GPAFN2, (Hw8-Hw0), Hw5|Hw4|Hw1|Hw0);	/* GPIO_A[17:16] */
				BITSET(gpio->GPAEN, Hw17|Hw16);
				BITCLR(gpio->GPADAT, Hw17|Hw16);
			} else if (ch == 1) {
				/* NC */
			}
			break;
		}
	}
}

