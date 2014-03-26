/*
 * PWM Timer Count 
 *
 * Copyright (C) 2009, 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <mach/bsp.h>
#include <asm/mach-types.h>
#include <mach/gpio.h>
#include <mach/tca_tco.h>
#include <mach/io.h>

#include <mach/TCC92x_Physical.h>


#define	HwTCO_BASE_ADDR(X)		(((volatile void *)&HwTMR_BASE)+(X)*0x10)

PTIMERN IO_TCO_GetBaseAddr(unsigned uCH)
{
	return (volatile PTIMERN)(HwTCO_BASE_ADDR(uCH));
}

unsigned IO_TCO_GetGpioFunc(unsigned uCH, unsigned uGPIO)
{
	switch(uGPIO)
	{
		case TCC_GPA(4): 	// TCO0
		case TCC_GPB(5):	// TCO1
		case TCC_GPB(6):	// TCO2
		case TCC_GPB(7):	// TCO3
		case TCC_GPB(14):	// TCO4
		case TCC_GPB(15):	// TCO5
			return GPIO_FN(2);

		default:
			break;
	}
	return 0;
}


int tca_tco_pwm_ctrl(unsigned tco_ch, unsigned uGPIO, unsigned int max_cnt, unsigned int level_cnt)
{
	unsigned uFGPIO;
	volatile PTIMERN pTIMER = IO_TCO_GetBaseAddr(tco_ch);

//	printk("tco:%d, GPIO(G:%d, Num:%d) max:%d level:%d TCOaddr:0x%p \n", tco_ch, (uGPIO>>5), (uGPIO &0x1F), max_cnt, level_cnt, pTIMER);
	if(pTIMER == NULL)
		return -1;

	gpio_direction_output(uGPIO, 1);
 
	if(max_cnt <= level_cnt)	{
		tcc_gpio_config(uGPIO,  GPIO_FN(0));
		gpio_set_value(uGPIO, 1);
	}
	else if(level_cnt == 0) {
		tcc_gpio_config(uGPIO, GPIO_FN(0));
		gpio_set_value(uGPIO, 0);
	}
	else 	{
		pTIMER->TREF = max_cnt;
		pTIMER->TCFG	= 0x105;	
		pTIMER->TMREF = level_cnt;
		pTIMER->TCFG	= 0x105;
		uFGPIO = IO_TCO_GetGpioFunc(tco_ch, uGPIO);
		tcc_gpio_config(uGPIO,  uFGPIO);
	}
}

