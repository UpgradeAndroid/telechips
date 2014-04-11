/*
 * Driver for UUTEK UOR6X5X 2-touch touchscreens
 *
 * Copyright (c) 2014 Upgrade Android
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/input/uor-ts.h>
#include <linux/of.h>
#include <linux/of_gpio.h>

#define UOR_REPORT_POINTS	2
#define UOR_PENUP_TIMEOUT_MS	10

/* UOR Command values */
#define INITX		0x00
#define INITY		0x10
#define MSRX_2T         0x40
#define MSRY_2T         0x50
#define MSRX_1T         0xC0
#define MSRY_1T         0xD0
#define MSRZ_1T        0xE0
#define MSRZ_2T        0xF0

struct uor_ts {
	struct i2c_client *client;
	struct input_dev *input;
	const struct uor_ts_platdata *pdata;
	char phys[32];
	wait_queue_head_t wait;
	bool stopped;
};

struct uor_point_t {
	uint16_t coord_x;
	uint16_t coord_y;
	uint16_t pressure;
};

static int uor_read(struct i2c_client *client, uint8_t cmd,
			uint8_t *buf, size_t buflen)
{
	struct i2c_msg msgs[] = {
		{
			.addr 	= client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= &cmd,
		}, {
			.addr	= client->addr,
			.flags	= I2C_M_RD,
			.len	= buflen,
			.buf	= buf,
		}
	};

	if (i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs)) != ARRAY_SIZE(msgs)) {
		dev_err(&client->dev, "i2c communication error\n");
		return -EIO;
	}

	return 0;
}

static int uor_collect_data(struct uor_ts *ts,
				   struct uor_point_t *point)
{
	struct i2c_client *client = ts->client;
	const struct uor_ts_platdata *pdata = ts->pdata;
	uint8_t raw_x1[2], raw_y1[2], raw_z1[2];
	uint8_t raw_x2[3], raw_y2[3], raw_z2[2];
	int ret;

	/* Read coordinates for touches */
	ret = uor_read(client, MSRX_1T, raw_x1, sizeof(raw_x1));
	if (ret < 0)
		return ret;
	ret = uor_read(client, MSRY_1T, raw_y1, sizeof(raw_y1));
	if (ret < 0)
		return ret;
	ret = uor_read(client, MSRZ_1T, raw_z1, sizeof(raw_z1));
	if (ret < 0)
		return ret;
	ret = uor_read(client, MSRX_2T, raw_x2, sizeof(raw_x2));
	if (ret < 0)
		return ret;
	ret = uor_read(client, MSRY_2T, raw_y2, sizeof(raw_y2));
	if (ret < 0)
		return ret;
	ret = uor_read(client, MSRZ_2T, raw_z2, sizeof(raw_z2));
	if (ret < 0)
		return ret;

	point[0].coord_x = raw_x1[0] << 4 | raw_x1[1] >> 4;
	point[0].coord_y = raw_y1[0] << 4 | raw_y1[1] >> 4;
	point[0].pressure = raw_z1[0] << 4 | raw_z1[1] >> 4;

	// Validate!

	point[1].coord_x = raw_x2[2];
	point[1].coord_y = raw_y2[2];
	point[1].pressure = raw_z2[0] << 4 | raw_z2[1] >> 4;

	// Validate!

	pr_info("(%u,%u,%u) (%u,%u,%u)\n",
		point[0].coord_x, point[0].coord_y, point[0].pressure,
		point[1].coord_x, point[1].coord_y, point[1].pressure);

	return 0;
}

static irqreturn_t uor_interrupt(int irq, void *dev_id)
{
	struct uor_ts *ts = dev_id;
	const struct uor_ts_platdata *pdata = ts->pdata;
	struct uor_point_t point[UOR_REPORT_POINTS];
	int i, ret, fingers = 0;
	int abs = -1;

	while (!ts->stopped) {
		if (gpio_get_value(pdata->gpio_int) == 1) {
			input_mt_sync(ts->input);
			input_report_key(ts->input, BTN_TOUCH, 0);
			input_sync(ts->input);
			break;
		}

		ret = uor_collect_data(ts, point);
		if (ret < 0) {
			wait_event_timeout(ts->wait, ts->stopped,
				msecs_to_jiffies(UOR_PENUP_TIMEOUT_MS));
			continue;
		}

		for (i = 0; i < UOR_REPORT_POINTS; i++) {
			if (point[i].coord_x > 0 || point[i].coord_y > 0) {
				input_report_abs(ts->input, ABS_MT_POSITION_X,
						 point[i].coord_x);
				input_report_abs(ts->input, ABS_MT_POSITION_Y,
						 point[i].coord_y);
				input_mt_sync(ts->input);

				/* use first finger as source for singletouch */
				if (fingers == 0)
					abs = i;

				/* number of touch points could also be queried
				 * via i2c but would require an additional call
				 */
				fingers++;
			}
		}

		input_report_key(ts->input, BTN_TOUCH, fingers > 0);

		if (abs > -1) {
			input_report_abs(ts->input, ABS_X, point[abs].coord_x);
			input_report_abs(ts->input, ABS_Y, point[abs].coord_y);
		}

		input_sync(ts->input);

		wait_event_timeout(ts->wait, ts->stopped,
				 msecs_to_jiffies(UOR_PENUP_TIMEOUT_MS));
	}

	return IRQ_HANDLED;
}

static int uor_start(struct uor_ts *ts)
{
	ts->stopped = false;
	mb();
	enable_irq(ts->client->irq);

	return 0;
}

static int uor_stop(struct uor_ts *ts)
{
	/* disable receiving of interrupts */
	disable_irq(ts->client->irq);
	ts->stopped = true;
	mb();
	wake_up(&ts->wait);
	return 0;
}

static int uor_input_open(struct input_dev *dev)
{
	struct uor_ts *ts = input_get_drvdata(dev);
	int ret;

	ret = uor_start(ts);
	if (ret)
		return ret;

	return 0;
}

static void uor_input_close(struct input_dev *dev)
{
	struct uor_ts *ts = input_get_drvdata(dev);

	uor_stop(ts);

	return;
}

#ifdef CONFIG_PM_SLEEP
static int uor_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct uor_ts *ts = i2c_get_clientdata(client);
	struct input_dev *input = ts->input;
	int ret = 0;

	mutex_lock(&input->mutex);

	/* when configured as wakeup source, device should always wake system
	 * therefore start device if necessary
	 */
	if (device_may_wakeup(&client->dev)) {
		/* need to start device if not open, to be wakeup source */
		if (!input->users) {
			ret = uor_start(ts);
			if (ret)
				goto unlock;
		}

		enable_irq_wake(client->irq);
	} else if (input->users) {
		ret = uor_stop(ts);
	}

unlock:
	mutex_unlock(&input->mutex);

	return ret;
}

static int uor_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct uor_ts *ts = i2c_get_clientdata(client);
	struct input_dev *input = ts->input;
	int ret = 0;

	mutex_lock(&input->mutex);

	if (device_may_wakeup(&client->dev)) {
		disable_irq_wake(client->irq);

		/* need to stop device if it was not open on suspend */
		if (!input->users) {
			ret = uor_stop(ts);
			if (ret)
				goto unlock;
		}

		/* device wakes automatically from SLEEP */
	} else if (input->users) {
		ret = uor_start(ts);
	}

unlock:
	mutex_unlock(&input->mutex);

	return ret;
}
#endif

static SIMPLE_DEV_PM_OPS(uor_pm_ops,
			 uor_suspend, uor_resume);

#ifdef CONFIG_OF
static struct uor_ts_platdata *uor_parse_dt(struct device *dev)
{
	struct uor_ts_platdata *pdata;
	struct device_node *np = dev->of_node;

	if (!np)
		return ERR_PTR(-ENOENT);

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		dev_err(dev, "failed to allocate platform data\n");
		return ERR_PTR(-ENOMEM);
	}

	pdata->gpio_int = of_get_gpio(np, 0);
	if (!gpio_is_valid(pdata->gpio_int)) {
		dev_err(dev, "failed to get interrupt gpio\n");
		return ERR_PTR(-EINVAL);
	}

	if (of_property_read_u32(np, "x-size", &pdata->x_max))
		pdata->x_max = 4095;

	if (of_property_read_u32(np, "y-size", &pdata->y_max))
		pdata->y_max = 4095;

	return pdata;
}
#else
static struct uor_ts_platdata *uor_parse_dt(struct device *dev)
{
	return ERR_PTR(-EINVAL);
}
#endif

static int uor_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	const struct uor_ts_platdata *pdata;
	struct uor_ts *ts;
	struct input_dev *input_dev;
	int error;

	pdata = dev_get_platdata(&client->dev);
	if (!pdata) {
		pdata = uor_parse_dt(&client->dev);
		if (IS_ERR(pdata))
			return PTR_ERR(pdata);
	}

	ts = devm_kzalloc(&client->dev,
			  sizeof(struct uor_ts), GFP_KERNEL);
	if (!ts)
		return -ENOMEM;

	input_dev = devm_input_allocate_device(&client->dev);
	if (!input_dev) {
		dev_err(&client->dev, "could not allocate input device\n");
		return -ENOMEM;
	}

	ts->pdata = pdata;
	ts->client = client;
	ts->input = input_dev;
	ts->stopped = true;
	init_waitqueue_head(&ts->wait);

	snprintf(ts->phys, sizeof(ts->phys),
		 "%s/input0", dev_name(&client->dev));

	input_dev->name = "UOR6X5X touchscreen";
	input_dev->phys = ts->phys;
	input_dev->id.bustype = BUS_I2C;

	input_dev->open = uor_input_open;
	input_dev->close = uor_input_close;

	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);

	__set_bit(BTN_TOUCH, input_dev->keybit);

	/* For single touch */
	input_set_abs_params(input_dev, ABS_X, 0, pdata->x_max, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, pdata->y_max, 0, 0);

	/* For multi touch */
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0,
			     pdata->x_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0,
			     pdata->y_max, 0, 0);

	input_set_drvdata(ts->input, ts);

	error = devm_gpio_request_one(&client->dev, pdata->gpio_int,
				      GPIOF_DIR_IN, "uor_ts_int");
	if (error) {
		dev_err(&client->dev, "request of gpio %d failed, %d\n",
			pdata->gpio_int, error);
		return error;
	}

	error = devm_request_threaded_irq(&client->dev, client->irq,
					  NULL, uor_interrupt,
					  IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					  input_dev->name, ts);
	if (error) {
		dev_err(&client->dev, "irq %d requested failed, %d\n",
			client->irq, error);
		return error;
	}

	/* stop device and put it into deep sleep until it is opened */
	error = uor_stop(ts);
	if (error)
		return error;

	error = input_register_device(input_dev);
	if (error) {
		dev_err(&client->dev, "could not register input device, %d\n",
			error);
		return error;
	}

	i2c_set_clientdata(client, ts);

	return 0;
}

static const struct i2c_device_id uor_idtable[] = {
	{ "uor6x5x", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, uor_idtable);

#ifdef CONFIG_OF
static struct of_device_id uor_ts_dt_idtable[] = {
	{ .compatible = "uutek,uor6x5x" },
	{},
};
MODULE_DEVICE_TABLE(of, uor_ts_dt_idtable);
#endif

static struct i2c_driver uor_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "uor_ts",
		.pm	= &uor_pm_ops,
		.of_match_table	= of_match_ptr(uor_ts_dt_idtable),
	},
	.probe		= uor_probe,
	.id_table	= uor_idtable,
};

module_i2c_driver(uor_driver);

MODULE_AUTHOR("Ithamar R. Adema <ithamar@upgrade-android.com>");
MODULE_DESCRIPTION("UOR6x5x touchscreen driver");
MODULE_LICENSE("GPL v2");
