/* 
 * linux/drivers/char/tcc_clockctrl.c
 *
 * Author:  <android@telechips.com>
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

#include <linux/cpufreq.h>

#define dbg(msg...)	if (0) { printk( "TCC_CLOCKCTRL: " msg); }

#define CLOCKCTRL_DEV_NAME		"clockctrl"

enum {
	TCC_CLOCKCTRL_APP_MAX_CLOCK_DISABLE = 100,
	TCC_CLOCKCTRL_APP_MAX_CLOCK_ENABLE,
	TCC_CLOCKCTRL_USB_MAX_CLOCK_DISABLE = 102,
	TCC_CLOCKCTRL_USB_MAX_CLOCK_ENABLE,
	TCC_CLOCKCTRL_V2IP_MAX_CLOCK_DISABLE = 104,
	TCC_CLOCKCTRL_V2IP_MAX_CLOCK_ENABLE,	
	TCC_CLOCKCTRL_VOIP_MAX_CLOCK_DISABLE = 106,
	TCC_CLOCKCTRL_VOIP_MAX_CLOCK_ENABLE,
	TCC_CLOCKCTRL_FLASH_MAX_CLOCK_DISABLE = 108,
	TCC_CLOCKCTRL_FLASH_MAX_CLOCK_ENABLE,	
#if defined(CONFIG_CPU_HIGHSPEED)
	TCC_CLOCKCTRL_LIMIT_OVERCLOCK_DISABLE = 110,
	TCC_CLOCKCTRL_LIMIT_OVERCLOCK_ENABLE
#endif
};

static struct class *clockctrl_class;
static struct device *clockctrl_dev;
static struct workqueue_struct *tcc_clockctrl_wq;
static int MajorNum;

extern struct tcc_freq_table_t gtAppClockLimitTable;
extern struct tcc_freq_table_t gtUSBClockLimitTable[];
extern struct tcc_freq_table_t gtJpegMaxClockLimitTable;
extern struct tcc_freq_table_t gtVoipClockLimitTable[];

static long tcc_clockctrl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	dbg("%s:\n", __func__);

	switch (cmd) {
		case TCC_CLOCKCTRL_APP_MAX_CLOCK_ENABLE:
			tcc_cpufreq_set_limit_table(&gtAppClockLimitTable, TCC_FREQ_LIMIT_APP, 1);
			break;
		case TCC_CLOCKCTRL_APP_MAX_CLOCK_DISABLE:
			tcc_cpufreq_set_limit_table(&gtAppClockLimitTable, TCC_FREQ_LIMIT_APP, 0);
			break;
		case TCC_CLOCKCTRL_USB_MAX_CLOCK_ENABLE:
			tcc_cpufreq_set_limit_table(&gtUSBClockLimitTable[1], TCC_FREQ_LIMIT_USB_ACTIVED, 1);
			break;
		case TCC_CLOCKCTRL_USB_MAX_CLOCK_DISABLE:
			tcc_cpufreq_set_limit_table(&gtUSBClockLimitTable[1], TCC_FREQ_LIMIT_USB_ACTIVED, 0);
			break;
		case TCC_CLOCKCTRL_V2IP_MAX_CLOCK_ENABLE:
			tcc_cpufreq_set_limit_table(&gtJpegMaxClockLimitTable, TCC_FREQ_LIMIT_V2IP, 1);
			break;
		case TCC_CLOCKCTRL_V2IP_MAX_CLOCK_DISABLE:			
			tcc_cpufreq_set_limit_table(&gtJpegMaxClockLimitTable, TCC_FREQ_LIMIT_V2IP, 0);
			break;	
		case TCC_CLOCKCTRL_FLASH_MAX_CLOCK_ENABLE:
			tcc_cpufreq_set_limit_table(&gtJpegMaxClockLimitTable, TCC_FREQ_LIMIT_FLASH, 1);
			break;
		case TCC_CLOCKCTRL_FLASH_MAX_CLOCK_DISABLE:			
			tcc_cpufreq_set_limit_table(&gtJpegMaxClockLimitTable, TCC_FREQ_LIMIT_FLASH, 0);
			break;	
			
		case TCC_CLOCKCTRL_VOIP_MAX_CLOCK_ENABLE:
			tcc_cpufreq_set_limit_table(&gtVoipClockLimitTable[arg], TCC_FREQ_LIMIT_VOIP+arg, 1);
			break;
		case TCC_CLOCKCTRL_VOIP_MAX_CLOCK_DISABLE:
			tcc_cpufreq_set_limit_table(&gtVoipClockLimitTable[arg], TCC_FREQ_LIMIT_VOIP+arg, 0);
			break;	
#if defined(CONFIG_CPU_HIGHSPEED)
		case TCC_CLOCKCTRL_LIMIT_OVERCLOCK_ENABLE:
			tcc_cpufreq_set_limit_table(NULL, TCC_FREQ_LIMIT_OVERCLOCK, 1);
			break;
		case TCC_CLOCKCTRL_LIMIT_OVERCLOCK_DISABLE:
			tcc_cpufreq_set_limit_table(NULL, TCC_FREQ_LIMIT_OVERCLOCK, 0);
			break;
#endif
		default:
			break;
	}
	
	return 0;
}

#if defined(CONFIG_SATA_AHCI)
extern void ahci_set_retry(void);
#endif

int android_system_booting_finished = 0;
static ssize_t tcc_clockctrl_write(struct file *filp, const char __user * user, size_t size, loff_t * sss)
{
	if (android_system_booting_finished == 0)
	{
		printk("kernel boot complete\n");


		/* If SATA HDD has a problem, System will not boot-up because of SATA retry fucntion. 
		So disable retry function during boot-up, after then enable it for SATA hotplug-in/out. */
		#if defined(CONFIG_SATA_AHCI)
		ahci_set_retry();
		#endif
	}

	android_system_booting_finished = 1;
	return 0;
}

static int tcc_clockctrl_release(struct inode *inode, struct file *filp)
{
	dbg("%s:\n", __func__);
	return 0;
}

static int tcc_clockctrl_open(struct inode *inode, struct file *filp)
{
	dbg("%s:\n", __func__);
	return 0;
}

struct file_operations tcc_clockctrl_fops = {
	.owner          = THIS_MODULE,
	.open           = tcc_clockctrl_open,
	.release        = tcc_clockctrl_release,
	.unlocked_ioctl = tcc_clockctrl_ioctl,
	.write          = tcc_clockctrl_write,
};

int __init tcc_clockctrl_init(void)
{
	int err=0;

	android_system_booting_finished = 0;

	tcc_clockctrl_wq = create_singlethread_workqueue("tcc_clockctrl_wq");
	if (!tcc_clockctrl_wq)
		return -ENOMEM;

	MajorNum = register_chrdev(0, CLOCKCTRL_DEV_NAME, &tcc_clockctrl_fops);
	if (MajorNum < 0) {
		printk("%s: device failed widh %d\n", __func__, MajorNum);
		err = -ENODEV;
		goto out_wq;
	}

	clockctrl_class = class_create(THIS_MODULE, CLOCKCTRL_DEV_NAME);
	if (IS_ERR(clockctrl_dev)) {
		err = PTR_ERR(clockctrl_dev);
		goto out_chrdev;
	}
	device_create(clockctrl_class,NULL,MKDEV(MajorNum, 0),NULL,CLOCKCTRL_DEV_NAME);

	printk(KERN_INFO "%s\n", __FUNCTION__);
	goto out;

out_chrdev:
	unregister_chrdev(MajorNum, CLOCKCTRL_DEV_NAME);
out_wq:
	if (tcc_clockctrl_wq)
		destroy_workqueue(tcc_clockctrl_wq);
out:
	return err;
}

void __exit tcc_clockctrl_exit(void)
{
	device_destroy(clockctrl_class, MKDEV(MajorNum, 0));
	class_destroy(clockctrl_class);
	unregister_chrdev(MajorNum, CLOCKCTRL_DEV_NAME);
	if (tcc_clockctrl_wq)
		 destroy_workqueue(tcc_clockctrl_wq);
	
    printk(KERN_INFO "%s\n", __FUNCTION__);
}

module_init(tcc_clockctrl_init);
module_exit(tcc_clockctrl_exit);

MODULE_DESCRIPTION("TCCxxx clockctrl driver");
MODULE_LICENSE("GPL");
