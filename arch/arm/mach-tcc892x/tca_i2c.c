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
	//PI2CPORTCFG i2c_portcfg = (PI2CPORTCFG)tcc_p2v(HwI2C_PORTCFG_BASE);

	switch (core)
	{
		case 0:
			#if defined(CONFIG_MACH_TCC8920ST)
				//I2C[26] - GPIOG[18][19]
				//i2c_portcfg->PCFG0.bREG.MASTER0 = 26;
				BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 26);
				tcc_gpio_config(TCC_GPG(18), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
				tcc_gpio_config(TCC_GPG(19), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);

			#elif defined(CONFIG_MACH_M805_892X)	
				//I2C[23] - GPIOG[6][7]		
				//i2c_portcfg->PCFG0.bREG.MASTER0 = 23;
				BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 23);
				tcc_gpio_config(TCC_GPG(6), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
				tcc_gpio_config(TCC_GPG(7), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
			#else
				//I2C[8] - GPIOB[9][8]
				//i2c_portcfg->PCFG0.bREG.MASTER0 = 8;
				BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x000000FF, 8);
				tcc_gpio_config(TCC_GPB(9), GPIO_FN11|GPIO_OUTPUT|GPIO_LOW);
				tcc_gpio_config(TCC_GPB(8), GPIO_FN11|GPIO_OUTPUT|GPIO_LOW);
			#endif
			break;
		case 1:
			#if defined(CONFIG_MACH_M805_892X)
			//I2C[24] - GPIOG[10][11]
			//i2c_portcfg->PCFG0.bREG.MASTER1 = 24;
			BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 24<<8);
			tcc_gpio_config(TCC_GPG(10), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
			tcc_gpio_config(TCC_GPG(11), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);

			#else
			//I2C[22] - GPIOG[2][3]
			//i2c_portcfg->PCFG0.bREG.MASTER1 = 22;
			BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x0000FF00, 22<<8);
			tcc_gpio_config(TCC_GPG(2), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
			tcc_gpio_config(TCC_GPG(3), GPIO_FN4|GPIO_OUTPUT|GPIO_LOW);
			#endif
			break;
		case 2:
			//I2C[28] - GPIO_ADC[2][3]
			//i2c_portcfg->PCFG0.bREG.MASTER2 = 28;
			BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0x00FF0000, 28<<16);
			tcc_gpio_config(TCC_GPADC(2), GPIO_FN3|GPIO_OUTPUT|GPIO_LOW);
			tcc_gpio_config(TCC_GPADC(3), GPIO_FN3|GPIO_OUTPUT|GPIO_LOW);
			break;
		case 3:
			//I2C[27] - GPIO_HDMI[2][3]
			//i2c_portcfg->PCFG0.bREG.MASTER3 = 27;
			BITCSET(((PI2CPORTCFG)io_p2v(HwI2C_PORTCFG_BASE))->PCFG0.nREG, 0xFF000000, 27<<24);
			tcc_gpio_config(TCC_GPHDMI(2), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
			tcc_gpio_config(TCC_GPHDMI(3), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
			//Not used..
			break;
		default:
			break;
	}
}

