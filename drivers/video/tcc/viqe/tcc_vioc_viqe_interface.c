#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <mach/bsp.h>
#include <mach/tca_ckc.h>
#include <linux/delay.h>

#include <mach/vioc_rdma.h>
#include <mach/vioc_viqe.h>
#include <mach/vioc_scaler.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_disp.h>
#include <mach/vioc_config.h>
#include <mach/vioc_api.h>
#include <mach/vioc_plugin_tcc892x.h>
#include "tcc_vioc_viqe.h"
#include "tcc_vioc_viqe_interface.h"
#include <mach/tcc_viqe_ioctl.h>

#include <linux/fb.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tca_fb_output.h>
#include <plat/pmap.h>

#define USE_DEINTERLACE_S
static pmap_t pmap_viqe;
#define PA_VIQE_BASE_ADDR	pmap_viqe.base
#define PA_VIQE_BASE_SIZE	pmap_viqe.size

static VIQE *pVIQE;
static VIOC_RDMA *pRDMABase;
static VIOC_SC *pSCALERBase;
static VIOC_DISP *pDISPBase;
static VIOC_WMIX *pWMIXBase;
static VIOC_IREQ_CONFIG *pIREQConfig;

#ifdef USE_DEINTERLACE_S
static VIOC_VIQE_DEINTL_MODE gDI_mode = VIOC_VIQE_DEINTL_S;
static int gusingDI_S = 0;
#else
static VIOC_VIQE_DEINTL_MODE gDI_mode = VIOC_VIQE_DEINTL_MODE_2D;
#endif
static int gFrmCnt = 0;
static int gbfield =0;
static int gRDMA_reg = 0;
static int gVIQE_RDMA_num = 0;
static int gSC_RDMA_num = 0;
static int gSCALER_reg = 0;
static int gSCALER_num = 0;
static int gusingScale = 0;
static int gLcdc_layer = -1;
static int gImg_fmt = -1;
static VIOC_VIQE_FMT_TYPE gViqe_fmt;
static unsigned int gPMEM_VIQE_BASE;
static unsigned int gPMEM_VIQE_SIZE;
static int gpreCrop_left = 0;
static int gpreCrop_right = 0;
static int gpreCrop_top = 0;
static int gpreCrop_bottom = 0;


#if 0
#define dprintk(msg...)	 { printk( "tca_hdmi: " msg); }
#else
#define dprintk(msg...)	 
#endif

extern void tccxxx_GetAddress(unsigned char format, unsigned int base_Yaddr, unsigned int src_imgx, unsigned int  src_imgy,
					unsigned int start_x, unsigned int start_y, unsigned int* Y, unsigned int* U,unsigned int* V);

/* VIQE Set */

int VIOC_API_VIQE_SetPlugIn(unsigned int viqe, unsigned int path)
{
	int iResult = VIOC_DRIVER_ERR;

	iResult = VIOC_CONFIG_PlugIn(viqe, path);

	if(iResult == VIOC_DEVICE_CONNECTED)
	{
		iResult = VIOC_DRIVER_NOERR;
	}
	else
	{
		iResult = VIOC_DRIVER_ERR;
	}
	
	return iResult;
}

int VIOC_API_VIQE_SetPlugOut(unsigned int viqe)
{
	int iResult = VIOC_DRIVER_ERR;

	iResult = VIOC_CONFIG_PlugOut(viqe);

	if(iResult == VIOC_DEVICE_CONNECTED)
	{
		iResult = VIOC_DRIVER_NOERR;
	}
	else
	{
		iResult = VIOC_DRIVER_ERR;
	}
	
	return iResult;
}

//////////////////////////////////////////////////////////////////////////////////////////
void TCC_VIQE_DI_Init(int scalerCh, int useWMIXER, unsigned int srcWidth, unsigned int srcHeight,
						int crop_top, int crop_bottom, int crop_left, int crop_right, int OddFirst)
{
	unsigned int deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3;
	unsigned int framebufWidth, framebufHeight;
	int imgSize;
	VIOC_VIQE_FMT_TYPE img_fmt = VIOC_VIQE_FMT_YUV420;
	int top_size_dont_use = OFF;		//If this value is OFF, The size information is get from VIOC modules.

	pmap_get_info("viqe", &pmap_viqe);

	if(useWMIXER)
	{
		gRDMA_reg = HwVIOC_RDMA14;
		gVIQE_RDMA_num = VIOC_VIQE_RDMA_14;	
	}
	else
	{
		if(scalerCh == 0)
		{
			gRDMA_reg = HwVIOC_RDMA12;
			gVIQE_RDMA_num = VIOC_VIQE_RDMA_12;
		}
		else if(scalerCh == 1)
	{
			gRDMA_reg = HwVIOC_RDMA02;//HwVIOC_RDMA14;
			gVIQE_RDMA_num = VIOC_VIQE_RDMA_02;//VIOC_VIQE_RDMA_14;
		}
		else
	{
			gRDMA_reg = HwVIOC_RDMA06;
			gVIQE_RDMA_num = VIOC_VIQE_RDMA_06;
		}
	}
		
	gPMEM_VIQE_BASE = PA_VIQE_BASE_ADDR;
	gPMEM_VIQE_SIZE = PA_VIQE_BASE_SIZE;

	pVIQE= (VIQE *)tcc_p2v(HwVIOC_VIQE0);
	pRDMABase = (VIOC_RDMA *)tcc_p2v(gRDMA_reg);
	
	framebufWidth = ((srcWidth - crop_left - crop_right) >> 3) << 3;			// 8bit align
	framebufHeight = ((srcHeight - crop_top - crop_bottom) >> 1) << 1;		// 2bit align

	printk("TCC_VIQE_DI_Init, W:%d, H:%d, FMT:%s, OddFirst:%d, RDMA:%d\n", framebufWidth, framebufHeight, (img_fmt?"YUV422":"YUV420"), OddFirst, ((gRDMA_reg-HwVIOC_RDMA00)/256));
	
	VIOC_RDMA_SetImageY2REnable(pRDMABase, FALSE);
	VIOC_RDMA_SetImageY2RMode(pRDMABase, 0x02); /* Y2RMode Default 0 (Studio Color) */
	VIOC_RDMA_SetImageIntl(pRDMABase, 1);
	VIOC_RDMA_SetImageBfield(pRDMABase, OddFirst);
#ifdef USE_DEINTERLACE_S
	deintl_dma_base0	= NULL;
	deintl_dma_base1	= NULL;
	deintl_dma_base2	= NULL;
	deintl_dma_base3	= NULL;
	VIOC_API_VIQE_SetPlugIn(VIOC_DEINTLS, gVIQE_RDMA_num);
#else
	// If you use 3D(temporal) De-interlace mode, you have to set physical address for using DMA register.
	//If 2D(spatial) mode, these registers are ignored
	imgSize = (framebufWidth * framebufHeight * 2);
	deintl_dma_base0	= gPMEM_VIQE_BASE;
	deintl_dma_base1	= deintl_dma_base0 + imgSize;
	deintl_dma_base2	= deintl_dma_base1 + imgSize;
	deintl_dma_base3	= deintl_dma_base2 + imgSize;	
	if (top_size_dont_use == OFF)
	{
		framebufWidth  = 0;
		framebufHeight = 0;
	}
	
	VIOC_VIQE_SetControlRegister(pVIQE, framebufWidth, framebufHeight, img_fmt);
	VIOC_VIQE_SetDeintlRegister(pVIQE, img_fmt, top_size_dont_use, framebufWidth, framebufHeight, gDI_mode, deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3);
	VIOC_VIQE_SetControlEnable(pVIQE, OFF, OFF, OFF, OFF, ON);
	if(gVIQE_RDMA_num == VIOC_VIQE_RDMA_02)
	{
		VIOC_VIQE_SetImageY2REnable(pVIQE, TRUE);
		VIOC_VIQE_SetImageY2RMode(pVIQE, 0x02);
	}
	VIOC_API_VIQE_SetPlugIn(VIOC_VIQE, gVIQE_RDMA_num);
	if(OddFirst)
		gbfield =1;
	else
	gbfield =0;
#endif

	gFrmCnt= 0;
}


void TCC_VIQE_DI_Run(unsigned int srcWidth, unsigned int srcHeight,	
						int crop_top, int crop_bottom, int crop_left, int crop_right, int OddFirst)
{
#if 0
{
	unsigned int pBase0, pBase1, pBase2;
	unsigned int pBase3, pBase4, pBase5;
	pBase0 = address[0] + (crop_top * srcWidth + crop_left);
	pBase1 = address[1] + (crop_top / 2 * srcWidth + crop_left);
	pBase2 = address[2] + (crop_top / 2 * srcWidth + crop_left);

	pBase3 = address[3] + (crop_top * srcWidth + crop_left);
	pBase4 = address[4] + (crop_top / 2 * srcWidth + crop_left);
	pBase5 = address[5] + (crop_top / 2 * srcWidth + crop_left);
}	
#endif

	if(gFrmCnt == 0)
		printk("TCC_VIQE_DI_Run\n");

#ifndef USE_DEINTERLACE_S
	if(gFrmCnt == 3)
		VIOC_VIQE_SetDeintlMode(pVIQE, VIOC_VIQE_DEINTL_MODE_3D);

#if 1
	if (gbfield) 					// end fied of bottom field
	{
		VIOC_RDMA_SetImageBfield(pRDMABase, 0);				// change the bottom to top field
		// if you want to change the base address, you call the RDMA SetImageBase function in this line.
		gbfield = 0;

	} 
	else 
	{
		VIOC_RDMA_SetImageBfield(pRDMABase, 1);				// change the top to bottom field
		gbfield = 1;
	}
#else
	VIOC_RDMA_SetImageBfield(pRDMABase, OddFirst);				// change the top to bottom field
#endif
#else
	VIOC_RDMA_SetImageY2REnable(pRDMABase, FALSE);
	VIOC_RDMA_SetImageY2RMode(pRDMABase, 0x02); /* Y2RMode Default 0 (Studio Color) */
#endif
	gFrmCnt++;	
}

void TCC_VIQE_DI_DeInit(void)
{
	volatile PVIOC_IREQ_CONFIG pIREQConfig;
	pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

	printk("TCC_VIQE_DI_DeInit\n");
#ifdef USE_DEINTERLACE_S	
	VIOC_API_VIQE_SetPlugOut(VIOC_DEINTLS);
	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x01<<17)); // DEINTLS reset
	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x00<<17)); // DEINTLS reset
#else
	VIOC_API_VIQE_SetPlugOut(VIOC_VIQE);
	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x01<<16)); // VIQE reset
	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x00<<16)); // VIQE reset
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
void TCC_VIQE_DI_Init60Hz(int lcdCtrlNum, int Lcdc_layer, int useSCALER, unsigned int img_fmt, 
						unsigned int srcWidth, unsigned int srcHeight,
						unsigned int destWidth, unsigned int destHeight,
						unsigned int offset_x, unsigned int offset_y, int OddFirst)
{
	unsigned int deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3;
	unsigned int framebufWidth, framebufHeight;
	unsigned int lcd_width = 0, lcd_height = 0, scale_x = 0, scale_y = 0;
	int imgSize;
	int top_size_dont_use = OFF;		//If this value is OFF, The size information is get from VIOC modules.

	gLcdc_layer = Lcdc_layer;
	
	if(img_fmt == 24)
		gViqe_fmt = VIOC_VIQE_FMT_YUV420;
	else
		gViqe_fmt = VIOC_VIQE_FMT_YUV420;
	gImg_fmt = img_fmt;
		
	pmap_get_info("viqe", &pmap_viqe);
	gPMEM_VIQE_BASE = PA_VIQE_BASE_ADDR;
	gPMEM_VIQE_SIZE = PA_VIQE_BASE_SIZE;

	if(lcdCtrlNum)
	{
		gRDMA_reg = HwVIOC_RDMA06;
		gVIQE_RDMA_num = VIOC_VIQE_RDMA_06;
		gSC_RDMA_num = VIOC_SC_RDMA_06;
		gSCALER_reg = HwVIOC_SC1;
		gSCALER_num = VIOC_SC1;

		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);		
	}
	else
	{
		gRDMA_reg = HwVIOC_RDMA02;
		gVIQE_RDMA_num = VIOC_VIQE_RDMA_02;
		gSC_RDMA_num = VIOC_SC_RDMA_02;
		gSCALER_reg = HwVIOC_SC1;
		gSCALER_num = VIOC_SC1;

		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
	}

	pVIQE= (VIQE *)tcc_p2v(HwVIOC_VIQE0);
	pRDMABase = (VIOC_RDMA *)tcc_p2v(gRDMA_reg);
	pSCALERBase = (VIOC_SC *)tcc_p2v(gSCALER_reg );
	pIREQConfig = (VIOC_IREQ_CONFIG *)tcc_p2v(HwVIOC_IREQ);

	framebufWidth = ((srcWidth) >> 3) << 3;			// 8bit align
	framebufHeight = ((srcHeight) >> 1) << 1;		// 2bit align
	printk("TCC_VIQE_DI_Init60Hz, W:%d, H:%d, DW:%d, DH:%d, FMT:%d, VFMT:%s, OddFirst:%d, RDMA:%d\n", framebufWidth, framebufHeight, destWidth, destHeight, img_fmt, (gViqe_fmt?"YUV422":"YUV420"), OddFirst, ((gRDMA_reg-HwVIOC_RDMA00)/256));

	VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);
	if((!lcd_width) || (!lcd_height))
	{
		printk("%s invalid lcd size\n", __func__);
		return;
	}

	//RDMA SETTING
#ifdef USE_DEINTERLACE_S
	VIOC_RDMA_SetImageY2REnable(pRDMABase, TRUE);
	VIOC_RDMA_SetImageY2RMode(pRDMABase, 0x02); /* Y2RMode Default 0 (Studio Color) */
#else
#if 0
	if( gDI_mode == VIOC_VIQE_DEINTL_MODE_BYPASS)
	{
		VIOC_RDMA_SetImageY2REnable(pRDMABase, TRUE);
		VIOC_RDMA_SetImageY2RMode(pRDMABase, 0x02); /* Y2RMode Default 0 (Studio Color) */
	}
	else
#endif		
	{
		VIOC_RDMA_SetImageY2REnable(pRDMABase, FALSE);
		VIOC_RDMA_SetImageY2RMode(pRDMABase, 0x02); /* Y2RMode Default 0 (Studio Color) */
	}
#endif		
	VIOC_RDMA_SetImageOffset(pRDMABase, img_fmt, framebufWidth);
	VIOC_RDMA_SetImageFormat(pRDMABase, img_fmt);
	VIOC_RDMA_SetImageScale(pRDMABase, scale_x, scale_y);
	VIOC_RDMA_SetImageSize(pRDMABase, framebufWidth, framebufHeight);
	VIOC_RDMA_SetImageIntl(pRDMABase, 1);
	VIOC_RDMA_SetImageBfield(pRDMABase, OddFirst);


#ifdef USE_DEINTERLACE_S
	deintl_dma_base0	= NULL;
	deintl_dma_base1	= NULL;
	deintl_dma_base2	= NULL;
	deintl_dma_base3	= NULL;
	VIOC_CONFIG_PlugIn(VIOC_DEINTLS, gVIQE_RDMA_num);
	gusingDI_S = 1;
#else
	// If you use 3D(temporal) De-interlace mode, you have to set physical address for using DMA register.
	//If 2D(spatial) mode, these registers are ignored
	imgSize = (framebufWidth * framebufHeight * 2);
	deintl_dma_base0	= gPMEM_VIQE_BASE;
	deintl_dma_base1	= deintl_dma_base0 + imgSize;
	deintl_dma_base2	= deintl_dma_base1 + imgSize;
	deintl_dma_base3	= deintl_dma_base2 + imgSize;	

	VIOC_VIQE_SetControlRegister(pVIQE, framebufWidth, framebufHeight, gViqe_fmt);
	VIOC_VIQE_SetDeintlRegister(pVIQE, gViqe_fmt, top_size_dont_use, framebufWidth, framebufHeight, gDI_mode, deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3);
	VIOC_VIQE_SetDenoise(pVIQE, gViqe_fmt, framebufWidth, framebufHeight, 1, 0, deintl_dma_base0, deintl_dma_base1); 	//for bypass path on progressive frame
	VIOC_VIQE_SetControlEnable(pVIQE, OFF, OFF, OFF, OFF, ON);
//		if(gDI_mode != VIOC_VIQE_DEINTL_MODE_BYPASS)
	{
		VIOC_VIQE_SetImageY2REnable(pVIQE, TRUE);
		VIOC_VIQE_SetImageY2RMode(pVIQE, 0x02);
	}
	VIOC_CONFIG_PlugIn(VIOC_VIQE, gVIQE_RDMA_num);
#endif		

	//SCALER SETTING
	if(useSCALER)
	{
		VIOC_CONFIG_PlugIn (gSCALER_num, gSC_RDMA_num);			
		VIOC_SC_SetBypass (pSCALERBase, OFF);
		
		VIOC_SC_SetDstSize (pSCALERBase, destWidth, destHeight);			// set destination size in scaler
		VIOC_SC_SetOutSize (pSCALERBase, destWidth, destHeight);			// set output size in scaer
		VIOC_SC_SetUpdate (pSCALERBase);
		gusingScale = 1;
	}
	
	VIOC_WMIX_SetPosition(pWMIXBase, Lcdc_layer,  offset_x, offset_y);
	VIOC_WMIX_SetUpdate(pWMIXBase);

	gFrmCnt= 0;		
}


void TCC_VIQE_DI_Run60Hz(int useSCALER, unsigned int addr0, unsigned int addr1, unsigned int addr2,
						unsigned int srcWidth, unsigned int srcHeight,	
						int crop_top, int crop_bottom, int crop_left, int crop_right, 
						unsigned int destWidth, unsigned int destHeight, 
						unsigned int offset_x, unsigned int offset_y, int OddFirst, int FrameInfo_Interlace)
{
	unsigned int lcd_width = 0, lcd_height = 0, scale_x = 0, scale_y = 0;
	int cropWidth, cropHeight;

	VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);
	if((!lcd_width) || (!lcd_height))
	{
		printk("%s invalid lcd size\n", __func__);
		return;
	}
	dprintk("%s lcd_width:%d, lcd_height:%d\n", __func__, lcd_width, lcd_height);

	cropWidth = crop_right - crop_left;
	cropHeight = crop_bottom - crop_top;
	{
		int addr_Y = (unsigned int)addr0;
		int addr_U = (unsigned int)addr1;
		int addr_V = (unsigned int)addr2;
#ifndef USE_DEINTERLACE_S
		if((gpreCrop_left != crop_left) || (gpreCrop_right !=crop_right) || (gpreCrop_top != crop_top) || (gpreCrop_bottom !=crop_bottom))
		{
			unsigned int deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3;
			int imgSize;
			int top_size_dont_use = OFF;		//If this value is OFF, The size information is get from VIOC modules.
			VIOC_RDMA_SetImageDisable(pRDMABase);	
			VIOC_CONFIG_PlugOut(VIOC_VIQE);

			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x01<<16)); // VIQE reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x00<<16)); // VIQE reset

			imgSize = (srcWidth * srcHeight * 2);
			deintl_dma_base0	= gPMEM_VIQE_BASE;
			deintl_dma_base1	= deintl_dma_base0 + imgSize;
			deintl_dma_base2	= deintl_dma_base1 + imgSize;
			deintl_dma_base3	= deintl_dma_base2 + imgSize;	

			VIOC_VIQE_SetControlRegister(pVIQE, srcWidth, srcHeight, gViqe_fmt);
			VIOC_VIQE_SetDeintlRegister(pVIQE, gViqe_fmt, top_size_dont_use, srcWidth, srcHeight, gDI_mode, deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3);
			VIOC_VIQE_SetDenoise(pVIQE, gViqe_fmt, srcWidth, srcHeight, 1, 0, deintl_dma_base0, deintl_dma_base1); 	//for bypass path on progressive frame
			VIOC_VIQE_SetImageY2REnable(pVIQE, TRUE);
			VIOC_VIQE_SetImageY2RMode(pVIQE, 0x02);
			VIOC_CONFIG_PlugIn(VIOC_VIQE, gVIQE_RDMA_num);
		}
#endif			

		tccxxx_GetAddress(gImg_fmt, (unsigned int)addr0, srcWidth, srcHeight, crop_left, crop_top, &addr_Y, &addr_U, &addr_V);
		
		VIOC_RDMA_SetImageSize(pRDMABase, cropWidth, cropHeight);
		VIOC_RDMA_SetImageBase(pRDMABase, addr_Y, addr_U, addr_V);
		gpreCrop_left = crop_left;
		gpreCrop_right = crop_right;
		gpreCrop_top = crop_top;
		gpreCrop_bottom = crop_bottom;
	}
	VIOC_RDMA_SetImageBfield(pRDMABase, OddFirst);
	VIOC_RDMA_SetImageEnable(pRDMABase);
	dprintk(" Image Crop left=[%d], right=[%d], top=[%d], bottom=[%d], W:%d, H:%d odd:%d\n", crop_left, crop_right, crop_top, crop_bottom, cropWidth, cropHeight, OddFirst);


#ifdef USE_DEINTERLACE_S
	if(FrameInfo_Interlace)
	{
		VIOC_RDMA_SetImageIntl(pRDMABase, 1);
		if(!gusingDI_S)
		{
			VIOC_CONFIG_PlugIn(VIOC_DEINTLS, gVIQE_RDMA_num);
			gusingDI_S = 1;
		}
	}
	else
	{
		VIOC_RDMA_SetImageIntl(pRDMABase, 0);
		if(gusingDI_S)
		{
			VIOC_CONFIG_PlugOut(VIOC_DEINTLS);
			gusingDI_S = 0;
		}
	}
#else
	if(FrameInfo_Interlace)
	{
		if(gFrmCnt >= 3)
		{
			VIOC_VIQE_SetDeintlMode(pVIQE, VIOC_VIQE_DEINTL_MODE_3D);
			gDI_mode = VIOC_VIQE_DEINTL_MODE_3D;			
		}
		else
		{
			VIOC_VIQE_SetDeintlMode(pVIQE, VIOC_VIQE_DEINTL_MODE_2D);
			gDI_mode = VIOC_VIQE_DEINTL_MODE_2D;			
		}
		VIOC_VIQE_SetControlMode(pVIQE, OFF, OFF, OFF, OFF, ON);
		VIOC_RDMA_SetImageIntl(pRDMABase, 1);
	}
	else
	{	
		VIOC_VIQE_SetControlMode(pVIQE, OFF, OFF, OFF, ON, OFF);
		VIOC_RDMA_SetImageIntl(pRDMABase, 0);
		gFrmCnt = 0;
	}	
#endif


	if(useSCALER)
	{
		if(!gusingScale) {
			gusingScale = 1;
			VIOC_CONFIG_PlugIn (gSCALER_num, gSC_RDMA_num);			
			VIOC_SC_SetBypass (pSCALERBase, OFF);
		}
		
		VIOC_SC_SetDstSize (pSCALERBase, destWidth, destHeight);			// set destination size in scaler
		VIOC_SC_SetOutSize (pSCALERBase, destWidth, destHeight);			// set output size in scaer
		VIOC_SC_SetUpdate (pSCALERBase);
	}
	else
	{
		if(gusingScale == 1)	{
			VIOC_RDMA_SetImageDisable(pRDMABase);
			VIOC_CONFIG_PlugOut(gSCALER_num);
			gusingScale = 0;
		}
	}

	// position
	VIOC_WMIX_SetPosition(pWMIXBase, gLcdc_layer,  offset_x, offset_y);
	VIOC_WMIX_SetUpdate(pWMIXBase);

	gFrmCnt++;	
}

void TCC_VIQE_DI_DeInit60Hz(void)
{
	printk("TCC_VIQE_DI_DeInit60Hz\n");
	VIOC_RDMA_SetImageDisable(pRDMABase);	
	VIOC_RDMA_SetImageY2REnable(pRDMABase, TRUE);
	VIOC_RDMA_SetImageY2RMode(pRDMABase, 0x02); /* Y2RMode Default 0 (Studio Color) */
	VIOC_CONFIG_PlugOut(gSCALER_num);
	gusingScale = 0;
	
#ifdef USE_DEINTERLACE_S
	VIOC_CONFIG_PlugOut(VIOC_DEINTLS);
	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x01<<17)); // DEINTLS reset
	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x00<<17)); // DEINTLS reset
	gusingDI_S = 0;
#else

	VIOC_CONFIG_PlugOut(VIOC_VIQE);
	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x01<<16)); // VIQE reset
	BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x00<<16)); // VIQE reset
#endif
}

