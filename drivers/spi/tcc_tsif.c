/*
 * linux/drivers/char/tsif/tcc_tsif_module.c
 *
 * Author:  <linux@telechips.com>
 * Created: 1st April, 2009
 * Description: Driver for Telechips TS Parallel/Serial Controllers
 *
 * Copyright (c) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/poll.h>
#include <linux/spi/spi.h>

#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/sched.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <asm/atomic.h>

#include <mach/bsp.h>
#include <mach/gpio.h>
#include <linux/spi/tcc_tsif_module.h>
#include <mach/tca_tsif.h>


#define tca_ckc_set_iobus_swreset
#define RB_GDMA1CONTROLLER  0

static unsigned int guiPCRPID;
static struct clk *gpsb_clk;
static struct clk *tsif_clk;

struct tcc_tsif_module_pri_handle {
    wait_queue_head_t wait_q;
    struct mutex mutex;
    int open_cnt;
    u32 gpio_port;
};

static struct tcc_tsif_module_handle tsif_module_handle;
static struct tcc_tsif_module_pri_handle tsif_module_pri;

static int __init tsif_module_drv_probe(struct platform_device *pdev)
{

    return 0;
}


/*
 * NOTE: before suspend, you must close spi.
 */
static int tsif_module_drv_suspend(struct platform_device *dev, pm_message_t state)
{
	return 0;
}
static int tsif_module_drv_resume(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver tsif_module_platform_driver = {
	.driver = {
		.name	= "tcc-tsif-module",
		.owner	= THIS_MODULE,
	},
	.suspend = tsif_module_drv_suspend,
	.resume  = tsif_module_drv_resume,
};


static void tea_free_dma_linux(struct tea_dma_buf *tdma)
{
    if(tdma) {
        if(tdma->v_addr != 0) {
            dma_free_writecombine(0, tdma->buf_size, tdma->v_addr, tdma->dma_addr);

			printk("tcc-tsif-module : dma buffer free @0x%X(Phy=0x%X), size:%d\n", (unsigned int)tdma->v_addr, (unsigned int)tdma->dma_addr, tdma->buf_size);
        }
        memset(tdma, 0, sizeof(struct tea_dma_buf));
    }
}

static int tea_alloc_dma_linux(struct tea_dma_buf *tdma, unsigned int size)
{
    int ret = -1;

    if(tdma) {
        tea_free_dma_linux(tdma);

        tdma->buf_size = size;
        tdma->v_addr = dma_alloc_writecombine(0, tdma->buf_size, &tdma->dma_addr, GFP_KERNEL);

        ret = tdma->v_addr ? 0 : 1;

		printk("tcc-tsif-module : dma buffer alloc @0x%X(Phy=0x%X), size:%d\n", (unsigned int)tdma->v_addr, (unsigned int)tdma->dma_addr, tdma->buf_size);
    }

    return ret;
} 

static irqreturn_t tsif_module_dma_handler(int irq, void *dev_id)
{

    return IRQ_HANDLED;
}


static int tsif_get_readable_cnt(struct tcc_tsif_module_handle *h)
{

	return 0;
}

static ssize_t tcc_tsif_module_read(struct file *filp, char *buf, size_t len, loff_t *ppos)
{
    return 0;
}

static unsigned int tcc_tsif_module_poll(struct file *filp, struct poll_table_struct *wait)
{

    return 0;
}

static ssize_t tcc_tsif_module_write(struct file *filp, const char *buf, size_t len, loff_t *ppos)
{
    return 0;
}

static int tcc_tsif_module_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	switch (cmd) {
        case IOCTL_TSIF_DMA_START:
			{
			}
			break;			
		case IOCTL_TSIF_DMA_STOP:
    	    break;			
		case IOCTL_TSIF_GET_MAX_DMA_SIZE:
			{
	        }
	        break;
			
		case IOCTL_TSIF_SET_PID:
			{
			}
            break;

		case IOCTL_TSIF_DXB_POWER:
			break;

        case IOCTL_TSIF_SET_PCRPID:
            {
            }
            break;
        case IOCTL_TSIF_GET_STC:
             {
             }
             break;

	    default:
	        printk("tcc-tsif-module : unrecognized ioctl (0x%X)\n", cmd);
	        ret = -EINVAL;
	        break;
    }
    return ret;
}

static int tcc_tsif_module_init(int tsif_num)
{

	return 0;
}

static void tcc_tsif_module_deinit(int tsif_num)
{
	

}


static int tcc_tsif_module_open(struct inode *inode, struct file *filp)
{
    return 0;
}


static int tcc_tsif_module_release(struct inode *inode, struct file *filp)
{
    return 0;
}


struct file_operations tcc_tsif_module_fops = {
    .owner          = THIS_MODULE,
    .read           = tcc_tsif_module_read,
    .write          = tcc_tsif_module_write,
    .unlocked_ioctl = tcc_tsif_module_ioctl,
    .open           = tcc_tsif_module_open,
    .release        = tcc_tsif_module_release,
    .poll           = tcc_tsif_module_poll,
};

static struct class *tsif_module_class; //jhlim

static int __init tsif_module_init(void)
{
    return 0;
}


static void __exit tsif_module_exit(void)
{

}
module_init(tsif_module_init);
module_exit(tsif_module_exit);

MODULE_AUTHOR("Telechips Inc. linux@telechips.com");
MODULE_DESCRIPTION("Telechips TSIF driver");
MODULE_LICENSE("GPL");

