
/*
 * drivers/media/video/tcccam/tcc83x_cif.c
 *
 * Copyright (C) 2008 Telechips, Inc. 
 * 
 * Video-for-Linux (Version 2) camera capture driver for
 * the TCC78xx camera controller.
 *
 * leverage some code from CEE distribution 
 * Copyright (C) 2003-2004 MontaVista Software, Inc.
 * 
 * This package is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. 
 * 
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED 
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. 
 */
 
#include <generated/autoconf.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/sched.h>

#include <mach/irqs.h>
//#include <asm/arch/dma.h>
#include <mach/hardware.h>
#include <asm/io.h>

#if defined(CONFIG_ARCH_TCC92XX) || defined(CONFIG_ARCH_TCC93XX)  || defined(CONFIG_ARCH_TCC88XX)
#include <mach/bsp.h>
#elif defined(CONFIG_ARCH_TCC79X)
#include <mach/tcc79x.h>
#include <asm/plat-tcc/tcc_ckc.h>
#endif

#include <mach/memory.h>
#include <asm/scatterlist.h>
#include <asm/mach-types.h>

#include "sensor_if.h"
#include "tcc_cam.h"
#include "camera_hw_if.h"
#include "camera_core.h"
#include "tdd_cif.h"
#include "cam.h"
#include "cam_reg.h"
#include <plat/pmap.h>
#include <mach/tcc_cam_ioctrl.h>
#include "tcc_cam_i2c.h"
#if defined(CONFIG_USE_ISP)
#include "tccisp/isp_interface.h"
#endif

#if 1
static int debug	   = 1;
#else
static int debug	   = 0;
#endif

#define dprintk(msg...)	if (debug) { printk( "Tcc_cam: " msg); }

#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP)
extern unsigned int gnSizeOfAITALLJPEG;
extern unsigned int gnSizeOfAITMainJPEG;
extern unsigned int gnSizeOfAITThumbnail;
extern unsigned int gnoffsetOfAITThumbnail;
extern unsigned short gnWidthOfAITMainJPEG;
extern unsigned short gnHeightOfAITMainJPEG;
#endif

#ifdef JPEG_ENCODE_WITH_CAPTURE
#include "tcc83xx/tcc83xx_jpeg.h"
#include "tcc83xx/jpeg_param.h"

JPEG_ENC_INFO  gJPEG_ENC_Info;
JPEG_DEC_INFO  gJPEG_DEC_Info;
JPEG_BUFFER_CTRL gJPEG_Buffer_Info;
JPEG_DISPLAY_INFO JPEG_Info;
jpeg_parser_rsp gJpegInfo;
extern volatile char gJpegDecodeDone;
extern volatile char gJpegEncodeDone;
unsigned char gJpegCodecMode = JPEG_UNKNOWN_MODE;
extern uint8 gIsExifThumbnailProcess;
extern jpeg_encode_option_type gEncodeOption;

unsigned char gIsRolling;

void *jpeg_header_remapped_address;   // Header Buffer
void *jpeg_remapped_base_address;

//#define JPEG_STREAM      	0x180000 //1600*1024
//#define JPEG_HEADER      	0x020000 //96*1024
#define JPEG_STREAM      	0x1D0000 //1856*1024
#define JPEG_HEADER      	0x030000 //192*1024

#define CAPTURE_PHY_ADDRESS	PA_VIDEO_BASE
#define JPEG_PHY_ADDRESS	PA_VIDEO_BASE + CAPTURE_MEM
#endif

static pmap_t pmap_cam;

#define PA_PREVIEW_BASE_ADDR	pmap_cam.base
#define PREVIEW_MEM_SIZE	pmap_cam.size

u8 current_effect_mode = 0;
struct TCCxxxCIF hardware_data;
#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
TCC_SENSOR_INFO_TYPE tcc_sensor_info;
int CameraID;
#endif

#define NUM_FREQS 2
static unsigned int gtCamSizeTable[NUM_FREQS] =
{
	720 * 480 , //D1,	
	1280 * 720, //HD720P,
};

void cif_timer_register(void* priv, unsigned long timeover);
void cif_dma_hw_reg(unsigned char frame_num);

static unsigned char cam_irq = 0;
static unsigned char skip_frm = 0, skipped_frm = 0;
static unsigned char frame_lock = 0;

#if defined(CONFIG_USE_ISP)
#ifdef TCCISP_GEN_CFG_UPD
static 	struct tccxxx_cif_buffer * next_buf = 0;
static unsigned next_num = 0;
static unsigned chkpnt = 0;
#endif
#endif


void cif_set_frameskip(unsigned char skip_count, unsigned char locked)
{
#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_PERFORMANCE
	skip_frm = frame_lock = 0;
#else
	skip_frm = skip_count;
	frame_lock = locked;
#endif
}
EXPORT_SYMBOL(cif_set_frameskip);

int cif_get_capturedone(void)
{
	if(hardware_data.cif_cfg.cap_status == CAPTURE_DONE)
	{
		mdelay(5);
		return 1;
	}
	
	return 0;
}
EXPORT_SYMBOL(cif_get_capturedone);

int Get_Index_sizeTable(unsigned int Image_Size)
{
	int i;

	for(i=0; i<NUM_FREQS; i++)
	{
		if( gtCamSizeTable[i] >= Image_Size)
		{
			return i;
		}
	}
	
	return (NUM_FREQS -1);
}
EXPORT_SYMBOL(Get_Index_sizeTable);

int tccxxx_isalive_camera(void)
{
	unsigned short HSize, VSize;
	volatile PCKC pCKC = (PCKC)tcc_p2v(HwCLK_BASE);
	volatile PCIF pCIF = (PCIF)tcc_p2v(HwCIF_BASE);

#if defined(CONFIG_USE_ISP)
	struct TCCxxxCIF *data = (struct TCCxxxCIF *)&hardware_data;

	if(pCKC->PCLK_ISPS & Hw29)
	{
		dprintk("JPEG - Cam Alive = %d x %d \n", data->cif_cfg.main_set.target_x, data->cif_cfg.main_set.target_y);
		return (data->cif_cfg.main_set.target_x * data->cif_cfg.main_set.target_y);
	}
#else
	#if defined(CONFIG_ARCH_TCC92XX)
	if(pCKC->PCLK_CIFMC & Hw28)
	#else // CONFIG_ARCH_TCC93XX or CONFIG_ARCH_TCC88XX
	if(pCKC->PCLK_CIFMC & Hw29)
	#endif
	{
		if(pCIF->ICPCR1 & Hw31)
		{
			HSize = (pCIF->IIS >> 16) - (pCIF->IIW1 >> 16) - (pCIF->IIW1 & 0xFFFF);
			VSize = (pCIF->IIS & 0xFFFF) - (pCIF->IIW2 >> 16) - (pCIF->IIW2 & 0xFFFF);

			dprintk("JPEG - Cam Alive = %d x %d \n", HSize, VSize);
			return HSize * VSize;
		}
	}
#endif

	return 0;
}
EXPORT_SYMBOL(tccxxx_isalive_camera);


static void cif_data_init(void *priv)
{
	//카메라 초기화 
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) priv;

#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
	//Default Setting
	data->cif_cfg.polarity_pclk 	= tcc_sensor_info.p_clock_pol;

	data->cif_cfg.polarity_vsync 	= tcc_sensor_info.v_sync_pol;
	data->cif_cfg.polarity_href 	= tcc_sensor_info.h_sync_pol;

	data->cif_cfg.zoom_step			= 0;
	
	data->cif_cfg.base_buf			= hardware_data.cif_buf.addr;
	data->cif_cfg.pp_num			= 0;//TCC_CAMERA_MAX_BUFNBRS;

	data->cif_cfg.fmt				= tcc_sensor_info.format;
	
	data->cif_cfg.order422 			= CIF_YCBYCR;

	data->cif_cfg.oper_mode 		= OPER_PREVIEW;
	data->cif_cfg.main_set.target_x = 0;
	data->cif_cfg.main_set.target_y = 0;

	data->cif_cfg.esd_restart 		= OFF;

	data->cif_cfg.cap_status 		= CAPTURE_NONE;
	data->cif_cfg.retry_cnt			= 0;
	
	current_effect_mode = 0;
#else // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
	//Default Setting
	data->cif_cfg.order422 			= CIF_YCBYCR;

#if defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9P111) || defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP) \
	|| defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006) || defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150) || defined(CONFIG_VIDEO_ATV_SENSOR_RDA5888) \
	|| defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9M113) || defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9D112) || defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9T113)
	data->cif_cfg.polarity_pclk 	= POSITIVE_EDGE;
#else
	data->cif_cfg.polarity_pclk 	= NEGATIVE_EDGE;
#endif

#if defined(CONFIG_VIDEO_CAMERA_SENSOR_MV9317) || defined(CONFIG_VIDEO_CAMERA_SENSOR_OV7690)
	data->cif_cfg.polarity_vsync 	= ACT_LOW;
#else
	data->cif_cfg.polarity_vsync 	= ACT_HIGH;
#endif
	data->cif_cfg.polarity_href 	= ACT_HIGH;

	data->cif_cfg.oper_mode 		= OPER_PREVIEW;
	data->cif_cfg.zoom_step			= 0;
	
	#if (1) //20111209 ysseung   test...
	data->cif_cfg.base_buf			= NULL;
	#else
	data->cif_cfg.base_buf			= hardware_data.cif_buf.addr;
	#endif

	data->cif_cfg.pp_num			= 0;//TCC_CAMERA_MAX_BUFNBRS;
#if defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150) || defined(CONFIG_VIDEO_ATV_SENSOR_RDA5888)
	data->cif_cfg.fmt				= M420_EVEN;
#else
	data->cif_cfg.fmt				= M420_ZERO;
#endif

	data->cif_cfg.main_set.target_x = 0;
	data->cif_cfg.main_set.target_y = 0;

	data->cif_cfg.esd_restart 		= OFF;

	data->cif_cfg.cap_status 		= CAPTURE_NONE;
	data->cif_cfg.retry_cnt			= 0;
	
	current_effect_mode = 0;
#endif // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT

#if defined(CONFIG_USE_ISP)
	ISP_Init();
#endif
}

#if defined(CONFIG_USE_ISP)
static irqreturn_t isp_cam_isr0(int irq, void *client_data/*, struct pt_regs *regs*/)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *)&hardware_data;
	struct tccxxx_cif_buffer *curr_buf, *next_buf;

	unsigned int curr_num, next_num;
	volatile PCIF pCIF;

	pCIF = (PCIF)tcc_p2v(HwISPBASE);
	client_data = client_data;
}

//20101126 ysseung   test code.
typedef struct {
	unsigned int *y_addr;
	unsigned int *u_addr;
	unsigned int *v_addr;	
} isp_tmp;
isp_tmp isp[5];
extern void cif_interrupt_disable(void);
static irqreturn_t isp_cam_isr1(int irq, void *client_data/*, struct pt_regs *regs*/)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *)&hardware_data;
#ifdef TCCISP_GEN_CFG_UPD
	struct tccxxx_cif_buffer *curr_buf;
	unsigned int curr_num;
#else
	struct tccxxx_cif_buffer *curr_buf, *next_buf;

	unsigned int curr_num, next_num;
#endif
	volatile PCIF pCIF;

	pCIF = (PCIF)tcc_p2v(HwISPBASE);
	
	client_data = client_data;
#ifndef TCCISP_GEN_CFG_UPD	
	if ( ISP_CheckInterrupt() )
	{
		ISP_ClearInterrupt();
		printk ("isp_cam_isr1: Collision Detected!!!! \n");

		return IRQ_HANDLED;
	}
#endif
	ISP_EventHandler2();

	if(ISP_EventHandler2_getEvent(1))
	{
		if(data->stream_state == STREAM_ON)
		{
			if(skip_frm == 0 && !frame_lock)
			{
				curr_buf = data->buf + data->cif_cfg.now_frame_num;
				curr_num = data->cif_cfg.now_frame_num;
#ifndef TCCISP_GEN_CFG_UPD
				next_buf = list_entry(data->list.next->next, struct tccxxx_cif_buffer, buf_list);
				next_num = next_buf->v4lbuf.index;

				if(next_buf != &data->list && curr_buf != next_buf )
				{
					//exception process!!
#else
				if(next_buf != &data->list && curr_buf != next_buf && (chkpnt==1))
				{
	#ifndef TCCISP_ONE_IRQ
					//exception process!!
					//next_buf = 0; 
					chkpnt = 2;
	#endif   // TCCISP_ONE_IRQ				
#endif
	#ifndef TCCISP_ONE_IRQ
					if(curr_num != curr_buf->v4lbuf.index)
					{
						printk("Frame num mismatch :: true num  :: %d \n", curr_num);
						printk("Frame num mismatch :: false num :: %d \n", curr_buf->v4lbuf.index);
						curr_buf->v4lbuf.index = curr_num ;
					}

					#if (0) //20101122 ysseung   test code.
					{
						//isp_tmp = ioremap(data->cif_cfg.preview_buf[curr_num].p_Cr, 4096);
						*isp[curr_num].y_addr = 0x80808080;
						*(isp[curr_num].y_addr + data->cif_cfg.main_set.target_x/4) = 0x80808080;
						*(isp[curr_num].y_addr + data->cif_cfg.main_set.target_x/4*2) = 0x80808080;
						*(isp[curr_num].y_addr + data->cif_cfg.main_set.target_x/4*3) = 0x80808080;

						*isp[curr_num].u_addr = 0x80808080;
						*(isp[curr_num].u_addr + data->cif_cfg.main_set.target_x/8) = 0x80808080;

						*isp[curr_num].v_addr = 0x80808080;
						*(isp[curr_num].v_addr + data->cif_cfg.main_set.target_x/8) = 0x80808080;
					}
					#endif
					#if (0)
					{
						static unsigned int d_old = 0;
						unsigned int d = 0;
						//volatile unsigned int *temp;

						d = *(volatile unsigned int *)isp[curr_num].y_addr;
						if (d_old)
						{
							if (d != d_old)
							{
								printk ("Check point....(isp) \n");
							}
						}
						d_old = d;
					}
					#endif
					//cif_dma_hw_reg(next_num);
					{	
						struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
						data->cif_cfg.now_frame_num = next_num;

						#ifndef TCCISP_GEN_CFG_UPD
						ISP_Zoom_Apply (1);
						ISP_SetPreviewH_StartAddress((unsigned int)data->cif_cfg.preview_buf[next_num].p_Y,
														(unsigned int)data->cif_cfg.preview_buf[next_num].p_Cr,
														(unsigned int)data->cif_cfg.preview_buf[next_num].p_Cb);
						#else
							#ifdef TCCISP_UPDATE

							#endif
						#endif
					}
					if(data->cif_cfg.fmt == M420_ZERO)
						curr_buf->v4lbuf.bytesused = data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y*2;
					else
						curr_buf->v4lbuf.bytesused = (data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y*3)/2;

				        //cif_buf->v4lbuf.sequence = cam->buf_seq[bufno];
					curr_buf->v4lbuf.flags &= ~V4L2_BUF_FLAG_QUEUED;
					curr_buf->v4lbuf.flags |= V4L2_BUF_FLAG_DONE;
					//spin_lock_irqsave(&data->dev_lock, flags);
					//cif_buf->v4lbuf.timestamp = gettimeofday(.., ..);

					list_move_tail(&curr_buf->buf_list, &data->done_list);
				}
				else
				{
					dprintk("no-buf change... wakeup!! \n");
					skipped_frm++;
				}

				data->wakeup_int = 1;
				wake_up_interruptible(&data->frame_wait);
			}
			else
			{
				if(skip_frm > 0)
					skip_frm--;
				else
					skip_frm = 0;
			}
	#endif   // TCCISP_ONE_IRQ	

		}
		else if((data->stream_state == STREAM_OFF) && (data->cif_cfg.oper_mode == OPER_CAPTURE)) // capture mode.
		{
			// turn off Camera Flash
			sensor_turn_off_camera_flash();
			
			ISP_Stop_Irq();
/*
			data->cif_cfg.now_frame_num = 0;
			
			curr_buf = data->buf + data->cif_cfg.now_frame_num;
			
			curr_buf->v4lbuf.bytesused 	= data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y*2;
			curr_buf->v4lbuf.flags 		&= ~V4L2_BUF_FLAG_QUEUED;
			curr_buf->v4lbuf.flags 		&= ~V4L2_BUF_FLAG_DONE;

			list_move_tail(&curr_buf->buf_list, &data->done_list);
*/
			data->cif_cfg.cap_status = CAPTURE_DONE;

			wake_up_interruptible(&data->frame_wait); // POLL
		}
	}

	return IRQ_HANDLED;
}

static irqreturn_t isp_cam_isr2(int irq, void *client_data/*, struct pt_regs *regs*/)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *)&hardware_data;
	struct tccxxx_cif_buffer *curr_buf; //, *next_buf;
	unsigned int curr_num; //, next_num;
	volatile PCIF pCIF;

	pCIF = (PCIF)tcc_p2v(HwISPBASE);
	
	//client_data = client_data;
#ifndef TCCISP_GEN_CFG_UPD	
	if ( ISP_CheckInterrupt() )
	{
		printk ("isp_cam_isr2: Collision Detected!!!!======================================== \n");
	}
#endif
	ISP_EventHandler3();

	ISP_EventHandler3_getEvent(8);	

	if(ISP_EventHandler3_getEvent(0x40))  //DRVISPINT_VSTART
	{
		sensor_if_check_control();
#ifdef TCCISP_GEN_CFG_UPD		
		if(data->stream_state == STREAM_ON)
		{
			if(skip_frm == 0 && !frame_lock)
			{
				curr_buf = data->buf + data->cif_cfg.now_frame_num;
				curr_num = data->cif_cfg.now_frame_num;

				next_buf = list_entry(data->list.next->next, struct tccxxx_cif_buffer, buf_list);	
				next_num = next_buf->v4lbuf.index;

				if(next_buf != &data->list && curr_buf != next_buf)
				{
					ISP_SetPreviewH_StartAddress((unsigned int)data->cif_cfg.preview_buf[next_num].p_Y,
								(unsigned int)data->cif_cfg.preview_buf[next_num].p_Cb,
								(unsigned int)data->cif_cfg.preview_buf[next_num].p_Cr);
				}
				chkpnt =1;
			}
		}
#endif
	}
	
	return IRQ_HANDLED;
}

static irqreturn_t isp_cam_isr3(int irq, void *client_data/*, struct pt_regs *regs*/)
{
}
#endif

static irqreturn_t cif_cam_isr(int irq, void *client_data/*, struct pt_regs *regs*/)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *)&hardware_data;
	struct tccxxx_cif_buffer *curr_buf, *next_buf;
	unsigned int curr_num, next_num;
#if defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150) || defined(CONFIG_VIDEO_ATV_SENSOR_RDA5888)
	static int	isr_cnt = 0;
#endif
	
#ifdef JPEG_ENCODE_WITH_CAPTURE
	unsigned int uCLineCnt, uTempVCNT;
#endif

#if defined(CONFIG_ARCH_TCC92XX) || defined(CONFIG_ARCH_TCC93XX) || defined(CONFIG_ARCH_TCC88XX)
	volatile PCIF pCIF;

	pCIF = (PCIF)tcc_p2v(HwCIF_BASE);
#endif

	client_data = client_data;

#if defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150)
	if (isr_cnt >= 2)
		BITSET(pCIF->ICPCR1, 1<<26);
	isr_cnt++;	
#endif

#ifdef JPEG_ENCODE_WITH_CAPTURE
	if (data->cif_cfg.oper_mode == OPER_CAPTURE) 
	{
		// turn off Camera Flash
		sensor_turn_off_camera_flash();
		
		if(TDD_CIF_GetIntStatus(GET_CIF_INT_VCNT_GEN))
		{
			if (gIsRolling)
			{
				uCLineCnt = pCIF->HwCCLC;
				
				BITSET(pCIF->HwCIRQ, HwCIRQ_VIT);
				
				uTempVCNT = pCIF->HwCCLC;
				while(uTempVCNT%FRAME_LINE_CNT == 0 && uTempVCNT != data->cif_cfg.main_set.target_y)
					uTempVCNT = pCIF->HwCCLC;

				BITSET(pCIF->HwCIRQ, HwCIRQ_VIT);

				if(gJpegCodecMode != JPEG_ENCODE_MODE)
				{
					gJpegCodecMode = JPEG_ENCODE_MODE;
					BITCSET(HwJPEGENC->JP_START, 0x0000000F, HwJP_START_RUN);
					BITCSET(HwJPEGENC->HwJP_SBUF_WCNT, 0x0000FFFF, uCLineCnt);
				}
				else if(((uCLineCnt%FRAME_LINE_CNT==0)||(uCLineCnt >= data->cif_cfg.main_set.target_y - FRAME_LINE_CNT)))
				{
					if(uCLineCnt >= (data->cif_cfg.main_set.target_y - FRAME_LINE_CNT))
					{
						BITCSET(HwJPEGENC->HwJP_SBUF_WCNT, 0x0000FFFF, data->cif_cfg.main_set.target_y);
					}
					else
						BITCSET(HwJPEGENC->HwJP_SBUF_WCNT, 0x0000FFFF, uCLineCnt);
				}
			}

			return IRQ_HANDLED;
		}
	
		if(TDD_CIF_GetIntStatus(GET_CIF_INT_ENC_STRT))
		{
			BITSET(pCIF->HwCIRQ, HwCIRQ_ENS);
		
			gJpegCodecMode = JPEG_ENCODE_MODE;
			BITCSET(HwJPEGENC->JP_START, 0x0000000F, HwJP_START_RUN);
			return IRQ_HANDLED;
		}
	}
#else
	if(TDD_CIF_GetIntStatus(GET_CIF_INT_ENC_STRT))
	{
		data->cif_cfg.now_frame_num = 0;
		curr_buf = data->buf + data->cif_cfg.now_frame_num;		
		
		curr_buf->v4lbuf.bytesused =	data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y*2;
		curr_buf->v4lbuf.flags &= ~V4L2_BUF_FLAG_QUEUED;
		curr_buf->v4lbuf.flags |= V4L2_BUF_FLAG_DONE;

		list_move_tail(&curr_buf->buf_list, &data->done_list);		

		data->cif_cfg.cap_status = CAPTURE_DONE;

		wake_up_interruptible(&data->frame_wait); //POLL
		
	#if defined(CONFIG_ARCH_TCC92XX) || defined(CONFIG_ARCH_TCC93XX)  || defined(CONFIG_ARCH_TCC88XX)
		BITSET(pCIF->CIRQ, HwCIRQ_ENS);
		BITSET(pCIF->CIRQ, HwCIRQ_ENS);
	#endif

		return IRQ_HANDLED;
	}
#endif

	//preview : Stored-One-Frame in DMA
	if(TDD_CIF_GetIntStatus(GET_CIF_INT_ONEFRAME_STORE))
	{
		if(data->stream_state == STREAM_ON)
		{
			#if !defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150)
			TDD_CIF_SetInterrupt(SET_CIF_UPDATE_WITHOUT_VSYNC);
			sensor_if_check_control();
			#endif
			if(skip_frm == 0 && !frame_lock)
			{				
				//dprintk("[Camera Preview] Interrupt Rising Up!!\n");
				curr_buf = data->buf + data->cif_cfg.now_frame_num;
				curr_num = data->cif_cfg.now_frame_num;
	
				next_buf = list_entry(data->list.next->next, struct tccxxx_cif_buffer, buf_list);	
				next_num = next_buf->v4lbuf.index;

				if(next_buf != &data->list && curr_buf != next_buf)
				{
					//exception process!!
					if(curr_num != curr_buf->v4lbuf.index)
					{
						printk("Frame num mismatch :: true num  :: %d \n", curr_num);
						printk("Frame num mismatch :: false num :: %d \n", curr_buf->v4lbuf.index);
						curr_buf->v4lbuf.index = curr_num ;
					}
		
					cif_dma_hw_reg(next_num);
					//spin_unlock_irqrestore(&data->dev_lock, flags);
					if(data->cif_cfg.fmt == M420_ZERO)
						curr_buf->v4lbuf.bytesused = data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y*2;
					else
						curr_buf->v4lbuf.bytesused = (data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y*3)/2;

					//cif_buf->v4lbuf.sequence = cam->buf_seq[bufno];
					curr_buf->v4lbuf.flags &= ~V4L2_BUF_FLAG_QUEUED;
					curr_buf->v4lbuf.flags |= V4L2_BUF_FLAG_DONE;
					//spin_lock_irqsave(&data->dev_lock, flags);
					//cif_buf->v4lbuf.timestamp = gettimeofday(.., ..);

					list_move_tail(&curr_buf->buf_list, &data->done_list);					
				}
				else
				{
					dprintk("no-buf change... wakeup!! \n");
					skipped_frm++;
				}

				data->wakeup_int = 1;
				wake_up_interruptible(&data->frame_wait);
			}
			else
			{
				if(skip_frm > 0)
					skip_frm--;
				else
					skip_frm = 0;
			}
			
			#if defined(CONFIG_ARCH_TCC92XX) || defined(CONFIG_ARCH_TCC93XX) || defined(CONFIG_ARCH_TCC88XX)
			BITSET(pCIF->CIRQ, HwCIRQ_SOF);
			BITSET(pCIF->CIRQ, HwCIRQ_MSOF);
			BITCLR(pCIF->CIRQ, HwCIRQ_MSOF);
			#else // (CONFIG_ARCH_TCC79X)
			BITSET(HwCIRQ, HwCIRQ_SOF);
			BITSET(HwCIRQ, HwCIRQ_MSOF);
			BITCLR(HwCIRQ, HwCIRQ_MSOF);
			#endif

			#if defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150)
			// Do nothing.
			#else
			TDD_CIF_SetInterrupt(SET_CIF_UPDATE_IN_VSYNC);
			#endif
		}		
	}

	return IRQ_HANDLED;
}
 
/* ------------- below are interface functions ----------------- */
/* ------------- these functions are named tccxxxcif_<name> -- */

/* Enables the camera. Takes camera out of reset. Enables the clocks. */ 
static int cif_enable(void *priv)
{
#if 0	
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) priv;

	/* give clock to camera_module */
	data->camera_regs->mode = (FIFO_TRIGGER_LVL << THRESHOLD_BIT);
	data->camera_regs->ctrlclock = MCLK_EN | CAMEXCLK_EN;1394

	omap16xx_cam_clear_fifo(data);
#endif
	/* wait for camera to settle down */
	mdelay(5);

	return 0;
}
 
/* Disables all the camera clocks. Put the camera interface in reset. */
static int cif_disable(void)
{ 	
	sensor_if_cleanup();
	
	return 0;
}

static int cif_cleanup(void)
{  
	cif_disable();
	return 0;
}

static void cif_check_handler(unsigned long arg)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) arg;
	unsigned long timeover = 0;
		
	if(data->cif_cfg.oper_mode == OPER_PREVIEW) 
	{
		if(sensor_if_isESD()) // Check Sensor-ESD register!!
		{
			data->cif_cfg.esd_restart = ON;
			timeover = 0;
		}
		else
			timeover = HZ;

		cif_timer_register(data, timeover);
	}
}

void cif_timer_register(void* priv, unsigned long timeover)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) priv;

	init_timer(&data->cam_timer);
	data->cam_timer.function = cif_check_handler;
	data->cam_timer.data = (unsigned long)data;
	data->cam_timer.expires = get_jiffies_64()+ timeover;

	add_timer(&data->cam_timer);
}

void cif_timer_deregister(void)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;

	del_timer(&data->cam_timer);
	data->cam_timer.expires = 0;
}


void cif_scaler_set(void *priv, enum cifoper_mode mode)
{
	cif_main_set *data = (cif_main_set *) priv;
	unsigned int sc_hfactor, sc_vfactor;
	unsigned int crop_hori_start, crop_hori_end, crop_vert_start, crop_vert_end;
	unsigned int h_ratio, v_ratio, temp_x;	
#if defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150) || defined(CONFIG_VIDEO_ATV_SENSOR_RDA5888)
	unsigned int crop_hor_ofst, crop_ver_ofst;

	data->scaler_x = data->target_x;
	data->scaler_y = data->target_y;
#else
#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP)
	if(mode == OPER_CAPTURE)
	{
		data->source_x = data->scaler_x = 1536; // 1535fixed Value, the pClks of One HSync
		data->source_y = data->scaler_y = (gnSizeOfAITALLJPEG+1535)/1536;

		crop_hori_start = crop_hori_end = crop_vert_start = crop_vert_end = 0;
	}
	else
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006)
	if(mode == OPER_CAPTURE)
	{
		data->source_x = data->scaler_x = CAP_W;
		data->source_y = data->scaler_y = CAP_H;

		crop_hori_start = crop_hori_end = crop_vert_start = crop_vert_end = 0;
	}
	else
#endif
	{
		if(((data->target_x * data->source_y) / data->source_x) == data->target_y)
		{
			data->scaler_x = data->target_x;
			data->scaler_y = data->target_y;
		}
		else
		{		
			h_ratio = (data->source_x * 100 / data->target_x);
			v_ratio = (data->source_y * 100 / data->target_y);
				
			if(h_ratio > v_ratio)
			{	
				data->scaler_y = data->target_y;
				data->scaler_x = (data->source_x * data->target_y)/data->source_y;
		
				temp_x = ((data->scaler_x+15) >> 4) << 4;
		
				if((temp_x - data->scaler_x) > 8)
					data->scaler_x = (data->scaler_x >> 4) << 4;
				else				
					data->scaler_x = temp_x;
			}
			else
			{
				data->scaler_x = data->target_x;
				data->scaler_y = (data->source_y * data->target_x)/data->source_x;
				
				data->scaler_y = (data->scaler_y >> 1) << 1;
			}
		}
		
		crop_hori_start = (data->scaler_x - data->target_x)/2;
		crop_hori_end 	= (data->scaler_x - data->target_x - crop_hori_start);
		crop_vert_start = (data->scaler_y - data->target_y)/2;
		crop_vert_end 	= (data->scaler_y - data->target_y - crop_vert_start);			
	}
#endif /*CONFIG_VIDEO_CAMERA_SENSOR_TVP5150*/
	
	sc_hfactor = (data->source_x*256/data->scaler_x);
	sc_vfactor = (data->source_y*256/data->scaler_y);

	TDD_CIF_SetFreeScale(SET_CIF_SCALER_SRC_SIZE|SET_CIF_SCALER_DST_SIZE,
							data->source_x, data->source_y, 0, 0, data->scaler_x, data->scaler_y, 0, 0);
	TDD_CIF_SetFreeScale(SET_CIF_SCALER_SRC_OFFSET,
							0, 0, data->win_hor_ofst, data->win_ver_ofst, 0, 0, 0, 0);
	TDD_CIF_SetFreeScale(SET_CIF_SCALER_FACTOR, 0, 0, 0, 0, 0, 0, sc_hfactor, sc_vfactor);
#if defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150) || defined(CONFIG_VIDEO_ATV_SENSOR_RDA5888)
	crop_hor_ofst = (data->scaler_x - data->target_x)/2;
	crop_ver_ofst = (data->scaler_y - data->target_y)/2;
	//TDD_CIF_SetImage(INPUT_IMG, data->scaler_x, data->scaler_y, 
	TDD_CIF_SetImage(INPUT_IMG, data->source_x, data->source_y,
								crop_hor_ofst, (data->scaler_x - data->target_x - crop_hor_ofst), 
								crop_ver_ofst, (data->scaler_y - data->target_y - crop_ver_ofst));
	TDD_CIF_SetBaseAddr_offset(INPUT_IMG, data->scaler_x, data->scaler_x/2);
#else
	//Crop
	TDD_CIF_SetImage(INPUT_IMG, data->scaler_x, data->scaler_y, crop_hori_start, crop_hori_end, crop_vert_start, crop_vert_end);

	#if defined(CONFIG_ARCH_TCC88XX)
		TDD_CIF_SetBaseAddr_offset(INPUT_IMG, data->target_x, data->target_x/2);
	#else
		TDD_CIF_SetBaseAddr_offset(INPUT_IMG, data->scaler_x, data->scaler_x/2);
	#endif
	
	dprintk("Crop Information -> crop_hori_start : %d, crop_hori_end : %d, crop_vert_start : %d, crop_vert_end : %d \n", crop_hori_start, crop_hori_end, crop_vert_start, crop_vert_end);
#endif

#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP) || defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006) || defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150) || defined(CONFIG_VIDEO_ATV_SENSOR_RDA5888)
	if(mode == OPER_CAPTURE)
	{
		TDD_CIF_SetScalerCtrl(SET_CIF_SCALER_DISABLE);
	}
	else
#endif
	{
		TDD_CIF_SetScalerCtrl(SET_CIF_SCALER_ENABLE);
	}
}

void cif_scaler_calc(void)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
	enum cifoper_mode mode = data->cif_cfg.oper_mode;
	unsigned int off_x = 0, off_y = 0, width = 0, height = 0;

#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP)
	unsigned int nCntOfHsync;

	if(mode == OPER_CAPTURE)
	{
		nCntOfHsync = (gnSizeOfAITALLJPEG+1535)/1536;
		width = 1536; // 1535fixed Value, the pClks of One HSync
		height = nCntOfHsync;
		
		data->cif_cfg.zoom_step = 0;
	}
	else
	{
		off_x  = PRV_ZOFFX;
		off_y  = PRV_ZOFFY;
		width  = PRV_W;
		height = PRV_H;
	}
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006)
	if(mode == OPER_CAPTURE)
	{
		width  = CAP_W; 
		height = CAP_H;
		
		data->cif_cfg.zoom_step = 0;
	}
	else
	{
		off_x  = PRV_ZOFFX;
		off_y  = PRV_ZOFFY;
		width  = PRV_W;
		height = PRV_H;
	}
#else
#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
	if(mode == OPER_PREVIEW
		|| (mode == OPER_CAPTURE && data->cif_cfg.main_set.target_x < tcc_sensor_info.cam_capchg_width) )
	{
		off_x  = tcc_sensor_info.preview_zoom_offset_x;
		off_y  = tcc_sensor_info.preview_zoom_offset_y;
		width  = tcc_sensor_info.preview_w;
		height = tcc_sensor_info.preview_h;
	}
	else
	{
		off_x  = tcc_sensor_info.capture_zoom_offset_x;
		off_y  = tcc_sensor_info.capture_zoom_offset_y;
		width  = tcc_sensor_info.capture_w;
		height = tcc_sensor_info.capture_h;
	}
#else // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
	if(mode == OPER_PREVIEW
		|| (mode == OPER_CAPTURE && data->cif_cfg.main_set.target_x < CAM_CAPCHG_WIDTH) )
	{
		off_x  = PRV_ZOFFX;
		off_y  = PRV_ZOFFY;
		width  = PRV_W;
		height = PRV_H;
	}
	else
	{
		off_x  = CAP_ZOFFX;
		off_y  = CAP_ZOFFY;
		width  = CAP_W;
		height = CAP_H;
	}
#endif // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
#endif

	data->cif_cfg.main_set.win_hor_ofst = off_x * data->cif_cfg.zoom_step;	
	data->cif_cfg.main_set.win_ver_ofst = off_y * data->cif_cfg.zoom_step;	
	data->cif_cfg.main_set.source_x 	= width - (off_x * data->cif_cfg.zoom_step)*2;
	data->cif_cfg.main_set.source_y 	= height - (off_y * data->cif_cfg.zoom_step)*2;

#if defined(CONFIG_USE_ISP)
	if(mode == OPER_PREVIEW)
	{
		ISP_SetPreview_Window(data->cif_cfg.main_set.win_hor_ofst, data->cif_cfg.main_set.win_ver_ofst,
								data->cif_cfg.main_set.source_x, data->cif_cfg.main_set.source_y );
	}
	else
	{
		ISP_SetCapture_Window(data->cif_cfg.main_set.win_hor_ofst, data->cif_cfg.main_set.win_ver_ofst,
								data->cif_cfg.main_set.source_x, data->cif_cfg.main_set.source_y );
	}
#else
	TDD_CIF_SetSensorOutImgSize(width, height);

	cif_scaler_set(&data->cif_cfg.main_set, mode);
#endif	
}

void cif_dma_hw_reg(unsigned char frame_num)
{	
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;

	data->cif_cfg.now_frame_num = frame_num;

#if defined(CONFIG_USE_ISP)
	ISP_SetPreviewH_StartAddress((unsigned int)data->cif_cfg.preview_buf[frame_num].p_Y,
									(unsigned int)data->cif_cfg.preview_buf[frame_num].p_Cb,
									(unsigned int)data->cif_cfg.preview_buf[frame_num].p_Cr);
#else
	TDD_CIF_SetBaseAddr(INPUT_IMG, (unsigned int)data->cif_cfg.preview_buf[frame_num].p_Y,
									(unsigned int)data->cif_cfg.preview_buf[frame_num].p_Cb,
									(unsigned int)data->cif_cfg.preview_buf[frame_num].p_Cr);
#if defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150)
//	TDD_CIF_SetBaseAddr_offset(INPUT_IMG, data->cif_cfg.main_set.target_x, data->cif_cfg.main_set.target_x/2);
#endif
#endif //CONFIG_USE_ISP
}

void cif_preview_dma_set(void)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
	unsigned char i;
	dma_addr_t base_addr = data->cif_cfg.base_buf;
	unsigned int y_offset;
	unsigned int uv_offset = 0;
	unsigned int total_off = 0;
	
	data->cif_cfg.now_frame_num = 0;	
	y_offset = data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y;

	if(data->cif_cfg.fmt == M420_ZERO)
		uv_offset = (data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y)/2;
	else
		uv_offset = (data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y)/4;

	dprintk("RDA5888E Interrupt, Width = %d, Height = %d \n",data->cif_cfg.main_set.target_x, data->cif_cfg.main_set.target_y);
        
	total_off = y_offset + uv_offset*2;
  	total_off = PAGE_ALIGN(total_off);      
	for(i=0; i < data->cif_cfg.pp_num; i++) {
		data->cif_cfg.preview_buf[i].p_Y  = (unsigned int)PAGE_ALIGN( base_addr + total_off*i);
		data->cif_cfg.preview_buf[i].p_Cb = (unsigned int)data->cif_cfg.preview_buf[i].p_Y + y_offset;
		data->cif_cfg.preview_buf[i].p_Cr = (unsigned int)data->cif_cfg.preview_buf[i].p_Cb + uv_offset;

		#if (0) // defined(CONFIG_USE_ISP) //20101126 ysseung   test code.
		isp[i].y_addr = ioremap_nocache(data->cif_cfg.preview_buf[i].p_Y, 4096);
		isp[i].u_addr = ioremap_nocache(data->cif_cfg.preview_buf[i].p_Cb, 4096);
		isp[i].v_addr = ioremap_nocache(data->cif_cfg.preview_buf[i].p_Cr, 4096);
		#endif
	}
}

void cif_capture_dma_set(void *priv)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
	dma_addr_t base_addr;
	unsigned int y_offset;
	unsigned int uv_offset = 0;
	unsigned int target_width, target_height;

#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP)
	base_addr = PA_JPEG_RAW_BASE_ADDR;
	target_width  = 1536;
	target_height = (gnSizeOfAITALLJPEG+1535)/1536;
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006)
	base_addr = PA_PREVIEW_BASE_ADDR;
	target_width  = CAP_W;
	target_height = CAP_H;
#else
	base_addr = data->cif_cfg.base_buf;
	target_width  = data->cif_cfg.main_set.target_x;
	target_height = data->cif_cfg.main_set.target_y;
#endif

#if defined(JPEG_ENCODE_WITH_CAPTURE)
	if(gIsRolling) {
		y_offset = JPEG_Info.JpegCaptureBuffSize/2;
		if(data->cif_cfg.fmt == M420_ZERO)
		    uv_offset = JPEG_Info.JpegCaptureBuffSize/4;
		else
		    uv_offset = JPEG_Info.JpegCaptureBuffSize/4/2;
	}
	else
#endif
	{
		y_offset = target_width*target_height;
		if(data->cif_cfg.fmt == M420_ZERO)
		    uv_offset = (target_width * target_height) / 2;
		else
		    uv_offset = (target_width * target_height) / 4;
	} 

	#if (0) //20111212 ysseung   test...
	data->cif_cfg.capture_buf.p_Y  = (unsigned int)base_addr;
	data->cif_cfg.capture_buf.p_Cb = (unsigned int)data->cif_cfg.capture_buf.p_Y + y_offset;
	data->cif_cfg.capture_buf.p_Cr = (unsigned int)data->cif_cfg.capture_buf.p_Cb + uv_offset;
	#endif

#if defined(CONFIG_USE_ISP)
	//ISP_SetCapture_Window(data->cif_cfg.main_set.win_hor_ofst*2, data->cif_cfg.main_set.win_ver_ofst*2, 
	//						data->cif_cfg.main_set.source_x*2, data->cif_cfg.main_set.source_y*2);	// for capture
	//sISP_SetCapture_Window(0, 0, target_width, target_height);
	//						data->cif_cfg.main_set.source_x*2, data->cif_cfg.main_set.source_y*2);
	printk("cif_capture_dma_set:  yAddr=0x%08x. \n", (unsigned int)data->cif_cfg.capture_buf.p_Y);
	ISP_SetCapture_MainImageStartAddress((unsigned int)data->cif_cfg.capture_buf.p_Y,
									(unsigned int)data->cif_cfg.capture_buf.p_Cb,
									(unsigned int)data->cif_cfg.capture_buf.p_Cr);
	ISP_SetCapture_MainResolution(target_width, target_height);

	if(data->cif_cfg.fmt == M420_ZERO) {
		ISP_SetCapture_MainImageSize(target_width*target_height, target_width*target_height/2, target_width*target_height/2);
		ISP_SetCapture_MainFormat(ISP_FORMAT_YUV422);
	} else {
		ISP_SetCapture_MainImageSize(target_width*target_height, target_width*target_height/4, target_width*target_height/4);
		ISP_SetCapture_MainFormat(ISP_FORMAT_YUV420);
	}

	#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
	if(data->cif_cfg.main_set.target_x < tcc_sensor_info.cam_capchg_width) {
		ISP_SetCapture_MakeZoomTable(tcc_sensor_info.preview_zoom_offset_x, tcc_sensor_info.preview_zoom_offset_y, tcc_sensor_info.preview_w, tcc_sensor_info.preview_h, tcc_sensor_info.max_zoom_step+1);
	} else {
		ISP_SetCapture_MakeZoomTable(tcc_sensor_info.capture_zoom_offset_x, tcc_sensor_info.capture_zoom_offset_y, tcc_sensor_info.capture_w, tcc_sensor_info.capture_h, tcc_sensor_info.max_zoom_step+1);
	}
	#else // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
	if(data->cif_cfg.main_set.target_x < CAM_CAPCHG_WIDTH) {
		ISP_SetCapture_MakeZoomTable(PRV_ZOFFX, PRV_ZOFFY, PRV_W, PRV_H, CAM_MAX_ZOOM_STEP);
	} else {
		ISP_SetCapture_MakeZoomTable(CAP_ZOFFX, CAP_ZOFFY, CAP_W, CAP_H, CAM_MAX_ZOOM_STEP);
	}
	#endif // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT 

	ISP_SetCapture_Zoom(data->cif_cfg.zoom_step);	
#else // CONFIG_USE_ISP
	TDD_CIF_SetBaseAddr(INPUT_IMG,  (unsigned int)data->cif_cfg.capture_buf.p_Y,
									(unsigned int)data->cif_cfg.capture_buf.p_Cb,
									(unsigned int)data->cif_cfg.capture_buf.p_Cr);

	#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP) || defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006)
	TDD_CIF_SetBaseAddr_offset(INPUT_IMG, target_width*2, target_width/2);	
	TDD_CIF_SetEffectMode(SET_CIF_CEM_YCS);
	#elif defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150) || defined(CONFIG_VIDEO_ATV_SENSOR_RDA5888)
	TDD_CIF_SetBaseAddr_offset(INPUT_IMG, target_width, target_width/2);
	#else
	TDD_CIF_SetBaseAddr_offset(INPUT_IMG, target_width, target_width/2);
	#endif

	#if defined(CONFIG_VIDEO_CAMERA_SENSOR_OV7690)
	TDD_CIF_SetEffectMode(SET_CIF_CEM_YCS);
	#endif

	#ifdef JPEG_ENCODE_WITH_CAPTURE
	if(gIsRolling)
	{
		if(data->cif_cfg.fmt == M420_ZERO)
		{
			TDD_CIF_SetBaseAddr(IN_IMG_ROLLING,
										((unsigned int)data->cif_cfg.capture_buf.p_Y+(JPEG_Info.JpegCaptureBuffSize/2)-4),
										((unsigned int)data->cif_cfg.capture_buf.p_Cb+(JPEG_Info.JpegCaptureBuffSize/2/2)-4),
										((unsigned int)data->cif_cfg.capture_buf.p_Cr+(JPEG_Info.JpegCaptureBuffSize/2/2)-4));
		}
		else
		{
			TDD_CIF_SetBaseAddr(IN_IMG_ROLLING,
										((unsigned int)data->cif_cfg.capture_buf.p_Y+(JPEG_Info.JpegCaptureBuffSize/2)-4),
										((unsigned int)data->cif_cfg.capture_buf.p_Cb+(JPEG_Info.JpegCaptureBuffSize/2/4)-4),
										((unsigned int)data->cif_cfg.capture_buf.p_Cr+(JPEG_Info.JpegCaptureBuffSize/2/4)-4));
		}
	}
	#endif
#endif // CONFIG_USE_ISP
}

void cif_interrupt_enable(enum cifoper_mode mode)
{
#if defined(CONFIG_USE_ISP)
	ISP_INTERRUPT_SET();
#else
	TDD_CIF_SetMaskIntStatus(SET_CIF_INT_ALL_MASK);

	if(mode == OPER_PREVIEW)
	{
		TDD_CIF_SetMaskIntStatus(SET_CIF_INT_STORE_1FRM_NOT_MASK);
	}
	else
	{
#ifdef JPEG_ENCODE_WITH_CAPTURE
		if(gIsRolling)
		{
			TDD_CIF_SetMaskIntStatus(SET_CIF_INT_VCNT_NOT_MASK);
		}
		else
#endif
		{
			TDD_CIF_SetMaskIntStatus(SET_CIF_INT_ENC_STRT_NOT_MASK);
			TDD_CIF_SetInterrupt(SET_CIF_UPDATE_WITHOUT_VSYNC);
#if defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150)
			TDD_CIF_SetInterrupt(SET_CIF_INT_TYPE_HOLDUP);
#endif
		}
	}

	TDD_CIF_SetInterrupt(SET_CIF_INT_ENABLE);
#endif //CONFIG_USE_ISP	
}

void cif_interrupt_disable(void)
{
#if defined(CONFIG_USE_ISP)
	ISP_INTERRUPT_CLEAR();
#else
	TDD_CIF_SetInterrupt(SET_CIF_INT_DISABLE);
#endif
}

void cif_global_reset(void)
{
	bool bEnableOfATV = FALSE;
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;

#if defined(CONFIG_USE_ISP)
	// todo : 
#else
#if defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150)	
	TDD_CIF_SetInterrupt(SET_CIF_UPDATE_WITHOUT_VSYNC);	//TDD_CIF_SetInterrupt(SET_CIF_INT_TYPE_HOLDUP);
	TDD_CIF_SetInterrupt(SET_CIF_INT_TYPE_HOLDUP);
#else	/* kilee@add_end */
	TDD_CIF_SetInterrupt(SET_CIF_INT_TYPE_HOLDUP); //should be '1'
	TDD_CIF_SetInterrupt(SET_CIF_UPDATE_IN_VSYNC);
#endif

#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP) || defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006)
	if(data->cif_cfg.oper_mode == OPER_CAPTURE)
		TDD_CIF_SetInfo(SET_CIF_ALL, NON_SEPARATE, MSB_FIRST, MODE_YUV, FMT422, BAYER_RGB, MODE16, data->cif_cfg.order422, SWICH_BUS);
	else
		TDD_CIF_SetInfo(SET_CIF_ALL, SEPARATE, MSB_FIRST, MODE_YUV, FMT422, BAYER_RGB, MODE16, data->cif_cfg.order422, SWICH_BUS);
#else
	TDD_CIF_SetInfo(SET_CIF_ALL, SEPARATE, MSB_FIRST, MODE_YUV, FMT422, BAYER_RGB, MODE16, data->cif_cfg.order422, SWICH_BUS);
#endif
	
	//DMA configure
	TDD_CIF_SetTransfer(SET_CIF_TRANSFER_ALL, BURST8, LOCK_TR, BURST_TRANS);
	TDD_CIF_SetSyncPol(data->cif_cfg.polarity_href, data->cif_cfg.polarity_vsync);

	//Effect all clear
	TDD_CIF_SetEffectMode(SET_CIF_CEM_NOR);

#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP) || defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006)
	if(data->cif_cfg.oper_mode == OPER_CAPTURE)
		TDD_CIF_SetCtrl(SET_CIF_II_ALL, CIF_PWDN_DISABLE, CIF_BYPASS_SCALER_ENABLE,
						data->cif_cfg.polarity_pclk, FRAME_0, data->cif_cfg.fmt, CIF_656CONVERT_DISABLE, bEnableOfATV);
	else
		TDD_CIF_SetCtrl(SET_CIF_II_ALL, CIF_PWDN_DISABLE, CIF_BYPASS_SCALER_DISABLE,
							data->cif_cfg.polarity_pclk, FRAME_0, data->cif_cfg.fmt, CIF_656CONVERT_DISABLE, bEnableOfATV);
#elif defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150) //|| defined(CONFIG_VIDEO_ATV_SENSOR_RDA5888)
	bEnableOfATV = TRUE;
	#ifdef	CONFIG_VIDEO_ATV_SENSOR_TVP5150
		TDD_CIF_Set656FormatConfig(SET_CIF_656_FIELD_INFO| SET_CIF_656_H_BLANK | SET_CIF_656_V_BLANK, 0, 0, 0, 0, 0x0D, 0x09, 0x0B);
		TDD_CIF_SetCtrl(SET_CIF_II_ALL, CIF_PWDN_DISABLE, CIF_BYPASS_SCALER_ENABLE,
			data->cif_cfg.polarity_pclk, FRAME_0, data->cif_cfg.fmt, CIF_656CONVERT_ENABLE, bEnableOfATV);
		//DMA configure
		TDD_CIF_SetTransfer(SET_CIF_TRANSFER_ALL, BURST8, NON_LOCK_TR, BURST_TRANS);
		//HwCIF->ICPCR1 = 0xB8802406;	//0xB8802406;	// 0xB8802406
		//HwCIF->CCIR656FCR2 = 0x352B;	//0x352B;	// 0x352B
		//HwCIF->CDCR1 = 0x03;
	#else
		bEnableOfATV = FALSE;
		TDD_CIF_SetCtrl(SET_CIF_II_ALL, CIF_PWDN_DISABLE, CIF_BYPASS_SCALER_ENABLE,
		data->cif_cfg.polarity_pclk, FRAME_0, data->cif_cfg.fmt, CIF_656CONVERT_DISABLE, bEnableOfATV);
	#endif	

#else
	TDD_CIF_SetCtrl(SET_CIF_II_ALL, CIF_PWDN_DISABLE, CIF_BYPASS_SCALER_DISABLE,
						data->cif_cfg.polarity_pclk, FRAME_0, data->cif_cfg.fmt, CIF_656CONVERT_DISABLE, bEnableOfATV);
#endif
#endif //CONFIG_USE_ISP
}

#ifdef JPEG_ENCODE_WITH_CAPTURE
static irqreturn_t tccxxx_jpeg_handler(int irq, void *dev_id/*, struct pt_regs *reg*/)
{  	
	unsigned long lFlag, TempFlag;
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
	struct tccxxx_cif_buffer *cif_buf;

	lFlag = HwJP_INT_FLAG;

	if (lFlag & HwJP_INT_FLAG_CODED_BUF_STAT)
	{
#ifdef JPEG_ENCODE_WITH_CAPTURE
	data->cif_cfg.cap_status = CAPTURE_OVERFLOW;
	#ifdef JPEG_TESTCODE
		JPEG_BUS_Disable(JPEG_ENCODE_MODE); // temp!! NO-thumbnail!!
		JPEG_INTERRUPT_Disable(JPEG_ENCODE_MODE);
	#endif
		wake_up_interruptible(&data->frame_wait);
#endif // JPEG_ENCODE_WITH_CAPTURE
		dprintk(KERN_ALERT "JPEG ENCODING BUFFER-ERROR! \r\n");

	}
	else if (lFlag & HwJP_INT_FLAG_DECODING_ERR)
	{

	}
	else if (lFlag & HwJP_INT_FLAG_JOB_FINISHED)
	{
		//Save to memory
		if (gJpegCodecMode == JPEG_ENCODE_MODE)
		{
			gJpegEncodeDone = 1;
			data->cif_cfg.cap_status = CAPTURE_DONE;		

#ifdef JPEG_ENCODE_WITH_CAPTURE
	#ifdef JPEG_TESTCODE
			tccxxx_jpeg_make_header(&(data->cif_cfg.jpg_info.jpg_len), &(data->cif_cfg.jpg_info.jpg_hdr_len));
			JPEG_BUS_Disable(JPEG_ENCODE_MODE); // temp!! NO-thumbnail!!
			JPEG_INTERRUPT_Disable(JPEG_ENCODE_MODE);
	#endif
			wake_up_interruptible(&data->frame_wait);
#endif // JPEG_ENCODE_WITH_CAPTURE
			
			dprintk(KERN_ALERT "JPEG ENCODING DONE! \r\n");
		}
		else if (gJpegCodecMode == JPEG_DECODE_MODE)
		{
			gJpegDecodeDone = 1;
			dprintk(KERN_ALERT "JPEG DECODING DONE \r\n");
		}
	}
#ifdef TCC83XX
	else if (lFlag & HwJP_INT_FLAG_YBUF_ROLLING)
	{
		BITSET(HwJP_INT_MASK, HwJP_INT_MASK_YBUF_ROLLING);
	}
#endif

	TempFlag = HwJP_INT_ACK;

	return IRQ_HANDLED;
}

static int
tccxxx_jpeg_encode_method(void)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
	unsigned int  ucReturnVal = FALSE;
	unsigned int  temp_1, temp_2;

	JPEG_Info.JpegCaptureBuffSize = CAPTURE_MEM;

	if (JPEG_Info.JpegCaptureBuffSize < data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y*2)
		ucReturnVal = TRUE;

	if(ucReturnVal)
	{
		temp_1 = (JPEG_Info.JpegCaptureBuffSize/2)/data->cif_cfg.main_set.target_x;

		if (temp_1%FRAME_LINE_CNT != 0)
		{
			temp_2 = temp_1/FRAME_LINE_CNT;
			JPEG_Info.JpegCaptureBuffSize = (temp_2*FRAME_LINE_CNT)*data->cif_cfg.main_set.target_x*2;
		}
	}

	return ucReturnVal;
}

static int
tccxxx_jpeg_encode_init(enum jpeg_quantisation_val val)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;

	unsigned int uiOutputBufferSize, uiInputBufferSize;
	void *BufferY, *BufferU, *BufferV;

	gJpegCodecMode = JPEG_UNKNOWN_MODE;
	JPEG_Exif_Header_Info_Init();

	BufferY = (void*)data->cif_cfg.capture_buf.p_Y ;
	BufferU = (void*)data->cif_cfg.capture_buf.p_Cb;
	BufferV = (void*)data->cif_cfg.capture_buf.p_Cr;
	
	JPEG_BUS_Enable(JPEG_ENCODE_MODE);
	JPEG_SW_Reset(JPEG_ENCODE_MODE);
	gJpegEncodeDone = 0;
	
	uiInputBufferSize = (JPEG_Info.JpegCaptureBuffSize/2)/data->cif_cfg.main_set.target_x;
	uiOutputBufferSize = gJPEG_Buffer_Info.pBaseBitstreamDataSize/1024;

	gJPEG_ENC_Info.is_rolling = gIsRolling;
	gJPEG_ENC_Info.y_addr = (unsigned int)BufferY;
	gJPEG_ENC_Info.u_addr = (unsigned int)BufferU;
	gJPEG_ENC_Info.v_addr = (unsigned int)BufferV;
	gJPEG_ENC_Info.ifrm_hsize = data->cif_cfg.main_set.target_x;
	gJPEG_ENC_Info.ibuf_vsize = uiInputBufferSize;
	gJPEG_ENC_Info.q_value = val;
	gJPEG_ENC_Info.img_hsize = data->cif_cfg.main_set.target_x;
	gJPEG_ENC_Info.img_vsize = data->cif_cfg.main_set.target_y;
	gJPEG_ENC_Info.cbuf_addr = (unsigned int)gJPEG_Buffer_Info.pBaseBitstreamDataAddr;
	gJPEG_ENC_Info.cbuf_size = uiOutputBufferSize;
	gJPEG_ENC_Info.frame_cnt = FRAME_LINE_CNT;
	gJPEG_ENC_Info.chroma = HwJP_CHROMA_422;

#ifdef JPEG_TESTCODE
	data->cif_cfg.jpg_info.jpg_buf_addr = jpeg_remapped_base_address;
	data->cif_cfg.jpg_info.jpg_hdr_addr = jpeg_header_remapped_address;
#endif	

	JPEG_SET_Encode_Config(&gJPEG_ENC_Info);

	JPEG_INTERRUPT_Enable(JPEG_ENCODE_MODE);

	return 0;
}

int tccxxx_jpeg_make_header(unsigned int *uiBitStreamSize, unsigned int *uiHeaderSize)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
	void *HeaderBuffer;

	//while (!gJpegEncodeDone)

	if (gIsExifThumbnailProcess)
	{
		gEncodeOption.ThumbnailInfo.pThumbnail_Buff = (void *)((uint32)jpeg_header_remapped_address + 2048);
		gEncodeOption.ThumbnailInfo.thumbnail_image_size = JPEG_Encoded_Data_Size();
	}
	else
	{
		*uiBitStreamSize = JPEG_Encoded_Data_Size();

		gEncodeOption.BitStreamSize = *uiBitStreamSize;
		gEncodeOption.ThumbnailInfo.thumbnail_image_size = 0;
		gEncodeOption.ImageWidth = data->cif_cfg.main_set.target_x;
		gEncodeOption.ImageHeight = data->cif_cfg.main_set.target_y;
		gEncodeOption.Q_FactorValue = JPEG_DEFAULT;
	}

	HeaderBuffer = jpeg_header_remapped_address;

	*uiHeaderSize = TCCXXX_JPEG_Make_Header((uint32)HeaderBuffer, &gEncodeOption, &gJpegHeaderExifRsp);
	*uiBitStreamSize = gEncodeOption.BitStreamSize;

	gJpegEncodeDone = 0;
	gIsExifThumbnailProcess = FALSE;

	return 0;
}
#endif

#if (1) //20111209 ysseung   test...
int tccxxx_cif_buffer_set(struct v4l2_requestbuffers *req)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
	struct tccxxx_cif_buffer *buf = NULL;
	unsigned int y_offset = data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y;
	unsigned int uv_offset = 0;
	unsigned int buff_size;

	if(req->count == 0) {
		data->cif_cfg.now_frame_num = 0;

		if(data->cif_cfg.fmt == M420_ZERO)
			buff_size = PAGE_ALIGN(y_offset*2);
		else
			buff_size = PAGE_ALIGN(y_offset + y_offset/2);

		data->buf->v4lbuf.length = buff_size;

		memset(data->static_buf, 0x00, req->count * sizeof(struct tccxxx_cif_buffer));

		data->done_list.prev = data->done_list.next = &data->done_list;
		data->list.prev 	 = data->list.next 		= &data->list;

		data->cif_cfg.base_buf = (unsigned int)req->reserved[0];
	}

	buf = &(data->static_buf[req->count]);

	INIT_LIST_HEAD(&buf->buf_list);

	buf->mapcount 		= 0;
	buf->cam 			= data;
	buf->v4lbuf.index 	= req->count;
	buf->v4lbuf.type 	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf->v4lbuf.field 	= V4L2_FIELD_NONE;
	buf->v4lbuf.memory 	= V4L2_MEMORY_MMAP;
	buf->v4lbuf.length 	= buff_size;

	if(data->cif_cfg.fmt == M420_ZERO)
		uv_offset = (data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y)/2;
	else
		uv_offset = (data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y)/4;

	data->cif_cfg.preview_buf[req->count].p_Y  = (unsigned int)req->reserved[0];
	data->cif_cfg.preview_buf[req->count].p_Cb = (unsigned int)data->cif_cfg.preview_buf[req->count].p_Y + y_offset;
	data->cif_cfg.preview_buf[req->count].p_Cr = (unsigned int)data->cif_cfg.preview_buf[req->count].p_Cb + uv_offset;

	data->cif_cfg.pp_num = req->count;

	return 0;
}

void tccxxx_set_camera_addr(int index, unsigned int addr, unsigned int cameraStatus)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
	unsigned int y_offset = 0, uv_offset = 0;

	y_offset = data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y;
	if(data->cif_cfg.fmt == M420_ZERO)
		uv_offset = (data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y)/2;
	else
		uv_offset = (data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y)/4;

	if(cameraStatus == 1 /* MODE_PREVIEW */) {
		data->cif_cfg.preview_buf[index].p_Y  = addr;
		data->cif_cfg.preview_buf[index].p_Cb = (unsigned int)data->cif_cfg.preview_buf[index].p_Y + y_offset;
		data->cif_cfg.preview_buf[index].p_Cr = (unsigned int)data->cif_cfg.preview_buf[index].p_Cb + uv_offset;
	} else if(cameraStatus == 3 /* MODE_CAPTURE */) {
		data->cif_cfg.capture_buf.p_Y  = addr;
		data->cif_cfg.capture_buf.p_Cb = (unsigned int)data->cif_cfg.capture_buf.p_Y + y_offset;
		data->cif_cfg.capture_buf.p_Cr = (unsigned int)data->cif_cfg.capture_buf.p_Cb + uv_offset;
	}
}
#else
int tccxxx_cif_buffer_set(struct v4l2_requestbuffers *req)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
	dma_addr_t base_addr = data->cif_cfg.base_buf;
	unsigned int y_offset = data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y;
	unsigned int uv_offset = 0;
	unsigned int total_off = 0;
	unsigned char i;
	unsigned long total_req_buf_size=0;
	unsigned char req_buf = req->count;
	unsigned int buff_size;
	
	data->cif_cfg.now_frame_num = 0;

	if (req->count > TCC_CAMERA_MAX_BUFNBRS) 
		req->count = TCC_CAMERA_MAX_BUFNBRS;

	if (req->count < 0) 
		req->count = 2;

	if(data->cif_cfg.fmt == M420_ZERO)
		buff_size = PAGE_ALIGN(y_offset*2);
	else
		buff_size = PAGE_ALIGN(y_offset + y_offset/2);

	data->buf->v4lbuf.length = buff_size;

retry:
	if(req_buf == 1)
	{
#if 0  // ZzaU :: Don't check Buffer in Rolling-Capture.
		if (data->buf->v4lbuf.length > CAPTURE_MEM_SIZE)
		{
			printk("reqbufs: count invalid\n");
			return -1;
		}
#endif
	}
	else
	{
		if (data->buf->v4lbuf.length*req->count > PREVIEW_MEM_SIZE) 
		{
			req->count--;
			if (req->count <= 0) 
			{
				printk("reqbufs: count invalid\n");
				return -1;
			}
			goto retry;
		}
	}

	memset(data->static_buf,0x00,req->count * sizeof(struct tccxxx_cif_buffer));
	
	data->done_list.prev = data->done_list.next = &data->done_list;
	data->list.prev 	 = data->list.next 		= &data->list;
		
	for (data->n_sbufs = 0; data->n_sbufs < req->count; (data->n_sbufs++)) 
	{
		struct tccxxx_cif_buffer *buf = &(data->static_buf[data->n_sbufs]);

		INIT_LIST_HEAD(&buf->buf_list);

		//buf->v4lbuf.length = PAGE_ALIGN(cam->pix_format.sizeimage);
		buf->v4lbuf.length = buff_size;
		
		total_req_buf_size += buf->v4lbuf.length;
		//dprintk(KERN_WARNING "<total size is 0x%x / 0x%x>\n", (unsigned int)(buf->v4lbuf.length), (unsigned int)(total_req_buf_size));	
	  		
		buf->mapcount = 0;
		buf->cam = data;
		buf->v4lbuf.index 	= data->n_sbufs;
		buf->v4lbuf.type 	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf->v4lbuf.field 	= V4L2_FIELD_NONE;
		buf->v4lbuf.memory 	= V4L2_MEMORY_MMAP;
		
		/*
		 * Offset: must be 32-bit even on a 64-bit system.	videobuf-dma-sg
		 * just uses the length times the index, but the spec warns
		 * against doing just that - vma merging problems.	So we
		 * leave a gap between each pair of buffers.
		 */
		//buf->v4lbuf.m.offset = 2*index*buf->v4lbuf.length;			
		
	}	

	//data->cif_cfg.pp_num = req->count;
	data->cif_cfg.pp_num = data->n_sbufs;
	req->count = data->cif_cfg.pp_num;


	if(data->cif_cfg.fmt == M420_ZERO)
		uv_offset = (data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y)/2;
	else
		uv_offset = (data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y)/4;
        
	total_off = y_offset + uv_offset*2;
  	total_off = PAGE_ALIGN(total_off);        
	for(i = 0; i<data->cif_cfg.pp_num; i++)
	{
		data->cif_cfg.preview_buf[i].p_Y  = (unsigned int)PAGE_ALIGN( base_addr + total_off*i);
		data->cif_cfg.preview_buf[i].p_Cb = (unsigned int)data->cif_cfg.preview_buf[i].p_Y + y_offset;
		data->cif_cfg.preview_buf[i].p_Cr = (unsigned int)data->cif_cfg.preview_buf[i].p_Cb + uv_offset;
	}	

	dprintk("buffer count = %d \n",data->cif_cfg.pp_num);
	
	return 0;	
}
#endif

int tccxxx_cif_cam_restart(struct v4l2_pix_format *pix, unsigned long xclk)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
	struct tccxxx_cif_buffer *cif_buf;

	dprintk("%s Start!! \n", __FUNCTION__);

	tccxxx_cif_stop_stream();

	sensor_if_restart();
	sensor_if_change_mode(OPER_PREVIEW);

	while(!list_empty(&data->done_list)) 
	{
		cif_buf = list_entry(data->done_list.next, struct tccxxx_cif_buffer, buf_list);
		list_del(&cif_buf->buf_list);
	
		cif_buf->v4lbuf.flags &= ~V4L2_BUF_FLAG_DONE;	
		cif_buf->v4lbuf.flags |= V4L2_BUF_FLAG_QUEUED;
		
		list_add_tail(&cif_buf->buf_list, &data->list);	
	}

	// Sensor setting Again!! 
	sensor_if_set_current_control();

	cif_dma_hw_reg(0);
	data->stream_state = STREAM_ON;
	cif_interrupt_enable(data->cif_cfg.oper_mode);

	data->cif_cfg.esd_restart = OFF;
	cif_timer_register(data, HZ);

	dprintk("%s End!! \n", __FUNCTION__);

#if defined(CONFIG_USE_ISP)
	#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
	ISP_SetPreview_Window(0, 0, tcc_sensor_info.preview_w, tcc_sensor_info.preview_h);
	#else
	ISP_SetPreview_Window(0, 0, PRV_W, PRV_H);
	#endif
	if(data->cif_cfg.fmt == M420_ZERO)
	{
		ISP_SetPreviewH_Size( data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y, 
	              	                         data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y/2, 
	                     	                  data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y/2);
		ISP_SetPreviewH_Format(ISP_FORMAT_YUV422);
	}
	else
	{
		ISP_SetPreviewH_Size( data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y, 
	              	                         data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y/4, 
	                     	                  data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y/4);
		ISP_SetPreviewH_Format(ISP_FORMAT_YUV420);		
	}
	printk ("prevSize:(%d,%d)=%x restart \n", data->cif_cfg.main_set.target_x, data->cif_cfg.main_set.target_y, data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y);

	ISP_SetPreviewH_Resolution( data->cif_cfg.main_set.target_x, data->cif_cfg.main_set.target_y);

	tccxxx_cif_set_effect(current_effect_mode);
#ifdef TCCISP_GEN_CFG_UPD	
	chkpnt = 0;
#endif
	ISP_SetPreview_Control(ON);
#endif

	return 0;
}

int tccxxx_cif_start_stream(void)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;

	dprintk("%s Start!! \n", __FUNCTION__);

	data->cif_cfg.oper_mode = OPER_PREVIEW;
	data->cif_cfg.cap_status = CAPTURE_NONE;

#if defined(CONFIG_USE_ISP)

	sensor_if_change_mode(OPER_PREVIEW);

	//cif_scaler_calc();
	//cif_scaler_set(NULL, data->cif_cfg.oper_mode); //20101124 ysseung   test code.
	#if (0) //20111209 ysseung   test...
	cif_preview_dma_set();
	#endif

	cif_dma_hw_reg(0);
	
	data->stream_state = STREAM_ON;
		
	cif_interrupt_enable(data->cif_cfg.oper_mode);
	cif_timer_register(data, HZ);

	cif_set_frameskip(0, 0);
	skipped_frm = 0;
	
	while(skip_frm)
	{
		msleep(1);
	}
	
	//ISP_SetPreview_Window(0, 0, PRV_W, PRV_H);
	
	if(data->cif_cfg.fmt == M420_ZERO)
	{
		ISP_SetPreviewH_Size( data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y, 
	              	                         data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y/2, 
	                     	                  data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y/2);
		ISP_SetPreviewH_Format(ISP_FORMAT_YUV422);
	}
	else
	{
		ISP_SetPreviewH_Size( data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y, 
	              	                         data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y/4, 
	                     	                  data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y/4);
		ISP_SetPreviewH_Format(ISP_FORMAT_YUV420);		
	}
	printk ("prevSize:(%d,%d)=%x\n", data->cif_cfg.main_set.target_x, data->cif_cfg.main_set.target_y, data->cif_cfg.main_set.target_x*data->cif_cfg.main_set.target_y);
	
	ISP_SetPreviewH_Resolution( data->cif_cfg.main_set.target_x, data->cif_cfg.main_set.target_y);
	#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
	ISP_SetPreview_MakeZoomTable(tcc_sensor_info.preview_zoom_offset_x, tcc_sensor_info.preview_zoom_offset_y, tcc_sensor_info.preview_w, tcc_sensor_info.preview_h, tcc_sensor_info.max_zoom_step+1);
	#else
	ISP_SetPreview_MakeZoomTable(PRV_ZOFFX, PRV_ZOFFY, PRV_W, PRV_H, CAM_MAX_ZOOM_STEP);
	#endif
	ISP_SetPreview_Zoom(data->cif_cfg.zoom_step);
	
	tccxxx_cif_set_effect(current_effect_mode);
#ifdef TCCISP_GEN_CFG_UPD	
	chkpnt = 0;
#endif
	ISP_SetPreview_Control(ON);
#else
	TDD_CIF_ONOFF(OFF);
	TDD_CIF_SWReset();
	TDD_CIF_SetCaptureCtrl(SET_CIF_SKIP_NUM, 0, 0, SET_CIF_CAP_DISABLE); // after capture!!
	
	sensor_if_change_mode(OPER_PREVIEW);
#if !defined(CONFIG_VIDEO_ATV_SENSOR_TVP5150)
	TDD_CIF_SetInterrupt(SET_CIF_UPDATE_IN_VSYNC);
#endif
	cif_global_reset();

	tccxxx_cif_set_effect(current_effect_mode);
	
	cif_scaler_calc();
	cif_preview_dma_set();
	cif_dma_hw_reg(0);

	#if defined(CONFIG_VIDEO_CAMERA_SENSOR_OV7690)
		TDD_CIF_SetEffectMode(SET_CIF_CEM_YCS);
	#endif
	
	TDD_CIF_ONOFF(ON);

	data->stream_state = STREAM_ON;
		
	cif_interrupt_enable(data->cif_cfg.oper_mode);
	cif_timer_register(data, HZ);

	#if defined(CONFIG_VIDEO_CAMERA_SENSOR_GT2005)
		cif_set_frameskip(35, 0);		// Temporary Setting!! By Mun
	#else
		cif_set_frameskip(0, 0);
	#endif

	skipped_frm = 0;
	
	while(skip_frm)
	{
		msleep(1);
	}
#endif //CONFIG_USE_ISP	

	dprintk("%s End!! \n", __FUNCTION__);
	
	return 0;
}

int tccxxx_cif_stop_stream(void)
{	
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;

#if defined(CONFIG_USE_ISP)
	ISP_SetPreview_Control(OFF);
#endif
	cif_timer_deregister();
	cif_interrupt_disable();
	
	data->stream_state= STREAM_OFF;	

	dprintk("\n\n SKIPPED FRAME = %d \n\n", skipped_frm);

/*	
	mutex_lock(&data->lock);	

	INIT_LIST_HEAD(&data->done_list);
	INIT_LIST_HEAD(&data->list);

	for(i=0; i<data->cif_cfg.pp_num;i++)
	{
		INIT_LIST_HEAD(&data->buf[i].buf_list);
		data->buf[i].v4lbuf.flags &= ~V4L2_BUF_FLAG_QUEUED;
		data->buf[i].v4lbuf.flags &= ~V4L2_BUF_FLAG_DONE;
	}
	mutex_unlock(&data->lock);	
*/		

	return 0;
}

void tccxxx_cif_set_overlay(void)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
	cif_SuperImpose *SiData = (cif_SuperImpose *)&(data->cif_cfg.si_overlay);

	unsigned int frame_width = data->cif_cfg.main_set.scaler_x;  // Scaler_out width
	unsigned int frame_height = data->cif_cfg.main_set.scaler_y;  // Scaler_out width
	unsigned int siFrame_hoffset_s 	= 0;
	unsigned int siFrame_hoffset_e 		= 0;   
	unsigned int siFrame_voffset_s 	= 0;
	unsigned int siFrame_voffset_e 		= 0;
	unsigned int si_buffer_addr 	= data->cif_buf.addr + SiData->buff_offset;

	siFrame_hoffset_s = siFrame_hoffset_e = (frame_width - SiData->width)/2;
	siFrame_voffset_s = siFrame_voffset_e = (frame_height - SiData->height)/2;

#if defined(CONFIG_USE_ISP)
	// todo : 
#else
	TDD_CIF_OverlayCtrl(SET_CIF_OVERLAY_DISABLE, OL_RGB_565);

	if(SiData->buff_offset != 0)
	{
		TDD_CIF_SetImage(OVERLAY_IMG, frame_width, frame_height,
						siFrame_hoffset_s, siFrame_hoffset_e,	siFrame_voffset_s, siFrame_voffset_e);
		TDD_CIF_SetOverlay (SET_CIF_OVERLAY_ALL, 49, FULL_OVERLAY, OP_ALPHA, OP_ALPHA, ALPHA100, ALPHA100);
		TDD_CIF_SetOverlayKey(SET_CIF_OVERLAYKEY_ALL, SiData->chromakey_info.key_y, 
													  SiData->chromakey_info.key_u, 
													  SiData->chromakey_info.key_v, 
													  SiData->chromakey_info.mask_r, 
													  SiData->chromakey_info.mask_g, 
													  SiData->chromakey_info.mask_b);
		
		TDD_CIF_OverlayCtrl(SET_CIF_CHROMA_ENABLE|SET_CIF_OVERLAY_ENABLE|SET_CIF_COLOR_CONV_ENABLE|
						SET_CIF_COLOR_MODE_RGB,	OL_RGB_565);
		TDD_CIF_SetBaseAddr(OVERLAY_IMG, (unsigned int)si_buffer_addr, 0, 0);				
	}
#endif	
}

int tccxxx_cif_set_effect (u8 nCameraEffect)
{
	unsigned int uiSepiaU = 0x64;
	unsigned int uiSepiaV = 0x8C;
	unsigned int uiSKCoeff0 = 0x02;
	unsigned int uiSKCoeff1 = 0x0;
	unsigned int uiSKCoeff2 = 0xff;
	unsigned int uiSkecthThreshold = 0x80;

#ifdef CONFIG_USE_ISP
	ISP_SetImg_Effect(nCameraEffect);
#else
	if (nCameraEffect == TCC_EFFECT_NORMAL)
		TDD_CIF_SetEffectMode(SET_CIF_CEM_NOR);
	else if(nCameraEffect == TCC_EFFECT_GRAY)
		TDD_CIF_SetEffectMode(SET_CIF_CEM_GRAY);
	else if(nCameraEffect == TCC_EFFECT_NEGATIVE)
		TDD_CIF_SetEffectMode(SET_CIF_CEM_NEGA);
	else if(nCameraEffect == TCC_EFFECT_SKETCH)
	{
		uiSkecthThreshold = 0x89;
		TDD_CIF_SetEffectMode(SET_CIF_CEM_SKT);
		TDD_CIF_SetEffectHFilterCoeff(uiSKCoeff0, uiSKCoeff1, uiSKCoeff2);
		TDD_CIF_SetEffectSketchTh(uiSkecthThreshold);
	}
	else if(nCameraEffect == TCC_EFFECT_SEPHIA)
	{
		uiSepiaU =  0x64;
		uiSepiaV = 0x8C;
		TDD_CIF_SetEffectMode(SET_CIF_CEM_SEPI);
		TDD_CIF_SetEffectSepiaUV(uiSepiaU, uiSepiaV);
	}
	else if(nCameraEffect == TCC_EFFECT_GREEN)
	{		
		uiSepiaU =  100;
		uiSepiaV = 80;
		TDD_CIF_SetEffectMode(SET_CIF_CEM_SEPI);
		TDD_CIF_SetEffectSepiaUV(uiSepiaU, uiSepiaV);

	}
	else if(nCameraEffect == TCC_EFFECT_AQUA)
	{
		uiSepiaU =  192;
		uiSepiaV = 80;
		TDD_CIF_SetEffectMode(SET_CIF_CEM_SEPI);
		TDD_CIF_SetEffectSepiaUV(uiSepiaU, uiSepiaV);

	}
#endif	
	current_effect_mode = (u8) nCameraEffect;

	return 0;
}

int tccxxx_cif_capture(int quality)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;
	unsigned int target_width, target_height;
	unsigned int ens_addr;
	int skip_frame = 0;
#if defined(CONFIG_USE_ISP)
	int mode=0;

	data->cif_cfg.oper_mode = OPER_CAPTURE;

	memset(&(data->cif_cfg.jpg_info), 0x00, sizeof(TCCXXX_JPEG_ENC_DATA));

	target_width  = data->cif_cfg.main_set.target_x;
	target_height = data->cif_cfg.main_set.target_y;

	#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
	if(target_width >= tcc_sensor_info.cam_capchg_width && !(data->cif_cfg.retry_cnt))
	#else
	if(target_width >= CAM_CAPCHG_WIDTH && !(data->cif_cfg.retry_cnt))
	#endif
	{
		#if defined(CONFIG_VIDEO_CAMERA_SENSOR_MT9T113)
			unsigned char data_r[2];
			unsigned short data_read;
			unsigned char polling_reg[2] = {0x09, 0x8E};
			unsigned char polling_cmd[2] = {0x30, 0x00};
			unsigned short read_reg = 0x0990;

			do {
				DDI_I2C_Write(polling_reg, 2, 0);	
				DDI_I2C_Write(polling_cmd, 2, 0);		
				DDI_I2C_Read(read_reg, 2, data_r, 2);

				data_read = data_r[0];
				data_read = (data_read<<8)&0xff00 | data_r[1];

				printk("MT9T113 Autofocus 0[0x%x] 1[0x%x] data_read = 0x%x \n",data_r[0], data_r[1], data_read);

				if(data_read == 0x0001)
					break;
				else
					msleep(10);
								
			} while(data_read != 0x0001);
		#endif
		
		sensor_if_change_mode(OPER_CAPTURE);
		msleep(300); //20101125 ysseung   test code.
		mode = 1;
	}

	//cif_scaler_calc();
	//cif_scaler_set(NULL, data->cif_cfg.oper_mode); //20101124 ysseung   test code.
	cif_capture_dma_set(data);

	// for Rotate Capture
	data->cif_cfg.jpg_info.start_phy_addr = data->cif_cfg.base_buf;
	
	cif_interrupt_enable(data->cif_cfg.oper_mode);
	
	if(data->cif_cfg.si_overlay.buff_offset != 0)
		tccxxx_cif_set_overlay();

	data->cif_cfg.cap_status = CAPTURE_NONE;

#ifdef TCCISP_GEN_CFG_UPD	
	chkpnt==0;
#endif

	// Turn on Camera Flash
	sensor_turn_on_camera_flash();
	
	ISP_Capture_Shooting(mode);
	ISP_SetPreview_Control(OFF);
#else
	cif_interrupt_disable();
	CIF_OpStop(1, 1);
	
	data->cif_cfg.oper_mode = OPER_CAPTURE;

	cif_global_reset();
	tccxxx_cif_set_effect(current_effect_mode);

	memset(&(data->cif_cfg.jpg_info), 0x00, sizeof(TCCXXX_JPEG_ENC_DATA));
	
#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP)
	sensor_if_capture_config(data->cif_cfg.main_set.target_x, data->cif_cfg.main_set.target_y);
	target_width  = 1536;
	target_height = (gnSizeOfAITALLJPEG+1535)/1536;

	data->cif_cfg.jpg_info.start_phy_addr	= PA_JPEG_RAW_BASE_ADDR;
	data->cif_cfg.jpg_info.bitstream_offset = 2;
	data->cif_cfg.jpg_info.thumb_offset 	= gnoffsetOfAITThumbnail;
	data->cif_cfg.jpg_info.header_offset 	= gnSizeOfAITALLJPEG;
	data->cif_cfg.jpg_info.bitstream_size 	= gnSizeOfAITMainJPEG;
	data->cif_cfg.jpg_info.thumb_size 		= gnSizeOfAITThumbnail;
	data->cif_cfg.jpg_info.header_size 		= 0;
#elif defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006)
	sensor_if_capture_config(data->cif_cfg.main_set.target_x,data->cif_cfg.main_set.target_y);

	target_width  = CAP_W;
	target_height = CAP_H;

	data->cif_cfg.jpg_info.start_phy_addr	= PA_PREVIEW_BASE_ADDR;
	data->cif_cfg.jpg_info.bitstream_offset = 0;
	data->cif_cfg.jpg_info.thumb_offset 	= 0;
	data->cif_cfg.jpg_info.header_offset	= 0;
	data->cif_cfg.jpg_info.bitstream_size	= 0;
	data->cif_cfg.jpg_info.thumb_size		= 0;
	data->cif_cfg.jpg_info.header_size		= 0;
#else
	target_width  = data->cif_cfg.main_set.target_x;
	target_height = data->cif_cfg.main_set.target_y;
#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
	if(target_width >= tcc_sensor_info.cam_capchg_width && !(data->cif_cfg.retry_cnt)){
		sensor_if_change_mode(OPER_CAPTURE);
		msleep(300); 
	}
#else // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
	if(target_width >= CAM_CAPCHG_WIDTH && !(data->cif_cfg.retry_cnt)){
		sensor_if_change_mode(OPER_CAPTURE);
		msleep(300); 
	}
#endif // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT	
#endif
	cif_scaler_calc();

#ifdef JPEG_ENCODE_WITH_CAPTURE
	gIsRolling = tccxxx_jpeg_encode_method();
#endif
	cif_capture_dma_set(data);

	// for Rotate Capture
	data->cif_cfg.jpg_info.start_phy_addr = data->cif_cfg.base_buf;
	//capture config
	if(data->cif_cfg.retry_cnt)
	{
#if defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006)
		sensor_if_change_mode(OPER_PREVIEW);
		msleep(100);
#endif	
		skip_frame = 0;
	}
	else
	{
#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
		skip_frame = tcc_sensor_info.capture_skip_frame;
#else // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
		skip_frame = FRAMESKIP_COUNT_FOR_CAPTURE;
#endif // CONFIG_VIDEO_DUAL_CAMERA_SUPPORT
	}
	
#ifdef JPEG_ENCODE_WITH_CAPTURE
	if(gIsRolling)
	{
		TDD_CIF_SetInterrupt(SET_CIF_UPDATE_WITHOUT_VSYNC);
		TDD_CIF_SetCaptureCtrl(SET_CIF_SKIP_NUM|SET_CIF_VCNT_NUM, skip_frame, FRAME_LINE_CNT/16,
								SET_CIF_EIT_ENC_INT|SET_CIF_RLV_ENABLE|SET_CIF_RLU_ENABLE|SET_CIF_RLY_ENABLE|
								SET_CIF_CAP_ENABLE|SET_CIF_VEN_ENABLE);
	}
	else
#endif
	{
#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP) || defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006)
		ens_addr = (unsigned int)data->cif_cfg.capture_buf.p_Y + (target_width * (target_height - 100));
#else
		ens_addr = (unsigned int)data->cif_cfg.capture_buf.p_Y + (target_width * (target_height - 2));
#endif

		TDD_CIF_SetCaptureCtrl(SET_CIF_SKIP_NUM, skip_frame, 0,SET_CIF_EIT_ENC_INT|SET_CIF_CAP_ENABLE|SET_CIF_UES_ENABLE);		
		TDD_CIF_SetBaseAddr(IN_ENC_START_ADDR, ens_addr, 0, 0);
	}

	cif_interrupt_enable(data->cif_cfg.oper_mode);

#ifdef JPEG_ENCODE_WITH_CAPTURE
	tccxxx_jpeg_encode_init(quality);
#endif

	if(data->cif_cfg.si_overlay.buff_offset != 0)
		tccxxx_cif_set_overlay();

	data->cif_cfg.cap_status = CAPTURE_NONE;
#if defined(CONFIG_VIDEO_CAMERA_SENSOR_AIT848_ISP) || defined(CONFIG_VIDEO_CAMERA_SENSOR_ISX006)
	sensor_if_change_mode(OPER_CAPTURE);
#endif

	// Turn on Camera Flash
	sensor_turn_on_camera_flash();

	TDD_CIF_ONOFF(ON);
	msleep(100); 
#endif //CONFIG_USE_ISP

	return 0;
}
		

int tccxxx_cif_set_zoom(unsigned char arg)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;

	data->cif_cfg.zoom_step = arg;

	#ifdef CONFIG_USE_ISP
		ISP_SetPreview_Zoom(data->cif_cfg.zoom_step);
	#else
	if(data->stream_state != STREAM_OFF)
	{
		dprintk("zoom level = %d. \n", data->cif_cfg.zoom_step);
		TDD_CIF_ONOFF(OFF);
		cif_scaler_calc();
		TDD_CIF_ONOFF(ON);
	}
	#endif

	return 0;
}

int tccxxx_cif_set_resolution(unsigned int pixel_fmt, unsigned short width, unsigned short height)
{
	struct TCCxxxCIF *data = &hardware_data;

	//if(data->cif_cfg.main_set.target_x == width 
	//	&& data->cif_cfg.fmt == pixel_fmt)
	//	return 0;

	if(pixel_fmt == V4L2_PIX_FMT_YUYV) // YUV422
		data->cif_cfg.fmt = M420_ZERO;
	else
		data->cif_cfg.fmt = M420_ODD;

	data->cif_cfg.main_set.target_x = width;
	data->cif_cfg.main_set.target_y = height;

	if(data->stream_state != STREAM_OFF) {
		tccxxx_cif_stop_stream();
		tccxxx_cif_start_stream();
	}

	return 0;	
}

int tccxxx_cif_open(void)
{
	int ret;
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;

	data->done_list.next	= &data->done_list;
	data->list.next		= &data->list;

#if defined(CONFIG_USE_ISP)
	if(!cam_irq)
	{
		if ((ret = request_irq(IRQ_ISP1, isp_cam_isr1, IRQF_DISABLED, "isp2", NULL)) < 0) 
		{
			printk("FAILED to aquire camera-irq\n");
			return ret;
		}
		if ((ret = request_irq(IRQ_ISP2, isp_cam_isr2, IRQF_DISABLED, "isp3", NULL)) < 0) 
		{
			printk("FAILED to aquire camera-irq\n");
			return ret;
		}		
		cam_irq = 1;
	}

#else
	if(!cam_irq)
	{
		#if defined(CONFIG_ARCH_TCC88XX)
			if ((ret = request_irq(IRQ_CIF, cif_cam_isr, IRQF_DISABLED, "camera", NULL)) < 0) 
		#else
			if ((ret = request_irq(IRQ_CAM, cif_cam_isr, IRQF_DISABLED, "camera", NULL)) < 0) 
		#endif
		{
			printk("FAILED to aquire camera-irq\n");
			return ret;
		}
		cam_irq = 1;
	}
	
#if defined(JPEG_ENCODE_WITH_CAPTURE) && !defined(NLY_ENCODE_JPEG_IN_ROLLING_CAPTURE)
	free_irq(IRQ_JPGE, NULL);
	if ((ret = request_irq(IRQ_JPE, tccxxx_jpeg_handler, IRQF_DISABLED, "jpeg", NULL)) < 0) 
	{
		printk("FAILED to aquire irq\n");
		return ret;
	}
#endif
#endif //CONFIG_USE_ISP

	return 0;
}

int tccxxx_cif_close(void)
{
	struct TCCxxxCIF *data = (struct TCCxxxCIF *) &hardware_data;	

/*
    int i;

	mutex_lock(&data->lock);	

	INIT_LIST_HEAD(&data->done_list);
	INIT_LIST_HEAD(&data->list);

	for(i=0; i<data->cif_cfg.pp_num;i++)
	{
		INIT_LIST_HEAD(&data->buf[i].buf_list);
		data->buf[i].v4lbuf.flags &= ~V4L2_BUF_FLAG_QUEUED;
		data->buf[i].v4lbuf.flags &= ~V4L2_BUF_FLAG_DONE;
	}
	
	mutex_unlock(&data->lock);	
*/

#if defined(CONFIG_USE_ISP)
	ISP_Exit();
#endif

	cif_interrupt_disable();
	cif_cleanup();

	dprintk("reamp : [0x%x - 0x%x] -> [0x%x] \n",data->cif_buf.addr, data->cif_buf.bytes, (unsigned int)data->cif_buf.area);		
	iounmap(data->cif_buf.area);

#if defined(CONFIG_USE_ISP)
	//free_irq(IRQ_ISP0, NULL);
	free_irq(IRQ_ISP1, NULL);
	free_irq(IRQ_ISP2, NULL);
	//free_irq(IRQ_ISP3, NULL);
#else
	#if defined(CONFIG_ARCH_TCC88XX)
		free_irq(IRQ_CIF, NULL);
	#else
		free_irq(IRQ_CAM, NULL);
	#endif
#endif

	cam_irq = 0;
	
#ifdef JPEG_ENCODE_WITH_CAPTURE
	free_irq(IRQ_JPGE, NULL);
#endif

	cif_timer_deregister();

	return 0;
}

#if defined(CONFIG_VIDEO_DUAL_CAMERA_SUPPORT)
int tcc_get_sensor_info(int index)
{
	CameraID = index;
	if(CameraID)
		sensor_if_set_facing_front();
	else
		sensor_if_set_facing_back();
}

int tccxxx_sensor_init(void * priv)
{
	 TCC_SENSOR_INFO_TYPE *data = (struct TCC_SENSOR_INFO_TYPE *) priv;
}
#endif

/* Initialise the OMAP camera interface */
int  tccxxx_cif_init(void)
{
	struct cif_dma_buffer *buf = &hardware_data.cif_buf;    

	pmap_get_info("camera", &pmap_cam);

	memset(&hardware_data,0x00,sizeof(struct TCCxxxCIF));
	hardware_data.buf = hardware_data.static_buf;

#if (0) //20111209 ysseung   test...
#ifdef JPEG_ENCODE_WITH_CAPTURE
	buf->bytes = PAGE_ALIGN(CAPTURE_MEM+JPEG_MEM);
	buf->addr = PA_VIDEO_BASE;
	buf->area = ioremap_nocache(buf->addr,buf->bytes);

	jpeg_remapped_base_address = (void *)ioremap_nocache(JPEG_PHY_ADDRESS, PAGE_ALIGN(JPEG_MEM-PAGE_SIZE));    //JPEG Buffer
	jpeg_header_remapped_address = jpeg_remapped_base_address + JPEG_STREAM;

	gJPEG_Buffer_Info.pBaseRawDataAddr = (void*)CAPTURE_PHY_ADDRESS;
	gJPEG_Buffer_Info.pBaseRawDataSize = CAPTURE_MEM;
	gJPEG_Buffer_Info.pBaseBitstreamDataAddr = (void*)JPEG_PHY_ADDRESS;
	gJPEG_Buffer_Info.pBaseBitstreamDataSize = JPEG_STREAM;
	gJPEG_Buffer_Info.pBaseHeaderDataAddr = (void*)(JPEG_PHY_ADDRESS + JPEG_STREAM);
	gJPEG_Buffer_Info.pBaseHeaderDataSize = JPEG_HEADER;
#else
	buf->bytes = PAGE_ALIGN(PREVIEW_MEM_SIZE);
	buf->addr = PA_PREVIEW_BASE_ADDR;
	buf->area = ioremap_nocache(buf->addr,buf->bytes);
	dprintk("reamp : [0x%x - 0x%x] -> [0x%x] \n",buf->addr, buf->bytes, (unsigned int)buf->area);		
#endif

	if (buf->area == NULL) 
	{
		printk(KERN_ERR CAM_NAME ": cannot remap buffer\n");
		return ENODEV;
	}
#endif

	/* Init the camera IF */
	cif_data_init((void*)&hardware_data);
	/* enable it. This is needed for sensor detection */
	cif_enable((void*)&hardware_data);

	init_waitqueue_head(&hardware_data.frame_wait);
	spin_lock_init(&hardware_data.dev_lock);

	INIT_LIST_HEAD(&hardware_data.list);
	INIT_LIST_HEAD(&hardware_data.done_list);	
	mutex_init(&hardware_data.lock);

	return 0;
}

