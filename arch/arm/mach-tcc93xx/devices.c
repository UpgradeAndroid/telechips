/*
 * linux/arch/arm/mach-tcc93xx/devices.c
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
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/android_pmem.h>

#include <asm/setup.h>
#include <asm/io.h>
#include <mach/irqs.h>
#include <asm/mach-types.h>
#include <asm/mach/map.h>

#include <asm/mach/flash.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/usb/android_composite.h>

#include <mach/ohci.h>
#include <mach/bsp.h>
#include <plat/pmap.h>
#include <mach/tcc_fb.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tcc_cam_ioctrl.h>

/*----------------------------------------------------------------------
 * Device     : Remote Controller Driver resource
 * Description: tcc9300_remote_resources 
 *----------------------------------------------------------------------*/
#if defined(CONFIG_INPUT_TCC_REMOTE) 
static struct resource tcc9300_remote_resources[] = {
#if 0
	[0] = {
		.start	= 0xB0101000,		
		.end	= 0xB0101000+ 0x20,	
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_RMT,
		.end   = INT_RMT,
		.flags = IORESOURCE_IRQ,
	},
#endif
};

static struct platform_device tcc9300_remote_device = {
	.name		= "tcc-remote",
	.id		= -1,
	.resource	= tcc9300_remote_resources,
	.num_resources	= ARRAY_SIZE(tcc9300_remote_resources),
};

static inline void tcc9300_init_remote(void)
{
	platform_device_register(&tcc9300_remote_device);
}
#endif /* CONFIG_INPUT_TCC_REMOT */



/*----------------------------------------------------------------------
 * Device	  : ADC resource
 * Description: tcc9300_adc_resource
 *----------------------------------------------------------------------*/

struct resource tcc9300_adc_resources[] = {
	[0] = {
		.start	= TCC_TSADC_BASE,		
		.end	= TCC_TSADC_BASE + 0x40,	
		.flags	= IORESOURCE_MEM,
	},
};

struct platform_device tcc9300_adc_device = {
	.name		= "tcc-adc",
	.id		= -1,
	.resource	= tcc9300_adc_resources,
	.num_resources	= ARRAY_SIZE(tcc9300_adc_resources),
};

/*----------------------------------------------------------------------
 * Device     : RTC resource
 * Description: tcc9300_rtc_resources
 *----------------------------------------------------------------------*/
#if defined(CONFIG_RTC_DRV_TCC9300) || defined(CONFIG_RTC_DRV_TCC_MODULE)
static struct resource tcc9300_rtc_resource[] = {
	[0] = {
		.start = TCC_RTC_BASE,
		.end   = TCC_RTC_BASE + 0xff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start  = INT_RTC,
		.end    = INT_RTC,
		.flags  = IORESOURCE_IRQ,
	}
};

static struct platform_device tcc9300_rtc_device = {
	.name           = "tcc-rtc",
	.id             = -1,
	.resource       = tcc9300_rtc_resource,
	.num_resources  = ARRAY_SIZE(tcc9300_rtc_resource),
	.dev		= {    
		.platform_data  = NULL,   
	}
};

static inline void tcc9300_init_rtc(void)
{
	platform_device_register(&tcc9300_rtc_device);
}
#endif  /* CONFIG_RTC_DRV_TCC */


#if defined(CONFIG_I2C_TCC)
#define I2C_IRQSTR_OFFSET	0xC0	/* only I2C */
#define I2C_ICLK_OFFSET		0x80	/* only SMU_I2C */

/*----------------------------------------------------------------------
 * Device     : I2C resource
 * Description: tcc9300_i2c_core0_resources
 *				tcc9300_i2c_core1_resources
 *				tcc9300_i2c_core2_resources
 *
 * [0] = {										// channel
 *		.name	= "master0"						// channel name
 *		.start	= 100,							// speed (KHz)
 *		.end	= tcc_p2v(HwI2C_CORE0_BASE),	// base address
 *		.flags	= IORESOURCE_IRQ,				// resource type
 * },
 *----------------------------------------------------------------------*/
#if defined(CONFIG_I2C_TCC_CORE0)
static struct resource tcc9300_i2c_core0_resources[] = {
	[0] = {
		.name	= "master0",		/* only HDMI */
		.start  = TCC_I2C_CORE0_PHYS_BASE,
		.end    = TCC_I2C_CORE0_PHYS_BASE + I2C_IRQSTR_OFFSET,
		.flags	= IORESOURCE_IO,
    },
	[1] = {
		.name	= "master1",		/* MIPI Camera */
		.start	= TCC_I2C_CORE0_PHYS_BASE + 0x40,
		.end	= TCC_I2C_CORE0_PHYS_BASE + I2C_IRQSTR_OFFSET,
		.flags	= IORESOURCE_IO,
    },
};
struct platform_device tcc9300_i2c_core0_device = {
    .name           = "tcc-i2c",
    .id             = 0,
    .resource       = tcc9300_i2c_core0_resources,
    .num_resources  = ARRAY_SIZE(tcc9300_i2c_core0_resources),
};
#endif
#if defined(CONFIG_I2C_TCC_CORE1)
static struct resource tcc9300_i2c_core1_resources[] = {
	[0] = {
		.name	= "master0",							/* CODEC, GPIO_EXPENDER, Power Sub */
		.start  = TCC_I2C_CORE1_PHYS_BASE,
		.end    = TCC_I2C_CORE1_PHYS_BASE + I2C_IRQSTR_OFFSET,
		.flags	= IORESOURCE_IO,
    },
	[1] = {
		.name	= "master1",
		.start	= TCC_I2C_CORE1_PHYS_BASE + 0x40,
		.end	= TCC_I2C_CORE1_PHYS_BASE + I2C_IRQSTR_OFFSET,
		.flags	= IORESOURCE_IO,
    },
};
struct platform_device tcc9300_i2c_core1_device = {
    .name           = "tcc-i2c",
    .id             = 1,
    .resource       = tcc9300_i2c_core1_resources,
    .num_resources  = ARRAY_SIZE(tcc9300_i2c_core1_resources),
};
#endif
#if defined(CONFIG_I2C_TCC_CORE2)
static struct resource tcc9300_i2c_core2_resources[] = {
	[0] = {
		.name	= "master0",		/* Touch, DXB */
		.start  = TCC_I2C_CORE2_PHYS_BASE,
		.end    = TCC_I2C_CORE2_PHYS_BASE + I2C_IRQSTR_OFFSET,
		.flags	= IORESOURCE_IO,
    },
	[1] = {
		.name	= "master1",
		.start	= TCC_I2C_CORE2_PHYS_BASE + 0x40,
		.end	= TCC_I2C_CORE2_PHYS_BASE + I2C_IRQSTR_OFFSET,
		.flags	= IORESOURCE_IO,
    },
};
struct platform_device tcc9300_i2c_core2_device = {
    .name           = "tcc-i2c",
    .id             = 2,
    .resource       = tcc9300_i2c_core2_resources,
    .num_resources  = ARRAY_SIZE(tcc9300_i2c_core2_resources),
};
#endif
#if defined(CONFIG_I2C_TCC_SMU)
static struct resource tcc9300_i2c_smu_resources[] = {
	[0] = {
		.name	= "master0",		/* SATA PHY */
		.start  = TCC_SMUI2C_PHYS_BASE,
        .end    = TCC_SMUI2C_PHYS_BASE + I2C_ICLK_OFFSET,
		.flags	= IORESOURCE_IO,
    },
	[1] = {
		.name	= "master1",		/* HDMI PHY */
		.start	= TCC_SMUI2C_PHYS_BASE + 0x40,
		.end    = TCC_SMUI2C_PHYS_BASE + I2C_ICLK_OFFSET,
		.flags	= IORESOURCE_IO,
    },
};
struct platform_device tcc9300_i2c_smu_device = {
    .name           = "tcc-i2c",
    .id             = 3,
    .resource       = tcc9300_i2c_smu_resources,
    .num_resources  = ARRAY_SIZE(tcc9300_i2c_smu_resources),
};
#endif
#endif  /* #if defined(CONFIG_I2C_TCC) */

#if defined(CONFIG_FB_TCC93XX) || defined(CONFIG_FB_TCC93XX_MODULE)
/*----------------------------------------------------------------------
 * Device     : LCD Frame Buffer resource
 * Description: 
 *----------------------------------------------------------------------*/
static u64 tcc9300_device_lcd_dmamask = 0xffffffffUL;
struct platform_device tcc_lcd_device = {
	.name	  = "tccxxx-lcd",
	.id	  = -1,
	.dev      = {
		.dma_mask		= &tcc9300_device_lcd_dmamask,
//		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif


/*----------------------------------------------------------------------
 * Device     : Serial-ATA resource
 * Description: 
 *----------------------------------------------------------------------*/
#if 0
#if defined(CONFIG_SATA_TCC) || defined(CONFIG_SATA_TCC_MODULE)
static struct resource sata_resources[] = {
    [0] = {
	    .start = 0xF0560000,
	    .end   = 0xF0560800,
	    .flags = IORESOURCE_MEM,
    },
    [1] = {
	    .start	= INT_SATA,
	    .end	= INT_SATA,
	    .flags	= IORESOURCE_IRQ,
    },
};

static struct platform_device tcc9300_sata_device = {
	.name		= "tcc-sata",
	.id		= 0,
	.resource	= sata_resources,
	.num_resources	= ARRAY_SIZE(sata_resources),
};

static inline void tcc9300_init_sata(void)
{
	platform_device_register(&tcc9300_sata_device);
}
#endif

#if defined(CONFIG_BLK_DEV_PATA_TCC89X) || defined(CONFIG_BLK_DEV_PATA_TCC89X_MODULE)
static struct resource ide_resources[] = {
    [0] = {
	    .start = 0xF0520000,
	    .end   = 0xF0520080,
	    .flags = IORESOURCE_MEM,
    },
    [1] = {
	    .start = INT_EI5,
	    .end   = INT_EI5,
	    .flags = IORESOURCE_IRQ,
    },
};

static struct platform_device tcc9300_ide_device = {
	.name		= "tcc-ide",
	.id		= 0,
	.resource	= ide_resources,
	.num_resources	= ARRAY_SIZE(ide_resources),
};

static inline void tcc9300_init_ide(void)
{
	platform_device_register(&tcc9300_ide_device);
}
#endif
#endif

/*----------------------------------------------------------------------
 * Device     : SPI(GPSB) Master resource
 * Description: 
 *----------------------------------------------------------------------*/
#if defined(CONFIG_SPI_TCCXXXX_MASTER) || defined(CONFIG_SPI_TCCXXXX_MASTER_MODULE)
static struct resource spi0_resources[] = {
	[0] = {
		.start = TCC_SPI0_BASE,
		.end   = TCC_SPI0_BASE + 0x38,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_GPSB0_DMA,
		.end   = INT_GPSB0_DMA,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
#if defined(CONFIG_MACH_TCC9300CM)
		.start	= 0xD,
		.end	= 0xD,
#else
		.start	= 0xE,		/* GPIO_E[08:11] */
		.end	= 0xE,
#endif		
		.flags	= IORESOURCE_IO,
	},
};

static struct platform_device tcc9300_spi0_device = {
	.name		= "tcc-spi",
	.id		= 0,
	.resource	= spi0_resources,
	.num_resources	= ARRAY_SIZE(spi0_resources),
};

/* jhlim
static struct resource spi1_resources[] = {
	[0] = {
		.start = TCC_SPI1_BASE,
		.end   = TCC_SPI1_BASE + 0x38,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_GPSB1_DMA,
		.end   = INT_GPSB1_DMA,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start	= 0xE,		// GPIO_E[11:8]
		.end	= 0xE,
		.flags	= IORESOURCE_IO,
	},
};

static struct platform_device tcc9300_spi1_device = {
	.name		= "tcc-spi",
	.id		= 1,
	.resource	= spi1_resources,
	.num_resources	= ARRAY_SIZE(spi1_resources),
};
*/

static inline void tcc9300_init_spi(void)
{
	platform_device_register(&tcc9300_spi0_device);
	//platform_device_register(&tcc9300_spi1_device); jhlim
}
#endif


/*----------------------------------------------------------------------
 * Device     : SPI(TSIF) Slave resource
 * Description:
 *----------------------------------------------------------------------*/
#if defined(CONFIG_SPI_TCCXXXX_TSIF_SLAVE) || defined(CONFIG_SPI_TCCXXXX_TSIF_SLAVE_MODULE)
static struct resource tsif_resources[] = {
	[0] = {
#if defined(CONFIG_MACH_TCC9300CM)
        .start	= 0xE,
		.end	= 0xE,
#else		
		.start	= 0xC,		/* GPIO_C[28:31] */
		.end	= 0xC,
#endif		
		.flags	= IORESOURCE_IO,
	}
};

static struct platform_device tcc_tsif_device = {
	.name		= "tcc-tsif",
	.id		= -1,
	.resource	= tsif_resources,
	.num_resources	= ARRAY_SIZE(tsif_resources),
};

static inline void tcc9300_init_tsif(void)
{
	platform_device_register(&tcc_tsif_device);
}
#endif


/*----------------------------------------------------------------------
 * Device     : TSIF Parallel resource
 * Description:
 *----------------------------------------------------------------------*/
#if defined(CONFIG_TSIF_TCCXXXX)
static struct resource tsif_module_resources[] = {
	[0] = {
		.start	= 0xE,		/* GPIO_C[28:31] */
		.end	= 0xE,
		.flags	= IORESOURCE_IO,
	}
};

static struct platform_device tcc_tsif_module_device = {
	.name		= "tcc-tsif-module",
	.id			= -1,
	.resource	= tsif_module_resources,
	.num_resources	= ARRAY_SIZE(tsif_module_resources),
};

static inline void tcc9300_init_tsif_module(void)
{
	platform_device_register(&tcc_tsif_module_device);
}
#endif


/*----------------------------------------------------------------------
 * Device	  : SD/MMC resource
 * Description: tcc9300_sdhc0_resource
 *				tcc9300_sdhc1_resource
 *				tcc9300_sdhc2_resource
				tcc9300_sdhc3_resource

 *----------------------------------------------------------------------*/
static u64 tcc9300_device_sdhc_dmamask = 0xffffffffUL;
#if defined(CONFIG_MMC_TCC_SDHC0) || defined(CONFIG_MMC_TCC_SDHC_CORE0_MODULE)
static struct resource tcc9300_sdhc0_resource[] = {
	[0] = {
		.start	= TCC_SDMMC0_PHYS_BASE,
		.end	= TCC_SDMMC0_PHYS_BASE + 0x1ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD0,
		.end	= INT_SD0,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device tcc9300_sdhc0_device = {
	.name		= "tcc-sdhc0",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(tcc9300_sdhc0_resource),
	.resource	= tcc9300_sdhc0_resource,
	.dev		= {
		.dma_mask		= &tcc9300_device_sdhc_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC1) || defined(CONFIG_MMC_TCC_SDHC_CORE1_MODULE)
static struct resource tcc9300_sdhc1_resource[] = {
	#if defined(CONFIG_MACH_TCC9300ST)
	[0] = {
		.start	= TCC_SDMMC3_PHYS_BASE,
		.end	= TCC_SDMMC3_PHYS_BASE + 0x1ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD3,
		.end	= INT_SD3,
		.flags	= IORESOURCE_IRQ,
	},
	#else
	[0] = {
		.start	= TCC_SDMMC1_PHYS_BASE,
		.end	= TCC_SDMMC1_PHYS_BASE + 0x1ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD1,
		.end	= INT_SD1,
		.flags	= IORESOURCE_IRQ,
	},
	#endif
};
struct platform_device tcc9300_sdhc1_device = {
	.name		= "tcc-sdhc1",
	.id 		= 1,
	.num_resources	= ARRAY_SIZE(tcc9300_sdhc1_resource),
	.resource	= tcc9300_sdhc1_resource,
	.dev		= {
		.dma_mask		= &tcc9300_device_sdhc_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC2) || defined(CONFIG_MMC_TCC_SDHC_CORE2_MODULE)
static struct resource tcc9300_sdhc2_resource[] = {
	[0] = {
		.start	= TCC_SDMMC2_PHYS_BASE,
		.end	= TCC_SDMMC2_PHYS_BASE + 0x1ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD2,
		.end	= INT_SD2,
		.flags	= IORESOURCE_IRQ,
	},
};
struct platform_device tcc9300_sdhc2_device = {
	.name		= "tcc-sdhc2",
	.id 		= 2,
	.num_resources	= ARRAY_SIZE(tcc9300_sdhc2_resource),
	.resource	= tcc9300_sdhc2_resource,
	.dev		= {
		.dma_mask		= &tcc9300_device_sdhc_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC3) || defined(CONFIG_MMC_TCC_SDHC_CORE3_MODULE)
static struct resource tcc9300_sdhc3_resource[] = {
	#if defined(CONFIG_MACH_TCC9300ST)
	[0] = {
		.start	= TCC_SDMMC1_PHYS_BASE,
		.end	= TCC_SDMMC1_PHYS_BASE + 0x1ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD1,
		.end	= INT_SD1,
		.flags	= IORESOURCE_IRQ,
	},
	#else
	[0] = {
		.start	= TCC_SDMMC3_PHYS_BASE,
		.end	= TCC_SDMMC3_PHYS_BASE + 0x1ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD3,
		.end	= INT_SD3,
		.flags	= IORESOURCE_IRQ,
	},
	#endif
};
struct platform_device tcc9300_sdhc3_device = {
	.name		= "tcc-sdhc3",
	.id 		= 3,
	.num_resources	= ARRAY_SIZE(tcc9300_sdhc3_resource),
	.resource	= tcc9300_sdhc3_resource,
	.dev		= {
		.dma_mask		= &tcc9300_device_sdhc_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_TCC_GMAC) || defined(CONFIG_TCC_GMAC_MODULE)
static u64 tcc_gmac_dma_mask = ~(u32)0;
static struct resource tcc9300_gmac_resources[] = {
	[0] = {
		.start	= TCC_GMAC_BASE,
		.end	= TCC_GMAC_BASE + (0x2000-1),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_GMAC,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device tcc9300_gmac_device = {
	.name			= "tcc-gmac",
	.id				= -1, 
	.dev			= {
		.dma_mask = &tcc_gmac_dma_mask,
		.coherent_dma_mask = 0xffffffffUL,
	},
	.num_resources	= ARRAY_SIZE(tcc9300_gmac_resources),
	.resource		= tcc9300_gmac_resources,
};

static inline void tcc9300_init_gmac(void)
{
	platform_device_register(&tcc9300_gmac_device);
}
#endif

#if defined(CONFIG_SERIAL_TCC) || defined(CONFIG_SERIAL_TCC_MODULE)
/*----------------------------------------------------------------------
 * Device     : UART resource
 * Description: uart0_resources
 *              uart1_resources
 *              uart2_resources (not used)
 *              uart3_resources (not used)
 *              uart4_resources (not used)
 *              uart5_resources (not used)
 *----------------------------------------------------------------------*/
static struct resource uart0_resources[] = {
    /* PA -> VA */
    [0] = {
        .start  = TCC_UART0_BASE,
        .end    = TCC_UART0_BASE + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART0,
        .end    = INT_UART0,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc9300_uart0_device = {
    .name           = "tcc-uart",
    .id             = 0,
    .resource       = uart0_resources,
    .num_resources  = ARRAY_SIZE(uart0_resources),
};

static struct resource uart1_resources[] = {
    [0] = {
        .start  = TCC_UART1_BASE,
        .end    = TCC_UART1_BASE + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART1,
        .end    = INT_UART1,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc9300_uart1_device = {
    .name           = "tcc-uart",
    .id             = 1,
    .resource       = uart1_resources,
    .num_resources  = ARRAY_SIZE(uart1_resources),
};

static struct resource uart2_resources[] = {
    [0] = {
        .start  = TCC_UART2_BASE,
        .end    = TCC_UART2_BASE + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART2,
        .end    = INT_UART2,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc9300_uart2_device = {
    .name           = "tcc-uart",
    .id             = 2,
    .resource       = uart2_resources,
    .num_resources  = ARRAY_SIZE(uart2_resources),
};

static struct resource uart3_resources[] = {
    [0] = {
        .start  = TCC_UART3_BASE,
        .end    = TCC_UART3_BASE + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART3,
        .end    = INT_UART3,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc9300_uart3_device = {
    .name           = "tcc-uart",
    .id             = 3,
    .resource       = uart3_resources,
    .num_resources  = ARRAY_SIZE(uart3_resources),
};

static struct resource uart4_resources[] = {
    [0] = {
        .start  = TCC_UART4_BASE,
        .end    = TCC_UART4_BASE + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART4,
        .end    = INT_UART4,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc9300_uart4_device = {
    .name           = "tcc-uart",
    .id             = 4,
    .resource       = uart4_resources,
    .num_resources  = ARRAY_SIZE(uart4_resources),
};

#if defined(CONFIG_GPS)
static struct resource uart5_resources[] = {
    [0] = {
        .start  = TCC_UART5_BASE,
        .end    = TCC_UART5_BASE + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART5,
        .end    = INT_UART5,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc9300_uart5_device = {
    .name           = "tcc-uart",
    .id             = 5,
    .resource       = uart5_resources,
    .num_resources  = ARRAY_SIZE(uart5_resources),
};
#endif

static inline void tcc9300_init_uart(void)
{
    platform_device_register(&tcc9300_uart0_device);
    platform_device_register(&tcc9300_uart1_device);
    platform_device_register(&tcc9300_uart2_device);
    platform_device_register(&tcc9300_uart3_device);
    platform_device_register(&tcc9300_uart4_device);

#if defined(CONFIG_GPS)
    platform_device_register(&tcc9300_uart5_device);
#endif 
}
#endif

#ifdef CONFIG_BATTERY_TCC
struct platform_device tcc_battery_device = {
    .name           = "tcc-battery",
    .id             = -1,
};

/*
static inline void tcc9300_init_battery_init(void)
{
        platform_device_register(&tcc_battery_device);
}
*/
#endif //config_battery_tcc

/*----------------------------------------------------------------------
 * Device     : USB Android Gadget 
 * Description: 
 *----------------------------------------------------------------------*/
static struct usb_mass_storage_platform_data mass_storage_pdata = {
#ifdef CONFIG_SCSI
	//.nluns = 4, // for iNand
	.nluns = 3, 
#else
	.nluns = 2,
#endif
	.vendor = "Telechips ",
	.product = "Android ",
	.release = 0x0100,
};

static struct platform_device usb_mass_storage_device = {
	.name = "usb_mass_storage",
	.id = -1,
	.dev = {
		.platform_data = &mass_storage_pdata,
		},
};

#ifdef CONFIG_USB_ANDROID_RNDIS
static struct usb_ether_platform_data rndis_pdata = {
	/* ethaddr is filled by board_serialno_setup */
	.vendorID	= 0x0XXX,
	.vendorDescr	= "Telechips",
};

static struct platform_device rndis_device = {
	.name	= "rndis",
	.id	= -1,
	.dev	= {
		.platform_data = &rndis_pdata,
	},
};
#endif

static char *usb_functions_ums[] = {
	"usb_mass_storage",
};

static char *usb_functions_ums_adb[] = {
	"usb_mass_storage",
	"adb",
};

static char *usb_functions_rndis[] = {
	"rndis",
};

static char *usb_functions_rndis_adb[] = {
	"rndis",
	"adb",
};


static char *usb_functions_all[] = {
#ifdef CONFIG_USB_ANDROID_RNDIS
	"rndis",
#endif
	"usb_mass_storage",
	"adb",
#ifdef CONFIG_USB_ANDROID_ACM
	"acm",
#endif
};

static struct android_usb_product usb_products[] = {
	{
		.product_id	= 0xb058, /* Telechips UMS PID */
		.num_functions	= ARRAY_SIZE(usb_functions_ums),
		.functions	= usb_functions_ums,
	},
	{
		.product_id	= 0xdeed,
		.num_functions	= ARRAY_SIZE(usb_functions_ums_adb),
		.functions	= usb_functions_ums_adb,
	},
	{
		.product_id	= 0x0002,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis),
		.functions	= usb_functions_rndis,
	},
	{
		.product_id	= 0x0003,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis_adb),
		.functions	= usb_functions_rndis_adb,
	},
};

static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id      = 0x18D1,
	.product_id     = 0x0001,
	//.adb_product_id = 0xDEED,
	.version	= 0x0100,
	.product_name	= "Android USB Composite device",
	.manufacturer_name = "Telechips",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id		= -1,
	.dev		= {
		.platform_data = &android_usb_pdata,
	},
};

void __init tcc_add_usb_devices(void)
{
#ifdef CONFIG_USB_ANDROID_RNDIS
	platform_device_register(&rndis_device);
#endif
	platform_device_register(&usb_mass_storage_device);
	platform_device_register(&android_usb_device);
}

/*----------------------------------------------------------------------
 * Device     : USB DWC OTG resource
 * Description: dwc_otg_resources
 *----------------------------------------------------------------------*/
#if defined(CONFIG_TCC_DWC_OTG)
static u64 tcc9300_dwc_otg_dmamask = 0xffffffffUL;
#if defined(CONFIG_TCC_DWC_OTG0) || defined(CONFIG_TCC_DWC_OTG0_MODULE)
static struct resource dwc_otg0_resources[] = {
	[0] = {
		.start = 0/*port number*/,
		.end   = 0/*port number*/,
		.flags = IORESOURCE_IO,
	},
	[1] = {
		.start	= tcc_p2v(HwUSB20OTG_BASE),
		.end	= tcc_p2v(HwUSB20OTG_BASE) + 0x100,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start = INT_UOTG,
		.end   = INT_UOTG,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device tcc9300_dwc_otg0_device = {
	.name			= "dwc_otg",
	.id				= 0,
	.resource		= dwc_otg0_resources,
	.num_resources	= ARRAY_SIZE(dwc_otg0_resources),
	.dev			= {
		.dma_mask			= &tcc9300_dwc_otg_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
};
#endif
	
#if defined(CONFIG_TCC_DWC_OTG1) || defined(CONFIG_TCC_DWC_OTG1_MODULE)
static struct resource dwc_otg1_resources[] = {
	[0] = {
		.start = 1/*port number*/,
		.end   = 1/*port number*/,
		.flags = IORESOURCE_IO,
	},
	[1] = {
		.start	= tcc_p2v(HwUSB20OTG1_BASE),
		.end	= tcc_p2v(HwUSB20OTG1_BASE) + 0x100,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start = INT_UOTG1,
		.end   = INT_UOTG1,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device tcc9300_dwc_otg1_device = {
	.name			= "dwc_otg",
	.id 			= 1,
	.resource		= dwc_otg1_resources,
	.num_resources	= ARRAY_SIZE(dwc_otg1_resources),
	.dev			= {
		.dma_mask 			= &tcc9300_dwc_otg_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
};
#endif

static inline void tcc9300_init_dwc_otg(void)
{
	struct platform_device *pdev_first = NULL;
	struct platform_device *pdev_second = NULL;
#if defined(CONFIG_TCC_DWC_OTG_2PORT_0OTG_1HOST)
	pdev_first = &tcc9300_dwc_otg1_device;
	pdev_second = &tcc9300_dwc_otg0_device;
#elif defined(CONFIG_TCC_DWC_OTG_2PORT_0HOST_1OTG)
	pdev_first = &tcc9300_dwc_otg0_device;
	pdev_second = &tcc9300_dwc_otg1_device;
#else
#if defined(CONFIG_TCC_DWC_OTG0) || defined(CONFIG_TCC_DWC_OTG0_MODULE)
	pdev_first = &tcc9300_dwc_otg0_device;
#endif
#if defined(CONFIG_TCC_DWC_OTG1) || defined(CONFIG_TCC_DWC_OTG1_MODULE)
	pdev_second = &tcc9300_dwc_otg1_device;
	#endif
#endif

	if (pdev_first) platform_device_register(pdev_first);
	if (pdev_second) platform_device_register(pdev_second);
}
#endif

#if defined(CONFIG_SATA_AHCI) || defined(CONFIG_SATA_AHCI_MODULE)
static struct resource tcc_ahci_resources[] = {
	{
		.start	=	0xB0810000,
		.end		=	0xB0810000 + 0x1FFF,
		.flags	=	IORESOURCE_MEM,
	},
	{
		.start	=	INT_SATAH,
		.flags	=	IORESOURCE_IRQ,
	}
};

static int tcc_ahci_data = 0;
static struct platform_device tcc_ahci_device = {
	.name	=	"ahci",
	.id	=	-1,
	.dev	=	{
				.platform_data = &tcc_ahci_data,
				.coherent_dma_mask = 0xffffffff,
			},
	.num_resources = ARRAY_SIZE(tcc_ahci_resources),
	.resource	= tcc_ahci_resources,
};

int __init tcc_register_sata(void)
{
	return platform_device_register(&tcc_ahci_device);
}
#endif

int tcc_panel_id = -1;

static int __init parse_tag_tcc_panel(const struct tag *tag)
{
	int *id = (int *) &tag->u;

	tcc_panel_id = *id;

	return 0;
}
__tagtable(ATAG_TCC_PANEL, parse_tag_tcc_panel);

int tcc_is_camera_enable = -1;

static int __init parse_tag_tcc_is_camera_enable(const struct tag *tag)
{
        int *is_camera_enable = (int *) &tag->u;

        tcc_is_camera_enable = *is_camera_enable;

        return 0;
}
__tagtable(ATAG_CAMERA, parse_tag_tcc_is_camera_enable);

struct display_platform_data tcc_display_data = {
	.resolution	= 0,
	.output		= 0,
	.composite	= 0,
};

struct android_pmem_platform_data pmem_pdata = {
	.name = "pmem",
	.no_allocator = 1,
	.cached = 1,
};

struct platform_device pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &pmem_pdata },
};

struct android_pmem_platform_data pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.no_allocator = 1,
	.cached = 1,
};

struct platform_device pmem_adsp_device = {
        .name = "android_pmem",
        .id = 1,
        .dev = { .platform_data = &pmem_adsp_pdata },
};


static void tcc_add_pmem_devices(void)
{
	pmap_t pmem;
	pmap_t pmem_cam;

	pmap_get_info("pmem", &pmem);
	pmap_get_info("camera", &pmem_cam);

	pmem_pdata.start = pmem.base;
	pmem_pdata.size = pmem.size;
	platform_device_register(&pmem_device);

	pmem_adsp_pdata.start = pmem_cam.base;
	pmem_adsp_pdata.size = pmem_cam.size;
	platform_device_register(&pmem_adsp_device);
}



/*
 * This gets called after board-specific INIT_MACHINE, and initializes most
 * on-chip peripherals accessible on this board (except for few like USB):
 *
 *  (a) Does any "standard config" pin muxing needed.  Board-specific
 *  code will have muxed GPIO pins and done "nonstandard" setup;
 *  that code could live in the boot loader.
 *  (b) Populating board-specific platform_data with the data drivers
 *  rely on to handle wiring variations.
 *  (c) Creating platform devices as meaningful on this board and
 *  with this kernel configuration.
 *
 * Claiming GPIOs, and setting their direction and initial values, is the
 * responsibility of the device drivers.  So is responding to probe().
 *
 * Board-specific knowlege like creating devices or pin setup is to be
 * kept out of drivers as much as possible.  In particular, pin setup
 * may be handled by the boot loader, and drivers should expect it will
 * normally have been done by the time they're probed.
 */
static int __init tcc9300_init_devices(void)
{
#if defined(CONFIG_I2C_TCC)
//	tcc9300_init_i2c();
#endif

#if defined(CONFIG_RTC_DRV_TCC9300) || defined(CONFIG_RTC_DRV_TCC_MODULE)
    tcc9300_init_rtc();
#endif

#if defined(CONFIG_SERIAL_TCC) || defined(CONFIG_SERIAL_TCC_MODULE)
    tcc9300_init_uart();
#endif

#if defined(CONFIG_SPI_TCCXXXX_MASTER) || defined(CONFIG_SPI_TCCXXXX_MASTER_MODULE)
	tcc9300_init_spi();
#endif
#if defined(CONFIG_SPI_TCCXXXX_TSIF_SLAVE) || defined(CONFIG_SPI_TCCXXXX_TSIF_SLAVE_MODULE)
	tcc9300_init_tsif();
#endif
#if defined(CONFIG_TSIF_TCCXXXX)
	tcc9300_init_tsif_module();
#endif


#if defined(CONFIG_INPUT_TCC_REMOTE)
    tcc9300_init_remote();
#endif

#if defined(CONFIG_TCC_DWC_OTG) || defined(CONFIG_TCC_DWC_OTG_MODULE)
	tcc9300_init_dwc_otg();
#endif

#if defined(CONFIG_USB_ANDROID)
	tcc_add_usb_devices();
#endif

#if defined(CONFIG_TCC_GMAC) || defined(CONFIG_TCC_GMAC_MODULE)
	tcc9300_init_gmac();
#endif

#if 0
#if defined(CONFIG_SATA_TCC) || defined(CONFIG_SATA_TCC_MODULE)
    tcc9300_init_sata();
#endif

#if defined(CONFIG_BLK_DEV_PATA_TCC89X) || defined(CONFIG_BLK_DEV_PATA_TCC89X_MODULE)
    tcc9300_init_ide();
#endif
#endif

#if defined(CONFIG_SATA_AHCI) || defined(CONFIG_SATA_AHCI_MODULE)
	tcc_register_sata();
#endif

	tcc_add_pmem_devices();

    return 0;
}

arch_initcall(tcc9300_init_devices);
