/* linux/arch/arm/mach-msm/irq.c
 *
 * Copyright (C) 2008, 2009 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#define TCC_WMIXER_IOCTRL 					0x1


#ifndef ADDRESS_ALIGNED
#define ADDRESS_ALIGNED
#define ALIGN_BIT 							(0x8-1)
#define BIT_0 								3
#define GET_ADDR_YUV42X_spY(Base_addr) 		(((((unsigned int)Base_addr) + ALIGN_BIT)>> BIT_0)<<BIT_0)
#define GET_ADDR_YUV42X_spU(Yaddr, x, y) 	(((((unsigned int)Yaddr+(x*y)) + ALIGN_BIT)>> BIT_0)<<BIT_0)
#define GET_ADDR_YUV422_spV(Uaddr, x, y) 	(((((unsigned int)Uaddr+(x*y/2)) + ALIGN_BIT) >> BIT_0)<<BIT_0)
#define GET_ADDR_YUV420_spV(Uaddr, x, y) 	(((((unsigned int)Uaddr+(x*y/4)) + ALIGN_BIT) >> BIT_0)<<BIT_0)
#endif

typedef enum {
	WMIXER_POLLING,
	WMIXER_INTERRUPT,
	WMIXER_NOWAIT
} WMIXER_RESPONSE_TYPE;

typedef enum {
	WMIXER_LAYER0,
	WMIXER_LAYER1,
	WMIXER_LAYER2,
	WMIXER_LAYER3,
	WMIXER_LAYER_MAX
} WMIXER_CHANNEL_TYPE;

typedef enum {
	SCALER_YUV422_sq0,
	SCALER_YUV422_sq1,
	SCALER_YUV422_sp,
	SCALER_YUV420_sp,
	SCALER_YUV422_inter,
	SCALER_YUV420_inter,
	SCALER_RGB565,
	SCALER_RGB555,
	SCALER_RGB454,
	SCALER_RGB444,
	SCALER_ARGB8888,
	SCALER_FMT_MAX
} WMIXER_IMAGE_FORMAT_TYPE;

typedef struct {
	unsigned int 			rsp_type; 		// wmix response type
	unsigned int 			wmix_channel; 	// wmix channel
	unsigned int 			wmix_uv_change; // wmix v <-> v swap

	unsigned int 			src_y_addr;		// source y address
	unsigned int 			src_u_addr;		// source u address
	unsigned int 			src_v_addr;		// source v address
	unsigned int 			src_img_fmt; 	// source image format
	unsigned int 			src_img_width; 	// source image width
	unsigned int 			src_img_height; // source image height
	//unsigned int 			src_win_left;
	//unsigned int 			src_win_top;
	//unsigned int 			src_win_right;
	//unsigned int 			src_win_bottom;
	
	unsigned int 			dst_y_addr; 	// destination image address
	unsigned int 			dst_u_addr; 	// destination image address
	unsigned int 			dst_v_addr; 	// destination image address
	unsigned int 			dst_fmt;		// destination image format
	//unsigned int 			dst_img_width; 	// destination image width
	//unsigned int 			dst_img_height; // destination image height
	unsigned int 			dst_win_left;
	unsigned int 			dst_win_top;
	unsigned int 			dst_win_right;
	unsigned int 			dst_win_bottom;
}WMIXER_INFO_TYPE;


