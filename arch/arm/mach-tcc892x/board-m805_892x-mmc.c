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
#include "board-m805_892x.h"

typedef enum {
#if defined(CONFIG_MMC_TCC_SDHC2)
	TCC_MMC_TYPE_EMMC,
#endif
#if defined(CONFIG_MMC_TCC_SDHC3)
	TCC_MMC_TYPE_SD3_0,
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

// PIC 0
#define HwINT0_EI4					Hw7						// R/W, External Interrupt 4 enable
#define HwINT0_EI5					Hw8						// R/W, External Interrupt 5 enable

// PIC 1
#define HwINT1_SD0					Hw12					// R/W, SD/MMC 0 interrupt enable
#define HwINT1_SD1					Hw13					// R/W, SD/MMC 1 interrupt enable
#define HwINT1_SD2	 				Hw1 					// R/W, SD/MMC 2 Interrupt enable
#define HwINT1_SD3		 			Hw0 					// R/W, SD/MMC 3 Interrupt enable


static struct mmc_port_config mmc_ports[] = {
	[TCC_MMC_TYPE_SD] = {
		.data0	= TCC_GPC(2),
		.data1	= TCC_GPC(3),
		.data2	= TCC_GPC(4),
		.data3	= TCC_GPC(5),
		.data4	= TCC_MMC_PORT_NULL,
		.data5	= TCC_MMC_PORT_NULL,
		.data6	= TCC_MMC_PORT_NULL,
		.data7	= TCC_MMC_PORT_NULL,
		.cmd	= TCC_GPC(1),
		.clk	= TCC_GPC(0),
		.cd = TCC_GPD(17),
		.func = GPIO_FN(3),
		.width = TCC_MMC_BUS_WIDTH_4,

		.pwr = TCC_MMC_PORT_NULL,
	},
};

int m805_892x_mmc_init(struct device *dev, int id)
{
	BUG_ON(id >= TCC_MMC_TYPE_MAX);

	gpio_request(mmc_ports[id].pwr, "sd_power");

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

int m805_892x_mmc_card_detect(struct device *dev, int id)
{
	BUG_ON(id >= TCC_MMC_TYPE_MAX);

	if(id == TCC_MMC_TYPE_WIFI)
		return 1;

	return gpio_get_value(mmc_ports[id].cd) ? 0 : 1;	
}

int m805_892x_mmc_suspend(struct device *dev, int id)
{
	if(id == TCC_MMC_TYPE_WIFI) 
		gpio_direction_output(GPIO_SD1_ON,0);

	return 0;
}

int m805_892x_mmc_resume(struct device *dev, int id)
{
	if(id == TCC_MMC_TYPE_WIFI) 
		gpio_direction_output(GPIO_SD1_ON,1);

 	return 0;
}

int m805_892x_mmc_set_power(struct device *dev, int id, int on)
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

int m805_892x_mmc_set_bus_width(struct device *dev, int id, int width)
{
	return 0;
}

int m805_892x_mmc_cd_int_config(struct device *dev, int id, unsigned int cd_irq)
{
	if(id == TCC_MMC_TYPE_SD)
	{
		tcc_gpio_config_ext_intr(INT_EI4, EXTINT_GPIOD_17);
	}
	else
	{
		return -1;
	}	

	return 0;
}


struct tcc_mmc_platform_data m805_892x_mmc_platform_data[] = {
	
	[TCC_MMC_TYPE_SD] = {
		.slot	= 0,
		.caps	= MMC_CAP_SDIO_IRQ | MMC_CAP_4_BIT_DATA
			/* MMC_CAP_8_BIT_DATA */
			| MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED,
		.f_min	= 100000,
		.f_max	= 48000000,	/* support highspeed mode */
		.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
		.init	= m805_892x_mmc_init,
		.card_detect = m805_892x_mmc_card_detect,
		.cd_int_config = m805_892x_mmc_cd_int_config,
		.suspend = m805_892x_mmc_suspend,
		.resume	= m805_892x_mmc_resume,
		.set_power = m805_892x_mmc_set_power,
		.set_bus_width = m805_892x_mmc_set_bus_width,

		.cd_int_num = HwINT0_EI4,
		.cd_irq_num = INT_EI4,
		.peri_name = PERI_SDMMC0,
		.io_name = RB_SDMMC0CONTROLLER,
		.pic = HwINT1_SD0,
	},
};

static int __init m805_892x_init_mmc(void)
{
	if (!machine_is_m805_892x())
		return 0;

	printk("%s\n",__func__);

#if defined(CONFIG_MMC_TCC_SDHC)
#if defined(CONFIG_MMC_TCC_SDHC0)
	tcc_sdhc0_device.dev.platform_data = &m805_892x_mmc_platform_data[0];
	platform_device_register(&tcc_sdhc0_device);
#else
	return 0;
#endif
#endif

	return 0;
}
device_initcall(m805_892x_init_mmc);
