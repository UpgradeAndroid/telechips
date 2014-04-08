/*
 * Copyright 2014 Upgrade Android
 *     Ithamar R. Adema <ithamar@upgrade-android.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * TODO:
 * - use devm calls in probe (currently si_clk ref is leaked)
 */

//#define DEBUG

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>

#define ANDROID_NUMBER_OF_BUFFERS	2

/* Controller registers */
#define LCTRL	0x00

#define CHANNEL_SIZE	0x38

/* Channel registers */
#define LIC	0x00
#define LIP	0x04
#define LIS	0x08
#define LIBA0	0x0c
#define LICA	0x10
#define LIBA1	0x14
#define LIBA2	0x18
#define LIO	0x1c
#define LISR	0x20
#define LIA	0x24
#define LIKR	0x28
#define LIKG	0x2c
#define LIKB	0x30
#define LIEN	0x34

struct tcc_vsync_info {
	wait_queue_head_t wait;
	unsigned int count;
};

struct tccddi_fb {
	size_t map_size;
	void *map_cpu;
	dma_addr_t map_dma;

	void __iomem *channel_base;
	void __iomem *regs;
	struct device *dev;

	struct clk *clk;
	struct clk *si_clk;
	struct tcc_vsync_info vsync_info;

	u32 cmap[16];
};

static inline void __iomem *channel_base(struct tccddi_fb *fb, int ch)
{
	BUG_ON(ch < 0 || ch > 2);
	return fb->channel_base + (ch * CHANNEL_SIZE);
}

static int __init tcc_map_video_memory(struct fb_info *fbinfo)
{
	struct tccddi_fb *fb = fbinfo->par;

	pr_debug("%s 1\n", __func__);

        fb->map_size = PAGE_ALIGN(fbinfo->fix.smem_len + PAGE_SIZE);
	pr_debug("%s 2 (%u)\n", __func__, fb->map_size);
	fb->map_cpu = dma_alloc_writecombine(fb->dev, fb->map_size + PAGE_SIZE, &fb->map_dma, GFP_KERNEL);
	pr_debug("%s 3\n", __func__);
        if (fb->map_cpu) {
	pr_debug("%s 4 %p\n", __func__, fb->map_cpu);
                //memset(fb->map_cpu, 0xa0, fb->map_size);

                fbinfo->screen_base = fb->map_cpu;
                fbinfo->fix.smem_start = fb->map_dma;
        }

	pr_debug("%s 5\n", __func__);
	writel_relaxed(fb->map_dma, channel_base(fb,2) + LIBA0);

        return fb->map_cpu ? 0 : -ENOMEM;
}

static void tcc_unmap_video_memory(struct fb_info *fbinfo)
{
	struct tccddi_fb *fb = fbinfo->par;
        dma_free_writecombine(fb->dev, fb->map_size, fb->map_cpu, fb->map_dma);
}

static int tcc_detect_props(struct fb_info *fbinfo)
{
	struct tccddi_fb *fb = fbinfo->par;
	int ret = -EINVAL;
	u32 val;

	/* If base address is set, we assume this one is in use */
	val = readl_relaxed(channel_base(fb, 2) + LIBA0);
	if (val) {
		/* Determine bits per pixel of display */
		val = readl_relaxed(channel_base(fb, 2) + LIC);
		switch(readl_relaxed(channel_base(fb, 2) + LIC) & 0x1f) {
			case 0xa: /* RGB565 */
				fbinfo->var.bits_per_pixel = 16;
				break;
			case 0xc: /* RGB888 */
				fbinfo->var.bits_per_pixel = 32;
				break;
			default:
				return ret;
		}

		/* Determine dimension of display */
		val = readl_relaxed(channel_base(fb, 2) + LIS);
		if (!val)
			return ret;

		fbinfo->var.xres = val & 0xffff;
		fbinfo->var.yres = val >> 16;

		/* XXX check validity of xres/yres */
		ret = 0;
	}

	return ret;
}

#ifdef DEBUG
static void dump_registers(struct fb_info *fbinfo)
{
	struct tccddi_fb *fb = fbinfo->par;
	int o;

	for (o=0; o < 0x120; o += 4)
		printk("LCDC: %04x = %08x\n", o, readl_relaxed(fb->regs + o));
}
#endif

static int tcc_get_props(struct device_node *np, struct fb_info *fbinfo)
{
	if (of_property_read_u32(np, "width", &fbinfo->var.xres) ||
		of_property_read_u32(np, "height", &fbinfo->var.yres) ||
		of_property_read_u32(np, "depth", &fbinfo->var.bits_per_pixel)) {
		return tcc_detect_props(fbinfo);
        }

        return 0;
}

static inline u32 convert_bitfield(int val, struct fb_bitfield *bf)
{
	unsigned int mask = (1 << bf->length) - 1;

	return (val >> (16 - bf->length) & mask) << bf->offset;
}

/* set the software color map.  Probably doesn't need modifying. */
static int
tccddi_fb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
		    unsigned int blue, unsigned int transp,
		    struct fb_info *info)
{
	struct tccddi_fb *fb = info->par;

	if (regno < 16) {
		fb->cmap[regno] = convert_bitfield(transp, &info->var.transp) |
		    convert_bitfield(blue, &info->var.blue) |
		    convert_bitfield(green, &info->var.green) |
		    convert_bitfield(red, &info->var.red);
		pr_debug("%s(%p,%d, %d,%d,%d,%d) => %08x\n", __func__, info, regno, red,green,blue,transp, fb->cmap[regno]);
		return 0;
	} else {
		return 1;
	}
}

/* check var to see if supported by this device.  Probably doesn't
 * need modifying.
 */
static int tccddi_fb_check_var(struct fb_var_screeninfo *var,
			       struct fb_info *info)
{
	pr_debug("%s(%p)\n", __func__, info);

	if ((var->rotate & 1) != (info->var.rotate & 1)) {
		if ((var->xres != info->var.yres) ||
		    (var->yres != info->var.xres) ||
		    (var->xres_virtual != info->var.yres) ||
		    (var->yres_virtual >
		     info->var.xres * ANDROID_NUMBER_OF_BUFFERS) ||
		    (var->yres_virtual < info->var.xres)) {
			pr_debug("check_var failed 1\n");
			return -EINVAL;
		}
	} else {
		if ((var->xres != info->var.xres) ||
		    (var->yres != info->var.yres) ||
		    (var->xres_virtual != info->var.xres) ||
		    (var->yres_virtual >
		     info->var.yres * ANDROID_NUMBER_OF_BUFFERS) ||
		    (var->yres_virtual < info->var.yres)) {
			pr_debug("check_var failed 2\n");
			return -EINVAL;
		}
	}
	if ((var->xoffset != info->var.xoffset) ||
	    (var->bits_per_pixel != info->var.bits_per_pixel) ||
	    (var->grayscale != info->var.grayscale)) {
		pr_debug("check_var failed 3\n");
		return -EINVAL;
	}

        if (var->bits_per_pixel == 16) {
                var->red.offset         = 11;
                var->green.offset       = 5;
                var->blue.offset        = 0;
                var->red.length         = 5;
                var->green.length       = 6;
                var->blue.length        = 5;
                var->transp.length      = 0;
        } else if (var->bits_per_pixel == 32) {
                var->red.offset         = 16;
                var->green.offset       = 8;
                var->blue.offset        = 0;
                var->transp.offset      = 24;
                var->red.length         = 8;
                var->green.length       = 8;
                var->blue.length        = 8;
                var->transp.length      = 8;
	} else {
		pr_err("Unsupported pixel depth %d!\n", var->bits_per_pixel);
		return -EINVAL;
	}


	return 0;
}

/* Handles screen rotation if device supports it. */
static int tccddi_fb_set_par(struct fb_info *fbinfo)
{
	struct fb_var_screeninfo *var = &fbinfo->var;

	pr_debug("%s(%p)\n", __func__, fbinfo);

	if (var->bits_per_pixel == 16)
		fbinfo->fix.visual = FB_VISUAL_TRUECOLOR;
	else if (var->bits_per_pixel == 32)
		fbinfo->fix.visual = FB_VISUAL_TRUECOLOR;
	else
		fbinfo->fix.visual = FB_VISUAL_PSEUDOCOLOR;

        fbinfo->fix.line_length = (var->xres_virtual * var->bits_per_pixel) / 8;

	return 0;
}

/* Pan the display if device supports it. */
static int tccddi_fb_pan_display(struct fb_var_screeninfo *var,
				 struct fb_info *fbinfo)
{
        struct tccddi_fb *fb = fbinfo->par;
        unsigned int base_addr = 0;

        base_addr = fb->map_dma +
		fbinfo->fix.line_length * var->yoffset +
		(fbinfo->var.xoffset * fbinfo->var.bits_per_pixel / 8);

	pr_debug("%s(%p,%08x)\n", __func__, fbinfo, base_addr);
	writel_relaxed(base_addr, channel_base(fb, 2) + LIBA0);

        return 0;
}

static struct fb_ops tccddi_fb_ops = {
	.owner = THIS_MODULE,
	.fb_check_var = tccddi_fb_check_var,
	.fb_set_par = tccddi_fb_set_par,
	.fb_setcolreg = tccddi_fb_setcolreg,
	.fb_pan_display = tccddi_fb_pan_display,

	/* These are generic software based fb functions */
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
};

static const struct of_device_id of_tccddi_fb_table[];

static int tccddi_fb_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
        const struct of_device_id *match;

	struct fb_info *fbinfo;
	struct tccddi_fb *fb;
	int ret;

	/* allocate framebuffer */
	fbinfo = framebuffer_alloc(sizeof(struct tccddi_fb), &pdev->dev);
	if (!fbinfo) {
		ret = -ENOMEM;
		goto err_fb_alloc_failed;
	}
	platform_set_drvdata(pdev, fbinfo);
	fb = fbinfo->par;

	fbinfo->fbops = &tccddi_fb_ops;
	fbinfo->flags = FBINFO_FLAG_DEFAULT;
	fbinfo->pseudo_palette = fb->cmap;

	/* setup private structure */
	fb->regs = of_iomap(np, 0);
	if (!fb->regs) {
		ret = -EIO;
		goto err_reg_iomap_failed;
	}
        match = of_match_node(of_tccddi_fb_table, pdev->dev.of_node);
	fb->channel_base = fb->regs + (u32)match->data;
	init_waitqueue_head(&fb->vsync_info.wait);

	fb->clk = of_clk_get(np, 0 /*lcdcX*/);
	if (IS_ERR(fb->clk)) {
		ret = PTR_ERR(fb->clk);
		goto err_fb_clk_get_failed;
	}

	fb->si_clk = of_clk_get(np, 1 /*lcdsi*/);
	if (IS_ERR(fb->si_clk)) {
		ret = PTR_ERR(fb->si_clk);
		goto err_si_clk_get_failed;
	}

	ret = clk_prepare_enable(fb->clk);
	if (ret != 0)
		goto err_fb_clk_enable_failed;

	ret = clk_prepare_enable(fb->si_clk);
	if (ret != 0)
		goto err_si_clk_enable_failed;

	/* setup var */
	ret = tcc_get_props(np, fbinfo);
	if (ret)
		goto err_get_props_failed;

#ifdef DEBUG
	dump_registers(fbinfo);
#endif

	fbinfo->var.xres_virtual = fbinfo->var.xres;
	fbinfo->var.yres_virtual = fbinfo->var.yres * ANDROID_NUMBER_OF_BUFFERS;
	fbinfo->var.activate = FB_ACTIVATE_NOW;
	fbinfo->var.vmode = FB_VMODE_NONINTERLACED;

	/* setup fix */
	fbinfo->fix.type = FB_TYPE_PACKED_PIXELS;
	fbinfo->fix.ypanstep = 1;
	fbinfo->fix.accel = FB_ACCEL_NONE;
	fbinfo->fix.smem_len = fbinfo->var.xres_virtual * fbinfo->var.yres_virtual * 32 / 8;
	fbinfo->pseudo_palette = (void*) fb->cmap;

	ret = tcc_map_video_memory(fbinfo);
	if (ret)
		goto err_map_mem_failed;

	tccddi_fb_check_var(&fbinfo->var,fbinfo);
	ret = fb_set_var(fbinfo, &fbinfo->var);
	if (ret)
		goto err_fb_set_var_failed;

	tccddi_fb_set_par(fbinfo);

	ret = register_framebuffer(fbinfo);
	if (ret)
		goto err_register_framebuffer_failed;

	fb_prepare_logo(fbinfo, 0);
	fb_show_logo(fbinfo, 0);

	return 0;

err_register_framebuffer_failed:
	tcc_unmap_video_memory(fbinfo);
err_map_mem_failed:
err_fb_set_var_failed:
err_get_props_failed:
	clk_disable_unprepare(fb->clk);
err_si_clk_enable_failed:
	clk_disable_unprepare(fb->si_clk);
err_fb_clk_enable_failed:
	clk_put(fb->si_clk);
err_si_clk_get_failed:
	clk_put(fb->clk);
err_fb_clk_get_failed:
	iounmap(fb->regs);
err_reg_iomap_failed:
	framebuffer_release(fbinfo);
err_fb_alloc_failed:
	return ret;
}

static int tccddi_fb_remove(struct platform_device *pdev)
{
	struct fb_info *fbinfo = platform_get_drvdata(pdev);

	unregister_framebuffer(fbinfo);
	framebuffer_release(fbinfo);
	tcc_unmap_video_memory(fbinfo);

	return 0;
}

static const struct of_device_id of_tccddi_fb_table[] = {
	{ .compatible = "tcc,tcc92xx-lcdc", .data = (void*)0x78, },
	{ .compatible = "tcc,tcc88xx-lcdc", .data = (void*)0x80, },
	{ /* sentinel */ },
};

MODULE_DEVICE_TABLE(of, of_tccddi_fb_table);

static struct platform_driver tccddi_fb_driver = {
	.probe = tccddi_fb_probe,
	.remove = tccddi_fb_remove,
	.driver = {
		.name = "tccddi_fb",
		.of_match_table = of_tccddi_fb_table,
	}
};
module_platform_driver(tccddi_fb_driver);

MODULE_AUTHOR("Ithamar R. Adema");
MODULE_DESCRIPTION("DDI Framebuffer driver for Telechips SoCs");
MODULE_LICENSE("GPL");
