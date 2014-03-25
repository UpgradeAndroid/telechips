/* 
 * linux/drivers/char/tcc_dxb/ctrl/tcc_dxb_control.c
 *
 * Author:  <linux@telechips.com>
 * Created: 10th Jun, 2008 
 * Description: Telechips Linux BACK-LIGHT DRIVER
 *
 * Copyright (c) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <asm/mach-types.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <mach/tcc_dxb_ctrl.h>
#include "board-tcc9300cm.h"

static unsigned int guiBoardType = BOARD_TDMB_TCC3150;

static void tcc_dxb_ctrl_power_off(int deviceIdx)
{
	if(machine_is_tcc9300cm())
	{
		if(guiBoardType == BOARD_DVBT_DIB9090 || guiBoardType == BOARD_DVBT_DIB7070)
		{
			if(guiBoardType == BOARD_DVBT_DIB9090 )
			{
				switch(deviceIdx)
				{
				case 0:
					tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
					gpio_request(GPIO_DXB1_PD, NULL);
					gpio_direction_output(GPIO_DXB1_PD, 0);
					break;
				default:
					break;
				}
			}
			else
			{
				switch(deviceIdx)
				{
				case 0:
					tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
					gpio_request(GPIO_DXB1_PD, NULL);
					gpio_direction_output(GPIO_DXB1_PD, 1);
					break;
				default:
					break;
				}
			}
	
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_RST,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_RST, NULL);
				gpio_direction_output(GPIO_DXB1_RST, 0);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DXB_NMI32X)
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_RST, 0);
				gpio_set_value(GPIO_DXB1_PD, 0);
				gpio_set_value(GPIO_DXB0_PD, 0);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_TDMB_TCC3150 )		
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_PD, 0);
				gpio_set_value(GPIO_DXB1_RST, 0);
				break;
			default:
				break;
			}
		}	
		else if(guiBoardType == BOARD_DXB_TCC3510 || guiBoardType == BOARD_TDMB_TCC3161)	
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_PD, NULL);
				gpio_direction_output(GPIO_DXB1_PD, 0);
				gpio_set_value(GPIO_DXB1_RST, 0);
				break;
			case 1:
				tcc_gpio_config(GPIO_DXB0_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB0_PD, NULL);
				gpio_direction_output(GPIO_DXB0_PD, 0);
				gpio_set_value(GPIO_DXB0_RST, 0);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_ISDBT_DIB10096 )	
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_PD, NULL);
				gpio_direction_output(GPIO_DXB1_PD, 0);
	
				tcc_gpio_config(GPIO_DXB0_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB0_PD, NULL);
				gpio_direction_output(GPIO_DXB0_PD, 0);
				break;
			default:
				break;
			}
		}	
		else if(guiBoardType == BOARD_DVBT_DIB9090M_PA)
		{
			switch(deviceIdx)
			{
			case 0:
				//Control DEEPPWDN this is only for tcc9300cm
				gpio_request(TCC_GPE(23), NULL);
				gpio_direction_output(TCC_GPE(23), 0);
				break;
			default:
				break;
			}
		}
		else if (guiBoardType == BOARD_ISDBT_MTV818 || guiBoardType == BOARD_ISDBT_TOSHIBA)
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_RST, 0);
				break;
			default:
				break;
			}
		}
	
		/* GPIO_EXPAND DXB_ON Power-off */
		gpio_request(TCC_GPEXT1(12 /*8*/),NULL);
		gpio_direction_output(TCC_GPEXT1(12 /*8*/), 0);
		/* GPIO_EXPAND DEEPPWDN Power-off */
		gpio_request(TCC_GPEXT2(12 /*8*/),NULL);
		gpio_direction_output(TCC_GPEXT2(12 /*8*/), 0);
	}
}

static void tcc_dxb_ctrl_power_on(int deviceIdx)
{
	if(machine_is_tcc9300cm())
	{
		/* GPIO_EXPAND DXB_ON Power-on */
		gpio_request(TCC_GPEXT1(12 /*8*/),NULL);
		gpio_direction_output(TCC_GPEXT1(12 /*8*/), 1);
	
		/* GPIO_EXPAND DEEPPWDN Power-on */
		gpio_request(TCC_GPEXT2(12 /*8*/),NULL);
		gpio_direction_output(TCC_GPEXT2(12 /*8*/), 1);
	
		if(guiBoardType == BOARD_DVBT_DIB9090 || guiBoardType == BOARD_DVBT_DIB7070)
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_RST,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_RST, NULL);
				gpio_direction_output(GPIO_DXB1_RST, 1);
				break;
			default:
				break;
			}
	
			if(guiBoardType == BOARD_DVBT_DIB9090)
			{
				switch(deviceIdx)
				{
				case 0:
					/* Power */
					tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
					gpio_request(GPIO_DXB1_PD, NULL);
					gpio_direction_output(GPIO_DXB1_PD, 1);
	
					/* RF Control */
					tcc_gpio_config(GPIO_DXB0_PD,  GPIO_FN(0));
					gpio_request(GPIO_DXB0_PD, NULL);
					gpio_direction_output(GPIO_DXB0_PD, 1);
					break;
				default:
					break;
				}
			}
			else
			{ 
				switch(deviceIdx)
				{
				case 0:
					tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
					gpio_request(GPIO_DXB1_PD, NULL);
					gpio_direction_output(GPIO_DXB1_PD, 0);
					break;
				default:
					break;
				}
			}
		}
		else if(guiBoardType == BOARD_DXB_NMI32X)
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_RST, 0);
				gpio_set_value(GPIO_DXB1_PD, 1);
				gpio_set_value(GPIO_DXB0_PD, 1);
	
				msleep (20);
	
				gpio_set_value(GPIO_DXB1_RST, 1);
				msleep (20);
				gpio_set_value(GPIO_DXB1_RST, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB1_RST, 1);
				msleep (20);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_TDMB_TCC3150 )	
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_PD, 1);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DXB_TCC3510 || guiBoardType == BOARD_TDMB_TCC3161)	
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_PD, GPIO_FN(0));
				tcc_gpio_config(GPIO_DXB1_RST, GPIO_FN(0));
	
				gpio_request(GPIO_DXB1_PD, NULL);
				gpio_direction_output(GPIO_DXB1_PD, 0);
				gpio_set_value(GPIO_DXB1_RST, 0);
				gpio_set_value(GPIO_DXB1_PD, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB1_PD, 1);
				msleep (20);
	
				gpio_request(GPIO_DXB1_RST, NULL);
				gpio_direction_output(GPIO_DXB1_RST, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB1_RST, 1);
				break;
			case 1:
				tcc_gpio_config(GPIO_DXB0_PD,  GPIO_FN(0));
				tcc_gpio_config(GPIO_DXB0_RST,  GPIO_FN(0));
	
				gpio_request(GPIO_DXB0_PD, NULL);
				gpio_direction_output(GPIO_DXB0_PD, 0);
	
				gpio_set_value(GPIO_DXB0_RST, 0);
				gpio_set_value(GPIO_DXB0_PD, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB0_PD, 1);
				msleep (20);
	
				gpio_request(GPIO_DXB0_RST, NULL);
				gpio_direction_output(GPIO_DXB0_RST, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB0_RST, 1);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_ISDBT_DIB10096 )	
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_RST,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_RST, NULL);
				gpio_direction_output(GPIO_DXB1_RST, 1);
	
				tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_PD, NULL);
				gpio_direction_output(GPIO_DXB1_PD, 1);
	
				tcc_gpio_config(GPIO_DXB0_PD,  GPIO_FN(0));
				gpio_request(GPIO_DXB0_PD, NULL);
				gpio_direction_output(GPIO_DXB0_PD, 1);
				break;
			default:
				break;
			}
	
		}	
		else if(guiBoardType == BOARD_DVBT_DIB9090M_PA)
		{
			switch(deviceIdx)
			{
			case 0:
				//Control DEEPPWDN this is only for tcc9300cm
				gpio_request(TCC_GPE(23), NULL);
				gpio_direction_output(TCC_GPE(23), 1);
				break;
			default:
				break;
			}
		}
		else if (guiBoardType == BOARD_ISDBT_MTV818 || guiBoardType == BOARD_ISDBT_TOSHIBA)
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_RST, 1);
				break;
			default:
				break;
			}
		}
	}
}

static void tcc_dxb_ctrl_power_reset(int deviceIdx)
{
	if(machine_is_tcc9300cm())
	{
		if(guiBoardType == BOARD_DVBT_DIB9090 || guiBoardType == BOARD_DVBT_DIB7070)
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_RST, 0);
				msleep(100);		
				gpio_set_value(GPIO_DXB1_RST, 1);
				msleep(100);		
				break;
			default:
				break;
			}
		}	
		else if(guiBoardType == BOARD_DXB_NMI32X)
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_RST, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB1_RST, 1);
				msleep (20);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_TDMB_TCC3150 )	
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_RST, 0);
				msleep(100);		
				gpio_set_value(GPIO_DXB1_RST, 1);
				msleep(100);		
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DXB_TCC3510 || guiBoardType == BOARD_TDMB_TCC3161)	
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_RST,  GPIO_FN(0));
				gpio_request(GPIO_DXB1_RST, NULL);
				gpio_direction_output(GPIO_DXB1_RST, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB1_RST, 1);
				break;
			case 1:
				tcc_gpio_config(GPIO_DXB0_RST,  GPIO_FN(0));
				gpio_request(GPIO_DXB0_RST, NULL);
				gpio_direction_output(GPIO_DXB0_RST, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB0_RST, 1);
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_ISDBT_DIB10096 )	
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_RST, 0);
				msleep (20);
				gpio_set_value(GPIO_DXB1_RST, 1);
				msleep (20);
				break;
			default:
				break;
			}
		}	
		else if(guiBoardType == BOARD_DVBT_DIB9090M_PA)
		{
			//Control TOTALPWDN
		}
		else if(guiBoardType == BOARD_ISDBT_TOSHIBA)
		{
			switch (deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_RST, 0);
				msleep(20);
				gpio_set_value(GPIO_DXB1_RST, 1);
				msleep(1);
				break;
			default:
				break;
			}
		}	
	}
}

static void tcc_dxb_ctrl_set_board(unsigned int uiboardtype)
{
	guiBoardType = uiboardtype;
	if(machine_is_tcc9300cm())
	{
		if(guiBoardType == BOARD_TDMB_TCC3150)
		{
			tcc_gpio_config(INT_DXB1_IRQ,  GPIO_FN(0));
			gpio_request(INT_DXB1_IRQ, NULL);
			gpio_direction_input(INT_DXB1_IRQ);
	
			tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
			gpio_request(GPIO_DXB1_PD, NULL);
			gpio_direction_output(GPIO_DXB1_PD, 0);
	
			tcc_gpio_config(GPIO_DXB1_RST,  GPIO_FN(0));
			gpio_request(GPIO_DXB1_RST, NULL);
			gpio_direction_output(GPIO_DXB1_RST, 0);
		}
		else if (guiBoardType == BOARD_DXB_NMI32X)
		{
			tcc_gpio_config(GPIO_DXB1_RST,  GPIO_FN(0));
			gpio_request(GPIO_DXB1_RST, NULL);
			gpio_direction_output(GPIO_DXB1_RST, 0);
	
			tcc_gpio_config(GPIO_DXB1_PD,  GPIO_FN(0));
			gpio_request(GPIO_DXB1_PD, NULL);
			gpio_direction_output(GPIO_DXB1_PD, 0);
	
			tcc_gpio_config(GPIO_DXB0_PD,  GPIO_FN(0));
			gpio_request(GPIO_DXB0_PD, NULL);
			gpio_direction_output(GPIO_DXB0_PD, 0);
		}
		else if (guiBoardType == BOARD_ISDBT_MTV818 || guiBoardType == BOARD_ISDBT_TOSHIBA)
		{
			tcc_gpio_config(GPIO_DXB1_RST, GPIO_FN(0));
			gpio_request(GPIO_DXB1_RST, NULL);
			gpio_direction_output(GPIO_DXB1_RST, 0);
		}
	}
}

static void tcc_dxb_ctrl_get_info(ST_CTRLINFO_ARG *pstCtrlInfo)
{
	if(machine_is_tcc9300cm())
	{
		if(guiBoardType == BOARD_DVBT_DIB9090M_PA)
			pstCtrlInfo->uiI2C = 4; //this is only for tcc9300cm
		else	
			pstCtrlInfo->uiI2C = 1;
	}
}

static void tcc_dxb_ctrl_rf_path(unsigned int flag)
{
}

static void tcc_dxb_init(void)
{
#if(0)
	if(machine_is_tcc9300cm())
	{
		/*PULL_UP is disabled to save current.*/
	
		//TCC_GPE(3)
		tcc_gpio_config(GPIO_DXB1_PD, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_PD, NULL);
		gpio_direction_output(GPIO_DXB1_PD, 0);
	
		//TCC_GPE(8)
		tcc_gpio_config(GPIO_DXB1_SFRM, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_SFRM, NULL);
		gpio_direction_output(GPIO_DXB1_SFRM, 0);
	
		//TCC_GPE(9)
		tcc_gpio_config(GPIO_DXB1_SCLK, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_SCLK, NULL);
		gpio_direction_output(GPIO_DXB1_SCLK, 0);
	
		//TCC_GPE(10)
		tcc_gpio_config(GPIO_DXB1_SDI, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_SDI, NULL);
		gpio_direction_output(GPIO_DXB1_SDI, 0);
	
		//TCC_GPE(11)
		tcc_gpio_config(GPIO_DXB1_RST, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_RST, NULL);
		gpio_direction_output(GPIO_DXB1_RST, 0);
	
		//TCC_GPA(11)
		tcc_gpio_config(INT_DXB1_IRQ, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(INT_DXB1_IRQ, NULL);
		gpio_direction_output(INT_DXB1_IRQ, 0);
	/*
		//TCC_GPD(12) or TCC_GPD(10)
		tcc_gpio_config(GPIO_DXB0_PD, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB0_PD, NULL);
		gpio_direction_output(GPIO_DXB0_PD, 0);
	
		//TCC_GPC(31) or TCC_GPD(5)
		tcc_gpio_config(GPIO_DXB0_SFRM, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB0_SFRM, NULL);
		gpio_direction_output(GPIO_DXB0_SFRM, 0);
	
		//TCC_GPC(30) or TCC_GPD(6)
		tcc_gpio_config(GPIO_DXB0_SCLK, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB0_SCLK, NULL);
		gpio_direction_output(GPIO_DXB0_SCLK, 0);
	
		//TCC_GPC(29) or TCC_GPD(7)
		tcc_gpio_config(GPIO_DXB0_SDI, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB0_SDI, NULL);
		gpio_direction_output(GPIO_DXB0_SDI, 0);
	
		//TCC_GPE(2) or TCC_GPD(8)
		tcc_gpio_config(GPIO_DXB0_RST, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB0_RST, NULL);
		gpio_direction_output(GPIO_DXB0_RST, 0);
	
		//TCC_GPF(26) or TCC_GPD(9)
		tcc_gpio_config(INT_DXB0_IRQ, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(INT_DXB0_IRQ, NULL);
		gpio_direction_output(INT_DXB0_IRQ, 0);
	*/
		//TCC_GPE(4)
		tcc_gpio_config(GPIO_DXB_UARTTX, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB_UARTTX, NULL);
		gpio_direction_output(GPIO_DXB_UARTTX, 0);
	
		//TCC_GPE(5)
		tcc_gpio_config(GPIO_DXB_UARTRX, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB_UARTRX, NULL);
		gpio_direction_output(GPIO_DXB_UARTRX, 0);
	}
#endif
}

static struct tcc_dxb_platform_data tcc_dxb_pdata = {
	.init		= tcc_dxb_init,
	.power_off	= tcc_dxb_ctrl_power_off,
	.power_on	= tcc_dxb_ctrl_power_on,
	.power_reset= tcc_dxb_ctrl_power_reset,
	.rf_path	= tcc_dxb_ctrl_rf_path,
	.set_board	= tcc_dxb_ctrl_set_board,
	.get_info	= tcc_dxb_ctrl_get_info,
};

static struct platform_device tcc_dxb_device = {
	.name	= "tcc_dxb_ctrl",
	.dev	= {
		.platform_data	= &tcc_dxb_pdata,
	},
};

static int __init tcc9300cm_init_tcc_dxb_ctrl(void)
{
	if(!machine_is_tcc9300cm())
		return 0;

	printk("%s (built %s %s)\n", __func__, __DATE__, __TIME__);
	platform_device_register(&tcc_dxb_device);
	return 0;
}
device_initcall(tcc9300cm_init_tcc_dxb_ctrl);
