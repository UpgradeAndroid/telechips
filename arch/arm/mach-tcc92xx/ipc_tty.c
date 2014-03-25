/* lnux/arch/arm/mach-tcc92x/ipc-tty.c
 *
 * Author:	<linux@telechips.com>
 * Created: 12th july, 2009
 * Description:
 *
 * Copyright (C) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/tty.h>
#include <linux/serial_core.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/wakelock.h>

#include <asm/byteorder.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <mach/irqs.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <mach/bsp.h>
#include <mach/tca_ckc.h>


#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>

//#include <mach/msm_smd.h>
#include "IO_IPC.h"
#include "IO_EHI.h"

#define MAX_IPC_TTYS 32

static DEFINE_MUTEX(ipc_tty_lock);

struct ipc_tty_info {
//	ipc_channel_t *ch;
	struct tty_struct *tty;
	struct wake_lock wake_lock;
	int open_count;
};

static struct ipc_tty_info ipc_tty[MAX_IPC_TTYS];

//static void ipc_tty_notify(void *priv, unsigned event)
static void ipc_tty_notify(unsigned int ch, IPC_RX_LINE_TYPE* line, void* data, unsigned int len)
{
	unsigned char *ptr;
	int avail;
	struct ipc_tty_info info = ipc_tty[ch];
	struct tty_struct *tty = ipc_tty[ch].tty;

	if (!tty)
		return;

//	printk("\n open count= %d",info.open_count);

//	if (info.open_count==0)
//		return;
		
#if 0
	if (event != SMD_EVENT_DATA)
		return;

	for (;;) {
		if (test_bit(TTY_THROTTLED, &tty->flags)) break;
		avail = smd_read_avail(info->ch);
		if (avail == 0) break;

		avail = tty_prepare_flip_string(tty, &ptr, avail);

		if (smd_read(info->ch, ptr, avail) != avail) {
			/* shouldn't be possible since we're in interrupt
			** context here and nobody else could 'steal' our
			** characters.
			*/
			printk(KERN_ERR "OOPS - smd_tty_buffer mismatch?!");
		}

		wake_lock_timeout(&info->wake_lock, HZ / 2);
		tty_flip_buffer_push(tty);
	}
#endif

//telechips IPC
#if 1

		printk("%s ch: %d len: %d  buf address 0x%lx \n",__func__,ch,len,data);	

//		IPC_Manager.ch[ch].cbf(ch, &line, (void*)IPC_Manager.ch[ch].rx.data, IPC_Manager.ch[ch].rx.hdr.len);	
		
		tty_insert_flip_string(tty, (const unsigned char *)data, len);

//		wake_lock_timeout(&info.wake_lock, HZ / 2);  // necessary?

		tty_flip_buffer_push(tty);		

#endif

//	printk("read occured");

	/* XXX only when writable and necessary */
	tty_wakeup(tty);
}

static irqreturn_t ipc_tty_isr(int irq, void *data)
{
//	printk("ipc_tty_isr is called!!!!!!!!! ");
	IO_IPC_RxNotify();

	return IRQ_HANDLED;
}


static int ipc_tty_open(struct tty_struct *tty, struct file *f)
{
	int res = 0;
	int ch = tty->index;
	struct ipc_tty_info *info=&ipc_tty[ch];
	const char *name;
#if 0
	if (n == 0) {
		name = "EHI_DS";
	} else if (n == 27) {
		name = "SMD_GPSNMEA";
	} else {
		return -ENODEV;
	}

	info = smd_tty + n;

	mutex_lock(&smd_tty_lock);
	wake_lock_init(&info->wake_lock, WAKE_LOCK_SUSPEND, name);
	tty->driver_data = info;

	if (info->open_count++ == 0) {
		info->tty = tty;
		if (info->ch) {
			smd_kick(info->ch);
		} else {
			res = smd_open(name, &info->ch, info, smd_tty_notify);
		}
	}
	mutex_unlock(&smd_tty_lock);
#endif
	mutex_lock(&ipc_tty_lock);

	wake_lock_init(&ipc_tty[ch].wake_lock, WAKE_LOCK_SUSPEND, "IPC_TTY");	
	tty->driver_data = &info;
	info->open_count++;

	ipc_tty[ch].tty = tty;

	mutex_unlock(&ipc_tty_lock);

	printk("tty ch %d open\n",ch);
	return res;
}

static void ipc_tty_close(struct tty_struct *tty, struct file *f)
{
	struct ipc_tty_info *info = tty->driver_data;

	if (info == 0)
		return;
#if 0
	mutex_lock(&ipc_tty_lock);
	if (--info->open_count == 0) {
		info->tty = 0;
		tty->driver_data = 0;
		wake_lock_destroy(&info->wake_lock);
		if (info->ch) {
			smd_close(info->ch);
			info->ch = 0;
		}
	}
	mutex_unlock(&ipc_tty_lock);
#endif	
	mutex_lock(&ipc_tty_lock);

	if(--info->open_count ==0)
	{
		tty->driver_data = 0;
		wake_lock_destroy(&info->wake_lock);		
	}		

	mutex_unlock(&ipc_tty_lock);


}

static int ipc_tty_write(struct tty_struct *tty, const unsigned char *buf, int len)
{
	int ch = tty->index;
	IPC_TX_LINE_TYPE	line;
	struct ipc_tty_info *info = tty->driver_data;	
	int avail;
	int ret;

	printk("%s ch: %d len: %d  buf address 0x%lx \n",__func__,ch,len,buf);

#if 1 //just for low level test

	line.cts = 5+(ch*10);
	line.dsr = 6+(ch*10);
#endif

	ret=IO_IPC_Write(ch, &line, (void*) buf, len);	

	switch(ret)
	{			
		case	EHI_RET_FAIL:
		case	EHI_RET_FIFO_NOT_READY:
		case	EHI_RET_WFIFO_BUSY:			
		case	EHI_RET_SEMA_OBTAIN_FAIL:						
				printk("IO_IPC_Write occure error %d!!!\n",ret);
				ret= -1;   //temporary
				break;
		case	EHI_RET_SUCCESS:
				ret=len;			
				break;
		default :
				break;
	}	

//	printk("write len %d\n",ret); 	
	return ret;


#if 0
	/* if we're writing to a packet channel we will
	** never be able to write more data than there
	** is currently space for
	*/
	avail = ehi_write_avail(info->ch);
	if (len > avail)
		len = avail;

	return smd_write(info->ch, buf, len);
#endif



	return 0;
}

static int ipc_tty_write_room(struct tty_struct *tty)
{
	int ch = tty->index;
	
	return	IO_IPC_Write_avail(ch);
}

static int ipc_tty_chars_in_buffer(struct tty_struct *tty)
{
	int ch = tty->index;

	return	IO_IPC_Read_avail(ch);
}

static void ipc_tty_unthrottle(struct tty_struct *tty)
{
	int ch = tty->index;

#if 0
	struct smd_tty_info *info = tty->driver_data;
	smd_kick(info->ch);
#endif
}

static struct tty_operations ipc_tty_ops = {
	.open = ipc_tty_open,
	.close = ipc_tty_close,
	.write = ipc_tty_write,
	.write_room = ipc_tty_write_room,
	.chars_in_buffer = ipc_tty_chars_in_buffer,
	.unthrottle = ipc_tty_unthrottle,
};

static struct tty_driver *ipc_tty_driver;

static int __init ipc_tty_init(void)
{
	int i;
	int ret;

	printk("ipc_tty driver init. %d ch(s)",MAX_IPC_TTYS);
	
	ipc_tty_driver = alloc_tty_driver(MAX_IPC_TTYS);
	if (ipc_tty_driver == 0)
		return -ENOMEM;

	ipc_tty_driver->owner = THIS_MODULE;
	ipc_tty_driver->driver_name = "ipc_tty_driver";
	ipc_tty_driver->name = "ipc_tty";
	ipc_tty_driver->major = 0;
	ipc_tty_driver->minor_start = 0;
	ipc_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
	ipc_tty_driver->subtype = SERIAL_TYPE_NORMAL;
	ipc_tty_driver->init_termios = tty_std_termios;
	ipc_tty_driver->init_termios.c_iflag = 0;
	ipc_tty_driver->init_termios.c_oflag = 0;
	ipc_tty_driver->init_termios.c_cflag = B38400 | CS8 | CREAD;
	ipc_tty_driver->init_termios.c_lflag = 0;
	ipc_tty_driver->flags = TTY_DRIVER_RESET_TERMIOS |
		TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	tty_set_operations(ipc_tty_driver, &ipc_tty_ops);

	ret = tty_register_driver(ipc_tty_driver);
	if (ret) return ret;

#if 1
	//Interrupt
	HwPIC->IEN0 	|= HwINT0_EHI0; 
	HwPIC->SEL0 	|= HwINT0_EHI0;
	HwPIC->INTMSK0	|= HwINT0_EHI0;
#endif	

	ret = request_irq(INT_EHI0, ipc_tty_isr, IRQF_DISABLED, "ipc_tty_isr", NULL);
	if(ret)
		printk(KERN_ERR "failed to register IPC_ISR\n");	

#if 1     //IPC init
		IO_IPC_Initialize();  
		IO_IPC_Handshaking();
#endif

	for(i=0; i < MAX_IPC_TTYS ; i++)
	{
		tty_register_device(ipc_tty_driver, i, 0);
		IO_IPC_RegisterRxCbf(i, ipc_tty_notify);		//
	}


	return 0;
}

module_init(ipc_tty_init);
