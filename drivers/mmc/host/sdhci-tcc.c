/*
 * SDHCI support for Telechips SoCs
 *
 * Copyright (c) 2014 Upgrade Android
 *
 * Licensed under GPLv2.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/mmc/host.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/mmc/slot-gpio.h>
#include <linux/pinctrl/consumer.h>
#include "sdhci-pltfm.h"

struct sdhci_tcc_priv {
	struct clk *clk;
	u32 slot, port;
	int gpio_cd;
	int gpio_power;
};

/* Channel Control Register Map */
#define SDPORTCTRL	0x00
#define SDPORTDLY0	0x04
#define SDPORTDLY1	0x04
#define SDPORTDLY2	0x04
#define SDPORTDLY3	0x04

static void __iomem *channelctrl;

static unsigned int sdhci_tcc_get_max_clk(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_tcc_priv *priv = pltfm_host->priv;
	return clk_get_rate(priv->clk);
}

static struct sdhci_ops sdhci_tcc_ops = {
	.get_max_clock	= sdhci_tcc_get_max_clk,
};

static struct sdhci_pltfm_data sdhci_tcc_pdata = {
	.ops = &sdhci_tcc_ops,
	.quirks = 0
		| SDHCI_QUIRK_BROKEN_DMA
		| SDHCI_QUIRK_BROKEN_ADMA
};

static int sdhci_tcc_probe(struct platform_device *pdev)
{
	struct sdhci_host *host;
	struct sdhci_pltfm_host *pltfm_host;
	struct sdhci_tcc_priv *priv;
	struct pinctrl *pinctrl;
	u32 reg;
	int ret;

	pinctrl = devm_pinctrl_get_select_default(&pdev->dev);
	if (IS_ERR(pinctrl)) {
		dev_err(&pdev->dev, "unable to get pinmux");
		return PTR_ERR(pinctrl);
	}

	priv = devm_kzalloc(&pdev->dev, sizeof(struct sdhci_tcc_priv),
		GFP_KERNEL);
	if (!priv) {
		dev_err(&pdev->dev, "unable to allocate private data");
		return -ENOMEM;
	}

	priv->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(priv->clk)) {
		dev_err(&pdev->dev, "unable to get clock");
		return PTR_ERR(priv->clk);
	}

	if (pdev->dev.of_node) {
		priv->gpio_cd = of_get_named_gpio(pdev->dev.of_node,
			"cd-gpios", 0);
		priv->gpio_power = of_get_named_gpio(pdev->dev.of_node,
			"power-gpios", 0);
		BUG_ON(of_property_read_u32(pdev->dev.of_node, "tcc,slot", &priv->slot));
		BUG_ON(of_property_read_u32(pdev->dev.of_node, "tcc,port", &priv->port));
	} else {
		priv->gpio_cd = -EINVAL;
		priv->gpio_power = -EINVAL;
	}

	host = sdhci_pltfm_init(pdev, &sdhci_tcc_pdata);
	if (IS_ERR(host)) {
		ret = PTR_ERR(host);
		goto err_sdhci_pltfm_init;
	}

	pltfm_host = sdhci_priv(host);
	pltfm_host->priv = priv;

	sdhci_get_of_property(pdev);

	clk_prepare_enable(priv->clk);

	/* Map channel control IO */
	if (!channelctrl || IS_ERR(channelctrl))
		BUG_ON(IS_ERR(channelctrl = of_iomap(pdev->dev.of_node, 1)));

	/* Program selected port into our slot */
	reg = readl_relaxed(channelctrl + SDPORTCTRL);
	reg &= ~(7 << (4 * priv->slot));
	reg |= priv->port << (4 * priv->slot);
	writel_relaxed(reg, channelctrl + SDPORTCTRL);

	if (gpio_is_valid(priv->gpio_power)) {
		gpio_direction_output(priv->gpio_power, 1);
	}

	ret = sdhci_add_host(host);
	if (ret)
		goto err_sdhci_add;

	/*
	 * We must request the IRQ after sdhci_add_host(), as the tasklet only
	 * gets setup in sdhci_add_host() and we oops.
	 */
	if (gpio_is_valid(priv->gpio_cd)) {
		ret = mmc_gpio_request_cd(host->mmc, priv->gpio_cd);
		if (ret) {
			dev_err(&pdev->dev, "card detect irq request failed: %d\n",
				ret);
			goto err_request_cd;
		}
	}

	return 0;

err_request_cd:
	sdhci_remove_host(host, 0);
err_sdhci_add:
	clk_disable_unprepare(priv->clk);
	sdhci_pltfm_free(pdev);
err_sdhci_pltfm_init:
	return ret;
}

static int sdhci_tcc_remove(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_tcc_priv *priv = pltfm_host->priv;

	sdhci_pltfm_unregister(pdev);

	if (gpio_is_valid(priv->gpio_cd))
		mmc_gpio_free_cd(host->mmc);

	clk_disable_unprepare(priv->clk);
	return 0;
}

static const struct of_device_id sdhci_tcc_of_match[] = {
	{ .compatible = "tcc,tcc92xx-sdhc" },
	{ }
};
MODULE_DEVICE_TABLE(of, sdhci_tcc_of_match);

static struct platform_driver sdhci_tcc_driver = {
	.driver		= {
		.name	= "sdhci-tcc",
		.owner	= THIS_MODULE,
		.of_match_table = sdhci_tcc_of_match,
	},
	.probe		= sdhci_tcc_probe,
	.remove		= sdhci_tcc_remove,
};

module_platform_driver(sdhci_tcc_driver);

MODULE_DESCRIPTION("SDHCI driver for Telechips SoCs");
MODULE_AUTHOR("Ithamar R. Adema <ithamar@upgrade-android.com>");
MODULE_LICENSE("GPL");
