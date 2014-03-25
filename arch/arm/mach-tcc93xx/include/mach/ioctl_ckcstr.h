/****************************************************************************
 *   FileName    : ioctl_ckcstr.h
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

#ifndef __IOCTL_STR_H__
#define __IOCTL_STR_H__

//#include "bsp.h"

#define MAJOR_NUM 247

// For CKC Controller
typedef struct _stckcioctl{
	unsigned int ioctlcode;
	//Reset or bus Enable name
	unsigned int prbname; 
	//Peri Clock
	unsigned int pckcname; 
	unsigned int pckcenable; 
	unsigned int pckcsource; 
	unsigned int pckcfreq;
	//PLL Cllock
	unsigned int pllchannel;
	unsigned int pllvalue;
	unsigned int P;
	unsigned int M;
	unsigned int S;
	//CPU Cllock
	unsigned int cpuvalue; 
	//BUS Cllock
	unsigned int busvalue;
	//mode 
	unsigned int mode; // Enable, Disable, ahalf, athird

	unsigned int priority;

	unsigned int cpudivider;
	unsigned int pmuoffname;
	
	unsigned int bspmax;	
	//Fbus Clock
	unsigned int fbusname;
	unsigned int fbusenable; 
	unsigned int fbussource; 
	unsigned int fbusfreq;

	//DDI PWDN
	unsigned int ddipdname;

	//ETC Block
	unsigned int etcblock;
	
	//Dynamic Min/Max
	unsigned int dynamicmax;
	unsigned int dynamicmin;
	unsigned int dynamiccycle;

	// Video BUS CFG
	unsigned int videobuscfgname;
	
}stckcioctl;


typedef struct _stckcinfo{
	unsigned int currentbusfreq;
	unsigned int currentsysfreq;
	unsigned int currentcpufreq;
	int pckcfreq; //return etc frequency
	unsigned int validpll012[30];
	unsigned int validpll345[30];
	int retVal;
	unsigned int currentpriority;

	unsigned int state;

	int fbusfreq;
	
	int pll1freq;
	int pll2freq;
	int pll3freq;
	int pll4freq;
	int pll5freq;
	
	int ddifreq;
	int grpfreq;
	int iofreq;
	int vbusfreq;
	int vpufreq;
	int smufreq;
	int hsbusfreq;
	int cambusfreq;
	int ddisubfreq;	
	
}stckcinfo;

//Bruce_temp..
#if 1
struct ckc_ioctl{
	stckcioctl  in_ckc;
	stckcinfo   out_ckc;
};

//AlenOh
struct storage_direct{
	void *buf;
	size_t count;
	loff_t pos;
	ssize_t actual;
	unsigned int user_space;
};
 
#define IOCTL_STORAGE_DIRECT_READ   _IO(MAJOR_NUM, 100)
#define IOCTL_STORAGE_DIRECT_WRITE  _IO(MAJOR_NUM, 101)
#define IOCTL_STORAGE_PING          _IO(MAJOR_NUM, 102)
#endif 
#endif /* __IOCTL_STR_H__ */
