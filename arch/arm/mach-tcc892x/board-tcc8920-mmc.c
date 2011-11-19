#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>

#include <mach/mmc.h>

#include <mach/gpio.h>
//#include <mach/bsp.h>
#include <mach/bsp.h>
#include <asm/mach-types.h>

#include "devices.h"
#include "board-tcc8920.h"

typedef enum {
#if defined(CONFIG_MMC_TCC_SDHC2)
	TCC_MMC_TYPE_EMMC,
#endif
	TCC_MMC_TYPE_SD,
	TCC_MMC_TYPE_WIFI,
	TCC_MMC_TYPE_MAX
} tcc_mmc_type;

typedef enum {
	TCC_MMC_BUS_WIDTH_4 = 4,
	TCC_MMC_BUS_WIDTH_8 = 8
} tcc_mmc_bus_width_type;

#define TCC_MMC_PORT_NULL	0x0FFFFFFF

static struct mmc_port_config mmc_ports[] = {
#if defined(CONFIG_MMC_TCC_SDHC2)
	[TCC_MMC_TYPE_EMMC] = {
		.data0	= TCC_GPF(19),
		.data1	= TCC_GPF(20),
		.data2	= TCC_GPF(21),
		.data3	= TCC_GPF(22),
		.data4	= TCC_GPF(23),
		.data5	= TCC_GPF(24),
		.data6	= TCC_GPF(25),
		.data7	= TCC_GPF(26),
		.cmd	= TCC_GPF(18),
		.clk	= TCC_GPF(17),
		.cd = TCC_GPD(12),
		.func = GPIO_FN(2),
		.width = TCC_MMC_BUS_WIDTH_8,

		.pwr = GPIO_SD0_ON,
	},
#endif
	[TCC_MMC_TYPE_SD] = {
		.data0	= TCC_GPD(18),
		.data1	= TCC_GPD(17),
		.data2	= TCC_GPD(16),
		.data3	= TCC_GPD(15),
		.data4	= TCC_MMC_PORT_NULL,
		.data5	= TCC_MMC_PORT_NULL,
		.data6	= TCC_MMC_PORT_NULL,
		.data7	= TCC_MMC_PORT_NULL,
		.cmd	= TCC_GPD(19),
		.clk	= TCC_GPD(20),
		.cd = TCC_GPF(9),
		.func = GPIO_FN(2),
		.width = TCC_MMC_BUS_WIDTH_4,

		.pwr = GPIO_SD2_ON,
	},
	[TCC_MMC_TYPE_WIFI] = {
		.data0	= TCC_GPB(20),
		.data1	= TCC_GPB(21),
		.data2	= TCC_GPB(22),
		.data3	= TCC_GPB(23),
		.data4	= TCC_MMC_PORT_NULL,
		.data5	= TCC_MMC_PORT_NULL,
		.data6	= TCC_MMC_PORT_NULL,
		.data7	= TCC_MMC_PORT_NULL,
		.cmd	= TCC_GPB(19),
		.clk	= TCC_GPB(28),
		.cd = TCC_GPB(29),
		.func = GPIO_FN(3),
		.width = TCC_MMC_BUS_WIDTH_4,

#if defined(CONFIG_WIFI_SUB_BOARD)
		.pwr = GPIO_SDWF_RST,
#else
		.pwr = GPIO_SD1_ON,
#endif
	},
};

int tcc8920_mmc_init(struct device *dev, int id)
{
	BUG_ON(id >= TCC_MMC_TYPE_MAX);

	gpio_request(mmc_ports[id].pwr, "sd_power");

	if(id == TCC_MMC_TYPE_WIFI)
	{
		gpio_request(GPIO_SD1_ON, "wifi_pre_power");
		gpio_direction_output(GPIO_SD1_ON, 0);
		msleep(100);
		gpio_direction_output(GPIO_SD1_ON, 1);

#if defined(CONFIG_WIFI_SUB_BOARD)
		gpio_request(TCC_GPG(11),"wifi_rst");

		gpio_direction_output(TCC_GPG(11), 0);
		msleep(100);
		gpio_direction_output(TCC_GPG(11), 1);
#endif
	}

	tcc_gpio_config(mmc_ports[id].data0, mmc_ports[id].func | GPIO_CD(1));
	tcc_gpio_config(mmc_ports[id].data1, mmc_ports[id].func | GPIO_CD(1));
	tcc_gpio_config(mmc_ports[id].data2, mmc_ports[id].func | GPIO_CD(1));
	tcc_gpio_config(mmc_ports[id].data3, mmc_ports[id].func | GPIO_CD(1));

	if(mmc_ports[id].width == TCC_MMC_BUS_WIDTH_8)
	{
		tcc_gpio_config(mmc_ports[id].data4, mmc_ports[id].func | GPIO_CD(1));
		tcc_gpio_config(mmc_ports[id].data5, mmc_ports[id].func | GPIO_CD(1));
		tcc_gpio_config(mmc_ports[id].data6, mmc_ports[id].func | GPIO_CD(1));
		tcc_gpio_config(mmc_ports[id].data7, mmc_ports[id].func | GPIO_CD(1));
	}

	tcc_gpio_config(mmc_ports[id].cmd, mmc_ports[id].func | GPIO_CD(1));
	tcc_gpio_config(mmc_ports[id].clk, mmc_ports[id].func | GPIO_CD(3));

	tcc_gpio_config(mmc_ports[id].cd, GPIO_FN(0)|GPIO_PULL_DISABLE);
	gpio_request(mmc_ports[id].cd, "sd_cd");

	gpio_direction_input(mmc_ports[id].cd);

	return 0;
}

int tcc8920_mmc_card_detect(struct device *dev, int id)
{
	BUG_ON(id >= TCC_MMC_TYPE_MAX);

#if defined(CONFIG_WIFI_SUB_BOARD)
	if(id == TCC_MMC_TYPE_WIFI)
		return 1;
#endif

#if defined(CONFIG_MMC_TCC_SDHC2)
	if(id == TCC_MMC_TYPE_EMMC)
		return 1;
#endif

	return gpio_get_value(mmc_ports[id].cd) ? 0 : 1;	

}

int tcc8920_mmc_suspend(struct device *dev, int id)
{
	if(id == TCC_MMC_TYPE_WIFI) {
		gpio_direction_output(GPIO_SD1_ON, 0);
	} 

	return 0;
}

int tcc8920_mmc_resume(struct device *dev, int id)
{
	if (id == TCC_MMC_TYPE_WIFI) {
		gpio_direction_output(GPIO_SD1_ON, 1);
	}

 	return 0;
}

int tcc8920_mmc_set_power(struct device *dev, int id, int on)
{
	if (on) {
		/* power */
		gpio_direction_output(mmc_ports[id].pwr, 1);
		mdelay(1);
	} else {

		//mdelay(10);
	}

	return 0;
}

int tcc8920_mmc_set_bus_width(struct device *dev, int id, int width)
{
	return 0;
}

int tcc8920_mmc_cd_int_config(struct device *dev, int id, unsigned int cd_irq)
{
	if(id == TCC_MMC_TYPE_SD)
	{
		tcc_gpio_config_ext_intr(INT_EI4, EXTINT_GPIOF_09);
		set_irq_type(INT_EI4, IRQ_TYPE_EDGE_BOTH);
	}
	#if !defined(CONFIG_WIFI_SUB_BOARD)
	else if(id == TCC_MMC_TYPE_WIFI)
	{
		tcc_gpio_config_ext_intr(INT_EI5, EXTINT_GPIOB_29);
		set_irq_type(INT_EI5, IRQ_TYPE_EDGE_BOTH);
	}
	#endif
	else
	{
		return -1;
	}	

	return 0;
}


struct tcc_mmc_platform_data tcc8920_mmc_platform_data[] = {
	#if defined(CONFIG_MMC_TCC_SDHC2)		// [0]:eMMC,   [1]:SD,   [2]:WiFi,   [3]:X
	[TCC_MMC_TYPE_EMMC] = {
		.slot	= 5,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			| MMC_CAP_8_BIT_DATA
			/*| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED*/,		// SD0 Slot
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8920_mmc_init,
		.card_detect = tcc8920_mmc_card_detect,
		.cd_int_config = tcc8920_mmc_cd_int_config,
		.suspend = tcc8920_mmc_suspend,
		.resume	= tcc8920_mmc_resume,
		.set_power = tcc8920_mmc_set_power,
		.set_bus_width = tcc8920_mmc_set_bus_width,

		.cd_int_num = -1,
		//.cd_irq_num = INT_EI5,
		.peri_name = PERI_SDMMC1,
		.io_name = RB_SDMMC1CONTROLLER,
		.pic = HwINT1_SD1,
	},
	#endif
	[TCC_MMC_TYPE_SD] = {
		.slot	= 4,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/* MMC_CAP_8_BIT_DATA */
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,		// SD2 Slot is used CPU Board
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8920_mmc_init,
		.card_detect = tcc8920_mmc_card_detect,
		.cd_int_config = tcc8920_mmc_cd_int_config,
		.suspend = tcc8920_mmc_suspend,
		.resume	= tcc8920_mmc_resume,
		.set_power = tcc8920_mmc_set_power,
		.set_bus_width = tcc8920_mmc_set_bus_width,

		.cd_int_num = HwINT1_EI4,
		.cd_irq_num = INT_EI4,
		.peri_name = PERI_SDMMC0,
		.io_name = RB_SDMMC0CONTROLLER,
		.pic = HwINT1_SD0,
	},
	#if defined(CONFIG_WIFI_SUB_BOARD)
	[TCC_MMC_TYPE_WIFI] = {
		.slot	= 2,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/*| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED*/,		// SD3 Slot
		.f_min	= 100000,
//		.f_max	= 48000000,	/* support highspeed mode */
		.f_max	= 24000000,	// Only Atheros WiFi(AR6102)
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8920_mmc_init,
		.card_detect = tcc8920_mmc_card_detect,
		.cd_int_config = tcc8920_mmc_cd_int_config,
		.suspend = tcc8920_mmc_suspend,
		.resume	= tcc8920_mmc_resume,
		.set_power = tcc8920_mmc_set_power,
		.set_bus_width = tcc8920_mmc_set_bus_width,

		.cd_int_num = -1,
		//.cd_irq_num = INT_EI5,
		.peri_name = PERI_SDMMC2,
		.io_name = RB_SDMMC2CONTROLLER,
		.pic = HwINT1_SD2,
	},
	#else
	[TCC_MMC_TYPE_WIFI] = {
		.slot	= 3,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/*| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED*/,		// SD1 Slot
		.f_min	= 100000,
//		.f_max	= 48000000,	/* support highspeed mode */
		.f_max	= 24000000,	// Only Atheros WiFi(AR6102)
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8920_mmc_init,
		.card_detect = tcc8920_mmc_card_detect,
		.cd_int_config = tcc8920_mmc_cd_int_config,
		.suspend = tcc8920_mmc_suspend,
		.resume	= tcc8920_mmc_resume,
		.set_power = tcc8920_mmc_set_power,
		.set_bus_width = tcc8920_mmc_set_bus_width,

		.cd_int_num = HwINT1_EI5,
		.cd_irq_num = INT_EI5,
		.peri_name = PERI_SDMMC3,
		.io_name = RB_SDMMC3CONTROLLER,
		.pic = HwINT1_SD3,
	},
	#endif
	#if (TCC_MMC_TYPE_WIFI == 1)		// [0]:SD,   [1]:WiFi
	[2] = {
		.slot	= 2,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/*| MMC_CAP_8_BIT_DATA */
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8920_mmc_init,
		.card_detect = tcc8920_mmc_card_detect,
		.suspend = tcc8920_mmc_suspend,
		.resume	= tcc8920_mmc_resume,
		.set_power = tcc8920_mmc_set_power,
		.set_bus_width = tcc8920_mmc_set_bus_width,

		.cd_int_num = -1,
		.peri_name = PERI_SDMMC2,
		.io_name = RB_SDMMC2CONTROLLER,
		.pic = HwINT1_SD2,
	},
	#endif
	[3] = {
		.slot	= 3,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/*| MMC_CAP_8_BIT_DATA */
			  | MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= tcc8920_mmc_init,
		.card_detect = tcc8920_mmc_card_detect,
		.suspend = tcc8920_mmc_suspend,
		.resume	= tcc8920_mmc_resume,
		.set_power = tcc8920_mmc_set_power,
		.set_bus_width = tcc8920_mmc_set_bus_width,

		.cd_int_num = -1,
		.peri_name = PERI_SDMMC3,
		.io_name = RB_SDMMC3CONTROLLER,
		.pic = HwINT1_SD3,
	},
};

static int __init tcc8920_init_mmc(void)
{
	if (!machine_is_tcc8920())
		return 0;

	printk("%s\n",__func__);

#if defined(CONFIG_MMC_TCC_SDHC)
#if defined(CONFIG_MMC_TCC_SDHC0)
	tcc_sdhc0_device.dev.platform_data = &tcc8920_mmc_platform_data[0];
	platform_device_register(&tcc_sdhc0_device);
#endif
#if defined(CONFIG_MMC_TCC_SDHC1)
	tcc_sdhc1_device.dev.platform_data = &tcc8920_mmc_platform_data[1];
	platform_device_register(&tcc_sdhc1_device);
#endif
#if defined(CONFIG_MMC_TCC_SDHC2)
	tcc_sdhc2_device.dev.platform_data = &tcc8920_mmc_platform_data[2];
	platform_device_register(&tcc_sdhc2_device);
#endif
#if defined(CONFIG_MMC_TCC_SDHC3)
	tcc_sdhc3_device.dev.platform_data = &tcc8920_mmc_platform_data[3];
	platform_device_register(&tcc_sdhc3_device);
#endif
#endif

	return 0;
}
device_initcall(tcc8920_init_mmc);
