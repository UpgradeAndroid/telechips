/*
 * linux/arch/arm/mach-tcc92xx/devices.c
 *
 * Author:  <linux@telechips.com>
 * Created: 10th Feb, 2009
 * Description:
 *
 * Copyright (C) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/android_pmem.h>

#include <asm/setup.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/mach/map.h>

#include <asm/mach/flash.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>

#include <mach/irqs.h>

#include <mach/ohci.h>
#include <mach/bsp.h>
#include <plat/pmap.h>
#include <mach/tcc_fb.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tcc_cam_ioctrl.h>
#include <mach/tca_i2c.h>

#if 1
/*----------------------------------------------------------------------
 * Device     : Adc resource
 * Description: tcc9200_adc_resources
 *----------------------------------------------------------------------*/
static struct resource tcc_adc_resources[] = {
	[0] = {
		.start	= 0xF05F4000,
		.end	= 0xF05F4000 + 0x4C,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_TSADC,
		.end   = INT_TSADC,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device tcc9200_adc_device = {
	.name			= "tcc9200-adc",
	.id				= 0,
	.resource		= tcc_adc_resources,
	.num_resources	= ARRAY_SIZE(tcc_adc_resources),
};
#endif
/*----------------------------------------------------------------------
 * Device     : RTC resource
 * Description: tcc9200_rtc_resources
 *----------------------------------------------------------------------*/
#if defined(CONFIG_RTC_DRV_TCC9200)
static struct resource tcc9200_rtc_resource[] = {
    [0] = {
        .start = 0xF05F2000,
        .end   = 0xF05F20FF,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_RTC,
        .end    = INT_RTC,
        .flags  = IORESOURCE_IRQ,
    }
};

static struct platform_device tcc9200_rtc_device = {
    .name           = "tcc-rtc",
    .id             = -1,
    .resource       = tcc9200_rtc_resource,
    .num_resources  = ARRAY_SIZE(tcc9200_rtc_resource),
};
#endif  /* CONFIG_RTC_DRV_TCC */

#if defined(CONFIG_INPUT_TCC_REMOTE) 
static struct resource tcc9200_remote_resources[] = {
#if 0
	[0] = {
		.start	= 0xF05F3000,		
		.end	= 0xF05F3000 + 0x20,	
		.flags	= IORESOURCE_MEM,
	},
    [1] = {
        .start = INT_RMT,
        .end   = INT_RMT,
        .flags = IORESOURCE_IRQ,
    },
#endif
};

static struct platform_device tcc9200_remote_device = {
	.name			= "tcc-remote",
	.id				= -1,
//	.resource		= tcc9200_remote_resources,
//	.num_resources	= ARRAY_SIZE(tcc9200_remote_resources),
};
#if 1
static inline void tcc9200_init_remote(void)
{
	platform_device_register(&tcc9200_remote_device);
}
#endif
#endif /* CONFIG_INPUT_TCC_REMOTE */

/*----------------------------------------------------------------------
 * Device     : I2C resource
 * Description: tcc9200_i2c_resources has master0 and master1
 *----------------------------------------------------------------------*/
static struct resource tcc9200_i2c_resources[] = {
	[0] = {
        .name   = "master0",
		.start	= 0xF0530000,						/* I2C master ch0 base address */
		.end	= 0xF0530000 + I2C_IRQSTR_OFFSET,	/* I2C master ch1 base address */
		.flags	= IORESOURCE_IO,
    },
	[1] = {
        .name   = "master1",
		.start	= 0xF0530040,						/* I2C ch0 100Kbps */
		.end	= 0xF0530000 + I2C_IRQSTR_OFFSET,	/* I2C ch1 400Kbps */  //SYS3 100Kbps 
		.flags	= IORESOURCE_IO,
    },
};

struct platform_device tcc9200_i2c_device = {
    .name           = "tcc-i2c",
    .id             = 0,
    .resource       = tcc9200_i2c_resources,
    .num_resources  = ARRAY_SIZE(tcc9200_i2c_resources),
};

static struct resource tcc9200_smu_resources[] = {
	/* SMU_I2C */
	[0] = {
        .name   = "master0",
		.start	= 0xF0405000,						/* SMU_I2C master ch0 SATA-PHY base address */
		.end	= 0xF0405000 + I2C_ICLK_OFFSET,		/* SMU_I2C master ch1 HDMI-PHY base address */
		.flags	= IORESOURCE_IO,
    },
	[1] = {
        .name   = "master1",
		.start	= 0xF0405040,						/* SMU_I2C ch0 100Kbps */
		.end	= 0xF0405000 + I2C_ICLK_OFFSET,		/* SMU_I2C ch1 100Kbps */
		.flags	= IORESOURCE_IO,
    },
};

struct platform_device tcc9200_smu_device = {
    .name           = "tcc-i2c",
    .id             = 1,
    .resource       = tcc9200_smu_resources,
    .num_resources  = ARRAY_SIZE(tcc9200_smu_resources),
};

/*----------------------------------------------------------------------
 * Device     : LCD Frame Buffer resource
 * Description: 
 *----------------------------------------------------------------------*/

static u64 tcc_device_lcd_dmamask = 0xffffffffUL;

struct platform_device tcc_lcd_device = {
	.name	  = "tccxxx-lcd",
	.id		  = -1,
	.dev      = {
		.dma_mask		    = &tcc_device_lcd_dmamask,
//		.coherent_dma_mask	= 0xffffffffUL
	}
};

/*----------------------------------------------------------------------
 * Device     : SPI(GPSB) Master resource
 * Description: 
 *----------------------------------------------------------------------*/
#if defined(CONFIG_SPI_TCCXXXX_MASTER) || defined(CONFIG_SPI_TCCXXXX_MASTER_MODULE)
static struct resource spi0_resources[] = {
    [0] = {
		.start = 0xF0536000,
		.end   = 0xF0536038,
		.flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = INT_GPSB0_DMA,
        .end   = INT_GPSB0_DMA,
        .flags = IORESOURCE_IRQ,
    },
    [2] = {
        .start	= 0xD,		/* GPIO_C[32:28] */
        .end	= 0xD,
        .flags	= IORESOURCE_IO,
    },
};
static struct platform_device tcc9200_spi0_device = {
    .name			= "tcc-spi",
    .id				= 0,
    .resource		= spi0_resources,
    .num_resources	= ARRAY_SIZE(spi0_resources),
};
/* jhlim
static struct resource spi1_resources[] = {
    [0] = {
		.start = 0xF0536100,
		.end   = 0xF0536138,
		.flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = INT_GPSB1_DMA,
        .end   = INT_GPSB1_DMA,
        .flags = IORESOURCE_IRQ,
    },
};
static struct platform_device tcc9200_spi1_device = {
    .name			= "tcc-spi",
    .id				= 1,
    .resource		= spi1_resources,
    .num_resources	= ARRAY_SIZE(spi1_resources),
};
*/
static inline void tcc9200_init_spi(void)
{
    platform_device_register(&tcc9200_spi0_device);
// jhlim    platform_device_register(&tcc9200_spi1_device);
}
#endif

/*----------------------------------------------------------------------
 * Device     : SPI(TSIF) Slave resource
 * Description:
 *----------------------------------------------------------------------*/
#if defined(CONFIG_SPI_TCCXXXX_TSIF_SLAVE) || defined(CONFIG_SPI_TCCXXXX_TSIF_SLAVE_MODULE)
/************* TSIF ********************/
static struct resource tsif_resources[] = {
	[0] = {
		.start	= 0xE,		/* GPIO_E[10:8] */
		.end	= 0xE,
		.flags	= IORESOURCE_IO,
	}
};

static struct platform_device tcc9200_tsif_device = {
	.name	= "tcc-tsif",
	.id		= -1,
	.resource	= tsif_resources,
	.num_resources	= ARRAY_SIZE(tsif_resources),


};

static inline void tcc9200_init_tsif(void)
{
    platform_device_register(&tcc9200_tsif_device);
}
#endif

/*----------------------------------------------------------------------
 * Device	  : SD/MMC resource
 * Description: 
 *----------------------------------------------------------------------*/
#if !defined(CONFIG_MMC_TCC_4SD_SLOT)
#if defined(CONFIG_MMC_TCC_SDHC_CORE0)
static u64 tcc_device_mmc_core0_dmamask = 0xffffffffUL;

static struct resource tcc_mmc_core0_resource[] = {
	[0] = {
		.start	= 0xF05A0000,
		.end	= 0xF05A00FF,
		.flags	= IORESOURCE_MEM,
	},	
	[1] = {
		.start	= INT_SD0,
		.end	= INT_SD0,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device tcc_mmc_core0_device = {
	.name			= "tcc-mmc0",
	.id 			= 0,
	.num_resources	= ARRAY_SIZE(tcc_mmc_core0_resource),
	.resource		= tcc_mmc_core0_resource,
	.dev			= {
		.dma_mask		= &tcc_device_mmc_core0_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC_CORE1)
static u64 tcc_device_mmc_core1_dmamask = 0xffffffffUL;

static struct resource tcc_mmc_core1_resource[] = {
	[0] = {
		.start	= 0xF05A0200,
		.end	= 0xF05A02FF,
		.flags	= IORESOURCE_MEM,
	},	
	[1] = {
		.start	= INT_SD1,
		.end	= INT_SD1,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device tcc_mmc_core1_device = {
	.name			= "tcc-mmc1",
	.id 			= 1,
	.num_resources	= ARRAY_SIZE(tcc_mmc_core1_resource),
	.resource		= tcc_mmc_core1_resource,
	.dev			= {
		.dma_mask		= &tcc_device_mmc_core1_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif
#else
/*----------------------------------------------------------------------
 * Device	  : SD/MMC resource
 * Description: tcc_mmc_core0_slot0_resource
 *				tcc_mmc_core1_slot1_resource
 *				tcc_mmc_core0_slot2_resource
 *				tcc_mmc_core1_slot3_resource
 *----------------------------------------------------------------------*/
#if defined(CONFIG_MMC_TCC_SDHC_CORE0_SLOT0) || defined(CONFIG_MMC_TCC_SDHC_CORE0_SLOT0_MODULE)
static u64 tcc_mmc_core0_slot0_dmamask = 0xffffffffUL;
static struct resource tcc_mmc_core0_slot0_resource[] = {
	[0] = {
		.start	= 0xF05A0000,
		.end	= 0xF05A00FF,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD0_SLOT0,
		.end	= INT_SD0_SLOT0,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.start	= INT_EI8,
		.end	= INT_EI8,
		.flags	= IORESOURCE_IRQ,
	},	
};
struct platform_device tcc_mmc_core0_slot0_device = {
	.name			= "tcc-mmc0",
	.id 			= 0,
	.num_resources	= ARRAY_SIZE(tcc_mmc_core0_slot0_resource),
	.resource		= tcc_mmc_core0_slot0_resource,
	.dev			= {
		.dma_mask		= &tcc_mmc_core0_slot0_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC_CORE0_SLOT1) || defined(CONFIG_MMC_TCC_SDHC_CORE0_SLOT1_MODULE)
static u64 tcc_mmc_core0_slot1_dmamask = 0xffffffffUL;
static struct resource tcc_mmc_core0_slot1_resource[] = {
	[0] = {
		.start	= 0xF05A0100,
		.end	= 0xF05A01FF,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD0_SLOT1,
		.end	= INT_SD0_SLOT1,
		.flags	= IORESOURCE_IRQ,
	},
};
struct platform_device tcc_mmc_core0_slot1_device = {
	.name			= "tcc-mmc0",
	.id 			= 1,
	.num_resources	= ARRAY_SIZE(tcc_mmc_core0_slot1_resource),
	.resource		= tcc_mmc_core0_slot1_resource,
	.dev			= {
		.dma_mask		= &tcc_mmc_core0_slot1_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC_CORE1_SLOT2) || defined(CONFIG_MMC_TCC_SDHC_CORE1_SLOT2_MODULE)
static u64 tcc_mmc_core1_slot2_dmamask = 0xffffffffUL;
static struct resource tcc_mmc_core1_slot2_resource[] = {
	[0] = {
		.start	= 0xF05A0200,
		.end	= 0xF05A02FF,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD1_SLOT2,
		.end	= INT_SD1_SLOT2,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.start	= INT_EI9,
		.end	= INT_EI9,
		.flags	= IORESOURCE_IRQ,
	},		
};
struct platform_device tcc_mmc_core1_slot2_device = {
	.name			= "tcc-mmc1",
	.id 			= 2,
	.num_resources	= ARRAY_SIZE(tcc_mmc_core1_slot2_resource),
	.resource		= tcc_mmc_core1_slot2_resource,
	.dev			= {
		.dma_mask		= &tcc_mmc_core1_slot2_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC_CORE1_SLOT3) || defined(CONFIG_MMC_TCC_SDHC_CORE1_SLOT3_MODULE)
static u64 tcc_mmc_core1_slot3_dmamask = 0xffffffffUL;
static struct resource tcc_mmc_core1_slot3_resource[] = {
	[0] = {
		.start	= 0xF05A0300,
		.end	= 0xF05A03FF,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD1_SLOT3,
		.end	= INT_SD1_SLOT3,
		.flags	= IORESOURCE_IRQ,
	},
};
struct platform_device tcc_mmc_core1_slot3_device = {
	.name			= "tcc-mmc1",
	.id 			= 3,
	.num_resources	= ARRAY_SIZE(tcc_mmc_core1_slot3_resource),
	.resource		= tcc_mmc_core1_slot3_resource,
	.dev			= {
		.dma_mask		= &tcc_mmc_core1_slot3_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif
#endif
/*----------------------------------------------------------------------
 *  * Device     : Serial-ATA resource
 *   * Description: 
 *    *----------------------------------------------------------------------*/
#if defined(CONFIG_SATA_TCC) || defined(CONFIG_SATA_TCC_MODULE)
static struct resource sata_resources[] = {
	[0] = {
		.start = 0xF0560000,
		.end   = 0xF0560800,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_SATA,
		.end   = INT_SATA,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device tcc8900_sata_device = {
	.name			= "tcc-sata",
	.id				= 0,
	.resource			= sata_resources,
	.num_resources	= ARRAY_SIZE(sata_resources),
};
#endif


/*----------------------------------------------------------------------
 * Device	  : WM8750 controller resource definitions
 * Description: 
 *----------------------------------------------------------------------*/

//static struct resource tcc9200_wm8750_resource[] = {
//	[0] = {
//		.start = 0x20080000,
//		.end   = 0x20100000-1,
//		.flags = IORESOURCE_MEM,
//	},
//};

static u64 tcc9200_device_wm8750_dmamask = 0xffffffffUL;

struct platform_device tcc9200_wm8750_device = {
	.name		  = "tcc9200-audio",
	.id 	  = -1,
//	.num_resources	  = ARRAY_SIZE(tcc7901_wm8731_resource),
//	.resource	  = tcc9200_wm8731_resource,
	.dev			  = {
		.dma_mask		= &tcc9200_device_wm8750_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};

/********** above is WM8750 resource *************/


/*----------------------------------------------------------------------
 * Device     : UART resource
 * Description: uart0_resources
 *              uart1_resources
 *              uart2_resources
 *              uart3_resources
 *              uart4_resources
 *              uart5_resources
 *----------------------------------------------------------------------*/
static struct resource uart0_resources[] = {
    /* PA -> VA */
    [0] = {
        .start  = 0xF0532000,
        .end    = 0xF05320FF,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART0,
        .end    = INT_UART0,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc9200_uart0_device = {
    .name           = "tcc-uart",
    .id             = 0,
    .resource       = uart0_resources,
    .num_resources  = ARRAY_SIZE(uart0_resources),
};

static struct resource uart1_resources[] = {
    [0] = {
        .start  = 0xF0532100,
        .end    = 0xF05321FF,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART1,
        .end    = INT_UART1,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc9200_uart1_device = {
    .name           = "tcc-uart",
    .id             = 1,
    .resource       = uart1_resources,
    .num_resources  = ARRAY_SIZE(uart1_resources),
};

static struct resource uart2_resources[] = {
    [0] = {
        .start  = 0xF0532200,
        .end    = 0xF05322FF,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART2,
        .end    = INT_UART2,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc9200_uart2_device = {
    .name           = "tcc-uart",
    .id             = 2,
    .resource       = uart2_resources,
    .num_resources  = ARRAY_SIZE(uart2_resources),
};

static struct resource uart3_resources[] = {
    [0] = {
        .start  = 0xF0532300,
        .end    = 0xF05323FF,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART3,
        .end    = INT_UART3,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc9200_uart3_device = {
    .name           = "tcc-uart",
    .id             = 3,
    .resource       = uart3_resources,
    .num_resources  = ARRAY_SIZE(uart3_resources),
};

static struct resource uart4_resources[] = {
    [0] = {
        .start  = 0xF0532400,
        .end    = 0xF05324FF,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART4,
        .end    = INT_UART4,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc9200_uart4_device = {
    .name           = "tcc-uart",
    .id             = 4,
    .resource       = uart4_resources,
    .num_resources  = ARRAY_SIZE(uart4_resources),
};

static struct resource uart5_resources[] = {
    [0] = {
        .start  = 0xF0532500,
        .end    = 0xF05325FF,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART5,
        .end    = INT_UART5,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc9200_uart5_device = {
    .name           = "tcc-uart",
    .id             = 5,
    .resource       = uart5_resources,
    .num_resources  = ARRAY_SIZE(uart5_resources),
};

/*-----------------------------------------------------------------------
 * Device     : USB OTG resource
 * Description: tcc9200_otg_resources
 *----------------------------------------------------------------------*/
static u64 tcc8900_dwc_otg_dmamask = 0xffffffffUL;
static struct resource dwc_otg_resources[] = {
	[0] = {
		.start	= 0xF0550000,
		.end	= 0xF0550100,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_UOTG,
		.end   = INT_UOTG,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device tcc_otg_device = {
	.name			= "dwc_otg",
	.id			= 0,
	.resource		= dwc_otg_resources,
	.num_resources 		= ARRAY_SIZE(dwc_otg_resources),
	.dev			= {
		.dma_mask		= &tcc8900_dwc_otg_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
};

/*----------------------------------------------------------------------
 * Device     : USB OHCI resource
 * Description: tcc8900_ohci_resources
 *----------------------------------------------------------------------*/
 #if defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)
 static u64 tcc9200_device_ohci_dmamask = 0xffffffffUL;
 
 static struct resource tcc9200_ohci_resources[] = {
	[0] = {
		.start = 0xF0500000,
		.end   = 0xF050005C,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_U11H,
		.end   = INT_U11H,
		.flags = IORESOURCE_IRQ,
	}
};

static struct platform_device ohci_device = {
	.name		= "tcc-ohci",
	.id		= -1,
	.dev		= {
		.dma_mask = &tcc9200_device_ohci_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources  = ARRAY_SIZE(tcc9200_ohci_resources),
	.resource       = tcc9200_ohci_resources,
};

static int tcc9200_ohci_init(struct device *dev)
{
	//TODO: init code????
	return 0;
}

static struct tccohci_platform_data tcc9200_ohci_platform_data = {
//	.port_mode	= USBOHCI_PPM_NPS,
	.port_mode	= USBOHCI_PPM_PERPORT,
	.init		= tcc9200_ohci_init,
};

void __init tcc_set_ohci_info(struct tccohci_platform_data *info)
{
	ohci_device.dev.platform_data = info;
}

static inline void tcc9200_init_usbhost(void)
{
	platform_device_register(&ohci_device);
	tcc_set_ohci_info(&tcc9200_ohci_platform_data);
}
#endif /*CONFIG_USB_OHCI_HCD  || CONFIG_USB_OHCI_HCD_MODULE*/

#ifdef CONFIG_BATTERY_TCC
static struct platform_device tcc_battery_device = {
    .name           = "tcc-battery",
    .id             = -1,
};
#endif /* CONFIG_BATTERY_TCC */

#if defined(CONFIG_TCC_DM9000) || defined(CONFIG_TCC_DM9000_MODULE)
/*----------------------------------------------------------------------
 *  * Device     : Ethernet driver resource
 *   * Description: 
 *    *----------------------------------------------------------------------*/
struct platform_device tcc9200_dm9000_device = {
		.name	  = "tcc-dm9000",
		.id		  = -1,
};
#endif

#if defined(CONFIG_SWITCH_GPIO_TCC)
static struct platform_device tcc_earjack_detect_device = {
        .name = "switch-gpio-earjack-detect",
            .id   = -1,
};
#endif

#if 0
struct flash_platform_data tcc_nand_data = {
	.parts		= NULL,
	.nr_parts	= 0,
};

struct platform_device tcc_nand_device = {
	.name		= "tcc_mtd",
	.id		= -1,
	.dev		= {
		.platform_data = &tcc_nand_data,
	},
};
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
};

static struct android_pmem_platform_data pmem_pdata = {
	.name = "pmem",
	.no_allocator = 1,
	.cached = 1,
};

static struct android_pmem_platform_data pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.no_allocator = 0,
	.cached = 0,
};

static struct platform_device pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &pmem_pdata },
};

static struct platform_device pmem_adsp_device = {
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

#if defined(CONFIG_TCC_ECID_SUPPORT)
static struct platform_device tcc_cpu_id_device = {
	.name	= "cpu-id",
	.id		= -1,
};
#endif

struct platform_device *devices[] __initdata = {
	&tcc9200_uart0_device,
	&tcc9200_uart1_device,
	&tcc9200_uart2_device,
	&tcc9200_uart3_device,
	&tcc9200_uart4_device,
	&tcc9200_uart5_device,

#if defined(CONFIG_TCC_ECID_SUPPORT)
	&tcc_cpu_id_device,
#endif

	//&tcc_nand_device,

#if defined(CONFIG_SPI_TCCXXXX_MASTER) || defined(CONFIG_SPI_TCCXXXX_MASTER_MODULE)
	&tcc9200_spi0_device,
// jhlim	&tcc9200_spi1_device,
#endif
#if defined(CONFIG_SPI_TCCXXXX_TSIF_SLAVE) || defined(CONFIG_SPI_TCCXXXX_TSIF_SLAVE_MODULE)
	&tcc9200_tsif_device,
#endif
	
#if defined(CONFIG_INPUT_TCC_REMOTE)
    &tcc9200_remote_device,
#endif

#if defined(CONFIG_SATA_TCC) || defined(CONFIG_SATA_TCC_MODULE)
	&tcc8900_sata_device,
#endif
	&tcc9200_wm8750_device,
#ifdef CONFIG_USB_OHCI_HCD
//	&ohci_device,
#endif
	&tcc9200_adc_device,
#if defined(CONFIG_RTC_DRV_TCC9200)
	&tcc9200_rtc_device,
#endif
#ifdef CONFIG_TCC_CS8900
	&tcc_cs8900_device,
#endif
#ifdef CONFIG_BATTERY_TCC
        &tcc_battery_device,
#endif
#if defined(CONFIG_TCC_DM9000) || defined(CONFIG_TCC_DM9000_MODULE) 
	&tcc9200_dm9000_device,
#endif

#if defined(CONFIG_SWITCH_GPIO_TCC)
	&tcc_earjack_detect_device,
#endif

};

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
static int __init tcc9200_init_devices(void)
{
	tcc_add_pmem_devices();

	platform_add_devices(devices, ARRAY_SIZE(devices));

#if defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)
	tcc9200_init_usbhost();
#endif

    return 0;
}

arch_initcall(tcc9200_init_devices);
