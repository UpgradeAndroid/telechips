#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>

#include <mach/mmc.h>
#include <mach/gpio.h>
#include <mach/bsp.h>
#include <asm/mach-types.h>

#include "devices.h"
#include "board-m57te.h"

#undef SD_EXTENTION_SUB  //MCC

struct mmc_port_config {
	int data0;
	int data1;
	int data2;
	int data3;
	int clk;
	int cmd;
	int cd;
};

static struct mmc_port_config mmc_ports[] = {
	[0] = {
		.data0	= TCC_GPF(0),
		.data1	= TCC_GPF(1),
		.data2	= TCC_GPF(2),
		.data3	= TCC_GPF(3),
		.cmd	= TCC_GPF(8),
		.clk	= TCC_GPF(9),
		.cd	= NULL,
	},
	[1] = {
		.data0	= TCC_GPF(21),
		.data1	= TCC_GPF(20),
		.data2	= TCC_GPF(19),
		.data3	= TCC_GPF(18),
		.cmd	= TCC_GPF(22),
		.clk	= TCC_GPF(23),
		.cd	= TCC_GPA(10),
	},
};


int m57te_mmc_init(struct device *dev, int id)
{
	BUG_ON(id > 1);
//	gpio_request(GPIO_SD0_ON, "sd0_power");
//	gpio_request(GPIO_SD1_ON, "sd1_power");

#if 1
	gpio_request(TCC_GPF(18), NULL);
	gpio_request(TCC_GPF(19), NULL);
	gpio_request(TCC_GPF(20), NULL);
	gpio_request(TCC_GPF(21), NULL);

	gpio_request(TCC_GPF(22), NULL);
	gpio_request(TCC_GPF(23), NULL);
	gpio_request(TCC_GPF(10), NULL);
	//gpio_request(TCC_GPA(6), NULL);
#endif

	tcc_gpio_config(mmc_ports[id].data0, GPIO_FN(2) | GPIO_CD(0));
	tcc_gpio_config(mmc_ports[id].data1, GPIO_FN(2) | GPIO_CD(0));
	tcc_gpio_config(mmc_ports[id].data2, GPIO_FN(2) | GPIO_CD(0));
	tcc_gpio_config(mmc_ports[id].data3, GPIO_FN(2) | GPIO_CD(0));
	tcc_gpio_config(mmc_ports[id].cmd, GPIO_FN(2));
	tcc_gpio_config(mmc_ports[id].clk, GPIO_FN(2) | GPIO_CD(3));

	if(id==1){
		tcc_gpio_config(mmc_ports[id].cd, GPIO_FN(0));
		gpio_request(mmc_ports[id].cd, "sd_cd");
		gpio_direction_input(mmc_ports[id].cd);
	}

	//gpio_direction_output(GPIO_SD0_ON, 0);
	//msleep(100);
	//gpio_direction_output(GPIO_SD0_ON, 1);
	//msleep(100);
	
#if 0	
	if(id == 0) {
		gpio_direction_output(GPIO_SD1_ON, 0);
		msleep(100);
		gpio_direction_output(GPIO_SD1_ON, 1);
		msleep(100);
	}
#endif

	return 0;
}

int m57te_mmc_card_detect(struct device *dev, int id)
{
	BUG_ON(id > 1);
	
	if(id==0)
		return 1;
	else
		return gpio_get_value(mmc_ports[id].cd) ? 0 : 1;
}

int m57te_mmc_suspend(struct device *dev, int id)
{
	struct tcc_mmc_platform_data *mmc = dev->platform_data;
	
	if(id==0)
		gpio_direction_output(TCC_GPF(10),0);
/*
	if(mmc->cd_int_num >0)
	{		
		writel( readl(&HwPIC->INTMSK1)&~(1<<mmc->cd_int_num) , &HwPIC->INTMSK1);
	}
*/
	return 0;
}

int m57te_mmc_resume(struct device *dev, int id)
{
	struct tcc_mmc_platform_data *mmc = dev->platform_data;

	if(id==0)
		gpio_direction_output(TCC_GPF(10),1);
/*
	if(mmc->cd_int_num >0)
	{		
		writel( readl(&HwPIC->INTMSK1)|(1<<mmc->cd_int_num) , &HwPIC->INTMSK1);
	}
*/
	return 0;
}

int m57te_mmc_set_power(struct device *dev, int id, int on)
{
	if (on) {
		/* power */
		if (id == 1) {
#if 0			
			gpio_direction_output(TCC_GPEXT1(6), 0);
			//tcc_pca953x_setup(PCA9539_U3_SLAVE_ADDR, SD0, OUTPUT, OFF, SET_DIRECTION|SET_VALUE);
			msleep(100);
			gpio_direction_output(TCC_GPEXT1(6), 1);
			//tcc_pca953x_setup(PCA9539_U3_SLAVE_ADDR, SD0, OUTPUT, ON, SET_DIRECTION|SET_VALUE);
#endif			
		}
		//mdelay(10);
	} else {
		if (id == 1) {
#if 0			
			gpio_direction_output(GPIO_SD0_ON, 0);

			/* slot0 port2 - GPIO mode */
			tcc_gpio_config(TCC_GPE(12), GPIO_FN(0));
			tcc_gpio_config(TCC_GPE(13), GPIO_FN(0));
			tcc_gpio_config(TCC_GPE(14), GPIO_FN(0));
			tcc_gpio_config(TCC_GPE(15), GPIO_FN(0));

			tcc_gpio_config(TCC_GPE(20), GPIO_FN(0));
			tcc_gpio_config(TCC_GPE(21), GPIO_FN(0));
			//tcc_gpio_config(TCC_GPA(6), GPIO_FN(0));

			/* output mode */
			gpio_direction_output(TCC_GPE(12), 1);
			gpio_direction_output(TCC_GPE(13), 1);
			gpio_direction_output(TCC_GPE(14), 1);
			gpio_direction_output(TCC_GPE(15), 1);

			gpio_direction_output(TCC_GPE(20), 1);
			gpio_direction_output(TCC_GPE(21), 1);
			//gpio_direction_output(TCC_GPA(6), 1);
#endif			
		}
		//mdelay(10);
	}
	return 0;
}

int m57te_mmc_set_bus_width(struct device *dev, int id, int width)
{
	return 0;
}

struct tcc_mmc_platform_data m57te_mmc_platform_data[] = {
	[0] = {
		.slot	= 3,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/* MMC_CAP_8_BIT_DATA */
			/*| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED*/,
		.f_min	= 100000,
#if defined(CONFIG_BCM4325_WIFI)
		.f_max	= 24000000;
#else
		.f_max	= 48000000,
#endif
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.cd_int_num =-1,
		.init	= m57te_mmc_init,
		.card_detect = m57te_mmc_card_detect,
		.suspend = m57te_mmc_suspend,
		.resume	= m57te_mmc_resume,
		.set_power = m57te_mmc_set_power,
		.set_bus_width = m57te_mmc_set_bus_width,
	},	
	[1] = {
		.slot	= 1,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/*| MMC_CAP_8_BIT_DATA */
				| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.cd_int_num =INT_EI8,		
		.init	= m57te_mmc_init,
		.card_detect = m57te_mmc_card_detect,
		.suspend = m57te_mmc_suspend,
		.resume	= m57te_mmc_resume,
		.set_power = m57te_mmc_set_power,
		.set_bus_width = m57te_mmc_set_bus_width,
	},
};

static int __init m57te_init_mmc(void)
{
	if (!machine_is_m57te())
		return 0;

#if defined(CONFIG_MMC_TCC_SDHC)
#if defined(CONFIG_MMC_TCC_SDHC_CORE0)
	tcc_mmc_core0_device.dev.platform_data = &m57te_mmc_platform_data[0];
	platform_device_register(&tcc_mmc_core0_device);
#endif
#if defined(CONFIG_MMC_TCC_SDHC_CORE1)
	tcc_mmc_core1_device.dev.platform_data = &m57te_mmc_platform_data[1];
	platform_device_register(&tcc_mmc_core1_device);
#endif
#endif
	return 0;
}
device_initcall(m57te_init_mmc);
