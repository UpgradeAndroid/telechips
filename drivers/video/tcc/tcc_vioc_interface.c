/*
 * linux/drivers/video/tcc/tccfb_interface.c
 *
 * Based on:    Based on s3c2410fb.c, sa1100fb.c and others
 * Author:  <linux@telechips.com>
 * Created: June 10, 2008
 * Description: TCC LCD Controller Frame Buffer Driver
 *
 * Copyright (C) 2008-2009 Telechips 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/mach-types.h>

#include <mach/globals.h>
#include <mach/reg_physical.h>
#include <mach/tca_ckc.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <plat/pmap.h>
#include <mach/tcc_fb.h>
#include "tccfb.h"
#include <mach/tccfb_ioctrl.h>
#include <mach/TCC_LCD_Interface.h>
#include <mach/tcc_scaler_ctrl.h>

#include <mach/tcc_composite_ioctl.h>
#include <mach/tcc_component_ioctl.h>

#include <mach/tca_lcdc.h>
#include <mach/tca_fb_hdmi.h>
#include <mach/tca_fb_output.h>

#include <linux/tcc_pwm.h>

#include <mach/timex.h>

#include <mach/vioc_outcfg.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_disp.h>

/* Debugging stuff */
static int debug = 0;
#define dprintk(msg...)	if (debug) { printk( "VIOC_I: " msg); }

static int screen_debug = 0;
#define sprintk(msg...)	if (screen_debug) { printk( "VIOC scr: " msg); }

extern unsigned int tcc_output_fb_get_disable(void);

struct lcd_struct {
	spinlock_t lock;
	wait_queue_head_t waitq;
	char state;
};
static struct lcd_struct lcdc_struct;


char Fb_Lcdc_num = 0;

static volatile VIOC_DISP * pDISPBase;
static volatile VIOC_WMIX* pWIXBase;
static volatile VIOC_RDMA *pRDMABase;

static struct clk *lcdc0_clk;
static struct clk *lcdc1_clk;
static struct clk *ddi_cache;
#ifdef CONFIG_LCD_CPU_INTERFACE
static struct clk *lcd_si;
#endif//CONFIG_LCD_CPU_INTERFACE

void tca92xxfb_clock_init(void)
{
	lcdc0_clk = clk_get(0, "lcdc0");
	lcdc1_clk = clk_get(0, "lcdc1");

	ddi_cache = clk_get(0, "ddi_cache");

	#ifdef CONFIG_LCD_CPU_INTERFACE
	lcd_si = clk_get(0, "lcdsi");
	#endif//

	BUG_ON(lcdc0_clk == NULL);
	BUG_ON(lcdc1_clk == NULL);
	BUG_ON(ddi_cache == NULL);
	#ifdef CONFIG_LCD_CPU_INTERFACE
	BUG_ON(lcd_si == NULL);
	#endif//

	
}

void tca92xxfb_clock_delete(void)
{
	clk_put(lcdc0_clk);
	clk_put(lcdc1_clk);
	clk_put(ddi_cache);

	#ifdef CONFIG_LCD_CPU_INTERFACE
	clk_put(lcd_si);
	#endif//
}

#if defined(CONFIG_CPU_FREQ)
extern struct tcc_freq_table_t stFBClockLimitTable;
#endif//
static int  tca92xxfb_clock_set(int cmd)
{
	int ret = 0;
    switch (cmd) {
	    case PWR_CMD_OFF:
			#if defined(CONFIG_CPU_FREQ)
			tcc_cpufreq_set_limit_table(&stFBClockLimitTable, TCC_FREQ_LIMIT_FB, 0);
			#endif			
			clk_disable(ddi_cache);
			clk_disable(lcdc0_clk);
			clk_disable(lcdc1_clk);

			#ifdef CONFIG_LCD_CPU_INTERFACE
			clk_disable(lcd_si);
			#endif//

			break;
			
	    case PWR_CMD_ON:
			clk_enable(lcdc0_clk);
			clk_enable(lcdc1_clk);
			clk_enable(ddi_cache);

			#ifdef CONFIG_LCD_CPU_INTERFACE
			clk_enable(lcd_si);
			#endif//

			#if defined(CONFIG_CPU_FREQ)
			tcc_cpufreq_set_limit_table(&stFBClockLimitTable, TCC_FREQ_LIMIT_FB, 1);
			#endif			

			break;

	    default:
			ret = -EINVAL;
	        break;
    }

    return ret;
	
}


/* tccfb_pan_display
 *
 * pandisplay (set) the controller from the given framebuffer
 * information
*/
int tca_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	return 0;
}
EXPORT_SYMBOL(tca_fb_pan_display);

void tca_fb_vsync_activate(struct fb_var_screeninfo *var, struct tccfb_info *fbi)
{
	unsigned int base_addr = 0;
	unsigned int bpp = fbi->fb->var.bits_per_pixel / 8;	/* bytes per pixel */


 	base_addr = fbi->map_dma +fbi->fb->var.xres * bpp * var->yoffset;
	if(var->yoffset > var->yres)	{
		base_addr = PAGE_ALIGN(base_addr);
	}
	
	#ifdef CONFIG_FB_TCC_USE_VSYNC_INTERRUPT
	{
		int ret;
		
		pDISPBase->uLSTATUS.nREG = 0xFFFFFFFF;
		tca_lcdc_interrupt_onoff(TRUE, Fb_Lcdc_num);
		lcdc_struct.state = 0;
		ret = wait_event_interruptible_timeout(lcdc_struct.waitq, lcdc_struct.state == 1, msecs_to_jiffies(50));
		if(!ret)	{
		 	printk("  [%d]: tcc_setup_interrupt timed_out!! \n", ret);
		}
		#ifdef CONFIG_USE_EXT_INTERRUPT
		else
		{
			if(!( pDISPBase->uLSTATUS.bREG.RU))
			{
				tca_lcdc_interrupt_onoff(TRUE, Fb_Lcdc_num);
				lcdc_struct.state = 0;
				ret = wait_event_interruptible_timeout(lcdc_struct.waitq, lcdc_struct.state == 1, msecs_to_jiffies(50));
				if(!ret)	{
				 	printk("  [%d]: tcc_setup_interrupt timed_out!! \n", ret);
				}
			}
		}
			#endif//CONFIG_USE_EXT_INTERRUPT
	}
	#endif //CONFIG_FB_TCC_USE_VSYNC_INTERRUPT
}

void tcc_disable_internal_display(void)
{
	if(pRDMABase){
		/* Disable : RDMA1 */		
		if(pRDMABase->uCTRL.bREG.AEN){
			VIOC_RDMA_SetImageDisable(pRDMABase);
		}
	}	
}

/* tccfb_activate_var
 * activate (set) the controller from the given framebuffer
 * information
*/
void tca_fb_activate_var(struct tccfb_info *fbi,  struct fb_var_screeninfo *var)
{
	unsigned int imgch, fmt , tmp_value, base_addr;
	unsigned int regl, lcd_width, lcd_height, img_width, img_height;

	#define IMG_AOPT       	2
	
	unsigned int alpha_type = 0, alpha_blending_en = 0;
	unsigned int chromaR, chromaG, chromaB, chroma_en;
	unsigned ch;

	imgch = fbi->fb->node; //FB_driver
	if(fbi->fb->var.bits_per_pixel == 32)
	{
		chroma_en = 0;
		alpha_type = 1;
		alpha_blending_en = 1;
		fmt = TCC_LCDC_IMG_FMT_RGB888;
	}
	else if(fbi->fb->var.bits_per_pixel == 16)
	{
		chroma_en = 1;
		alpha_type = 0;
		alpha_blending_en = 0;
		fmt = TCC_LCDC_IMG_FMT_RGB565; 
	}
	else	{
		printk("%s:fb%d Not Supported BPP!\n", __FUNCTION__, fbi->fb->node);
		return;
	}

	chromaR = chromaG = chromaB = 0;

	sprintk("%s: fb%d Supported BPP!\n", __FUNCTION__, fbi->fb->node);

 	base_addr = fbi->map_dma + fbi->fb->var.xres * var->yoffset * (fbi->fb->var.bits_per_pixel/8);
	if(fbi->fb->var.yoffset > fbi->fb->var.yres)	{
		base_addr = PAGE_ALIGN(base_addr);
	}

	sprintk("%s: fb%d Baddr:0x%x Naddr:0x%x!\n", __FUNCTION__, fbi->fb->node, base_addr, pRDMABase->nBASE0);

	regl = pDISPBase->uLSIZE.nREG;

	lcd_width = (regl & 0xFFFF);
	lcd_height = ((regl>>16) & 0xFFFF);
	img_width = fbi->fb->var.xres;
	img_height = fbi->fb->var.yres;

	if(img_width > lcd_width)	
		img_width = lcd_width;
	
	if(img_height > lcd_height)	
		img_height = lcd_height;

	VIOC_WMIX_SetSize(pWIXBase, img_width, img_height);
	VIOC_WMIX_SetOverlayPriority(pWIXBase, 24);		
	VIOC_WMIX_SetBGColor(pWIXBase, 0x10, 0x80, 0x80, 0xff);

	VIOC_DISP_SetSize(pDISPBase, img_height, img_width);
	pDISPBase->uCTRL.bREG.PXDW = fmt; //set format
	
	/* write new registers */
	switch(imgch)
	{
		case 0:
			/* If LCD CH1 is enabled, Do not update anythings on Ch2 */
			if(tcc_output_fb_get_disable()){
				VIOC_RDMA_SetImageDisable(pRDMABase);
				break;
			}else{
				VIOC_RDMA_SetImageEnable(pRDMABase);
			}
			
			// default framebuffer 
			VIOC_RDMA_SetImageFormat(pRDMABase, fmt );	//fmt
			VIOC_RDMA_SetImageOffset (pRDMABase, fmt, img_width  );	//offset	
			VIOC_RDMA_SetImageSize (pRDMABase,  img_height , img_width );	//size	
			VIOC_RDMA_SetImageBase (pRDMABase , base_addr, 0 , 0 );
			VIOC_WMIX_SetPosition(pWIXBase, imgch, 0, 0);

			//overlay setting
			VIOC_WMIX_SetChromaKey(pWIXBase, imgch, alpha_blending_en, chromaR, chromaG, chromaB, 0xF8, 0xFC, 0xF8);			
	
			VIOC_RDMA_SetImageFormat(HwVIOC_RDMA01, fmt );	//fmt
			VIOC_RDMA_SetImageOffset (HwVIOC_RDMA01, fmt, img_width  );	//offset	

			VIOC_WMIX_SetUpdate(pWIXBase);	

			VIOC_DISP_TurnOn(pDISPBase);
			VIOC_RDMA_SetImageUpdate(pRDMABase);

			#ifdef CONFIG_FB_TCC_USE_VSYNC_INTERRUPT
			tca_fb_vsync_activate(var, fbi);
			#else
			msleep(16);
			#endif//
			
			break;

		case 1:
			
			VIOC_RDMA_SetImageFormat(HwVIOC_RDMA02, fmt );	//fmt
			VIOC_RDMA_SetImageOffset (HwVIOC_RDMA02, fmt, img_width  );	//offset	
			VIOC_RDMA_SetImageSize (HwVIOC_RDMA02,  img_height , img_width );	//size	
			VIOC_RDMA_SetImageBase (HwVIOC_RDMA02 , base_addr, 0 , 0 );

			VIOC_WMIX_SetUpdate(pWIXBase);	

			VIOC_RDMA_SetImageUpdate(HwVIOC_RDMA02);
			break;

		case 2:
			
			VIOC_RDMA_SetImageFormat(HwVIOC_RDMA03, fmt );	//fmt
			VIOC_RDMA_SetImageOffset (HwVIOC_RDMA03, fmt, img_width  );	//offset	
			VIOC_RDMA_SetImageSize (HwVIOC_RDMA03,  img_height , img_width );	//size	
			VIOC_RDMA_SetImageBase (HwVIOC_RDMA03 , base_addr, 0 , 0 );

			VIOC_WMIX_SetUpdate(pWIXBase);	

			VIOC_RDMA_SetImageUpdate(HwVIOC_RDMA03);
			break;
	}

	return;
	
}
EXPORT_SYMBOL(tca_fb_activate_var);



#ifdef CONFIG_HAS_EARLYSUSPEND

void tca_fb_earlier_suspend(struct early_suspend *h)
{
	printk("%s: START Fb_Lcdc_num:%d \n", __FUNCTION__, Fb_Lcdc_num);
}
EXPORT_SYMBOL(tca_fb_earlier_suspend);

static volatile lcdc_gamma_params LCDC_Gamma_BackUp;
static volatile VIOC_RDMA * pRDMA_BackUp;

extern unsigned int tcc_LCDC_set_gamma(lcdc_gamma_params *gamma);
extern unsigned int tcc_LCDC_get_gamma(lcdc_gamma_params *gamma);



void tca_fb_early_suspend(struct early_suspend *h)
{
	pRDMA_BackUp = pRDMABase;

	//gamma setting backup
	LCDC_Gamma_BackUp.lcdc_num = Fb_Lcdc_num;
	tcc_LCDC_get_gamma(&LCDC_Gamma_BackUp);
	#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
	tca_lcdc_interrupt_onoff(FALSE, Fb_Lcdc_num);
	#endif
	
	tcc_LCDC_Wait_signal_disable(0);

	tcc_LCDC_Wait_signal_disable(1);

	tca92xxfb_clock_set(PWR_CMD_OFF);

}
EXPORT_SYMBOL(tca_fb_early_suspend);


void tca_fb_late_resume(struct early_suspend *h)
{

	tca92xxfb_clock_set(PWR_CMD_ON);

	tcc_onthefly_init();
	tcc_ddi_cache_setting();

	pRDMABase = pRDMA_BackUp;
	 
	//gamma setting restore
	tcc_LCDC_set_gamma(&LCDC_Gamma_BackUp);

	VIOC_RDMA_SetImageEnable(pRDMABase);
	
	printk(" end \n ");
}
EXPORT_SYMBOL(tca_fb_late_resume);


void tca_fb_later_resume(struct early_suspend *h)
{
	printk("%s: START Fb_Lcdc_num:%d \n", __FUNCTION__, Fb_Lcdc_num);

	// lcd module on
//  	tca_bkl_powerup();
}
EXPORT_SYMBOL(tca_fb_later_resume);
#endif



/* suspend and resume support for the lcd controller */
int tca_fb_suspend(struct platform_device *dev, pm_message_t state)
{
	printk("%s:  \n", __FUNCTION__);
	return 0;
}
EXPORT_SYMBOL(tca_fb_suspend);


int tca_fb_resume(struct platform_device *dev)
{
	printk("%s:  \n", __FUNCTION__);
	return 0;
}
EXPORT_SYMBOL(tca_fb_resume);


static irqreturn_t tcc_lcd_handler(int irq, void *dev_id)
{
	unsigned int VOICstatus = pDISPBase->uLSTATUS.nREG;
	
 	sprintk("%s lcdc_struct.state:%d STATUS:0x%x 0x%x \n",__func__, 	lcdc_struct.state, VOICstatus, pDISPBase->uLSTATUS.nREG);
	
//	if(LCDCstatus & HwLSTATUS_VSF)
	{
		#ifndef CONFIG_USE_EXT_INTERRUPT		
			pDISPBase->uLSTATUS.nREG = 0xFFFFFFFF;
		#endif//
		
		#ifndef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
		tca_lcdc_interrupt_onoff(FALSE, Fb_Lcdc_num);
		#endif

		if(lcdc_struct.state == 0)		{
			lcdc_struct.state = 1;
			wake_up_interruptible(&lcdc_struct.waitq);
		}
	}

	return IRQ_HANDLED;
}


int tcc_lcd_interrupt_reg(char SetClear)
{
	int ret = 0;
	dprintk("%s SetClear:%d lcdc_num:%d \n",__func__, SetClear, Fb_Lcdc_num);

	if(SetClear)
	{
		#ifdef CONFIG_USE_EXT_INTERRUPT

			//EXT INT 6 Setting
			BITSET(HwPIC->POL0, HwINT0_EI6);	
			BITCLR(HwPIC->MODE0, HwINT0_EI6);
			BITCLR(HwPIC->MODEA0, HwINT0_EI6);
			BITSET(HwPIC->SEL0, HwINT0_EI6);	
			BITCLR(HwPIC->INTMSK0, HwINT0_EI6);	

			if (request_irq(IRQ_EI6, tcc_lcd_handler,	IRQF_SHARED,
							"TCC_LCD",	tcc_lcd_handler))	{
				printk(KERN_ERR "tcc_lcd_interrupt_reg request_irq ERROR!\n");
			}
			
		#else		
			tca_lcdc_interrupt_onoff(0, Fb_Lcdc_num);

			ret	= request_irq(Hw20, tcc_lcd_handler,	IRQF_SHARED,
					"TCC_LCD",	tcc_lcd_handler);				
			
		#endif//CONFIG_USE_EXT_INTERRUPT
	}
	else
	{
		free_irq(Hw20, tcc_lcd_handler);
	}
	
	return ret;
}
EXPORT_SYMBOL(tcc_lcd_interrupt_reg);


int tca_fb_init(void)
{
	struct lcd_panel *lcd_info;
	pmap_t pmap_fb_video;
	unsigned int ch;	

	printk(KERN_INFO " tcc892X %s (built %s %s)\n", __func__, __DATE__, __TIME__);

	pmap_get_info("fb_video", &pmap_fb_video);

#ifdef TCC_LCDC1_USE

		pWIXBase = (VIOC_WMIX*)HwVIOC_WMIX1;
		pDISPBase = (VIOC_DISP *)HwVIOC_DISP1;
		pRDMABase = (VIOC_RDMA *)HwVIOC_RDMA05;
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_HDMI, VIOC_OUTCFG_DISP1);
		Fb_Lcdc_num = 1;
#else
		pWIXBase = (VIOC_WMIX*)HwVIOC_WMIX0;
		pDISPBase = (VIOC_DISP *)HwVIOC_DISP0;
		pRDMABase = (VIOC_RDMA *)HwVIOC_RDMA01;
		VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_MRGB, VIOC_OUTCFG_DISP0);
		Fb_Lcdc_num = 0;

		

#endif//	TCC_LCDC1_USE

	tca92xxfb_clock_init();
	tca92xxfb_clock_set(PWR_CMD_ON);

	tcc_onthefly_init();
	tcc_ddi_cache_setting();

	TCC_OUTPUT_LCDC_Init();

    init_waitqueue_head(&lcdc_struct.waitq);
	lcdc_struct.state = 1;


	printk(" %s LCDC:%d  end \n", __func__, Fb_Lcdc_num);

	return 0;
}
EXPORT_SYMBOL(tca_fb_init);


void tca_fb_cleanup(void)
{
	dprintk(" %s LCDC:%d \n", __func__, Fb_Lcdc_num);
	tca92xxfb_clock_delete();
}
EXPORT_SYMBOL(tca_fb_cleanup);


