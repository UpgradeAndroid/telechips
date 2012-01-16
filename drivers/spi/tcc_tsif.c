/*
 * linux/drivers/char/tsif/tcc_tsif.c
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

#include <mach/clock.h>
#include <mach/bsp.h>
#include <mach/gpio.h>
#include <linux/spi/tcc_tsif.h>
#include <mach/tca_tsif.h>
#include "tsdemux/TSDEMUX_sys.h"

#define USE_STATIC_DMA_BUFFER
#undef  TSIF_DMA_SIZE
#define TSIF_DMA_SIZE 0x200000

struct tea_dma_buf *g_static_dma;

static struct clk *tsif_clk;

struct tcc_tsif_pri_handle {
    wait_queue_head_t wait_q;
    struct mutex mutex;
    int open_cnt;
	u32 tsif_port;
	u32 tsif_channel;
	u32 reg_base;
	u32 drv_major_num;
	u32 pcr_pid;
	u32 bus_num;
	u32 irq_no;
	const char *name;
};

static tcc_tsif_handle_t tsif_ex_handle;
static struct tcc_tsif_pri_handle tsif_ex_pri;

static ssize_t show_port(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "tsif port : %d\n", tsif_ex_pri.tsif_port);

}
static ssize_t store_port(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 port;

	port = simple_strtoul(buf, (char **)NULL, 16);
	/* valid port: 0xC, 0xD, 0xE */
	if (port > 20 ) {
		printk("tcc-tsif: invalid port! (use 0xc/d/e)\n");
		return -EINVAL;
	}

	tsif_ex_pri.tsif_port = port;
	return count;

}
static DEVICE_ATTR(tcc_port, S_IRUSR|S_IWUSR, show_port, store_port);

static int __init tsif_ex_drv_probe(struct platform_device *pdev)
{
    int ret = 0;
	int irq = -1;
    struct resource *regs = NULL;
    struct resource *port = NULL;

    regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!regs) {
        return -ENXIO;
    }
    irq = platform_get_irq(pdev, 0);
    if (irq < 0) {
        return -ENXIO;
    }
    port = platform_get_resource(pdev, IORESOURCE_IO, 0);
    if (!port) {
        return -ENXIO;
    }

	tsif_ex_pri.bus_num =  pdev->id;
	tsif_ex_pri.irq_no = irq;
	tsif_ex_pri.reg_base = regs->start;
	tsif_ex_pri.tsif_port = port->start;
	tsif_ex_pri.name = port->name;

	printk("%s:[%s]\n",__func__, tsif_ex_pri.name);
    tsif_clk = clk_get(NULL, tsif_ex_pri.name);

	platform_set_drvdata(pdev, &tsif_ex_handle);

	/* TODO: device_remove_file(&pdev->dev, &dev_attr_tcc_port); */
	ret = device_create_file(&pdev->dev, &dev_attr_tcc_port);

	//printk("[%s]%d: init port:%d re:%d\n", pdev->name, gTSIFCH, tsif_ex_pri.gpio_port, ret);
	return 0;
}


/*
 * NOTE: before suspend, you must close tsif.
 */
static int tsif_ex_drv_suspend(struct platform_device *dev, pm_message_t state)
{
	return 0;
}
static int tsif_ex_drv_resume(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver tsif_ex_platform_driver = {
	.driver = {
		.name	= "tcc-tsif_ex",
		.owner	= THIS_MODULE,
	},
	.suspend = tsif_ex_drv_suspend,
	.resume  = tsif_ex_drv_resume,
};


static void tea_free_dma_linux(struct tea_dma_buf *tdma)
{
    if(g_static_dma){        
        return;
    }
    if(tdma) {
        if(tdma->v_addr != 0) {
            dma_free_writecombine(0, tdma->buf_size, tdma->v_addr, tdma->dma_addr);

			printk("tcc-tsif_ex : dma buffer free @0x%X(Phy=0x%X), size:%d\n", (unsigned int)tdma->v_addr, (unsigned int)tdma->dma_addr, tdma->buf_size);
        }
        memset(tdma, 0, sizeof(struct tea_dma_buf));
    }
}

static int tea_alloc_dma_linux(struct tea_dma_buf *tdma, unsigned int size)
{
    int ret = -1;

    if(g_static_dma){        
        tdma->buf_size = g_static_dma->buf_size;
        tdma->v_addr = g_static_dma->v_addr;
        tdma->dma_addr = g_static_dma->dma_addr;
        return 0;
    }

    if(tdma) {
        tea_free_dma_linux(tdma);

        tdma->buf_size = size;
        tdma->v_addr = dma_alloc_writecombine(0, tdma->buf_size, &tdma->dma_addr, GFP_KERNEL);

        ret = tdma->v_addr ? 0 : 1;

		printk("tcc-tsif_ex : dma buffer alloc @0x%X(Phy=0x%X), size:%d\n", (unsigned int)tdma->v_addr, (unsigned int)tdma->dma_addr, tdma->buf_size);
    }

    return ret;
} 

static irqreturn_t tsif_ex_dma_handler(int irq, void *dev_id)
{
    tcc_tsif_handle_t *handle = (tcc_tsif_handle_t *)dev_id;
    struct tcc_tsif_pri_handle *h_pri = (struct tcc_tsif_pri_handle *)handle->private_data;
  	if (h_pri->open_cnt > 0) 
   	{
        handle->tsif_isr(handle);
        //Check PCR & Make STC
        if(h_pri->pcr_pid < 0x1FFF )
        {	
            int start_pos, search_pos, search_size;
            start_pos = handle->cur_q_pos - handle->dma_intr_packet_cnt;
            if(start_pos > 0 )
            {
                search_pos = start_pos;
                search_size = handle->dma_intr_packet_cnt;
            }	
            else
            {
                search_pos = 0;
                search_size = handle->cur_q_pos;
            }	
            TSDEMUX_MakeSTC((unsigned char *)handle->rx_dma.v_addr + search_pos*TSIF_PACKET_SIZE, search_size*TSIF_PACKET_SIZE, h_pri->pcr_pid );				
        }			
        wake_up(&(h_pri->wait_q));
    }
    return IRQ_HANDLED;
}


static int tsif_get_readable_cnt(struct tcc_tsif_handle *H)
{
    if (H) {
        int dma_pos = H->cur_q_pos;
        int q_pos = H->q_pos;
        int readable_cnt = 0;
        if (dma_pos > q_pos) {
            readable_cnt = dma_pos - q_pos;
        } else if (dma_pos < q_pos) {
            readable_cnt = H->dma_total_packet_cnt - q_pos;
            readable_cnt += dma_pos;
        } 

        //check data validation
        if(tsif_ex_handle.mpeg_ts == (Hw0|Hw1) && readable_cnt){
            if(q_pos < H->dma_total_packet_cnt)
            {
                char *sync_byte = (char *)tsif_ex_handle.rx_dma.v_addr + q_pos * TSIF_PACKET_SIZE;
                if( *sync_byte != 0x47){
                    printk("tsif-resync !!!!\n");
                    tsif_ex_handle.dma_stop(&tsif_ex_handle);
                    clk_disable(tsif_clk);
                    msleep(1);
                    clk_enable(tsif_clk); 
                    msleep(1);
                    tsif_ex_handle.hw_init(&tsif_ex_handle);
                    tsif_ex_handle.tsif_set(&tsif_ex_handle);
                    tca_tsif_register_pids(&tsif_ex_handle, tsif_ex_handle.match_pids, tsif_ex_handle.match_pids_count);
                    tsif_ex_handle.q_pos = tsif_ex_handle.cur_q_pos = 0;
                    tsif_ex_handle.dma_start(&tsif_ex_handle);                           
                    return 0;
                }
            }
        }
        return readable_cnt;
    }
	return 0;
}

static ssize_t tcc_tsif_read(struct file *filp, char *buf, size_t len, loff_t *ppos)
{
    int readable_cnt = 0, copy_cnt = 0;
    int copy_byte = 0;

    readable_cnt = tsif_get_readable_cnt(&tsif_ex_handle);
    if (readable_cnt > 0) {
        copy_byte = readable_cnt * TSIF_PACKET_SIZE;
        if (copy_byte > len) {
            copy_byte = len;
        }

        copy_byte -= copy_byte % TSIF_PACKET_SIZE;
        copy_cnt = copy_byte / TSIF_PACKET_SIZE;
        copy_cnt -= copy_cnt % tsif_ex_handle.dma_intr_packet_cnt;
        copy_byte = copy_cnt * TSIF_PACKET_SIZE;

        if (copy_cnt >= tsif_ex_handle.dma_intr_packet_cnt) {
            int offset = tsif_ex_handle.q_pos * TSIF_PACKET_SIZE;
            if (copy_cnt > tsif_ex_handle.dma_total_packet_cnt - tsif_ex_handle.q_pos) {
                int first_copy_byte = (tsif_ex_handle.dma_total_packet_cnt - tsif_ex_handle.q_pos) * TSIF_PACKET_SIZE;
                int first_copy_cnt = first_copy_byte / TSIF_PACKET_SIZE;
                int second_copy_byte = (copy_cnt - first_copy_cnt) * TSIF_PACKET_SIZE;

                if (copy_to_user(buf, tsif_ex_handle.rx_dma.v_addr + offset, first_copy_byte)) {
                    return -EFAULT;
                }
                if (copy_to_user(buf + first_copy_byte, tsif_ex_handle.rx_dma.v_addr, second_copy_byte)) {
                    return -EFAULT;
                }

                tsif_ex_handle.q_pos = copy_cnt - first_copy_cnt;
            } else {
                if (copy_to_user(buf, tsif_ex_handle.rx_dma.v_addr + offset, copy_byte)) {
                    return -EFAULT;
                }

                tsif_ex_handle.q_pos += copy_cnt;
                if (tsif_ex_handle.q_pos >= tsif_ex_handle.dma_total_packet_cnt) {
                    tsif_ex_handle.q_pos = 0;
                }
            }
            return copy_byte;
        }
    }
    return 0;
}

static unsigned int tcc_tsif_poll(struct file *filp, struct poll_table_struct *wait)
{
    if (tsif_get_readable_cnt(&tsif_ex_handle) >= tsif_ex_handle.dma_intr_packet_cnt) {
		return  (POLLIN | POLLRDNORM);
    }

    poll_wait(filp, &(tsif_ex_pri.wait_q), wait);
    if (tsif_get_readable_cnt(&tsif_ex_handle) >= tsif_ex_handle.dma_intr_packet_cnt) {
		return  (POLLIN | POLLRDNORM);
    }
    return 0;
}

static ssize_t tcc_tsif_write(struct file *filp, const char *buf, size_t len, loff_t *ppos)
{
    return 0;
}

static int tcc_tsif_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	//printk("%s[0x%X] - in\n", __func__, cmd);
    switch (cmd) {
    case IOCTL_TSIF_DMA_START:
        {
            struct tcc_tsif_param param;
            int clk_polarity,  valid_polarity, sync_polarity, msb_first, mpeg_ts;
            if (copy_from_user(&param, (void *)arg, sizeof(struct tcc_tsif_param))) {
                printk("cannot copy from user tcc_tsif_param in IOCTL_TSIF_DMA_START !!! \n");
                return -EFAULT;
            }

            if (((TSIF_PACKET_SIZE * param.ts_total_packet_cnt) > tsif_ex_handle.dma_total_size)
                || (param.ts_total_packet_cnt <= 0)) {
                printk("so big ts_total_packet_cnt !!! \n");
                param.ts_total_packet_cnt = tsif_ex_handle.dma_total_size / TSIF_PACKET_SIZE;
            }

            if(tsif_ex_handle.dma_stop(&tsif_ex_handle) == 0)
                tsif_ex_handle.clear_fifo_packet(&tsif_ex_handle);

            if(param.dma_mode == 1)
                tsif_ex_handle.mpeg_ts |= Hw0;
            else
                tsif_ex_handle.mpeg_ts = 0;

            if(param.mode & SPI_CPOL)
               clk_polarity = 0x01;    //1:falling edge 0:rising edge
            else
               clk_polarity = 0x00;    //1:falling edge 0:rising edge

            sync_polarity = 0x00;   //0:sync high active 1:sync low active
            if(param.mode & SPI_CS_HIGH)
            {
                valid_polarity = 0x01;  //1:valid high active 0:valid low active
                if(tsif_ex_handle.mpeg_ts == 0)
                    sync_polarity = 0x00;   //0:sync high active 1:sync low active
            }
            else
            {
                valid_polarity = 0x00;  //1:valid high active 0:valid low active
                if(tsif_ex_handle.mpeg_ts == 0)
                    sync_polarity = 0x01;   //0:sync high active 1:sync low active
            }

            if(param.mode & SPI_LSB_FIRST)
           	    msb_first = 0x00;       //1:msb first, 0:lsb first
            else
               	msb_first = 0x01;       //1:msb first, 0:lsb first

            if( tsif_ex_handle.clk_polarity != clk_polarity ||\
                tsif_ex_handle.valid_polarity != valid_polarity ||\
                tsif_ex_handle.sync_polarity != sync_polarity ||\
                tsif_ex_handle.msb_first != msb_first)
            {
                tsif_ex_handle.msb_first = msb_first;       //1:msb first, 0:lsb first
                tsif_ex_handle.clk_polarity = clk_polarity;    //1:falling edge 0:rising edge
                tsif_ex_handle.valid_polarity = valid_polarity;  //1:valid high active 0:valid low active
                tsif_ex_handle.sync_polarity = sync_polarity;   //0:sync high active 1:sync low active
                tsif_ex_handle.tsif_set(&tsif_ex_handle);
            }

            tsif_ex_handle.dma_total_packet_cnt = param.ts_total_packet_cnt;
            tsif_ex_handle.dma_intr_packet_cnt = param.ts_intr_packet_cnt;
            tsif_ex_handle.q_pos = tsif_ex_handle.cur_q_pos = 0;
            printk("interrupt packet count [%u]\n", tsif_ex_handle.dma_intr_packet_cnt);
            tsif_ex_handle.dma_start(&tsif_ex_handle);                        
        }
        break;			
    case IOCTL_TSIF_DMA_STOP:
            tsif_ex_handle.dma_stop(&tsif_ex_handle);
        break;			
    case IOCTL_TSIF_GET_MAX_DMA_SIZE:
        {
            struct tcc_tsif_param param;
            param.ts_total_packet_cnt = tsif_ex_handle.dma_total_size / TSIF_PACKET_SIZE;
            param.ts_intr_packet_cnt = 1;

            if (copy_to_user((void *)arg, (void *)&param, sizeof(struct tcc_tsif_param))) {
                printk("cannot copy to user tcc_tsif_param in IOCTL_TSIF_GET_MAX_DMA_SIZE !!! \n");
                return -EFAULT;
            }
        }
        break;        
    case IOCTL_TSIF_SET_PID:
        {
            struct tcc_tsif_pid_param param;
            if (copy_from_user(&param, (void *)arg, sizeof(struct tcc_tsif_pid_param))) {
                printk("cannot copy from user tcc_tsif_pid_param in IOCTL_TSIF_SET_PID !!! \n");
                return -EFAULT;
            }
            ret = tca_tsif_register_pids(&tsif_ex_handle, param.pid_data, param.valid_data_cnt);
        } 
        break;		    
    case IOCTL_TSIF_DXB_POWER:
        break;

    case IOCTL_TSIF_SET_PCRPID:
        if (copy_from_user((void *)&tsif_ex_pri.pcr_pid, (const void *)arg, sizeof(int))) {
            return -EFAULT;
        }		
        printk("Set PCR PID[0x%X]\n", tsif_ex_pri.pcr_pid);
        if( tsif_ex_pri.pcr_pid < 0x1FFF)
            TSDEMUX_Open();
        break;
    case IOCTL_TSIF_GET_STC:
        {
            unsigned int uiSTC;
            uiSTC = TSDEMUX_GetSTC();
            //printk("STC %d\n", uiSTC);
            if (copy_to_user((void *)arg, (void *)&uiSTC, sizeof(int))) {
                printk("cannot copy to user tcc_tsif_param in IOCTL_TSIF_GET_PCR !!! \n");
                return -EFAULT;
            }
        }
        break;
    default:
        printk("tcc-tsif : unrecognized ioctl (0x%X)\n", cmd);
        ret = -EINVAL;
        break;
    }
	//printk("%s[0x%X] - out\n", __func__, cmd);
    return ret;
}

static int tcc_tsif_init(void)
{
    int ret = 0, tsif_channel = 0;
    memset(&tsif_ex_handle, 0, sizeof(tcc_tsif_handle_t));   
    tsif_channel = 0;
   	if(!strcmp(tsif_ex_pri.name,"tsif1"))
        tsif_channel = 1;
    else if(!strcmp(tsif_ex_pri.name,"tsif2"))
        tsif_channel = 2;

	if (tca_tsif_init(&tsif_ex_handle,
					 (volatile struct tcc_tsif_regs *)tsif_ex_pri.reg_base,
                     tea_alloc_dma_linux,
                     tea_free_dma_linux,
                     TSIF_DMA_SIZE,
                     tsif_channel,
                     0,
                     0,
					 tsif_ex_pri.tsif_port)) {
        printk("%s: tca_tsif_init error !!!!!\n", __func__);
		ret = -EBUSY;
		goto err_tsif;
    }
   
    tsif_ex_handle.serial_mode = 1;
	tsif_ex_handle.private_data = (void *)&tsif_ex_pri;
    init_waitqueue_head(&(tsif_ex_pri.wait_q));

    tsif_ex_handle.dma_total_packet_cnt = tsif_ex_handle.dma_total_size / TSIF_PACKET_SIZE;
    tsif_ex_handle.dma_intr_packet_cnt = 1;
    tsif_ex_handle.hw_init(&tsif_ex_handle);
    tsif_ex_handle.clear_fifo_packet(&tsif_ex_handle);
    tsif_ex_handle.dma_stop(&tsif_ex_handle);
    tsif_ex_handle.msb_first = 0x01;       //1:msb first, 0:lsb first
    tsif_ex_handle.clk_polarity = 0x00;    //1:falling edge 0:rising edge
    tsif_ex_handle.valid_polarity = 0x01;  //1:valid high active 0:valid low active
    tsif_ex_handle.sync_polarity = 0x00;   //0:sync high active 1:sync low active
    tsif_ex_handle.big_endian = 0x00;	     //1:big endian, 0:little endian
    tsif_ex_handle.serial_mode = 0x01;     //1:serialmode 0:parallel mode
    tsif_ex_handle.sync_delay = 0x00;
	tsif_ex_handle.mpeg_ts = 0;
    tsif_ex_handle.tsif_set(&tsif_ex_handle);
	
    ret = request_irq(tsif_ex_handle.irq, tsif_ex_dma_handler, IRQF_SHARED, TSIF_DEV_NAME, &tsif_ex_handle);
	if (ret) { 
		goto err_irq; 
	}
	return 0;

err_irq:
	free_irq(tsif_ex_handle.irq, &tsif_ex_handle);
	
err_tsif:
	tca_tsif_clean(&tsif_ex_handle);
	return ret;
}

static void tcc_tsif_deinit(void)
{
	free_irq(tsif_ex_handle.irq, &tsif_ex_handle);
    tca_tsif_clean(&tsif_ex_handle);
}


static int tcc_tsif_open(struct inode *inode, struct file *filp)
{
    int ret = 0;	
    if (tsif_ex_pri.open_cnt == 0) {
        tsif_ex_pri.open_cnt++;
    } else {
        return -EBUSY;
    }

   	if(tsif_clk == NULL)
        return -EBUSY;

    clk_enable(tsif_clk); 
    mutex_lock(&(tsif_ex_pri.mutex));
	tsif_ex_pri.pcr_pid = 0xFFFF;
	tcc_tsif_init();
    mutex_unlock(&(tsif_ex_pri.mutex));
	return ret;
}


static int tcc_tsif_release(struct inode *inode, struct file *filp)
{
    if (tsif_ex_pri.open_cnt > 0) {
        tsif_ex_pri.open_cnt--;
    }	

    if(tsif_ex_pri.open_cnt == 0)
    {
   	    mutex_lock(&(tsif_ex_pri.mutex));
        tsif_ex_handle.dma_stop(&tsif_ex_handle);
        tcc_tsif_deinit();
      	TSDEMUX_Close();
        mutex_unlock(&(tsif_ex_pri.mutex));
        clk_disable(tsif_clk);
    }    
    return 0;
}


struct file_operations tcc_tsif_ex_fops = {
    .owner          = THIS_MODULE,
    .read           = tcc_tsif_read,
    .write          = tcc_tsif_write,
    .unlocked_ioctl = tcc_tsif_ioctl,
    .open           = tcc_tsif_open,
    .release        = tcc_tsif_release,
    .poll           = tcc_tsif_poll,
};

static struct class *tsif_ex_class;
int tsif_ex_init(void)
{
    int ret = 0;

    memset(&tsif_ex_pri, 0, sizeof(struct tcc_tsif_pri_handle));
    mutex_init(&(tsif_ex_pri.mutex));
    ret = register_chrdev(0, TSIF_DEV_NAME, &tcc_tsif_ex_fops);
    if (ret < 0) {
        printk("[%s:%d] register_chrdev error !!!!!\n", __func__, __LINE__); 
        return ret;
    }
    tsif_ex_pri.drv_major_num = ret;
    printk("[%s:%d] major number = %d\n", __func__, __LINE__, tsif_ex_pri.drv_major_num);
    g_static_dma = NULL;
#ifdef      USE_STATIC_DMA_BUFFER
     g_static_dma = kmalloc(sizeof(struct tea_dma_buf), GFP_KERNEL);
     if(g_static_dma)
     {
        g_static_dma->buf_size = TSIF_DMA_SIZE;
        g_static_dma->v_addr = dma_alloc_writecombine(0, g_static_dma->buf_size, &g_static_dma->dma_addr, GFP_KERNEL);
		printk("tcc-tsif_ex : dma buffer alloc @0x%X(Phy=0x%X), size:%d\n", (unsigned int)g_static_dma->v_addr, (unsigned int)g_static_dma->dma_addr, g_static_dma->buf_size);
        if(g_static_dma->v_addr == NULL)
        {
            kfree(g_static_dma);
            g_static_dma = NULL;
        }
     }
#endif    

    ret = platform_driver_probe(&tsif_ex_platform_driver, tsif_ex_drv_probe);

    tsif_ex_class = class_create(THIS_MODULE, TSIF_DEV_NAME);
    device_create(tsif_ex_class, NULL, MKDEV(tsif_ex_pri.drv_major_num, 1), NULL, TSIF_DEV_NAME);

	tsif_ex_pri.pcr_pid = 0xFFFF;
    return 0;
}

void tsif_ex_exit(void)
{
   	unregister_chrdev(tsif_ex_pri.drv_major_num, TSIF_DEV_NAME);
    platform_driver_unregister(&tsif_ex_platform_driver);
	if(tsif_clk)
	{
	    clk_disable(tsif_clk);
    	clk_put(tsif_clk);
	}
	if(g_static_dma)
    {
        dma_free_writecombine(0, g_static_dma->buf_size, g_static_dma->v_addr, g_static_dma->dma_addr);
        kfree(g_static_dma);
        g_static_dma = NULL;
    }
}

#if 1
EXPORT_SYMBOL(tsif_ex_init);
EXPORT_SYMBOL(tsif_ex_exit);
#else
static int __init tsif_ex_init(void)
{
    return tsif_ex_init();
}

static void __exit tsif_ex_exit(void)
{
    tsif_ex_exit();
}
module_init(tsif_ex_init);
module_exit(tsif_ex_exit);
MODULE_AUTHOR("Telechips Inc. linux@telechips.com");
MODULE_DESCRIPTION("Telechips TSIF driver");
MODULE_LICENSE("GPL");
#endif
