/****************************************************************************
 *   FileName    : tca_backlight.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/

#ifndef __TCA_FB_OUTPUT_H__
#define __TCA_FB_OUTPUT_H__

#ifdef __cplusplus
extern 
"C" { 
#endif
#include <mach/tccfb_ioctrl.h>

/*****************************************************************************
*
* Defines
*
******************************************************************************/
#define TCC_FB_OUT_MAX_WIDTH		1920
#define TCC_FB_OUT_MAX_HEIGHT		1080


/*****************************************************************************
*
* Enum
*
******************************************************************************/

typedef enum{
	TCC_OUTPUT_LCDC0,
	TCC_OUTPUT_LCDC1,
	TCC_OUTPUT_LCDC_MAX
} TCC_OUTPUT_LCDC;

typedef enum{
	TCC_OUTPUT_NONE,
	TCC_OUTPUT_COMPOSITE,
	TCC_OUTPUT_COMPONENT,
	TCC_OUTPUT_HDMI,
	TCC_OUTPUT_MAX
} TCC_OUTPUT_TYPE;

/*****************************************************************************
*
* Type Defines
*
******************************************************************************/


/*****************************************************************************
*
* Structures
*
******************************************************************************/


/*****************************************************************************
*
* External Variables
*
******************************************************************************/


/*****************************************************************************
*
* External Functions
*
******************************************************************************/

extern void TCC_OUTPUT_LCDC_Init(void);
extern void TCC_OUTPUT_LCDC_OnOff(char output_type, char output_lcdc_num, char onoff);
extern void TCC_OUTPUT_LCDC_CtrlLayer(char output_type, char interlace, char format);
extern char TCC_OUTPUT_FB_Update(unsigned int width, unsigned int height, unsigned int bits_per_pixel, unsigned int addr, unsigned int type);
extern void TCC_OUTPUT_FB_UpdateSync(unsigned int type);

extern void TCC_OUTPUT_FB_WaitVsyncInterrupt(void);
extern int TCC_OUTPUT_SetOutputResizeMode(int mode);

#ifdef __cplusplus
 } 
#endif


#endif	//__TCA_FB_HDMI_H__

