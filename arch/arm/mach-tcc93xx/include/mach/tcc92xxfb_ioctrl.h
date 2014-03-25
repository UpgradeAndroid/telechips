/****************************************************************************
 *	 TCC Version 0.6
 *	 Copyright (c) telechips, Inc.
 *	 ALL RIGHTS RESERVED
 *
****************************************************************************/

#ifndef _TCC_FB_IOCTL_H
#define _TCC_FB_IOCTL_H

#include <linux/types.h>

#define TCCFB_IOCTL_MAGIC	'f'

#define TCC_LCD_FB_IOCTL		0x46FF

typedef enum{
	TCC_HDMI_SUSEPNED,
	TCC_HDMI_RESUME,	
	TCC_HDMI_VIDEO_START,
	TCC_HDMI_VIDEO_END,
}TCC_HDMI_M;


#define TCC_LCDC_HDMI_START				0x0010
#define TCC_LCDC_HDMI_TIMING			0x0011
#define TCC_LCDC_HDMI_DISPLAY			0x0012
#define TCC_LCDC_HDMI_END				0x0013
#define TCC_LCDC_HDMI_CHECK				0x0014
#define TCC_LCDC_HDMI_MODE_SET			0x0015
#define TCC_LCDC_HDMI_SET_SIZE			0x0016
#define TCC_LCDC_HDMI_GET_SIZE			0x0017

#define TCC_LCDC_TVOUT_MODE_SET			0x0018


#define TCC_LCDC_COMPOSITE_CHECK		0x0020
#define TCC_LCDC_COMPOSITE_MODE_SET		0x0021

#define TCC_LCDC_COMPONENT_CHECK		0x0030
#define TCC_LCDC_COMPONENT_MODE_SET		0x0031

#define TCC_LCDC_OUTPUT_MODE_CHECK		0x0040
#define TCC_LCDC_OUTPUT_MODE_SET		0x0041

#define TCC_LCDC_SET_ENABLE				0x0050
#define TCC_LCDC_SET_DISABLE			0x0051
#define TCC_LCDC_LAYER_ENABLE			0x0052
#define TCC_LCDC_LAYER_DISABLE			0x0053
#define TCC_LCDC_SET_BGCOLOR			0x0054
#define TCC_LCDC_SET_ALPHA				0x0055
#define TCC_LCDC_GET_IMAGE_INFO			0x0056

#if defined(CONFIG_TCC_VIDEO_DISPLAY_BY_VSYNC_INT) || defined(TCC_VIDEO_DISPLAY_BY_VSYNC_INT)
#define	TCC_LCDC_VIDEO_START_VSYNC		0x0060
#define	TCC_LCDC_VIDEO_END_VSYNC		0x0061
#define	TCC_LCDC_VIDEO_PUSH_VSYNC		0x0062
#define	TCC_LCDC_VIDEO_GET_DISPLAYED	0x0063
#define	TCC_LCDC_VIDEO_CLEAR_FRAME		0x0064
#define	TCC_LCDC_VIDEO_SKIP_FRAME_START	0x0067
#define	TCC_LCDC_VIDEO_SKIP_FRAME_END	0x0068
#define	TCC_LCDC_VIDEO_DEINTERLACE_MODE	0x0069
#define	TCC_LCDC_VIDEO_SET_SIZE_CHANGE	0x006A
#define	TCC_LCDC_VIDEO_SET_FRAMERATE	0x006B
#endif

#define TCC_LCD_BL_SET					0x0100

#define TCC_FB_FLUSH					0x1000

#define TCCFB_BLIT	_IOW(TCCFB_IOCTL_MAGIC, 0x1100, unsigned int)

typedef enum{
	LCDC_TVOUT_NOE_MODE,
	LCDC_TVOUT_UI_MODE,
	LCDC_TVOUT_VIDEO_ONLY_MODE,
	LCDC_TVOUT_OVERALY_MODE,
	LCDC_TVOUT_MODE_MAX
}LCDC_TVOUT_MODE;

typedef enum{
	LCDC_COMPOSITE_NONE_MODE,
	LCDC_COMPOSITE_UI_MODE,
	LCDC_COMPOSITE_VIDEO_ONLY_MODE,
	LCDC_COMPOSITE_OVERALY_MODE,
	LCDC_COMPOSITE_MODE_MAX
}LCDC_COMPOSITE_MODE;

typedef enum{
	LCDC_COMPONENT_NONE_MODE,
	LCDC_COMPONENT_UI_MODE,
	LCDC_COMPONENT_VIDEO_ONLY_MODE,
	LCDC_COMPONENT_OVERALY_MODE,
	LCDC_COMPONENT_MODE_MAX
}LCDC_COMPONENT_MODE;

typedef enum
{
	OUTPUT_NONE,
	OUTPUT_HDMI,
	OUTPUT_COMPOSITE,
	OUTPUT_COMPONENT,
	OUTPUT_MAX
}OUTPUT_SELECT;

typedef enum
{
	OUTPUT_SELECT_NONE,
	OUTPUT_SELECT_HDMI_ENABLE,
	OUTPUT_SELECT_COMPONENT_ENABLE,
	OUTPUT_SELECT_COMPOSITE_ENABLE,
	OUTPUT_SELECT_MAX
}OUTPUT_SELECT_MODE;

struct display_platform_data 
{
	uint32_t resolution;
	uint32_t output;
        uint32_t composite;
}; 

#define TCCFB_ROT_90	0x4
#define TCCFB_ROT_180	0x8
#define TCCFB_ROT_270	0x10

typedef struct 
{
	int width;
	int height;
}tcc_display_size;

struct tcc_rect {
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
};

struct tcc_img {
	uint32_t width;
	uint32_t height;
	uint32_t format;
	uint32_t offset;
	int fd;
	void *addr;
};

struct tcc_blit_req {
	struct tcc_img src;
	struct tcc_img dst;
	struct tcc_rect src_rect;
	struct tcc_rect dst_rect;
	uint32_t alpha;
    uint32_t transform;
};

struct tcc_blit_req_list {
	uint32_t count;
	struct tcc_blit_req req[];
};


typedef enum{
	LCDC_LAYER_0, 
	LCDC_LAYER_1, 
	LCDC_LAYER_2, 
	LCDC_LAYER_MAX
}LCD_IMG_LAYER_TYPE;


typedef enum{
	TCC_LCDC_IMG_FMT_1BPP,
	TCC_LCDC_IMG_FMT_2BPP,
	TCC_LCDC_IMG_FMT_4BPP,
	TCC_LCDC_IMG_FMT_8BPP,
	TCC_LCDC_IMG_FMT_RGB332 = 8,
	TCC_LCDC_IMG_FMT_RGB444 = 9,
	TCC_LCDC_IMG_FMT_RGB565 = 10,
	TCC_LCDC_IMG_FMT_RGB555 = 11,
	TCC_LCDC_IMG_FMT_RGB888 = 12,
	TCC_LCDC_IMG_FMT_RGB666 = 13,
	TCC_LCDC_IMG_FMT_YUV420SP = 24,	
	TCC_LCDC_IMG_FMT_YUV422SP = 25, 
	TCC_LCDC_IMG_FMT_YUV422SQ = 26, 
	TCC_LCDC_IMG_FMT_YUV420ITL0 = 28, 
	TCC_LCDC_IMG_FMT_YUV420ITL1 = 29, 
	TCC_LCDC_IMG_FMT_YUV422ITL0 = 30, 
	TCC_LCDC_IMG_FMT_YUV422ITL1 = 31, 
	TCC_LCDC_IMG_FMT_MAX
}TCC_LCDC_IMG_FMT_TYPE;

struct tcc_lcdc_image_update
{
	unsigned int Lcdc_layer;
	unsigned int enable;
	unsigned int Frame_width;
	unsigned int Frame_height;

	unsigned int Image_width;
	unsigned int Image_height;
	unsigned int offset_x; //position
	unsigned int offset_y; 	//position
	unsigned int addr0;
	unsigned int addr1;
	unsigned int addr2;
	unsigned int fmt;	//TCC_LCDC_IMG_FMT_TYPE
	unsigned int on_the_fly; // 0: not use , 1 : scaler0 ,  2 :scaler1
#if defined(CONFIG_TCC_VIDEO_DISPLAY_BY_VSYNC_INT) || defined(TCC_VIDEO_DISPLAY_BY_VSYNC_INT)
	int time_stamp;
	int sync_time;
	unsigned int buffer_unique_id;
	unsigned int overlay_used_flag;
	OUTPUT_SELECT outputMode;

	int deinterlace_mode;
	int odd_first_flag;
#endif
};


struct tcc_lcdc_get_image_info
{
	unsigned int lcdc_num;
	unsigned int layer;
	unsigned int enable;
	unsigned int lcd_width;
	unsigned int lcd_height;
	unsigned int image_width;
	unsigned int image_height;
	unsigned int offset_x; //position
	unsigned int offset_y; 	//position
	unsigned int addr0;
	unsigned int addr1;
	unsigned int addr2;
	unsigned int fmt;	//TCC_LCDC_IMG_FMT_TYPE
};


typedef enum{
	LCDC_HDMI_NONE_MODE,
	LCDC_HDMI_UI_MODE,
	LCDC_HDMI_VIDEO_ONLY_MODE,
	LCDC_HDMI_OVERALY_MODE,
	LCDC_HDMI_MODE_MAX
}LCDC_HDMI_MODE;

struct lcdc_timimg_parms_t {

	unsigned int id;
	unsigned int iv;
	unsigned int ih;
	unsigned int ip;
	unsigned int dp;
	unsigned int ni;
	unsigned int tv;
	unsigned int tft;
	unsigned int stn;

	//LHTIME1
	unsigned int lpw;
	unsigned int lpc;
	//LHTIME2
	unsigned int lswc;
	unsigned int lewc;
	//LVTIME1
	unsigned int vdb;
	unsigned int vdf;
	unsigned int fpw;
	unsigned int flc;
	//LVTIME2
	unsigned int fswc;
	unsigned int fewc;
	//LVTIME3
	unsigned int fpw2;
	unsigned int flc2;
	//LVTIME4
	unsigned int fswc2;
	unsigned int fewc2;

};

typedef struct  
{
	unsigned lcdc_num;
	unsigned layer_num;
}lcdc_layerctrl_params;

typedef struct  
{
	unsigned lcdc_num;
	unsigned red;
	unsigned green;
	unsigned blue;
}lcdc_bgcolor_params;

typedef struct  
{
	unsigned lcdc_num;
	unsigned layer_num;
	unsigned layer_alpha;
}lcdc_alpha_params;

#endif
