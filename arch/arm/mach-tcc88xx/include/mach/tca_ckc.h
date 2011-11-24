/****************************************************************************
 *   FileName    : tca_ckc.h
 *   Description :
 ****************************************************************************
*
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
*
 ****************************************************************************/


/************************************************************************************************
*                                    Revision History                                           *
*                                                                                               *
* Version : 1.0    : 2009, 2, 04                                                                *
************************************************************************************************/

#ifndef __TCA_CKC_H__
#define __TCA_CKC_H__

//#include "bsp.h"

#if defined(_LINUX_)
#ifndef VOLATILE
#define VOLATILE
#endif
#else
#ifndef VOLATILE
#define VOLATILE	volatile
#endif
#endif

/************************************************************************************************
*										 MACRO												   *
************************************************************************************************/

/************************************************************************************************
*									  DEFINE											*
************************************************************************************************/
/************************************************************************************************
*										 ENUM													*
************************************************************************************************/
/****************************************************************************************
* FUNCTION :void tca_ckc_init(void)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE void tca_ckc_init(void);

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getpll(unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE unsigned int tca_ckc_getpll(unsigned int ch);

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getcpu(void)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE unsigned int tca_ckc_getcpu(void);

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getbus(void)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE unsigned int tca_ckc_getbus(void);

/****************************************************************************************
* FUNCTION :void tca_ckc_setfbusctrl(unsigned int clkname,unsigned int isenable,unsigned int freq, unsigned int sor)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE void tca_ckc_setfbusctrl(unsigned int clkname,unsigned int isenable,unsigned int md,unsigned int freq, unsigned int sor);

/****************************************************************************************
* FUNCTION :void tca_ckc_getfbusctrl(unsigned int clkname)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE int tca_ckc_getfbusctrl(unsigned int clkname);

/****************************************************************************************
* FUNCTION :int tca_ckc_setpll(unsigned int pll, unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE int tca_ckc_setpll(unsigned int pll, unsigned int ch);

/****************************************************************************************
* FUNCTION :void tca_ckc_validpll(unsigned int * pvalidpll045, unsigned int * pvalidpll123)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE void tca_ckc_validpll(unsigned int * pvalidpll045, unsigned int * pvalidpll123);

/****************************************************************************************
* FUNCTION :void tca_ckc_setpmupwroff( unsigned int periname , unsigned int isenable)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE void tca_ckc_setpmupwroff( unsigned int periname , unsigned int isenable);

/****************************************************************************************
* FUNCTION :void tca_ckc_getpmupwroff( unsigned int pmuoffname)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE int tca_ckc_getpmupwroff( unsigned int pmuoffname);

/****************************************************************************************
* FUNCTION :void tca_ckc_setperi(unsigned int periname,unsigned int isenable, unsigned int freq, unsigned int sor)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE void tca_ckc_setperi(unsigned int periname,unsigned int isenable, unsigned int freq, unsigned int sor);

/****************************************************************************************
* FUNCTION : int tca_ckc_getperi(unsigned int periname)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE int tca_ckc_getperi(unsigned int periname);

/****************************************************************************************
* FUNCTION :void tca_ckc_setcpu(unsigned int n)
* DESCRIPTION :  n is n/16
* example : CPU == PLL : n=16 - CPU == PLL/2 : n=8
* ***************************************************************************************/
extern VOLATILE void tca_ckc_setcpu(unsigned int n);
extern VOLATILE void tca_ckc_setcpuXIN(unsigned int n);

/****************************************************************************************
* FUNCTION :void tca_ckc_setswreset(unsigned int lfbusname, unsigned int mode)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE void  tca_ckc_setswreset(unsigned int lfbusname, unsigned int mode);


/****************************************************************************************
* FUNCTION : int tca_ckc_setiobus(unsigned int sel, unsigned int mode)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE void tca_ckc_setiobus(unsigned int sel, unsigned int mode);
/****************************************************************************************
* FUNCTION :  int tca_ckc_getiobus(unsigned int sel)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE int tca_ckc_getiobus(unsigned int sel);

/****************************************************************************************
* FUNCTION :void tca_ckc_setioswreset(unsigned int sel, unsigned int mode)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE unsigned int tca_ckc_set_iobus_swreset(unsigned int sel, unsigned int mode);
extern VOLATILE void tca_ckc_setioswreset(unsigned int sel, unsigned int mode);

/****************************************************************************************
* FUNCTION :  int tca_ckc_setsmui2c(unsigned int freq)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE void tca_ckc_setsmui2c(unsigned int freq);
/****************************************************************************************
* FUNCTION :  int tca_ckc_getsmui2c(void)
* DESCRIPTION : unit : 100Hz
* ***************************************************************************************/
extern VOLATILE int tca_ckc_getsmui2c(void);

/****************************************************************************************
* FUNCTION : void tca_ckc_setddipwdn(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of DDI_CONFIG
* ***************************************************************************************/
extern VOLATILE void tca_ckc_setddipwdn(unsigned int lpwdn , unsigned int lmode);
/****************************************************************************************
* FUNCTION : int tca_ckc_getddipwdn(unsigned int lpwdn)
* DESCRIPTION : Power Down Register of DDI_CONFIG
* ***************************************************************************************/
extern VOLATILE int tca_ckc_getddipwdn(unsigned int lpwdn);

extern VOLATILE void tca_ckc_setddiswreset(unsigned int lpwdn, unsigned int mode);

/****************************************************************************************
* FUNCTION : void tca_ckc_setgrppwdn(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of GRP_CONFIG
* ***************************************************************************************/
extern VOLATILE void tca_ckc_setgrppwdn(unsigned int lpwdn , unsigned int lmode);
/****************************************************************************************
* FUNCTION : int tca_ckc_getgrppwdn(unsigned int lpwdn)
* DESCRIPTION : Power Down Register of GRP_CONFIG
* ***************************************************************************************/
extern VOLATILE int tca_ckc_getgrppwdn(unsigned int lpwdn);

extern VOLATILE void tca_ckc_setgrpswreset(unsigned int lpwdn, unsigned int mode);

/****************************************************************************************
* FUNCTION : void tca_ckc_setvideobuscfg(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of Video Bus Config
* ***************************************************************************************/
extern VOLATILE void tca_ckc_setvideobuscfgpwdn(unsigned int lpwdn , unsigned int lmode);

/****************************************************************************************
* FUNCTION : int tca_ckc_getddipwdn(unsigned int lpwdn)
* DESCRIPTION : Power Down Register of DDI_CONFIG
* ***************************************************************************************/
extern VOLATILE int tca_ckc_getvideobuscfgpwdn(unsigned int lpwdn);

/****************************************************************************************
* FUNCTION : void tca_ckc_setvideobuscfgswreset(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of Video Bus Config
* ***************************************************************************************/
extern VOLATILE void tca_ckc_setvideobuscfgswreset(unsigned int lpwdn , unsigned int lmode);

/****************************************************************************************
* FUNCTION :void tca_ckc_getclkctrl0(void)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE unsigned int tca_ckc_getclkctrl0(void);

/****************************************************************************************
* FUNCTION :void tca_ckc_sethsiobus(unsigned int sel, unsigned int mode)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE void tca_ckc_sethsiobus(unsigned int lpwdn, unsigned int mode);

/****************************************************************************************
* FUNCTION :int tca_ckc_gethsiobus(unsigned int sel)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE int tca_ckc_gethsiobus(unsigned int lpwdn);

/****************************************************************************************
* FUNCTION : void tca_ckc_sethsiobusswreset(unsigned int sel, unsigned int mode)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE void tca_ckc_sethsiobusswreset(unsigned int lpwdn, unsigned int mode);

/****************************************************************************************
* FUNCTION : int tca_ckc_setcommonhsiobus(unsigned int hsiocontroller, unsigned int enable)
* DESCRIPTION :
* ***************************************************************************************/
extern VOLATILE int tca_ckc_setcommonhsiobus(unsigned int hsiocontroller, unsigned int enable);

extern VOLATILE void tca_ckc_setcambuspwdn(unsigned int lpwdn, unsigned int mode);

extern VOLATILE int tca_ckc_getcambuspwdn(unsigned int lpwdn);

extern VOLATILE void tca_ckc_setcambusswreset(unsigned int lpwdn, unsigned int mode);

extern VOLATILE void tca_ckc_enable(int clk, int enable);
extern VOLATILE void tca_ckc_pclk_enable(int pclk, int enable);
#endif  /* __TCA_CKC_H__ */



