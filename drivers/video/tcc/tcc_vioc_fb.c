/*
 * linux/drivers/video/tcc/tccfb.c
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
#include <linux/err.h>
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
#include <asm/mach-types.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>

#include <mach/bsp.h>
#include <mach/tca_ckc.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include "tccfb.h"
#include <plat/pmap.h>
#include <mach/tcc_fb.h>
#include <mach/tcc_scaler_ctrl.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tcc_composite_ioctl.h>
#include <mach/tcc_component_ioctl.h>
#include <mach/TCC_LCD_Interface.h>

#include <mach/tca_fb_hdmi.h>
#include <mach/tca_fb_output.h>
#include <mach/tca_lcdc.h>
#include <linux/console.h>

#if defined(CONFIG_LCD_LCDC0_USE)
#define EX_OUT_LCDC		1
#define LCD_LCDC_NUM		0
#else
#define EX_OUT_LCDC		0
#define LCD_LCDC_NUM		1
#endif
#define TCC_FB_DOUBLE

/* Debugging stuff */
static int debug = 0;
#define dprintk(msg...)	if (debug) { printk( "tccfb: " msg); }

static int screen_debug = 0;
#define sprintk(msg...)	if (screen_debug) { printk( "tcc92fb scr: " msg); }

extern int tca_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info);
extern void tca_fb_activate_var(struct tccfb_info *fbi,  struct fb_var_screeninfo *var);
#ifdef CONFIG_HAS_EARLYSUSPEND
extern void tca_fb_early_suspend(struct early_suspend *h);
extern void tca_fb_earlier_suspend(struct early_suspend *h);
extern void tca_fb_late_resume(struct early_suspend *h);
extern void tca_fb_later_resume(struct early_suspend *h);
#endif
extern int tca_fb_suspend(struct platform_device *dev, pm_message_t state);
extern int tca_fb_resume(struct platform_device *dev);
extern int tca_fb_init(void);
extern void tca_fb_exit(void);
extern int tcc_lcd_interrupt_reg(char SetClear);


static pmap_t pmap_fb_video;
#define FB_VIDEO_MEM_BASE	pmap_fb_video.base
#define FB_VIDEO_MEM_SIZE	pmap_fb_video.size


#define CONFIG_FB_TCC_DEVS_MAX	3	// do not change!
#define CONFIG_FB_TCC_DEVS		1

#if (CONFIG_FB_TCC_DEVS > CONFIG_FB_TCC_DEVS_MAX)
	#undef CONFIG_FB_TCC_DEVS
	#define CONFIG_FB_TCC_DEVS	CONFIG_FB_TCC_DEVS_MAX
#endif


#define SCREEN_DEPTH_MAX	32	// 32 or 16
//								 : 32 - 32bpp(alpah+rgb888)
//								 : 16 - 16bpp(rgb565)


const unsigned int default_scn_depth[CONFIG_FB_TCC_DEVS_MAX] =
{
/* fb0, Layer0: bottom */  (16), // 32 or 16
/* fb1, Layer1: middle */  (16), //  "
/* fb2, Layer2: top    */  (16)  //  "
};


#define LCD_OUT_LCDC 	1
static struct lcd_panel *lcd_panel;


TCC_OUTPUT_TYPE	Output_SelectMode =  TCC_OUTPUT_NONE;
static unsigned int Output_BaseAddr;


static char  HDMI_pause = 0;
static char HDMI_video_mode = 0;

static unsigned int HDMI_video_width = 0;
static unsigned int HDMI_video_height = 0;
static unsigned int HDMI_video_hz = 0;


char fb_power_state;

// for frame buffer clear
static u_char *fb_mem_vaddr[CONFIG_FB_TCC_DEVS]= {0,};
static u_int   fb_mem_size [CONFIG_FB_TCC_DEVS]= {0,};


void tccfb_hdmi_starter(char hdmi_lcdc_num, struct lcdc_timimg_parms_t *lcdc_timing)
{
	TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_HDMI, hdmi_lcdc_num, 1);

	TCC_HDMI_LCDC_Timing(EX_OUT_LCDC, lcdc_timing);
	Output_SelectMode = TCC_OUTPUT_HDMI;
}

static int tccfb_check_var(struct fb_var_screeninfo *var,
			       struct fb_info *info)
{
	/* validate bpp */
	if (var->bits_per_pixel > 32)
		var->bits_per_pixel = 32;
	else if (var->bits_per_pixel < 16)
		var->bits_per_pixel = 16;

	/* set r/g/b positions */
	if (var->bits_per_pixel == 16) {
		var->red.offset 	= 11;
		var->green.offset	= 5;
		var->blue.offset	= 0;
		var->red.length 	= 5;
		var->green.length	= 6;
		var->blue.length	= 5;
		var->transp.length	= 0;
	} else if (var->bits_per_pixel == 32) {
		var->red.offset 	= 16;
		var->green.offset	= 8;
		var->blue.offset	= 0;
		var->transp.offset	= 24;
		var->red.length 	= 8;
		var->green.length	= 8;
		var->blue.length	= 8;
		var->transp.length	= 8;
	} else {
		var->red.length 	= var->bits_per_pixel;
		var->red.offset 	= 0;
		var->green.length	= var->bits_per_pixel;
		var->green.offset	= 0;
		var->blue.length	= var->bits_per_pixel;
		var->blue.offset	= 0;
		var->transp.length	= 0;
	}
	if (var->yres_virtual < var->yoffset + var->yres)
		var->yres_virtual = var->yoffset + var->yres;

	return 0;
}




/* tccfb_pan_display
 *
 * pandisplay (set) the controller from the given framebuffer
 * information
*/
static int tccfb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	char tcc_output_ret = 0;
	unsigned int base_addr = 0;
	struct tccfb_info *fbi =(struct tccfb_info *) info->par;

	if(!fb_power_state)
		return 0;

 	base_addr = Output_BaseAddr = fbi->map_dma +fbi->fb->var.xres * (fbi->fb->var.bits_per_pixel/8) * var->yoffset;
	if(var->yoffset > var->yres)	{
		base_addr = Output_BaseAddr = PAGE_ALIGN(base_addr);
	}
	sprintk("%s addr:0x%x Yoffset:%d \n", __func__, Output_BaseAddr, var->yoffset);

	tca_fb_pan_display(var, info);

	if(Output_SelectMode)
	{
		tcc_output_ret = TCC_OUTPUT_FB_Update(fbi->fb->var.xres, fbi->fb->var.yres, fbi->fb->var.bits_per_pixel, base_addr, Output_SelectMode);
	}


#if !defined(CONFIG_TCC_HDMI_UI_DISPLAY_OFF)

	if(tcc_output_ret )
		TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);

#endif//CONFIG_TCC_HDMI_UI_DISPLAY_OFF


	return 0;
}
/* tccfb_activate_var
 *
 * activate (set) the controller from the given framebuffer
 * information
*/
static void tccfb_activate_var(struct tccfb_info *fbi,  struct fb_var_screeninfo *var)
{
	unsigned int imgch = 0;

	sprintk("%s node:0x%x TCC_DEVS:%d \n", __func__, fbi->fb->node, CONFIG_FB_TCC_DEVS);

	if((0 <= fbi->fb->node) && (fbi->fb->node < CONFIG_FB_TCC_DEVS))
		imgch = fbi->fb->node;
	else
		return;

	tca_fb_activate_var(fbi, var);
}


/*
 *      tccfb_set_par - Optional function. Alters the hardware state.
 *      @info: frame buffer structure that represents a single frame buffer
 *
 */
static int tccfb_set_par(struct fb_info *info)
{
	struct tccfb_info *fbi = info->par;
	struct fb_var_screeninfo *var = &info->var;

	sprintk("- tccfb_set_par pwr:%d  output:%d \n",fb_power_state, Output_SelectMode);

	if (var->bits_per_pixel == 16)
		fbi->fb->fix.visual = FB_VISUAL_TRUECOLOR;
	else if (var->bits_per_pixel == 32)
		fbi->fb->fix.visual = FB_VISUAL_TRUECOLOR;
	else
		fbi->fb->fix.visual = FB_VISUAL_PSEUDOCOLOR;

	fbi->fb->fix.line_length = (var->xres*var->bits_per_pixel)/8;

	#ifndef CONFIG_TCC_OUTPUT_STARTER
	/* activate this new configuration */
   	if(fb_power_state && Output_SelectMode!=TCC_OUTPUT_COMPONENT) //&& Output_SelectMode!=TCC_OUTPUT_COMPOSITE
		tccfb_activate_var(fbi, var);
	#endif

	return 0;
}

static int tccfb_ioctl(struct fb_info *info, unsigned int cmd,unsigned long arg)
{
	struct tccfb_info *fb_info = info->par;
	unsigned int imgch=0;
	int screen_width, screen_height;

	screen_width = lcd_panel->xres;
	screen_height = lcd_panel->yres;

	if((0 <= info->node) && (info->node < CONFIG_FB_TCC_DEVS))	{
		imgch = info->node;
	}
	else	{
		dprintk("ioctl: Error - fix.id[%d]\n", info->node);
		return 0;
	}


	switch(cmd)
	{
		case TCC_LCDC_HDMI_START:
			TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_HDMI, EX_OUT_LCDC, 1);
			break;

		case TCC_LCDC_HDMI_TIMING:
			{
				struct lcdc_timimg_parms_t lcdc_timing;
				dprintk(" TCC_LCDC_HDMI_TIMING: \n");

				if (copy_from_user((void*)&lcdc_timing, (const void*)arg, sizeof(struct lcdc_timimg_parms_t)))
					return -EFAULT;

				TCC_HDMI_LCDC_Timing(EX_OUT_LCDC, &lcdc_timing);
				Output_SelectMode = TCC_OUTPUT_HDMI;

				#if !defined(CONFIG_TCC_HDMI_UI_DISPLAY_OFF)
				TCC_OUTPUT_FB_Update(fb_info->fb->var.xres, fb_info->fb->var.yres, fb_info->fb->var.bits_per_pixel, Output_BaseAddr, Output_SelectMode);
				TCC_OUTPUT_FB_UpdateSync(Output_SelectMode);
				#endif /*CONFIG_TCC_HDMI_UI_DISPLAY_OFF*/
				TCC_HDMI_LCDC_OutputEnable(EX_OUT_LCDC, 1);

				TCC_OUTPUT_FB_MouseIconSelect(TCC_OUTPUT_HDMI);
				TCC_OUTPUT_FB_MouseShow(0, TCC_OUTPUT_HDMI);
		}
			break;

		case TCC_LCDC_HDMI_DISPLAY:
			{
				struct tcc_lcdc_image_update ImageInfo;
				if (copy_from_user((void *)&ImageInfo, (const void *)arg, sizeof(struct tcc_lcdc_image_update))){
					return -EFAULT;
				}

				dprintk("%s : TCC_LCDC_HDMI_DISPLAY\n", __func__);

				if(Output_SelectMode == TCC_OUTPUT_HDMI)	{
  					TCC_HDMI_DISPLAY_UPDATE(EX_OUT_LCDC, (struct tcc_lcdc_image_update *)&ImageInfo);
				}
			}
			break;

		case TCC_LCDC_HDMI_END:
			#if !defined(CONFIG_TCC_OUTPUT_DUAL_UI)
			if(Output_SelectMode == TCC_OUTPUT_HDMI) 
			#endif
			{
				#if defined(CONFIG_ARCH_TCC93XX)
					TCC_OUTPUT_FB_BackupVideoImg(Output_SelectMode);
				#endif

				Output_SelectMode = TCC_OUTPUT_NONE;
				TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_HDMI, EX_OUT_LCDC, 0);

			}

			TCC_OUTPUT_FB_MouseIconSelect(TCC_OUTPUT_HDMI);
			TCC_OUTPUT_FB_MouseShow(0, TCC_OUTPUT_HDMI);
			break;

		case TCC_LCDC_HDMI_CHECK:
			{
				unsigned int ret_mode = 0;
				if((!fb_power_state) || HDMI_pause || ((screen_width < screen_height)&& (!HDMI_video_mode)))
				{
					ret_mode = 1;
					dprintk("\n %d %d : %d %d  \n ", fb_power_state, HDMI_pause, screen_width, screen_height);
				}

				put_user(ret_mode, (unsigned int __user*)arg);
			}
			break;

		case TCC_LCDC_HDMI_MODE_SET:
 			{
				TCC_HDMI_M uiHdmi;

				if(get_user(uiHdmi, (int __user *) arg))
					return -EFAULT;

				dprintk("%s: TCC_LCDC_HDMI_MODE_SET [%d] video_M:%d Output_SelectMode:%d   \n", __func__ , uiHdmi , HDMI_video_mode, Output_SelectMode);

				switch(uiHdmi)
				{
					case TCC_HDMI_SUSEPNED:
						HDMI_pause = 1;
						break;
					case TCC_HDMI_RESUME:
						HDMI_pause = 0;
						break;
					case TCC_HDMI_VIDEO_START:
						HDMI_video_mode = 1;
						break;
					case TCC_HDMI_VIDEO_END:
						HDMI_video_mode = 0;
						break;
					default:
						break;
				}
 			}
			break;
			
		case TCC_LCDC_HDMI_GET_SIZE:
			{
				tcc_display_size HdmiSize;
				HdmiSize.width = HDMI_video_width;
				HdmiSize.height = HDMI_video_height;
  				HdmiSize.frame_hz= HDMI_video_hz;

				dprintk("%s: TCC_LCDC_HDMI_GET_SIZE -  HDMI_video_width:%d HDMI_video_height:%d   \n", __func__ , HDMI_video_width, HDMI_video_height);
				if (copy_to_user((tcc_display_size *)arg, &HdmiSize, sizeof(HdmiSize)))		{
					return -EFAULT;
				}
			}
			break;


		case TCC_LCDC_HDMI_SET_SIZE:
			{
				tcc_display_size HdmiSize;
				if (copy_from_user((void *)&HdmiSize, (const void *)arg, sizeof(tcc_display_size)))
					return -EFAULT;

				HDMI_video_width = HdmiSize.width;
				HDMI_video_height = HdmiSize.height;
				HDMI_video_hz = HdmiSize.frame_hz;

				dprintk("%s: TCC_LCDC_HDMI_SET_SIZE -  HDMI_video_width:%d HDMI_video_height:%d   \n", __func__ , HDMI_video_width, HDMI_video_height);
			}
			break;

		case TCC_LCDC_MOUSE_SHOW:
			{
				unsigned int enable;

				if(copy_from_user((void *)&enable, (const void *)arg, sizeof(unsigned int)))
					return -EFAULT;
				TCC_OUTPUT_FB_MouseShow(enable, Output_SelectMode);
			}
			break;
		case TCC_LCDC_MOUSE_MOVE:
			{
				tcc_mouse mouse;
				if (copy_from_user((void *)&mouse, (const void *)arg, sizeof(tcc_mouse)))
					return -EFAULT;
				TCC_OUTPUT_FB_MouseMove(fb_info->fb->var.xres, fb_info->fb->var.yres, &mouse, Output_SelectMode);
			}
			break;

		default:
			dprintk("ioctl: Unknown [%d/0x%X]", cmd, cmd);
			break;
	}


	return 0;
}

static void schedule_palette_update(struct tccfb_info *fbi,
				    unsigned int regno, unsigned int val)
{
	unsigned long flags;

	local_irq_save(flags);

	local_irq_restore(flags);
}

/* from pxafb.c */
static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int tccfb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	struct tccfb_info *fbi = info->par;
	unsigned int val;

	/* dprintk("setcol: regno=%d, rgb=%d,%d,%d\n", regno, red, green, blue); */

	switch (fbi->fb->fix.visual) {
		case FB_VISUAL_TRUECOLOR:
			/* true-colour, use pseuo-palette */

			if (regno < 16) {
				u32 *pal = fbi->fb->pseudo_palette;

				val  = chan_to_field(red,   &fbi->fb->var.red);
				val |= chan_to_field(green, &fbi->fb->var.green);
				val |= chan_to_field(blue,  &fbi->fb->var.blue);

				pal[regno] = val;
			}
			break;

		case FB_VISUAL_PSEUDOCOLOR:
			if (regno < 256) {
				/* currently assume RGB 5-6-5 mode */

				val  = ((red   >>  0) & 0xf800);
				val |= ((green >>  5) & 0x07e0);
				val |= ((blue  >> 11) & 0x001f);

				//writel(val, S3C2410_TFTPAL(regno));
				schedule_palette_update(fbi, regno, val);
			}
			break;

		default:
			return 1;   /* unknown type */
	}

	return 0;
}


/**
 *      tccfb_blank
 *	@blank_mode: the blank mode we want.
 *	@info: frame buffer structure that represents a single frame buffer
 *
 *	Blank the screen if blank_mode != 0, else unblank. Return 0 if
 *	blanking succeeded, != 0 if un-/blanking failed due to e.g. a
 *	video mode which doesn't support it. Implements VESA suspend
 *	and powerdown modes on hardware that supports disabling hsync/vsync:
 *	blank_mode == 2: suspend vsync
 *	blank_mode == 3: suspend hsync
 *	blank_mode == 4: powerdown
 *
 *	Returns negative errno on error, or zero on success.
 *
 */
static int tccfb_blank(int blank_mode, struct fb_info *info)
{
	dprintk("blank(mode=%d, info=%p)\n", blank_mode, info);
	return 0;
}


#ifdef CONFIG_HAS_EARLYSUSPEND
static void tcc_fb_earlier_suspend(struct early_suspend *h)
{
	printk("%s:  \n", __FUNCTION__);

#if 0
	if(Output_SelectMode == TCC_OUTPUT_HDMI)
	{
		Output_SelectMode = TCC_OUTPUT_NONE;
		TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_HDMI, EX_OUT_LCDC, 0);
		TCC_FB_LCDC_NumSet(1, 0);
	}
#endif
	tca_fb_earlier_suspend(h);
}

static void tcc_fb_early_suspend(struct early_suspend *h)
{
	printk("%s:  \n", __FUNCTION__);
	console_lock();
	fb_power_state = 0;

	tca_fb_early_suspend(h);

	if (lcd_panel->set_power)
		lcd_panel->set_power(lcd_panel, 0, LCD_LCDC_NUM);

	console_unlock();
	printk("%s: finish \n", __FUNCTION__);

}

static void tcc_fb_late_resume(struct early_suspend *h)
{
	printk("%s:  \n", __FUNCTION__);
	
	console_lock();

	tca_fb_late_resume(h);

	if (lcd_panel->set_power)
		lcd_panel->set_power(lcd_panel, 1, LCD_LCDC_NUM);
	
	fb_power_state = 1;
	console_unlock();
	printk("%s: finish \n", __FUNCTION__);

}

static void tcc_fb_later_resume(struct early_suspend *h)
{
	printk("%s:  \n", __FUNCTION__);
	tca_fb_later_resume(h);
}
#endif

#ifdef CONFIG_PM
/* suspend and resume support for the lcd controller */
static int tccfb_suspend(struct platform_device *dev, pm_message_t state)
{
	tca_fb_suspend(dev, state);
	return 0;
}

static int tccfb_resume(struct platform_device *dev)
{
	tca_fb_resume(dev);
	return 0;
}

static void tccfb_shutdown(struct platform_device *dev)
{
	pm_message_t state = {0};
	tccfb_suspend(dev, state);
}

#else
#define tccfb_suspend	NULL
#define tccfb_resume	NULL
#define tccfb_shutdown	NULL
#endif



static struct fb_ops tccfb_ops = {
	.owner			= THIS_MODULE,
	.fb_check_var	= tccfb_check_var,
	.fb_set_par		= tccfb_set_par,
	.fb_blank		= tccfb_blank,
	.fb_setcolreg	= tccfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
	.fb_ioctl		= tccfb_ioctl,
	.fb_pan_display = tccfb_pan_display,
};


/*
 * tccfb_map_video_memory():
 *	Allocates the DRAM memory for the frame buffer.  This buffer is
 *	remapped into a non-cached, non-buffered, memory region to
 *	allow palette and pixel writes to occur without flushing the
 *	cache.  Once this area is remapped, all virtual memory
 *	access to the video memory should occur at the new region.
 */

static int __init tccfb_map_video_memory(struct tccfb_info *fbi, int plane)
{

	fbi->map_size = PAGE_ALIGN(fbi->fb->fix.smem_len + PAGE_SIZE);


	if(FB_VIDEO_MEM_SIZE == 0)
	{
		fbi->map_cpu = dma_alloc_writecombine(fbi->dev, fbi->map_size, &fbi->map_dma, GFP_KERNEL);
		printk("map_video_memory (fbi=%p) kernel memory, dma:0x%x map_size:%08x\n", fbi,fbi->map_dma, fbi->map_size);
	}
	else
	{
		fbi->map_dma =  FB_VIDEO_MEM_BASE;
		fbi->map_cpu = ioremap_nocache(fbi->map_dma, fbi->map_size);
		printk("map_video_memory (fbi=%p) used map memory,map dma:0x%x size:%08x\n", fbi, fbi->map_dma ,fbi->map_size);
	}

	fbi->map_size = fbi->fb->fix.smem_len;

	if (fbi->map_cpu) {
		/* prevent initial garbage on screen */
		dprintk("map_video_memory: clear %p:%08x\n", fbi->map_cpu, fbi->map_size);

		memset(fbi->map_cpu, 0x00, fbi->map_size);

		fbi->screen_dma		= fbi->map_dma;
		fbi->fb->screen_base	= fbi->map_cpu;
		fbi->fb->fix.smem_start  = fbi->screen_dma;

		// Set the LCD frame buffer start address
		switch (plane)
		{
			case 2:	// IMG2
				fb_mem_vaddr[plane] = fbi->map_cpu;
				fb_mem_size [plane] = fbi->map_size;
				break;
			case 1:	// IMG1
				fb_mem_vaddr[plane] = fbi->map_cpu;
				fb_mem_size [plane] = fbi->map_size;
				break;
			case 0:	// IMG0
				fb_mem_vaddr[plane] = fbi->map_cpu;
				fb_mem_size [plane] = fbi->map_size;
				break;
		}
		dprintk("map_video_memory: dma=%08x cpu=%p size=%08x\n",
			fbi->map_dma, fbi->map_cpu, fbi->fb->fix.smem_len);
	}

	return fbi->map_cpu ? 0 : -ENOMEM;
}

static inline void tccfb_unmap_video_memory(struct tccfb_info *fbi)
{
	dma_free_writecombine(fbi->dev,fbi->map_size,fbi->map_cpu, fbi->map_dma);

	fb_mem_vaddr[fbi->imgch] = (u_char*)NULL;
	fb_mem_size [fbi->imgch] = (u_int)NULL;
}

static char tccfb_driver_name[]="tccfb";
static int __init tccfb_probe(struct platform_device *pdev)
{
	struct tccfb_info *info;
	struct fb_info *fbinfo;
	int ret;
	int plane = 0;
	unsigned int screen_width, screen_height;

	if (!lcd_panel) {
		pr_err("tccfb: no LCD panel data\n");
		return -EINVAL;
	}
	pr_info("LCD panel is %s %s %d x %d\n", lcd_panel->manufacturer, lcd_panel->name,
		lcd_panel->xres, lcd_panel->yres);

        screen_width      = lcd_panel->xres;
        screen_height     = lcd_panel->yres;

#if defined(CONFIG_TCC_HDMI_UI_SIZE_1280_720)
        if(tcc_display_data.resolution == 1)
        {
            screen_width      = 720;
            screen_height     = 576;
        }
        else if(tcc_display_data.resolution == 2)
        {
            screen_width 	  = 800;
            screen_height 	  = 480;
        }
#endif

	printk("%s, screen_width=%d, screen_height=%d\n", __func__, screen_width, screen_height);

	pmap_get_info("fb_video", &pmap_fb_video);

#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
	pmap_get_info("exclusive_viqe", &pmap_eui_viqe);
#endif
	tcc_lcd_interrupt_reg(TRUE);

	for (plane = 0; plane < CONFIG_FB_TCC_DEVS; plane++)
	{
		fbinfo = framebuffer_alloc(sizeof(struct tccfb_info), &pdev->dev);
		info = fbinfo->par;
		info->fb = fbinfo;
		//platform_set_drvdata(pdev, fbinfo);

		strcpy(fbinfo->fix.id, tccfb_driver_name);

		fbinfo->fix.type		= FB_TYPE_PACKED_PIXELS;
		fbinfo->fix.type_aux		= 0;
		fbinfo->fix.xpanstep		= 0;
		fbinfo->fix.ypanstep		= 1;
		fbinfo->fix.ywrapstep		= 0;
		fbinfo->fix.accel		= FB_ACCEL_NONE;

		fbinfo->var.nonstd		= 0;
		fbinfo->var.activate		= FB_ACTIVATE_NOW;

		fbinfo->var.accel_flags		= 0;
		fbinfo->var.vmode		= FB_VMODE_NONINTERLACED;

		fbinfo->fbops			= &tccfb_ops;
		fbinfo->flags			= FBINFO_FLAG_DEFAULT;

		fbinfo->var.xres		= screen_width;
		fbinfo->var.xres_virtual	= fbinfo->var.xres;
		fbinfo->var.yres		= screen_height;

#ifdef TCC_FB_DOUBLE
		fbinfo->var.yres_virtual	= fbinfo->var.yres * 2;
#else
		fbinfo->var.yres_virtual	= fbinfo->var.yres;
#endif//
		fbinfo->var.bits_per_pixel	= default_scn_depth[plane];
		fbinfo->fix.line_length 	= fbinfo->var.xres * fbinfo->var.bits_per_pixel/8;

		tccfb_check_var(&fbinfo->var, fbinfo);

		// the memory size that LCD should occupy
		fbinfo->fix.smem_len = fbinfo->var.xres * fbinfo->var.yres_virtual * SCREEN_DEPTH_MAX / 8;

		info->imgch = plane;

		/* Initialize video memory */
		ret = tccfb_map_video_memory(info, plane);
		if (ret) {
			dprintk( KERN_ERR "Failed to allocate video RAM: %d\n", ret);
			ret = -ENOMEM;
		}

		ret = register_framebuffer(fbinfo);
		if (ret < 0) {
			dprintk(KERN_ERR "Failed to register framebuffer device: %d\n", ret);
			goto free_video_memory;
		}

//		fbinfo->var.reserved[0] = 0x54445055;

		tccfb_set_par(fbinfo);

		if (plane == 0)	// top layer
			if (fb_prepare_logo(fbinfo,	FB_ROTATE_UR)) {
			/* Start display and show logo on boot */
			fb_set_cmap(&fbinfo->cmap, fbinfo);

			dprintk("fb_show_logo\n");
			fb_show_logo(fbinfo, FB_ROTATE_UR);
        	}
		printk(KERN_INFO "fb%d: %s frame buffer device\n",	fbinfo->node, fbinfo->fix.id);
	}



#ifdef CONFIG_HAS_EARLYSUSPEND
	info->early_suspend.suspend = tcc_fb_early_suspend;
	info->early_suspend.resume 	= tcc_fb_late_resume;
	info->early_suspend.level 	= EARLY_SUSPEND_LEVEL_DISABLE_FB - 2;
	register_early_suspend(&info->early_suspend);

	info->earlier_suspend.suspend = tcc_fb_earlier_suspend;
	info->earlier_suspend.resume 	= tcc_fb_later_resume;
	info->earlier_suspend.level 	= EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	register_early_suspend(&info->earlier_suspend);
#endif

	return 0;

free_video_memory:
	tccfb_unmap_video_memory(info);
	dprintk("TCC92xx fb init failed.\n");
	return ret;
}


/*
 *  Cleanup
 */
static int tccfb_remove(struct platform_device *pdev)
{
	struct fb_info	   *fbinfo = platform_get_drvdata(pdev);
	struct tccfb_info *info = fbinfo->par;

	dprintk(" %s  \n", __func__);

	tcc_lcd_interrupt_reg(FALSE);

	tccfb_unmap_video_memory(info);

	//release_mem_region((unsigned long)S3C24XX_VA_LCD, S3C24XX_SZ_LCD);
	unregister_framebuffer(fbinfo);

	return 0;
}

int tccfb_register_panel(struct lcd_panel *panel)
{
	dprintk(" %s  name:%s \n", __func__, panel->name);

	lcd_panel = panel;
	return 1;
}
EXPORT_SYMBOL(tccfb_register_panel);

struct lcd_panel *tccfb_get_panel(void)
{
	return lcd_panel;
}
EXPORT_SYMBOL(tccfb_get_panel);

static struct platform_driver tccfb_driver = {
	.probe		= tccfb_probe,
	.remove		= tccfb_remove,
	.suspend	= tccfb_suspend,
	.shutdown	= tccfb_shutdown,
	.resume		= tccfb_resume,
	.driver		= {
		.name	= "tccxxx-lcd",
		.owner	= THIS_MODULE,
	},
};

//int __devinit tccfb_init(void)
static int __init tccfb_init(void)
{
    printk(KERN_INFO " %s (built %s %s)\n", __func__, __DATE__, __TIME__);

	fb_power_state = 1;

	tca_fb_init();

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
	spin_lock_init(&vsync_lock) ;
	spin_lock_init(&vsync_lockDisp ) ;
#endif

	printk(" %s  \n", __func__);

	return platform_driver_register(&tccfb_driver);
}

static void __exit tccfb_exit(void)
{
	dprintk(" %s \n", __func__);

	tca_fb_exit();
	platform_driver_unregister(&tccfb_driver);
}


module_init(tccfb_init);
module_exit(tccfb_exit);

MODULE_AUTHOR("linux <linux@telechips.com>");
MODULE_DESCRIPTION("Telechips TCC Framebuffer driver");
MODULE_LICENSE("GPL");
