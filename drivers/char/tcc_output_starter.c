/*
 * linux/drivers/video/tcc_output_starter.c
 *
 * Author:  <linux@telechips.com>
 * Created: March 18, 2010
 * Description: TCC Component TV-Out Driver
 *
 * Copyright (C) 20010-2011 Telechips 
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
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/i2c/hdmi_phy.h>
#include <linux/i2c/hdmi_edid.h>

#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/uaccess.h>
#include <asm/gpio.h>

#include <linux/slab.h>

#if defined(CONFIG_ARCH_TCC92XX) || defined(CONFIG_ARCH_TCC93XX)
#include <mach/bsp.h>
#elif defined(CONFIG_ARCH_TCC79X)
#include <mach/tcc79x.h>
#endif
#include <asm/delay.h>
#include <linux/delay.h>
#include <mach/tca_ckc.h>

#include <mach/audio.h>
#include <mach/video.h>
#include <mach/hdmi.h>
#include <mach/tccfb_ioctrl.h>

#include <mach/tcc_composite_ioctl.h>
#include <mach/tcc_component_ioctl.h>
#include <mach/tca_fb_output.h>
#include <mach/tcc_scaler_ctrl.h>
#include "hdmi/hdmi/regs-hdmi.h"
#include "../video/tcc/tcc_component.h"
#include "../video/tcc/tcc_composite.h"

/* Debugging stuff */
static int debug = 0;
#define DPRINTF(msg...)	if (debug) { printk( "tcc_output_starter: " msg); }

#if defined (CONFIG_MACH_TCC9300ST)
#define GPIO_OUTPUT_COMPOSITE_DETECT	TCC_GPE(30)
#define GPIO_OUTPUT_COMPONENT_DETECT	TCC_GPE(28)
#elif defined(CONFIG_MACH_TCC8800ST)
#define GPIO_OUTPUT_COMPOSITE_DETECT	TCC_GPF(27)
#define GPIO_OUTPUT_COMPONENT_DETECT	TCC_GPF(26)
#else
#define GPIO_OUTPUT_COMPOSITE_DETECT	NULL
#define GPIO_OUTPUT_COMPONENT_DETECT	NULL
#endif

extern void tcc_hdmi_power_on(void);
extern void hdmi_phy_reset(void);
extern void tcc92xxfb_hdmi_starter(char hdmi_lcdc_num, struct lcdc_timimg_parms_t *lcdc_timing);
extern int hdmi_set_color_space(enum ColorSpace);
extern int hdmi_set_color_depth(enum ColorDepth);
extern void hdmi_set_video_mode(struct device_video_params mode);
extern int hdmi_set_pixel_limit(enum PixelLimit);
extern int hdmi_set_hdmimode(int mode);
extern int hdmi_set_pixel_aspect_ratio(enum PixelAspectRatio ratio);
extern void hdmi_start(void);
extern int tcc_composite_set_lcdc(int lcdc_num);
extern void tcc_composite_get_spec(COMPOSITE_MODE_TYPE mode, COMPOSITE_SPEC_TYPE *spec);
extern int tcc_component_set_lcdc(int lcdc_num);
extern void tcc_component_get_spec(COMPONENT_MODE_TYPE mode, COMPONENT_SPEC_TYPE *spec);
extern void cs4954_enable(int type);
extern void ths8200_enable(int type);
extern void TCC_OUTPUT_LCDC_OnOff(char output_type, char output_lcdc_num, char onoff);

extern struct display_platform_data tcc_display_data;

extern struct HDMIVideoParameter gHdmiVideoParms;

//! Structure for video timing parameters
static const struct hdmi_video_params
{
    /** [H Blank] */
    unsigned int    HBlank;

    /** [V Blank] */
    unsigned int    VBlank;

    /**
     * [H Total : V Total] @n
     * For more information, refer HDMI register map.
     */
    unsigned int    HVLine;

    /**
     * [H Sync polarity : H Sync end point : H Sync start point]@n
     * For more information, refer HDMI register map.
     */
    unsigned int    HSYNCGEN;

    /**
     * [V Sync start line num + V Sync end line num] @n
     * For more information, refer HDMI register map.
     */
    unsigned int    VSYNCGEN;

    /** CEA VIC */
    unsigned char   AVI_VIC;
    /** CEA VIC for 16:9 pixel ratio */
    unsigned char   AVI_VIC_16_9;

    /** 0 - progresive, 1 - interlaced */
    unsigned char   interlaced;

    /** Pixel repetition if double, set 1 */
    unsigned char   repetition;

    /** V Sync polarity */
    unsigned char   polarity;

    /**
     * In case of interlaced mode, @n
     * [end point of bottom field's active region : start point of that]@n
     * For more information, refer HDMI register map.
     */
    unsigned int    VBLANK_F;

    /**
     * In case of interlaced mode, @n
     * [start line of bottom field's V Sync : end line of that]@n
     * For more information, refer HDMI register map.
     */
    unsigned int    VSYNCGEN2;

    /**
     * In case of interlaced mode, @n
     * [start transition point of bottom field's V Sync : end transition of that]@n
     * For more information, refer HDMI register map.
     */
    unsigned int    VSYNCGEN3;

    /** Pixel frequency */
    enum PixelFreq PixelClock; // pixel clock
} HDMIVideoParams[] =
{

  //{ 0x140, 0x13326, 0x540326, 0x1070A0, 0x2008 , 1 , 1 , 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_65, },  // v1024x768p_60Hz
  //{ 0x140, 0x13326, 0x540326, 0x1701C , 0x3009 , 1 , 1 , 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_65, },  // v1024x768p_60Hz
    { 0xA0 , 0x16A0D, 0x32020D, 0x11B80E, 0xA00C , 1 , 1 , 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_25_200,  },  // v640x480p_60Hz
    { 0x8A , 0x16A0D, 0x35A20D, 0x11300E, 0x900F , 2 , 3 , 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_27_027,  },  // v720x480p_60Hz
    { 0x172, 0xF2EE , 0x6722EE, 0x2506C , 0x500A , 4 , 4 , 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1280x720p_60Hz
    { 0x118, 0xB232 , 0x898465, 0x20856 , 0x2007 , 5 , 5 , 1, 0, 0, 0x232A49, 0x234239, 0x4A44A4, PIXEL_FREQ_74_250,  },  // v1920x1080i_60Hz
    { 0x114, 0xB106 , 0x6B420D, 0x128024, 0x4007 , 6 , 7 , 1, 1, 1, 0x10691D, 0x10A10D, 0x380380, PIXEL_FREQ_27_027,  },  // v720x480i_60Hz
    { 0x114, 0xB106 , 0x6B4106, 0x128024, 0x4007 , 8 , 9 , 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_27_027,  },  // v720x240p_60Hz
    { 0x228, 0xB106 , 0xD6820D, 0x15084A, 0x4007 , 10, 11, 1, 1, 1, 0x10691D, 0x10A10D, 0x700700, PIXEL_FREQ_54_054,  },  // v2880x480i_60Hz
    { 0x228, 0xB106 , 0x6B4106, 0x15084A, 0x4007 , 12, 13, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_54_054,  },  // v2880x240p_60Hz
    { 0x114, 0x16A0D, 0x6B420D, 0x12681E, 0x900F , 14, 15, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_54_054,  },  // v1440x480p_60Hz
    { 0x118, 0x16C65, 0x898465, 0x20856 , 0x4009 , 16, 16, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1920x1080p_60Hz
    { 0x90 , 0x18A71, 0x360271, 0x11280A, 0x500A , 17, 18, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_27,      },  // v720x576p_50Hz
    { 0x2BC, 0xF2EE , 0x7BC2EE, 0x779B6 , 0x500A , 19, 19, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1280x720p_50Hz
    { 0x2D0, 0xB232 , 0xA50465, 0x8EA0E , 0x2007 , 20, 20, 1, 0, 0, 0x232A49, 0x234239, 0x738738, PIXEL_FREQ_74_250,  },  // v1920x1080i_50Hz
    { 0x120, 0xC138 , 0x6C0271, 0x125016, 0x2005 , 21, 22, 1, 1, 1, 0x138951, 0x13A13D, 0x378378, PIXEL_FREQ_27,      },  // v720x576i_50Hz
    { 0x120, 0xC138 , 0x6C0138, 0x125016, 0x3006 , 23, 24, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_27,      },  // v720x288p_50Hz
    { 0x240, 0xC138 , 0xD80271, 0x14A82E, 0x2005 , 25, 26, 1, 1, 1, 0x138951, 0x13A13D, 0x6F06F0, PIXEL_FREQ_54,      },  // v2880x576i_50Hz
    { 0x240, 0xC138 , 0xD80138, 0x14A82E, 0x2005 , 27, 28, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_54,      },  // v2880x288p_50Hz
    { 0x120, 0x18A71, 0x6C0271, 0x125816, 0x500A , 29, 30, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_54,      },  // v1440x576p_50Hz
    { 0x2D0, 0x16C65, 0xA50465, 0x8EA0E , 0x4009 , 31, 31, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1920x1080p_50Hz
    { 0x33E, 0x16C65, 0xABE465, 0xAA27C , 0x4009 , 32, 32, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1920x1080p_24Hz
    { 0x2D0, 0x16C65, 0xA50465, 0x8EA0E , 0x4009 , 33, 33, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1920x1080p_25Hz
    { 0x2D0, 0x16C65, 0xA50465, 0x8EA0E , 0x4009 , 34, 34, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_74_250,  },  // v1920x1080p_30Hz
    { 0x228, 0x16A0D, 0xD6820D, 0x14D83E, 0x900F , 35, 36, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_108_108, },  // v2880x480p_60Hz
    { 0x240, 0x18A71, 0xD80271, 0x14B82E, 0x500A , 37, 38, 0, 1, 1, 0       , 0       , 0       , PIXEL_FREQ_108,     },  // v2880x576p_50Hz
    { 0x180, 0x2AA71, 0x9004E2, 0x3181E , 0x1701C, 39, 39, 0, 0, 0, 0x2712C6, 0x28728F, 0x4a44a4, PIXEL_FREQ_72,      },  // v1920x1080i_50Hz(1250)
    { 0x2D0, 0xB232 , 0xA50465, 0x8EA0E , 0x2007 , 40, 40, 1, 0, 0, 0x232A49, 0x234239, 0x738738, PIXEL_FREQ_148_500, },  // v1920x1080i_100Hz
    { 0x2BC, 0xF2EE , 0x7BC2EE, 0x779B6 , 0x500A , 41, 41, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1280x720p_100Hz
    { 0x90 , 0x18A71, 0x360271, 0x11280A, 0x500A , 42, 43, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_54,      },  // v720x576p_100Hz
    { 0x120, 0xC138 , 0x6C0271, 0x125016, 0x2005 , 44, 45, 1, 1, 1, 0x138951, 0x13A13D, 0x378378, PIXEL_FREQ_54,      },  // v720x576i_100Hz
    { 0x118, 0xB232 , 0x898465, 0x20856 , 0x2007 , 46, 46, 1, 0, 0, 0x232A49, 0x234239, 0x4A44A4, PIXEL_FREQ_148_500, },  // v1920x1080i_120Hz
    { 0x172, 0xF2EE , 0x6722EE, 0x2506C , 0x500A , 47, 47, 0, 0, 0, 0       , 0       , 0       , PIXEL_FREQ_148_500, },  // v1280x720p_120Hz
    { 0x8A , 0x16A0D, 0x35A20D, 0x11300E, 0x900F , 48, 49, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_54_054,  },  // v720x480p_120Hz
    { 0x114, 0xB106 , 0x6B420D, 0x128024, 0x4007 , 50, 51, 1, 1, 1, 0x10691D, 0x10A10D, 0x380380, PIXEL_FREQ_54_054,  },  // v720x480i_120Hz
    { 0x90 , 0x18A71, 0x360271, 0x11280A, 0x500A , 52, 53, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_108,     },  // v720x576p_200Hz
    { 0x120, 0xC138 , 0x6C0271, 0x125016, 0x2005 , 54, 55, 1, 1, 1, 0x138951, 0x13A13D, 0x378378, PIXEL_FREQ_108,     },  // v720x576i_200Hz
    { 0x8A , 0x16A0D, 0x35A20D, 0x11300E, 0x900F , 56, 57, 0, 0, 1, 0       , 0       , 0       , PIXEL_FREQ_108_108, },  // v720x480p_240Hz
    { 0x114, 0xB106 , 0x6B420D, 0x128024, 0x4007 , 58, 59, 1, 1, 1, 0x10691D, 0x10A10D, 0x380380, PIXEL_FREQ_108_108, },  // v720x480i_240Hz
};

#if defined(CONFIG_MACH_TCC9200) || defined(CONFIG_MACH_TCC9201) || defined(CONFIG_MACH_TCC8900)
static const unsigned char phy_config[][3][40] = {
	// for internal PLL
	// freq = 25.200 MHz
	{
		{ 0x85, 0x88, 0x20, 0x50, 0x00, 0x0C, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x46, 0x08, 0x0A, 0xBB, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x0C, 0x03, 0x23, 0x38, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x24, 0x96, 0x6D, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x20, 0x50, 0x00, 0x0C, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x56, 0x08, 0x0A, 0xBB, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x0C, 0x04, 0x23, 0x46, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x24, 0x96, 0x57, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x20, 0x50, 0x00, 0x0C, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x56, 0x08, 0x0A, 0xBB, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x08, 0x02, 0x23, 0x2A, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x24, 0x16, 0x92, 0x02, 0x00, 0x80 },
	},
	// freq = 25.175 MHz
	{
		{ 0x85, 0x88, 0x20, 0x50, 0x00, 0x0C, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x46, 0x07, 0x0A, 0xBB, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x0C, 0x03, 0x23, 0x38, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x24, 0x96, 0x6D, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x20, 0x50, 0x00, 0x0C, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x56, 0x07, 0x0A, 0xBB, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x0C, 0x04, 0x23, 0x46, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x24, 0x96, 0x57, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x20, 0x50, 0x00, 0x0C, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x56, 0x07, 0x0A, 0xBB, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x08, 0x02, 0x23, 0x2A, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x24, 0x56, 0x92, 0x02, 0x00, 0x80 },
	},
	// freq = 27 MHz
	{
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x0B, 0xD8, 0xC2, 0xA4, 0x06, 0xD8, 0x5B, 0xE2, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x1C, 0x03, 0x23, 0x3C, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xFF, 0x55, 0x66, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x0B, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0xE2, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x1C, 0x04, 0x23, 0x4B, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xFF, 0xD5, 0x51, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x0B, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0xE2, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x18, 0x02, 0x23, 0x2D, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xFF, 0x55, 0x88, 0x02, 0x00, 0x80 },
	},
	// freq = 27.027 MHz
	{
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x0B, 0xD8, 0xC2, 0xA4, 0x46, 0x28, 0x4D, 0xE3, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x1C, 0x03, 0x23, 0x3C, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xFE, 0x15, 0x66, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x0B, 0xD8, 0xC2, 0xA4, 0x56, 0x28, 0x4D, 0xE3, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x1C, 0x04, 0x23, 0x4B, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xFE, 0x95, 0x51, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x0B, 0xD8, 0xC2, 0xA4, 0x56, 0x28, 0x4D, 0xE3, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x18, 0x02, 0x23, 0x2D, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xFE, 0x55, 0x88, 0x02, 0x00, 0x80 },
	},
	// freq = 54 MHz
	{
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x07, 0xD8, 0xC2, 0xA4, 0x06, 0xD8, 0x5B, 0xE2, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x18, 0x03, 0x23, 0x3C, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xFF, 0x54, 0x66, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x07, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0xE2, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x18, 0x04, 0x23, 0x4B, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xFF, 0xD4, 0x51, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x07, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0xE2, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x14, 0x02, 0x23, 0x2D, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xFF, 0x54, 0x88, 0x02, 0x00, 0x80 },
	},
	// freq = 54.054 MHz
	{
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x07, 0xD8, 0xC2, 0xA4, 0x46, 0x28, 0x4D, 0xE3, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x18, 0x03, 0x23, 0x3C, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xFF, 0x14, 0x66, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x07, 0xD8, 0xC2, 0xA4, 0x56, 0x28, 0x4D, 0xE3, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x18, 0x04, 0x23, 0x4B, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xFF, 0x94, 0x51, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x07, 0xD8, 0xC2, 0xA4, 0x56, 0x28, 0x4D, 0xE3, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x14, 0x02, 0x23, 0x2D, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xFF, 0x54, 0x88, 0x02, 0x00, 0x80 },
	},
	// freq = 74.25 MHz
	{
		{ 0x85, 0x88, 0x00, 0xA0, 0x2C, 0x07, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x06, 0xD8, 0x5B, 0x22, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x18, 0x03, 0xA3, 0x52, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xB9, 0x54, 0x4A, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x2C, 0x07, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0x22, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x14, 0x01, 0xA3, 0x33, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xB9, 0x14, 0x77, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x2C, 0x07, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0x22, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x14, 0x02, 0x23, 0x3E, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xB9, 0x14, 0x63, 0x02, 0x00, 0x80 },
	},
	// freq = 74.176 MHz
	{
		{ 0x85, 0x88, 0x00, 0xA0, 0x2C, 0x07, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x46, 0xD8, 0x5B, 0x23, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x18, 0x03, 0xA3, 0x52, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xB9, 0x54, 0x4A, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x2C, 0x07, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x56, 0xD8, 0x5B, 0x23, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x14, 0x01, 0xA3, 0x33, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xB9, 0x14, 0x77, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x2C, 0x07, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x56, 0xD8, 0x5B, 0x23, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x14, 0x02, 0xE3, 0x3D, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xB9, 0x54, 0x63, 0x02, 0x00, 0x80 },
	},
	// freq = 148.5 MHz
	{
		{ 0x85, 0x88, 0x00, 0xA0, 0x2C, 0x07, 0x00, 0x02, 0xD8, 0xC2, 0xA4, 0x06, 0xD8, 0x5B, 0x22, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x14, 0x03, 0xA3, 0x52, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x5C, 0x54, 0x4A, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x2C, 0x07, 0x00, 0x02, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0x22, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x10, 0x01, 0xA3, 0x33, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x5C, 0x14, 0x77, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x2C, 0x07, 0x00, 0x02, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0x22, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x10, 0x02, 0x23, 0x3E, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x5C, 0x14, 0x63, 0x02, 0x00, 0x80 },
	},
	// freq = 148.352 MHz
	{
		{ 0x85, 0x88, 0x00, 0xA0, 0x2C, 0x07, 0x00, 0x02, 0xD8, 0xC2, 0xA4, 0x46, 0xD8, 0x5B, 0x23, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x14, 0x03, 0xA3, 0x52, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x5C, 0x54, 0x4A, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x2C, 0x07, 0x00, 0x02, 0xD8, 0xC2, 0xA4, 0x56, 0xD8, 0x5B, 0x23, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x10, 0x01, 0xA3, 0x33, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x5C, 0x14, 0x77, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x2C, 0x07, 0x00, 0x02, 0xD8, 0xC2, 0xA4, 0x56, 0xD8, 0x5B, 0x23, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x10, 0x02, 0xE3, 0x3D, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x5C, 0x54, 0x63, 0x02, 0x00, 0x80 },
	},
	// freq = 108.108 MHz
	{
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x03, 0xD8, 0xC2, 0xA4, 0x46, 0x28, 0x4D, 0xE3, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x14, 0x03, 0x23, 0x3C, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x7F, 0x14, 0x66, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x03, 0xD8, 0xC2, 0xA4, 0x56, 0x28, 0x4D, 0xE3, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x14, 0x04, 0x23, 0x4B, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x7F, 0x94, 0x51, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x03, 0xD8, 0xC2, 0xA4, 0x56, 0x28, 0x4D, 0xE3, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x10, 0x02, 0x23, 0x2D, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x7F, 0x54, 0x88, 0x02, 0x00, 0x80 },
	},
	// freq = 72 MHz
	{
		{ 0x85, 0x88, 0x00, 0xA0, 0x30, 0x0B, 0x00, 0x02, 0xD8, 0xC2, 0xA4, 0x06, 0xD8, 0x5B, 0x82, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x08, 0x03, 0x23, 0x50, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xBF, 0x94, 0x4C, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x30, 0x0B, 0x00, 0x02, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0x82, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x04, 0x01, 0x23, 0x32, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xBF, 0xD4, 0x7A, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x30, 0x0B, 0x00, 0x02, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0x82, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x04, 0x02, 0x23, 0x3C, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xBF, 0x54, 0x66, 0x02, 0x00, 0x80 },
	},
	// freq = 25 MHz
	{
		{ 0x85, 0x88, 0x00, 0x30, 0x00, 0x0C, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x06, 0xD8, 0x5B, 0xB2, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x0C, 0x03, 0xA3, 0x37, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x28, 0x56, 0x6E, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0x30, 0x00, 0x0C, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0xB2, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x0C, 0x04, 0xA3, 0x45, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x28, 0x56, 0x58, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0x30, 0x00, 0x0C, 0x00, 0x06, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0xB2, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x08, 0x02, 0xE3, 0x29, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x28, 0x56, 0x93, 0x02, 0x00, 0x80 },
	},
	// freq = 65 MHz
	{
		{ 0x85, 0x88, 0x40, 0xA0, 0x82, 0x0B, 0x00, 0x02, 0xD8, 0xC2, 0xA4, 0x06, 0xD8, 0x5B, 0x12, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x08, 0x03, 0x63, 0x48, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xD4, 0xD4, 0x54, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x40, 0xA0, 0x82, 0x0B, 0x00, 0x02, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0x12, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x04, 0x01, 0x63, 0x2D, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xD4, 0xD4, 0x87, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x40, 0xA0, 0x82, 0x0B, 0x00, 0x02, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0x12, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x04, 0x02, 0x63, 0x36, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0xD4, 0x54, 0x71, 0x02, 0x00, 0x80 },
	},
	// freq = 108 MHz
	{
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x03, 0xD8, 0xC2, 0xA4, 0x06, 0xD8, 0x5B, 0xE2, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x14, 0x03, 0x23, 0x3C, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x7F, 0x54, 0x66, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x03, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0xE2, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x14, 0x04, 0x23, 0x4B, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x7F, 0xD4, 0x51, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0xA0, 0x34, 0x07, 0x0D, 0x03, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0xE2, 0x84, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x10, 0x02, 0x23, 0x2D, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x7F, 0x54, 0x88, 0x02, 0x00, 0x80 },
	},
	// freq = 162 MHz
	{
		{ 0x85, 0x88, 0x00, 0x10, 0x00, 0x06, 0x00, 0x00, 0xD8, 0xC2, 0xA4, 0x06, 0xD8, 0x5B, 0xF2, 0x83, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x10, 0x00, 0x23, 0x2D, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x54, 0x54, 0x88, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0x10, 0x00, 0x06, 0x00, 0x00, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0xF2, 0x83, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x10, 0x01, 0x63, 0x38, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x54, 0x14, 0x6D, 0x02, 0x00, 0x80 },
		{ 0x85, 0x88, 0x00, 0x10, 0x00, 0x06, 0x00, 0x00, 0xD8, 0xC2, 0xA4, 0x16, 0xD8, 0x5B, 0xF2, 0x83, 0x03, 0x82, 0x55/*0x04*/, 0xE0, 0x00, 0x10, 0x02, 0xA3, 0x43, 0x38, 0x40, 0x88, 0x7C, 0x09, 0x13, 0x06, 0x00, 0x54, 0x54, 0xD4, 0x5A, 0x02, 0x00, 0x80 },
	}
};
#elif defined(CONFIG_MACH_TCC9300) || defined(CONFIG_MACH_TCC9300CM) || defined(CONFIG_MACH_TCC9300ST) || defined(CONFIG_MACH_TCC8800ST) || (defined(CONFIG_MACH_TCC8800) && defined(CONFIG_TCC_STB_MODE))
static const unsigned char phy_config[][3][31] = {// TCC9300 HDMI PHY Setting
        //25.200
    {
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x50, 0x11, 0x54, 0x58, 0x20, 0x5B, 0xA8, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0x11, 0x27, 0x00, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x50, 0x11, 0x54, 0x58, 0x20, 0x5B, 0xA8, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0x11, 0x27, 0x00, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x50, 0x11, 0x54, 0x58, 0x20, 0x5B, 0xA8, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0x11, 0x27, 0x00, 0x00, 0x00, 0x80},
    },
        //25.175
    {
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0xFC, 0x48, 0x6B, 0x50, 0x11, 0x54, 0x58, 0x20, 0x5B, 0xA8, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0x12, 0x27, 0x00, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0xFC, 0x48, 0x6B, 0x50, 0x11, 0x54, 0x58, 0x20, 0x5B, 0xA8, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0x12, 0x27, 0x00, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0xFC, 0x48, 0x6B, 0x50, 0x11, 0x54, 0x58, 0x20, 0x5B, 0xA8, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0x12, 0x27, 0x00, 0x00, 0x00, 0x80},
    },
        //27.000
    {
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x10, 0x02, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x00, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x10, 0x02, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x00, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x10, 0x02, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x00, 0x00, 0x00, 0x80},
    },
        //27.027
    {
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x02, 0x32, 0x6B, 0x10, 0x02, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x00, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x02, 0x32, 0x6B, 0x10, 0x02, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x00, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x02, 0x32, 0x6B, 0x10, 0x02, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x00, 0x00, 0x00, 0x80},
    },
        //54.000
    {
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x10, 0x01, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x01, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x10, 0x01, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x01, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x10, 0x01, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x01, 0x00, 0x00, 0x80},
    },
        //54.054
    {
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x10, 0x01, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x01, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x10, 0x01, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x01, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x10, 0x01, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x01, 0x00, 0x00, 0x80},
    },
        //74.250
    {
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x10, 0x01, 0x54, 0x98, 0x21, 0x5B, 0xA5, 0x84, 0x00, 0x20, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xB9, 0x26, 0x01, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x10, 0x01, 0x54, 0x98, 0x21, 0x5B, 0xA5, 0x84, 0x00, 0x20, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xB9, 0x26, 0x01, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x10, 0x01, 0x54, 0x98, 0x21, 0x5B, 0xA5, 0x84, 0x00, 0x20, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xB9, 0x26, 0x01, 0x00, 0x00, 0x80},
    },
        //74.1
    {
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x56, 0x5B, 0x6B, 0x10, 0x01, 0x54, 0x18, 0x25, 0x5B, 0xA5, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xB9, 0x26, 0x01, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x56, 0x5B, 0x6B, 0x10, 0x01, 0x54, 0x18, 0x25, 0x5B, 0xA5, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xB9, 0x26, 0x01, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x56, 0x5B, 0x6B, 0x10, 0x01, 0x54, 0x18, 0x25, 0x5B, 0xA5, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xB9, 0x26, 0x01, 0x00, 0x00, 0x80},
    },
        //148.500
    {
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x18, 0x01, 0x54, 0x98, 0x21, 0x5B, 0xA5, 0x84, 0x00, 0x20, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xB9, 0x26, 0x02, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x18, 0x01, 0x54, 0x98, 0x21, 0x5B, 0xA5, 0x84, 0x00, 0x20, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xB9, 0x26, 0x02, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x18, 0x01, 0x54, 0x98, 0x21, 0x5B, 0xA5, 0x84, 0x00, 0x20, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xB9, 0x26, 0x02, 0x00, 0x00, 0x80},
    },
        //148.352
    {
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x56, 0x5B, 0x6B, 0x18, 0x01, 0x54, 0x18, 0x25, 0x5B, 0xA5, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xB9, 0x26, 0x02, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x56, 0x5B, 0x6B, 0x18, 0x01, 0x54, 0x18, 0x25, 0x5B, 0xA5, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xB9, 0x26, 0x02, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x56, 0x5B, 0x6B, 0x18, 0x01, 0x54, 0x18, 0x25, 0x5B, 0xA5, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xB9, 0x26, 0x02, 0x00, 0x00, 0x80},
    },
        //108.108
    {
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x02, 0x32, 0x6B, 0x18, 0x01, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x02, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x02, 0x32, 0x6B, 0x18, 0x01, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x02, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x02, 0x32, 0x6B, 0x18, 0x01, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x02, 0x00, 0x00, 0x80},
    },
        //72.000
    {
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x18, 0x01, 0x54, 0x68, 0x21, 0x5B, 0x50, 0x84, 0x00, 0x21, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xBF, 0x26, 0x01, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x18, 0x01, 0x54, 0x68, 0x21, 0x5B, 0x50, 0x84, 0x00, 0x21, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xBF, 0x26, 0x01, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x18, 0x01, 0x54, 0x68, 0x21, 0x5B, 0x50, 0x84, 0x00, 0x21, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xBF, 0x26, 0x01, 0x00, 0x00, 0x80},
    },
        //25.000
    {
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0xE8, 0x36, 0x6B, 0x50, 0x11, 0x54, 0x58, 0x20, 0x5B, 0xA7, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0x13, 0x27, 0x00, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0xE8, 0x36, 0x6B, 0x50, 0x11, 0x54, 0x58, 0x20, 0x5B, 0xA7, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0x13, 0x27, 0x00, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0xE8, 0x36, 0x6B, 0x50, 0x11, 0x54, 0x58, 0x20, 0x5B, 0xA7, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0x13, 0x27, 0x00, 0x00, 0x00, 0x80},
    },
        //65.000
    {
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x08, 0x36, 0x6B, 0x10, 0x01, 0x54, 0x48, 0x20, 0x5B, 0x90, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xD4, 0x26, 0x01, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x08, 0x36, 0x6B, 0x10, 0x01, 0x54, 0x48, 0x20, 0x5B, 0x90, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xD4, 0x26, 0x01, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x9C, 0x08, 0x36, 0x6B, 0x10, 0x01, 0x54, 0x48, 0x20, 0x5B, 0x90, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xD4, 0x26, 0x01, 0x00, 0x00, 0x80},
    },
        //108.000
    {
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x18, 0x01, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x02, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x18, 0x01, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x02, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x18, 0x01, 0x54, 0x38, 0x20, 0x5B, 0x78, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xFF, 0x26, 0x02, 0x00, 0x00, 0x80},
    },
        //162.000
    {
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x18, 0x01, 0x54, 0x48, 0x21, 0x5B, 0xB4, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xAA, 0x26, 0x02, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x18, 0x01, 0x54, 0x48, 0x21, 0x5B, 0xB4, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xAA, 0x26, 0x02, 0x00, 0x00, 0x80},
        {0x05, 0x00, 0xD8, 0x10, 0x1C, 0x30, 0x40, 0x6B, 0x18, 0x01, 0x54, 0x48, 0x21, 0x5B, 0xB4, 0x84, 0x00, 0x00, 0x38, 0x00, 0x08, 0x10, 0xE0, 0x22, 0x40, 0xAA, 0x26, 0x02, 0x00, 0x00, 0x80},
    }
};
#else
#error not defined chip
#endif

#if (0)
#define DEFAULT_HDMI_LCDC_TIMING	{ 0,  1,  1,  0,  0,  1,  0,  1,  0,  61 ,    639 ,   59 ,     37 ,    0,  0,  5,  479 ,   29,  8,   5,   479 ,    29,  8, /*640x480p @ 60Hz       25.200MHz   */ }
#else
#define DEFAULT_HDMI_LCDC_TIMING	{ 0,  0,  0,  0,  0,  1,  0,  1,  0,  43 ,    1919,   147,     87 ,    0,  0,  4,  1079,   35,  3,   4,   1079,    35,  3, /*1920x1080p @ 60Hz     148.5MHz    */ }
#endif

static const struct lcdc_timimg_parms_t LCDCTimimgParams[] = 
{                                                  /*  ID  IV  IH  IP  DP  NI  TV TFT STN  LPW       LPC   LSWC     LEWC   VDB VDF FPW    FLC  FSWC FEWC FPW2   FLC2  FSWC2 FEWC2 */
  /* v640x480p_60Hz         PIXEL_FREQ_25_200,  */    { 0,  1,  1,  0,  0,  1,  0,  1,  0,  61 ,    639 ,   59 ,     37 ,    0,  0,  5,  479 ,   29,  8,   5,   479 ,    29,  8, /*640x480p @ 60Hz       25.200MHz   */ },
  /* v720x480p_60Hz         PIXEL_FREQ_27_027,  */    { 0,  1,  1,  0,  0,  1,  0,  1,  0,  61 ,    719 ,   59 ,     15 ,    0,  0,  5,  479 ,   29,  8,   5,   479 ,    29,  8, /*720x480p @ 60Hz       27.027MHz   */ },
  /* v1280x720p_60Hz        PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  39 ,    1279,   219,     109,    0,  0,  4,  719 ,   19,  4,   4,   719 ,    19,  4, /*1280x720p @ 60Hz      74.25MHz    */ },
  /* v1920x1080i_60Hz       PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  0,  1,  0,  0,  43 ,    1919,   147,     87 ,    0,  0,  9,  1079,   29,  3,   9,   1079,    31,  3, /*1920x1080i @ 60Hz     74.25MHz    */ },
  /* v720x480i_60Hz         PIXEL_FREQ_27_027,  */    { 0,  1,  1,  0,  1,  0,  1,  0,  0,  123,    1439,   113,     37 ,    0,  0,  5,  479 ,   29,  7,   5,   479 ,    31,  7, /*720x480i @ 60Hz       27.027MHz   */ },
  /* v720x240p_60Hz         PIXEL_FREQ_27_027,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v2880x480i_60Hz        PIXEL_FREQ_54_054,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v2880x240p_60Hz        PIXEL_FREQ_54_054,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1440x480p_60Hz        PIXEL_FREQ_54_054,  */    {0, 0, 0, 0, 1, 1, 0, 1, 0, 123, 1439, 120, 30, 0, 0, 5, 479, 32, 5, 5, 479, 32, 5, /*1440x480p @ 60Hz 54.054MHz*/ },
  /* v1920x1080p_60Hz       PIXEL_FREQ_148_500, */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  43 ,    1919,   147,     87 ,    0,  0,  4,  1079,   35,  3,   4,   1079,    35,  3, /*1920x1080p @ 60Hz     148.5MHz    */ },
  /* v720x576p_50Hz         PIXEL_FREQ_27,      */    { 0,  1,  1,  0,  0,  1,  0,  1,  0,  63 ,    719 ,   67 ,     11 ,    0,  0,  4,  575 ,   38,  4,   4,   575 ,    38,  4, /*720x576p @ 50Hz       27MHz       */ },
  /* v1280x720p_50Hz        PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  39 ,    1279,   219,     439,    0,  0,  4,  719 ,   19,  4,   4,   719 ,    19,  4, /*1280x720p @ 50Hz      74.25MHz    */ },
  /* v1920x1080i_50Hz       PIXEL_FREQ_74_250,  */    { 0,  0,  0,  0,  0,  0,  1,  0,  0,  43 ,    1919,   147,     527,    0,  0,  9,  1079,   29,  3,   9,   1079,    31,  3, /*1920x1080i @ 50Hz     74.25MHz    */ },
  /* v720x576i_50Hz         PIXEL_FREQ_27,      */    { 0,  1,  1,  0,  1,  0,  1,  0,  0,  125,    1439,   137,     23 ,    0,  0,  5,  575 ,   37,  3,   5,   575 ,    39,  3, /*720x576i @ 50Hz       27MHz       */ },
  /* v720x288p_50Hz         PIXEL_FREQ_27,      */    DEFAULT_HDMI_LCDC_TIMING,
  /* v2880x576i_50Hz        PIXEL_FREQ_54,      */    DEFAULT_HDMI_LCDC_TIMING,
  /* v2880x288p_50Hz        PIXEL_FREQ_54,      */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1440x576p_50Hz        PIXEL_FREQ_54,      */    {0, 0, 1, 0, 1, 1, 0, 1, 0, 125, 1439, 137, 23, 0, 0, 4, 575, 38, 4, 4, 575, 38, 4, /*1440x576p @ 50Hz 54MHz    */ },
  /* v1920x1080p_50Hz       PIXEL_FREQ_148_500, */    { 0,  0,  0,  0,  0,  1,  0,  1,  0,  43 ,    1919,   147,     87 ,    0,  0,  4,  1079,   35,  3,   4,   1079,    35,  3, /*1920x1080p @ 50Hz     148.5MHz    */ },
  /* v1920x1080p_24Hz       PIXEL_FREQ_74_250,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1920x1080p_25Hz       PIXEL_FREQ_74_250,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1920x1080p_30Hz       PIXEL_FREQ_74_250,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v2880x480p_60Hz        PIXEL_FREQ_108_108, */    DEFAULT_HDMI_LCDC_TIMING,
  /* v2880x576p_50Hz        PIXEL_FREQ_108,     */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1920x1080i_50Hz(1250) PIXEL_FREQ_72,      */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1920x1080i_100Hz      PIXEL_FREQ_148_500, */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1280x720p_100Hz       PIXEL_FREQ_148_500, */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x576p_100Hz        PIXEL_FREQ_54,      */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x576i_100Hz        PIXEL_FREQ_54,      */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1920x1080i_120Hz      PIXEL_FREQ_148_500, */    DEFAULT_HDMI_LCDC_TIMING,
  /* v1280x720p_120Hz       PIXEL_FREQ_148_500, */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x480p_120Hz        PIXEL_FREQ_54_054,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x480i_120Hz        PIXEL_FREQ_54_054,  */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x576p_200Hz        PIXEL_FREQ_108,     */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x576i_200Hz        PIXEL_FREQ_108,     */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x480p_240Hz        PIXEL_FREQ_108_108, */    DEFAULT_HDMI_LCDC_TIMING,
  /* v720x480i_240Hz        PIXEL_FREQ_108_108, */    DEFAULT_HDMI_LCDC_TIMING,
};

#define SUPPORT_HDMI_MODE_NUM	4
enum VideoFormat TccSupportHdmiMode[] = {
	/** 1920x1080p\@60Hz */
 	v1920x1080p_60Hz,
	/** 1920x1080i\@60Hz */
 	v1920x1080i_60Hz,
	/** 1280x700p\@60Hz */
	v1280x720p_60Hz,
	/** 720x480p\@60Hz */
	v720x480p_60Hz,
	/** 640x480p\@60Hz */
	v640x480p_60Hz,
};

enum
{
	STARTER_LCDC_0 = 0,
	STARTER_LCDC_1,
	STARTER_LCDC_MAX
};

enum
{
	STARTER_OUTPUT_LCD = 0,
	STARTER_OUTPUT_HDMI,
	STARTER_OUTPUT_COMPOSITE,
	STARTER_OUTPUT_COMPONENT,
	STARTER_OUTPUT_MAX
};

enum
{
	STARTER_HDMI_1920x1080P = 0,
	STARTER_HDMI_1920x1080I,
	STARTER_HDMI_1280x720P,
	STARTER_HDMI_720x480P,
	STARTER_HDMI_640x480P,
	STARTER_HDMI_MAX
};

enum
{
	STARTER_COMPOSITE_NTSC = 0,
	STARTER_COMPOSITE_PAL,
	STARTER_COMPOSITE_MAX
};

enum
{
	STARTER_COMPONENT_480I_NTSC = 0,
	STARTER_COMPONENT_576I_PAL,
	STARTER_COMPONENT_720P,
	STARTER_COMPONENT_1080I,
	STARTER_COMPONENT_MAX
};

#define PHY_I2C_ADDRESS				0x70>>1
#define PHY_REG_MODE_SET_DONE		0x28
#define PHY_CONFIG_START_OFFSET     0x01

#define EDID_I2C_ADDRESS			0xA0
#define EDID_SEG_I2C_ADDRESS		0x60

static unsigned short probe_i2c[] = {3, PHY_I2C_ADDRESS, I2C_CLIENT_END};	/* { i2c_num, i2c_addr } */
static unsigned short dummy[] = {I2C_CLIENT_END};

static struct i2c_client_address_data addr_data = {
	.normal_i2c = dummy,
	.probe = probe_i2c,
	.ignore = dummy,
};

#if defined(CONFIG_ARCH_TCC93XX) || defined(CONFIG_ARCH_TCC88XX)
static const struct i2c_device_id tcc_hdmi_phy_i2c_id[] = {
	{ "tcc-hdmi-phy", 0, },
	{ }
};

static const struct i2c_device_id tcc_hdmi_edid_i2c_id[] = {
	{ "tcc-hdmi-edid", 0, },
	{ }
};

static const struct i2c_device_id tcc_hdmi_edid_seg_i2c_id[] = {
	{ "tcc-hdmi-edid-seg", 0, },
	{ }
};

struct tcc_hdmi_i2c_chip_info {
	unsigned gpio_start;
	uint16_t reg_output;
	uint16_t reg_direction;

	struct i2c_client *client;
	struct gpio_chip gpio_chip;
};
#endif

static struct i2c_driver tcc_hdmi_phy_i2c_driver;

struct i2c_client *tcc_hdmi_phy_i2c_client = NULL;
struct i2c_client *tcc_hdmi_edid_i2c_client = NULL;
struct i2c_client *tcc_hdmi_edid_seg_i2c_client = NULL;

#if defined(CONFIG_ARCH_TCC93XX) || defined(CONFIG_ARCH_TCC88XX)
static int tcc_hdmi_phy_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct tcc_hdmi_phy_i2c_platform_data 	*pdata;
	struct tcc_hdmi_i2c_chip_info 			*chip;

	pdata = client->dev.platform_data;
	if(pdata == NULL)
	{
		dev_dbg(&client->dev, "no platform data\n");
		printk("\n tcc_hdmi_phy_i2c  :  no platform data. \n");
		return -EINVAL;
	}

	chip = kzalloc(sizeof(struct tcc_hdmi_i2c_chip_info), GFP_KERNEL);
	if(chip == NULL)
	{
		printk("\n tcc_hdmi_phy_i2c  :  no chip info. \n");
		return -ENOMEM;
	}

	chip->client = client;
	i2c_set_clientdata(client, chip);
	tcc_hdmi_phy_i2c_client = client;

	return 0;
}

static int tcc_hdmi_phy_i2c_remove(struct i2c_client *client)
{
	struct tcc_hdmi_phy_i2c_platform_data 	*pdata = client->dev.platform_data;
	struct tcc_hdmi_i2c_chip_info 			*chip  = i2c_get_clientdata(client);
	int ret = 0;

	if(pdata->teardown)
	{
		ret = pdata->teardown(client, chip->gpio_chip.base, chip->gpio_chip.ngpio, pdata->context);
		if(ret < 0)
		{
			dev_err(&client->dev, "%s failed, %d\n", "teardown", ret);
			printk("\n tcc_hdmi_phy_i2c  :  teardown error. \n");
			return ret;
		}
	}

	kfree(chip);
	tcc_hdmi_phy_i2c_client = NULL;
	
	return 0;
}

static struct i2c_driver tcc_hdmi_phy_i2c_driver = {
	.driver = {
		.name	= "tcc-hdmi-phy",
	},
	.probe		= tcc_hdmi_phy_i2c_probe,
	.remove		= tcc_hdmi_phy_i2c_remove,
	.id_table	= tcc_hdmi_phy_i2c_id,
};

static int tcc_hdmi_edid_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct tcc_hdmi_edid_i2c_platform_data 	*pdata;
	struct tcc_hdmi_i2c_chip_info 			*chip;

	pdata = client->dev.platform_data;
	if(pdata == NULL)
	{
		dev_dbg(&client->dev, "no platform data\n");
		printk("\n tcc_hdmi_edid_i2c  :  no platform data. \n");
		return -EINVAL;
	}

	chip = kzalloc(sizeof(struct tcc_hdmi_i2c_chip_info), GFP_KERNEL);
	if(chip == NULL)
	{
		printk("\n tcc_hdmi_edid_i2c  :  no chip info. \n");
		return -ENOMEM;
	}

	chip->client = client;
	i2c_set_clientdata(client, chip);
	tcc_hdmi_edid_i2c_client = client;

	return 0;
}

static int tcc_hdmi_edid_i2c_remove(struct i2c_client *client)
{
	struct tcc_hdmi_edid_i2c_platform_data 	*pdata = client->dev.platform_data;
	struct tcc_hdmi_i2c_chip_info 			*chip  = i2c_get_clientdata(client);
	int ret = 0;

	if(pdata->teardown)
	{
		ret = pdata->teardown(client, chip->gpio_chip.base, chip->gpio_chip.ngpio, pdata->context);
		if(ret < 0)
		{
			dev_err(&client->dev, "%s failed, %d\n", "teardown", ret);
			printk("\n tcc_hdmi_edid_ii2c  :  teardown error. \n");
			return ret;
		}
	}

	kfree(chip);
	tcc_hdmi_edid_i2c_client = NULL;
	
	return 0;
}

static struct i2c_driver tcc_hdmi_edid_i2c_driver = {
	.driver = {
		.name	= "tcc-hdmi-edid",
	},
	.probe		= tcc_hdmi_edid_i2c_probe,
	.remove		= tcc_hdmi_edid_i2c_remove,
	.id_table	= tcc_hdmi_edid_i2c_id,
};

static int tcc_hdmi_edid_seg_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct tcc_hdmi_edid_seg_i2c_platform_data 	*pdata;
	struct tcc_hdmi_i2c_chip_info 				*chip;

	pdata = client->dev.platform_data;
	if(pdata == NULL)
	{
		dev_dbg(&client->dev, "no platform data\n");
		printk("\n tcc_hdmi_edid_i2c  :  no platform data. \n");
		return -EINVAL;
	}

	chip = kzalloc(sizeof(struct tcc_hdmi_i2c_chip_info), GFP_KERNEL);
	if(chip == NULL)
	{
		printk("\n tcc_hdmi_edid_i2c  :  no chip info. \n");
		return -ENOMEM;
	}

	chip->client = client;
	i2c_set_clientdata(client, chip);
	tcc_hdmi_edid_seg_i2c_client = client;

	return 0;
}

static int tcc_hdmi_edid_seg_i2c_remove(struct i2c_client *client)
{
	struct tcc_hdmi_edid_seg_i2c_platform_data 	*pdata = client->dev.platform_data;
	struct tcc_hdmi_i2c_chip_info 				*chip  = i2c_get_clientdata(client);
	int ret = 0;

	if(pdata->teardown)
	{
		ret = pdata->teardown(client, chip->gpio_chip.base, chip->gpio_chip.ngpio, pdata->context);
		if(ret < 0)
		{
			dev_err(&client->dev, "%s failed, %d\n", "teardown", ret);
			printk("\n tcc_hdmi_edid_ii2c  :  teardown error. \n");
			return ret;
		}
	}

	kfree(chip);
	tcc_hdmi_edid_seg_i2c_client = NULL;
	
	return 0;
}

static struct i2c_driver tcc_hdmi_edid_seg_i2c_driver = {
	.driver = {
		.name	= "tcc-hdmi-edid-seg",
	},
	.probe		= tcc_hdmi_edid_seg_i2c_probe,
	.remove		= tcc_hdmi_edid_seg_i2c_remove,
	.id_table	= tcc_hdmi_edid_seg_i2c_id,
};
#else // defined(CONFIG_ARCH_TCC93XX)
static int tcc_hdmi_phy_i2c_probe(struct i2c_adapter *adap, int addr, int kind)
{
	struct i2c_client *i2c;

	i2c = kzalloc(sizeof(*i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	strcpy(i2c->name, "HDMIPHY");
	i2c->flags = 0;
	i2c->addr = addr;
	i2c->adapter = adap;
	i2c->driver = &tcc_hdmi_phy_i2c_driver;

	tcc_hdmi_phy_i2c_client = i2c;

	return i2c_attach_client(i2c);
}

static int tcc_hdmi_phy_i2c_detach(struct i2c_client *client)
{
	i2c_detach_client(client);
	kfree(tcc_hdmi_phy_i2c_client);
	return 0;
}

static int tcc_hdmi_phy_i2c_attach(struct i2c_adapter *adap)
{
	return i2c_probe(adap, &addr_data, tcc_hdmi_phy_i2c_probe);
}

static struct i2c_driver tcc_hdmi_phy_i2c_driver = {
	.driver = {
		.name = "HDMIPHY",
		.owner = THIS_MODULE,
	},
	.id = 1,
	.attach_adapter = tcc_hdmi_phy_i2c_attach,
	.detach_client = tcc_hdmi_phy_i2c_detach,
};
#endif // defined(CONFIG_ARCH_TCC93XX)

#if defined(CONFIG_ARCH_TCC93XX)
int tcc_hdmi_edid_i2c_read_eddc(unsigned char segment, unsigned char offset, unsigned int size, unsigned char *buffer)
{
	int ret;
	struct i2c_adapter *adap;
    struct i2c_msg msgs[2];
    
    /* set segment pointer */
    msgs[0].addr  = tcc_hdmi_edid_seg_i2c_client->addr;
    msgs[0].flags = 0;
    msgs[0].len   = 1;
    msgs[0].buf   = &segment;

	adap = tcc_hdmi_edid_seg_i2c_client->adapter;
	ret = i2c_transfer(adap, &msgs, 1);

	printk("%s, addr=0x%02x, ret=%d\n", __func__, msgs[0].addr, ret);

    /* set offset */
    msgs[0].addr  = tcc_hdmi_edid_i2c_client->addr;
    msgs[0].flags = 0;
    msgs[0].len   = 1;
    msgs[0].buf   = &offset;

    /* read data */
    msgs[1].addr  = tcc_hdmi_edid_i2c_client->addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len   = size;
    msgs[1].buf   = buffer;

	adap = tcc_hdmi_edid_i2c_client->adapter;
	ret = i2c_transfer(adap, &msgs, 2);

	printk("%s, addr=0x%02x, ret=%d\n", __func__, msgs[0].addr, ret);

	return 1;
}

int tcc_hdmi_edid_i2c_read_block(unsigned int block_num, unsigned char *buffer)
{
    int segNum, offset;

    /* calculate */
    segNum = block_num / 2;
    offset = (block_num % 2) * 0x80; // 0x80(=128): size fo EDID block

    /* read block */
    if(!tcc_hdmi_edid_i2c_read_eddc(segNum, offset, 0x80, buffer))
    {
        printk("Fail to Read %dth EDID Block \n", block_num);
        return 0;
    }

	return 1;
}
#endif

int tcc_hdmi_phy_i2c_write(unsigned char reg, unsigned char *val, unsigned short len)
{
	unsigned char data[132];
	unsigned int cnt;
	unsigned char bytes;

	if (tcc_hdmi_phy_i2c_client == NULL)
		return ENODEV;

	data[0] = reg;
	bytes = 1;
	
	for(cnt = 0; cnt<len; cnt++)
	{
		data[bytes + cnt] = val[cnt];
	}

	bytes += len;

	if(i2c_master_send(tcc_hdmi_phy_i2c_client, data, bytes) != bytes)
	{
		printk("write error!!!! \n");
		return -EIO; 
	}
	
	return 0;
}

int tcc_hdmi_phy_i2c_read(unsigned char reg, unsigned char *val, unsigned short len)
{
	unsigned char bytes;
	
	if (tcc_hdmi_phy_i2c_client == NULL)
		return ENODEV;

	bytes = 1;
	if (i2c_master_send(tcc_hdmi_phy_i2c_client, &reg, bytes) != bytes)
	{
		printk("read(w) error!!!! \n");
		return -EIO;
	}

	bytes = len;
	if (i2c_master_recv(tcc_hdmi_phy_i2c_client, val, bytes) != bytes)
	{
		printk("read error!!!! \n");
		return -EIO;    
	}

	return 0;
}

int tcc_hdmi_phy_config(const enum PixelFreq clk, const enum ColorDepth depth)
{
    int index, freq;
    int size;
    unsigned char *buffer;
    unsigned char reg;

	DPRINTF("%s\n", __func__);

    // get depth index
    switch (depth)
    {
        case HDMI_CD_24:
            index = 0;
            break;
        case HDMI_CD_30:
            index = 1;
            break;
        case HDMI_CD_36:
            index = 2;
            break;
        default:
            printk("not available depth arg = %d\n", (int)depth);
            return 0;
    }

    // get clk freq index
    switch (clk)
    {
        case PIXEL_FREQ_25_200 :
            freq = 0;
            break;
        case PIXEL_FREQ_25_175 :
            freq = 1;
            break;
        case PIXEL_FREQ_27 :
            freq = 2;
            break;
        case PIXEL_FREQ_27_027 :
            freq = 3;
            break;
        case PIXEL_FREQ_54 :
            freq = 4;
            break;
        case PIXEL_FREQ_54_054 :
            freq = 5;
            break;
        case PIXEL_FREQ_74_250 :
            freq = 6;
            break;
        case PIXEL_FREQ_74_176 :
            freq = 7;
            break;
        case PIXEL_FREQ_148_500 :
            freq = 8;
            break;
        case PIXEL_FREQ_148_352 :
            freq = 9;
            break;
        case PIXEL_FREQ_108_108 :
            freq = 10;
            break;
        case PIXEL_FREQ_72 :
            freq = 11;
            break;
        case PIXEL_FREQ_25 :
            freq = 12;
            break;
        case PIXEL_FREQ_65 :
            freq = 13;
            break;
        case PIXEL_FREQ_108 :
            freq = 14;
            break;
        case PIXEL_FREQ_162 :
            freq = 15;
            break;
        default:
            printk("not availlable clk arg = %d\n",(int)clk);
            return 0;
    }

    // start to reconfig after that phy_ready goes down
    reg = 0x00;

	#if 0	
    if (!PHYWrite(PHY_I2C_ADDRESS,PHY_REG_MODE_SET_DONE, 1, &reg))
    {
        DPRINTF("fail to write reconfig 0x02%x\n",PHY_REG_MODE_SET_DONE);
        return 0;
    }
	#else
	tcc_hdmi_phy_i2c_write(PHY_REG_MODE_SET_DONE, &reg, 1);
	#endif

	{volatile int ttt;for(ttt=0;ttt<500;ttt++);}

    size = sizeof(phy_config[freq][index]) / sizeof(phy_config[freq][index][0]);
    buffer = (unsigned char *) phy_config[freq][index];

	#if 0
    if (!PHYWrite(PHY_I2C_ADDRESS, PHY_CONFIG_START_OFFSET, size, buffer))
    {
        return 0;
    }
    #endif

	tcc_hdmi_phy_i2c_write(PHY_CONFIG_START_OFFSET, buffer, size);

    //for ( idx = 0 ; idx < 1000 ; idx ++ );            // delay

    return 1;
}

int tcc_hdmi_set_phy_config(const struct HDMIVideoParameter * const hdmi_video_mode)
{
    unsigned char phy_ready = 0;
    int i = 0;

	DPRINTF("%s\n", __func__);

	phy_ready = readb(HDMI_PHY_STATUS);

    if (!phy_ready)
		hdmi_phy_reset();

    if (!tcc_hdmi_phy_config(HDMIVideoParams[hdmi_video_mode->resolution].PixelClock, hdmi_video_mode->colorDepth))
    {
        DPRINTF("phy config failed!\n");
        return 0;
    }

    do {
		phy_ready = readb(HDMI_PHY_STATUS);
        if (i++ == 1000) break;
    } while (!phy_ready);

    if (!phy_ready)
    {
        DPRINTF("phy is not ready!!!\n");
    }
    else
    {
        DPRINTF("phy configured\n");
    }

    return 1;
}

int tcc_hdmi_set_lcdc_config(enum VideoFormat hdmi_video_format)
{
	PDDICONFIG pDDICfg = (PDDICONFIG)tcc_p2v(HwDDI_CONFIG_BASE);
    struct lcdc_timimg_parms_t device;

	DPRINTF("%s\n", __func__);

    memcpy((void*)&device,(const void*)&(LCDCTimimgParams[hdmi_video_format]),sizeof(device));

	if(pDDICfg->HDMI_CTRL & HwDDIC_HDMI_CTRL_SEL_LCDC1)
		tcc92xxfb_hdmi_starter(1, &device);
	else
		tcc92xxfb_hdmi_starter(0, &device);

    return 1;
}

int tcc_hdmi_set_video_mode(const struct HDMIVideoParameter * const hdmi_video_mode)
{
    struct device_video_params device;
	enum PixelLimit pxl_lmt = HDMI_FULL_RANGE;

	DPRINTF("%s\n", __func__);

    if (!hdmi_video_mode)
    {
        DPRINTF("bad args: hdmi_video_mode\n");
        return 0;
    }

    // parsing video parameters
    memcpy((void*)&device,(const void*)&(HDMIVideoParams[hdmi_video_mode->resolution]),sizeof(device));

	// set lcdc
	tcc_hdmi_set_lcdc_config(hdmi_video_mode->resolution);
	
    // set pixel aspect ratio
    // !! must be setting before 'HDMI_IOC_SET_VIDEOMODE'
    hdmi_set_pixel_aspect_ratio(hdmi_video_mode->pixelAspectRatio);

    // set video parameters
	hdmi_set_video_mode(device);

    // set color space
    if ( !hdmi_set_color_space(hdmi_video_mode->colorSpace) )
    {
        DPRINTF("bad args: hdmi_video_mode->colorSpace : Not Correct Arg = %d\n", hdmi_video_mode->colorSpace);
    }

    // set color depth
    if ( !hdmi_set_color_depth(hdmi_video_mode->colorDepth) )
    {
        DPRINTF("bad args: hdmi_video_mode->colorDepth : Not Correct Arg = %d\n", hdmi_video_mode->colorDepth);
    }

	// set pixel limitation.
	switch(hdmi_video_mode->colorSpace) 
	{
		case HDMI_CS_RGB:		/** RGB color space */
			if (hdmi_video_mode->resolution == v640x480p_60Hz)
				pxl_lmt = HDMI_FULL_RANGE;
			else
				pxl_lmt = HDMI_RGB_LIMIT_RANGE;
			break;

		case HDMI_CS_YCBCR444:	/** YCbCr 4:4:4 color space */
		case HDMI_CS_YCBCR422:	/** YCbCr 4:2:2 color space */
			pxl_lmt = HDMI_YCBCR_LIMIT_RANGE;
			break;
	}

    if (!hdmi_set_pixel_limit(pxl_lmt))
    {
		DPRINTF("bad args: hdmi_video_mode->colorDepth : Not Correct Arg = %d\n", pxl_lmt);
    }

    // set phy
    if (!tcc_hdmi_set_phy_config(hdmi_video_mode))
    {
        DPRINTF("fail to config PHY!\n");
    }

    return 1;
}

static int tcc_hdmi_detect_cable(void)
{
	PGPIO pGPIO = (PGPIO)tcc_p2v(HwGPIO_BASE);

	DPRINTF("%s\n", __func__);

	BITCLR(pGPIO->GPAEN, Hw14);
	BITCLR(pGPIO->GPAFN1, Hw28-Hw24);
	{volatile int ttt;for(ttt=0;ttt<500;ttt++);}

	if(ISSET(pGPIO->GPADAT, Hw14))
	{
		DPRINTF("HDMI cable is detected!!\n");
		return true;
	}
	else
	{
		DPRINTF("HDMI cable is not detected!!\n");
		return false;
	}
}

static int tcc_composite_detect_cable(void)
{
	int ret = true;

	#if defined (CONFIG_MACH_TCC9300ST) || defined (CONFIG_MACH_TCC8800ST) 
		#if defined(CONFIG_TCC_OUTPUT_AUTO_DETECTION)
			tcc_gpio_config(GPIO_OUTPUT_COMPOSITE_DETECT, GPIO_FN(0));
			gpio_request(GPIO_OUTPUT_COMPOSITE_DETECT, NULL);
			gpio_direction_input(GPIO_OUTPUT_COMPOSITE_DETECT);
			ret = gpio_get_value(GPIO_OUTPUT_COMPOSITE_DETECT)? false : true;
		#endif
	#endif

	return ret;
}

static int tcc_component_detect_cable(void)
{
	int ret = true;

	#if defined (CONFIG_MACH_TCC9300ST) || defined (CONFIG_MACH_TCC8800ST)
		#if defined(CONFIG_TCC_OUTPUT_AUTO_DETECTION)
			tcc_gpio_config(GPIO_OUTPUT_COMPONENT_DETECT, GPIO_FN(0));
			gpio_request(GPIO_OUTPUT_COMPONENT_DETECT, NULL);
			gpio_direction_input(GPIO_OUTPUT_COMPONENT_DETECT);
			ret = gpio_get_value(GPIO_OUTPUT_COMPONENT_DETECT)? false : true;
		#endif
	#endif

	return ret;
}

void tcc_output_starter_setport(char lcdctrl_num, unsigned bit_per_pixel)
{
	char LCD_PORT_ENABLE;
	PGPION	pGPIO_C = (volatile PGPION)tcc_p2v(HwGPIOC_BASE);
	
	if(lcdctrl_num)
		LCD_PORT_ENABLE = 5;
	else
		LCD_PORT_ENABLE = 2;

	BITCSET(pGPIO_C->GPFN3, HwPORTCFG_GPFN0_MASK | HwPORTCFG_GPFN1_MASK | HwPORTCFG_GPFN2_MASK | HwPORTCFG_GPFN3_MASK,
							HwPORTCFG_GPFN0(LCD_PORT_ENABLE) | HwPORTCFG_GPFN1(LCD_PORT_ENABLE) | HwPORTCFG_GPFN2(LCD_PORT_ENABLE) | 
							HwPORTCFG_GPFN3(LCD_PORT_ENABLE));

	switch (bit_per_pixel) {
		case 24:
			BITCSET(pGPIO_C->GPFN2, HwPORTCFG_GPFN2_MASK | HwPORTCFG_GPFN3_MASK | HwPORTCFG_GPFN4_MASK | HwPORTCFG_GPFN5_MASK |
									HwPORTCFG_GPFN6_MASK | HwPORTCFG_GPFN7_MASK,
									HwPORTCFG_GPFN2(LCD_PORT_ENABLE) | HwPORTCFG_GPFN3(LCD_PORT_ENABLE) | HwPORTCFG_GPFN4(LCD_PORT_ENABLE) | 
									HwPORTCFG_GPFN5(LCD_PORT_ENABLE) | HwPORTCFG_GPFN6(LCD_PORT_ENABLE) | HwPORTCFG_GPFN7(LCD_PORT_ENABLE));
		case 18:
			BITCSET(pGPIO_C->GPFN2, HwPORTCFG_GPFN0_MASK | HwPORTCFG_GPFN1_MASK,
									HwPORTCFG_GPFN0(LCD_PORT_ENABLE) | HwPORTCFG_GPFN1(LCD_PORT_ENABLE));

		case 16:
			BITCSET(pGPIO_C->GPFN1, HwPORTCFG_GPFN0_MASK | HwPORTCFG_GPFN1_MASK | HwPORTCFG_GPFN2_MASK | HwPORTCFG_GPFN3_MASK | 
									HwPORTCFG_GPFN4_MASK | HwPORTCFG_GPFN5_MASK | HwPORTCFG_GPFN6_MASK | HwPORTCFG_GPFN7_MASK,
									HwPORTCFG_GPFN0(LCD_PORT_ENABLE) | HwPORTCFG_GPFN1(LCD_PORT_ENABLE) | HwPORTCFG_GPFN2(LCD_PORT_ENABLE) | 
									HwPORTCFG_GPFN3(LCD_PORT_ENABLE) | HwPORTCFG_GPFN4(LCD_PORT_ENABLE) | HwPORTCFG_GPFN5(LCD_PORT_ENABLE) | 
									HwPORTCFG_GPFN6(LCD_PORT_ENABLE) | HwPORTCFG_GPFN7(LCD_PORT_ENABLE));
		case 8:
			BITCSET(pGPIO_C->GPFN0, HwPORTCFG_GPFN0_MASK | HwPORTCFG_GPFN1_MASK | HwPORTCFG_GPFN2_MASK | HwPORTCFG_GPFN3_MASK | 
									HwPORTCFG_GPFN4_MASK | HwPORTCFG_GPFN5_MASK | HwPORTCFG_GPFN6_MASK | HwPORTCFG_GPFN7_MASK,
									HwPORTCFG_GPFN0(LCD_PORT_ENABLE) | HwPORTCFG_GPFN1(LCD_PORT_ENABLE) | HwPORTCFG_GPFN2(LCD_PORT_ENABLE) | 
									HwPORTCFG_GPFN3(LCD_PORT_ENABLE) | HwPORTCFG_GPFN4(LCD_PORT_ENABLE) | HwPORTCFG_GPFN5(LCD_PORT_ENABLE) | 
									HwPORTCFG_GPFN6(LCD_PORT_ENABLE) | HwPORTCFG_GPFN7(LCD_PORT_ENABLE));
			break;
			
		default:
			// do nothing
			break;
	}

}

void tcc_output_starter_hdmi(unsigned char lcdc_num, unsigned char hdmi_resolution)
{
	PDDICONFIG pDDICfg = (PDDICONFIG)tcc_p2v(HwDDI_CONFIG_BASE);
    struct device_video_params device;
	struct clk *lcdc_clk;
	
	PLCDC 			pLCDC_IMG = (PLCDC)tcc_p2v(HwLCDC1_BASE);
	PLCDC_CHANNEL 	pLCDC_IMG_CH = (PLCDC_CHANNEL)&pLCDC_IMG->LI2C;
	PLCDC 			pLCDC = (PLCDC)tcc_p2v(HwLCDC0_BASE);
	PLCDC_CHANNEL 	pLCDC_CH = (PLCDC_CHANNEL)&pLCDC->LI0C;
	unsigned int img_addr, img_width, img_height, img_offset;

	if(lcdc_num)
	{
		pLCDC = (PLCDC)tcc_p2v(HwLCDC1_BASE);
		pLCDC_CH = (PLCDC_CHANNEL)&pLCDC->LI2C;
	}

	img_addr 	= pLCDC_IMG_CH->LIBA0;
	img_width 	= (pLCDC_IMG_CH->LIS & 0x00001FFF);
	img_height 	= (pLCDC_IMG_CH->LIS & 0x1FFF0000)>>16;
	img_offset	= pLCDC_IMG_CH->LIO;

	DPRINTF("%s\n", __func__);

	BITSET(pDDICfg->SWRESET, Hw8);	// HDMI
	BITCLR(pDDICfg->SWRESET, Hw8);	// HDMI	

	if(lcdc_num)
	{
		BITSET(pDDICfg->SWRESET, Hw3);	// LCDC1
		BITCLR(pDDICfg->SWRESET, Hw3);	// LCDC1	
		
		lcdc_clk = clk_get(0, "lcdc1");
		clk_enable(lcdc_clk);
	}
	else
	{
		BITSET(pDDICfg->SWRESET, Hw2);	// LCDC0
		BITCLR(pDDICfg->SWRESET, Hw2);	// LCDC0	
		
		lcdc_clk = clk_get(0, "lcdc0");
		clk_enable(lcdc_clk);
	}
	
	tcc_hdmi_power_on();

	if(hdmi_resolution > SUPPORT_HDMI_MODE_NUM)
		hdmi_resolution = STARTER_HDMI_1920x1080P;		

	gHdmiVideoParms.mode = HDMI;
	gHdmiVideoParms.resolution = TccSupportHdmiMode[hdmi_resolution];
	gHdmiVideoParms.colorSpace = HDMI_CS_RGB;
	gHdmiVideoParms.colorDepth = HDMI_CD_24;
	gHdmiVideoParms.colorimetry = HDMI_COLORIMETRY_NO_DATA;
	gHdmiVideoParms.pixelAspectRatio = HDMI_PIXEL_RATIO_16_9;
	
	/* video mode HDMI/DVI */
	hdmi_set_hdmimode(gHdmiVideoParms.mode);

	tcc_hdmi_set_video_mode(&gHdmiVideoParms);

	#if 0
	if (gHdmiVideoParms.mode == HDMI)
		hdmi_set_audio_mode(&audio);
	#endif

	hdmi_start();

	#if defined(CONFIG_ARCH_TCC92XX)
		pLCDC_CH->LIC 	= Hw28 | 10*Hw0;
		pLCDC_CH->LIP 	= (0 << 16) | 0;
		pLCDC_CH->LIS 	= (img_height << 16) | img_width;
		pLCDC_CH->LIBA0 = img_addr;
		pLCDC_CH->LIO 	= img_offset;
		pLCDC_CH->LISR 	= (0<<4) | 0;
	#endif
}

void tcc_output_starter_composite(unsigned char lcdc_num, unsigned char type)
{
#ifdef CONFIG_FB_TCC_COMPOSITE
	unsigned int lcd_reg = 0;
	unsigned int img_addr, img_width, img_height, img_offset;
	COMPOSITE_SPEC_TYPE composite_spec;
//	SCALER_TYPE fbscaler;
	struct clk *lcdc_clk, *dac_clk;

	PLCDC 			pLCDC_IMG = (PLCDC)tcc_p2v(HwLCDC1_BASE);
	PLCDC_CHANNEL 	pLCDC_IMG_CH = (PLCDC_CHANNEL)&pLCDC_IMG->LI2C;
	PLCDC 			pLCDC = (PLCDC)tcc_p2v(HwLCDC0_BASE);
	PLCDC_CHANNEL 	pLCDC_CH = (PLCDC_CHANNEL)&pLCDC->LI0C;
	PDDICONFIG 		pDDICfg = (PDDICONFIG)tcc_p2v(HwDDI_CONFIG_BASE);
	PNTSCPAL 		pTVE = (PNTSCPAL)tcc_p2v(HwTVE_BASE);
	PNTSCPALOP 		pTVE_VEN = (PNTSCPALOP)tcc_p2v(HwTVE_VEN_BASE);

	DPRINTF("%s, lcdc_num=%d, type=%d\n", __func__, lcdc_num, type);

	img_addr 	= pLCDC_IMG_CH->LIBA0;
	img_width 	= (pLCDC_IMG_CH->LIS & 0x00001FFF);
	img_height 	= (pLCDC_IMG_CH->LIS & 0x1FFF0000)>>16;
	img_offset	= pLCDC_IMG_CH->LIO;

	if(lcdc_num)
	{
		pLCDC = (PLCDC)tcc_p2v(HwLCDC1_BASE);
		pLCDC_CH = (PLCDC_CHANNEL)&pLCDC->LI2C;

		BITSET(pDDICfg->SWRESET, Hw3);	// LCDC1
		BITCLR(pDDICfg->SWRESET, Hw3);	// LCDC1	
		BITSET(pDDICfg->NTSCPAL_SEL, Hw0); // LCDC1
		#if defined(CONFIG_ARCH_TCC93XX)
			tca_ckc_setperi(PERI_LCD1, ENABLE, 270000, PCDIRECTPLL2);
		#else
			tca_ckc_setperi(PERI_LCD1, ENABLE, 270000, PCDIRECTPLL0);
		#endif
		
		lcdc_clk = clk_get(0, "lcdc1");
		clk_enable(lcdc_clk);
		dac_clk = clk_get(0, "videodac");
		clk_enable(dac_clk);
	}
	else
	{
		pLCDC = (PLCDC)tcc_p2v(HwLCDC0_BASE);
		pLCDC_CH = (PLCDC_CHANNEL)&pLCDC->LI0C;

		BITSET(pDDICfg->SWRESET, Hw2);	// LCDC0
		BITCLR(pDDICfg->SWRESET, Hw2);	// LCDC0	
		BITCLR(pDDICfg->NTSCPAL_SEL, Hw0); // LCDC0
		#if defined(CONFIG_ARCH_TCC93XX)
			tca_ckc_setperi(PERI_LCD0, ENABLE, 270000, PCDIRECTPLL2);
		#else
			tca_ckc_setperi(PERI_LCD0, ENABLE, 270000, PCDIRECTPLL2);
		#endif
		
		lcdc_clk = clk_get(0, "lcdc0");
		clk_enable(lcdc_clk);
		dac_clk = clk_get(0, "videodac");
		clk_enable(dac_clk);
	}
	
	TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_COMPOSITE, lcdc_num, 1);
	tcc_composite_set_lcdc(lcdc_num);
	
	if(type == STARTER_COMPOSITE_NTSC)
		tcc_composite_get_spec(NTSC_M, &composite_spec);
	else
		tcc_composite_get_spec(PAL_B, &composite_spec);
	
	BITCSET(pLCDC->LCLKDIV, 0x01FF0000, (1 << 16));
	BITCSET(pLCDC->LCLKDIV, 0x000001FF, 0);
	BITCSET(pLCDC->LHTIME1, 0x0003FFFF, composite_spec.composite_LPC);
	BITCSET(pLCDC->LHTIME1, 0x01FF0000, (composite_spec.composite_LPW << 16));
	BITCSET(pLCDC->LHTIME2, 0x000001FF, composite_spec.composite_LEWC);
	BITCSET(pLCDC->LHTIME2, 0x01FF0000, (composite_spec.composite_LSWC << 16));
	BITCSET(pLCDC->LVTIME1, 0x00003FFF, composite_spec.composite_FLC1);
	BITCSET(pLCDC->LVTIME1, 0x003F0000, (composite_spec.composite_FPW1 << 16));
	BITCSET(pLCDC->LVTIME1, 0x03C00000, (composite_spec.composite_VDF << 22));
	BITCSET(pLCDC->LVTIME1, 0xF8000000, (composite_spec.composite_VDB << 27));
	BITCSET(pLCDC->LVTIME2, 0x01FF0000, (composite_spec.composite_FSWC1 << 16));
	BITCSET(pLCDC->LVTIME2, 0x000001FF, composite_spec.composite_FEWC1);
	BITCSET(pLCDC->LVTIME3, 0x00003FFF, composite_spec.composite_FLC2);
	BITCSET(pLCDC->LVTIME3, 0x003F0000, (composite_spec.composite_FPW2 << 16));
	BITCSET(pLCDC->LVTIME4, 0x01FF0000, (composite_spec.composite_FSWC2 << 16));
	BITCSET(pLCDC->LVTIME4, 0x000001FF, composite_spec.composite_FEWC2);
	BITCSET(pLCDC->LDS, 0x0000FFFF, composite_spec.composite_lcd_width);
	BITCSET(pLCDC->LDS, 0xFFFF0000, (composite_spec.composite_lcd_height << 16));

	BITCSET(lcd_reg, Hw29|Hw28, Hw29|Hw28);				// R2YMD - RGB to YCbCr Conversion Option
	BITCSET(lcd_reg, Hw23, Hw23);						// CKG - Clock Gating Enable for Timing Generator
	BITCSET(lcd_reg, Hw14, Hw14);						// IV - Inverted Vertical Sync
	BITCSET(lcd_reg, Hw13, Hw13);						// IH - Inverted Horizontal Sync
	BITCSET(lcd_reg, Hw12, Hw12);						// IP - Inverted Pixel Clock
	BITCSET(lcd_reg, Hw11, Hw11);						// CLEN - Clipping Enable
	BITCSET(lcd_reg, Hw10, Hw10);						// R2Y - RGB to YCbCr Converter Enable for OUPUT
	BITCSET(lcd_reg, Hw7, Hw7);							// TV - TV mode
	BITCSET(lcd_reg, Hw3|Hw2|Hw1, Hw3|Hw1);				// OVP
#if 0 // COMPOSITE_CCIR656
	BITCSET(lcd_reg, Hw24, Hw24);						// 656 - CCIR 656 Mode
	BITCSET(lcd_reg, Hw19|Hw18|Hw17|Hw16, Hw19);		// PXDW - Pixel Data Width
	BITCSET(lcd_reg, Hw9, Hw9);							// DP - Double Pixel Data
#else
	BITCSET(lcd_reg, Hw19|Hw18|Hw17|Hw16, Hw18|Hw17);	// PXDW - Pixel Data Width
#endif

	BITCSET(pLCDC->LCTRL, 0xFFFFFFFF, lcd_reg);

	/* LCD Interrupt mask */
	pLCDC->LIM &= 0xFFFFFFE6;
	pLCDC->LIM |= ((1 << 4) | 1);

	#if defined(CONFIG_ARCH_TCC92XX)
		pLCDC_CH->LIP 	= (0 << 16) | 0;
		pLCDC_CH->LIS 	= (composite_spec.composite_lcd_height << 16) | composite_spec.composite_lcd_width;
		pLCDC_CH->LIBA0 = img_addr;
		pLCDC_CH->LIO 	= img_offset;
		pLCDC_CH->LISR 	= (0<<4) | 0;
	#endif

	BITCSET(pLCDC_CH->LIKR, 0xff <<  0, (0)    <<  0); // key
	BITCSET(pLCDC_CH->LIKR, 0xff << 16, (0xF8)	<< 16); // keymask
	BITCSET(pLCDC_CH->LIKG, 0xff <<  0, (0)    <<  0); // key
	BITCSET(pLCDC_CH->LIKG, 0xff << 16, (0xFC)	<< 16); // keymask
	BITCSET(pLCDC_CH->LIKB, 0xff <<  0, (0)    <<  0); // key
	BITCSET(pLCDC_CH->LIKB, 0xff << 16, (0xF8)	<< 16); // keymask

	#if defined(CONFIG_ARCH_TCC92XX)
		pLCDC_CH->LIC = Hw31 | Hw29 | Hw28 | Hw16 | 10*Hw0;
	#else
		pLCDC_CH->LIC = Hw31 | Hw29 | Hw16 | 10*Hw0;
	#endif

	//Disconnect LCDC with NTSC/PAL encoder
	BITCLR(pTVE_VEN->VENCON, HwTVEVENCON_EN_EN);
		
	//Set ECMDA Register
	if(type == STARTER_COMPOSITE_NTSC)
	{
		// NTSC_M
		pTVE->ECMDA  = 
			HwTVECMDA_PWDENC_PD 			|	// [7]	 Power down mode for entire digital logic of TV encoder
			HwTVECMDA_FDRST_1				|	// [6]	 Chroma is free running as compared to H-sync
			HwTVECMDA_FSCSEL_NTSC			|	// [5:4] Color subcarrier frequency is 3.57954545 MHz for NTSC
			HwTVECMDA_PEDESTAL				|	// [3]	 Video Output has a pedestal (0 is NTSC-J)
			HwTVECMDA_PIXEL_601 			|	// [2]	 Input data is at 601 rates.
			HwTVECMDA_IFMT_525				|	// [1]	 Output data has 525 lines
			HwTVECMDA_PHALT_NTSC			|	// [0]	 NTSC encoded chroma signal output
			0;
	}
	else
	{
		// PAL_B,G,H,I
		pTVE->ECMDA  = 
			HwTVECMDA_PWDENC_PD 			|	// [7]	 Power down mode for entire digital logic of TV encoder
			HwTVECMDA_FDRST_1				|	// [6]	 Chroma is free running as compared to H-sync
			HwTVECMDA_FSCSEL_PALX			|	// [5:4] Color subcarrier frequency is 4.43361875 MHz for PAL-B,D,G,H,I,N
			HwTVECMDA_PIXEL_601 			|	// [2]	 Input data is at 601 rates.
			HwTVECMDA_IFMT_625				|	// [1]	 Output data has 625 lines
			HwTVECMDA_PHALT_PAL 			|	// [0]	 PAL encoded chroma signal output
			0;
	}
	
	//Set DACSEL Register
	BITSET(pTVE->DACSEL, HwTVEDACSEL_DACSEL_CVBS);
	//Set DACPD Register
	BITSET(pTVE->DACPD, HwTVEDACPD_PD_EN);

	BITSET(pTVE->ICNTL, HwTVEICNTL_VSIP_HIGH);
	BITSET(pTVE->ICNTL, HwTVEICNTL_HSVSP_RISING);
	#if 0 // COMPOSITE_CCIR656
	BITCSET(pTVE->ICNTL, HwTVEICNTL_ISYNC_MASK, HwTVEICNTL_ISYNC_ESAV_F);
	#else
	BITCSET(pTVE->ICNTL, HwTVEICNTL_ISYNC_MASK, HwTVEICNTL_ISYNC_HVSI);
	#endif
		
	//Set the Vertical Offset
	BITCSET(pTVE->HVOFFST, 0x07, ((0 & 0x700)>>8));
	pTVE->HOFFST = (0 & 0xFF);
			
	//Set the Horizontal Offset
	BITCSET(pTVE->HVOFFST, 0x08, ((1 & 0x100)>>5));
	pTVE->VOFFST = (1 & 0xFF);
			
	//Set the Digital Output Format
	BITCSET(pTVE->HVOFFST, HwTVEHVOFFST_INSEL_MASK, HwTVEHVOFFST_INSEL(2));
			
	//Set HSVSO Register
	BITCSET(pTVE->HSVSO, 0x07, ((0 & 0x700)>>8));
	pTVE->HSOE = (0 & 0xFF);
	BITCSET(pTVE->HSVSO, 0x38, ((0 & 0x700)>>5));
	pTVE->HSOB = (0 & 0xFF);
	BITCSET(pTVE->HSVSO, 0x40, ((0 & 0x100)>>2));
	pTVE->VSOB = (0 & 0xFF);

	//Set VSOE Register
	BITCSET(pTVE->VSOE, 0x1F, (0 & 0x1F));
	BITCSET(pTVE->VSOE, 0xC0, (0 & 0x03)<<6);
	BITCSET(pTVE->VSOE, 0x20, (0 & 0x01)<<5);
			
	//Set the Connection Type
	BITSET(pTVE_VEN->VENCIF, HwTVEVENCIF_FMT_1);

	BITSET(pTVE_VEN->VENCON, HwTVEVENCON_EN_EN);
	BITCLR(pTVE->DACPD, HwTVEDACPD_PD_EN);
	BITCLR(pTVE->ECMDA, HwTVECMDA_PWDENC_PD);

	/* Enable LCDC */
	BITSET(pLCDC->LCTRL, Hw0);
#endif
}

void tcc_output_starter_component(unsigned char lcdc_num, unsigned char type)
{
#ifdef CONFIG_FB_TCC_COMPONENT
	unsigned int lcd_reg = 0;
	unsigned int img_addr, img_width, img_height, img_offset;
	COMPONENT_SPEC_TYPE component_spec;
//	SCALER_TYPE fbscaler;
	struct clk *lcdc_clk;

	PLCDC 			pLCDC_IMG = (PLCDC)tcc_p2v(HwLCDC1_BASE);
	PLCDC_CHANNEL 	pLCDC_IMG_CH = (PLCDC_CHANNEL)&pLCDC_IMG->LI2C;
	PLCDC 			pLCDC = (PLCDC)tcc_p2v(HwLCDC1_BASE);
	PLCDC_CHANNEL 	pLCDC_CH = (PLCDC_CHANNEL)&pLCDC->LI2C;
	PDDICONFIG 		pDDICfg = (PDDICONFIG)tcc_p2v(HwDDI_CONFIG_BASE);
	PGPIO 			pGPIO = (volatile PGPIO)tcc_p2v(HwGPIO_BASE);

	DPRINTF("%s, lcdc_num=%d, type=%d\n", __func__, lcdc_num, type);

	img_addr 	= pLCDC_IMG_CH->LIBA0;
	img_width 	= (pLCDC_IMG_CH->LIS & 0x00001FFF);
	img_height 	= (pLCDC_IMG_CH->LIS & 0x1FFF0000)>>16;
	img_offset	= pLCDC_IMG_CH->LIO;

	switch(type)
	{
		case STARTER_COMPONENT_480I_NTSC:
			tcc_component_get_spec(COMPONENT_MODE_NTSC_M, &component_spec);
			break;
		case STARTER_COMPONENT_576I_PAL:
			tcc_component_get_spec(COMPONENT_MODE_PAL_B, &component_spec);
			break;
		case STARTER_COMPONENT_720P:
			tcc_component_get_spec(COMPONENT_MODE_720P, &component_spec);
			break;
		case STARTER_COMPONENT_1080I:
			tcc_component_get_spec(COMPONENT_MODE_1080I, &component_spec);
			break;
	}
	
	if(lcdc_num)
	{
		pLCDC = (PLCDC)tcc_p2v(HwLCDC1_BASE);
		pLCDC_CH = (PLCDC_CHANNEL)&pLCDC->LI2C;

		BITSET(pDDICfg->SWRESET, Hw3);	// LCDC1
		BITCLR(pDDICfg->SWRESET, Hw3);	// LCDC1
		#if defined(CONFIG_ARCH_TCC93XX)
			tca_ckc_setperi(PERI_LCD1, ENABLE, component_spec.component_clk, PCDIRECTPLL2);
		#else
			tca_ckc_setperi(PERI_LCD1, ENABLE, component_spec.component_clk, PCDIRECTPLL0);
		#endif

		lcdc_clk = clk_get(0, "lcdc1");
		clk_enable(lcdc_clk);
	}
	else
	{
		pLCDC = (PLCDC)tcc_p2v(HwLCDC0_BASE);
		pLCDC_CH = (PLCDC_CHANNEL)&pLCDC->LI0C;

		BITSET(pDDICfg->SWRESET, Hw2);	// LCDC0
		BITCLR(pDDICfg->SWRESET, Hw2);	// LCDC0
		#if defined(CONFIG_ARCH_TCC93XX)
			tca_ckc_setperi(PERI_LCD0, ENABLE, component_spec.component_clk, PCDIRECTPLL2);
		#else
			tca_ckc_setperi(PERI_LCD0, ENABLE, component_spec.component_clk, PCDIRECTPLL0);
		#endif

		lcdc_clk = clk_get(0, "lcdc0");
		clk_enable(lcdc_clk);
	}
	
	DPRINTF("LCDC0 clk:%d, LCDC1 clk:%d, PLL%1d:%d\n", tca_ckc_getperi(PERI_LCD0), tca_ckc_getperi(PERI_LCD1), PCDIRECTPLL2, tca_ckc_getpll(PCDIRECTPLL2));
	
	tcc_output_starter_setport(lcdc_num, component_spec.component_bus_width);

	TCC_OUTPUT_LCDC_OnOff(TCC_OUTPUT_COMPONENT, lcdc_num, 1);
	tcc_component_set_lcdc(lcdc_num);

	BITCSET(pLCDC->LCLKDIV, 0x00FF0000, 0<<16);
	BITCSET(pLCDC->LCLKDIV, 0x000000FF, 0);
	BITCSET(pLCDC->LHTIME1, 0x0003FFFF, component_spec.component_LPC);
	BITCSET(pLCDC->LHTIME1, 0x01FF0000, (component_spec.component_LPW << 16));
	BITCSET(pLCDC->LHTIME2, 0x000001FF, component_spec.component_LEWC);
	BITCSET(pLCDC->LHTIME2, 0x01FF0000, (component_spec.component_LSWC << 16));
	BITCSET(pLCDC->LVTIME1, 0x00003FFF, component_spec.component_FLC1);
	BITCSET(pLCDC->LVTIME1, 0x003F0000, (component_spec.component_FPW1 << 16));
	BITCSET(pLCDC->LVTIME1, 0x03C00000, (component_spec.component_VDF << 22));
	BITCSET(pLCDC->LVTIME1, 0xF8000000, (component_spec.component_VDB << 27));
	BITCSET(pLCDC->LVTIME2, 0x01FF0000, (component_spec.component_FSWC1 << 16));
	BITCSET(pLCDC->LVTIME2, 0x000001FF, component_spec.component_FEWC1);
	BITCSET(pLCDC->LVTIME3, 0x00003FFF, component_spec.component_FLC2);
	BITCSET(pLCDC->LVTIME3, 0x003F0000, (component_spec.component_FPW2 << 16));
	BITCSET(pLCDC->LVTIME4, 0x01FF0000, (component_spec.component_FSWC2 << 16));
	BITCSET(pLCDC->LVTIME4, 0x000001FF, component_spec.component_FEWC2);
	BITCSET(pLCDC->LDS, 0x0000FFFF, component_spec.component_lcd_width);
	BITCSET(pLCDC->LDS, 0xFFFF0000, (component_spec.component_lcd_height << 16));

	switch(type)
	{
		case STARTER_COMPONENT_480I_NTSC:
		case STARTER_COMPONENT_576I_PAL:
			BITCSET(lcd_reg, Hw29|Hw28, Hw29|Hw28);				// R2YMD - RGB to YCbCr Conversion Option
			BITCSET(lcd_reg, Hw23, Hw23);						// CKG - Clock Gating Enable for Timing Generator
			BITCSET(lcd_reg, Hw15, Hw15);						// ID - Inverted Data Enable
			BITCSET(lcd_reg, Hw14, Hw14);						// IV - Inverted Vertical Sync
			BITCSET(lcd_reg, Hw13, Hw13);						// IH - Inverted Horizontal Sync
			BITCSET(lcd_reg, Hw12, Hw12);						// IP - Inverted Pixel Clock
			BITCSET(lcd_reg, Hw10, Hw10);						// R2Y - RGB to YCbCr Converter Enable for OUPUT
			BITCSET(lcd_reg, Hw7, Hw7);							// TV - TV mode
			BITCSET(lcd_reg, Hw3|Hw2|Hw1, Hw3|Hw1);				// OVP
			BITCSET(lcd_reg, Hw0, Hw0);							// LEN - LCD Controller Enable
			#if 0 // COMPONENT_CCIR656
			BITCSET(lcd_reg, Hw24, Hw24);						// 656 - CCIR 656 Mode
			BITCSET(lcd_reg, Hw19|Hw18|Hw17|Hw16, Hw19);		// PXDW - Pixel Data Width
			BITCSET(lcd_reg, Hw9, Hw9);							// DP - Double Pixel Data
			#else
			BITCSET(lcd_reg, Hw19|Hw18|Hw17|Hw16, Hw18|Hw17);	// PXDW - Pixel Data Width
			#endif
			break;

		case STARTER_COMPONENT_720P:
			BITCSET(lcd_reg, Hw29|Hw28, Hw29|Hw28);				// R2YMD - RGB to YCbCr Conversion Option
			BITCSET(lcd_reg, Hw23, Hw23);						// CKG - Clock Gating Enable for Timing Generator
			#if defined(CONFIG_MACH_TCC9300ST)
//			BITCSET(lcd_reg, Hw15, Hw15);						// ID - Inverted Data Enable
			BITCSET(lcd_reg, Hw14, Hw14);						// IV - Inverted Vertical Sync
			BITCSET(lcd_reg, Hw13, Hw13);						// IH - Inverted Horizontal Sync
//			BITCSET(lcd_reg, Hw12, Hw12);						// IP - Inverted Pixel Clock
			#else
			BITCSET(lcd_reg, Hw15, Hw15);						// ID - Inverted Data Enable
			BITCSET(lcd_reg, Hw14, Hw14);						// IV - Inverted Vertical Sync
//			BITCSET(lcd_reg, Hw13, Hw13);						// IH - Inverted Horizontal Sync
			BITCSET(lcd_reg, Hw12, Hw12);						// IP - Inverted Pixel Clock
			#endif
			BITCSET(lcd_reg, Hw8, Hw8);							// NI - Non-interlace Mode
			BITCSET(lcd_reg, Hw3|Hw2|Hw1, Hw3|Hw1);				// OVP
			BITCSET(lcd_reg, Hw0, Hw0);							// LEN - LCD Controller Enable
			#if 0 // COMPONENT_CCIR656
			BITCSET(lcd_reg, Hw10, Hw10);						// R2Y - RGB to YCbCr Converter Enable for OUPUT
			BITCSET(lcd_reg, Hw19|Hw18|Hw17|Hw16, Hw19);		// PXDW - Pixel Data Width
			#else
			BITCSET(lcd_reg, Hw19|Hw18|Hw17|Hw16, Hw19|Hw18);	// PXDW - Pixel Data Width
			#endif
			break;

		case STARTER_COMPONENT_1080I:
			BITCSET(lcd_reg, Hw29|Hw28, Hw29|Hw28);				// R2YMD - RGB to YCbCr Conversion Option
			BITCSET(lcd_reg, Hw23, Hw23);						// CKG - Clock Gating Enable for Timing Generator
			#if defined(CONFIG_MACH_TCC9300ST)
//			BITCSET(lcd_reg, Hw15, Hw15);						// ID - Inverted Data Enable
			BITCSET(lcd_reg, Hw14, Hw14);						// IV - Inverted Vertical Sync
			BITCSET(lcd_reg, Hw13, Hw13);						// IH - Inverted Horizontal Sync
//			BITCSET(lcd_reg, Hw12, Hw12);						// IP - Inverted Pixel Clock
			#else
			BITCSET(lcd_reg, Hw15, Hw15);						// ID - Inverted Data Enable
			BITCSET(lcd_reg, Hw14, Hw14);						// IV - Inverted Vertical Sync
//			BITCSET(lcd_reg, Hw13, Hw13);						// IH - Inverted Horizontal Sync
			BITCSET(lcd_reg, Hw12, Hw12);						// IP - Inverted Pixel Clock
			#endif
			BITCSET(lcd_reg, Hw7, Hw7);							// TV
			BITCSET(lcd_reg, Hw3|Hw2|Hw1, Hw3|Hw1);				// OVP
			BITCSET(lcd_reg, Hw0, Hw0);							// LEN - LCD Controller Enable
			#if 0 // COMPONENT_CCIR656
			BITCSET(lcd_reg, Hw10, Hw10);						// R2Y - RGB to YCbCr Converter Enable for OUPUT
			BITCSET(lcd_reg, Hw19|Hw18|Hw17|Hw16, Hw19);		// PXDW - Pixel Data Width
			#else
			BITCSET(lcd_reg, Hw19|Hw18|Hw17|Hw16, Hw19|Hw18);	// PXDW - Pixel Data Width
			#endif
			break;

		default:
			break;
	}
	
	BITCSET(pLCDC->LCTRL, 0xFFFFFFFF, lcd_reg);

	/* LCD Interrupt mask */
	pLCDC->LIM &= 0xFFFFFFE6;
	pLCDC->LIM |= ((1 << 4) | 1);
	
	#if defined(CONFIG_ARCH_TCC92XX)
		pLCDC_CH->LIP 	= (0 << 16) | 0;
		pLCDC_CH->LIS 	= (component_spec.component_lcd_height << 16) | component_spec.component_lcd_width;
		pLCDC_CH->LIBA0 = img_addr;
		pLCDC_CH->LIO 	= img_offset;
		pLCDC_CH->LISR 	= (0<<4) | 0;
	#endif
	
	BITCSET(pLCDC_CH->LIKR, 0xff <<  0, (0)    <<  0); // key
	BITCSET(pLCDC_CH->LIKR, 0xff << 16, (0xF8)	<< 16); // keymask
	BITCSET(pLCDC_CH->LIKG, 0xff <<  0, (0)    <<  0); // key
	BITCSET(pLCDC_CH->LIKG, 0xff << 16, (0xFC)	<< 16); // keymask
	BITCSET(pLCDC_CH->LIKB, 0xff <<  0, (0)    <<  0); // key
	BITCSET(pLCDC_CH->LIKB, 0xff << 16, (0xF8)	<< 16); // keymask
	
	if(type == STARTER_COMPONENT_720P)
	{
		pLCDC_CH->LIC = Hw29 | Hw28 | Hw16 | 10*Hw0;

		TCC_OUTPUT_LCDC_CtrlLayer(TCC_OUTPUT_COMPONENT, 0, TCC_LCDC_IMG_FMT_RGB565);

		/* PID port of THS8200 */
		#if defined(CONFIG_ARCH_TCC92XX)
			#if defined(CONFIG_MACH_TCC8900) || defined(CONFIG_MACH_TCC9200) || defined(CONFIG_MACH_TCC9201)
			BITSET(pGPIO->GPCDAT, Hw28);
			#else
			BITSET(pGPIO->GPCDAT, Hw30);
			#endif
		#elif defined(CONFIG_ARCH_TCC93XX)
			#if defined(CONFIG_MACH_TCC9300ST)
			BITCLR(pGPIO->GPCFN3, Hw4-Hw0);
			BITSET(pGPIO->GPCEN, Hw24);
			BITSET(pGPIO->GPCDAT, Hw24);
			#elif defined(CONFIG_MACH_TCC9300CM)
			BITSET(pGPIO->GPCDAT, Hw28);
			#else
			BITSET(pGPIO->GPEDAT, Hw1);
			#endif
		#elif defined(CONFIG_ARCH_TCC88XX)
			#if defined(CONFIG_MACH_TCC9300ST)
			BITCLR(pGPIO->GPCFN3, Hw4-Hw0);
			BITSET(pGPIO->GPCEN, Hw24);
			BITSET(pGPIO->GPCDAT, Hw24);
			#endif
		#endif
	}
	else
	{
		pLCDC_CH->LIC = Hw31 | Hw29 | Hw28 | Hw16 | 10*Hw0;
	}
	
	/* Enable Component Chip */
	if(type == STARTER_COMPONENT_480I_NTSC)
		cs4954_enable(COMPONENT_MODE_NTSC_M);
	else if(type == STARTER_COMPONENT_576I_PAL)
		cs4954_enable(COMPONENT_MODE_PAL_B);
	else if(type == STARTER_COMPONENT_720P)
		ths8200_enable(COMPONENT_MODE_720P);
	else if(type == STARTER_COMPONENT_1080I)
		ths8200_enable(COMPONENT_MODE_1080I);
	else
		cs4954_enable(COMPONENT_MODE_NTSC_M);

	#if 0 // for debugging
	{
		unsigned int i;
		unsigned int *pReg = (unsigned int *)pLCDC_CH;
		for(i=0; i<32; i++)
		{
			printk("0x%08x: 0x%08x\n", pReg+i, *(pReg+i));
		}
	}
	#endif

	/* Enable LCDC */
	BITSET(pLCDC->LCTRL, Hw0);
#endif
}


static int tcc_output_starter_ioctl(struct inode *inode, struct file *file, unsigned int cmd, void *arg)
{
	switch(cmd)
	{
		default:
			DPRINTF(" Unsupported IOCTL!!!\n");      
			break;			
	}
	return 0;
}

static int tcc_output_starter_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int tcc_output_starter_open(struct inode *inode, struct file *file)
{
	return 0;
}

static struct file_operations tcc_output_starter_fops = 
{
	.owner		= THIS_MODULE,
	.write		= NULL,
	.read		= NULL,
	.poll 		= NULL,
	.ioctl		= tcc_output_starter_ioctl,
	.mmap		= NULL,
	.open		= tcc_output_starter_open,
	.release	= tcc_output_starter_release,
};

static struct class *tcc_output_starter_class;

#define DEVICE_NAME			"output_starter"
#define MAJOR_ID			251
#define MINOR_ID			1

int __init tcc_output_starter_init(void)
{
	unsigned char lcdc_1st, lcdc_2nd, hdmi_resolution;

	printk("%s\n", __func__);

	register_chrdev(MAJOR_ID, DEVICE_NAME, &tcc_output_starter_fops);

	if(i2c_add_driver(&tcc_hdmi_phy_i2c_driver) != 0)
	{
		printk("Failed to register hdmi phy I2C client\n");
	}

	#if defined(CONFIG_ARCH_TCC93XX)
	if(i2c_add_driver(&tcc_hdmi_edid_i2c_driver) != 0)
	{
		printk("Failed to register hdmi edid I2C client\n");
	}

	if(i2c_add_driver(&tcc_hdmi_edid_seg_i2c_driver) != 0)
	{
		printk("Failed to register hdmi edid segment I2C client\n");
	}
	#endif

	tcc_output_starter_class = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(tcc_output_starter_class, NULL, MKDEV(MAJOR_ID, MINOR_ID), NULL, DEVICE_NAME);

	#if defined(CONFIG_ARCH_TCC93XX) || defined(CONFIG_ARCH_TCC88XX)
		lcdc_1st = STARTER_LCDC_0; /* LCDC0: HDMI or Component */
		lcdc_2nd = STARTER_LCDC_1; /* LCDC1: Composite */
	#else
		lcdc_1st = STARTER_LCDC_1; /* LCDC1: HDMI or Component */
		lcdc_2nd = STARTER_LCDC_0; /* LCDC0: Composite */
	#endif

	#if defined(TCC_OUTPUT_STARTER_DUAL)
		/* 1st Output Setting */
		if(tcc_display_data.output >= STARTER_OUTPUT_MAX)
			tcc_output_starter_hdmi(lcdc_1st, STARTER_HDMI_640x480P);
		else
			tcc_output_starter_hdmi(lcdc_1st, tcc_display_data.hdmi_resolution);

		/* 2nd Output Setting */
		if(tcc_display_data.output == STARTER_OUTPUT_COMPONENT)
			tcc_output_starter_component(lcdc_2nd, STARTER_COMPONENT_720P+tcc_display_data.component_resolution);
		else
			tcc_output_starter_composite(lcdc_2nd, tcc_display_data.composite_resolution);
	#elif defined(TCC_OUTPUT_STARTER_AUTO_DETECT)
		/* 1st Output Setting */
		if(tcc_display_data.output >= STARTER_OUTPUT_MAX)
			tcc_output_starter_hdmi(lcdc_1st, STARTER_HDMI_640x480P);
		else
			tcc_output_starter_hdmi(lcdc_1st, tcc_display_data.hdmi_resolution);

		if(tcc_display_data.output == 3)
		{
			if(tcc_component_detect_cable() == true)
				tcc_output_starter_component(lcdc_2nd, STARTER_COMPONENT_720P+tcc_display_data.component_resolution);
			else if(tcc_composite_detect_cable() == true)
				tcc_output_starter_composite(lcdc_2nd, tcc_display_data.composite_resolution);
		}
		else
		{
			if(tcc_composite_detect_cable() == true)
				tcc_output_starter_composite(lcdc_2nd, tcc_display_data.composite_resolution);
			else if(tcc_component_detect_cable() == true)
				tcc_output_starter_component(lcdc_2nd, STARTER_COMPONENT_720P+tcc_display_data.component_resolution);
		}
	#else
		switch(tcc_display_data.output)
		{
			case 0:
			case 1:
				tcc_output_starter_hdmi(lcdc_1st, STARTER_HDMI_1920x1080P);
				break;
			case 2:
				tcc_output_starter_composite(lcdc_2nd, STARTER_COMPOSITE_NTSC);
				break;
			case 3:
				#if defined(TCC_COMPONENT_IC_CS4954)
					tcc_output_starter_component(lcdc_2nd, STARTER_COMPONENT_480I_NTSC);
				#else
					tcc_output_starter_component(lcdc_2nd, STARTER_COMPONENT_720P);
				#endif
				break;
			default:
				tcc_output_starter_hdmi(lcdc_1st, STARTER_HDMI_1920x1080P);
				break;
		}
	#endif

	return 0;
}

void __exit tcc_output_starter_cleanup(void)
{
	printk("%s\n", __func__);

	unregister_chrdev(MAJOR_ID, DEVICE_NAME);

	i2c_del_driver(&tcc_hdmi_phy_i2c_driver);
	
	#if defined(CONFIG_ARCH_TCC93XX)
	i2c_del_driver(&tcc_hdmi_edid_i2c_driver);
	i2c_del_driver(&tcc_hdmi_edid_seg_i2c_driver);
	#endif
	
	if(tcc_hdmi_phy_i2c_client)
	{
		kfree(tcc_hdmi_phy_i2c_client);
		tcc_hdmi_phy_i2c_client = NULL;
	}

	return;
}

MODULE_AUTHOR("TELECHIPS");
MODULE_LICENSE("GPL");

module_init(tcc_output_starter_init);
module_exit(tcc_output_starter_cleanup);
