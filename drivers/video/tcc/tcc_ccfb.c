/*
 * drivers/video/tcc_ccfb.c
 *
 * Copyright (C) 2011 Telechips, Inc. 
 *
 * This file used for DTV subtitle display for TCC93xx/TCC88xx. Not for TCC92xx. 
 * 
 * 
 * This package is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. 
 * 
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED 
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. 
 *
 */
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/videodev2.h>
#include <linux/miscdevice.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#include <linux/poll.h>
#include <mach/bsp.h>
#include <mach/tcc_ccfb_ioctl.h>
#include <mach/tcc_fb.h>
#include <mach/tccfb_ioctrl.h>


/****************************************************************************
DEFINITION
****************************************************************************/
#define dprintk(msg...)	//printk(msg)

#define DEVICE_NAME		"ccfb"
#define DEV_MINOR		204
#define MAX_LCDC_NUM	2


/****************************************************************************
DEFINITION OF TYPE
****************************************************************************/
typedef enum
{
	CCFB_STATE_CLOSED,
	CCFB_STATE_OPENED,
	CCFB_STATE_RUNNING,
	CCFB_STATE_PAUSE,

	CCFB_STATE_MAX
}ccfb_state_t;

typedef struct
{
	ccfb_state_t			cur_state;
	int32_t				act_lcdc_idx;
	LCDC_CHANNEL		*pCurLcdc;
	LCDC_CHANNEL		*pUiLcdc;
	struct clk 			*pLcdcClk[MAX_LCDC_NUM];	/* TCCxx machine has two LCD controller */
}ccfb_dev_config_t;


/****************************************************************************
DEFINITION OF EXTERNAL VARIABLES
****************************************************************************/


/****************************************************************************
DEFINITION OF STATIC VARIABLES
****************************************************************************/
static ccfb_dev_config_t	g_dev_cfg;
static DEFINE_MUTEX(g_ccfb_mutex);

/****************************************************************************
DEFINITION OF EXTERNAL FUNCTIONS
****************************************************************************/
extern int range_is_allowed(unsigned long pfn, unsigned long size);


/****************************************************************************
DEFINITION OF LOCAL FUNCTIONS
****************************************************************************/
static ccfb_dev_config_t* get_ccfb_dev(void)
{
	return &g_dev_cfg;
}

static int tccxxx_ccfb_mmap(struct file *file, struct vm_area_struct *vma)
{
	dprintk("==> %s\n", __func__);
	
	if(range_is_allowed(vma->vm_pgoff, vma->vm_end - vma->vm_start) < 0){
		printk(KERN_ERR  "ccfb: this address is not allowed \n");
		return -EAGAIN;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	if(remap_pfn_range(vma,vma->vm_start, vma->vm_pgoff, 
						vma->vm_end - vma->vm_start, 
						vma->vm_page_prot))
	{
		return -EAGAIN;
	}

	vma->vm_ops	= NULL;
	vma->vm_flags 	|= VM_IO;
	vma->vm_flags 	|= VM_RESERVED;
	
	return 0;
}

static int tccxxx_ccfb_act_clock(ccfb_dev_config_t *dev, int lcdc_num)
{
	char *pDevName[2]={"lcdc0", "lcdc1"};

	dprintk("==> %s\n", __func__);

	if((dev == NULL) && (lcdc_num < 0) && (lcdc_num >= 2))
	{
		printk(KERN_ERR "Invalid parameters for lcdc(%d, %p)\n", lcdc_num, dev);
		return -EINVAL;
	}

	dev->pLcdcClk[lcdc_num] = clk_get(0, pDevName[lcdc_num]);
	if (IS_ERR(dev->pLcdcClk[lcdc_num])){
		printk(KERN_ERR "%s clock get fail.\n", pDevName[lcdc_num]);
		return -EIO;
	}
	clk_enable(dev->pLcdcClk[lcdc_num]);	

	dev->act_lcdc_idx = lcdc_num;

	if(lcdc_num == 0){		
		dev->pCurLcdc = (LCDC_CHANNEL *)tcc_p2v(HwLCDC0_CH_BASE(1));	
		dev->pUiLcdc = (LCDC_CHANNEL *)tcc_p2v(HwLCDC0_CH_BASE(2));
	}
	else{
		dev->pCurLcdc = (LCDC_CHANNEL *)tcc_p2v(HwLCDC1_CH_BASE(1));
		dev->pUiLcdc = (LCDC_CHANNEL *)tcc_p2v(HwLCDC1_CH_BASE(2));
	}
	
	return 0;
}

#if 0
static int tccxxx_ccfb_deact_clock(ccfb_dev_config_t *dev)
{
	int i;

	dprintk("==> %s\n", __func__);

	for(i = 0 ; i<MAX_LCDC_NUM ; i++)
	{
		if(dev->pLcdcClk[i] != NULL){
			clk_disable(dev->pLcdcClk[i]);
			dev->pLcdcClk[i] = NULL;
		}
	}
	
	return 0;
}
#endif    /* End of 0 */

static int tccxxx_ccfb_lcdc_enable(ccfb_dev_config_t *dev)
{
	dprintk("==> %s\n", __func__);
	
	if(dev->pCurLcdc){
		BITCSET (dev->pCurLcdc->LIC, 0x1<<28, (1) << 28);	// IEN set
	#if !defined(CONFIG_ARCH_TCC92XX)	
		BITCSET (dev->pCurLcdc->LIC, HwLCT_RU, HwLCT_RU);
	#endif
	}
	
	return 0;
}

static int tccxxx_ccfb_lcdc_disable(ccfb_dev_config_t *dev)
{
	dprintk("==> %s\n", __func__);
	
	if(dev->pCurLcdc){
		BITCSET (dev->pCurLcdc->LIC, 0x1<<28, (0) << 28);	// IEN unset
	#if !defined(CONFIG_ARCH_TCC92XX)
		BITCSET (dev->pCurLcdc->LIC, HwLCT_RU, HwLCT_RU);
	#endif
	}
	
	return 0;
}

static int tccxxx_ccfb_get_config(ccfb_dev_config_t *dev, void *arg)
{
	ccfb_config_t cfg;
	struct lcd_panel *panel = tccfb_get_panel();

	cfg.res.disp_w = panel->xres;
	cfg.res.disp_h = panel->yres;

	//printk("==> %s - %dx%d (%d)\n", __func__, panel->xres, panel->yres, tcc_display_data.resolution);
	
	if(copy_to_user((ccfb_config_t *)arg, &cfg, sizeof(ccfb_config_t)))
		return -EFAULT;
	
	return 0;
}

static int tccxxx_ccfb_set_config(ccfb_dev_config_t *dev, ccfb_config_t *arg)
{
	int ret = -ENODEV;
	ccfb_config_t cfg;

	dprintk("==> %s\n", __func__);
	
	if(copy_from_user((void*)&cfg, (void *)arg, sizeof(ccfb_config_t)))
		return -EFAULT;	

	ret = tccxxx_ccfb_act_clock(dev, cfg.curLcdc);
	if(ret == 0)
	{
		// position (full screen update)
		BITCSET (dev->pCurLcdc->LIP, 0xffff<< 16, (cfg.res.disp_y)  << 16);
		BITCSET (dev->pCurLcdc->LIP, 0xffff<<  0, (cfg.res.disp_x)  <<  0);

		// size
		BITCSET (dev->pCurLcdc->LIS, 0xffff<< 16, (cfg.res.disp_h) << 16);
		BITCSET (dev->pCurLcdc->LIS, 0xffff<<  0, (cfg.res.disp_w) <<  0);

		/*
		  * TCC92xx 
		  * 0:no scale, 1:x2, 2:x3, 3:x4, 4:x8 - Only Upscale supported 
		  *
		  * TCC93/88xx
		  * 0:no scale, 1:/2, 2:/3, 3:/4, 4-6:rsvd, 7:/8
		  * 8:rsvd, 9:x2, 10:x3, 11:x4, 12-14:rsvd, 15:x8
		  */
		BITCSET (dev->pCurLcdc->LISR, 0xff, (((cfg.res.disp_m)<<4)|cfg.res.disp_m));
	
		// ARGB 32bit format
		BITCSET (dev->pCurLcdc->LIC, 0x1f<< 0, (0xc) <<  0);
		BITCSET (dev->pCurLcdc->LIO, 0x0000FFFF, (cfg.res.mem_w * 4));
		
		BITCSET (dev->pCurLcdc->LIC, 0x1<<  8, (0)  <<  8);

		// Chroma-keying disable
		BITCSET (dev->pCurLcdc->LIC, 0x1<< 29, 0 << 29);

		// Alpha enable
		BITCSET (dev->pCurLcdc->LIC, 0x1<<24, (1)  << 24); 	// ASEL set
		BITCSET (dev->pCurLcdc->LIC, 0x1<<30, (1)  << 30); 	// AEN set

		// Ch enable
		BITCSET (dev->pCurLcdc->LIC, 0x1<<28, (0) << 28);		// IEN set
	#if !defined(CONFIG_ARCH_TCC92XX)
		BITCSET (dev->pCurLcdc->LIC, HwLCT_RU, HwLCT_RU);
	#endif	
		
	}	
	
	return ret;
}

static int tccxxx_ccfb_disp_update(ccfb_dev_config_t *dev, unsigned int* arg)
{
	unsigned int cur_addr;

	dprintk("==> %s\n", __func__);
	
	if(copy_from_user((void*)&cur_addr, (void *)arg, sizeof(unsigned int)))
	{
		printk(KERN_ERR "error\n");
		return -EFAULT;
	}
	
	dprintk("updated address : 0x%08x\n", cur_addr);
	if(dev->pUiLcdc->LIC & Hw28){
		printk("==>>> [%s] WARNING : UI is enabled.\n", __func__);
		BITCSET(dev->pUiLcdc->LIC, 0x0<<28, 0x0<<28);
	#if !defined(CONFIG_ARCH_TCC92XX)
		BITCSET (dev->pUiLcdc->LIC, HwLCT_RU, HwLCT_RU);
	#endif
	}

	BITCSET (dev->pCurLcdc->LIC, 0x1<<28, (1) << 28);	// IEN set
	BITCSET (dev->pCurLcdc->LIBA0, 0xFFFFFFFF,  cur_addr);
#if !defined(CONFIG_ARCH_TCC92XX)
	BITCSET (dev->pCurLcdc->LIC, HwLCT_RU, HwLCT_RU);
#endif	
	return 0;
}

static int tccxxx_ccfb_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = -EPERM;
	ccfb_dev_config_t	*dev = get_ccfb_dev();

	dprintk("==> %s\n", __func__);
	
	switch(cmd)
	{
		case CCFB_GET_CONFIG:
			mutex_lock(&g_ccfb_mutex);
			ret = tccxxx_ccfb_get_config(dev, (ccfb_config_t *)arg);
			mutex_unlock(&g_ccfb_mutex);
			break;
			
		case CCFB_SET_CONFIG:
			if(dev->cur_state != CCFB_STATE_CLOSED){
				mutex_lock(&g_ccfb_mutex);
				ret = tccxxx_ccfb_set_config(dev, (ccfb_config_t *)arg);
				mutex_unlock(&g_ccfb_mutex);
				if(ret == 0) dev->cur_state = CCFB_STATE_PAUSE;
			}		
			break;
			
		case CCFB_DISP_UPDATE:
			if(dev->cur_state == CCFB_STATE_RUNNING){
				mutex_lock(&g_ccfb_mutex);
				ret = tccxxx_ccfb_disp_update(dev, (unsigned int *)arg);
				mutex_unlock(&g_ccfb_mutex);
			}
			break;
		
		case CCFB_DISP_ENABLE:
			if(dev->cur_state == CCFB_STATE_PAUSE){
				mutex_lock(&g_ccfb_mutex);
				/* enable setting is moved to update routine */
				//ret = tccxxx_ccfb_lcdc_enable(dev);
				dev->cur_state = CCFB_STATE_RUNNING;
				mutex_unlock(&g_ccfb_mutex);
			}
			break;
			
		case CCFB_DISP_DISABLE:
			if(dev->cur_state == CCFB_STATE_RUNNING){
				mutex_lock(&g_ccfb_mutex);				
				ret = tccxxx_ccfb_lcdc_disable(dev);
				dev->cur_state = CCFB_STATE_PAUSE;
				mutex_unlock(&g_ccfb_mutex);
			}
			break;

		default:
			printk("%s - Unsupported IOCTL!!!(0x%X)\n", __func__, cmd);
			ret = -EINVAL;
			break;
	}

	return ret;
}

static int tccxxx_ccfb_release(struct inode *inode, struct file *file)
{
	ccfb_dev_config_t *dev = get_ccfb_dev();

	dprintk("==> %s\n", __func__);

	if(dev->cur_state == CCFB_STATE_CLOSED)
	{
		printk("WARNING : ccfb already closed.\n");
		return 0;
	}

	if((dev->cur_state == CCFB_STATE_RUNNING)||(dev->cur_state == CCFB_STATE_PAUSE))
	{
		tccxxx_ccfb_lcdc_disable(dev);
		//tccxxx_ccfb_deact_clock(dev);
	}

	if(dev->pUiLcdc){
		if((dev->pUiLcdc->LIC & Hw28)==0){
			BITSET(dev->pUiLcdc->LIC, 0x1<<28);
		#if !defined(CONFIG_ARCH_TCC92XX)
			BITCSET (dev->pUiLcdc->LIC, HwLCT_RU, HwLCT_RU);
		#endif
		}
	}

	dev->cur_state = CCFB_STATE_CLOSED;
	dev->act_lcdc_idx = -1;
	dev->pCurLcdc = NULL;
	dev->pUiLcdc = NULL;
	dev->pLcdcClk[0] = NULL;
	dev->pLcdcClk[1] = NULL;
	
	return 0;
}

static int tccxxx_ccfb_open(struct inode *inode, struct file *file)
{
	ccfb_dev_config_t *dev = get_ccfb_dev();

	dprintk("==> %s\n", __func__);
	
	if(dev->cur_state != CCFB_STATE_CLOSED)
	{
		printk("WARNING : ccfb already opened.\n");
		return 0;
	}

	dev->cur_state = CCFB_STATE_OPENED;
	dev->act_lcdc_idx = -1;
	dev->pCurLcdc = NULL;
	dev->pUiLcdc = NULL;
	dev->pLcdcClk[0] = NULL;
	dev->pLcdcClk[1] = NULL;
	
	return 0;
}

static struct file_operations tcc_ccfb_fops = 
{
	.owner          = THIS_MODULE,
	.unlocked_ioctl = tccxxx_ccfb_ioctl,
	.mmap           = tccxxx_ccfb_mmap,
	.open           = tccxxx_ccfb_open,
	.release        = tccxxx_ccfb_release,
};

static struct miscdevice ccfb_misc_device =
{
	DEV_MINOR,
	DEVICE_NAME,
	&tcc_ccfb_fops,
};

static int __init tcc_ccfb_probe(struct platform_device *pdev)
{
	dprintk("==> %s\n", __func__);
	
	if (misc_register(&ccfb_misc_device))
	{
		dprintk(KERN_WARNING "ccfb: Couldn't register device %d.\n", DEV_MINOR);
		return -EINVAL;
	}

	return 0;
}

static int tcc_ccfb_remove(struct platform_device *pdev)
{
	dprintk("==> %s\n", __func__);
	
	misc_deregister(&ccfb_misc_device);
	return 0;
}

#ifdef CONFIG_PM
static volatile LCDC_CHANNEL active_lcdc_backup;
static int tcc_ccfb_suspend(struct platform_device *pdev, pm_message_t state)
{
#if 0
	ccfb_dev_config_t	*dev = get_ccfb_dev();

	dprintk("==> %s\n", __func__);

	dev->cur_state = CCFB_STATE_PAUSE;
	msleep_interruptible(50);
	
	active_lcdc_backup = *(dev->pCurLcdc);
	tccxxx_ccfb_deact_clock(dev);
#endif    /* End of 0 */

	return 0;
}

static int tcc_ccfb_resume(struct platform_device *pdev)
{
#if 0
	ccfb_dev_config_t	*dev = get_ccfb_dev();

	dprintk("==> %s\n", __func__);

	tccxxx_ccfb_act_clock(dev, dev->act_lcdc_idx);
	*(dev->pCurLcdc) = active_lcdc_backup;
	dev->cur_state = CCFB_STATE_RUNNING;
#endif    /* End of 0 */
		
	return 0;
}
#else
#define tcc_ccfb_suspend NULL
#define tcc_ccfb_resume NULL
#endif

static struct platform_device tcc_ccfb_device = {
	.name	= "tcc_ccfb",
	.dev	= {
		.release 	= NULL,
	},
	.id	= 0,
};

static struct platform_driver tcc_ccfb_driver = {
	.driver         = {
	     .name   = "tcc_ccfb",
	     .owner  = THIS_MODULE,
	},
	.probe		= tcc_ccfb_probe,
	.remove         = tcc_ccfb_remove,
	.suspend        = tcc_ccfb_suspend,
	.resume         = tcc_ccfb_resume,
};

static int __init tccxxx_ccfb_init(void)
{
	dprintk("==> %s\n", __func__);
	
	platform_device_register(&tcc_ccfb_device);
	platform_driver_register(&tcc_ccfb_driver);

	return 0;
}
module_init(tccxxx_ccfb_init);

static void __exit tccxxx_ccfb_exit(void)
{
	dprintk("==> %s\n", __func__);
	
	platform_driver_unregister(&tcc_ccfb_driver);
	platform_device_unregister(&tcc_ccfb_device);
	
	return;
}
module_exit(tccxxx_ccfb_exit);


MODULE_AUTHOR("Telechips Inc.");
MODULE_DESCRIPTION("Telechips ccfb driver");
MODULE_LICENSE("GPL");
