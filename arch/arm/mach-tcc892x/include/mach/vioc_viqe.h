#ifndef _VIQE_H_
#define	_VIQE_H_

#include <mach/reg_physical.h>

#define FMT_FC_YUV420	0
#define FMT_FC_YUV422	1
#define VIQE_WIDTH		0
#define VIQE_HEIGHT		0
#define ON				1
#define OFF				0

typedef enum
{
	VIOC_VIQE_DEINTL_MODE_BYPASS = 0,
	VIOC_VIQE_DEINTL_MODE_SP,
	VIOC_VIQE_DEINTL_MODE_TM
}VIOC_VIQE_DEINTL_MODE;

typedef enum
{
	VIOC_VIQE_FMT_YUV420 = 0,
	VIOC_VIQE_FMT_YUV422

}VIOC_VIQE_FMT_TYPE;


/* Interface APIs */
extern void VIOC_VIQE_SetImageSize(VIQE *pVIQE, unsigned int width, unsigned int height);
extern void VIOC_VIQE_SetControlMisc(VIQE *pVIQE, unsigned int no_hor_intpl, unsigned int fmt_conv_disable, unsigned int fmt_conv_disable_using_fmt, unsigned int update_disable, unsigned int cfgupd, unsigned int h2h);
extern void VIOC_VIQE_SetControlDontUse(VIQE *pVIQE, unsigned int global_en_dont_use, unsigned int top_size_dont_use, unsigned int stream_deintl_info_dont_use);
extern void VIOC_VIQE_SetControlClockGate(VIQE *pVIQE, unsigned int deintl_dis, unsigned int d3d_dis, unsigned int pm_dis);
extern void VIOC_VIQE_SetControlEnable(VIQE *pVIQE, unsigned int his_cdf_or_lut_en, unsigned int his_en, unsigned int gamut_en, unsigned int denoise3d_en, unsigned int deintl_en);
extern void VIOC_VIQE_SetControlRegister(VIQE *pVIQE, unsigned int width, unsigned int height, unsigned int fmt);

extern void VIOC_VIQE_SetDeintlBase(VIQE *pVIQE, unsigned int frmnum, unsigned int base0, unsigned int base1, unsigned int base2, unsigned int base3);
extern void VIOC_VIQE_SetDeintlSize(VIQE *pVIQE, unsigned int width, unsigned int height);
extern void VIOC_VIQE_SetDeintlMisc(VIQE *pVIQE, unsigned int uvintpl, unsigned int cfgupd, unsigned int h2h, unsigned int top_size_dont_use);
extern void VIOC_VIQE_SetDeintlControl(VIQE *pVIQE, unsigned int fmt, unsigned int eof_control_ready, unsigned int dec_divisor, unsigned int ac_k0_limit, unsigned int ac_k1_limit, unsigned int ac_k2_limit);
extern void VIOC_VIQE_InitDeintlCoreBypass(VIQE *pVIQE);
extern void VIOC_VIQE_InitDeintlCoreSpatial(VIQE *pVIQE);
extern void VIOC_VIQE_InitDeintlCoreNormal(VIQE *pVIQE);
extern void VIOC_VIQE_SetDeintlCore(VIQE *pVIQE, unsigned int width, unsigned int height, VIOC_VIQE_FMT_TYPE fmt, unsigned int bypass, unsigned int top_size_dont_use);
extern void VIOC_VIQE_SetDeintlRegister(VIQE *pVIQE, unsigned int fmt, unsigned int width, unsigned int height, unsigned int bypass, unsigned int base0, unsigned int base1, unsigned int base2, unsigned int base3);

extern void VIOC_VIQE_SetDenoiseBase(VIQE *pVIQE, unsigned int  frmnum, unsigned int  base0, unsigned int  base1, unsigned int  offs0, unsigned int  offs1);
extern void VIOC_VIQE_SetDenoiseSize(VIQE *pVIQE, unsigned int  width, unsigned int  height);
extern void VIOC_VIQE_SetDenoiseMisc(VIQE *pVIQE, unsigned int  uvintpl, unsigned int  cfgupd, unsigned int  h2h, unsigned int  top_size_dont_use);
extern void VIOC_VIQE_SetDenoiseCoeff(VIQE *pVIQE, int luma_2d_strength, int luma_tem_strength, int chroma_2d_strength, int chroma_tem_strength);
extern void VIOC_VIQE_SetDenoiseCoreMisc(VIQE *pVIQE, unsigned int  top_size_dont_use, unsigned int  d3in, unsigned int  lut_init, unsigned int  cfgupd);
extern void VIOC_VIQE_SetDenoiseCoreBypass(VIQE *pVIQE, unsigned int  bypass_en);
extern void VIOC_VIQE_SetDenoiseFCCtrl(VIQE *pVIQE, unsigned int  fmt, unsigned int  eof_control_ready, unsigned int  dec_divisor, unsigned int  ac_k0_limit, unsigned int  ac_k1_limit, unsigned int  ac_k2_limit);
extern void VIOC_VIQE_SetDenoise(VIQE *pVIQE, unsigned int  fmt, unsigned int  width, unsigned int  height, unsigned int  bypass, unsigned int  frmnum, unsigned int  base0, unsigned int  base1);

//extern void VIOC_VIQE_IreqHandler(unsigned int vectorID);

#endif
