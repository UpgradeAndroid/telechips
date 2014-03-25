#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>

#include <mach/mmc.h>
#include <mach/gpio.h>
#include <mach/bsp.h>

#include "board-tcc9200s.h"

int tcc9200s_mmc_init(struct device *dev, int id)
{
#if defined(CONFIG_BCM4325_WIFI_9200S_DEMO_BOARD)
	gpio_request(TCC_GPE(4));
	gpio_request(TCC_GPD(4));

	gpio_direction_output(TCC_GPE(4), 1);
	msleep(10);
	gpio_direction_output(TCC_GPD(11), 1);
#endif

	if (id == 0) {
		BITSET(HwGPIOB->GPEN, Hw25|Hw26|Hw27|Hw28|Hw29|Hw30);	// SD_CLK(6), SD_CMD(6), SD_D0(6), SD_D1(6), SD_D2(6) SD_D3(6)	
		BITCSET(HwGPIOB->GPFN3, HwPORTCFG_GPFN1_MASK|HwPORTCFG_GPFN2_MASK|HwPORTCFG_GPFN3_MASK|HwPORTCFG_GPFN4_MASK|HwPORTCFG_GPFN5_MASK|HwPORTCFG_GPFN6_MASK,		
			HwPORTCFG_GPFN1(2)|HwPORTCFG_GPFN2(2)|HwPORTCFG_GPFN3(2)|HwPORTCFG_GPFN4(2)|		HwPORTCFG_GPFN5(2)|HwPORTCFG_GPFN6(2)	);

		BITCSET(HwGPIOB->GPCD1, 0x3FFC0000, 0x3FFC0000);	// driver strength

		BITSET(HwGPIOB->GPEN, Hw25|Hw26|Hw27|Hw28|Hw29|Hw30);	// SD_CLK(6), SD_CMD(6), SD_D0(6), SD_D1(6), SD_D2(6) SD_D3(6)
		BITCSET(HwGPIOB->GPFN3, 0x0FFFFFF0, 0x02222220);		// SD_CLK(6), SD_CMD(6), SD_D0(6), SD_D1(6), SD_D2(6) SD_D3(6)
		//BITSET(HwGPIOB->GPCD1, Hw29|Hw28 | Hw26|Hw25 | Hw24|Hw23 | Hw22|Hw21 | Hw20|Hw19 | Hw18|Hw17);	// drive strength //FIXME

		HwGPIOA->GPDAT &= ~Hw10;
	} else {
		BITCSET(HwGPIOE->GPFN0, HwPORTCFG_GPFN6_MASK|HwPORTCFG_GPFN7_MASK,
			HwPORTCFG_GPFN6(3)|HwPORTCFG_GPFN7(3));
		BITCSET(HwGPIOE->GPFN1, HwPORTCFG_GPFN0_MASK|HwPORTCFG_GPFN1_MASK|HwPORTCFG_GPFN2_MASK|HwPORTCFG_GPFN3_MASK,
			HwPORTCFG_GPFN0(3)|HwPORTCFG_GPFN1(3)|HwPORTCFG_GPFN2(3)|HwPORTCFG_GPFN3(3));
		
		BITCSET(HwGPIOE->GPCD0, 0x00FFF000, 0x00003000);	// driver strength
		
		HwGPIOA->GPDAT &= ~Hw13;
	}
	return 0;

}

int tcc9200s_mmc_card_detect(struct device *dev, int id)
{
	if (id == 1) {
		return gpio_get_value(TCC_GPA(13)) ? 0 : 1;
	} else {
#if defined(CONFIG_BCM4325_WIFI_9200S_DEMO_BOARD)
		return 1;
#else
		return gpio_get_value(TCC_GPA(10)) ? 0 : 1;
#endif
	}
}

int tcc9200s_mmc_suspend(struct device *dev, int id)
{
	struct tcc_mmc_platform_data *mmc = dev->platform_data; 

	gpio_direction_output(TCC_GPE(27), 0);

	if(mmc->cd_int_num >0)
	{		
		writel( readl(&HwPIC->INTMSK1)&~(1<<mmc->cd_int_num) , &HwPIC->INTMSK1);
	}
	
	return 0;
}

int tcc9200s_mmc_resume(struct device *dev, int id)
{
	struct tcc_mmc_platform_data *mmc = dev->platform_data; 

	gpio_direction_output(TCC_GPE(27), 1);

	if(mmc->cd_int_num >0)
	{		
		writel( readl(&HwPIC->INTMSK1)|(1<<mmc->cd_int_num) , &HwPIC->INTMSK1);
	}
	
	return 0;
}

int tcc9200s_mmc_set_power(struct device *dev, int id, int on)
{
	return 0;
}

int tcc9200s_mmc_set_bus_width(struct device *dev, int id, int width)
{
	switch (width) {
	case MMC_BUS_WIDTH_1:
		if (id == 1){
			BITCSET(HwGPIOE->GPFN0, HwPORTCFG_GPFN6_MASK|HwPORTCFG_GPFN7_MASK,
				HwPORTCFG_GPFN6(3)|HwPORTCFG_GPFN7(3));
			BITCSET(HwGPIOE->GPFN1, HwPORTCFG_GPFN0_MASK|HwPORTCFG_GPFN1_MASK
				|HwPORTCFG_GPFN2_MASK|HwPORTCFG_GPFN3_MASK,
				HwPORTCFG_GPFN0(3)|HwPORTCFG_GPFN1(3)|HwPORTCFG_GPFN2(3)
				|HwPORTCFG_GPFN3(3));
		} else if (id == 0) {
			//gpio	//
			//BITCSET(HwGPIOB->GPFN3, HwPORTCFG_GPFN1_MASK|HwPORTCFG_GPFN2_MASK
			//	|HwPORTCFG_GPFN3_MASK|HwPORTCFG_GPFN4_MASK|HwPORTCFG_GPFN5_MASK
			//	|HwPORTCFG_GPFN6_MASK,
			//	HwPORTCFG_GPFN1(2)|HwPORTCFG_GPFN2(2)|HwPORTCFG_GPFN3(2)
			//	|HwPORTCFG_GPFN4(2)|HwPORTCFG_GPFN5(2)|HwPORTCFG_GPFN6(2));
		}
		break;

	case MMC_BUS_WIDTH_4:
		if(id ==1) {
			BITCSET(HwGPIOE->GPFN0, HwPORTCFG_GPFN6_MASK|HwPORTCFG_GPFN7_MASK,
				HwPORTCFG_GPFN6(3)|HwPORTCFG_GPFN7(3));
			BITCSET(HwGPIOE->GPFN1, HwPORTCFG_GPFN0_MASK|HwPORTCFG_GPFN1_MASK
				|HwPORTCFG_GPFN2_MASK|HwPORTCFG_GPFN3_MASK,
				HwPORTCFG_GPFN0(3)|HwPORTCFG_GPFN1(3)|HwPORTCFG_GPFN2(3)
				|HwPORTCFG_GPFN3(3));
		}
		break;

	case MMC_BUS_WIDTH_8:
		break;
	}
	return 0;
}

struct tcc_mmc_platform_data tcc9200s_mmc_platform_data[] = {
	[0] = {
		.slot	= 6,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/* MMC_CAP_8_BIT_DATA */
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
		.f_min	= 100000,
#if defined(CONFIG_BCM4325_WIFI)
		.f_max	= 24000000,
#else
		.f_max	= 48000000,
#endif
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.cd_int_num =INT_EI8,				
		.card_detect = tcc9200s_mmc_card_detect,
		.init	= tcc9200s_mmc_init,
		.suspend = tcc9200s_mmc_suspend,
		.resume = tcc9200s_mmc_resume,
		.set_power = tcc9200s_mmc_set_power,
		.set_bus_width = tcc9200s_mmc_set_bus_width,
	},
	[1] = {
		.slot	= 4,
		.caps = MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/*| MMC_CAP_8_BIT_DATA */
			/* To support highspeed mode in TCC9200s EVM, the value of
			 * damping resistor should be changed. */
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.cd_int_num =INT_EI9,				
		.card_detect = tcc9200s_mmc_card_detect,
		.init	= tcc9200s_mmc_init,
		.suspend = tcc9200s_mmc_suspend,
		.resume = tcc9200s_mmc_resume,
		.set_power = tcc9200s_mmc_set_power,
		.set_bus_width = tcc9200s_mmc_set_bus_width,
	},
};
