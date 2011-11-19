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
#include <mach/gpio.h>


/*****************************************************************************
* Function Name : tca_i2c_setgpio(int ch)
* Description: I2C port configuration
* input parameter:
* 		int core; 	// I2C Core
*       int ch;   	// I2C master channel
******************************************************************************/
void tca_i2c_setgpio(int core, int ch)
{
	PI2CPORTCFG i2c_portcfg = (PI2CPORTCFG)tcc_p2v(HwI2C_PORTCFG_BASE);

	switch (core)
	{
		case 0:
			//I2C[8] - GPIOB[9][8]
			i2c_portcfg->PCFG0.bREG.MASTER0 = 8;
			tcc_gpio_config(TCC_GPB(9), GPIO_FN11|GPIO_OUTPUT|GPIO_LOW);
			tcc_gpio_config(TCC_GPB(8), GPIO_FN11|GPIO_OUTPUT|GPIO_LOW);
			break;
		case 1:
			//I2C[22] - GPIOG[2][3]
			i2c_portcfg->PCFG0.bREG.MASTER1 = 22;
			tcc_gpio_config(TCC_GPG(2), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
			tcc_gpio_config(TCC_GPG(3), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
			break;
		case 2:
			//I2C[28] - GPIO_ADC[2][3]
			i2c_portcfg->PCFG0.bREG.MASTER2 = 28;
			tcc_gpio_config(TCC_GPADC(2), GPIO_FN5|GPIO_OUTPUT|GPIO_LOW);
			tcc_gpio_config(TCC_GPADC(3), GPIO_FN5|GPIO_OUTPUT|GPIO_LOW);
			break;
		case 3:
			//Not used..
			break;
		default:
			break;
	}
}

