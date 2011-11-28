/*
 * linux/drivers/char/tcc_intr.c 
 *
 * Author:  <linux@telechips.com>
 * Created: 31th March, 2009 
 * Description: User-level interrupt Driver 
 *
 * Copyright (c) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/poll.h>
#include <linux/sched.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include <mach/bsp.h>
#include <mach/irqs.h>
#include <linux/tcc_intr.h>

//#define VC_INT_PROFILE // CAM_28pin, GPIO_D24
#ifdef VC_INT_PROFILE
static unsigned char toggle_int = 0;
#endif

//#define DBG_GPIO
#ifdef DBG_GPIO
#define TEST_GPIO(x) test_gpio(x)	
#else
#define TEST_GPIO(x)
#endif

#define TCC_INTR_VERSION	"2.0"
#define TCC_INTR_DEV_NAME	"tcc_intr"
#define TCC_INTR_DEV_VC		"tcc-intr-vc"
#define TCC_INTR_DEV_BROADCAST	"tcc-intr-bc"

#define VIPC_TIG0	*(volatile long *)0xF0401030

typedef struct _tcc_intr_data_t {
	wait_queue_head_t wq;
	spinlock_t lock;
	unsigned int count;
	unsigned int irq;
	unsigned int irq_bit;
	volatile PPIC irq_regs;
} tcc_intr_data_t;


/*
 *	TEST GPIO FUNCTION
 */
#ifdef DBG_GPIO
static int status;
static void test_gpio(int flag)
{
	if (flag == 0) {
		// gpio low
		BITCLR(*(volatile unsigned int *)0xF0102100, Hw22);			// low
	} else if (flag == 1) {
		// gpio high
		BITSET(*(volatile unsigned int *)0xF0102100, Hw22);			// high
	} else if (flag == 2) {
		// toggle
		if (status)	{
			BITCLR(*(volatile unsigned int *)0xF0102100, Hw22);			// low
			status = 0;
		} else { 
			BITSET(*(volatile unsigned int *)0xF0102100, Hw22);			// high
			status = 1;
		}
	} else if (flag == 3) {
		// gpio init 
		// GPIO_E22 - CAM_HS - JH2[5]
		BITCLR(*(volatile unsigned int *)0xF010212C, Hw28-Hw24);	// GPIO_E22
		BITSET(*(volatile unsigned int *)0xF0102104, Hw22);			// output
		BITCLR(*(volatile unsigned int *)0xF0102100, Hw22);			// low

		BITCLR(*(volatile unsigned int *)0xF0102130, Hw16-Hw12);	// GPIO_E26
		BITSET(*(volatile unsigned int *)0xF0102104, Hw26);			// output
		BITCLR(*(volatile unsigned int *)0xF0102100, Hw26);			// low
	} else {
	}
}
#endif

/*
 *	Video Codec Interface.
 */
static void init_vc_interrupt(tcc_intr_data_t *tcc_intr)
{
#if 0
	BITCLR(tcc_intr->irq_regs->MODE0, tcc_intr->irq_bit);	// edge-triggered
	BITCLR(tcc_intr->irq_regs->MODEA0, tcc_intr->irq_bit);	// single-edge
	BITSET(tcc_intr->irq_regs->POL0, tcc_intr->irq_bit);	// active-low
#endif
#if 0
	BITSET(tcc_intr->irq_regs->MODE0, tcc_intr->irq_bit);	// level-triggered
	BITSET(tcc_intr->irq_regs->POL0, tcc_intr->irq_bit);	// active-low
#endif
	BITCLR(tcc_intr->irq_regs->MODE0, tcc_intr->irq_bit);	// edge-triggered
	BITCLR(tcc_intr->irq_regs->MODEA0, tcc_intr->irq_bit);	// single-edge
	BITCLR(tcc_intr->irq_regs->POL0, tcc_intr->irq_bit);	// active-high
}

static irqreturn_t vc_handler(int irq, void *dev_id)
{
	tcc_intr_data_t *tcc_intr = dev_id;

#ifdef VC_INT_PROFILE
	if(toggle_int)
	{
		(HwGPIOD->GPEN |= Hw24);	(HwGPIOD->GPDAT |= Hw24);
		toggle_int = 0;
	}
	else
	{
		(HwGPIOD->GPEN |= Hw24);	(HwGPIOD->GPDAT &= ~Hw24);
		toggle_int = 1;
	}
#endif

	TEST_GPIO(0);

	spin_lock_irq(&(tcc_intr->lock));
	tcc_intr->count++;
	spin_unlock_irq(&(tcc_intr->lock));

	wake_up_interruptible(&(tcc_intr->wq));

	return IRQ_HANDLED;
}

/*
 *	Mem to Mem Scaler Interface.
 */
static void init_sc_interrupt(tcc_intr_data_t *tcc_intr)
{
	BITCLR(tcc_intr->irq_regs->MODE0, tcc_intr->irq_bit);	// edge-triggered
	BITCLR(tcc_intr->irq_regs->MODEA0, tcc_intr->irq_bit);	// single-edge
	BITSET(tcc_intr->irq_regs->POL0, tcc_intr->irq_bit);	// active-low
}

static irqreturn_t sc_handler(int irq, void *dev_id)
{
	tcc_intr_data_t *tcc_intr = dev_id;

	spin_lock_irq(&(tcc_intr->lock));
	tcc_intr->count++;
	spin_unlock_irq(&(tcc_intr->lock));

	wake_up_interruptible(&(tcc_intr->wq));

	return IRQ_HANDLED;
}

/*
 *	BroadCasting DXB Interface.
 */
static void init_bc_interrupt(tcc_intr_data_t *tcc_intr)
{	
	PGPIO pGPIO = (volatile PGPIO)tcc_p2v(HwGPIO_BASE);
	
#if defined(CONFIG_ARCH_TCC92XX)
	/* DXB1_IRQ - GPIO_A11 - ExINT3 */	
	BITCLR(pGPIO->GPAFN1, Hw16-Hw12);	// gpio
	BITCLR(pGPIO->GPAEN, Hw11);			// input mode
	//BITCSET(pGPIO->GPAPD0, Hw23, Hw22); // pull-up driving
	
	BITCSET(pGPIO->EINTSEL0, Hw30-Hw24, (11<<24));
#elif defined(CONFIG_ARCH_TCC93XX)
	/* DXB1_IRQ - GPIO_G12 - ExINT3 */	
	BITCLR(pGPIO->GPGFN1, Hw20-Hw16);	// gpio
	BITCLR(pGPIO->GPGEN, Hw12);			// input mode
	
	BITCSET(pGPIO->EINTSEL0, Hw30-Hw24, (65<<24));
#elif defined(CONFIG_ARCH_TCC88XX)
	/* DXB0_IRQ - GPIO_E5 - ExINT3 */	
	BITCLR(pGPIO->GPEFN0, Hw24-Hw20);	// gpio
	BITCLR(pGPIO->GPEEN, Hw5); 		// input mode

	BITCSET(pGPIO->EINTSEL0, Hw30-Hw24, (53<<24));
	BITSET(tcc_intr->irq_regs->EI37SEL, Hw3);
#endif
	/* Int trigger setting */
	BITCLR(tcc_intr->irq_regs->MODE0, tcc_intr->irq_bit);	// edge-triggered
	BITCLR(tcc_intr->irq_regs->MODEA0, tcc_intr->irq_bit);	// single-edge
	BITCLR(tcc_intr->irq_regs->POL0, tcc_intr->irq_bit);	// active-high
}

static irqreturn_t bc_handler(int irq, void *dev_id)
{
	tcc_intr_data_t *tcc_intr = dev_id;

	spin_lock_irq(&(tcc_intr->lock));
	tcc_intr->count++;
	spin_unlock_irq(&(tcc_intr->lock));

	wake_up_interruptible(&(tcc_intr->wq));

	return IRQ_HANDLED;
}

///////////////////////////////////////////////////////////////////////////
//
static long io_interrupt(tcc_intr_data_t *tcc_intr, long flag)
{
	/* NOTE: irq_regs->XXX0, XXX1 */
	long ret = 0;
	if (flag == 0) {
		BITSET(tcc_intr->irq_regs->CLR0, tcc_intr->irq_bit);			// clear intr
		BITCLR(tcc_intr->irq_regs->INTMSK0, tcc_intr->irq_bit);			// disable intr
	} else if (flag == 1) {
		BITSET(tcc_intr->irq_regs->CLR0, tcc_intr->irq_bit);
		BITSET(tcc_intr->irq_regs->INTMSK0, tcc_intr->irq_bit);			// enable intr
	} else if (flag == 2) {
		ret = (tcc_intr->irq_regs->INTMSK0 & tcc_intr->irq_bit)?1:0;	// get int-mask status
	} else {
		ret = -1;
	}
	return ret;
}

static unsigned int intr_poll(struct file *filp, poll_table *wait)
{
	tcc_intr_data_t *tcc_intr = (tcc_intr_data_t *)filp->private_data;

	if (tcc_intr == NULL)
	{
		return -EFAULT;
	}
	if (tcc_intr->count > 0) {
		spin_lock_irq(&(tcc_intr->lock));
		tcc_intr->count--;
		spin_unlock_irq(&(tcc_intr->lock));
		return (POLLIN | POLLRDNORM);
	}
	
	poll_wait(filp, &(tcc_intr->wq), wait);

	if (tcc_intr->count > 0) {
		spin_lock_irq(&(tcc_intr->lock));
		tcc_intr->count--;
		spin_unlock_irq(&(tcc_intr->lock));
		return (POLLIN | POLLRDNORM);
	} else {
		return 0;
	}
}

static int intr_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned long data;
	tcc_intr_data_t *tcc_intr = (tcc_intr_data_t *)filp->private_data;

	switch (cmd) {
		case IOCTL_INTR_SET:
			if (copy_from_user((void *)&data, (const void *)arg, sizeof(data)))
				return -EFAULT;
			if (data == 0 || data == 1) io_interrupt(tcc_intr, data);
			else return -EFAULT;
			break;
		case IOCTL_INTR_GET:
			data = io_interrupt(tcc_intr, 2);
			if (copy_to_user((void *)arg, (const void *)&data, sizeof(data)))
				return -EFAULT;
			break;
		default:
			printk("tcc_intr(%d): unrecognized ioctl (0x%x)\n", MINOR(inode->i_rdev), cmd);
			return -EINVAL;
			break;
	}

	return 0;
}

static int intr_test_ioctl(struct inode *inode, struct file *filp, 
							unsigned int cmd, unsigned long arg)
{
	unsigned long data;

	switch (cmd) {
		case IOCTL_INTR_TEST:
			if (copy_from_user((void *)&data, (const void *)arg, sizeof(data))) {
				return -EFAULT;
			} else {
				if (data == 1) {
					BITSET(VIPC_TIG0, (1 << INT_VCDC));
				} else if (data == 2) {
					BITSET(VIPC_TIG0, (1 << INT_SC0));
	#if defined(CONFIG_ARCH_TCC92XX) //임시									
				} else if (data == 3) {
					BITSET(VIPC_TIG0, (1 << INT_EI3));
	#elif defined(CONFIG_ARCH_TCC88XX)
				} else if (data == 3) {
					BITSET(VIPC_TIG0, (1 << INT_TSD));
	#endif
				} else {
					return -EFAULT;
				}
			}
			break;
		default:
			printk("tcc_intr(%d): unrecognized ioctl (0x%x)\n", MINOR(inode->i_rdev), cmd);
			return -EINVAL;
			break;
	}

	return 0;
}

static int intr_release(struct inode *inode, struct file *filp)
{
	tcc_intr_data_t *tcc_intr = (tcc_intr_data_t *)filp->private_data;

	free_irq(tcc_intr->irq, tcc_intr);
	kfree(tcc_intr);

	return 0;
}

///////////////////////////////////////////////////////////////////////////
//
static int intr_sc_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	tcc_intr_data_t *sc_data = NULL;

	sc_data = (tcc_intr_data_t *)kmalloc(sizeof(tcc_intr_data_t), GFP_KERNEL);
	if (sc_data == NULL) {
		ret = -ENOMEM;
		goto error;
	}
	memset(sc_data, 0, sizeof(tcc_intr_data_t));

	spin_lock_init(&(sc_data->lock));
	init_waitqueue_head(&(sc_data->wq));
	sc_data->irq = INT_SC0;
	sc_data->irq_bit = (1 << INT_SC0);
	sc_data->irq_regs = (volatile PPIC)tcc_p2v(HwPIC_BASE);

	init_sc_interrupt(sc_data);
	ret = request_irq(sc_data->irq, sc_handler, IRQF_DISABLED, TCC_INTR_DEV_M2M_SCALER, sc_data);
	if (ret) {
		//ZzaU :: exception process in case this abnormally stop!!for class concept in android!!
		printk("FAILED to aquire irq\n");
		//ret = -EFAULT;	goto error;
	}
	filp->private_data = (void *)sc_data;

	return 0;
error:
	if (sc_data) 
		kfree(sc_data);
	return ret;
}

static int intr_vc_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	tcc_intr_data_t *vc_data = NULL;

	vc_data = (tcc_intr_data_t *)kmalloc(sizeof(tcc_intr_data_t), GFP_KERNEL);
	if (vc_data == NULL) {
		ret = -ENOMEM;
		goto error;
	}
	memset(vc_data, 0, sizeof(tcc_intr_data_t));

	spin_lock_init(&(vc_data->lock));
	init_waitqueue_head(&(vc_data->wq));
	vc_data->irq = INT_VCDC;
	vc_data->irq_bit = (1 << INT_VCDC);
	vc_data->irq_regs = (volatile PPIC)tcc_p2v(HwPIC_BASE);

	init_vc_interrupt(vc_data);
	ret = request_irq(vc_data->irq, vc_handler, IRQF_DISABLED, TCC_INTR_DEV_VIDEO_CODEC, vc_data);
	if (ret) {
		//ZzaU :: exception process in case this abnormally stop!!for class concept in android!!
		printk("FAILED to aquire irq\n");
		//ret = -EFAULT;	goto error;
	}
	filp->private_data = (void *)vc_data;
	
	return 0;
error:
	if (vc_data) 
		kfree(vc_data);
	return ret;
}

static int intr_bc_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	tcc_intr_data_t *bc_data = NULL;

	bc_data = (tcc_intr_data_t *)kmalloc(sizeof(tcc_intr_data_t), GFP_KERNEL);
	if (bc_data == NULL) {
		ret = -ENOMEM;
		goto error;
	}
	memset(bc_data, 0, sizeof(tcc_intr_data_t));

	spin_lock_init(&(bc_data->lock));
	init_waitqueue_head(&(bc_data->wq));
#if defined(CONFIG_ARCH_TCC92XX) //임시	
	bc_data->irq = INT_EI3;
	bc_data->irq_bit = (1 << INT_EI3);
	bc_data->irq_regs = (volatile PPIC)tcc_p2v(HwPIC_BASE);

	init_bc_interrupt(bc_data);
	ret = request_irq(bc_data->irq, bc_handler, IRQF_DISABLED, TCC_INTR_DEV_BROADCAST, bc_data);
	if (ret) {
		//ZzaU :: exception process in case this abnormally stop!!for class concept in android!!
		printk("FAILED to aquire irq\n");
		//ret = -EFAULT;	goto error;
	}
#elif defined(CONFIG_ARCH_TCC88XX)
	bc_data->irq = INT_TSD;
	bc_data->irq_bit = (1 << INT_TSD);
	bc_data->irq_regs = (volatile PPIC)tcc_p2v(HwPIC_BASE);

	init_bc_interrupt(bc_data);
	ret = request_irq(bc_data->irq, bc_handler, IRQF_DISABLED, TCC_INTR_DEV_BROADCAST, bc_data);
	if (ret) {
		//ZzaU :: exception process in case this abnormally stop!!for class concept in android!!
		printk("FAILED to aquire irq\n");
		//ret = -EFAULT;	goto error;
	}
#endif

	filp->private_data = (void *)bc_data;
	
	return 0;
error:
	if (bc_data) 
		kfree(bc_data);
	return ret;
}

struct file_operations intr_vc_fops =
{
	.owner		= THIS_MODULE,
	.poll		= intr_poll,
	.ulocked_ioctl		= intr_ioctl,
	.open		= intr_vc_open,
	.release	= intr_release,
};

struct file_operations intr_sc_fops =
{
	.owner		= THIS_MODULE,
	.poll		= intr_poll,
	.ulocked_ioctl		= intr_ioctl,
	.open		= intr_sc_open,
	.release	= intr_release,
};

struct file_operations intr_bc_fops =
{
	.owner		= THIS_MODULE,
	.poll		= intr_poll,
	.ulocked_ioctl		= intr_ioctl,
	.open		= intr_bc_open,
	.release	= intr_release,
};

struct file_operations intr_test_fops =
{
	.owner		= THIS_MODULE,
	.ulocked_ioctl		= intr_test_ioctl,
};

static int tcc_intr_open(struct inode *inode, struct file *filp)
{
	switch (MINOR(inode->i_rdev)) {
		case 1: filp->f_op = &intr_vc_fops; break;
		case 2: filp->f_op = &intr_sc_fops; break;
		case 3: filp->f_op = &intr_bc_fops; break;
		case 4: filp->f_op = &intr_test_fops; break;
		default : return -ENXIO;
	}

	if (filp->f_op && filp->f_op->open)
		return filp->f_op->open(inode, filp);

	return 0;
}

struct file_operations tcc_intr_fops =
{
	.owner		= THIS_MODULE,
	.open		= tcc_intr_open,
};

static struct class *intr_class;

static int __init tcc_intr_init(void)
{
	int res;

	res = register_chrdev(TCC_INTR_DEV_MAJOR, TCC_INTR_DEV_NAME, &tcc_intr_fops);
	if (res < 0)
		return res;

	intr_class = class_create(THIS_MODULE, TCC_INTR_DEV_NAME);
	device_create(intr_class, NULL, MKDEV(TCC_INTR_DEV_MAJOR, 1), NULL, TCC_INTR_DEV_NAME);
	device_create(intr_class, NULL, MKDEV(TCC_INTR_DEV_MAJOR, 3), NULL, TCC_INTR_DEV_BROADCAST);
	printk("tcc_intr: init (ver %s)\n", TCC_INTR_VERSION);

	TEST_GPIO(3);

    printk("tcc_intr: init (ver %s)\n", TCC_INTR_VERSION);

    return 0;
}

static void __exit tcc_intr_exit(void)
{
	unregister_chrdev(TCC_INTR_DEV_MAJOR, TCC_INTR_DEV_NAME);
    printk("tcc_intr: exit\n");
}


module_init(tcc_intr_init);
module_exit(tcc_intr_exit);

MODULE_AUTHOR("Telechips Inc. SYS4-3 linux@telechips.com");
MODULE_DESCRIPTION("Telechips user level interrut driver");
MODULE_LICENSE("GPL");
