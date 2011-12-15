
/****************************************************************************
 *   FileName    : tca_backlight.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
/*****************************************************************************
*
* Header Files Include
*
******************************************************************************/
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

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include <mach/bsp.h>
#include <mach/tca_ckc.h>
#include <mach/tca_lcdc.h>

#include <linux/cpufreq.h>
#include <linux/wait.h>
#include <linux/kthread.h>

#include <plat/pmap.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tcc_grp_ioctrl.h>
#include <mach/tcc_scaler_ctrl.h>
#include <mach/tca_fb_output.h>
#include <mach/tcc_composite_ioctl.h>
#include <mach/tcc_component_ioctl.h>
#include <mach/tca_fb_hdmi.h>
#include <mach/tccfb_ioctrl.h>

#include <mach/vioc_outcfg.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_disp.h>
#include <mach/vioc_global.h>
#include <mach/vioc_config.h>
#include <mach/vioc_scaler.h>

#include "../tcc_mouse_icon.h"



#if 0
#define dprintk(msg...)	 { printk( "tca_fb_output: " msg); }
#else
#define dprintk(msg...)	 
#endif


#define VIOC_SCALER_PLUG_IN

#ifdef VIOC_SCALER_PLUG_IN
	#define FB_SCALE_MAX_WIDTH		1920
	#define FB_SCALE_MAX_HEIGHT		1080
#else
	#if (defined(CONFIG_ARCH_TCC88XX) || defined(CONFIG_ARCH_TCC93XX)|| defined(CONFIG_ARCH_TCC892X))// && defined(CONFIG_DRAM_DDR2)
	#define FB_SCALE_MAX_WIDTH		1920
	#define FB_SCALE_MAX_HEIGHT		1080
	#else
	#define FB_SCALE_MAX_WIDTH		1280
	#define FB_SCALE_MAX_HEIGHT		720
	#endif
#endif//
void grp_rotate_ctrl(G2D_BITBLIT_TYPE *g2d_p);

static char *fb_scaler_pbuf0;
static char *fb_scaler_pbuf1;
static char *fb_g2d_pbuf0;
static char *fb_g2d_pbuf1;

#define MOUSE_CURSOR_MAX_WIDTH			30
#define MOUSE_CURSOR_MAX_HEIGHT			30
#define MOUSE_CURSOR_WIDTH_30			30
#define MOUSE_CURSOR_HEIGHT_30			30
#define MOUSE_CURSOR_WIDTH_20			20
#define MOUSE_CURSOR_HEIGHT_20			20
#define MOUSE_CURSOR_WIDTH_12			12
#define MOUSE_CURSOR_HEIGHT_12			12

#define MOUSE_CURSOR_BUFF_SIZE		(MOUSE_CURSOR_MAX_WIDTH*MOUSE_CURSOR_MAX_HEIGHT*4)

static char *pMouseBuffer;
static unsigned int mouse_cursor_width;
static unsigned int mouse_cursor_height;

dma_addr_t		Gmap_dma;	/* physical */
u_char *		Gmap_cpu;	/* virtual */


/*****************************************************************************
*
* Defines
*
******************************************************************************/
//#define TCC_FB_UPSCALE

/*****************************************************************************
*
* structures
*
******************************************************************************/


typedef struct {
	unsigned int output;
	unsigned int ctrl;
	unsigned int width;
	unsigned int height;
	unsigned int interlace;
} output_video_img_info;

struct output_struct {
	spinlock_t lock;
	wait_queue_head_t waitq;
	char state;
};
static struct output_struct Output_struct;
static struct output_struct Output_lcdc0_struct;
static struct output_struct Output_lcdc1_struct;

/*****************************************************************************
*
* Variables
*
******************************************************************************/

/*****************************************************************************
*
* Functions
*
******************************************************************************/
typedef struct {
	unsigned int LCDC_N;
	VIOC_DISP *pVIOC_DispBase;
	VIOC_WMIX* pVIOC_WMIXBase;
	VIOC_RDMA* pVIOC_RDMA_FB;
	VIOC_RDMA* pVIOC_RDMA_Video;
	VIOC_RDMA* pVIOC_RDMA_Mouse;
}DisplayOutPut_S;

static DisplayOutPut_S pDISP_OUTPUT[TCC_OUTPUT_MAX];

static struct clk *lcdc0_output_clk;
static struct clk *lcdc1_output_clk;


static char output_lcdc[TCC_OUTPUT_MAX];
static char buf_index = 0;
static unsigned int FBimg_buf_addr;
static unsigned int output_layer_ctrl[TCC_OUTPUT_MAX] = {0, 0};

static unsigned char OutputType = 0;
static unsigned char UseVSyncInterrupt = 0;
static unsigned int uiOutputResizeMode = 0;
static lcdc_chroma_params output_chroma;
static output_video_img_info output_video_img;

struct inode scaler_inode;
struct file scaler_filp;

int (*scaler_ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
int (*scaler_open) (struct inode *, struct file *);
int (*scaler_release) (struct inode *, struct file *);

int tccxxx_scaler1_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
int tccxxx_scaler1_release(struct inode *inode, struct file *filp);
int tccxxx_scaler1_open(struct inode *inode, struct file *filp);
	

struct inode g2d_inode;
struct file g2d_filp;

int tccxxx_grp_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
int tccxxx_grp_release(struct inode *inode, struct file *filp);
int tccxxx_grp_open(struct inode *inode, struct file *filp);

int (*g2d_ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
int (*g2d_open) (struct inode *, struct file *);
int (*g2d_release) (struct inode *, struct file *);

static irqreturn_t TCC_OUTPUT_LCDC_Handler(int irq, void *dev_id)
{
	unsigned int LCDCstatus;

	if(pDISP_OUTPUT[OutputType].pVIOC_DispBase == NULL)
	{
		printk("%s - Err: Output LCDC is not valid, OutputType=%d\n", __func__, OutputType);
		return IRQ_HANDLED;
	}

	LCDCstatus = pDISP_OUTPUT[OutputType].pVIOC_DispBase->uLSTATUS.nREG;

 	dprintk("%s lcdc_struct.state:%d STATUS:0x%x OutputType:%d \n",__func__, Output_struct.state, LCDCstatus, OutputType);

	BITCSET(pDISP_OUTPUT[OutputType].pVIOC_DispBase->uLSTATUS.nREG, 0xFFFFFFFF, 0xFFFFFFFF);

	if(Output_struct.state == 0){
		Output_struct.state = 1;
		wake_up_interruptible(&Output_struct.waitq);
	}

	return IRQ_HANDLED;
}


void TCC_OUTPUT_LCDC_Init(void)
{
	pmap_t pmap;
	dprintk(" %s \n", __func__);
	lcdc0_output_clk = clk_get(0, "lcdc0");
	if (IS_ERR(lcdc0_output_clk))
		lcdc0_output_clk = NULL;

	lcdc1_output_clk = clk_get(0, "lcdc1");
	if (IS_ERR(lcdc1_output_clk))
		lcdc1_output_clk = NULL;


	pmap_get_info("fb_scale0", &pmap);
	fb_scaler_pbuf0 = (char *) pmap.base;
	pmap_get_info("fb_scale1", &pmap);
	fb_scaler_pbuf1 = (char *) pmap.base;

	pmap_get_info("fb_g2d0", &pmap);
	fb_g2d_pbuf0 = (char *) pmap.base;
	pmap_get_info("fb_g2d1", &pmap);
	fb_g2d_pbuf1 = (char *) pmap.base;

	#if defined(CONFIG_LCD_LCDC0_USE)
		pDISP_OUTPUT[TCC_OUTPUT_NONE].pVIOC_DispBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP0);
		pDISP_OUTPUT[TCC_OUTPUT_NONE].pVIOC_RDMA_FB= (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA00);
		pDISP_OUTPUT[TCC_OUTPUT_NONE].pVIOC_WMIXBase= (VIOC_WMIX *)tcc_p2v(HwVIOC_WMIX0);	
	#else
		pDISP_OUTPUT[TCC_OUTPUT_NONE].pVIOC_DispBase = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP1);
		pDISP_OUTPUT[TCC_OUTPUT_NONE].pVIOC_RDMA_FB= (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA04);
		pDISP_OUTPUT[TCC_OUTPUT_NONE].pVIOC_WMIXBase= (VIOC_WMIX *)tcc_p2v(HwVIOC_WMIX1);			
	#endif	
	memset((void *)&output_chroma, 0x00, sizeof((void *)&output_chroma));

	init_waitqueue_head(&Output_struct.waitq);
	Output_struct.state = 1;

	init_waitqueue_head(&Output_lcdc0_struct.waitq);
	Output_lcdc0_struct.state = 1;

	init_waitqueue_head(&Output_lcdc1_struct.waitq);
	Output_lcdc1_struct.state = 1;

	scaler_ioctl = tccxxx_scaler1_ioctl;
	scaler_open = tccxxx_scaler1_open;
	scaler_release = tccxxx_scaler1_release;

	g2d_ioctl = tccxxx_grp_ioctl;
	g2d_open = tccxxx_grp_open;
	g2d_release = tccxxx_grp_release;

	{
		unsigned int size = MOUSE_CURSOR_BUFF_SIZE;
		Gmap_cpu = dma_alloc_writecombine(0, size, &Gmap_dma, GFP_KERNEL);
		memset(Gmap_cpu, 0x00, MOUSE_CURSOR_BUFF_SIZE);

		pMouseBuffer = (char *)Gmap_dma;
		TCC_OUTPUT_FB_MouseIconSelect(0);
	}
 }



void TCC_OUTPUT_UPDATE_OnOff(char onoff)
{
	dprintk(" %s \n", __func__);
#ifdef VIOC_SCALER_PLUG_IN	
	#if defined(CONFIG_LCD_LCDC0_USE)
	if(onoff)	{
			VIOC_CONFIG_PlugIn (VIOC_SC0, VIOC_SC_RDMA_04);
		}
		else 	{
			VIOC_CONFIG_PlugOut (VIOC_SC0);
		}
	#else
		if(onoff)	{
			VIOC_CONFIG_PlugIn (VIOC_SC0, VIOC_SC_RDMA_00);
		}
		else 	{
			VIOC_CONFIG_PlugOut (VIOC_SC0);
		}
	#endif
	
#else
	if(onoff)	{
		g2d_open((struct inode *)&g2d_inode, (struct file *)&g2d_filp);
		scaler_open((struct inode *)&scaler_inode, (struct file *)&scaler_filp);
	}
	else 	{
		scaler_release((struct inode *)&scaler_inode, (struct file *)&scaler_filp);
		g2d_release((struct inode *)&g2d_inode, (struct file *)&g2d_filp);
	}
#endif//
}

#ifdef CONFIG_CPU_FREQ
extern struct tcc_freq_table_t gtHdmiClockLimitTable;
#endif//CONFIG_CPU_FREQ
void TCC_OUTPUT_LCDC_OnOff(char output_type, char output_lcdc_num, char onoff)
{
	int i;

	dprintk(" %s output_type:%d lcdc_reg:0x%08x output_lcdc_num:%d onoff:%d  \n", __func__, (unsigned int)output_type, pDISP_OUTPUT[output_type].pVIOC_DispBase, pDISP_OUTPUT[output_type].LCDC_N, (unsigned int)onoff);
	
	if(onoff)
	{
		OutputType = output_type;
				
		if(output_lcdc_num) {
			// hdmi , composite					
			clk_enable(lcdc1_output_clk);

			pDISP_OUTPUT[output_type].pVIOC_DispBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
			pDISP_OUTPUT[output_type].pVIOC_WMIXBase= (VIOC_WMIX *)tcc_p2v(HwVIOC_WMIX1);	
			pDISP_OUTPUT[output_type].pVIOC_RDMA_FB = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA04);
			pDISP_OUTPUT[output_type].pVIOC_RDMA_Video = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA06);
			pDISP_OUTPUT[output_type].pVIOC_RDMA_Mouse = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA07);
		}
		else
		{	// LCD
			clk_enable(lcdc0_output_clk);
			pDISP_OUTPUT[output_type].pVIOC_DispBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
			pDISP_OUTPUT[output_type].pVIOC_WMIXBase= (VIOC_WMIX *)tcc_p2v(HwVIOC_WMIX0);	
			pDISP_OUTPUT[output_type].pVIOC_RDMA_FB = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA00);
			pDISP_OUTPUT[output_type].pVIOC_RDMA_Video = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA02);	
			pDISP_OUTPUT[output_type].pVIOC_RDMA_Mouse = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA03);
		}
		
		BITCSET(pDISP_OUTPUT[output_type].pVIOC_DispBase->uCTRL.nREG,0xFFFFFFFF, 0);
		
		VIOC_RDMA_SetImageDisable( pDISP_OUTPUT[output_type].pVIOC_RDMA_FB);
		VIOC_RDMA_SetImageDisable( pDISP_OUTPUT[output_type].pVIOC_RDMA_Video);
		VIOC_RDMA_SetImageDisable( pDISP_OUTPUT[output_type].pVIOC_RDMA_Mouse);

		pDISP_OUTPUT[output_type].LCDC_N = output_lcdc_num;

		#ifdef CONFIG_CPU_FREQ
		tcc_cpufreq_set_limit_table(&gtHdmiClockLimitTable, TCC_FREQ_LIMIT_HDMI, 1);
		#endif//CONFIG_CPU_FREQ

		TCC_OUTPUT_UPDATE_OnOff(1);

		tca_lcdc_interrupt_onoff(0, output_lcdc_num);
		
		if(pDISP_OUTPUT[output_type].LCDC_N) {
			request_irq(INT_VIOC_DEV1, TCC_OUTPUT_LCDC_Handler,	IRQF_SHARED,
			"TCC_LCD1",	TCC_OUTPUT_LCDC_Handler);
		}
		else 	{
			request_irq(INT_VIOC_DEV0, TCC_OUTPUT_LCDC_Handler,	IRQF_SHARED,
			"TCC_LCD0",	TCC_OUTPUT_LCDC_Handler);
		}
	}
	else
	{		
		if(pDISP_OUTPUT[output_type].pVIOC_DispBase != NULL)
		{
			VIOC_DISP_TurnOff(pDISP_OUTPUT[output_type].pVIOC_DispBase);
		
			VIOC_RDMA_SetImageDisable( pDISP_OUTPUT[output_type].pVIOC_RDMA_FB);
			VIOC_RDMA_SetImageDisable( pDISP_OUTPUT[output_type].pVIOC_RDMA_Video);
			VIOC_RDMA_SetImageDisable( pDISP_OUTPUT[output_type].pVIOC_RDMA_Mouse);
		}
	
		i = 0;
		while(i < 0xF0000)
		{
			volatile unsigned int status;

			//status = pLCDC_OUTPUT[output_type]->LSTATUS;
			status = pDISP_OUTPUT[output_type].pVIOC_DispBase->uLSTATUS.nREG;
			
			if(status & HwLSTATUS_DD)	{
				dprintk(" lcdc disabled ! \n");
				break;
			}
			else	{
				i++;
			}
		}

		pDISP_OUTPUT[output_type].pVIOC_DispBase= NULL;
		pDISP_OUTPUT[output_type].pVIOC_WMIXBase= NULL;	
		pDISP_OUTPUT[output_type].pVIOC_RDMA_FB = NULL;
		pDISP_OUTPUT[output_type].pVIOC_RDMA_Video = NULL;
		pDISP_OUTPUT[output_type].pVIOC_RDMA_Mouse = NULL;

	#if defined(CONFIG_CPU_FREQ) && !defined(CONFIG_TCC_OUTPUT_DUAL_UI)
		tcc_cpufreq_set_limit_table(&gtHdmiClockLimitTable, TCC_FREQ_LIMIT_HDMI, 0);
	#endif//CONFIG_CPU_FREQ

		TCC_OUTPUT_UPDATE_OnOff(0);
		dprintk("lcd disable time %d \n", i);
		
		if(output_lcdc_num)	{						
			clk_disable(lcdc1_output_clk);
		}
		else {
			clk_disable(lcdc0_output_clk);
		}

		if(output_lcdc_num)
			free_irq(INT_VIOC_DEV1, TCC_OUTPUT_LCDC_Handler);
		else
			free_irq(INT_VIOC_DEV0, TCC_OUTPUT_LCDC_Handler);
	}

	memset((void *)output_layer_ctrl, 0x00, sizeof(output_layer_ctrl));
}

void TCC_OUTPUT_LCDC_CtrlLayer(char output_type, char interlace, char format)
{
	dprintk(" %s interlace:%d format:%d  \n", __func__, interlace, format);
	
	output_layer_ctrl[output_type] = HwDMA_IEN | format;

	if(interlace)
		output_layer_ctrl[output_type] |= HwDMA_INTL;
}

void TCC_OUTPUT_LCDC_CtrlChroma(lcdc_chroma_params lcdc_chroma)
{
	dprintk("%s\n", __func__);
	
	output_chroma.enable = lcdc_chroma.enable;
	output_chroma.color = lcdc_chroma.color;
}

char TCC_FB_G2D_FmtConvert(unsigned int width, unsigned int height, unsigned int g2d_rotate, unsigned int Sfmt, unsigned int Tfmt, unsigned int Saddr, unsigned int Taddr)
{
	G2D_BITBLIT_TYPE g2d_p;
	
	memset(&g2d_p, 0x00, sizeof(G2D_BITBLIT_TYPE));

	g2d_p.responsetype = G2D_POLLING;
	g2d_p.src0 = (unsigned int)Saddr;
	
	if(Sfmt == TCC_LCDC_IMG_FMT_RGB888)
		g2d_p.srcfm.format = GE_RGB888;
	else if(Sfmt == TCC_LCDC_IMG_FMT_YUV422SP)
		g2d_p.srcfm.format = GE_YUV422_sq;		
	else
		g2d_p.srcfm.format = GE_RGB565;

	g2d_p.srcfm.data_swap = 0;
	g2d_p.src_imgx = width;
	g2d_p.src_imgy = height;

	g2d_p.ch_mode = g2d_rotate;

	g2d_p.crop_offx = 0;
	g2d_p.crop_offy = 0;
	g2d_p.crop_imgx = width;
	g2d_p.crop_imgy = height;

	g2d_p.tgt0 = (unsigned int)Taddr;	// destination image address

	if(Tfmt == TCC_LCDC_IMG_FMT_RGB888)
		g2d_p.tgtfm.format = GE_RGB888;
	else if(Tfmt == TCC_LCDC_IMG_FMT_YUV422SP)
		g2d_p.tgtfm.format = GE_YUV422_sq;		
	else
		g2d_p.tgtfm.format = GE_RGB565;

	// destinaion f
	g2d_p.tgtfm.data_swap = 0;
	if((g2d_rotate == ROTATE_270) || (g2d_rotate == ROTATE_90)) 	{
		g2d_p.dst_imgx = height;
		g2d_p.dst_imgy = width;
	}
	else		{
		g2d_p.dst_imgx = width;
		g2d_p.dst_imgy = height;
	}
	
	g2d_p.dst_off_x = 0;
	g2d_p.dst_off_y = 0;
	g2d_p.alpha_value = 255;

	g2d_p.alpha_type = G2d_ALPHA_NONE;

	#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
		grp_rotate_ctrl(&g2d_p);
	#else
		g2d_ioctl((struct inode *)&g2d_inode, (struct file *)&g2d_filp, TCC_GRP_ROTATE_IOCTRL_KERNEL, &g2d_p);
	#endif

	return 1;
}


char TCC_OUTPUT_FB_Update(unsigned int width, unsigned int height, unsigned int bits_per_pixel, unsigned int addr, unsigned int type)
{
	unsigned int regl = 0, g2d_rotate_need = 0, g2d_format_need = 0, scaler_need= 0, m2m_onthefly_use = 0, interlace_output = 0;
	unsigned int lcd_width = 0, lcd_height = 0;
	unsigned int img_width = 0, img_height = 0, ifmt = 0;
	unsigned int  chromaR = 0, chromaG = 0, chromaB = 0;
	unsigned int  chroma_en = 0, alpha_blending_en = 0, alpha_type = 0;

	SCALER_TYPE fbscaler;
	VIOC_SC *pSC;

	pSC= (VIOC_SC *)tcc_p2v(HwVIOC_SC0);

	memset(&fbscaler, 0x00, sizeof(SCALER_TYPE));

	if(pDISP_OUTPUT[type].pVIOC_DispBase == NULL)
	{
		dprintk("%s - Err: Output LCDC is not valid, type=%d\n", __func__, type);
		return 0;
	}
	
	dprintk(" %s width=%d, height=%d, bpp=%d, type=%d\n", __func__, width, height, bits_per_pixel, type);

	regl = pDISP_OUTPUT[type].pVIOC_DispBase->uLSIZE.nREG; // get LCD size 
	lcd_width = (regl & 0xFFFF);
	lcd_height = ((regl>>16) & 0xFFFF);

	if(((!lcd_width) || (!lcd_height)) || ((lcd_width > TCC_FB_OUT_MAX_WIDTH) || (lcd_height > TCC_FB_OUT_MAX_HEIGHT)))
	{
		dprintk(" %s ERROR width=%d, height=%d, bpp=%d, type=%d LCD W:%d H:%d \n", __func__, width, height, bits_per_pixel, type, lcd_width, lcd_height);
		return 0;
	}
	
	if(width < height)
		g2d_rotate_need = 1;

	if(lcd_width != width || lcd_height != height)
		scaler_need = 1;


	if(bits_per_pixel == 32 )	{
		chroma_en = 0;
		alpha_type = 1;
		alpha_blending_en = 1;
		ifmt = TCC_LCDC_IMG_FMT_RGB888;		
	}
	else  {
		chroma_en = 1;
		alpha_type = 0;
		alpha_blending_en = 0;
		ifmt = TCC_LCDC_IMG_FMT_RGB565;
	}

#if 0
	if( !(pDISP_OUTPUT[type].pVIOC_DispBase->uCTRL.nREG & HwDISP_NI ))//interlace mode
		interlace_output = 1;
	else
#endif /* 0 */
		interlace_output = 0;

	if(g2d_rotate_need || g2d_format_need 
		#ifndef VIOC_SCALER_PLUG_IN
		|| scaler_need  
		#endif//
	)
	{
		UseVSyncInterrupt = 0;
	}
	else
	{
		UseVSyncInterrupt = 1;
	}

	img_width = width;
	img_height = height;
	FBimg_buf_addr = addr;

	dprintk(" %s width=%d, height=%d, bpp=%d, lcd_width=%d, lcd_height=%d, rotate=%d, format=%d, scale=%d, type=%d\n", 
			__func__, width, height, bits_per_pixel, lcd_width, lcd_height, g2d_rotate_need, g2d_format_need, scaler_need, type);
	
	if(g2d_rotate_need || g2d_format_need)
	{
		unsigned int rotate, taddr; 
		
		if(g2d_rotate_need)		{
			img_width = height;
			img_height= width;
			rotate = ROTATE_270;
		}
		else		{
			img_width = width;
			img_height = height;
			rotate = NOOP;
		}
		if(buf_index)
			taddr = fb_g2d_pbuf0;	// destination image address
		else
			taddr = fb_g2d_pbuf1;	// destination image address

		TCC_FB_G2D_FmtConvert(width, height, rotate, ifmt, TCC_LCDC_IMG_FMT_RGB888, addr, taddr );
	
		ifmt = TCC_LCDC_IMG_FMT_RGB888;
	
		FBimg_buf_addr = taddr;
	}

#ifndef VIOC_SCALER_PLUG_IN
	if(scaler_need)
	{
		//need to add scaler component
		fbscaler.responsetype = SCALER_NOWAIT;
		
		fbscaler.src_Yaddr = (char *)FBimg_buf_addr;

		if(ifmt == TCC_LCDC_IMG_FMT_YUV422SP)
			fbscaler.src_fmt = SCALER_YUV422_sq0;
		if(ifmt == TCC_LCDC_IMG_FMT_RGB888)
			fbscaler.src_fmt = SCALER_ARGB8888;		
		else
			fbscaler.src_fmt = SCALER_RGB565;

		fbscaler.src_ImgWidth = img_width;
		fbscaler.src_ImgHeight = img_height;

		fbscaler.src_winLeft = 0;
		fbscaler.src_winTop = 0;
		fbscaler.src_winRight = img_width;
		fbscaler.src_winBottom = img_height;

		if(buf_index)
			fbscaler.dest_Yaddr = fb_scaler_pbuf0;	// destination image address
		else
			fbscaler.dest_Yaddr = fb_scaler_pbuf1;	// destination image address

		buf_index = !buf_index;

		if((lcd_width > FB_SCALE_MAX_WIDTH) && (!m2m_onthefly_use))
		{
			m2m_onthefly_use = 0;
			img_width = lcd_width / 2;
		}
		else	{
			img_width = lcd_width;
		}

		if((lcd_height > FB_SCALE_MAX_HEIGHT) && (!m2m_onthefly_use)) 	{
			m2m_onthefly_use = 0;
			img_height = lcd_height/2;
		}
		else	{
			img_height = lcd_height;
		}

		//ifmt = TCC_LCDC_IMG_FMT_RGB888;
		fbscaler.dest_fmt = SCALER_ARGB8888;		// destination image format
		fbscaler.dest_ImgWidth = img_width;		// destination image width
		fbscaler.dest_ImgHeight = img_height; 	// destination image height
		fbscaler.dest_winLeft = 0;
		fbscaler.dest_winTop = 0;
		fbscaler.dest_winRight = img_width;
		fbscaler.dest_winBottom = img_height;

		FBimg_buf_addr = fbscaler.dest_Yaddr;
		
		scaler_ioctl((struct inode *)&scaler_inode, (struct file *)&scaler_filp, TCC_SCALER_IOCTRL_KERENL, &fbscaler);
	}
#else
	VIOC_SC_SetBypass (pSC, OFF);
	VIOC_SC_SetSrcSize(pSC, width, height);
	VIOC_SC_SetDstSize (pSC, lcd_width, lcd_height);			// set destination size in scaler
	VIOC_SC_SetOutSize (pSC, lcd_width, lcd_height);			// set output size in scaer

	VIOC_SC_SetUpdate (pSC);				// Scaler update
	VIOC_WMIX_SetUpdate (pDISP_OUTPUT[type].pVIOC_WMIXBase);			// WMIX update
#endif//

	// size
	VIOC_RDMA_SetImageSize(pDISP_OUTPUT[type].pVIOC_RDMA_FB, img_width, img_height);

	// format
	VIOC_RDMA_SetImageFormat(pDISP_OUTPUT[type].pVIOC_RDMA_FB, ifmt);
	if ( ifmt>= TCC_LCDC_IMG_FMT_YUV420SP && ifmt <= TCC_LCDC_IMG_FMT_YUV422ITL1)	{
		VIOC_RDMA_SetImageY2REnable(pDISP_OUTPUT[type].pVIOC_RDMA_FB, TRUE);
		VIOC_RDMA_SetImageY2RMode(pDISP_OUTPUT[type].pVIOC_RDMA_FB, 0); /* Y2RMode Default 0 (Studio Color) */
	}
	
	VIOC_RDMA_SetImageIntl(pDISP_OUTPUT[type].pVIOC_RDMA_FB, interlace_output);

	//offset
	VIOC_RDMA_SetImageOffset(pDISP_OUTPUT[type].pVIOC_RDMA_FB, ifmt, img_width);

	// alpha & chroma key color setting
	VIOC_RDMA_SetImageAlphaSelect(pDISP_OUTPUT[type].pVIOC_RDMA_FB, 1);
	VIOC_RDMA_SetImageAlphaEnable(pDISP_OUTPUT[type].pVIOC_RDMA_FB, 1);

	VIOC_WMIX_SetChromaKey(pDISP_OUTPUT[type].pVIOC_WMIXBase, 0, chroma_en, chromaR, chromaG, chromaB, 0xF8, 0xFC, 0xF8);			

	VIOC_RDMA_SetImageEnable(pDISP_OUTPUT[type].pVIOC_RDMA_FB);

	VIOC_WMIX_SetUpdate(pDISP_OUTPUT[type].pVIOC_WMIXBase);
	
	dprintk(" end g2d_rotate_need=%d g2d_format_need=%d, scaler_need=%d end\n", g2d_rotate_need, g2d_format_need, scaler_need);

	return 1;
}


void TCC_OUTPUT_FB_UpdateSync(unsigned int type)
{
	VIOC_RDMA * pRDMA_OUTPUT;

	if(pDISP_OUTPUT[type].pVIOC_DispBase == NULL)
	{
		dprintk("%s - Err: Output DISP is not valid, type=%d\n", __func__, type);
		return;
	}

	#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
		if(exclusive_ui_clear_force == 0)
		{
			dprintk("%s, force exclusive_ui not clear!!\n", __func__); 
			return;
		}
	#endif

	dprintk("%s type=%d UseVSyncInterrupt=%d\n", __func__, type, UseVSyncInterrupt);

	//pLCDC_OUTPUT_FB_CH = (volatile PLCDC_CHANNEL)&pLCDC_OUTPUT[type]->LI2C;

	VIOC_RDMA_SetImageBase(pDISP_OUTPUT[type].pVIOC_RDMA_FB, FBimg_buf_addr, 0, 0);

	FBimg_buf_addr = 0;

	if(UseVSyncInterrupt)	{
		TCC_OUTPUT_FB_WaitVsyncInterrupt(type);
	}
}

void TCC_OUTPUT_FB_WaitVsyncInterrupt(unsigned int type)
{
	int ret;
	
	if(type != OutputType)
		type = OutputType;

	if(pDISP_OUTPUT[type].pVIOC_DispBase == NULL)
	{
		dprintk("%s - Err: Output LCDC is not valid, OutputType=%d\n", __func__, type);
		return;
	}

	//pLCDC_OUTPUT[type]->LSTATUS = 0xFFFFFFFF; 
	BITCSET(pDISP_OUTPUT[type].pVIOC_DispBase->uLSTATUS.nREG, 0xFFFFFFFF, 0xFFFFFFFF);
	
	tca_lcdc_interrupt_onoff(TRUE, pDISP_OUTPUT[type].LCDC_N);

	#if defined(CONFIG_TCC_OUTPUT_DUAL_UI)
		if(output_lcdc[type] == 0)
		{
			Output_lcdc0_struct.state = 0;
			ret = wait_event_interruptible_timeout(Output_lcdc0_struct.waitq, Output_lcdc0_struct.state == 1, msecs_to_jiffies(50));
		}
		else
		{
			Output_lcdc1_struct.state = 0;
			ret = wait_event_interruptible_timeout(Output_lcdc1_struct.waitq, Output_lcdc1_struct.state == 1, msecs_to_jiffies(50));
		}
	#else
		Output_struct.state = 0;
		ret = wait_event_interruptible_timeout(Output_struct.waitq, Output_struct.state == 1, msecs_to_jiffies(50));
	#endif
	tca_lcdc_interrupt_onoff(FALSE, pDISP_OUTPUT[type].LCDC_N);

	if(ret <= 0)	{
	 	printk("  [%d]: tcc_setup_interrupt timed_out!! \n", ret);
	}
}

int TCC_OUTPUT_FB_MouseIconSelect(unsigned int output_type)
{
	VIOC_DISP * pDISPBase;
	VIOC_WMIX * pWMIXBase;
	VIOC_RDMA * pRDMABase;

	unsigned int lcd_width, lcd_height;

	if(pDISP_OUTPUT[output_type].pVIOC_DispBase == NULL)
	{
		dprintk("%s - Err: Output DISP is not valil\n", __func__);
		return;
	}

	pDISPBase = pDISP_OUTPUT[output_type].pVIOC_DispBase;

	VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);
	
	if((!lcd_width) || (!lcd_height))
		return;

	dprintk("%s : lcd_width = %d, lcd_height = %d\n", __func__, lcd_width, lcd_height);

	if( lcd_width > 1280 && lcd_height > 720 )
	{
		mouse_cursor_width = MOUSE_CURSOR_WIDTH_30;
		mouse_cursor_height = MOUSE_CURSOR_HEIGHT_30;
		memcpy(Gmap_cpu,(char *)&gMouseIcon30X30[0], mouse_cursor_width*mouse_cursor_height*4);
	}
	else if( lcd_width == 1280 || lcd_height == 720 )
	{
		mouse_cursor_width = MOUSE_CURSOR_WIDTH_20;
		mouse_cursor_height = MOUSE_CURSOR_HEIGHT_20;
		memcpy(Gmap_cpu,(char *)&gMouseIcon20X20[0], mouse_cursor_width*mouse_cursor_height*4);
	}
	else
	{
		mouse_cursor_width = MOUSE_CURSOR_WIDTH_12;
		mouse_cursor_height = MOUSE_CURSOR_WIDTH_12;
		memcpy(Gmap_cpu,(char *)&gMouseIcon12X12[0], mouse_cursor_width*mouse_cursor_height*4);
	}

	return 1;
}


int TCC_OUTPUT_FB_MouseShow(unsigned int enable, unsigned int type)
{
	VIOC_RDMA * pRDMABase;
	
	dprintk("%s : enable = %d\n", __func__, enable);

	if(pDISP_OUTPUT[type].pVIOC_RDMA_Mouse == NULL)
	{
		dprintk("%s - Err: Output DISP is not valid, type=%d\n", __func__, type);
		return;
	}

	pRDMABase = pDISP_OUTPUT[type].pVIOC_RDMA_Mouse;

	if(enable)
	{

	}
	else
	{
		VIOC_RDMA_SetImageDisable( pRDMABase);
	}

	return 1;
}


int TCC_OUTPUT_FB_MouseMove(unsigned int width, unsigned int height, tcc_mouse *mouse, unsigned int type)
{
	VIOC_DISP * pDISPBase;
	VIOC_WMIX * pWMIXBase;
	VIOC_RDMA * pRDMABase;
	
	unsigned int lcd_width, lcd_height, lcd_w_pos,lcd_h_pos, mouse_x, mouse_y, image_width, image_height;

	if(pDISP_OUTPUT[type].pVIOC_DispBase == NULL)
	{
		dprintk("%s - Err: Output LCDC is not valid, type=%d\n", __func__, type);
		return 0;
	}
	

	pDISPBase = pDISP_OUTPUT[type].pVIOC_DispBase;
	pWMIXBase = pDISP_OUTPUT[type].pVIOC_WMIXBase;
	pRDMABase = pDISP_OUTPUT[type].pVIOC_RDMA_Mouse;

	dprintk("%s pRDMA:0x%08x, pWMIX:0x%08x, pDISP:0x%08x\n", __func__, pRDMABase, pWMIXBase, pDISPBase);

	VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);
	
	if((!lcd_width) || (!lcd_height))
		return;

	mouse_x = (unsigned int)(lcd_width * mouse->x / width);
	mouse_y = (unsigned int)(lcd_height *mouse->y / height);

	if( mouse_x > lcd_width - mouse_cursor_width )
	{
		//pLCDC_OUTPUT[type]->LI3S = (mouse_cursor_height << 16) | lcd_width - mouse_x;
		image_width = lcd_width - mouse_x;
		image_height = mouse_cursor_height;
	}
	else
	{
		//pLCDC_OUTPUT[type]->LI3S = (mouse_cursor_height << 16) | mouse_cursor_width;
		image_width = mouse_cursor_width;
		image_height = mouse_cursor_height;
	}

		
	VIOC_RDMA_SetImageOffset(pRDMABase, TCC_LCDC_IMG_FMT_RGB888, image_width);
	VIOC_RDMA_SetImageFormat(pRDMABase, TCC_LCDC_IMG_FMT_RGB888);

	lcd_w_pos = mouse_x;
	lcd_h_pos = mouse_y;

	dprintk("%s lcd_width:%d, lcd_height:%d, lcd_w_pos:%d, lcd_h_pos:%d\n\n", __func__, lcd_width, lcd_height, lcd_w_pos, lcd_h_pos);
	
	// position
	//pLCDC_channel->LIP = ((lcd_h_pos << 16) | (lcd_w_pos));
	VIOC_WMIX_SetPosition(pWMIXBase, 3, lcd_w_pos, lcd_h_pos);

	// scale
	VIOC_RDMA_SetImageScale(pRDMABase, 0, 0);
	
	VIOC_RDMA_SetImageSize(pRDMABase, image_width, image_height);
		
	// position
	//if(ISZERO(pLCDC->LCTRL, HwLCTRL_NI)) //--
	if(pDISPBase->uCTRL.nREG & HwDISP_NI)
	{
		VIOC_RDMA_SetImageIntl(pRDMABase, 0);
		VIOC_WMIX_SetPosition(pWMIXBase, 3, lcd_w_pos, lcd_h_pos);
	}
	else
	{
		VIOC_RDMA_SetImageIntl(pRDMABase, 1);
		VIOC_WMIX_SetPosition(pWMIXBase, 3,  lcd_w_pos, (lcd_h_pos/2) );
	}

	// alpha & chroma key color setting
	VIOC_RDMA_SetImageAlphaSelect(pRDMABase, 1);
	VIOC_RDMA_SetImageAlphaEnable(pRDMABase, 1);
	
	// image address
	VIOC_RDMA_SetImageBase(pRDMABase, pMouseBuffer, NULL, NULL);

	VIOC_WMIX_SetUpdate(pWMIXBase);
	VIOC_RDMA_SetImageEnable(pRDMABase);


	return 1;
}

