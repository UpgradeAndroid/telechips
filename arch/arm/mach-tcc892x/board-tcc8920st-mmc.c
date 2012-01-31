#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>

#include <mach/mmc.h>

#include <mach/gpio.h>
#include <mach/irqs.h>
//#include <mach/bsp.h>
#include <mach/bsp.h>
#include <asm/mach-types.h>

#include "devices.h"
#include "board-tcc8920st.h"

extern void tcc_init_sdhc_devices(void);

struct tcc_mmc_platform_data tcc8920_mmc_platform_data[];

typedef enum {
	TCC_MMC_BUS_WIDTH_4 = 4,
	TCC_MMC_BUS_WIDTH_8 = 8
} tcc_mmc_bus_width_type;

#define TCC_MMC_PORT_NULL	0x0FFFFFFF

// PIC 0
#define HwINT0_EI4					Hw7						// R/W, External Interrupt 4 enable
#define HwINT0_EI5					Hw8						// R/W, External Interrupt 5 enable

// PIC 1
#define HwINT1_SD0					Hw12					// R/W, SD/MMC 0 interrupt enable
#define HwINT1_SD1					Hw13					// R/W, SD/MMC 1 interrupt enable
#define HwINT1_SD2	 				Hw1 					// R/W, SD/MMC 2 Interrupt enable
#define HwINT1_SD3		 			Hw0 					// R/W, SD/MMC 3 Interrupt enable


#define TCC_MMC_SDIO_WIFI_USED

typedef enum {
	TCC_MMC_TYPE_SD,
	#if defined(TCC_MMC_SDIO_WIFI_USED)
	TCC_MMC_TYPE_WIFI,
	#endif
	TCC_MMC_TYPE_MAX
} tcc_mmc_type;

static struct mmc_port_config mmc_ports[] = {
	[TCC_MMC_TYPE_SD] = {
		.data0	= TCC_GPF(19),
		.data1	= TCC_GPF(20),
		.data2	= TCC_GPF(21),
		.data3	= TCC_GPF(22),
		.data4	= TCC_MMC_PORT_NULL,
		.data5	= TCC_MMC_PORT_NULL,
		.data6	= TCC_MMC_PORT_NULL,
		.data7	= TCC_MMC_PORT_NULL,
		.cmd	= TCC_GPF(18),
		.clk	= TCC_GPF(17),
		.func	= GPIO_FN(2),
		.width	= TCC_MMC_BUS_WIDTH_4,

		.cd	= TCC_GPD(12),
		.pwr	= GPIO_SD0_ON,
	},
	#if defined(TCC_MMC_SDIO_WIFI_USED)
	#if defined(CONFIG_STB_BOARD_DONGLE)
	[TCC_MMC_TYPE_WIFI] = {
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
		.func	= GPIO_FN(2),
		.width	= TCC_MMC_BUS_WIDTH_4,

		.cd	= TCC_MMC_PORT_NULL,
		.pwr	= TCC_GPD(21),
	},
	#else
	[TCC_MMC_TYPE_WIFI] = {
		.data0	= TCC_GPB(2),
		.data1	= TCC_GPB(3),
		.data2	= TCC_GPB(4),
		.data3	= TCC_GPB(5),
		.data4	= TCC_MMC_PORT_NULL,
		.data5	= TCC_MMC_PORT_NULL,
		.data6	= TCC_MMC_PORT_NULL,
		.data7	= TCC_MMC_PORT_NULL,
		.cmd	= TCC_GPB(1),
		.clk	= TCC_GPB(0),
		.func	= GPIO_FN(3),
		.width	= TCC_MMC_BUS_WIDTH_4,

		.cd	= TCC_MMC_PORT_NULL,
		.pwr	= GPIO_SD1_ON,
	},
	#endif
	#endif
};

int tcc8920_mmc_init(struct device *dev, int id)
{
	BUG_ON(id >= TCC_MMC_TYPE_MAX);

	if(mmc_ports[id].pwr != TCC_MMC_PORT_NULL)
		gpio_request(mmc_ports[id].pwr, "sd_power");

	#if defined(TCC_MMC_SDIO_WIFI_USED)
	#if defined(CONFIG_STB_BOARD_DONGLE)
	if(id == TCC_MMC_TYPE_WIFI)
	{
		gpio_request(TCC_GPD(20), "wifi_pre_power");
		gpio_direction_output(TCC_GPD(20), 0);
		msleep(100);
		gpio_direction_output(TCC_GPD(20), 1);

	}
	#else
	if(id == TCC_MMC_TYPE_WIFI)
	{
		gpio_request(GPIO_SD1_ON, "wifi_pre_power");
		gpio_direction_output(GPIO_SD1_ON, 0);
		msleep(100);
		gpio_direction_output(GPIO_SD1_ON, 1);

	}
	#endif
	#endif

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

	if(mmc_ports[id].cd != TCC_MMC_PORT_NULL)
	{
		tcc_gpio_config(mmc_ports[id].cd, GPIO_FN(0)|GPIO_PULL_DISABLE);
		gpio_request(mmc_ports[id].cd, "sd_cd");

		gpio_direction_input(mmc_ports[id].cd);
	}

	return 0;
}

int tcc8920_mmc_card_detect(struct device *dev, int id)
{
	BUG_ON(id >= TCC_MMC_TYPE_MAX);

	if(mmc_ports[id].cd == TCC_MMC_PORT_NULL)
		return 1;

	return gpio_get_value(mmc_ports[id].cd) ? 0 : 1;	
}

int tcc8920_mmc_suspend(struct device *dev, int id)
{
	#if defined(TCC_MMC_SDIO_WIFI_USED)
	if(id == TCC_MMC_TYPE_WIFI) {
		gpio_direction_output(GPIO_SD1_ON, 0);
	} 
	#endif

	return 0;
}

int tcc8920_mmc_resume(struct device *dev, int id)
{
	#if defined(TCC_MMC_SDIO_WIFI_USED)
	if (id == TCC_MMC_TYPE_WIFI) {
		gpio_direction_output(GPIO_SD1_ON, 1);
	}
	#endif

 	return 0;
}

int tcc8920_mmc_set_power(struct device *dev, int id, int on)
{
	if (on) {
		/* power */
		if(mmc_ports[id].pwr != TCC_MMC_PORT_NULL)
		{
			gpio_direction_output(mmc_ports[id].pwr, 1);

			mdelay(1);
		}
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
	if(tcc8920_mmc_platform_data[id].cd_int_num > 0)
	{
		tcc_gpio_config_ext_intr(tcc8920_mmc_platform_data[id].cd_irq_num, tcc8920_mmc_platform_data[id].cd_ext_irq);
	}
	else
	{
		return -1;
	}	

	return 0;
}

//Start : Wakeup for SD Insert->Remove in suspend. - 120109, hjbae
int tcc892x_sd_card_detect(void)
{
	return gpio_get_value(mmc_ports[TCC_MMC_TYPE_SD].cd) ? 0 : 1;
}
//End

struct tcc_mmc_platform_data tcc8920_mmc_platform_data[] = {
	[TCC_MMC_TYPE_SD] = {
		.slot	= 5,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/* MMC_CAP_8_BIT_DATA */
#if defined(CONFIG_STB_BOARD_DONGLE)
			,
#else
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
#endif
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

		.cd_int_num = HwINT0_EI4,
		.cd_irq_num = INT_EI4,
		.cd_ext_irq = EXTINT_GPIOD_12,
		.peri_name = PERI_SDMMC1,
		.io_name = RB_SDMMC1CONTROLLER,
		.pic = HwINT1_SD1,
	},
	#if defined(TCC_MMC_SDIO_WIFI_USED)
	#if defined(CONFIG_STB_BOARD_DONGLE)
	[TCC_MMC_TYPE_WIFI] = {
		.slot	= 4,
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

		.cd_int_num = -1, 
		.peri_name = PERI_SDMMC0,
		.io_name = RB_SDMMC0CONTROLLER,
		.pic = HwINT1_SD0,
	},
	#else
	[TCC_MMC_TYPE_WIFI] = {
		.slot	= 2,
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

		.cd_int_num = -1, 
		.peri_name = PERI_SDMMC2,
		.io_name = RB_SDMMC2CONTROLLER,
		.pic = HwINT1_SD2,
	},
	#endif
	#endif

	#if 0	//for Example
	[x] = {
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
};

static int __init tcc8920_init_mmc(void)
{
	if (!machine_is_tcc8920st())
		return 0;

	printk("%s\n",__func__);

	tcc_init_sdhc_devices();

#if defined(CONFIG_MMC_TCC_SDHC)
#if defined(CONFIG_MMC_TCC_SDHC0)
	tcc_sdhc0_device.dev.platform_data = &tcc8920_mmc_platform_data[0];
	platform_device_register(&tcc_sdhc0_device);
#endif
#if defined(CONFIG_MMC_TCC_SDHC1)
	#if defined(TCC_MMC_SDIO_WIFI_USED)
	tcc_sdhc1_device.dev.platform_data = &tcc8920_mmc_platform_data[1];
	platform_device_register(&tcc_sdhc1_device);
	#endif
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
