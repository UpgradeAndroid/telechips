/*
 *  Support for ADC in Telechips SoCs
 *
 *  Copyright (C) 2014 Upgrade Android
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 * TODO: implement TS handling?
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/regulator/consumer.h>
#include <linux/of_platform.h>

#include <linux/iio/iio.h>
#include <linux/iio/machine.h>
#include <linux/iio/driver.h>

#define ADCCON			0x00
#define ADCTSC			0x04
#define ADCDLY			0x08
#define ADCDAT0			0x0c
#define ADCDAT1			0x10
#define ADCUPDN			0x14
#define ADCCLRINT		0x18
#define ADCCLRUPDN		0x20

#define MAX_ADC_CHANNELS	8

#define ADC_TIMEOUT		(msecs_to_jiffies(1000))

struct tcc_adc {
	void __iomem		*regs;
	struct clk		*clk;
	unsigned int		irq;

	struct completion	completion;

	u32			value;
};

static const struct of_device_id tcc_adc_match[] = {
	{ .compatible = "tcc,tcc92xx-adc" },
	{},
};

static int tcc_read_raw(struct iio_dev *indio_dev,
				struct iio_chan_spec const *chan,
				int *val,
				int *val2,
				long mask)
{
	struct tcc_adc *info = iio_priv(indio_dev);
	unsigned long timeout;
	u32 reg;

	if (mask != IIO_CHAN_INFO_RAW)
		return -EINVAL;

	mutex_lock(&indio_dev->mlock);

	/* Select the channel to be used and Trigger conversion */
	reg = readl(info->regs + ADCCON);
	reg &= ~(7 << 3);		/* clear AINx */
	reg |= chan->address << 3;	/* set AINx */
	reg |= BIT(0);			/* start conversion */
	writel(reg, info->regs + ADCCON);

	timeout = wait_for_completion_interruptible_timeout
			(&info->completion, ADC_TIMEOUT);
	*val = info->value;

	mutex_unlock(&indio_dev->mlock);

	if (timeout == 0)
		return -ETIMEDOUT;

	return IIO_VAL_INT;
}

static irqreturn_t tcc_adc_isr(int irq, void *dev_id)
{
	struct tcc_adc *info = dev_id;

	/* TODO check EOC to see if this interrupt is for us? */

	/* Read value */
	info->value = readl(info->regs + ADCDAT0) & 0x3ff;

	/* clear irq */
	writel(1, info->regs + ADCCLRINT);

	complete(&info->completion);

	return IRQ_HANDLED;
}

static int tcc_adc_reg_access(struct iio_dev *indio_dev,
			      unsigned reg, unsigned writeval,
			      unsigned *readval)
{
	struct tcc_adc *info = iio_priv(indio_dev);

	if (readval == NULL)
		return -EINVAL;

	*readval = readl(info->regs + reg);

	return 0;
}

static const struct iio_info tcc_adc_iio_info = {
	.read_raw = &tcc_read_raw,
	.debugfs_reg_access = &tcc_adc_reg_access,
	.driver_module = THIS_MODULE,
};

#define ADC_CHANNEL(_index, _id) {			\
	.type = IIO_VOLTAGE,				\
	.indexed = 1,					\
	.channel = _index,				\
	.address = _index,				\
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),	\
	.datasheet_name = _id,				\
}

static const struct iio_chan_spec tcc_adc_iio_channels[] = {
	ADC_CHANNEL(0, "adc0"),
	ADC_CHANNEL(1, "adc1"),
	ADC_CHANNEL(2, "adc2"),
	ADC_CHANNEL(3, "adc3"),
	ADC_CHANNEL(4, "adc4"),
	ADC_CHANNEL(5, "adc5"),
	ADC_CHANNEL(6, "adc6"),
	ADC_CHANNEL(7, "adc7"),
};

static int tcc_adc_remove_devices(struct device *dev, void *c)
{
	struct platform_device *pdev = to_platform_device(dev);

	platform_device_unregister(pdev);

	return 0;
}

static void tcc_adc_hw_init(struct tcc_adc *info)
{
	/* 50 clocks conversion delay */
	writel(50, info->regs + ADCDLY);
	/* Set prescaler to 12 */
	writel(BIT(14) | (11 << 6), info->regs + ADCCON);
}

static int tcc_adc_probe(struct platform_device *pdev)
{
	struct tcc_adc *info = NULL;
	struct device_node *np = pdev->dev.of_node;
	struct iio_dev *indio_dev = NULL;
	struct resource	*mem;
	int ret = -ENODEV;
	int irq;

	if (!np)
		return ret;

	indio_dev = iio_device_alloc(sizeof(struct tcc_adc));
	if (!indio_dev) {
		dev_err(&pdev->dev, "failed allocating iio device\n");
		return -ENOMEM;
	}

	info = iio_priv(indio_dev);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	info->regs = devm_request_and_ioremap(&pdev->dev, mem);
	if (!info->regs) {
		ret = -ENOMEM;
		goto err_iio;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "no irq resource?\n");
		ret = irq;
		goto err_iio;
	}

	info->irq = irq;

	init_completion(&info->completion);

	ret = request_irq(info->irq, tcc_adc_isr,
					0, dev_name(&pdev->dev), info);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed requesting irq, irq = %d\n",
							info->irq);
		goto err_iio;
	}

	info->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(info->clk)) {
		dev_err(&pdev->dev, "failed getting clock, err = %ld\n",
							PTR_ERR(info->clk));
		ret = PTR_ERR(info->clk);
		goto err_irq;
	}

	platform_set_drvdata(pdev, indio_dev);

	indio_dev->name = dev_name(&pdev->dev);
	indio_dev->dev.parent = &pdev->dev;
	indio_dev->dev.of_node = pdev->dev.of_node;
	indio_dev->info = &tcc_adc_iio_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = tcc_adc_iio_channels;

	indio_dev->num_channels = MAX_ADC_CHANNELS;

	ret = iio_device_register(indio_dev);
	if (ret)
		goto err_irq;

	clk_prepare_enable(info->clk);

	tcc_adc_hw_init(info);

	ret = of_platform_populate(np, tcc_adc_match, NULL, &pdev->dev);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed adding child nodes\n");
		goto err_of_populate;
	}

	return 0;

err_of_populate:
	device_for_each_child(&pdev->dev, NULL,
				tcc_adc_remove_devices);
	clk_disable_unprepare(info->clk);
	iio_device_unregister(indio_dev);
err_irq:
	free_irq(info->irq, info);
err_iio:
	iio_device_free(indio_dev);
	return ret;
}

static int tcc_adc_remove(struct platform_device *pdev)
{
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);
	struct tcc_adc *info = iio_priv(indio_dev);

	device_for_each_child(&pdev->dev, NULL,
				tcc_adc_remove_devices);
	clk_disable_unprepare(info->clk);
	iio_device_unregister(indio_dev);
	free_irq(info->irq, info);
	iio_device_free(indio_dev);

	return 0;
}

static struct platform_driver tcc_adc_driver = {
	.probe		= tcc_adc_probe,
	.remove		= tcc_adc_remove,
	.driver		= {
		.name	= "tcc-adc",
		.owner	= THIS_MODULE,
		.of_match_table = tcc_adc_match,
	},
};

module_platform_driver(tcc_adc_driver);

MODULE_AUTHOR("Ithamar R. Adema <ithamar@upgrade-android.com>");
MODULE_DESCRIPTION("Telechips ADC driver");
MODULE_LICENSE("GPL v2");
