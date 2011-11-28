/*
 * linux/drivers/usb/host/ehci-tcc.c
 *
 * Description: EHCI HCD (Host Controller Driver) for USB.
 *              Bus Glue for Telechips-SoC
 *
 *  Copyright (C) 2009 Atmel Corporation,
 *                     Nicolas Ferre <nicolas.ferre@atmel.com>
 *
 *  Modified for Telechips SoC from ehci-atmel.c
 *                     by Telechips Team Linux <linux@telechips.com> 25-01-2011
 *
 *  Based on various ehci-*.c drivers
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 */

#include <linux/clk.h>
#include <linux/platform_device.h>

#include <mach/bsp.h>
#include <asm/mach-types.h>

extern int tcc_ehci_vbus_Init(void);
extern int tcc_ehci_vbus_ctrl(int on);
extern void tcc_ehci_clkset(int on);

/* interface and function clocks */
static int clocked;

/*-------------------------------------------------------------------------*/
static void tcc_USB20HPHY_cfg(void)
{
	unsigned int temp;
	PHSIOBUSCFG pEHCIPHYCFG;
	
	pEHCIPHYCFG = (PHSIOBUSCFG)tcc_p2v(HwHSIOBUSCFG_BASE);
	
	temp = 0x00000000;
	temp = temp | Hw29 | Hw28;		// [31:28] UTM_TXFSLSTUNE[3:0]
	temp = temp | Hw25 | Hw24;		// [26:24] UTM_SQRXTUNE[2:0]
	//temp = temp | Hw22; 			// [22:20] UTM_OTGTUNE[2:0] // OTG Reference Value [00]
	temp = temp | Hw21 | Hw20;		// tmp
	//temp = temp | Hw17 | Hw16;	// [18:16] UTM_COMPDISTUNE[2:0]
	temp = temp | Hw18;				// [18:16] UTM_COMPDISTUNE[2:0] // OTG Reference Value[111]
	temp = temp | Hw13;				// [13] UTM_COMMONONN
	temp = temp | Hw11;				// [12:11] REFCLKSEL
	//temp = temp | Hw10;			// [10:9] REFCLKDIV
	//temp = temp | Hw8;			// [8] : SIDDQ
	temp = temp | Hw6;				// [6]
	temp = temp | Hw5;				// [5]
	temp = temp | Hw4;				// [4]
	temp = temp | Hw3;				// [3]
	temp = temp | Hw2;				// [2]
	pEHCIPHYCFG->USB20H_CFG0 = temp;

	temp = 0x00000000;
	temp = temp | Hw29;
	temp = temp | Hw28;				// [28] OTG Disable
	//temp = temp | Hw27;			// [27] IDPULLUP
	temp = temp | Hw19 | Hw18;		// [19:18]
	//temp = temp | Hw17;
	temp = temp | Hw16;
	temp = temp | Hw6 | Hw5;
	temp = temp | Hw0;				// [0] TP HS transmitter Pre-Emphasis Enable
	pEHCIPHYCFG->USB20H_CFG1 = temp;

	temp = 0x00000000;
	//temp = temp | Hw9;
	//temp = temp | Hw6;
	temp = temp | Hw5;
	pEHCIPHYCFG->USB20H_CFG2 = temp;

	pEHCIPHYCFG->USB20H_CFG0 = pEHCIPHYCFG->USB20H_CFG0 | Hw8;
	msleep(10);
	pEHCIPHYCFG->USB20H_CFG1 = pEHCIPHYCFG->USB20H_CFG1 | Hw31; // cfgUSB_SRSTn
	msleep(20);
}

static void tcc_ehci_phy_off(void)
{
	unsigned int temp;
	PHSIOBUSCFG pEHCIPHYCFG = (PHSIOBUSCFG)tcc_p2v(HwHSIOBUSCFG_BASE);
	pEHCIPHYCFG->USB20H_CFG0 &= ~Hw8;	//SIDDQ
}

static void tcc_ehci_phy_ctrl(int on)
{
	tca_ckc_setpmupwroff(PMU_USB1NANOPHY, on);
}

//static void tcc_ehci_hsiobus_reset(void)
//{
//	tca_ckc_sethsiobusswreset(HSIO_USB20HOST, ON);
//	tca_ckc_sethsiobusswreset(HSIO_USB20HOST, OFF);
//}

/*-------------------------------------------------------------------------*/

static void tcc_start_ehci(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "start\n");
	tcc_ehci_clkset(1);
	clocked = 1;
}

static void tcc_stop_ehci(struct platform_device *pdev)
{
	dev_dbg(&pdev->dev, "stop\n");
	tcc_ehci_clkset(0);
	clocked = 0;
}

/*-------------------------------------------------------------------------*/

static int ehci_tcc_setup(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int retval = 0;

	/* registers start at offset 0x0 */
	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs +
		HC_LENGTH(ehci, ehci_readl(ehci, &ehci->caps->hc_capbase));
	dbg_hcs_params(ehci, "reset");
	dbg_hcc_params(ehci, "reset");

	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	retval = ehci_halt(ehci);
	if (retval)
		return retval;

	/* data structure init */
	retval = ehci_init(hcd);
	if (retval)
		return retval;

	ehci->sbrn = 0x20;

	ehci_reset(ehci);
	ehci_port_power(ehci, 0);

	return retval;
}

static const struct hc_driver ehci_tcc_hc_driver = {
	.description		= hcd_name,
	.product_desc		= EHCI_PRODUCT_DESC,
	.hcd_priv_size		= sizeof(struct ehci_hcd),

	/* generic hardware linkage */
	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,

	/* basic lifecycle operations */
	.reset			= ehci_tcc_setup,
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,

	/* managing i/o requests and associated device resources */
	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,
	.endpoint_reset 	= ehci_endpoint_reset,

	/* scheduling support */
	.get_frame_number	= ehci_get_frame,

	/* root hub support */
	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume			= ehci_bus_resume,
	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,

	.clear_tt_buffer_complete = ehci_clear_tt_buffer_complete,
};

#if 0
#ifdef	CONFIG_PM
int ehci_hcd_tcc_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
	/* USB HOST Power Enable */
	tcc_ehci_vbus_ctrl(0);

	return 0;
}

static int ehci_hcd_tcc_drv_resume(struct platform_device *pdev)
{
	//tcc_ehci_clock_ctrl
	tcc_start_ehci(pdev);

	/* USB HS Nano-Phy Enable */
	tcc_ehci_phy_ctrl(1);

	/* USB HOST Power Enable */
	tcc_ehci_vbus_ctrl(1);

	//tcc_start_ehci(pdev);
	tcc_USB20HPHY_cfg();

	return 0;
}
#else
#define ohci_hcd_tcc_drv_suspend		NULL
#define ohci_hcd_tcc_drv_resume			NULL
#endif
#endif /* 0 */

static int __init ehci_tcc_drv_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	const struct hc_driver *driver = &ehci_tcc_hc_driver;
	struct resource *res;
	int irq;
	int retval;

	if (usb_disabled())
		return -ENODEV;
	
	pr_debug("Initializing TCC-SoC USB Host Controller\n");

	irq = platform_get_irq(pdev, 0);
	if (irq <= 0) {
		dev_err(&pdev->dev,
			"Found HC with no IRQ. Check %s setup!\n",
			dev_name(&pdev->dev));
		retval = -ENODEV;
		goto fail_create_hcd;
	}

	hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd) {
		retval = -ENOMEM;
		goto fail_create_hcd;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev,
			"Found HC with no register addr. Check %s setup!\n",
			dev_name(&pdev->dev));
		retval = -ENODEV;
		goto fail_request_resource;
	}
	hcd->rsrc_start = res->start;
	hcd->rsrc_len = res->end - res->start + 1;
	hcd->regs = (void __iomem *)(int)(hcd->rsrc_start);

	tcc_ehci_vbus_Init();
	
	/* USB HS Nano-Phy Enable */
	tcc_ehci_phy_ctrl(1);

	/* USB HOST Power Enable */
	if (tcc_ehci_vbus_ctrl(1) != 0) {
		printk(KERN_ERR "ehci-tcc: USB HOST VBUS failed\n");
		retval = -EIO;
		goto fail_request_resource;
	}

	tcc_start_ehci(pdev);
	tcc_USB20HPHY_cfg();

	retval = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (retval)
		goto fail_add_hcd;
	
	return retval;

fail_add_hcd:
	tcc_stop_ehci(pdev);
fail_request_resource:
	usb_put_hcd(hcd);
fail_create_hcd:
	dev_err(&pdev->dev, "init %s fail, %d\n",
		dev_name(&pdev->dev), retval);
	
	return retval;
}

static int __exit ehci_tcc_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	ehci_shutdown(hcd);
	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);

	tcc_ehci_phy_off();
	tcc_ehci_phy_ctrl(0);
	tcc_stop_ehci(pdev);
	tcc_ehci_vbus_ctrl(0);

	return 0;
}

static struct platform_driver ehci_tcc_driver = {
	.probe		= ehci_tcc_drv_probe,
	.remove		= __exit_p(ehci_tcc_drv_remove),
	.shutdown	= usb_hcd_platform_shutdown,
	//.suspend		= ehci_hcd_tcc_drv_suspend,
	//.resume			= ehci_hcd_tcc_drv_resume,
	.driver.name	= "tcc-ehci",
};
