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


static pmap_t pmap_viqe;
#define PA_VIQE_BASE_ADDR	pmap_viqe.base
#define PA_VIQE_BASE_SIZE	pmap_viqe.size

static VIQE *pVIQE;
static VIOC_RDMA *pRDMABase;
static VIOC_SC *pSCALERBase;
static VIOC_DISP *pDISPBase;
static VIOC_WMIX *pWMIXBase;

static int gFrmCnt = 0;
static int gbfield =0;
static int gRDMA_reg = 0;
static int gVIQE_RDMA_num = 0;
static int gSC_RDMA_num = 0;
static int gSCALER_reg = 0;
static int gSCALER_num = 0;
static int gusingScale = 0;
static int gLcdc_layer = -1;
static unsigned int gPMEM_VIQE_BASE;
static unsigned int gPMEM_VIQE_SIZE;

#if 0
#define dprintk(msg...)	 { printk( "tca_hdmi: " msg); }
#else
#define dprintk(msg...)	 
#endif



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
	VIOC_VIQE_DEINTL_MODE DI_mode = VIOC_VIQE_DEINTL_MODE_2D;
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
	if(DI_mode == VIOC_VIQE_DEINTL_S)
	{
		deintl_dma_base0	= NULL;
		deintl_dma_base1	= NULL;
		deintl_dma_base2	= NULL;
		deintl_dma_base3	= NULL;
	}
	else
	{
		// If you use 3D(temporal) De-interlace mode, you have to set physical address for using DMA register.
		//If 2D(spatial) mode, these registers are ignored
		imgSize = (framebufWidth * framebufHeight * 2);
		deintl_dma_base0	= gPMEM_VIQE_BASE;
		deintl_dma_base1	= deintl_dma_base0 + imgSize;
		deintl_dma_base2	= deintl_dma_base1 + imgSize;
		deintl_dma_base3	= deintl_dma_base2 + imgSize;	
	}

	if (top_size_dont_use == OFF)
	{
		framebufWidth  = 0;
		framebufHeight = 0;
	}
	
	VIOC_RDMA_SetImageY2REnable(pRDMABase, FALSE);
	VIOC_RDMA_SetImageY2RMode(pRDMABase, 0x02); /* Y2RMode Default 0 (Studio Color) */
	VIOC_RDMA_SetImageIntl(pRDMABase, 1);
	VIOC_RDMA_SetImageBfield(pRDMABase, OddFirst);
	
	if(DI_mode == VIOC_VIQE_DEINTL_S)
	{
		VIOC_API_VIQE_SetPlugIn(VIOC_DEINTLS, gVIQE_RDMA_num);
	}
	else
	{
		VIOC_VIQE_SetControlRegister(pVIQE, framebufWidth, framebufHeight, img_fmt);
		VIOC_VIQE_SetDeintlRegister(pVIQE, img_fmt, top_size_dont_use, framebufWidth, framebufHeight, DI_mode, deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3);
		VIOC_VIQE_SetControlEnable(pVIQE, OFF, OFF, OFF, OFF, ON);
		if(gVIQE_RDMA_num == VIOC_VIQE_RDMA_02)
		{
			VIOC_VIQE_SetImageY2REnable(pVIQE, TRUE);
			VIOC_VIQE_SetImageY2RMode(pVIQE, 0x02);
		}
		VIOC_API_VIQE_SetPlugIn(VIOC_VIQE, gVIQE_RDMA_num);
	}

	gFrmCnt= 0;
	if(OddFirst)
		gbfield =1;
	else
	gbfield =0;
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
	gFrmCnt++;	
}

void TCC_VIQE_DI_DeInit(void)
{
	VIOC_VIQE_DEINTL_MODE DI_mode = VIOC_VIQE_DEINTL_MODE_2D;
	volatile PVIOC_IREQ_CONFIG pIREQConfig;
	pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

	printk("TCC_VIQE_DI_DeInit\n");
	if(DI_mode == VIOC_VIQE_DEINTL_S)
	{
		VIOC_API_VIQE_SetPlugOut(VIOC_DEINTLS);
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x01<<17)); // DEINTLS reset
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x00<<17)); // DEINTLS reset
	}
	else
	{
		VIOC_API_VIQE_SetPlugOut(VIOC_VIQE);
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x01<<16)); // VIQE reset
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x00<<16)); // VIQE reset
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
void TCC_VIQE_DI_Init60Hz(int lcdCtrlNum, int Lcdc_layer, int useSCALER, unsigned int img_fmt, 
						unsigned int srcWidth, unsigned int srcHeight,
						int crop_top, int crop_bottom, int crop_left, int crop_right, 
						unsigned int destWidth, unsigned int destHeight,
						unsigned int offset_x, unsigned int offset_y, int OddFirst)
{
	unsigned int deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3;
	unsigned int framebufWidth, framebufHeight;
	unsigned int lcd_width = 0, lcd_height = 0, scale_x = 0, scale_y = 0;
	int imgSize;
	VIOC_VIQE_FMT_TYPE viqe_fmt;

	if(img_fmt == 24)
		viqe_fmt = VIOC_VIQE_FMT_YUV420;
	else
		viqe_fmt = VIOC_VIQE_FMT_YUV420;
		
	VIOC_VIQE_DEINTL_MODE DI_mode = VIOC_VIQE_DEINTL_MODE_2D;
	int top_size_dont_use = OFF;		//If this value is OFF, The size information is get from VIOC modules.

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

	framebufWidth = ((srcWidth - crop_left - crop_right) >> 3) << 3;			// 8bit align
	framebufHeight = ((srcHeight - crop_top - crop_bottom) >> 1) << 1;		// 2bit align
	printk("TCC_VIQE_DI_Init60Hz, W:%d, H:%d, DW:%d, DH:%d, FMT:%d, VFMT:%s, OddFirst:%d, RDMA:%d\n", framebufWidth, framebufHeight, destWidth, destHeight, img_fmt, (viqe_fmt?"YUV422":"YUV420"), OddFirst, ((gRDMA_reg-HwVIOC_RDMA00)/256));

	VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);
	if((!lcd_width) || (!lcd_height))
	{
		printk("%s invalid lcd size\n", __func__);
		return;
	}

	if(useSCALER)
	{
		VIOC_CONFIG_PlugIn (gSCALER_num, gSC_RDMA_num);			
		VIOC_SC_SetBypass (pSCALERBase, OFF);
		
		VIOC_SC_SetDstSize (pSCALERBase, destWidth, destHeight);			// set destination size in scaler
		VIOC_SC_SetOutSize (pSCALERBase, destWidth, destHeight);			// set output size in scaer
		gusingScale = 1;
	}

	dprintk("%s lcdc:%d, pRDMA:0x%08x, pWMIX:0x%08x, pDISP:0x%08x, addr0:0x%08x\n", __func__, hdmi_lcdc, pRDMABase, pWMIXBase, pDISPBase, ImageInfo->addr0);
		
	VIOC_RDMA_SetImageY2REnable(pRDMABase, FALSE);
	VIOC_RDMA_SetImageY2RMode(pRDMABase, 0x02); /* Y2RMode Default 0 (Studio Color) */

	VIOC_RDMA_SetImageOffset(pRDMABase, img_fmt, framebufWidth);
	VIOC_RDMA_SetImageFormat(pRDMABase, img_fmt);

	scale_x = 0;
	scale_y = 0;

	// scale
	VIOC_RDMA_SetImageScale(pRDMABase, scale_x, scale_y);
	VIOC_RDMA_SetImageSize(pRDMABase, framebufWidth, framebufHeight);

	//interlace image
	VIOC_RDMA_SetImageIntl(pRDMABase, 1);
	VIOC_RDMA_SetImageBfield(pRDMABase, OddFirst);
	VIOC_WMIX_SetPosition(pWMIXBase, Lcdc_layer,  offset_x, offset_y);
	if(useSCALER)
		VIOC_SC_SetUpdate (pSCALERBase);

	VIOC_WMIX_SetUpdate(pWMIXBase);
	gLcdc_layer = Lcdc_layer;

	if(DI_mode == VIOC_VIQE_DEINTL_S)
	{
		deintl_dma_base0	= NULL;
		deintl_dma_base1	= NULL;
		deintl_dma_base2	= NULL;
		deintl_dma_base3	= NULL;
	}
	else
	{
		// If you use 3D(temporal) De-interlace mode, you have to set physical address for using DMA register.
		//If 2D(spatial) mode, these registers are ignored
		imgSize = (framebufWidth * framebufHeight * 2);
		deintl_dma_base0	= gPMEM_VIQE_BASE;
		deintl_dma_base1	= deintl_dma_base0 + imgSize;
		deintl_dma_base2	= deintl_dma_base1 + imgSize;
		deintl_dma_base3	= deintl_dma_base2 + imgSize;	
	}

	if (top_size_dont_use == OFF)
	{
		framebufWidth  = 0;
		framebufHeight = 0;
	}
		
	if(DI_mode == VIOC_VIQE_DEINTL_S)
	{
		VIOC_CONFIG_PlugIn(VIOC_DEINTLS, gVIQE_RDMA_num);
	}
	else
	{
		VIOC_VIQE_SetControlRegister(pVIQE, framebufWidth, framebufHeight, viqe_fmt);
		VIOC_VIQE_SetDeintlRegister(pVIQE, viqe_fmt, top_size_dont_use, framebufWidth, framebufHeight, DI_mode, deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3);
		VIOC_VIQE_SetControlEnable(pVIQE, OFF, OFF, OFF, OFF, ON);
		VIOC_VIQE_SetImageY2REnable(pVIQE, TRUE);
		VIOC_VIQE_SetImageY2RMode(pVIQE, 0x02);
		VIOC_CONFIG_PlugIn(VIOC_VIQE, gVIQE_RDMA_num);
	}

	gFrmCnt= 0;		
}


void TCC_VIQE_DI_Run60Hz(int useSCALER, unsigned int addr0, unsigned int addr1, unsigned int addr2,
						unsigned int srcWidth, unsigned int srcHeight,	
						int crop_top, int crop_bottom, int crop_left, int crop_right, 
						unsigned int destWidth, unsigned int destHeight, 
						unsigned int offset_x, unsigned int offset_y, int OddFirst)
{
	unsigned int lcd_width = 0, lcd_height = 0, scale_x = 0, scale_y = 0;

	if(gFrmCnt == 0)
		printk("TCC_VIQE_DI_Run60Hz\n");

	if(gFrmCnt == 3)
		VIOC_VIQE_SetDeintlMode(pVIQE, VIOC_VIQE_DEINTL_MODE_3D);
	
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
			VIOC_CONFIG_PlugOut(VIOC_SC1);
			gusingScale = 0;
		}
	}

	VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);
	if((!lcd_width) || (!lcd_height))
	{
		printk("%s invalid lcd size\n", __func__);
		return;
	}

	dprintk("%s lcd_width:%d, lcd_height:%d\n", __func__, lcd_width, lcd_height);
	
	// position
	VIOC_WMIX_SetPosition(pWMIXBase, gLcdc_layer,  offset_x, offset_y);
	VIOC_WMIX_SetUpdate(pWMIXBase);

	VIOC_RDMA_SetImageBfield(pRDMABase, OddFirst);
	// image address
	VIOC_RDMA_SetImageBase(pRDMABase, addr0, addr1, addr2);
	VIOC_RDMA_SetImageEnable(pRDMABase);

	gFrmCnt++;	
}

void TCC_VIQE_DI_DeInit60Hz(void)
{
	VIOC_VIQE_DEINTL_MODE DI_mode = VIOC_VIQE_DEINTL_MODE_2D;
	volatile PVIOC_IREQ_CONFIG pIREQConfig;
	pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

	printk("TCC_VIQE_DI_DeInit60Hz\n");
	VIOC_RDMA_SetImageDisable(pRDMABase);	
	VIOC_RDMA_SetImageY2REnable(pRDMABase, TRUE);
	VIOC_RDMA_SetImageY2RMode(pRDMABase, 0x02); /* Y2RMode Default 0 (Studio Color) */
	VIOC_CONFIG_PlugOut(gSCALER_num);
	gusingScale = 0;
	
	if(DI_mode == VIOC_VIQE_DEINTL_S)
	{
		VIOC_CONFIG_PlugOut(VIOC_DEINTLS);
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x01<<17)); // DEINTLS reset
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x00<<17)); // DEINTLS reset
	}
	else
	{
		VIOC_CONFIG_PlugOut(VIOC_VIQE);
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x01<<16)); // VIQE reset
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x00<<16)); // VIQE reset
	}
}

