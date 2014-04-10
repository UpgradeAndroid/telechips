/*
 *  Driver for Freescale's 3-Axis Accelerometer MMA7660.
 *
 *  Copyright (C) 2011 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* XXX add check for smbus/i2c functionality/test smbus emulation */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input-polldev.h>
#include <linux/of_device.h>

#define MMA7660_DRV_NAME	"mma7660"

#define MMA7660_XOUT		0x00
#define MMA7660_YOUT		0x01
#define MMA7660_ZOUT		0x02
#define MMA7660_TILT		0x03
#define MMA7660_SRST		0x04
#define MMA7660_SPCNT		0x05
#define MMA7660_INTSU		0x06
#define MMA7660_MODE		0x07
#define MMA7660_SR		0x08
#define MMA7660_PDET		0x09
#define MMA7660_PD		0x0a

#define MODE_CHANGE_DELAY_MS	100
#define POLL_INTERVAL		500 //100
#define POLL_INTERVAL_MAX	500

struct mma7660 {
	struct i2c_client	*client;
	struct input_polled_dev	*idev;
};

static int mma7660_read(struct mma7660 *m, unsigned off)
{
	struct i2c_client *c = m->client;
	int ret;

	ret = i2c_smbus_read_byte_data(c, off);
	if (ret < 0)
		dev_err(&c->dev,
			"failed to read register 0x%02x, error %d\n",
			off, ret);

	return ret;
}

static int mma7660_write(struct mma7660 *m, unsigned off, u8 v)
{
	struct i2c_client *c = m->client;
	int error;

#if 0
	error = i2c_smbus_write_byte_data(c, off, v);
	if (error < 0) {
		dev_err(&c->dev,
			"failed to write to register 0x%02x, error %d\n",
			off, error);
		return error;
	}
#else
	u8 buf[2] = { (u8)off, v };
	i2c_master_send(c, buf, sizeof(buf));
#endif

	return 0;
}

static int mma7660_read_block(struct mma7660 *m, unsigned off,
			      u8 *buf, size_t size)
{
	struct i2c_client *c = m->client;
	int err;
#if 0
	err = i2c_smbus_read_i2c_block_data(c, off, size, buf);
#else
	u8 o = (u8)off;
	i2c_master_send(c, &o, sizeof(o));
	err = i2c_master_recv(c, buf, size);
#endif
	if (err < 0) {
		dev_err(&c->dev,
			"failed to read block data at 0x%02x, error %d\n",
			off, err);
		return err;
	}

	return 0;
}

static void mma7660_poll(struct input_polled_dev *dev)
{
	struct mma7660 *m = dev->private;
	int x, y, z;
	int ret;
	u8 buf[3];

	/* Loop until error or valid sample */
	do {
		ret = mma7660_read_block(m, MMA7660_XOUT, buf, sizeof(buf));
	} while(ret == 0 &&
		((buf[0] & 0x40) || (buf[1] & 0x40) || (buf[2] & 0x40)));

	if (ret < 0)
		return;

	x = buf[0] & 0x3f;
	if (x & BIT(5))
		x = -(x & ~BIT(5));
	y = buf[1] & 0x3f;
	if (y & BIT(5))
		y = -(y & ~BIT(5));
	z = buf[2] & 0x3f;
	if (z & BIT(5))
		z = -(z & ~BIT(5));

	input_report_abs(dev->input, ABS_X, x);
	input_report_abs(dev->input, ABS_Y, y);
	input_report_abs(dev->input, ABS_Z, z);
	input_sync(dev->input);
}

/* Initialize the MMA7660 chip */
static void mma7660_open(struct input_polled_dev *dev)
{
	struct mma7660 *m = dev->private;
	int err;

	/* set active mode */
	err = mma7660_write(m, MMA7660_MODE, 0x01);
	if (err)
		return;

	msleep(MODE_CHANGE_DELAY_MS);
}

static void mma7660_close(struct input_polled_dev *dev)
{
	struct mma7660 *m = dev->private;

	/* switch to standby mode */
	mma7660_write(m, MMA7660_MODE, 0x00);
}

/*
 * I2C init/probing/exit functions
 */
static int mma7660_probe(struct i2c_client *c,
				   const struct i2c_device_id *id)
{
	struct input_polled_dev *idev;
	struct mma7660 *m;
	int err;

	m = kzalloc(sizeof(struct mma7660), GFP_KERNEL);
	idev = input_allocate_polled_device();
	if (!m || !idev) {
		err = -ENOMEM;
		goto err_free_mem;
	}

	m->client = c;
	m->idev = idev;

	idev->private		= m;
	idev->input->name	= MMA7660_DRV_NAME;
	idev->input->id.bustype	= BUS_I2C;
	idev->poll		= mma7660_poll;
	idev->poll_interval	= POLL_INTERVAL;
	idev->poll_interval_max	= POLL_INTERVAL_MAX;
	idev->open		= mma7660_open;
	idev->close		= mma7660_close;

	__set_bit(EV_ABS, idev->input->evbit);
	input_set_abs_params(idev->input, ABS_X, -31, 31, 0, 0); //XX check fuzz/flat
	input_set_abs_params(idev->input, ABS_Y, -31, 31, 0, 0);
	input_set_abs_params(idev->input, ABS_Z, -31, 31, 0, 0);

	err = input_register_polled_device(idev);
	if (err) {
		dev_err(&c->dev, "failed to register polled input device\n");
		goto err_free_mem;
	}

	return 0;

err_free_mem:
	input_free_polled_device(idev);
	kfree(m);
	return err;
}

static int mma7660_remove(struct i2c_client *c)
{
	struct mma7660 *m = i2c_get_clientdata(c);
	struct input_polled_dev *idev = m->idev;

	input_unregister_polled_device(idev);
	input_free_polled_device(idev);
	kfree(m);

	return 0;
}

static const struct i2c_device_id mma7660_id[] = {
	{ MMA7660_DRV_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, mma7660_id);

static const struct of_device_id mma7660_dt_ids[] = {
	{ .compatible = "fsl,mma7660", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, mma7660_dt_ids);

static struct i2c_driver mma7660_driver = {
	.driver = {
		.name	= MMA7660_DRV_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = mma7660_dt_ids,
	},
	.probe		= mma7660_probe,
	.remove		= mma7660_remove,
	.id_table	= mma7660_id,
};

module_i2c_driver(mma7660_driver);

MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_DESCRIPTION("MMA7660 3-Axis Accelerometer Driver");
MODULE_LICENSE("GPL");
