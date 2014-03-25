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
#include "board-tcc9300.h"

static unsigned int guiBoardType = BOARD_TDMB_TCC3150;

static void tcc_dxb_ctrl_power_off(int deviceIdx)
{
	if(machine_is_tcc9300())
	{
		if(guiBoardType == BOARD_DVBT_DIB9090)
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_RST, GPIO_FN(0));
				gpio_request(GPIO_DXB1_RST, NULL);
				gpio_direction_output(GPIO_DXB1_RST, 0); 
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
				break;
			default:
				break;
			}
		}
		else if(guiBoardType == BOARD_DXB_TCC3510 || guiBoardType == BOARD_TDMB_TCC3161 )	
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_RST, GPIO_FN(0));
				gpio_request(GPIO_DXB1_RST, NULL);
				gpio_direction_output(GPIO_DXB1_RST, 0); 
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
				tcc_gpio_config(GPIO_DXB1_RST, GPIO_FN(0));
				gpio_request(GPIO_DXB1_RST, NULL);
				gpio_direction_output(GPIO_DXB1_RST, 0); 
				break;
			default:
				break;
			}
		}
	}
}

static void tcc_dxb_ctrl_power_on(int deviceIdx)
{
	if(machine_is_tcc9300())
	{
		if(guiBoardType == BOARD_DVBT_DIB9090)
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(INT_DXB1_IRQ, GPIO_FN(0));
				gpio_request(INT_DXB1_IRQ, NULL);
				gpio_direction_output(INT_DXB1_IRQ, 0); 
	
				tcc_gpio_config(GPIO_DXB1_RST, GPIO_FN(0));
				gpio_request(GPIO_DXB1_RST, NULL);
				gpio_direction_output(GPIO_DXB1_RST, 0); 
	
				/* RF Control //don't support 7070 */
				tcc_gpio_config(GPIO_DXB1_SDO, GPIO_FN(0));
				gpio_request(GPIO_DXB1_SDO, NULL);
				gpio_direction_output(GPIO_DXB1_SDO, 1); 
				break;
			default:
				break;
			}
		}	
		else if(guiBoardType == BOARD_TDMB_TCC3150 )	
		{
			//don't need control
		}
		else if(guiBoardType == BOARD_DXB_TCC3510 || guiBoardType == BOARD_TDMB_TCC3161)	
		{
			switch(deviceIdx)
			{
			case 0:
				tcc_gpio_config(GPIO_DXB1_RST, GPIO_FN(0));
				gpio_request(GPIO_DXB1_RST, NULL);
				gpio_direction_output(GPIO_DXB1_RST, 0); 
				msleep (20);
				gpio_set_value(GPIO_DXB1_RST, 1);
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
				tcc_gpio_config(INT_DXB1_IRQ, GPIO_FN(0));
				gpio_request(INT_DXB1_IRQ, NULL);
				gpio_direction_output(INT_DXB1_IRQ, 0); 
	
				tcc_gpio_config(GPIO_DXB1_RST, GPIO_FN(0));
				gpio_request(GPIO_DXB1_RST, NULL);
				gpio_direction_output(GPIO_DXB1_RST, 1); 
				break;
			default:
				break;
			}
		}
	}
}

static void tcc_dxb_ctrl_power_reset(int deviceIdx)
{
	if(machine_is_tcc9300())
	{
		if(guiBoardType == BOARD_DVBT_DIB9090)
		{
			switch(deviceIdx)
			{
			case 0:
				gpio_set_value(GPIO_DXB1_RST ,0);
				msleep(100);		
				gpio_set_value(GPIO_DXB1_RST ,1);
				msleep(100);
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
				gpio_set_value(GPIO_DXB1_RST ,0);
				msleep(100);		
				gpio_set_value(GPIO_DXB1_RST ,1);
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
				gpio_direction_output(GPIO_DXB1_RST ,0);
				msleep(20);		
				gpio_set_value(GPIO_DXB1_RST ,1);
				break;
			case 1:
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
				gpio_set_value(GPIO_DXB1_RST ,0);
				msleep(100);		
				gpio_set_value(GPIO_DXB1_RST ,1);
				msleep(100);
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
	if(machine_is_tcc9300())
	{
		if(guiBoardType == BOARD_TDMB_TCC3150)
		{
			tcc_gpio_config(INT_DXB1_IRQ, GPIO_FN(0));
			gpio_request(INT_DXB1_IRQ, NULL);
			gpio_direction_input(INT_DXB1_IRQ);
	
			tcc_gpio_config(GPIO_DXB1_RST, GPIO_FN(0));
			gpio_request(GPIO_DXB1_RST, NULL);
			gpio_direction_output(GPIO_DXB1_RST, 0);
		}
	}
}

static void tcc_dxb_ctrl_get_info(ST_CTRLINFO_ARG *pstCtrlInfo)
{
	if(machine_is_tcc9300())
	{
		pstCtrlInfo->uiI2C = 4;
	}
}

static void tcc_dxb_ctrl_rf_path(unsigned int flag)
{
}

static void tcc_dxb_init(void)
{
	if(machine_is_tcc9300())
	{
		//TCC_GPE(8)
		tcc_gpio_config(GPIO_DXB2_SFRM, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB2_SFRM, NULL);
		gpio_direction_output(GPIO_DXB2_SFRM, 0);
	
		//TCC_GPE(9)
		tcc_gpio_config(GPIO_DXB2_SCLK, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB2_SCLK, NULL);
		gpio_direction_output(GPIO_DXB2_SCLK, 0);
	
		//TCC_GPB(26)
		tcc_gpio_config(GPIO_DXB2_RST, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB2_RST, NULL);
		gpio_direction_output(GPIO_DXB2_RST, 0);
	
		//TCC_GPG(11)
		tcc_gpio_config(INT_DXB2_IRQ, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(INT_DXB2_IRQ, NULL);
		gpio_direction_output(INT_DXB2_IRQ, 0);
	
		//TCC_GPE(10)
		tcc_gpio_config(GPIO_DXB2_SDI, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB2_SDI, NULL);
		gpio_direction_output(GPIO_DXB2_SDI, 0);
	
		//TCC_GPE(11)
		tcc_gpio_config(GPIO_DXB2_SDO, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB2_SDO, NULL);
		gpio_direction_output(GPIO_DXB2_SDO, 0);
	
		//TCC_GPC(31)
		tcc_gpio_config(GPIO_DXB1_SFRM, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_SFRM, NULL);
		gpio_direction_output(GPIO_DXB1_SFRM, 0);
	
		//TCC_GPC(30)
		tcc_gpio_config(GPIO_DXB1_SCLK, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_SCLK, NULL);
		gpio_direction_output(GPIO_DXB1_SCLK, 0);
	
		//TCC_GPC(29)
		tcc_gpio_config(GPIO_DXB1_SDI, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_SDI, NULL);
		gpio_direction_output(GPIO_DXB1_SDI, 0);
		
		//TCC_GPC(28)
		tcc_gpio_config(GPIO_DXB1_SDO, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_SDO, NULL);
		gpio_direction_output(GPIO_DXB1_SDO, 0);
	
		//TCC_GPG(10)
		tcc_gpio_config(GPIO_DXB1_RST, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(GPIO_DXB1_RST, NULL);
		gpio_direction_output(GPIO_DXB1_RST, 0);
	
		//TCC_GPG(12)
		tcc_gpio_config(INT_DXB1_IRQ, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(INT_DXB1_IRQ, NULL);
		gpio_direction_output(INT_DXB1_IRQ, 0);
	}
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

static int __init tcc9300_init_tcc_dxb_ctrl(void)
{
	if(!machine_is_tcc9300())
		return 0;

	printk("%s (built %s %s)\n", __func__, __DATE__, __TIME__);
	platform_device_register(&tcc_dxb_device);
	return 0;
}
device_initcall(tcc9300_init_tcc_dxb_ctrl);
