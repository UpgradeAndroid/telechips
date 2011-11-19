//-------------------------------------------------------------------
// Copyright (c) Telechips, Inc.
// All right reserved.
//
//-------------------------------------------------------------------
/**
 * @file  hpd.c
 * @brief This file contains an implementation of HPD device driver.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/uaccess.h>

#include "../hdmi/regs-hdmi.h"

#define HPD_DEBUG 		1
#define HPD_DEBUG_GPIO 	0
#if HPD_DEBUG
#define DPRINTK(args...)    printk(args)
#else
#define DPRINTK(args...)
#endif

static struct clk *hdmi_hpd_clk;

static char tcc_hpd_open_num;
void tcc_hpd_initialize(void);

#define VERSION         "1.2" /* Driver version number */
#define HPD_MINOR       243 /* Major 10, Minor 243, /dev/hpd */

#define HPD_LO          0
#define HPD_HI          1


struct hpd_struct {
    spinlock_t lock;
    wait_queue_head_t waitq;
    atomic_t state;
};

static struct hpd_struct hpd_struct;

static int hpd_open(struct inode *inode, struct file *file);
static int hpd_release(struct inode *inode, struct file *file);
static ssize_t hpd_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos);
static unsigned int hpd_poll(struct file *file, poll_table *wait);
static irqreturn_t hpd_irq_handler(int irq, void *dev_id);
static int last_hpd_state;

static const struct file_operations hpd_fops =
{
    .owner   = THIS_MODULE,
    .open    = hpd_open,
    .release = hpd_release,
    .read    = hpd_read,
    .poll    = hpd_poll,
};

static struct miscdevice hpd_misc_device =
{
    HPD_MINOR,
    "hpd",
    &hpd_fops,
};


int hpd_open(struct inode *inode, struct file *file)
{
    unsigned char reg;

    DPRINTK(KERN_INFO "%s\n", __FUNCTION__);

	clk_enable(hdmi_hpd_clk);

//	if(tcc_hpd_open_num == 0)
	{
		tcc_hpd_initialize();
		tcc_hpd_open_num++;
	}

#if (1) && defined(TELECHIPS)
	writeb((1<<HDMI_IRQ_HPD_PLUG) | (1<<HDMI_IRQ_HPD_UNPLUG), HDMI_SS_INTC_FLAG);
#endif

    /* enable HPD interrupts */
    reg = readb(HDMI_SS_INTC_CON);
    writeb(reg | (1<<HDMI_IRQ_HPD_PLUG) | (1<<HDMI_IRQ_HPD_UNPLUG) | (1<<HDMI_IRQ_GLOBAL), HDMI_SS_INTC_CON);
    DPRINTK(KERN_INFO "%s end \n", __FUNCTION__);

    return 0;
}

int hpd_release(struct inode *inode, struct file *file)
{
    unsigned char reg;

    DPRINTK(KERN_INFO "%s\n", __FUNCTION__);

    /* disable HPD interrupts */
    reg = readb(HDMI_SS_INTC_CON);
    reg &= ~(1<<HDMI_IRQ_HPD_PLUG);
    reg &= ~(1<<HDMI_IRQ_HPD_UNPLUG);
    writeb(reg, HDMI_SS_INTC_CON);

	clk_disable(hdmi_hpd_clk);

    return 0;
}

ssize_t hpd_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
    ssize_t retval;

    DPRINTK(KERN_INFO "%s\n", __FUNCTION__);

    if (wait_event_interruptible(hpd_struct.waitq, atomic_read(&hpd_struct.state) != -1))
        return -ERESTARTSYS;

    spin_lock_irq(&hpd_struct.lock);
    retval = put_user(atomic_read(&hpd_struct.state), (unsigned int __user *) buffer);
    atomic_set(&hpd_struct.state, -1);
    spin_unlock_irq(&hpd_struct.lock);

    return retval;
}

unsigned int hpd_poll(struct file *file, poll_table *wait)
{
//    DPRINTK(KERN_INFO "%s\n", __FUNCTION__);

    poll_wait(file, &hpd_struct.waitq, wait);

    if (atomic_read(&hpd_struct.state) != -1)
        return POLLIN | POLLRDNORM;

    return 0;
}

/**
 * @brief HPD interrupt handler
 *
 * Handles interrupt requests from HPD hardware. \n
 * Handler changes value of internal variable and notifies waiting thread.
 */
static irqreturn_t hpd_irq_handler(int irq, void *dev_id)
{
    u8 flag;
    unsigned char reg;

    /* read flag register */
    flag = readb(HDMI_SS_INTC_FLAG);

    DPRINTK(KERN_INFO "%s  flag reg: 0x%02x  \n", __FUNCTION__, (int) flag);

    /* is this our interrupt? */
    if (!(flag & (1<<HDMI_IRQ_HPD_PLUG | 1<<HDMI_IRQ_HPD_UNPLUG))) {
        return IRQ_NONE;
    }

#if HPD_DEBUG_GPIO
	regl0 = readl(GPIOA(GP_DAT));
	
	regl1 = readl(GPIOA(GP_EN));
	
	regl2 = readl(GPIOA(GP_FN1));
    DPRINTK(KERN_INFO "A14_reg: data:0x%x en:0x%x fn:0x%x \n", regl0, regl1, regl2);
#endif//HPD_DEBUG

    if (flag == (1<<HDMI_IRQ_HPD_PLUG | 1<<HDMI_IRQ_HPD_UNPLUG) )
	{
        DPRINTK(KERN_INFO "HPD_HI && HPD_LO 0x%x \n", readb(HDMI_SS_HPD));
		
        if ( last_hpd_state == HPD_HI && readb(HDMI_SS_HPD))
            flag = 1<<HDMI_IRQ_HPD_UNPLUG;
        else
            flag = 1<<HDMI_IRQ_HPD_PLUG;
    }
	
	/* clear pending bit */
	writeb((1<<HDMI_IRQ_HPD_PLUG) | (1<<HDMI_IRQ_HPD_UNPLUG), HDMI_SS_INTC_FLAG);

    if (flag & (1<<HDMI_IRQ_HPD_PLUG)) 
	{
        DPRINTK(KERN_INFO "~  HPD_HI ~  \n");

#if 1
        // workaround: enable HDMI_IRQ_HPD_UNPLUG interrupt
        reg = readb(HDMI_SS_INTC_CON);
        reg |= 1<<HDMI_IRQ_HPD_UNPLUG;
        last_hpd_state = HPD_HI;
        writeb(reg, HDMI_SS_INTC_CON);
#endif
        atomic_set(&hpd_struct.state, HPD_HI);
        wake_up_interruptible(&hpd_struct.waitq);

    }
	else if (flag & (1<<HDMI_IRQ_HPD_UNPLUG)) 
	{
        DPRINTK(KERN_INFO "HPD_LO\n");
#if 1
//??? add comments
        // workaround: disable HDMI_IRQ_HPD_UNPLUG interrupt
        reg = readb(HDMI_SS_INTC_CON);
        reg &= ~(1<<HDMI_IRQ_HPD_UNPLUG);
        last_hpd_state = HPD_LO;
        writeb(reg, HDMI_SS_INTC_CON);
#endif

        atomic_set(&hpd_struct.state, HPD_LO);
        wake_up_interruptible(&hpd_struct.waitq);
    }

    return IRQ_HANDLED;
}

void tcc_hpd_initialize(void)
{
	unsigned int regl;
    DPRINTK(KERN_INFO "! %s  !\n", __FUNCTION__);


	regl = readl(GPIOA(GP_DAT));
	writel((regl & (~(1<<14))), GPIOA(GP_DAT)); 	//GPIO_A14 -> HPD intpur mode
	
	regl = readl(GPIOA(GP_EN));
	writel((regl & (~(1<<14))), GPIOA(GP_EN));		//GPIO_A14 -> HPD intpur mode
	
	regl = readl(GPIOA(GP_FN1));
	writel((regl & ~(0xF<<24)) | 1<<24, GPIOA(GP_FN1)); 	//GPIO_A14 -> HPD

	// HW HPD On
	writeb(HPD_SW_DISABLE|HPD_ON,HDMI_HPD);
	
	// setting PIC HDMI interrupt
	regl = readl(PIC_POL1);
	writel(regl & ~(1<<2), PIC_POL1);	// active-high

#if (1)
	regl = readl(PIC_MODE1);
	writel(regl | (1<<2), PIC_MODE1);	// level-triggered
#else
	regl = readl(PIC_MODE1);
	writel(regl & ~(1<<2), PIC_MODE1);	// edge-triggered
#endif
	regl = readl(PIC_MODEA1);
	writel(regl & ~(1<<2), PIC_MODEA1); // single-edge
	
    /* adjust the duration of HPD detection */
	writeb(0xFF, HDMI_HPD_GEN);

#ifndef TELECHIPS
    if (request_irq(IRQ_HDMI, hpd_irq_handler, IRQF_SHARED, "hpd", hpd_irq_handler))
    {
        printk(KERN_WARNING "HPD: IRQ %d is not free.\n", IRQ_HDMI);
        misc_deregister(&hpd_misc_device);
        return -EIO;
    }
#endif//TELECHIPS
    DPRINTK(KERN_INFO "%s END\n", __FUNCTION__);

}

static int hpd_probe(struct platform_device *pdev)
{
    printk(KERN_INFO "HPD Driver ver. %s (built %s %s)\n", VERSION, __DATE__, __TIME__);

    if (!machine_is_hdmidp())
        return -ENODEV;

	hdmi_hpd_clk = clk_get(0, "hdmi");
	
	clk_enable(hdmi_hpd_clk);


    if (misc_register(&hpd_misc_device))
    {
        printk(KERN_WARNING "HPD: Couldn't register device 10, %d.\n", HPD_MINOR);
        return -EBUSY;
    }

    init_waitqueue_head(&hpd_struct.waitq);
    spin_lock_init(&hpd_struct.lock);

    atomic_set(&hpd_struct.state, -1);
	
#ifndef TELECHIPS
    /* adjust the duration of HPD detection */
	writeb(0xFF, HDMI_HPD_GEN);
#endif//TELECHIPS

    if (request_irq(IRQ_HDMI, hpd_irq_handler, IRQF_SHARED, "hpd", hpd_irq_handler))
    {
        printk(KERN_WARNING "HPD: IRQ %d is not free.\n", IRQ_HDMI);
        misc_deregister(&hpd_misc_device);
        return -EIO;
    }
    DPRINTK(KERN_INFO "3 %s\n", __FUNCTION__);

	clk_disable(hdmi_hpd_clk);

    return 0;
}

static int hpd_remove(struct platform_device *pdev)
{
    DPRINTK(KERN_INFO "%s\n", __FUNCTION__);

    free_irq(IRQ_HDMI, hpd_irq_handler);
    misc_deregister(&hpd_misc_device);
	return 0;
}

static struct platform_driver tcc_hdmi_hpd = {
	.probe	= hpd_probe,
	.remove	= hpd_remove,
	.driver	= {
		.name	= "tcc_hdmi_hpd",
		.owner	= THIS_MODULE,
	},
};
static __init int hpd_init(void)
{
	return platform_driver_register(&tcc_hdmi_hpd);
}

static __exit void hpd_exit(void)
{
	platform_driver_unregister(&tcc_hdmi_hpd);
}


module_init(hpd_init);
module_exit(hpd_exit);
MODULE_AUTHOR("Telechips Inc. <linux@telechips.com>");
MODULE_LICENSE("GPL");

