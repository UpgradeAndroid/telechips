/*
 * linux/arch/arm/mach-tcc93xx/board-tcc9300.c
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
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/i2c/pca953x.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>

#include <asm/io.h>
#include <mach/irqs.h>
#include <mach/gpio.h>
#include <mach/bsp.h>
#include <plat/serial.h>
#include <mach/tca_serial.h>
#include <mach/tca_i2c.h>
#include <plat/tcc_ts.h>
#include <mach/common.h>

#include <plat/nand.h>

#include "board-tcc9300.h"

#include "devices.h"
#if defined(CONFIG_TCC_SENSOR_DRV)
#include <linux/i2c/sensor_i2c.h>
#endif
#if defined(CONFIG_TCC_OUTPUT_STARTER)
#include <linux/i2c/hdmi_phy.h>
#include <linux/i2c/hdmi_edid.h>
#endif
#if defined(CONFIG_FB_TCC_COMPONENT)
#include <linux/i2c/cs4954.h>
#endif

#if defined(CONFIG_VIDEO_TCCXX_CAMERA) //20100720 ysseung   add to sensor slave id.
#include <media/cam_i2c.h>
#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP)
#define SENSOR_I2C_SLAVE_ID 	(0x06>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9P111)
#define SENSOR_I2C_SLAVE_ID 	(0x7A>>1)
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
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9M113)
#define SENSOR_I2C_SLAVE_ID 	(0x78>>1)
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_OV7690)
#define SENSOR_I2C_SLAVE_ID 	(0x42>>1)
#elif defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150)
#define SENSOR_I2C_SLAVE_ID 	(0xB8>>1)
#elif defined(CONFIG_VIDEO_ATV_SENSOR_RDA5888)
#define SENSOR_I2C_SLAVE_ID 	(0xC4>>1)
#endif
#endif // defined(CONFIG_VIDEO_TCCXX_CAMERA)

#if defined(CONFIG_TCC_OUTPUT_STARTER)
#define HDMI_PHY_I2C_SLAVE_ID 		(0x70>>1)
#define HDMI_EDID_I2C_SLAVE_ID 		(0xA0>>1)
#define HDMI_EDID_SEG_I2C_SLAVE_ID	(0x60>>1)
#endif // defined(CONFIG_TCC_OUTPUT_STARTER)

#if defined(CONFIG_FB_TCC_COMPONENT)
#define CS4954_I2C_SLAVE_ID 		(0x08>>1)
#define THS8200_I2C_SLAVE_ID 		(0x40>>1)
#endif // defined(CONFIG_FB_TCC_COMPONENT)


extern void __init tcc9300_irq_init(void);
extern void __init tcc9300_init_gpio(void);
extern void __init tcc9300_map_common_io(void);


static struct spi_board_info tcc9300_spi0_board_info[] = {
	{
		.modalias = "spidev",
		.bus_num = 0,					// spi channel 0
		.chip_select = 0,
		/* you can modify the following member variables */
		.max_speed_hz = 2*1000*1000,	// default 2Mhz
		.mode = 0						// default 0, you can choose [SPI_CPOL|SPI_CPHA|SPI_CS_HIGH|SPI_LSB_FIRST]
	},
};

static struct spi_board_info tcc9300_spi1_board_info[] = {
	{
		.modalias = "spidev",
		.bus_num = 1,					// spi channel 1
		.chip_select = 0,
		/* you can modify the following member variables */
		.max_speed_hz = 2*1000*1000,	// default 2Mhz
		.mode = 0						// default 0, you can choose [SPI_CPOL|SPI_CPHA|SPI_CS_HIGH|SPI_LSB_FIRST]
	},
};

#if defined(CONFIG_VIDEO_TCCXX_CAMERA)
static struct cam_i2c_platform_data cam_i2c_data1 = {
};
#endif

#if defined(CONFIG_TCC_SENSOR_DRV)
static struct sensor_i2c_platform_data  sensor_i2c_data1 = {
};
#endif

#if defined(CONFIG_TCC_OUTPUT_STARTER)
static struct tcc_hdmi_phy_i2c_platform_data  hdmi_phy_i2c_data = {
};
static struct tcc_hdmi_phy_i2c_platform_data  hdmi_edid_i2c_data = {
};
static struct tcc_hdmi_phy_i2c_platform_data  hdmi_edid_seg_i2c_data = {
};
#endif

#if defined(CONFIG_FB_TCC_COMPONENT)
static struct cs4954_i2c_platform_data  cs4954_i2c_data = {
};
static struct cs4954_i2c_platform_data  ths8200_i2c_data = {
};
#endif

static struct tcc_i2c_platform_data tcc9300_core0_platform_data = {
    .core_clk_rate      = 4*1000*1000,    /* core clock rate: 4MHz */
    .core_clk_name      = "i2c0",
    .smu_i2c_flag       = 0,
    .i2c_ch_clk_rate[0] = 400,      /* SCL clock rate : 100kHz */
    .i2c_ch_clk_rate[1] = 400,      /* SCL clock rate : 100kHz */
};

static struct tcc_i2c_platform_data tcc9300_core1_platform_data = {
    .core_clk_rate      = 4*1000*1000,    /* core clock rate: 4MHz */
    .core_clk_name      = "i2c1",
    .smu_i2c_flag       = 0,
    .i2c_ch_clk_rate[0] = 100,      /* SCL clock rate : 100kHz */
    .i2c_ch_clk_rate[1] = 100,      /* SCL clock rate : 100kHz */
};

static struct tcc_i2c_platform_data tcc9300_core2_platform_data = {
    .core_clk_rate      = 4*1000*1000,    /* core clock rate: 4MHz */
    .core_clk_name      = "i2c2",
    .smu_i2c_flag       = 0,
    .i2c_ch_clk_rate[0] = 100,      /* SCL clock rate : 100kHz */
    .i2c_ch_clk_rate[1] = 100,      /* SCL clock rate : 100kHz */
};

static struct tcc_i2c_platform_data tcc9300_smu_platform_data = {
    .core_clk_name  = "smu",
    .smu_i2c_flag   = 1,
};

#if defined(CONFIG_TCC_OUTPUT_STARTER)
/* I2C core0 channel 0 devices */
static struct i2c_board_info __initdata i2c_devices0[] = {
    {
		I2C_BOARD_INFO("tcc-hdmi-edid", HDMI_EDID_I2C_SLAVE_ID),
		.platform_data = &hdmi_edid_i2c_data,
	},	
    {
		I2C_BOARD_INFO("tcc-hdmi-edid-seg", HDMI_EDID_SEG_I2C_SLAVE_ID),
		.platform_data = &hdmi_edid_seg_i2c_data,
	},	
};
#endif

/* I2C core0 channel 1 devices */
static struct i2c_board_info __initdata i2c_devices1[] = {
	#if defined(CONFIG_MACH_TCC9300ST) && defined(CONFIG_FB_TCC_COMPONENT)
	{
		I2C_BOARD_INFO("component-cs4954", CS4954_I2C_SLAVE_ID),
		.platform_data = &cs4954_i2c_data,
	},
	{
		I2C_BOARD_INFO("component-ths8200", THS8200_I2C_SLAVE_ID),
		.platform_data = &ths8200_i2c_data,
	},
	#endif
	#if defined(CONFIG_VIDEO_TCCXX_CAMERA)
	#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
	{
		#if defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9P111) || defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9T111)
			I2C_BOARD_INFO("tcc-cam-sensor-0", (0x7A>>1)), //20100716 ysseung   sign-up to sensor slave-id.
		#endif
		.platform_data = &cam_i2c_data1,
	},
	{
		#if defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9M113)
			I2C_BOARD_INFO("tcc-cam-sensor-1", (0x78>>1)), //20100716 ysseung   sign-up to sensor slave-id.
		#endif
		.platform_data = &cam_i2c_data1,
	},
	#else // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
	{
		I2C_BOARD_INFO("tcc-cam-sensor", SENSOR_I2C_SLAVE_ID), //20100716 ysseung   sign-up to sensor slave-id.
		.platform_data = &cam_i2c_data1,
	},
	#endif // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
	#endif // CONFIG_VIDEO_TCCXX_CAMERA
};

#if defined (CONFIG_TCC_SENSOR_DRV)
/* I2C core1 channel 1 devices */
static struct i2c_board_info __initdata i2c_devices3[] = {
	{
		I2C_BOARD_INFO("tcc-accel-sensor", 0x0B),
		.platform_data = &sensor_i2c_data1,
	},
};
#endif

/* I2C core2 channel 0 devices */
static struct i2c_board_info __initdata i2c_devices4[] = {
#if defined(CONFIG_MACH_TCC9300CM)
	#if defined(CONFIG_FB_TCC_COMPONENT)
	{
		I2C_BOARD_INFO("component-cs4954", CS4954_I2C_SLAVE_ID),
		.platform_data = &cs4954_i2c_data,
	},	
	{
		I2C_BOARD_INFO("component-ths8200", THS8200_I2C_SLAVE_ID),
		.platform_data = &ths8200_i2c_data,
	},	
	#endif
	#if defined(CONFIG_VIDEO_TCCXX_CAMERA)
	{
		I2C_BOARD_INFO("tcc-cam-sensor", SENSOR_I2C_SLAVE_ID), //20100716 ysseung   sign-up to sensor slave-id.
		.platform_data = &cam_i2c_data1,
	},
	#endif
	#endif
};

#if defined(CONFIG_TCC_OUTPUT_STARTER)
/* I2C SMU HDMI PHY devices */
static struct i2c_board_info __initdata i2c_devices7[] = {
	{
		I2C_BOARD_INFO("tcc-hdmi-phy", HDMI_PHY_I2C_SLAVE_ID),
		.platform_data = &hdmi_phy_i2c_data,
	},
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

/*----------------------------------------------------------------------
 * Device	  : ADC touchscreen resource
 * Description: tcc9300_touchscreen_resource
 *----------------------------------------------------------------------*/
#if defined(CONFIG_TOUCHSCREEN_TCCTS) || defined(CONFIG_TOUCHSCREEN_TCCTS_MODULE)
static struct resource tcc9300_touch_resources[] = {
	[0] = {
		.start	= TCC_TSADC_BASE,
		.end	= TCC_TSADC_BASE + 0x20,
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

static struct tcc_adc_ts_platform_data tcc9300_touchscreen_pdata = {
#ifdef CONFIG_TOUCHSCREEN_CALIBRATION
	.min_x = 1,
	.max_x = 4095,
	.min_y = 1,
	.max_y = 4095,
#else
	.min_x = 160,
	.max_x = 3790,
	.min_y = 380,
	.max_y = 3600,
#endif
	.reverse_x_pos = 1,
};

static struct platform_device tcc9300_touchscreen_device = {
	.name		= "tcc-ts",
	.id		= -1,
	.resource	= tcc9300_touch_resources,
	.num_resources	= ARRAY_SIZE(tcc9300_touch_resources),
	.dev = {
		.platform_data = &tcc9300_touchscreen_pdata
	},
};
#endif

static void __init tcc9300_map_io(void)
{
    tcc9300_map_common_io();
}

static void __init tcc9300_init_irq(void)
{
	tcc9300_irq_init();
}

static void tcc9300_nand_init(void)
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

static int tcc9300_nand_ready(void)
{
	return !gpio_get_value(GPIO_NAND_RDY);
}

struct tcc_nand_platform_data tcc_nand_platdata = {
	.parts		= NULL,
	.nr_parts	= 0,
	.gpio_wp	= GPIO_NAND_WP,
	.init		= tcc9300_nand_init,
	.ready		= tcc9300_nand_ready,
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




static struct platform_device *tcc9300_devices[] __initdata = {
#ifdef CONFIG_I2C_TCC_CORE0
	&tcc9300_i2c_core0_device,
#endif
#ifdef CONFIG_I2C_TCC_CORE1
	&tcc9300_i2c_core1_device,
#endif
#ifdef CONFIG_I2C_TCC_CORE2
	&tcc9300_i2c_core2_device,
#endif
#ifdef CONFIG_I2C_TCC_SMU
	&tcc9300_i2c_smu_device,
#endif
      &tcc9300_adc_device,
#ifdef CONFIG_BATTERY_TCC
 	&tcc_battery_device,
#endif      
#ifdef CONFIG_TOUCHSCREEN_TCCTS
	&tcc9300_touchscreen_device,
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

static void __init tcc9300_init_machine(void)
{
	tcc9300_init_gpio();

	#if defined(CONFIG_SPI_TCCXXXX_MASTER) || defined(CONFIG_SPI_TCCXXXX_MASTER_MODULE)
	spi_register_board_info(tcc9300_spi0_board_info, ARRAY_SIZE(tcc9300_spi0_board_info));
	//spi_register_board_info(tcc9300_spi1_board_info, ARRAY_SIZE(tcc9300_spi1_board_info)); //jhlim
	#endif

	#if defined(CONFIG_TCC_OUTPUT_STARTER)
	i2c_register_board_info(0, i2c_devices0, ARRAY_SIZE(i2c_devices0));
	#endif
	
	#if defined (CONFIG_TCC_SENSOR_DRV)
	i2c_register_board_info(3, i2c_devices3, ARRAY_SIZE(i2c_devices3));
	#endif

	i2c_register_board_info(1, i2c_devices1, ARRAY_SIZE(i2c_devices1));

	i2c_register_board_info(4, i2c_devices4, ARRAY_SIZE(i2c_devices4));

	#if defined(CONFIG_TCC_OUTPUT_STARTER)
	i2c_register_board_info(7, i2c_devices7, ARRAY_SIZE(i2c_devices7));
	#endif

	#if defined(CONFIG_SERIAL_TCC_DMA) || defined(CONFIG_BT)
    	/* BT: use UART1 and TX DMA */
    	platform_device_add_data(&tcc9300_uart1_device, &uart1_data_bt, sizeof(struct tcc_uart_platform_data));
	#endif

#if defined(CONFIG_I2C_TCC_CORE0)
	platform_device_add_data(&tcc9300_i2c_core0_device, &tcc9300_core0_platform_data, sizeof(struct tcc_i2c_platform_data));
#endif
#if defined(CONFIG_I2C_TCC_CORE1)
        platform_device_add_data(&tcc9300_i2c_core1_device, &tcc9300_core1_platform_data, sizeof(struct tcc_i2c_platform_data));
#endif
#if defined(CONFIG_I2C_TCC_CORE2)
        platform_device_add_data(&tcc9300_i2c_core2_device, &tcc9300_core2_platform_data, sizeof(struct tcc_i2c_platform_data));
#endif
#if defined(CONFIG_I2C_TCC_SMU)
        platform_device_add_data(&tcc9300_i2c_smu_device, &tcc9300_smu_platform_data, sizeof(struct tcc_i2c_platform_data));
#endif
	tcc_add_nand_device();

	platform_add_devices(tcc9300_devices, ARRAY_SIZE(tcc9300_devices));
}

MACHINE_START(TCC9300, "tcc9300")
    /* Maintainer: Telechips Linux BSP Team <linux@telechips.com> */
    .phys_io        = 0xb0000000,
    .io_pg_offst    = ((0xf0000000) >> 18) & 0xfffc,
    .boot_params    = PHYS_OFFSET + 0x00000100,
    .map_io         = tcc9300_map_io,
    .init_irq       = tcc9300_init_irq,
    .init_machine   = tcc9300_init_machine,
    .timer          = &tcc9300_timer,
MACHINE_END
