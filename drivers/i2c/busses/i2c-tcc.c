/*
 * Copyright 2014 Upgrade Android
 *     Ithamar R. Adema <ihamar@upgrade-android.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_i2c.h>
#include <linux/clk.h>

#include <linux/io.h>

struct tcc_i2c {
	spinlock_t lock;
	struct completion completion;
	struct tcc_i2c_regs *regs;
	u32 i2c_clk_rate;
	struct clk *clk;
	int irq;

	struct i2c_msg *msg;
	u32 msg_num;
	u32 msg_idx;
	u32 msg_ptr;

	struct device *dev;
	struct i2c_adapter adap;
};

#define TCC_I2C_PRES	0x00
#define TCC_I2C_CTRL	0x04
#define TCC_I2C_TXR	0x08
#define TCC_I2C_CMD	0x0c
#define TCC_I2C_RXR	0x10
#define TCC_I2C_SR	0x14
#define TCC_I2C_TIME	0x18

struct tcc_i2c_regs {
	volatile unsigned long PRES, CTRL, TXR, CMD, RXR, SR, TIME;
};

static irqreturn_t tcc_i2c_irq(int irq, void *dev_id)
{
	struct tcc_i2c *i2c = dev_id;


	if (i2c && (i2c->regs->SR & BIT(0)) != 0) {
		/* ack interrupt */
		i2c->regs->CMD |= BIT(0);

		if (i2c->regs->SR & BIT(5))
			dev_dbg(i2c->dev, "Arbitration lost!\n");

		/* unblock wait_intr */
		complete(&i2c->completion);

		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

/* tcc_i2c_message_start
 *
 * put the start of a message onto the bus
*/
static void tcc_i2c_message_start(struct tcc_i2c *i2c, struct i2c_msg *msg)
{
	unsigned int addr = (msg->addr & 0x7f) << 1;

	if (msg->flags & I2C_M_RD)
		addr |= 1;
	if (msg->flags & I2C_M_REV_DIR_ADDR)
		addr ^= 1;

	i2c->regs->TXR = addr;
	i2c->regs->CMD = BIT(7) | BIT(4);
}

static int wait_intr(struct tcc_i2c *i2c)
{
	return wait_for_completion_killable(&i2c->completion);
}

static int recv_i2c(struct tcc_i2c *i2c)
{
	int ret, i;

	tcc_i2c_message_start(i2c, i2c->msg);

	ret = wait_intr(i2c);
	if (ret != 0)
		return ret;

	for (i = 0; i < i2c->msg->len; i++) {
		if (i == (i2c->msg->len - 1))
			i2c->regs->CMD = BIT(5) | BIT(3);
		else
			i2c->regs->CMD = BIT(5);

		ret = wait_intr(i2c);
		if (ret != 0)
			return ret;

		i2c->msg->buf[i] = (unsigned char)i2c->regs->RXR;
	}

	i2c->regs->CMD = BIT(6);

	ret = wait_intr(i2c);
	if (ret != 0)
		return ret;

	return 1;
}

static int send_i2c(struct tcc_i2c *i2c)
{
	int ret, i, no_stop = 0;

	tcc_i2c_message_start(i2c, i2c->msg);

	ret = wait_intr(i2c);
	if (ret != 0)
		return ret;

	for (i = 0; i < i2c->msg->len; i++) {
		i2c->regs->TXR = i2c->msg->buf[i];

		i2c->regs->CMD = BIT(4);

		ret = wait_intr(i2c);
		if (ret != 0)
			return ret;
	}

	/*
	 *      Check no-stop operation condition (write -> read operation)
	 *      2. msg_num == 2
	 */
	if (i2c->msg_num > 1)
		no_stop = 1;

	if (no_stop == 0) {
		i2c->regs->CMD = BIT(6);
		ret = wait_intr(i2c);
		if (ret != 0)
			return ret;
	}

	return 1;
}

/* tcc_i2c_doxfer
 *
 * this starts an i2c transfer
*/
static int tcc_i2c_doxfer(struct tcc_i2c *i2c, struct i2c_msg *msgs, int num)
{
	int ret = -EINVAL, i;

	for (i = 0; i < num; i++) {
		spin_lock_irq(&i2c->lock);
		i2c->msg = &msgs[i];
		i2c->msg->flags = msgs[i].flags;
		i2c->msg_num = num;
		i2c->msg_ptr = 0;
		i2c->msg_idx = 0;
		spin_unlock_irq(&i2c->lock);

		if (i2c->msg->flags & I2C_M_RD) {
			ret = recv_i2c(i2c);
			if (ret != 1)
				printk("recv_i2c failed\n");
		} else {
			ret = send_i2c(i2c);
			if (ret != 1)
				printk("send_i2c failed\n");
		}
	}

	return ret;
}

/* tcc_i2c_xfer
 *
 * first port of call from the i2c bus code when an message needs
 * transferring across the i2c bus.
*/
static int tcc_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	struct tcc_i2c *i2c = (struct tcc_i2c *)adap->algo_data;
	int retry;
	int ret;

	for (retry = 0; retry < adap->retries; retry++) {
		ret = tcc_i2c_doxfer(i2c, msgs, num);
		if (ret == 1)
			return num;

		dev_dbg(&i2c->adap.dev, "Retrying transmission (%d)\n", retry);
		udelay(100);
	}

	return -EREMOTEIO;
}

static u32 tcc_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm tcc_i2c_algorithm = {
	.master_xfer = tcc_i2c_xfer,
	.functionality = tcc_i2c_func,
};

static int tcc_i2c_init(struct tcc_i2c *i2c)
{
	unsigned int prescale;
	volatile struct tcc_i2c_regs *i2c_reg = i2c->regs;
	int err;

	err = clk_prepare_enable(i2c->clk);
	if (err != 0) {
		dev_err(i2c->dev, "can't do i2c clock enable\n");
		return err;
	}

	prescale = (clk_get_rate(i2c->clk) / i2c->i2c_clk_rate) / 5 - 1;
	writel_relaxed(prescale, &i2c_reg->PRES);

	dev_info(i2c->dev, "core rate: %lu, i2c rate: %u, prescale: %d\n",
		clk_get_rate(i2c->clk),
		i2c->i2c_clk_rate,
		prescale);

	/* start enable, stop enable, 8bit mode */
	writel_relaxed(BIT(7) | BIT(6), &i2c_reg->CTRL);	
	/* clear pending interrupt */
	writel_relaxed(BIT(0) | readl_relaxed(&i2c_reg->CMD), &i2c_reg->CMD);

	return devm_request_irq(i2c->dev, i2c->irq, tcc_i2c_irq, IRQF_SHARED, dev_name(i2c->dev), i2c);
}

static const struct of_device_id tcc_i2c_match[] = {
	{ .compatible = "tcc,i2c-master", NULL, },
	{},
};

MODULE_DEVICE_TABLE(of, tcc_i2c_match);

static int tcc_i2c_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct pinctrl *pinctrl;
	struct resource *res;
	struct tcc_i2c *i2c;
	int ret = 0;

	pinctrl = devm_pinctrl_get_select_default(&pdev->dev);
	if (IS_ERR(pinctrl) && PTR_ERR(pinctrl) != -ENODEV) {
		dev_warn(&pdev->dev, "unable to get pinmux: %ld", PTR_ERR(pinctrl));
		return PTR_ERR(pinctrl);
	}

	i2c = devm_kzalloc(&pdev->dev, sizeof(*i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	i2c->dev = &pdev->dev;
	i2c->adap.owner = THIS_MODULE;
	i2c->adap.algo = &tcc_i2c_algorithm;
	i2c->adap.retries = 2;
	sprintf(i2c->adap.name, "%s", pdev->name);

	spin_lock_init(&i2c->lock);
	init_completion(&i2c->completion);

	/* Get core I2C IP registers */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	i2c->regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(i2c->regs))
		return PTR_ERR(i2c->regs);

	i2c->clk = devm_clk_get(&pdev->dev, 0);
	if (IS_ERR(i2c->clk))
		return PTR_ERR(i2c->clk);

	ret = of_property_read_u32(np, "clock-frequency", &i2c->i2c_clk_rate);
	if (ret) {
		dev_err(i2c->dev, "clock-frequency property missing!\n");
		return ret;
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res)
		i2c->irq = res->start;

	ret = tcc_i2c_init(i2c);
	if (ret)
		return ret;

	i2c->adap.algo_data = i2c;
	i2c->adap.dev.parent = &pdev->dev;
	i2c->adap.dev.of_node = pdev->dev.of_node;
	i2c->adap.class = I2C_CLASS_HWMON | I2C_CLASS_SPD;

	ret = i2c_add_adapter(&i2c->adap);
	if (ret < 0) {
		i2c_del_adapter(&i2c->adap);
		return ret;
	}

	platform_set_drvdata(pdev, i2c);
	of_i2c_register_devices(&i2c->adap);

	return 0;
}

static int tcc_i2c_remove(struct platform_device *pdev)
{
	struct tcc_i2c *i2c = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);

	i2c_del_adapter(&i2c->adap);
	/* I2C bus & clock disable */
	clk_disable_unprepare(i2c->clk);
	clk_put(i2c->clk);

	kfree(i2c);

	return 0;
}

static struct platform_driver tcc_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "tcc-i2c",
		.of_match_table = tcc_i2c_match,
	},
	.probe = tcc_i2c_probe,
	.remove = tcc_i2c_remove,
};

module_platform_driver(tcc_i2c_driver);

MODULE_AUTHOR("Ithamar R. Adema <ithamar@upgrade-android.com>");
MODULE_DESCRIPTION("Telechips SoC I2C driver");
MODULE_LICENSE("GPL");
