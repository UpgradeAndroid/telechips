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

#ifndef __TCA_BKL_H__
#define __TCA_BKL_H__

/*****************************************************************************
*
* Defines
*
******************************************************************************/


/*****************************************************************************
*
* Enum
*
******************************************************************************/


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
#ifdef __cplusplus
extern 
"C" { 
#endif

void tca_lcdc_port_setting(char Nlcdc);

void tca_bkl_init(char lcdctrl_num);
void tca_bkl_powerup(void);
void tca_bkl_powerdown(void);
void tca_bkl_setpowerval(unsigned int inValue);
unsigned int  tca_bkl_getpowerval(void);


#ifdef __cplusplus
 } 
#endif

void tca_lcdc_port_setting(char Nlcdc);

void tca_bkl_init(char lcdctrl_num);
void tca_bkl_powerup(void);
void tca_bkl_powerdown(void);
void tca_bkl_setpowerval(unsigned int inValue);
unsigned int tca_bkl_getpowerval(void);
#endif	//__TCA_BKL_H__

