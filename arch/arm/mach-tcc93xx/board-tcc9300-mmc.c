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
#include "board-tcc9300.h"

static struct mmc_port_config mmc_ports[] = {
	[0] = {
		.data0	= TCC_GPB(0),
		.data1	= TCC_GPB(1),
		.data2	= TCC_GPB(2),
		.data3	= TCC_GPB(3),
		.data4	= 0,
		.data5	= 0,
		.data6	= 0,
		.data7	= 0,
		.cmd	= TCC_GPB(12),
		.clk	= TCC_GPB(13),
		.cd	= TCC_GPA(10),

		.pwr = GPIO_SD0_ON,

	},
	[1] = {
		.data0	= TCC_GPE(12),
		.data1	= TCC_GPE(13),
		.data2	= TCC_GPE(14),
		.data3	= TCC_GPE(15),
		.data4	= 0,
		.data5	= 0,
		.data6	= 0,
		.data7	= 0,
		.cmd	= TCC_GPE(21),
		.clk	= TCC_GPE(20),
		.cd	= TCC_GPA(6),

		.pwr = GPIO_SD1_ON,
	},
};

int tcc9300_mmc_init(struct device *dev, int id)
{
	BUG_ON(id > 1);

	gpio_request(mmc_ports[id].pwr, "sd_power");

	gpio_request(TCC_GPE(12), NULL);
	gpio_request(TCC_GPE(13), NULL);
	gpio_request(TCC_GPE(14), NULL);
	gpio_request(TCC_GPE(15), NULL);

	gpio_request(TCC_GPE(20), NULL);
	gpio_request(TCC_GPE(21), NULL);
	//gpio_request(TCC_GPA(6), NULL);

	tcc_gpio_config(mmc_ports[id].data0, GPIO_FN(1) | GPIO_CD(0));
	tcc_gpio_config(mmc_ports[id].data1, GPIO_FN(1) | GPIO_CD(0));
	tcc_gpio_config(mmc_ports[id].data2, GPIO_FN(1) | GPIO_CD(0));
	tcc_gpio_config(mmc_ports[id].data3, GPIO_FN(1) | GPIO_CD(0));

	tcc_gpio_config(mmc_ports[id].cmd, GPIO_FN(1));
	tcc_gpio_config(mmc_ports[id].clk, GPIO_FN(1) | GPIO_CD(3));

	tcc_gpio_config(mmc_ports[id].cd, GPIO_FN(0));
	gpio_request(mmc_ports[id].cd, "sd_cd");
	
	gpio_direction_input(mmc_ports[id].cd);

	return 0;
}

int tcc9300_mmc_card_detect(struct device *dev, int id)
{
	BUG_ON(id > 1);

	return gpio_get_value(mmc_ports[id].cd) ? 0 : 1;
}

int tcc9300_mmc_suspend(struct device *dev, int id)
{
	struct tcc_mmc_platform_data *mmc = dev->platform_data;	

	if(id == 0) {
		gpio_direction_output(GPIO_SD0_ON, 0);
	}
	return 0;
}

int tcc9300_mmc_resume(struct device *dev, int id)
{
	struct tcc_mmc_platform_data *mmc = dev->platform_data;

	if (id == 0) {
		gpio_direction_output(GPIO_SD0_ON, 1);
	}
	return 0;
}

int tcc9300_mmc_set_power(struct device *dev, int id, int on)
{
	if (on) {
		/* power */
		gpio_direction_output(mmc_ports[id].pwr, 1);
		mdelay(1);
	} else {
		gpio_direction_output(mmc_ports[id].pwr, 0);
		//mdelay(10);
	}

	return 0;
}

int tcc9300_mmc_set_bus_width(struct device *dev, int id, int width)
{
	return 0;
}

int tcc9300_mmc_cd_int_config(struct device *dev, int id, unsigned int cd_irq)
{
	volatile PPIC pPIC = (volatile PPIC)tcc_p2v(HwPIC_BASE);
	volatile PGPIO pGPIO = (volatile PGPIO)tcc_p2v(HwGPIO_BASE);	
	//unsigned int temp_reg;
	//volatile PGPIOINT pEINT = (volatile PGPIOINT)tcc_p2v(HwEINTSEL_BASE);

	if(id==0)
	{
		#ifdef CONFIG_MACH_TCC9300ST
		BITCSET(pGPIO->EINTSEL2, HwEINTSEL2_EINT8_MASK, HwEINTSEL2_EINT8(SEL_GPIOE31));
		//	BITCSET(pEINT->EINTSEL2, HwEINTSEL2_EINT8_MASK, HwEINTSEL2_EINT8(SEL_GPIOE31));
		#else
		//	BITCSET(pEINT->EINTSEL2, HwEINTSEL2_EINT8_MASK, HwEINTSEL2_EINT8(SEL_GPIOE26));
		#endif
	}			
	else if(id==1)
	{

	}
	else if(id==2)
	{

	}		
	else
	{
	}	

	pPIC->SEL1		|= cd_irq;
	pPIC->INTMSK1	|= cd_irq;
	pPIC->MODE1 	&= (~(cd_irq)); // edge trigger
	pPIC->MODEA1	|= (cd_irq);	//both edge
	pPIC->IEN1		|= cd_irq;		
	pPIC->CLR1		|= cd_irq;	

	return 0;
}


struct tcc_mmc_platform_data tcc9300_mmc_platform_data[] = {
	[0] = {
		.slot	= 3,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/* MMC_CAP_8_BIT_DATA */
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
		.f_min	= 100000,
		.f_max	= 48000000,
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc9300_mmc_init,
		.card_detect = tcc9300_mmc_card_detect,
		.cd_int_config = tcc9300_mmc_cd_int_config,
		.suspend = tcc9300_mmc_suspend,
		.resume	= tcc9300_mmc_resume,
		.set_power = tcc9300_mmc_set_power,
		.set_bus_width = tcc9300_mmc_set_bus_width,

		.cd_int_num =HwINT0_EI8,
		.cd_irq_num = INT_EI8,		
		.peri_name = PERI_SDMMC0,
		.io_name = RB_SDMMC0CONTROLLER,
		.pic = HwINT1_SD0, 				
	},
	[1] = {
		.slot	= 1,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/*| MMC_CAP_8_BIT_DATA */
			/*| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED*/,
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc9300_mmc_init,
		.card_detect = tcc9300_mmc_card_detect,
		.cd_int_config = tcc9300_mmc_cd_int_config,
		.suspend = tcc9300_mmc_suspend,
		.resume	= tcc9300_mmc_resume,
		.set_power = tcc9300_mmc_set_power,
		.set_bus_width = tcc9300_mmc_set_bus_width,

		.cd_int_num =HwINT0_EI9,	
		.cd_irq_num = INT_EI9,		
		.peri_name = PERI_SDMMC1,
		.io_name = RB_SDMMC1CONTROLLER,
		.pic = HwINT1_SD1, 			
	},
};

static int __init tcc9300_init_mmc(void)
{
	if (!machine_is_tcc9300())
		return 0;

	printk("%s\n",__func__);
#if defined(CONFIG_MMC_TCC_SDHC)
#if defined(CONFIG_MMC_TCC_SDHC0)
	tcc9300_sdhc0_device.dev.platform_data = &tcc9300_mmc_platform_data[0];
	platform_device_register(&tcc9300_sdhc0_device);
#endif
#if defined(CONFIG_MMC_TCC_SDHC1)
	tcc9300_sdhc1_device.dev.platform_data = &tcc9300_mmc_platform_data[1];
	platform_device_register(&tcc9300_sdhc1_device);
#endif

#endif
	return 0;
}
device_initcall(tcc9300_init_mmc);
