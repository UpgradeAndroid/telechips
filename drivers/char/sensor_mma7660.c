/* 
 * linux/drivers/char/sensor_mma7660.c
 *
 * Author:  <linux@telechips.com>
 * Created: 10th Jun, 2008 
 * Description: Telechips Linux BACK-LIGHT DRIVER
 *
 * Copyright (c) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#include <linux/io.h>
#include <asm/mach-types.h>
#include <linux/delay.h>

#include <mach/bsp.h>

#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/input.h>

#include <linux/slab.h>

#include <mach/sensor_ioctl.h>

#define MMA7660_DEBUG    0

#if MMA7660_DEBUG
#define sensor_dbg(fmt, arg...)     printk(fmt, ##arg)
#else
#define sensor_dbg(arg...)
#endif

#define SENSOR_DEV_NAME			"sensor"
#define SENSOR_DEV_MAJOR		249
#define SENSOR_DEV_MINOR		1

#define M_RESOLUTIOIN    (32)

typedef struct {
	int x;
	int y;
	int z;
	int resolution;
	int delay_time;
} tcc_sensor_accel_t;

#define MMA7660_I2C_ADDRESS      0x4c

static struct i2c_driver sensor_i2c_driver;
static struct i2c_client *sensor_i2c_client;

static struct timer_list *sensor_timer;
static int sensor_used_count = 0;
static unsigned int sensor_state_flag = 0;
static unsigned int sensor_duration = 60;	//200;
static tcc_sensor_accel_t tcc_sensor_accel;
static tcc_sensor_accel_t calib_data;
static struct work_struct sensor_work_q;

static const struct i2c_device_id sensor_i2c_id[] = {
	{"tcc-accel-sensor", 0,},
	{}
};

struct sensor_i2c_chip_info {
	struct i2c_client *client;
};

static void sensor_timer_handler(unsigned long data)
{
	sensor_dbg("%s\n", __func__);

	if (schedule_work(&sensor_work_q) == 0)
		sensor_dbg("cannot schedule work !!!\n");
}

static void sensor_timer_registertimer(struct timer_list *ptimer,
				       unsigned int timeover)
{
	sensor_dbg("%s\n", __func__);
	init_timer(ptimer);
	ptimer->expires = jiffies + msecs_to_jiffies(timeover);
	ptimer->data = (unsigned long)NULL;
	ptimer->function = sensor_timer_handler;

	add_timer(ptimer);
}

static void SENSOR_SEND_CMD(unsigned char reg, unsigned char val)
{
	unsigned char cmd[2];
	cmd[0] = reg;
	cmd[1] = val;
	i2c_master_send(sensor_i2c_client, cmd, 2);
}

static void SENSOR_READ_DAT_S(unsigned char reg, unsigned char *buf, int size)
{
	i2c_master_send(sensor_i2c_client, (const char *)&reg, 1);
	i2c_master_recv(sensor_i2c_client, buf, size);
}

static int sensor_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	struct sensor_i2c_chip_info *chip;

	sensor_dbg("\n sensor_i2c_probe  :  %s \n", client->name);

	chip = kzalloc(sizeof(struct sensor_i2c_chip_info), GFP_KERNEL);
	if (chip == NULL) {
		sensor_dbg("\n tcc_sensor_i2c  :  no chip info. \n");
		return -ENOMEM;
	}

	chip->client = client;
	i2c_set_clientdata(client, chip);
	sensor_i2c_client = client;

	return 0;
}

static int sensor_i2c_remove(struct i2c_client *client)
{
	struct sensor_i2c_chip_info *chip = i2c_get_clientdata(client);

	kfree(chip);
	sensor_i2c_client = NULL;

	return 0;
}

/* mma7660 i2c control layer */
static struct i2c_driver sensor_i2c_driver = {
	.driver = {
		   .name = "tcc-accel-sensor",
		   .owner = THIS_MODULE,
		   },
	.probe = sensor_i2c_probe,
	.remove = sensor_i2c_remove,
	.id_table = sensor_i2c_id,
};

#define ACC_DATA_STORE_CNT	16
#define ACC_DATA_REAL_USED	8
volatile static tcc_sensor_accel_t acc_data_store[ACC_DATA_STORE_CNT];
volatile static tcc_sensor_accel_t acc_data_avg;
static int store_used_count = 0;
static int store_used_limit = ACC_DATA_REAL_USED;
static int store_save_pos = 0;

typedef struct _matrix3by3 {
	short _11;
	short _12;
	short _13;
	short _21;
	short _22;
	short _23;
	short _31;
	short _32;
	short _33;
} matrix3by3;

matrix3by3 gsenlayout[1] = {
#if defined(CONFIG_GSEN_TOP)
#if defined(CONFIG_GSEN_ROTATE_0)
	{1, 0, 0, 0, 1, 0, 0, 0, 1},
#elif defined(CONFIG_GSEN_ROTATE_90)
	{0, 1, 0, -1, 0, 0, 0, 0, 1},
#elif defined(CONFIG_GSEN_ROTATE_180)
	{-1, 0, 0, 0, -1, 0, 0, 0, 1},
#else // CONFIG_GSEN_ROTATE_270
	{0, -1, 0, 1, 0, 0, 0, 0, 1},
#endif
#else // CONFIG_GSEN_BOTTOM
#if defined(CONFIG_GSEN_ROTATE_0)
	{-1, 0, 0, 0, 1, 0, 0, 0, -1},
#elif defined(CONFIG_GSEN_ROTATE_90)
	{0, -1, 0, -1, 0, 0, 0, 0, -1},
#elif defined(CONFIG_GSEN_ROTATE_180)
	{1, 0, 0, 0, -1, 0, 0, 0, -1},
#else // CONFIG_GSEN_ROTATE_270
	{0, 1, 0, 1, 0, 0, 0, 0, -1},
#endif
#endif
};

static int tcc_sensor_avg_init(void)
{
	int i;
	for (i = 0; i < ACC_DATA_STORE_CNT; i++) {
		acc_data_store[i].x = 0;
		acc_data_store[i].y = 0;
		acc_data_store[i].z = 0;
	}
	acc_data_avg.x = 0;
	acc_data_avg.y = 0;
	acc_data_avg.z = 0;
	store_used_count = 0;
	store_save_pos = 0;

	return 0;
}

static void tcc_sensor_avg_count(int duration)
{
	if (duration >= 200)
		store_used_limit = 2;
	else if (duration >= 100)
		store_used_limit = 4;
	else if (duration >= 40)
		store_used_limit = 8;
	else
		store_used_limit = 16;
}

static void tcc_sensor_avg(int *data)
{
	int i;
	tcc_sensor_accel_t acc_data_sum;
	if (store_save_pos >= store_used_limit)
		store_save_pos = 0;

	acc_data_store[store_save_pos].x = ((signed char)data[0]);
	acc_data_store[store_save_pos].y = ((signed char)data[1]);
	acc_data_store[store_save_pos].z = ((signed char)data[2]);
	acc_data_sum.x = 0;
	acc_data_sum.y = 0;
	acc_data_sum.z = 0;
	store_save_pos++;

	store_used_count++;
	if (store_used_count >= store_used_limit)
		store_used_count = store_used_limit;
	for (i = 0; i < store_used_count; i++) {
		acc_data_sum.x += acc_data_store[i].x;
		acc_data_sum.y += acc_data_store[i].y;
		acc_data_sum.z += acc_data_store[i].z;
	}
	acc_data_avg.x = acc_data_sum.x / store_used_count;
	acc_data_avg.y = acc_data_sum.y / store_used_count;
	acc_data_avg.z = acc_data_sum.z / store_used_count;
printk("%s: %d, %d, %d  - %d %d %d , delay = %d\n", __func__, acc_data_sum.x, acc_data_sum.y, acc_data_sum.z, acc_data_avg.x, acc_data_avg.y, acc_data_avg.z, tcc_sensor_accel.delay_time);
}

static void tcc_sensor_landscapeDevice2AndroidPortrait(tcc_sensor_accel_t *
						       sensor_accel)
{
	int x, y, z;
	x = sensor_accel->x;
	y = sensor_accel->y;
	z = sensor_accel->z;
	sensor_accel->x = y;
	sensor_accel->y = -x;
	sensor_accel->z = z;
}

static void tcc_sensor_convertCoordination(tcc_sensor_accel_t * sensor_accel,
					   matrix3by3 * layout)
{
	int x, y, z;
	if (sensor_accel == NULL)
		return;
	x = sensor_accel->x;
	y = sensor_accel->y;
	z = sensor_accel->z;

	sensor_accel->x = x * layout->_11 + y * layout->_12 + z * layout->_13;
	sensor_accel->y = x * layout->_21 + y * layout->_22 + z * layout->_23;
	sensor_accel->z = x * layout->_31 + y * layout->_32 + z * layout->_33;
}

static void tcc_sensor_set_compensation_data(tcc_sensor_accel_t
					     compenPercentData)
{
	calib_data.x = (M_RESOLUTIOIN * compenPercentData.x) / 100;
	calib_data.y = (M_RESOLUTIOIN * compenPercentData.y) / 100;
	calib_data.z = (M_RESOLUTIOIN * compenPercentData.z) / 100;
}

static void tcc_sensor_compensation(void)
{
	tcc_sensor_accel.x += (int)(calib_data.x);
	tcc_sensor_accel.y += (int)(calib_data.y);
	tcc_sensor_accel.z += (int)(calib_data.z);
}

static void tcc_sensor_set_data(tcc_sensor_accel_t accelData)
{
	int x, y, z;
	x = accelData.x;
	y = accelData.y;
	z = accelData.z;

	tcc_sensor_accel.resolution = M_RESOLUTIOIN;
#if 1
	tcc_sensor_accel.x = x;
	tcc_sensor_accel.y = y;
	tcc_sensor_accel.z = z;
	tcc_sensor_convertCoordination(&tcc_sensor_accel, &gsenlayout[0]);
#else
	tcc_sensor_accel.x =
	    x * gsenlayout[0]._11 + y * gsenlayout[0]._12 +
	    z * gsenlayout[0]._13;
	tcc_sensor_accel.y =
	    x * gsenlayout[0]._21 + y * gsenlayout[0]._22 +
	    z * gsenlayout[0]._23;
	tcc_sensor_accel.z =
	    x * gsenlayout[0]._31 + y * gsenlayout[0]._32 +
	    z * gsenlayout[0]._33;
#endif

#if !defined(CONFIG_GSEN_PORTRAIT)
	tcc_sensor_landscapeDevice2AndroidPortrait(&tcc_sensor_accel);
#endif
}

static int tcc_sensor_get_accel(void)
{
	int x, y, z;
	char buf[3];
	int data[3];

	SENSOR_READ_DAT_S(0x00, buf, 3);
	x = buf[0] & 0x3f;
	y = buf[1] & 0x3f;
	z = buf[2] & 0x3f;

	if (x >= 32)
		x = -(64 - x);

	if (y >= 32)
		y = -(64 - y);

	if (z >= 32)
		z = 64 - z;
	else
		z = -z;

	if (-5 <= y && y <= 5)
		data[0] = -y * 2 / 5;
	else
		data[0] = -y;
	if (-5 <= x && x <= 5)
		data[1] = -x * 2 / 5;
	else
		data[1] = -x;
	if (-5 <= z && z <= 5)
		data[2] = -z * 2 / 5;
	else
		data[2] = -z;

	tcc_sensor_avg(&data[0]);

	// G-sensor coordination -> Device coordination
	tcc_sensor_set_data(acc_data_avg);

	tcc_sensor_compensation();
	printk("%s: else accel : %d, %d, %d - [%d:%d:%d]\n", __func__, tcc_sensor_accel.x, tcc_sensor_accel.y, tcc_sensor_accel.z,calib_data.x,calib_data.y,calib_data.z);
	return 0;
}

static void sensor_fetch_thread(struct work_struct *work)
{
	sensor_dbg("%s: sensor_duration =%d \n", __func__, sensor_duration);
	tcc_sensor_get_accel();
	sensor_timer_registertimer(sensor_timer, sensor_duration);
}

static ssize_t tcc_sensor_write(struct file *file, const char __user * user,
				size_t size, loff_t * o)
{
	sensor_dbg("%s\n", __func__);
	return 0;
}

static ssize_t tcc_sensor_read(struct file *file, char __user * user,
			       size_t size, loff_t * o)
{
	sensor_dbg("%s\n", __func__);
//    sensor_dbg("%s: IOCTL_read_SENSORS_ACCELERATION  %d, %d, %d\n", __func__, tcc_sensor_accel.x, tcc_sensor_accel.y, tcc_sensor_accel.z);

//    tcc_sensor_get_accel();   //test  reverse
	if (copy_to_user
	    ((tcc_sensor_accel_t *) user, (const void *)&tcc_sensor_accel,
	     sizeof(tcc_sensor_accel_t)) != 0) {
		sensor_dbg("tcc_gsensor_read error\n");
	}
	pr_debug("mma7660: x=%d,y=%d,z=%d resolution=%d, delay_time=%d\n",
		tcc_sensor_accel.x, tcc_sensor_accel.y, tcc_sensor_accel.z,
		tcc_sensor_accel.resolution, tcc_sensor_accel.delay_time);

	return 0;
}

static long tcc_sensor_ioctl(struct file *filp, unsigned int cmd,
			     unsigned long arg)
{
//    if (!sensor_used_count)
//        return -1;

	sensor_dbg("%s  (0x%x)  \n", __func__, cmd);

	switch (cmd) {

	case IOCTL_SENSOR_GET_DATA_ACCEL:

		if (copy_to_user
		    ((tcc_sensor_accel_t *) arg,
		     (const void *)&tcc_sensor_accel,
		     sizeof(tcc_sensor_accel_t)) != 0) {
			sensor_dbg("copy_to error\n");
		}
		sensor_dbg("%s: IOCTL_SENSOR_GET_DATA_ACCEL %d, %d, %d\n",
			   __func__, tcc_sensor_accel.x, tcc_sensor_accel.y,
			   tcc_sensor_accel.z);
		break;

	case IOCTL_SENSOR_SET_DELAY_ACCEL:
		if (copy_from_user
		    ((void *)&sensor_duration, (unsigned int *)arg,
		     sizeof(unsigned int)) != 0) {
			sensor_dbg("copy_from error\n");
		}
		sensor_dbg(KERN_INFO
			   "%s:  IOCTL_SENSOR_SET_DELAY_ACCEL (0x%x) %d \n",
			   __func__, cmd, sensor_duration);
		tcc_sensor_accel.delay_time = sensor_duration;
		tcc_sensor_avg_count(sensor_duration);
		break;

	case IOCTL_SENSOR_GET_STATE_ACCEL:
		sensor_state_flag = 1;
		if (copy_to_user
		    ((unsigned int *)arg, (const void *)&sensor_state_flag,
		     sizeof(unsigned int)) != 0) {
			sensor_dbg("copy_to error\n");
		}

		sensor_dbg(KERN_INFO
			   "%s: IOCTL_SENSOR_GET_STATE_ACCEL  (0x%x) %d \n",
			   __func__, cmd, sensor_state_flag);
		break;

	case IOCTL_SENSOR_SET_STATE_ACCEL:
		//    arg = sensor_state_flag;

		if (copy_from_user
		    ((void *)&sensor_state_flag, (unsigned int *)arg,
		     sizeof(unsigned int)) != 0) {
			sensor_dbg("copy_from error\n");
		}
		sensor_dbg(KERN_INFO
			   "%s: IOCTL_SENSOR_SET_STATE_ACCEL  (0x%x) %d \n",
			   __func__, cmd, sensor_state_flag);
		break;

	case IOCTL_SENSOR_SET_CALIB_ACCEL:
		{
			tcc_sensor_accel_t stTmp;
			stTmp.x = 0;
			stTmp.y = 0;
			stTmp.z = 0;
			if (copy_from_user
			    ((void *)&stTmp, (unsigned int *)arg,
			     sizeof(tcc_sensor_accel_t)) != 0) {
				sensor_dbg("copy_from error\n");
				return -1;
			}
			tcc_sensor_set_compensation_data(stTmp);
			//printk(KERN_INFO "%s:  IOCTL_SENSOR_SET_CALIB_ACCEL %d, %d, %d \n", __func__, calib_data.x, calib_data.y, calib_data.z);
		}
		break;
	default:
		sensor_dbg("sensor: unrecognized ioctl (0x%x)\n", cmd);
		return -EINVAL;
		break;
	}
	return 0;
}

static int tcc_sensor_release(struct inode *inode, struct file *filp)
{
	sensor_dbg("%s (%d)\n", __func__, sensor_used_count);
	sensor_used_count--;
	if (sensor_used_count < 0) {
		sensor_dbg("sensor: release error (over)\n");
		sensor_used_count = 0;
	}

	if (sensor_used_count == 0) {
		flush_scheduled_work();
		del_timer_sync(sensor_timer);
		//del_timer( sensor_timer);
		kfree(sensor_timer);

	}
	return 0;
}

static int tcc_sensor_open(struct inode *inode, struct file *filp)
{
	sensor_dbg("%s : \n", __func__);
	if (sensor_used_count == 0) {
		//Initate MMA7760FC 's Register 
		SENSOR_SEND_CMD(0x07, 0);	//set mode to standby Mode
		SENSOR_SEND_CMD(0x05, 2);	//set sleep count register
		SENSOR_SEND_CMD(0x06, 0x0);	//set 
		SENSOR_SEND_CMD(0x08, 0xe0);	//set auto-wake and active mode portrait
		SENSOR_SEND_CMD(0x09, 0x0f);	//tap/pulse detection register is set
		SENSOR_SEND_CMD(0x0A, 0x02);	//set pd register
		SENSOR_SEND_CMD(0x07, 0x39);	//set mode     

		sensor_timer = kmalloc(sizeof(struct timer_list), GFP_KERNEL);	// test
		if (sensor_timer == NULL) {
			sensor_dbg("%s: mem alloc fail\n", __func__);
			return -ENOMEM;
		}
		memset(sensor_timer, 0, sizeof(struct timer_list));	// test
		sensor_timer_registertimer(sensor_timer, sensor_duration);	// test
		tcc_sensor_avg_init();
	}
	sensor_used_count++;
	sensor_dbg("%s successed... \n", __func__);

	return 0;
}

struct file_operations tcc_sensor_fops = {
	.owner = THIS_MODULE,
	.open = tcc_sensor_open,
	.release = tcc_sensor_release,
	.unlocked_ioctl = tcc_sensor_ioctl,
	.read = tcc_sensor_read,
	.write = tcc_sensor_write,
};

static struct class *sensor_class;

int __init tcc_sensor_init(void)
{
	int ret;

	sensor_dbg(KERN_INFO "tcc_sensor_init \n", __func__);

        ret = i2c_add_driver(&sensor_i2c_driver);
        if (ret < 0) 
            return ret;

	ret =
	    register_chrdev(SENSOR_DEV_MAJOR, SENSOR_DEV_NAME,
			    &tcc_sensor_fops);
	if (ret < 0)
		return ret;

	sensor_class = class_create(THIS_MODULE, SENSOR_DEV_NAME);
	device_create(sensor_class, NULL,
		      MKDEV(SENSOR_DEV_MAJOR, SENSOR_DEV_MINOR), NULL,
		      SENSOR_DEV_NAME);

	INIT_WORK(&sensor_work_q, sensor_fetch_thread);

	tcc_sensor_accel.delay_time = sensor_duration;

	sensor_dbg(KERN_INFO "%s\n", __func__);
	return 0;
}

void __exit tcc_sensor_exit(void)
{
	sensor_dbg(KERN_INFO "%s\n", __func__);
	i2c_del_driver(&sensor_i2c_driver);
	unregister_chrdev(SENSOR_DEV_MAJOR, SENSOR_DEV_NAME);
}

module_init(tcc_sensor_init);
module_exit(tcc_sensor_exit);

MODULE_AUTHOR("Telechips Inc. c2-g2-2 linux@telechips.com");
MODULE_DESCRIPTION("TCCxxx accel-gsensor driver");
MODULE_LICENSE("GPL");
