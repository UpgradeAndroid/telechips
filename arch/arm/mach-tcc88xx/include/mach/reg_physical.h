/****************************************************************************
*   FileName    : reg_physical.h
*   Description : 
****************************************************************************
*
*   TCC Version : 1.0
*   Copyright (c) Telechips, Inc.
*   ALL RIGHTS RESERVED
*
****************************************************************************/

#ifndef __REG_PHYSICAL_H__
#define __REG_PHYSICAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "TCC88xx_Physical.h"
#include "TCC88xx_Structures.h"

#define tcc88xx_chip_rev() (*(volatile unsigned long*)0xEFE0008C)
#define TCC88XX_REV0		0x10004450
#define TCC88XX_REV1		0x10004444

#ifdef __cplusplus
}
#endif

#endif // __REG_PHYSICAL_H__
