/****************************************************************************
*   FileName    : bsp_cfg.h
*   Description : 
****************************************************************************
*
*   TCC Version : 1.0
*   Copyright (c) Telechips, Inc.
*   ALL RIGHTS RESERVED
*
****************************************************************************/

#ifndef __BSP_CFG_H__
#define __BSP_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

//loglevel option 
#define TC_LOG_OPTION (TC_ERROR | TC_LOG ) 
#define TC_LOG_LEVEL(a) ((TC_LOG_OPTION)&(a))

#if defined(_LINUX_)
#   define tc_debug pr_debug
#endif

//
#ifdef __cplusplus
}
#endif

#endif // __BSP_CFG_H__
