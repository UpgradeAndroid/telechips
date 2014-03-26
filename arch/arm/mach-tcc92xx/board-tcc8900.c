/* linux/arch/arm/mach-tcc92xx/board-tcc8900.c
 *
 * Copyright (C) 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/i2c/pca953x.h>
#include <linux/spi/spi.h>
#include <linux/regulator/act8810.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>

#include <asm/io.h>
#include <mach/irqs.h>
#include <mach/gpio.h>
#include <plat/serial.h>
#include <mach/tca_serial.h>
#include <mach/tca_i2c.h>
#include <plat/tcc_ts.h>
#include <mach/bsp.h>
#include <mach/common.h>
#include <plat/nand.h>

#include "devices.h"
#include "board-tcc8900.h"

#if defined(CONFIG_VIDEO_TCCXX_CAMERA) //20100720 ysseung   add to sensor slave id.
#include <media/cam_i2c.h>
#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP)
#define SENSOR_I2C_SLAVE_ID 	(0x06>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9P111)
#define SENSOR_I2C_SLAVE_ID 	(0x78>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9T111)
#define SENSOR_I2C_SLAVE_ID 	(0x7A>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_MV9317)
#define SENSOR_I2C_SLAVE_ID 	(0x50>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9D112)
#define SENSOR_I2C_SLAVE_ID 	(0x7A>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_OV3640)
#define SENSOR_I2C_SLAVE_ID 	(0x78>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_S5K4BAFB)
#define SENSOR_I2C_SLAVE_ID 	(0x52>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006)
#define SENSOR_I2C_SLAVE_ID 	(0x34>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_GT2005)
#define SENSOR_I2C_SLAVE_ID 	(0x78>>1)
#elif defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150)
#define SENSOR_I2C_SLAVE_ID 		(0xB8>>1)
#elif defined(CONFIG_VIDEO_ATV_SENSOR_RDA5888)
#define SENSOR_I2C_SLAVE_ID 		(0xC4>>1)
#endif
#endif // defined(CONFIG_VIDEO_TCCXX_CAMERA)

#if defined(CONFIG_RADIO_RDA5870)
#define RDA5870E_FM_I2C_SLAVE_ID	(0x20>>1)
#define RDA5870E_PMU_I2C_SLAVE_ID	(0x2C>>1)
#endif

extern void __init tcc9200_irq_init(void);
extern void __init tcc9200_map_common_io(void);
extern void __init tcc_init_time(void);

static struct spi_board_info tcc8900_spi0_board_info[] = {
	{
		.modalias = "spidev",
		.bus_num = 0,
		.chip_select = 0,
		.max_speed_hz = 2 * 1000 *1000,
	},
};

static void __init tcc9200_init_irq(void)
{
	tcc9200_irq_init();
}

static void tcc8900_nand_init(void)
{
	unsigned int gpio_wp = GPIO_NAND_WP;
	unsigned int gpio_rdy = GPIO_NAND_RDY;

	tcc_gpio_config(gpio_wp, GPIO_FN(0));
	tcc_gpio_config(gpio_rdy, GPIO_FN(0));

	gpio_request(gpio_wp, "nand_wp");
	gpio_direction_output(gpio_wp, 1);

	gpio_request(gpio_rdy, "nand_rdy");
	gpio_direction_input(gpio_rdy);
}

static int tcc8900_nand_ready(void)
{
	return !gpio_get_value(GPIO_NAND_RDY);
}

static struct tcc_nand_platform_data tcc_nand_platdata = {
	.parts		= NULL,
	.nr_parts	= 0,
	.gpio_wp	= GPIO_NAND_WP,
	.init		= tcc8900_nand_init,
	.ready		= tcc8900_nand_ready,
};

static struct platform_device tcc_nand_device = {
	.name		= "tcc_mtd",
	.id			= -1,
	.dev		= {
		.platform_data = &tcc_nand_platdata,
	},
};

static void tcc_add_nand_device(void)
{
	tcc_get_partition_info(&tcc_nand_platdata.parts, &tcc_nand_platdata.nr_parts);
	platform_device_register(&tcc_nand_device);
}

static struct pca953x_platform_data pca9539_data1 = {
	.gpio_base	= GPIO_PORTEXT1,
};


static struct pca953x_platform_data pca9539_data2 = {
	.gpio_base	= GPIO_PORTEXT2
};

static struct pca953x_platform_data pca9538_data = {
	.gpio_base	= GPIO_PORTEXT3,
};

#if defined(CONFIG_VIDEO_TCCXX_CAMERA)
static struct cam_i2c_platform_data cam_i2c_data = {
};
#endif

#if defined(CONFIG_I2C_TCC92XX)
static struct tcc_i2c_platform_data tcc9200_i2c_platform_data = {
    .core_clk_rate      = 4*1000*1000,    /* core clock rate: 4MHz */
    .core_clk_name      = "i2c",
    .smu_i2c_flag       = 0,
    .i2c_ch_clk_rate[0] = 100,      /* SCL clock rate : 100kHz */
    .i2c_ch_clk_rate[1] = 400,      /* SCL clock rate : 400kHz[dxb] */
};

static struct tcc_i2c_platform_data tcc9200_smu_platform_data = {
    .core_clk_name  = "smu",
    .smu_i2c_flag   = 1,
};
#endif

#if defined(CONFIG_REGULATOR_ACT8810)
static struct regulator_consumer_supply act8810_consumer = {
	.supply = "vdd_core",
};

static struct regulator_init_data act8810_dcdc_info = {
	.constraints = {
		.name = "vdd_core range",
		.min_uV = 1050000,
		.max_uV = 1700000,
		.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
	},
	.num_consumer_supplies = 1,
	.consumer_supplies     = &act8810_consumer,
};

static struct act8810_subdev_data act8810_subdev = {
	.name = "vdd_core",
	.id   = ACT8810_ID_DCDC1,
	.platform_data = &act8810_dcdc_info,
};

static struct act8810_platform_data act8810_info = {
	.num_subdevs = 1,
	.subdevs     = &act8810_subdev,
};
#endif

#ifdef CONFIG_I2C_BOARDINFO
static struct i2c_board_info __initdata i2c_devices0[] = {
	{
		I2C_BOARD_INFO("pca9539", 0x74),
		.platform_data = &pca9539_data1,
	},
	{
		I2C_BOARD_INFO("pca9539", 0x77),
		.platform_data = &pca9539_data2,
	},
	{
		I2C_BOARD_INFO("pca9538", 0x70),
		.platform_data = &pca9538_data,
	},
	#if defined(CONFIG_VIDEO_TCCXX_CAMERA)
	#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
	{
		I2C_BOARD_INFO("tcc-cam-sensor-0", (0x50>>1)), //20100716 ysseung   sign-up to sensor slave-id.
		.platform_data = &cam_i2c_data,
	},
	{
		I2C_BOARD_INFO("tcc-cam-sensor-1", (0x7A>>1)), //20100716 ysseung   sign-up to sensor slave-id.
		.platform_data = &cam_i2c_data,
	},
	#else // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
	{
		I2C_BOARD_INFO("tcc-cam-sensor", SENSOR_I2C_SLAVE_ID), //20100716 ysseung   sign-up to sensor slave-id.
		.platform_data = &cam_i2c_data,
	},
	#endif // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
	#endif // CONFIG_VIDEO_TCCXX_CAMERA
	#if defined(CONFIG_RADIO_RDA5870)
	{
		I2C_BOARD_INFO("RDA5870E-FM", RDA5870E_FM_I2C_SLAVE_ID),
		.platform_data = NULL,
	},
	{
		I2C_BOARD_INFO("RDA5870E-PMU", RDA5870E_PMU_I2C_SLAVE_ID),
		.platform_data = NULL,
	},
	#endif
	#if defined(CONFIG_REGULATOR_ACT8810)
	{
		I2C_BOARD_INFO("act8810", 0x5A),
		.platform_data = &act8810_info,
	},
	#endif
};

static struct i2c_board_info __initdata i2c_devices1[] = {
	{
		I2C_BOARD_INFO("tcc-tsc2003-ts", 0x48),
		.platform_data = NULL,
	},
};
#endif

#if defined(CONFIG_TOUCHSCREEN_TCCTS)
static struct resource tcc_touch_resources[] = {
	[0] = {
		.start	= 0xF05F4000,
		.end	= 0xF05F4000 + 0x20,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_TSADC,
		.end   = INT_TSADC,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start = INT_EI2,
		.end   = INT_EI2,
		.flags = IORESOURCE_IRQ,
	},
};

static struct tcc_adc_ts_platform_data tcc_touchscreen_pdata = {
#ifdef CONFIG_TOUCHSCREEN_CALIBRATION
	.min_x = 1,
	.max_x = 1023,
	.min_y = 1,
	.max_y = 1023,
#else
	.min_x = 54,
	.max_x = 960,
	.min_y = 110,
	.max_y = 925,
#endif
	.reverse_x_pos = 1,
// for LVDS touch using internal ADC touch
#if 0    
	.reverse_y_pos = 1,
	.swap_xy_pos = 1,
#endif
};

static struct platform_device tcc_touchscreen_device = {
	.name		= "tcc-ts",
	.id		= -1,
	.resource	= tcc_touch_resources,
	.num_resources	= ARRAY_SIZE(tcc_touch_resources),
	.dev = {
		.platform_data = &tcc_touchscreen_pdata
	},
};
#endif /* CONFIG_TOUCHSCREEN_TCCTS */

#if defined(CONFIG_TCC_WATCHDOG)
static struct platform_device tccwdt_device = {
	.name	= "tcc-wdt",
	.id		= -1,
};
#endif

#if defined(CONFIG_SERIAL_TCC_DMA) || defined(CONFIG_BT)
static struct tcc_uart_platform_data uart1_data_bt = {
    .bt_use         = 1,

    .tx_dma_use     = 0,
    .tx_dma_buf_size= SERIAL_TX_DMA_BUF_SIZE,
    .tx_dma_base    = &HwGDMA1_BASE,
    .tx_dma_ch      = SERIAL_TX_DMA_CH_NUM,
    .tx_dma_intr    = INT_DMA1_CH0,
    .tx_dma_mode    = SERIAL_TX_DMA_MODE,

    .rx_dma_use     = 1,
    .rx_dma_buf_size= SERIAL_RX_DMA_BUF_SIZE,
    .rx_dma_base    = &HwGDMA1_BASE,
    .rx_dma_ch      = SERIAL_RX_DMA_CH_NUM,
    .rx_dma_intr    = 0,
    .rx_dma_mode    = SERIAL_RX_DMA_MODE,
};
#endif


static struct platform_device *tcc8900_devices[] __initdata = {
#if defined(CONFIG_I2C_TCC92XX)
	&tcc9200_i2c_device,
	&tcc9200_smu_device,
#endif
#if defined(CONFIG_TOUCHSCREEN_TCCTS)
	&tcc_touchscreen_device,
#endif
	&tcc_otg_device,
#if defined(CONFIG_TCC_WATCHDOG)
	&tccwdt_device,
#endif
};

char* atheros_wifi_mac;

static int __init board_wifimac_setup(char *wifimac)
{
	atheros_wifi_mac = wifimac;

	return 1;
}
__setup("wifimac=", board_wifimac_setup);

EXPORT_SYMBOL(atheros_wifi_mac);

static void __init tcc9200_init_machine(void)
{
#ifdef CONFIG_I2C_BOARDINFO
	i2c_register_board_info(0, i2c_devices0, ARRAY_SIZE(i2c_devices0));
	i2c_register_board_info(1, i2c_devices1, ARRAY_SIZE(i2c_devices1));
#endif

	spi_register_board_info(tcc8900_spi0_board_info, ARRAY_SIZE(tcc8900_spi0_board_info));

#if defined(CONFIG_SERIAL_TCC_DMA) || defined(CONFIG_BT)
    /* BT: use UART1 and TX DMA */
    platform_device_add_data(&tcc9200_uart1_device, &uart1_data_bt, sizeof(struct tcc_uart_platform_data));
#endif

#if defined(CONFIG_I2C_TCC92XX)
    platform_device_add_data(&tcc9200_i2c_device, &tcc9200_i2c_platform_data, sizeof(tcc9200_i2c_platform_data));
    platform_device_add_data(&tcc9200_smu_device, &tcc9200_smu_platform_data, sizeof(tcc9200_smu_platform_data));
#endif

	tcc_add_nand_device();

	platform_add_devices(tcc8900_devices, ARRAY_SIZE(tcc8900_devices));
}


static void __init tcc9200_map_io(void)
{
    tcc9200_map_common_io();
}

MACHINE_START(TCC8900, "tcc8900")
    /* Maintainer: Telechips Linux BSP Team <linux@telechips.com> */
    .atag_offset    = 0x100,
    .map_io         = tcc9200_map_io,
    .init_irq       = tcc9200_init_irq,
    .init_machine   = tcc9200_init_machine,
    .init_time      = tcc_init_time,
MACHINE_END
