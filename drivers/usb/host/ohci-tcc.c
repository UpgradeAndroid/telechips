#include <linux/signal.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/clk.h>
#include <linux/of.h>

#define USBOTG		0x00
#define USB11H		0x04
#define IOAPB		0x08
#define STORAGE		0x0c
#define HCLKEN0		0x10
#define HCLKEN1		0x14
#define HCLKMEN		0x18
#define HRSTEN0		0x20
#define HRSTEN1		0x24
#define USBOTG0		0x28
#define USBOTG1		0x2c
#define USBOTG2		0x30
#define USBOTG3		0x34
#define IO_A2X		0x38

struct tcc_ohci {
	struct ohci_hcd ohci;
	void __iomem *cfg;
	struct clk *clk;
	int pwr_gp1_gpio;
	int hvbus_gpio;
};

#define to_tcc_ohci(hcd)	(struct tcc_ohci *)hcd_to_ohci(hcd)

static void tcc_start_ohci(struct tcc_ohci *ohci)
{
	u32 val;

	val = readl(ohci->cfg + HRSTEN0);
	writel(val & ~BIT(0), ohci->cfg + HRSTEN0);
	writel(val | BIT(0), ohci->cfg + HRSTEN0);

	val = readl(ohci->cfg + USB11H);
	val |= BIT(3);	/* DP pull down enable */
	val |= BIT(4);	/* DNPD pull down enable */
	writel(val, ohci->cfg + USB11H);

	if (gpio_is_valid(ohci->pwr_gp1_gpio))
		gpio_direction_output(ohci->pwr_gp1_gpio, 1);

	gpio_direction_output(ohci->hvbus_gpio, 1);

	clk_prepare_enable(ohci->clk);
}

static void tcc_stop_ohci(struct tcc_ohci *ohci)
{
	clk_disable_unprepare(ohci->clk);

	gpio_direction_output(ohci->hvbus_gpio, 0);
}

static int ohci_tcc_start(struct usb_hcd *hcd)
{
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);
	int ret;

	ret = ohci_init(ohci);
	if (ret < 0)
		return ret;
	ohci->regs = hcd->regs;

	ret = ohci_run(ohci);
	if (ret < 0) {
		dev_err(hcd->self.controller, "can't start\n");
		ohci_stop(hcd);
		return ret;
	}

	create_debug_files(ohci);

#ifdef DEBUG
	ohci_dump(ohci, 1);
#endif
	return 0;
}

static const struct hc_driver ohci_tcc_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "Telechips OHCI",
	.hcd_priv_size		= sizeof(struct tcc_ohci),

	/* generic hardware linkage */
	.irq			= ohci_irq,
	.flags			= HCD_USB11 | HCD_MEMORY,

	/* basic lifecycle operations */
	.start			= ohci_tcc_start,
	.stop			= ohci_stop,
	.shutdown		= ohci_shutdown,
#ifdef	CONFIG_PM
	.bus_suspend		= ohci_bus_suspend,
	.bus_resume		= ohci_bus_resume,
#endif

	/* managing i/o requests and associated device resources */
	.urb_enqueue		= ohci_urb_enqueue,
	.urb_dequeue		= ohci_urb_dequeue,
	.endpoint_disable	= ohci_endpoint_disable,

	/* scheduling support */
	.get_frame_number	= ohci_get_frame,

	/* root hub support */
	.hub_status_data	= ohci_hub_status_data,
	.hub_control		= ohci_hub_control,

	.start_port_reset	= ohci_start_port_reset,
};

static int tcc_ohci_hcd_drv_probe(struct platform_device *pdev)
{
	const struct hc_driver *driver = &ohci_tcc_hc_driver;
	struct usb_hcd *hcd = NULL;
	struct clk *usbh_clk;
	struct tcc_ohci *ohci_p;
	struct resource *res;
	void __iomem *cfg;
	int retval, irq;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		retval = irq;
		goto fail;
	}

	/*
	 * Right now device-tree probed devices don't get dma_mask set.
	 * Since shared usb code relies on it, set it here for now.
	 * Once we have dma capability bindings this can go away.
	 */
	if (!pdev->dev.dma_mask)
		pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;
	if (!pdev->dev.coherent_dma_mask)
		pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);

	usbh_clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(usbh_clk)) {
		dev_err(&pdev->dev, "Error getting interface clock\n");
		retval = PTR_ERR(usbh_clk);
		goto fail;
	}

	hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd) {
		retval = -ENOMEM;
		goto fail;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		retval = -ENODEV;
		goto err_put_hcd;
	}

	hcd->rsrc_start = pdev->resource[0].start;
	hcd->rsrc_len = resource_size(res);
	if (!devm_request_mem_region(&pdev->dev, hcd->rsrc_start, hcd->rsrc_len,
				hcd_name)) {
		dev_dbg(&pdev->dev, "request_mem_region failed\n");
		retval = -EBUSY;
		goto err_put_hcd;
	}

	hcd->regs = devm_ioremap(&pdev->dev, hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		dev_dbg(&pdev->dev, "ioremap failed\n");
		retval = -ENOMEM;
		goto err_put_hcd;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res) {
		retval = -ENODEV;
		goto err_put_hcd;
	}

	if (!devm_request_mem_region(&pdev->dev, res->start, resource_size(res),
				hcd_name)) {
		dev_dbg(&pdev->dev, "request_mem_region failed\n");
		retval = -EBUSY;
		goto err_put_hcd;
	}

	cfg = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (!hcd->regs) {
		dev_dbg(&pdev->dev, "ioremap failed\n");
		retval = -ENOMEM;
		goto err_put_hcd;
	}

	ohci_p = (struct tcc_ohci *)hcd_to_ohci(hcd);
	ohci_p->clk = usbh_clk;
	ohci_p->cfg = cfg;

	/* usb host vbus gpio; not optional! */
	retval = of_get_gpio(pdev->dev.of_node, 0);
	if (!gpio_is_valid(retval))
		goto err_put_hcd;

	ohci_p->hvbus_gpio = retval;
	retval = gpio_request_one(ohci_p->hvbus_gpio, GPIOF_OUT_INIT_LOW, "ohci_vbus");
	if (retval)
		goto err_put_hcd;

	/* Power plane gpio; optional */
	ohci_p->pwr_gp1_gpio = of_get_gpio(pdev->dev.of_node, 1);
	if (gpio_is_valid(ohci_p->pwr_gp1_gpio)) {
		retval = gpio_request_one(ohci_p->pwr_gp1_gpio, GPIOF_OUT_INIT_LOW, "pwr_gp1");
		if (retval)
			goto err_put_hcd;
	}

	tcc_start_ohci(ohci_p);
	ohci_hcd_init(hcd_to_ohci(hcd));

	retval = usb_add_hcd(hcd, platform_get_irq(pdev, 0), 0);
	if (retval == 0)
		return retval;

	tcc_stop_ohci(ohci_p);
err_put_hcd:
	usb_put_hcd(hcd);
fail:
	dev_err(&pdev->dev, "init fail, %d\n", retval);

	return retval;
}

static int tcc_ohci_hcd_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct tcc_ohci *ohci_p = to_tcc_ohci(hcd);

	usb_remove_hcd(hcd);
	if (ohci_p->clk)
		tcc_stop_ohci(ohci_p);

	usb_put_hcd(hcd);

	platform_set_drvdata(pdev, NULL);
	return 0;
}

#if defined(CONFIG_PM)
static int tcc_ohci_hcd_drv_suspend(struct platform_device *dev,
		pm_message_t message)
{
	struct usb_hcd *hcd = platform_get_drvdata(dev);
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);
	struct tcc_ohci *ohci_p = to_tcc_ohci(hcd);

	if (time_before(jiffies, ohci->next_statechange))
		msleep(5);
	ohci->next_statechange = jiffies;

	tcc_stop_ohci(ohci_p);
	return 0;
}

static int tcc_ohci_hcd_drv_resume(struct platform_device *dev)
{
	struct usb_hcd *hcd = platform_get_drvdata(dev);
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);
	struct tcc_ohci *ohci_p = to_tcc_ohci(hcd);

	if (time_before(jiffies, ohci->next_statechange))
		msleep(5);
	ohci->next_statechange = jiffies;

	tcc_start_ohci(ohci_p);
	ohci_resume(hcd, false);
	return 0;
}
#endif

static struct of_device_id tcc_ohci_id_table[] = {
	{ .compatible = "tcc,ohci", },
	{ },
};

/* Driver definition to register with the platform bus */
static struct platform_driver tcc_ohci_hcd_driver = {
	.probe =	tcc_ohci_hcd_drv_probe,
	.remove =	tcc_ohci_hcd_drv_remove,
#ifdef CONFIG_PM
	.suspend =	tcc_ohci_hcd_drv_suspend,
	.resume =	tcc_ohci_hcd_drv_resume,
#endif
	.driver = {
		.owner = THIS_MODULE,
		.name = "tcc-ohci",
		.of_match_table = of_match_ptr(tcc_ohci_id_table),
	},
};

MODULE_ALIAS("platform:tcc-ohci");
