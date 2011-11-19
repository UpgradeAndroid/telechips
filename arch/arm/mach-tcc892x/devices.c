/*
 * linux/arch/arm/mach-tcc892x/devices.
 *
 * Copyright (C) 2011 Telechips, Inc.
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
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/android_pmem.h>

#include <asm/setup.h>
#include <asm/io.h>
#include <asm/mach/flash.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>

#include <mach/bsp.h>
#include <plat/pmap.h>
#include <mach/tcc_fb.h>
#include <mach/ohci.h>
#include <mach/tcc_cam_ioctrl.h>
#include <mach/tccfb_ioctrl.h>

/*----------------------------------------------------------------------
 * Device	  : SD/MMC resource
 * Description: tcc8920_mmc_core0_resource
 *				tcc8920_mmc_core1_resource
 *				tcc8920_mmc_core2_resource
 *				tcc8920_mmc_core3_resource
 *----------------------------------------------------------------------*/
#if defined(CONFIG_MMC_TCC_SDHC0) || defined(CONFIG_MMC_TCC_SDHC0_MODULE)
static u64 tcc_device_sdhc0_dmamask = 0xffffffffUL;
static struct resource tcc_sdhc0_resource[] = {
	#if defined(CONFIG_MMC_TCC_SDHC2)	//for eMMC Booting
	[0] = {
		.start	= tcc_p2v(HwSDMMC1_BASE),
		.end	= tcc_p2v(HwSDMMC1_BASE + 0x1ff),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD1,
		.end	= INT_SD1,
		.flags	= IORESOURCE_IRQ,
	},
	#else	//for NAND Booting
	[0] = {
		.start	= tcc_p2v(HwSDMMC0_BASE),
		.end	= tcc_p2v(HwSDMMC0_BASE + 0x1ff),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD0,
		.end	= INT_SD0,
		.flags	= IORESOURCE_IRQ,
	},
	#endif
};
struct platform_device tcc_sdhc0_device = {
	.name			= "tcc-sdhc0",
	.id 			= 0,
	.num_resources	= ARRAY_SIZE(tcc_sdhc0_resource),
	.resource		= tcc_sdhc0_resource,
	.dev			= {
		.dma_mask			= &tcc_device_sdhc0_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC1) || defined(CONFIG_MMC_TCC_SDHC1_MODULE)
static u64 tcc_device_sdhc1_dmamask = 0xffffffffUL;
static struct resource tcc_sdhc1_resource[] = {
	#if defined(CONFIG_MMC_TCC_SDHC2)	//for eMMC Booting
	[0] = {
		.start	= tcc_p2v(HwSDMMC0_BASE),
		.end	= tcc_p2v(HwSDMMC0_BASE + 0x1ff),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD0,
		.end	= INT_SD0,
		.flags	= IORESOURCE_IRQ,
	},
	#elif defined(CONFIG_WIFI_SUB_BOARD)	//for NAND Booting + (BT+WiFi)
	[0] = {
		.start	= tcc_p2v(HwSDMMC2_BASE),
		.end	= tcc_p2v(HwSDMMC2_BASE + 0x1ff),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD2,
		.end	= INT_SD2,
		.flags	= IORESOURCE_IRQ,
	},
	#else	//for NAND Booting + WiFi
	[0] = {
		.start	= tcc_p2v(HwSDMMC3_BASE),
		.end	= tcc_p2v(HwSDMMC3_BASE + 0x1ff),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD3,
		.end	= INT_SD3,
		.flags	= IORESOURCE_IRQ,
	},
	#endif
};
struct platform_device tcc_sdhc1_device = {
	.name			= "tcc-sdhc1",
	.id 			= 1,
	.num_resources	= ARRAY_SIZE(tcc_sdhc1_resource),
	.resource		= tcc_sdhc1_resource,
	.dev			= {
		.dma_mask			= &tcc_device_sdhc1_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC2) || defined(CONFIG_MMC_TCC_SDHC2_MODULE)
static u64 tcc_device_sdhc2_dmamask = 0xffffffffUL;
static struct resource tcc_sdhc2_resource[] = {
	#if defined(CONFIG_MMC_TCC_SDHC2)	//for eMMC Booting
	#if defined(CONFIG_WIFI_SUB_BOARD)	//for eMMC Booting + (BT+WiFi)
	[0] = {
		.start	= tcc_p2v(HwSDMMC2_BASE),
		.end	= tcc_p2v(HwSDMMC2_BASE + 0x1ff),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD2,
		.end	= INT_SD2,
		.flags	= IORESOURCE_IRQ,
	},
	#else	//for eMMC Booting + WiFi
	[0] = {
		.start	= tcc_p2v(HwSDMMC3_BASE),
		.end	= tcc_p2v(HwSDMMC3_BASE + 0x1ff),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD3,
		.end	= INT_SD3,
		.flags	= IORESOURCE_IRQ,
	},
	#endif
	#else	//for NAND Booting
	[0] = {
		.start	= tcc_p2v(HwSDMMC1_BASE),
		.end	= tcc_p2v(HwSDMMC1_BASE + 0x1ff),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD1,
		.end	= INT_SD1,
		.flags	= IORESOURCE_IRQ,
	},
	#endif
};
struct platform_device tcc_sdhc2_device = {
	.name			= "tcc-sdhc2",
	.id 			= 2,
	.num_resources	= ARRAY_SIZE(tcc_sdhc2_resource),
	.resource		= tcc_sdhc2_resource,
	.dev			= {
		.dma_mask			= &tcc_device_sdhc2_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_MMC_TCC_SDHC3) || defined(CONFIG_MMC_TCC_SDHC3_MODULE)
static u64 tcc_device_sdhc3_dmamask = 0xffffffffUL;
static struct resource tcc_sdhc3_resource[] = {
	#if defined(CONFIG_WIFI_SUB_BOARD)	//for (BT+WiFi)
	[0] = {
		.start	= tcc_p2v(HwSDMMC3_BASE),
		.end	= tcc_p2v(HwSDMMC3_BASE + 0x1ff),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD3,
		.end	= INT_SD3,
		.flags	= IORESOURCE_IRQ,
	},
	#else	//for eMMC Booting + WiFi
	[0] = {
		.start	= tcc_p2v(HwSDMMC2_BASE),
		.end	= tcc_p2v(HwSDMMC2_BASE + 0x1ff),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_SD2,
		.end	= INT_SD2,
		.flags	= IORESOURCE_IRQ,
	},
	#endif
};
struct platform_device tcc_sdhc3_device = {
	.name			= "tcc-sdhc3",
	.id 			= 3,
	.num_resources	= ARRAY_SIZE(tcc_sdhc3_resource),
	.resource		= tcc_sdhc3_resource,
	.dev			= {
		.dma_mask			= &tcc_device_sdhc3_dmamask,
		.coherent_dma_mask	= 0xffffffffUL
	}
};
#endif

#if defined(CONFIG_I2C_TCC)
/*----------------------------------------------------------------------
 * Device     : I2C resource
 * Description: tcc8920_i2c_core0_resources
 *				tcc8920_i2c_core1_resources
 *				tcc8920_i2c_core2_resources
 *				tcc8920_i2c_core3_resources
 *				tcc8920_i2c_smu_resources
 *
 * [0] = {										// channel
 *		.name	= "master0"						// channel name
 *		.start	= tcc_p2v(HwI2C_MASTER0_BASE),	// base address
 *		.end	= HwI2C_PORTCFG_BASE + 0x0C     // IRQSTR
 *		.flags	= IORESOURCE_IRQ,				// resource type
 * },
 *----------------------------------------------------------------------*/
#if defined(CONFIG_I2C_TCC_CORE0)
static struct resource tcc8920_i2c_core0_resources[] = {
	[0] = {
		.name	= "master0",
		.start  = HwI2C_MASTER0_BASE,
		.end    = HwI2C_PORTCFG_BASE + 0x0C,
		.flags	= IORESOURCE_IO,
	},
};
struct platform_device tcc8920_i2c_core0_device = {
    .name           = "tcc-i2c",
    .id             = 0,
    .resource       = tcc8920_i2c_core0_resources,
    .num_resources  = ARRAY_SIZE(tcc8920_i2c_core0_resources),
};
#endif
#if defined(CONFIG_I2C_TCC_CORE1)
static struct resource tcc8920_i2c_core1_resources[] = {
	[0] = {
		.name	= "master1",
        .start  = HwI2C_MASTER1_BASE,
        .end    = HwI2C_PORTCFG_BASE + 0x0C,
		.flags	= IORESOURCE_IO,
    },
};
struct platform_device tcc8920_i2c_core1_device = {
    .name           = "tcc-i2c",
    .id             = 1,
    .resource       = tcc8920_i2c_core1_resources,
    .num_resources  = ARRAY_SIZE(tcc8920_i2c_core1_resources),
};
#endif
#if defined(CONFIG_I2C_TCC_CORE2)
static struct resource tcc8920_i2c_core2_resources[] = {
	[0] = {
		.name	= "master2",
		.start  = HwI2C_MASTER2_BASE,
		.end    = HwI2C_PORTCFG_BASE + 0x0C,
		.flags	= IORESOURCE_IO,
	},
};
struct platform_device tcc8920_i2c_core2_device = {
    .name           = "tcc-i2c",
    .id             = 0,
    .resource       = tcc8920_i2c_core2_resources,
    .num_resources  = ARRAY_SIZE(tcc8920_i2c_core2_resources),
};
#endif
#if defined(CONFIG_I2C_TCC_CORE3)
static struct resource tcc8920_i2c_core3_resources[] = {
	[0] = {
		.name	= "master3",
        .start  = HwI2C_MASTER3_BASE,
        .end    = HwI2C_PORTCFG_BASE + 0x0C,
		.flags	= IORESOURCE_IO,
    },
};
struct platform_device tcc8920_i2c_core3_device = {
    .name           = "tcc-i2c",
    .id             = 1,
    .resource       = tcc8920_i2c_core3_resources,
    .num_resources  = ARRAY_SIZE(tcc8920_i2c_core3_resources),
};
#endif
#if defined(CONFIG_I2C_TCC_SMU)
static struct resource tcc8920_i2c_smu_resources[] = {
	[0] = {
		.name	= "master0",
		.start  = HwSMUI2C_BASE,
        .end    = HwSMUI2C_BASE + 0x80,
		.flags	= IORESOURCE_IO,
    }
};
struct platform_device tcc8920_i2c_smu_device = {
    .name           = "tcc-i2c",
    .id             = 2,
    .resource       = tcc8920_i2c_smu_resources,
    .num_resources  = ARRAY_SIZE(tcc8920_i2c_smu_resources),
};
#endif
#endif  /* #if defined(CONFIG_I2C_TCC) */

#if defined(CONFIG_TCC_GMAC) || defined(CONFIG_TCC_GMAC_MODULE)

static u64 tcc_gmac_dma_mask = ~(u32)0;
static struct resource tcc_gmac_resources[] = {
	[0] = {
		.start	= tcc_p2v(HwGMAC_BASE),
		.end	= tcc_p2v(HwGMAC_BASE) + 0x2000,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_GMAC,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device tcc_gmac_device = {
	.name			= "tcc-gmac",
	.id 			= -1, 
	.dev			= {
		.dma_mask = &tcc_gmac_dma_mask,
		.coherent_dma_mask = 0xffffffffUL,
	},
	.num_resources	= ARRAY_SIZE(tcc_gmac_resources),
	.resource		= tcc_gmac_resources,
};
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
        .start  = tcc_p2v(HwUART0_BASE),
        .end    = tcc_p2v(HwUART0_BASE) + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART0,
        .end    = INT_UART0,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc8920_uart0_device = {
    .name           = "tcc-uart",
    .id             = 0,
    .resource       = uart0_resources,
    .num_resources  = ARRAY_SIZE(uart0_resources),
};

static struct resource uart1_resources[] = {
    [0] = {
        .start  = tcc_p2v(HwUART1_BASE),
        .end    = tcc_p2v(HwUART1_BASE) + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART1,
        .end    = INT_UART1,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc8920_uart1_device = {
    .name           = "tcc-uart",
    .id             = 1,
    .resource       = uart1_resources,
    .num_resources  = ARRAY_SIZE(uart1_resources),
};

static struct resource uart2_resources[] = {
    [0] = {
        .start  = tcc_p2v(HwUART2_BASE),
        .end    = tcc_p2v(HwUART2_BASE) + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART2,
        .end    = INT_UART2,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc8920_uart2_device = {
    .name           = "tcc-uart",
    .id             = 2,
    .resource       = uart2_resources,
    .num_resources  = ARRAY_SIZE(uart2_resources),
};

static struct resource uart3_resources[] = {
    [0] = {
        .start  = tcc_p2v(HwUART3_BASE),
        .end    = tcc_p2v(HwUART3_BASE) + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART3,
        .end    = INT_UART3,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc8920_uart3_device = {
    .name           = "tcc-uart",
    .id             = 3,
    .resource       = uart3_resources,
    .num_resources  = ARRAY_SIZE(uart3_resources),
};

static struct resource uart4_resources[] = {
    [0] = {
        .start  = tcc_p2v(HwUART4_BASE),
        .end    = tcc_p2v(HwUART4_BASE) + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART4,
        .end    = INT_UART4,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc8920_uart4_device = {
    .name           = "tcc-uart",
    .id             = 4,
    .resource       = uart4_resources,
    .num_resources  = ARRAY_SIZE(uart4_resources),
};

static struct resource uart5_resources[] = {
    [0] = {
        .start  = tcc_p2v(HwUART5_BASE),
        .end    = tcc_p2v(HwUART5_BASE) + 0xff,
        .flags  = IORESOURCE_MEM,
    },
    [1] = {
        .start  = INT_UART5,
        .end    = INT_UART5,
        .flags  = IORESOURCE_IRQ,
    },
};

struct platform_device tcc8920_uart5_device = {
    .name           = "tcc-uart",
    .id             = 5,
    .resource       = uart5_resources,
    .num_resources  = ARRAY_SIZE(uart5_resources),
};
#endif


/*----------------------------------------------------------------------
 * Device     : USB HOST2.0 EHCI resource
 * Description: tcc8920_ehci_resources
 *----------------------------------------------------------------------*/
#if defined(CONFIG_USB_EHCI_TCC) || defined(CONFIG_USB_EHCI_TCC_MODULE)
static u64 tcc8920_device_ehci_dmamask = 0xffffffffUL;
 
static struct resource tcc8920_ehci_hs_resources[] = {
	[0] = {
		.start = tcc_p2v(HwUSB20HOST_EHCI_BASE),
		.end   = tcc_p2v(HwUSB20HOST_EHCI_BASE) + 0x0fff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_USB20H,
		.end   = INT_USB20H,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device ehci_hs_device = {
	.name			= "tcc-ehci",
	.id				= -1,
	.num_resources  = ARRAY_SIZE(tcc8920_ehci_hs_resources),
	.resource       = tcc8920_ehci_hs_resources,
	.dev			= {
		.dma_mask 			= &tcc8920_device_ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
		//.platform_data	= tcc8920_ehci_hs_data,
	},
};

static int tcc8920_echi_fs_init(struct device *dev)
{
	return 0;
}

static struct tccohci_platform_data tcc8920_ehci_fs_platform_data = {
	.port_mode	= USBOHCI_PPM_PERPORT,
	.init		= tcc8920_echi_fs_init,
};

static struct resource tcc8920_ehci_fs_resources[] = {
	[0] = {
		.start = tcc_p2v(HwUSB20HOST_OHCI_BASE),
		.end   = tcc_p2v(HwUSB20HOST_OHCI_BASE) + 0x0fff,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_USB20H,
		.end   = INT_USB20H,
		.flags = IORESOURCE_IRQ,
	}
};

struct platform_device ehci_fs_device = {
	.name			= "tcc-ohci",
	.id				= 1,
	.num_resources  = ARRAY_SIZE(tcc8920_ehci_fs_resources),
	.resource       = tcc8920_ehci_fs_resources,
	.dev			= {
		.dma_mask 			= &tcc8920_device_ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
		.platform_data		= &tcc8920_ehci_fs_platform_data,
	},
};
#else
static inline void tcc8920_init_ehci(void) {}
#endif /*CONFIG_USB_EHCI_TCC  || CONFIG_USB_EHCI_TCC_MODULE*/

/*----------------------------------------------------------------------
 * Device     : USB DWC OTG resource
 * Description: dwc_otg_resources
 *----------------------------------------------------------------------*/
#if defined(CONFIG_TCC_DWC_OTG) || defined(CONFIG_TCC_DWC_OTG_MODULE)
static u64 tcc8920_dwc_otg_dmamask = 0xffffffffUL;
struct resource dwc_otg_resources[] = {
	[0] = {
		.start	= tcc_p2v(HwUSBOTG_BASE),
		.end	= tcc_p2v(HwUSBOTG_BASE) + 0x100,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_UOTG,
		.end   = INT_UOTG,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device tcc8920_dwc_otg_device = {
	.name			= "dwc_otg",
	.id				= 0,
	.resource		= dwc_otg_resources,
	.num_resources	= ARRAY_SIZE(dwc_otg_resources),
	.dev			= {
		.dma_mask 			= &tcc8920_dwc_otg_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
};
#endif

#if defined(CONFIG_FB_TCC)
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
#endif//

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
	.resolution = 0,
	.output = 0,
	.hdmi_resolution = 0,
	.composite_resolution = 0,
	.component_resolution = 0,
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

//#if(1) // defined(CONFIG_TCC_ADC) || defined(CONFIG_TCC_ADC_MODULE)
/*----------------------------------------------------------------------
 * Device	  : ADC resource
 * Description: tcc8920_adc_resource
 *----------------------------------------------------------------------*/
struct resource tcc8920_adc_resources[] = {
	[0] = {
		.start	= tcc_p2v(HwTSADC_BASE),		
		.end	= tcc_p2v(HwTSADC_BASE + 0x24),
		.flags	= IORESOURCE_MEM,
	},
};

struct platform_device tcc8920_adc_device = {
	.name		= "tcc-adc",
	.id			= -1,
	.resource	= tcc8920_adc_resources,
	.num_resources	= ARRAY_SIZE(tcc8920_adc_resources),
};
//#endif 

#ifdef CONFIG_BATTERY_TCC
struct platform_device tcc_battery_device = {
    .name           = "tcc-battery",
    .id             = -1,
};
#endif /* CONFIG_BATTERY_TCC */

/*----------------------------------------------------------------------
 * Device     : SPI(GPSB) Master resource
 * Description: 
 *----------------------------------------------------------------------*/
#if defined(CONFIG_SPI_TCCXXXX_MASTER) || defined(CONFIG_SPI_TCCXXXX_MASTER_MODULE)
static struct resource spi0_resources[] = {
	[0] = {
		.start = tcc_p2v(HwGPSB0_BASE),
		.end   = tcc_p2v(HwGPSB0_BASE + 0x38),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_GPSB0_DMA,
		.end   = INT_GPSB0_DMA,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start	= 0xD,/* GPIO_D[5:8] */
		.end	= 0xD,
		.flags	= IORESOURCE_IO,
	},
};

struct platform_device tcc8920_spi0_device = {
	.name		= "tcc-spi",
	.id			= 0,
	.resource	= spi0_resources,
	.num_resources	= ARRAY_SIZE(spi0_resources),
};
#endif

/*----------------------------------------------------------------------
 * Device     : RTC resource
 * Description: tcc8920_rtc_resources
 *----------------------------------------------------------------------*/
#if defined(CONFIG_RTC_DRV_TCC8920)
struct resource tcc8920_rtc_resource[] = {
    [0] = {
        .start	= tcc_p2v(HwRTC_BASE),
        .end	= tcc_p2v(HwRTC_BASE + 0x50),
        .flags	= IORESOURCE_MEM,
    },
    [1] = {
        .start	= INT_RTC,
        .end	= INT_RTC,
        .flags	= IORESOURCE_IRQ,
    }
};

struct platform_device tcc8920_rtc_device = {
    .name           = "tcc-rtc",
    .id             = -1,
    .resource       = tcc8920_rtc_resource,
    .num_resources  = ARRAY_SIZE(tcc8920_rtc_resource),
};
#endif  /* CONFIG_RTC_DRV_TCC */

/*----------------------------------------------------------------------
 * Device     : remote control resource
 * Description:
 *----------------------------------------------------------------------*/
#if defined(CONFIG_INPUT_TCC_REMOTE)
struct resource tcc8920_remote_resources[] = {
#if 1
	[0] = {
		.start	= tcc_p2v(HwREMOTE_BASE),
		.end	= tcc_p2v(HwREMOTE_BASE+0x44),
		.flags	= IORESOURCE_MEM,
	},
    [1] = {
        .start = INT_RMT,
        .end   = INT_RMT,
        .flags = IORESOURCE_IRQ,
    },
#endif
};

struct platform_device tcc8920_remote_device = {
	.name			= "tcc-remote",
	.id				= -1,
	.resource		= tcc8920_remote_resources,
	.num_resources	= ARRAY_SIZE(tcc8920_remote_resources),
};
#if 0
static inline void tcc8920_init_remote(void)
{
	platform_device_register(&tcc8920_remote_device);
}
#endif
#endif /* CONFIG_INPUT_TCC_REMOTE */

/*----------------------------------------------------------------------
 * Device     : SPI(TSIF) Slave resource
 * Description:
 *----------------------------------------------------------------------*/
#if defined(CONFIG_SPI_TCCXXXX_TSIF_SLAVE) || defined(CONFIG_SPI_TCCXXXX_TSIF_SLAVE_MODULE)
static struct resource tsif_resources[] = {
	[0] = {
   		.start	= 0xE,	/* GPIO_E[2,3,6,7] */
		.end	= 0xE,
		.flags	= IORESOURCE_IO,
	}
};

struct platform_device tcc_tsif_device = {
	.name		= "tcc-tsif",
	.id		= -1,
	.resource	= tsif_resources,
	.num_resources	= ARRAY_SIZE(tsif_resources),
};
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

struct platform_device tcc_tsif_module_device = {
	.name		= "tcc-tsif-module",
	.id			= -1,
	.resource	= tsif_module_resources,
	.num_resources	= ARRAY_SIZE(tsif_module_resources),
};
#endif

#if defined(CONFIG_TCC_ECID_SUPPORT)
struct platform_device tcc_cpu_id_device = {
	.name	= "cpu-id",
	.id		= -1,
};
#endif

static int __init tcc892x_init_devices(void)
{
	tcc_add_pmem_devices();
	return 0;
}
arch_initcall(tcc892x_init_devices);
