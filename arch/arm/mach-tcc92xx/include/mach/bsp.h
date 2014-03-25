/****************************************************************************
*   FileName    : bsp.h
*   Description : 
****************************************************************************
*
*   TCC Version : 1.0
*   Copyright (c) Telechips, Inc.
*   ALL RIGHTS RESERVED
*
****************************************************************************/

#ifndef __BSP_H__
#define __BSP_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__KERNEL__) && !defined(_LINUX_)
#define _LINUX_
#endif

#if defined(_LINUX_)
#include <mach/reg_physical.h>
#include <mach/bsp_cfg.h>
#include <mach/globals.h>
#include <mach/tca_ckc.h>
#else
//system os header file 
#include <system_type.h> 

//argument structur and define file
#include <args.h>

//globals macro, defines file
#include <globals.h>

//bsp option config file
#include <bsp_cfg.h>

//Physical Base address file
#include <reg_physical.h>

//Kernel Ioctl
#include <ioctl_code.h>
#include <ioctl_str.h>
#endif


#ifdef __cplusplus
}
#endif

#endif // __BSP_H__
