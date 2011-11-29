/****************************************************************************
 *	 FileName	 : tca_ckc.c
 *	 Description :
 ****************************************************************************
*
 *	 TCC Version 1.0
 *	 Copyright (c) Telechips, Inc.
 *	 ALL RIGHTS RESERVED
*
 ****************************************************************************/


#include <mach/bsp.h>
#include <asm/io.h>
#include <linux/mm.h>	// for PAGE_ALIGN
#include <linux/kernel.h>
#include <linux/module.h>
#include <mach/clock_info.h>

#ifndef VOLATILE
#define VOLATILE
#endif

#define SYS_CLK_CH	5
#define SYS_CLK_SRC	DIRECTPLL5

#define tca_wait()				{ volatile int i; for (i=0; i<0x2000; i++); }

#if defined(_LINUX_)
    #define iomap_p2v(x)            io_p2v(x)
#else
	#define iomap_p2v(x)			(OALPAtoVA(x,FALSE))
#endif

/****************************************************************************************
* Global Variable
* ***************************************************************************************/
PCKC	pCKC ;
PPMU	pPMU ;
PIOBUSCFG pIOBUSCFG;


/****************************************************************************************
* FUNCTION :void tca_ckc_init(void)
* DESCRIPTION :
* ***************************************************************************************/

VOLATILE void tca_ckc_init(void)
{
	pCKC = (PCKC)(iomap_p2v((unsigned int)HwCLK_BASE));
	pPMU = (PPMU)(iomap_p2v((unsigned int)HwPMU_BASE));
	pIOBUSCFG = (PIOBUSCFG)(iomap_p2v((unsigned int)HwIOBUSCFG_BASE));
	//return 0;
}
//arch_initcall(tca_ckc_init);

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getpll(unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE unsigned int tca_ckc_getpll(unsigned int ch)
{
	volatile unsigned	tPLL;
	volatile unsigned	tPLLCFG;
	unsigned	iP=0, iM=0, iS=0;

	switch(ch)
	{
		case DIRECTPLL0:
			tPLLCFG = pCKC->PLL0CFG;
			break;
		case DIRECTPLL1:
			tPLLCFG = pCKC->PLL1CFG;
			break;
		case DIRECTPLL2:
			tPLLCFG = pCKC->PLL2CFG;
			break;
		case DIRECTPLL3:
			tPLLCFG = pCKC->PLL3CFG;
			break;
		case 4:
			tPLLCFG = pCKC->PLL4CFG;
			break;
		case 5:
			tPLLCFG = pCKC->PLL5CFG;
			break;
	}

	//Fpll Clock
	iS	= (tPLLCFG & 0x7000000) >> 24;
	iM	= (tPLLCFG & 0xFFF00) >> 8;
	iP	= (tPLLCFG & 0x0003F) >> 0;

	tPLL= (((120000 * iM )/ iP) >> (iS));

	return tPLL;

}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getdividpll(unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE unsigned int tca_ckc_getdividpll(unsigned int ch)
{
	volatile unsigned int tDIVPLL;
	volatile unsigned int tPLL = tca_ckc_getpll(ch);
	unsigned int uiPDIV;

	switch(ch)
	{
		case 0:
			uiPDIV = (pCKC->CLKDIVC0 & (Hw30-Hw24))>>24;
			break;
		case 1:
			uiPDIV = (pCKC->CLKDIVC0 & (Hw22-Hw16))>>16;
			break;
		case 2:
			uiPDIV = (pCKC->CLKDIVC0 & (Hw14-Hw8))>>8;
			break;
		case 3:
			uiPDIV = (pCKC->CLKDIVC0 & (Hw6-Hw0));
			break;
		case 4:
			uiPDIV = (pCKC->CLKDIVC2 & (Hw30-Hw24))>>24;
			break;
		case 5:
			uiPDIV = (pCKC->CLKDIVC2 & (Hw22-Hw16))>>16;
			break;
	}

	//Fdivpll Clock
	tDIVPLL = (unsigned int)tPLL/(uiPDIV+1);

	return tDIVPLL;
}
/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getcpu(void)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE unsigned int tca_ckc_getcpu(void)
{
	unsigned int lcpu = 0;
	unsigned int lconfig = 0;
	unsigned int lcnt = 0;
	unsigned int li = 0;
	unsigned int lclksource = 0;

	lconfig = ((pCKC->CLK0CTRL & (Hw20-Hw4))>>4);

	for(li = 0; li < 16; li++)
	{
		if((lconfig & Hw0) == 1)
			lcnt++;
		lconfig = (lconfig >> 1);
	}

	switch(pCKC->CLK0CTRL & (Hw4-Hw0)) // Check CPU Source
	{
		case DIRECTPLL0 :
			lclksource =  tca_ckc_getpll(0);
			break;
		case DIRECTPLL1 :
			lclksource =  tca_ckc_getpll(1);
			break;
		case DIRECTPLL2 :
			lclksource =  tca_ckc_getpll(2);
			break;
		case DIRECTPLL3 :
			lclksource =  tca_ckc_getpll(3);
			break;
		case DIRECTXIN:
			lclksource =  120000;
			break;
		case DIVIDPLL0:
			lclksource = tca_ckc_getdividpll(0);
			break;
		case DIVIDPLL1:
			lclksource = tca_ckc_getdividpll(1);
			break;
		case DIRECTXTIN:
			lclksource =  120000;
			break;
		case DIRECTPLL4:
			lclksource =  tca_ckc_getpll(4);
			break;
		case DIRECTPLL5:
			lclksource =  tca_ckc_getpll(5);
			break;
		case DIVIDPLL2:
			lclksource = tca_ckc_getdividpll(2);
			break;
		case DIVIDPLL3:
			lclksource = tca_ckc_getdividpll(3);
			break;
		case DIVIDPLL4:
			lclksource = tca_ckc_getdividpll(4);
			break;
		case DIVIDPLL5:
			lclksource = tca_ckc_getdividpll(5);
			break;
		/*
		case DIVIDXIN:
			break;
		case DIVIDXTIN:
			break;
		*/
		default :
			lclksource =  tca_ckc_getpll(1);
			break;
	}

	lcpu = (lclksource * lcnt)/16;

	return lcpu;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getbus(void)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE unsigned int tca_ckc_getbus(void)
{
	unsigned int lbus = 0;
	unsigned int lconfig = 0;
	unsigned int lclksource = 0;

#if 0//20100826
	//Check synchronous clock mode
	if(!(pCKC->MBUSCTRL & Hw0))
	{
		//synchronous clock mode
		lconfig = ((pCKC->MBUSCTRL & (Hw20-Hw8))>>8);
		lclksource = tca_ckc_getcpu();
		lbus = lclksource /(lconfig+1);
		return lbus;
	}
#endif

	lconfig = ((pCKC->CLK2CTRL & (Hw8-Hw4))>>4);

	switch(pCKC->CLK2CTRL & (Hw4-Hw0)) // Check Memory Bus Source
	{
		case DIRECTPLL0 :
			lclksource =  tca_ckc_getpll(0);
			break;
		case DIRECTPLL1 :
			lclksource =  tca_ckc_getpll(1);
			break;
		case DIRECTPLL2 :
			lclksource =  tca_ckc_getpll(2);
			break;
		case DIRECTPLL3 :
			lclksource =  tca_ckc_getpll(3);
			break;
		case DIRECTXIN:
			lclksource =  120000;
			break;
		case DIVIDPLL0:
			lclksource = tca_ckc_getdividpll(0);
			break;
		case DIVIDPLL1:
			lclksource = tca_ckc_getdividpll(1);
			break;
		case DIRECTXTIN:
			lclksource =  120000;
			break;
		case DIRECTPLL4:
			lclksource =  tca_ckc_getpll(4);
			break;
		case DIRECTPLL5:
			lclksource =  tca_ckc_getpll(5);
			break;
		case DIVIDPLL2:
			lclksource = tca_ckc_getdividpll(2);
			break;
		case DIVIDPLL3:
			lclksource = tca_ckc_getdividpll(3);
			break;
		case DIVIDPLL4:
			lclksource = tca_ckc_getdividpll(4);
			break;
		case DIVIDPLL5:
			lclksource = tca_ckc_getdividpll(5);
			break;
		/*
		case DIVIDXIN:
			break;
		case DIVIDXTIN:
			break;
		*/
		default :
			lclksource =  tca_ckc_getpll(1);
			break;
	}

	lbus = lclksource /(lconfig+1);

	return lbus;
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_setclkctrlx(unsigned int isenable,unsigned int md,unsigned int config,unsigned int sel)
* DESCRIPTION : not ctrl 0 and ctrl 2 (CPU, BUS CTRL)
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gclkctrlx(unsigned int isenable,unsigned int config,unsigned int sel)
{
	unsigned int retVal = 0;
	retVal = ((isenable?1:0)<<21)|(config<<4)|(sel<<0);


	return retVal;
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_clkctrly(unsigned int isenable,unsigned int md,unsigned int config,unsigned int sel)
* DESCRIPTION : ctrl 0 and ctrl 2 (CPU and BUS CTRL)
*				config is divider (md = 0)
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gclkctrly(unsigned int isenable,unsigned int config,unsigned int sel, unsigned int ch)
{
	unsigned int retVal = 0;

	if(ch == CLKCTRL0)
	{
		switch(config)
		{
			case CLKDIV0:
					config = 0xFFFF; // 1111111111111111b 16/16
				break;
			case CLKDIV2:
					config = 0xAAAA; // 1010101010101010b 8/16
				break;
			case CLKDIV3:
					config = 0x9249; // 1001001001001001b 6/16
				break;
			case CLKDIV4:
					config = 0x8888; // 1000100010001000b 4/16
				break;
			case CLKDIVNONCHANGE:
					config = 0xFFFF; // 1111111111111111b
				break;
			default:
					config = 0xFFFF; // 1111111111111111b
				break;
		}
	}

	if(config == CLKDIVNONCHANGE)
	{
		//cpu
		retVal = (pCKC->CLK0CTRL & (Hw20-Hw4));

		retVal |= ((isenable?1:0)<<21)|(sel<<0);

	}
	else
		retVal = ((isenable?1:0)<<21)|(config<<4)|(sel<<0);

	return retVal;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setfbusctrl(unsigned int clkname,unsigned int isenable,unsigned int freq, unsigned int sor)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE void tca_ckc_setfbusctrl(unsigned int clkname,unsigned int isenable,unsigned int md,unsigned int freq, unsigned int sor)
{
	volatile unsigned	*pCLKCTRL;
	unsigned int clkdiv = 0;
	unsigned int clksource = 0;
	unsigned int lconfig = 0;

	pCLKCTRL =(volatile unsigned	*)((&pCKC->CLK0CTRL)+clkname);

#if defined(CONFIG_MACH_TCC8800ST)
	if (clkname != CLKCTRL3 /*&& clkname != CLKCTRL5*/) {
#endif
		if (sor == DIRECTPLL4)
			sor = DIRECTPLL3;
#if defined(CONFIG_MACH_TCC8800ST)
	}
#endif

	switch(sor)
	{
		case DIRECTPLL0 :
			clksource =  tca_ckc_getpll(0);
			break;
		case DIRECTPLL1 :
			clksource =  tca_ckc_getpll(1);
			break;
		case DIRECTPLL2 :
			clksource =  tca_ckc_getpll(2);
			break;
		case DIRECTPLL3 :
			clksource =  tca_ckc_getpll(3);
			break;
		case DIRECTXIN:
			clksource =  120000;
			break;
		case DIRECTPLL4:
			clksource =  tca_ckc_getpll(4);
			break;
		case DIRECTPLL5:
			clksource =  tca_ckc_getpll(5);
			break;
		default :
			clksource =  tca_ckc_getpll(1);
			break;
	}

	if (freq != 0)
	{
		clkdiv	= (clksource + (freq>>1)) / freq ;	// should be even number of division factor
		clkdiv -= 1;
	}
	else
		clkdiv	= 1;

	if(clkdiv == CLKDIV0) // The config value should not be "ZERO" = 1/(config+1)
		clkdiv = 1;


	if(clkname == CLKCTRL0 || clkname == CLKCTRL2)
	{
		*pCLKCTRL = tca_ckc_gclkctrly(isenable,clkdiv,sor,clkname);
	}
	else
	{
		if(isenable == DISABLE)
			*pCLKCTRL &= ~Hw21;
		else if (isenable == NOCHANGE)
			*pCLKCTRL = (*pCLKCTRL&(1<<21))|(clkdiv<<4)|(sor<<0);
		else
			*pCLKCTRL = tca_ckc_gclkctrlx(isenable,clkdiv,sor);
	}

}


/****************************************************************************************
* FUNCTION :void tca_ckc_getfbusctrl(unsigned int clkname)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE int tca_ckc_getfbusctrl(unsigned int clkname)
{
	volatile unsigned	*pCLKCTRL;
	unsigned int lcheck = 0;
	unsigned int lmd = 0;
	unsigned int lconfig = 0;
	unsigned int lsel = 0;
	unsigned int clksource = 0;

	pCLKCTRL =(volatile unsigned	*)((&pCKC->CLK0CTRL)+clkname);

	lcheck = ((*pCLKCTRL >> 21) & Hw0);
	lmd = ((*pCLKCTRL >> 20) & Hw0);
	lconfig = ((*pCLKCTRL >> 4) & 0xF);
	lsel = ((*pCLKCTRL) & 0xF);

	if(!lcheck || clkname == CLKCTRL0/* || clkname == CLKCTRL2*/)
		return -1;

	if(lmd == 0)
	{
		switch(lsel)
		{
			case DIRECTPLL0 :
				clksource =  tca_ckc_getpll(0);
				break;
			case DIRECTPLL1 :
				clksource =  tca_ckc_getpll(1);
				break;
			case DIRECTPLL2 :
				clksource =  tca_ckc_getpll(2);
				break;
			case DIRECTPLL3 :
				clksource =  tca_ckc_getpll(3);
				break;
			case DIRECTXIN:
				clksource =  120000;
				break;
			case DIRECTPLL4:
				clksource =  tca_ckc_getpll(4);
				break;
			case DIRECTPLL5:
				clksource =  tca_ckc_getpll(5);
				break;
			default :
				clksource =  tca_ckc_getpll(1);
				break;
		}

	}
	else
		return 0;

	return (clksource / (lconfig+1));
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_setpllxcfg(unsigned int isEnable, int P, int M, int S)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gpllxcfg(unsigned int isenable, unsigned int p, unsigned int m, unsigned int s, unsigned int vsel)
{
	unsigned int retVal = Hw31;//Disable

	if(isenable > 0)
	{
		retVal = (s<<24)|(m<<8)|(p<<0);
		retVal |= (vsel<<30);
		retVal |= Hw31;	//Enable
	}

	return retVal;
}

/****************************************************************************************
* FUNCTION :static void tca_ckc_pll(unsigned int p, unsigned int m, unsigned int s,unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE static void tca_ckc_pll(unsigned int p, unsigned int m, unsigned int s, unsigned int vsel, unsigned int ch)
{
	volatile unsigned	*pPLLCFG;
	unsigned int		pll_old, p_old, m_old, s_old;
	unsigned int		pll_src;

	if(ch <= 3)
		pPLLCFG =(volatile unsigned	*)((&pCKC->PLL0CFG)+ch);
	else //PLL4 or PLL5
		pPLLCFG =(volatile unsigned	*)((&pCKC->PLL4CFG)+(ch-4));

	// Get Current PLL value (unit. 100Hz)
	p_old = *pPLLCFG & 0x3F;
	if (ch == 0 || ch == 4 || ch == 5)
		m_old = (*pPLLCFG >> 8) & 0x3FF;
	else
		m_old = (*pPLLCFG >> 8) & 0x1FF;
	s_old = (*pPLLCFG >> 24) & 0x7;

	pll_old = ((120000*m)/p) >> s;

#if 0
	if (pll_old > 7680000)
		pll_src = DIRECTPLL4;
	else
	if (pll_old > 6480000)
		pll_src = DIRECTPLL0;
	else
#endif
	if (pll_old > 5800000)
		pll_src = DIRECTPLL2;
	else
		pll_src = DIRECTPLL1;	

	if(ch == SYS_CLK_CH) // PLL0 is System Clock Source
	{
		// Change System Clock Souce --> XIN (12Mhz)
		pCKC->CLK7CTRL = tca_ckc_gclkctrly(ENABLE,CLKDIV2,DIRECTXIN,CLKCTRL7);
		pCKC->CLK0CTRL = tca_ckc_gclkctrly(ENABLE,CLKDIVNONCHANGE,pll_src,CLKCTRL0);
		tca_wait();
	}

	//Disable PLL
	*pPLLCFG &= ~Hw31;
	//Set PMS
	*pPLLCFG = tca_ckc_gpllxcfg(ENABLE,p,m,s, vsel);
	//Enable PLL
	*pPLLCFG |= Hw31;
	tca_wait();
	//Restore System Clock Source
	if(ch == SYS_CLK_CH)
	{
		pCKC->CLK0CTRL = tca_ckc_gclkctrly(ENABLE,CLKDIVNONCHANGE,SYS_CLK_SRC,CLKCTRL0);
		pCKC->CLK7CTRL = tca_ckc_gclkctrly(ENABLE,CLKDIV4,SYS_CLK_SRC,CLKCTRL7);
	}

}


/****************************************************************************************
* FUNCTION :void tca_ckc_validpll(unsigned int * pvalidpll)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE void tca_ckc_validpll(unsigned int * pvalidpll045, unsigned int * pvalidpll123)
{
	unsigned int uCnt;
	sfPLL		*pPLL045, *pPLL123;

	pPLL045	= &pIO_CKC_PLL045[0];
	for (uCnt = 0; uCnt < NUM_PLL045; uCnt ++, pPLL045 ++)
	{
		*pvalidpll045 = pPLL045->uFpll ;

		pvalidpll045++;
	}

	pPLL123	= &pIO_CKC_PLL123[0];
	for (uCnt = 0; uCnt < NUM_PLL123; uCnt ++, pPLL123 ++)
	{
		*pvalidpll123 = pPLL123->uFpll ;

		pvalidpll123++;
	}
}

/****************************************************************************************
* FUNCTION :int tca_ckc_setpll(unsigned int pll, unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE int tca_ckc_setpll(unsigned int pll, unsigned int ch)
{
	unsigned	uCnt;
	int 		retVal = -1;


	sfPLL		*pPLL045, *pPLL123;
	if(pll != 0)
	{
		if(ch == 0 || ch == 4 || ch == 5)
		{
			pPLL045 = &pIO_CKC_PLL045[0];
			for (uCnt = 0; uCnt < NUM_PLL045; uCnt ++, pPLL045++)
				if (pPLL045->uFpll >= pll)
					break;

			if (uCnt < NUM_PLL045)
			{
				tca_ckc_pll(pPLL045->P,pPLL045->M ,pPLL045->S, pPLL045->VSEL, ch);
				retVal = 0;
				return 1;
			}
		}
		else
		{
			pPLL123 = &pIO_CKC_PLL123[0];
			for (uCnt = 0; uCnt < NUM_PLL123; uCnt ++, pPLL123++)
				if (pPLL123->uFpll >= pll)
					break;

			if (uCnt < NUM_PLL123)
			{
				tca_ckc_pll(pPLL123->P,pPLL123->M ,pPLL123->S, pPLL123->VSEL, ch);
				retVal = 0;
				return 1;
			}
		}
	}
	else
	{
		volatile unsigned	*pPLLCFG;

		if(ch <= 3)
			pPLLCFG =(volatile unsigned	*)((&pCKC->PLL0CFG)+ch);
		else //PLL4 or PLL5
			pPLLCFG =(volatile unsigned	*)((&pCKC->PLL4CFG)+(ch-4));

		//Disable PLL
		*pPLLCFG &= ~Hw31;
	}

	return -1;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setcpu(unsigned int n)
* DESCRIPTION :  n is n/16
* example : CPU == PLL : n=16 - CPU == PLL/2 : n=8
* ***************************************************************************************/
VOLATILE void tca_ckc_setcpu(unsigned int n)
{
	 unsigned int lckc0ctrl;
	 unsigned int lindex[] = {0x0,0x8000,0x8008,0x8808,0x8888,0xA888,0xA8A8,0xAAA8,0xAAAA,
							0xECCC,0xEECC,0xEEEC,0xEEEE,0xFEEE,0xFFEE,0xFFFE,0xFFFF};

	lckc0ctrl = pCKC->CLK0CTRL;
	lckc0ctrl &= ~(Hw20-Hw0);
	lckc0ctrl |= (lindex[n] << 4) | SYS_CLK_SRC;

	pCKC->CLK0CTRL = lckc0ctrl;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setcpu(unsigned int n)
* DESCRIPTION :  n is n/16
* example : CPU == PLL : n=16 - CPU == PLL/2 : n=8
* ***************************************************************************************/
VOLATILE void tca_ckc_setcpuXIN(unsigned int n)
{
	 unsigned int lckc0ctrl;
	 unsigned int lindex[] = {0x0,0x8000,0x8008,0x8808,0x8888,0xA888,0xA8A8,0xAAA8,0xAAAA,
							0xECCC,0xEECC,0xEEEC,0xEEEE,0xFEEE,0xFFEE,0xFFFE,0xFFFF};


	lckc0ctrl = pCKC->CLK0CTRL;
	lckc0ctrl &= ~(Hw20-Hw0);
	lckc0ctrl |= (lindex[n] << 4) | DIRECTXIN;

	pCKC->CLK0CTRL = lckc0ctrl;
}


/****************************************************************************************
* FUNCTION :void tca_ckc_setpmupwroff( unsigned int periname , unsigned int isenable)
* DESCRIPTION : PMU Block :  Power Off Register
* PMU_VIDEODAC
* PMU_HDMIPHY
* PMU_LVDSPHY
* PMU_USBNANOPHY
* PMU_SATAPHY
* PMU_MEMORYBUS
* PMU_VIDEOBUS
* PMU_DDIBUS
* PMU_GRAPHICBUS
* PMU_IOBUS
* ***************************************************************************************/
VOLATILE void tca_ckc_setpmupwroff( unsigned int periname , unsigned int isenable)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	if(isenable)
		pPMU->PWROFF |= lindex[periname];
	else
		pPMU->PWROFF &= ~(lindex[periname]);
}

/****************************************************************************************
* FUNCTION :void tca_ckc_getpmupwroff( unsigned int pmuoffname)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE int tca_ckc_getpmupwroff( unsigned int pmuoffname)
{
	unsigned int retVal = 0;
	retVal =  (pPMU->PWROFF >> pmuoffname)  & Hw0;

	return retVal;
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_setpckxxx(unsigned int isenable, unsigned int sel, unsigned int div)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gpckxxx(unsigned int isenable, unsigned int sel, unsigned int div)
{
	unsigned int retVal = Hw29; //Enable

	retVal = ((isenable?1:0)<<29)|(sel<<24)|(div<<0);
	return retVal;
}

/****************************************************************************************
* FUNCTION :static unsigned int tca_ckc_setpckyyy(unsigned int isenable, unsigned int sel, unsigned int div)
* DESCRIPTION : md (1: divider Mode, 0:DCO Mode)
* ***************************************************************************************/
VOLATILE static unsigned int tca_ckc_gpckyyy(unsigned int isenable, unsigned int md, unsigned int sel, unsigned int div)
{
	unsigned int retVal = Hw29;//Enable

	retVal = (md<<31)|((isenable?1:0)<<29)|(sel<<24)|(div<<0);

	return retVal;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setperi(unsigned int periname,unsigned int isenable, unsigned int freq, unsigned int sor)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE void tca_ckc_setperi(unsigned int periname,unsigned int isenable, unsigned int freq, unsigned int sor)
{
	unsigned uPll;
	unsigned int clkdiv = 0;
	unsigned int lclksource = 0;
	unsigned int clkmode = 1;

	volatile unsigned	*pPERI;
	pPERI =(volatile unsigned	*)((&pCKC->PCLK_TCX)+periname);

	if (sor == PCDIRECTPLL4)
		sor = PCDIRECTPLL3;
	if (sor == PCDIVIDPLL4)
		sor = PCDIVIDPLL3;

	switch(sor)
	{
		case PCDIRECTPLL0 :
			lclksource =  tca_ckc_getpll(0);
			break;
		case PCDIRECTPLL1 :
			lclksource =  tca_ckc_getpll(1);
			break;
		case PCDIRECTPLL2 :
			lclksource =  tca_ckc_getpll(2);
			break;
		case PCDIRECTPLL3 :
			lclksource =  tca_ckc_getpll(3);
			break;
		case PCDIRECTXIN :
			lclksource =  120000;
			break;
		case PCDIVIDPLL0:
			lclksource =  tca_ckc_getdividpll(0);
			break;
		case PCDIVIDPLL1:
			lclksource =  tca_ckc_getdividpll(1);
			break;
		case PCDIVIDPLL2:
			lclksource =  tca_ckc_getdividpll(2);
			break;
		case PCDIVIDPLL3:
			lclksource =  tca_ckc_getdividpll(3);
			break;
		/*
		case PCDIRECTXTIN:
			break;
		case PCEXITERNAL:
			break;
		case PCDIVIDXIN_HDMITMDS:
			break;
		case PCDIVIDXTIN_HDMIPCLK:
			break;
		*/
		case PCHDMI :
			lclksource =  270000;
			break;
		case PCSATA :
			lclksource =  250000;
			break;
		case PCUSBPHY:
			lclksource =  480000;
			break;
		/*
		case PCDIVIDXIN:
			break;
		case PCDIVIDXTIN:
			break;
		*/
		case PCDIRECTPLL4:
			lclksource =  tca_ckc_getpll(4);
			break;
		case PCDIRECTPLL5:
			lclksource =  tca_ckc_getpll(5);
			break;
		case PCDIVIDPLL4:
			lclksource =  tca_ckc_getdividpll(4);
			break;
		case PCDIVIDPLL5:
			lclksource =  tca_ckc_getdividpll(5);
			break;
#if 0
		case PCUSB1PHY:
			lclksource =  480000;
			break;
#endif
		/*
		case PCMIPI_PLL:
			break;
		*/
		default :
			lclksource =  tca_ckc_getpll(1);
			break;
	}

	if (freq != 0)
	{
		clkdiv	= (lclksource + (freq>>1)) / freq ; // should be even number of division factor
		clkdiv -= 1;
	}
	else
		clkdiv	= 0;

	if(periname == PERI_ADC || periname == PERI_AUD || periname == PERI_DAI0 || periname == PERI_DAI1 || periname == PERI_DAI2 || periname == PERI_SPDIF || periname == PERI_HDMIA)
	{
		if(periname == PERI_AUD || periname == PERI_DAI0 || periname == PERI_DAI1 || periname == PERI_DAI2 || periname == PERI_SPDIF || periname == PERI_HDMIA)
		{
			clkmode = 0;	// DCO Mode

			if (periname == PERI_SPDIF || periname == PERI_HDMIA)
                freq /= 2;

            if(freq >= 261243) {
				clkdiv = (freq *8192);
				uPll = lclksource;
				clkdiv = clkdiv/uPll;
				clkdiv <<= 3;
				clkdiv = clkdiv + 1;
            }
			else if(freq >= 131071)
			{
				clkdiv = (freq *16384);
				uPll = lclksource;
				clkdiv = clkdiv/uPll;
				clkdiv <<= 2;
				clkdiv = clkdiv + 1;
			}
			else
			{
				clkdiv = (freq *32768);
				uPll = lclksource;
				clkdiv = clkdiv/uPll;
				clkdiv <<= 1;
				clkdiv = clkdiv + 1;
			}

			if (periname == PERI_SPDIF || periname == PERI_HDMIA)
                clkdiv *= 2;
		}

		*pPERI = tca_ckc_gpckyyy(isenable,clkmode,sor,clkdiv);
	}
	else
	{
		*pPERI = tca_ckc_gpckxxx(isenable,sor,clkdiv);
	}
}

/****************************************************************************************
* FUNCTION : static int tca_ckc_gperi(unsigned int lclksrc, unsigned int ldiv,unsigned int lmd)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE static int tca_ckc_gperi(unsigned int lclksrc, unsigned int ldiv,unsigned int lmd)
{
	if(lmd == 1)
	{
		if(lclksrc == PCDIRECTXIN)
			return 120000/(ldiv+1);
		else if(lclksrc == PCDIRECTPLL0){
			return (tca_ckc_getpll(0)/(ldiv+1));
		}
		else if(lclksrc == PCDIRECTPLL1){
			return (tca_ckc_getpll(1)/(ldiv+1));
		}
		else if(lclksrc == PCDIRECTPLL2){
			return (tca_ckc_getpll(2)/(ldiv+1));
		}
		else if(lclksrc == PCDIRECTPLL3){
			return (tca_ckc_getpll(3)/(ldiv+1));
		}
		else if(lclksrc == PCDIRECTPLL4){
			return (tca_ckc_getpll(4)/(ldiv+1));
		}
		else if(lclksrc == PCDIRECTPLL5){
			return (tca_ckc_getpll(5)/(ldiv+1));
		}
		else if(lclksrc == PCHDMI){
			return (270000/(ldiv+1));
		}
		else if(lclksrc == PCSATA){
			return (250000/(ldiv+1));
		}
		else if(lclksrc == PCUSBPHY){
			return (480000/(ldiv+1));
		}
		else
			return -1; // Not Support Others

	}
	else
		return -1; // TO DO
}

/****************************************************************************************
* FUNCTION : int tca_ckc_getperi(unsigned int periname)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE int tca_ckc_getperi(unsigned int periname)
{
	unsigned int lreg = 0;
	unsigned int lmd = 1; // DIVIDER mode
	unsigned int lclksrc = 0;
	unsigned int ldiv = 0;

	lreg =*(volatile unsigned	*)((&pCKC->PCLK_TCX)+periname);
	lclksrc = (lreg&0x1F000000)>>24;

 	if(periname == PERI_ADC || periname == PERI_DAI0 || periname == PERI_DAI1 || periname == PERI_DAI2 || periname == PERI_SPDIF || periname == PERI_HDMIA)
	{
		lmd = (lreg&0x80000000);
		ldiv = (lreg & 0xFFFF);
		return tca_ckc_gperi(lclksrc, ldiv,lmd);
	}
	else
	{
		ldiv = (lreg & 0xFFF);
		return tca_ckc_gperi(lclksrc, ldiv,1);
	}
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setswreset(unsigned int lfbusname, unsigned int mode)
* DESCRIPTION :
*     lfbusname
*     mode  (1: reset , 0: non-reset)
* ***************************************************************************************/
VOLATILE void  tca_ckc_setswreset(unsigned int lfbusname, unsigned int mode)
{
	unsigned int hIndex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10};

	if(!mode)
		pCKC->SWRESET = ~(pCKC->SWRESET) | hIndex[lfbusname];
	else
		pCKC->SWRESET = ~(pCKC->SWRESET) & ~(hIndex[lfbusname]);
}

/****************************************************************************************
* FUNCTION :  int tca_ckc_setiobus(unsigned int sel, unsigned int mode)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE void tca_ckc_setiobus(unsigned int sel, unsigned int mode)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	unsigned int lrb_min;
	unsigned int lrb_max;
	unsigned int lrb_seperate;

	lrb_min = RB_USB11HOST;
	lrb_max = RB_ALLPERIPERALS;
	lrb_seperate = RB_ADMACONTROLLER;

	if(sel <  lrb_min || sel >=  lrb_max)
	{
		return;
	}

	if(sel >  lrb_seperate)
	{
		sel -=	(lrb_seperate+1);

		if(mode)
			pIOBUSCFG->HCLKEN1 |= lindex[sel];
		else
			pIOBUSCFG->HCLKEN1 &= ~lindex[sel];
	}
	else
	{
		if(mode)
			pIOBUSCFG->HCLKEN0 |= lindex[sel];
		else
			pIOBUSCFG->HCLKEN0 &= ~lindex[sel];
	}
}

/****************************************************************************************
* FUNCTION :  int tca_ckc_getiobus(unsigned int sel)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE int tca_ckc_getiobus(unsigned int sel)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};
	unsigned int lrb_min;
	unsigned int lrb_max;
	unsigned int lrb_seperate;
	int lretVal = 0;

	lrb_min = RB_USB11HOST;
	lrb_max = RB_ALLPERIPERALS;
	lrb_seperate = RB_ADMACONTROLLER;

	if(sel <  lrb_min || sel >=  lrb_max)
	{
		return -1;
	}

	if(sel >  lrb_seperate)
	{
		sel -=	(lrb_seperate+1);

		lretVal = (pIOBUSCFG->HCLKEN1  & lindex[sel]) ;
	}
	else
	{
		lretVal = (pIOBUSCFG->HCLKEN0  & lindex[sel]) ;
	}
	if(lretVal != 0)
		lretVal = 1;
	else
		lretVal = 0;

	return lretVal;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_set_iobus_swreset(unsigned int sel, unsigned int mode)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE unsigned int tca_ckc_set_iobus_swreset(unsigned int sel, unsigned int mode)
{
	printk("####### Do not call this functions on platform tcc88xx sel:%d, mode:%d ###########\n", sel, mode);
}
VOLATILE void tca_ckc_setioswreset(unsigned int sel, unsigned int mode)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	unsigned int lrb_min;
	unsigned int lrb_max;
	unsigned int lrb_seperate;

	lrb_min = RB_USB11HOST;
	lrb_max = RB_ALLPERIPERALS;
	lrb_seperate = RB_ADMACONTROLLER;

	if(sel <  lrb_min || sel >= lrb_max)
		return;
	if(sel >  lrb_seperate)
	{
		sel -=	(lrb_seperate+1);

		if(mode)
			pIOBUSCFG->HRSTEN1 &= ~lindex[sel];
		else
			pIOBUSCFG->HRSTEN1 |= lindex[sel];
	}
	else
	{
		if(mode)
			pIOBUSCFG->HRSTEN0 &= ~lindex[sel];
		else
			pIOBUSCFG->HRSTEN0 |= lindex[sel];
	}
}

/****************************************************************************************
* FUNCTION :  int tca_ckc_setsmui2c(unsigned int freq)
* DESCRIPTION : unit : 100Hz
* ***************************************************************************************/
VOLATILE void tca_ckc_setsmui2c(unsigned int freq)
{
	PSMUI2CICLK lSMUICLK;
	unsigned int lclkctrl7=0;
	unsigned int lsel=0;
	unsigned int lclksource=0;
	unsigned int lclkdiv=0;

	lSMUICLK = (PSMUI2CICLK)(iomap_p2v((unsigned int)HwSMUI2C_COMMON_BASE)); //0xF0400000
	lclkctrl7 = (unsigned int)pCKC->CLK7CTRL;

	lsel = (lclkctrl7 & (Hw4-Hw0));

	switch(lsel)
	{
		case DIRECTPLL0 :
			lclksource =  tca_ckc_getpll(0);
			break;
		case DIRECTPLL1 :
			lclksource =  tca_ckc_getpll(1);
			break;
		case DIRECTPLL2 :
			lclksource =  tca_ckc_getpll(2);
			break;
		case DIRECTPLL3 :
			lclksource =  tca_ckc_getpll(3);
			break;
		case DIRECTXIN:
			lclksource =  120000;
			break;
		case DIVIDPLL0:
			lclksource = tca_ckc_getdividpll(0);
			break;
		case DIVIDPLL1:
			lclksource = tca_ckc_getdividpll(1);
			break;
		case DIRECTXTIN:
			lclksource =  120000;
			break;
		case DIRECTPLL4:
			lclksource =  tca_ckc_getpll(4);
			break;
		case DIRECTPLL5:
			lclksource =  tca_ckc_getpll(5);
			break;
		case DIVIDPLL2:
			lclksource = tca_ckc_getdividpll(2);
			break;
		case DIVIDPLL3:
			lclksource = tca_ckc_getdividpll(3);
			break;
		case DIVIDPLL4:
			lclksource = tca_ckc_getdividpll(4);
			break;
		case DIVIDPLL5:
			lclksource = tca_ckc_getdividpll(5);
			break;
		/*
		case DIVIDXIN:
			break;
		case DIVIDXTIN:
			break;
		*/
		default :
			lclksource = tca_ckc_getpll(1);
			break;
	}

	if (freq != 0)
	{
		lclkdiv	= (lclksource + (freq>>1)) / freq ; // should be even number of division factor
		lSMUICLK->ICLK = (Hw31|lclkdiv);
	}
	else
	{
		lclkdiv	= 0;
		lSMUICLK->ICLK = 0;
	}
}
/****************************************************************************************
* FUNCTION :  int tca_ckc_getsmui2c(void)
* DESCRIPTION : unit : 100Hz
* ***************************************************************************************/
VOLATILE int tca_ckc_getsmui2c(void)
{
	PSMUI2CICLK lSMUICLK;
	unsigned int lclkctrl7;
	unsigned int lsel;
	unsigned int lclksource;
	unsigned int lclkdiv;

	lSMUICLK = (PSMUI2CICLK)(iomap_p2v((unsigned int)HwSMUI2C_COMMON_BASE)); //0xF0400000
	lclkctrl7 = (unsigned int)pCKC->CLK7CTRL;

	lsel = (lclkctrl7 & 0xF);

	switch(lsel)
	{
		case DIRECTPLL0 :
			lclksource =  tca_ckc_getpll(0);
			break;
		case DIRECTPLL1 :
			lclksource =  tca_ckc_getpll(1);
			break;
		case DIRECTPLL2 :
			lclksource =  tca_ckc_getpll(2);
			break;
		case DIRECTPLL3 :
			lclksource =  tca_ckc_getpll(3);
			break;
		case DIRECTXIN:
			lclksource =  120000;
			break;
		case DIVIDPLL0:
			lclksource = tca_ckc_getdividpll(0);
			break;
		case DIVIDPLL1:
			lclksource = tca_ckc_getdividpll(1);
			break;
		case DIRECTXTIN:
			lclksource =  120000;
			break;
		case DIRECTPLL4:
			lclksource =  tca_ckc_getpll(4);
			break;
		case DIRECTPLL5:
			lclksource =  tca_ckc_getpll(5);
			break;
		case DIVIDPLL2:
			lclksource = tca_ckc_getdividpll(2);
			break;
		case DIVIDPLL3:
			lclksource = tca_ckc_getdividpll(3);
			break;
		case DIVIDPLL4:
			lclksource = tca_ckc_getdividpll(4);
			break;
		case DIVIDPLL5:
			lclksource = tca_ckc_getdividpll(5);
			break;
		/*
		case DIVIDXIN:
			break;
		case DIVIDXTIN:
			break;
		*/
		default :
			lclksource = tca_ckc_getpll(1);
			break;
	}
	lclkdiv = (lclkctrl7 & 0xFFFF);

	if (lclkdiv != 0)
	{
		return (lclksource / lclkdiv) ;
	}
	else
		return -1;
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setddipwdn(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of DDI_CONFIG
* ***************************************************************************************/
VOLATILE void tca_ckc_setddipwdn(unsigned int lpwdn , unsigned int lmode)
{
	PDDICONFIG lDDIPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	if (!(pCKC->CLK1CTRL & Hw21))
		return;

	lDDIPWDN = (PDDICONFIG)(iomap_p2v((unsigned int)HwDDI_CONFIG_BASE)); //0xF0400000

	if(lmode)  // Normal
		lDDIPWDN->PWDN &= ~lindex[lpwdn];
	else // Power Down
		lDDIPWDN->PWDN |= lindex[lpwdn];

}
/****************************************************************************************
* FUNCTION : int tca_ckc_getddipwdn(unsigned int lpwdn)
* DESCRIPTION : Power Down Register of DDI_CONFIG
* ***************************************************************************************/
VOLATILE int tca_ckc_getddipwdn(unsigned int lpwdn)
{
	PDDICONFIG lDDIPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	if (!(pCKC->CLK1CTRL & Hw21))
		return -1;

	lDDIPWDN = (PDDICONFIG)(iomap_p2v((unsigned int)HwDDI_CONFIG_BASE)); //0xF0400000

	return (lDDIPWDN->PWDN & lindex[lpwdn]);
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setddiswreset(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : SWRESET Register of Display Bus Config
* ***************************************************************************************/
VOLATILE void tca_ckc_setddiswreset(unsigned int lpwdn , unsigned int lmode)
{
	PDDICONFIG lDDIPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	if (!(pCKC->CLK1CTRL & Hw21))
		return;

	lDDIPWDN = (PDDICONFIG)(iomap_p2v((unsigned int)HwDDI_CONFIG_BASE));

	if(lmode)
		lDDIPWDN->SWRESET |= lindex[lpwdn];
	else
		lDDIPWDN->SWRESET &= ~lindex[lpwdn];
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setgrppwdn(unsigned int lmode)
* DESCRIPTION : Power Down Register of GRPBUS 
* ***************************************************************************************/
VOLATILE void tca_ckc_setgrppwdn(unsigned int lpwdn, unsigned int lmode)
{
	PGRPBUSCONFIG lGRPPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	if (!(pCKC->CLK3CTRL & Hw21))
		return;

	lGRPPWDN = (PGRPBUSCONFIG)(iomap_p2v((unsigned int)HwGRPBUSCONFIG_BASE));

	if (lmode)
		lGRPPWDN->GRPBUS_PWRDOWN &= ~lindex[lpwdn];
	else
		lGRPPWDN->GRPBUS_PWRDOWN |= lindex[lpwdn];
}
/****************************************************************************************
* FUNCTION : int tca_ckc_getgrppwdn(void)
* DESCRIPTION : Power Down Register of GRPBUS 
* ***************************************************************************************/
VOLATILE int tca_ckc_getgrppwdn(unsigned int lpwdn)
{
	PGRPBUSCONFIG lGRPPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	if (!(pCKC->CLK3CTRL & Hw21))
		return -1;

	lGRPPWDN = (PGRPBUSCONFIG)(iomap_p2v((unsigned int)HwGRPBUSCONFIG_BASE));

	return (int)(lGRPPWDN->GRPBUS_PWRDOWN & lindex[lpwdn]);
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setgrpswreset(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : SWRESET Register of Graphic Bus Config
* ***************************************************************************************/
VOLATILE void tca_ckc_setgrpswreset(unsigned int lpwdn , unsigned int lmode)
{
	PGRPBUSCONFIG lGRPPWDN;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	if (!(pCKC->CLK3CTRL & Hw21))
		return;

	lGRPPWDN = (PGRPBUSCONFIG)(iomap_p2v((unsigned int)HwGRPBUSCONFIG_BASE));

	if(lmode)  // Normal
		lGRPPWDN->GRPBUS_SWRESET |= lindex[lpwdn];
	else // Power Down
		lGRPPWDN->GRPBUS_SWRESET &= ~lindex[lpwdn];
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setvideobuscfg(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of Video Bus Config
* ***************************************************************************************/
VOLATILE void tca_ckc_setvideobuscfgpwdn(unsigned int lpwdn , unsigned int lmode)
{
	PVIDEOBUSCFG pVIDEOBUSCFG;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	if (!(pCKC->CLK5CTRL & Hw21))
		return;

	pVIDEOBUSCFG = (PVIDEOBUSCFG)(iomap_p2v((unsigned int)HwVIDEOBUSCFG_BASE));

	if(lmode)  // Normal
		pVIDEOBUSCFG->PWDN &= ~lindex[lpwdn];
	else // Power Down
		pVIDEOBUSCFG->PWDN |= lindex[lpwdn];

}

/****************************************************************************************
* FUNCTION : int tca_ckc_getddipwdn(unsigned int lpwdn)
* DESCRIPTION : Power Down Register of Video Bus Config
* ***************************************************************************************/
VOLATILE int tca_ckc_getvideobuscfgpwdn(unsigned int lpwdn)
{
	PVIDEOBUSCFG pVIDEOBUSCFG;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	if (!(pCKC->CLK5CTRL & Hw21))
		return -1;

	pVIDEOBUSCFG = (PVIDEOBUSCFG)(iomap_p2v((unsigned int)HwVIDEOBUSCFG_BASE));

	return (pVIDEOBUSCFG->PWDN &  lindex[lpwdn]);
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setvideobuscfg(unsigned int lpwdn , unsigned int lmode)
* DESCRIPTION : Power Down Register of Video Bus Config
* ***************************************************************************************/
VOLATILE void tca_ckc_setvideobuscfgswreset(unsigned int lpwdn , unsigned int lmode)
{
	PVIDEOBUSCFG pVIDEOBUSCFG;
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	if (!(pCKC->CLK5CTRL & Hw21))
		return;

	pVIDEOBUSCFG = (PVIDEOBUSCFG)(iomap_p2v((unsigned int)HwVIDEOBUSCFG_BASE));

	if(lmode)
		pVIDEOBUSCFG->SWRESET |= lindex[lpwdn];
	else
		pVIDEOBUSCFG->SWRESET &= ~lindex[lpwdn];
}

/****************************************************************************************
* FUNCTION :void tca_ckc_getclkctrl0(void)
* DESCRIPTION :
* ***************************************************************************************/
VOLATILE unsigned int tca_ckc_getclkctrl0(void)
{
    return pCKC->CLK0CTRL;
}

VOLATILE void tca_ckc_sethsiobus(unsigned int lpwdn, unsigned int mode)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	PHSIOBUSCFG pHSIOBUSCFG = (PHSIOBUSCFG)(iomap_p2v((unsigned int)HwHSIOBUSCFG_BASE));

	if (!(pCKC->CLK8CTRL & Hw21))
		return;

	if(mode)
		pHSIOBUSCFG->HCLKMASK |= lindex[lpwdn];
	else
		pHSIOBUSCFG->HCLKMASK &= ~lindex[lpwdn];
}

VOLATILE int tca_ckc_gethsiobus(unsigned int lpwdn)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	PHSIOBUSCFG pHSIOBUSCFG = (PHSIOBUSCFG)(iomap_p2v((unsigned int)HwHSIOBUSCFG_BASE));

	if (!(pCKC->CLK8CTRL & Hw21))
		return 0;

	return (int)(pHSIOBUSCFG->HCLKMASK & lindex[lpwdn]);
}

VOLATILE void tca_ckc_sethsiobusswreset(unsigned int lpwdn, unsigned int mode)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	PHSIOBUSCFG pHSIOBUSCFG = (PHSIOBUSCFG)(iomap_p2v((unsigned int)HwHSIOBUSCFG_BASE));

	if (!(pCKC->CLK8CTRL & Hw21))
		return;

	if(mode)
		pHSIOBUSCFG->SWRESET &= ~lindex[lpwdn];
	else
		pHSIOBUSCFG->SWRESET |= lindex[lpwdn];
}

//This function can be used in only kernel mode.
VOLATILE int tca_ckc_setcommonhsiobus(unsigned int hsiocontroller, unsigned int enable)
{
    static unsigned int cur_status = 0;

	return 0;	// SangWon_Temp.  Not use


    if(enable == ENABLE)
    {
        if(cur_status == 0)
        {
            tca_ckc_setpll(5000000, 3);
            tca_ckc_setfbusctrl(CLKCTRL8, ENABLE, 0, 2500000, DIRECTPLL3);
        }
        cur_status |= hsiocontroller;
    }
    else
    {
        cur_status &= ~(hsiocontroller);
        if(cur_status == 0)
        {
            tca_ckc_setfbusctrl(CLKCTRL8, DISABLE, 0, 2500000, DIRECTPLL3);
            tca_ckc_setpll(0, 3);
        }
    }

	return cur_status;
}

VOLATILE void tca_ckc_setcambuspwdn(unsigned int lpwdn, unsigned int mode)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	PCAMBUSCFG pCAMBUSCFG = (PCAMBUSCFG)(iomap_p2v((unsigned int)HwCAMBUSCFG_BASE));

	if (!(pCKC->CLK9CTRL & Hw21))
		return;

	if(mode)
		pCAMBUSCFG->PowerDownMode &= ~lindex[lpwdn];
	else
		pCAMBUSCFG->PowerDownMode |= lindex[lpwdn];
}

VOLATILE int tca_ckc_getcambuspwdn(unsigned int lpwdn)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	PCAMBUSCFG pCAMBUSCFG = (PCAMBUSCFG)(iomap_p2v((unsigned int)HwCAMBUSCFG_BASE));

	if (!(pCKC->CLK9CTRL & Hw21))
		return 0;

	return (int)(pCAMBUSCFG->PowerDownMode & lindex[lpwdn]);
}

VOLATILE void tca_ckc_setcambusswreset(unsigned int lpwdn, unsigned int mode)
{
	unsigned int lindex[] = {Hw0,Hw1,Hw2,Hw3,Hw4,Hw5,Hw6,Hw7,Hw8,Hw9,Hw10,Hw11,Hw12,Hw13,Hw14,Hw15
							,Hw16,Hw17,Hw18,Hw19,Hw20,Hw21,Hw22,Hw23,Hw24,Hw25,Hw26,Hw27,Hw28,Hw29,Hw30,Hw31};

	PCAMBUSCFG pCAMBUSCFG = (PCAMBUSCFG)(iomap_p2v((unsigned int)HwCAMBUSCFG_BASE));

	if (!(pCKC->CLK9CTRL & Hw21))
		return;

	if(mode)
		pCAMBUSCFG->SoftResetRegister |= lindex[lpwdn];
	else
		pCAMBUSCFG->SoftResetRegister &= ~lindex[lpwdn];
}

VOLATILE void tca_ckc_enable(int clk, int enable)
{
	volatile unsigned *pCLKCTRL;
	pCLKCTRL = (volatile unsigned *)((&pCKC->CLK0CTRL)+clk);

	if (enable)
		*pCLKCTRL |= Hw21;
	else
		*pCLKCTRL &= ~Hw21;
}

VOLATILE void tca_ckc_pclk_enable(int pclk, int enable)
{
	volatile unsigned *pPERI;
	pPERI = (volatile unsigned *)((&pCKC->PCLK_TCX)+pclk);

	if (enable)
		*pPERI |= Hw29;
	else
		*pPERI &= ~Hw29;
}

/****************************************************************************************
* EXPORT_SYMBOL clock functions for Linux
* ***************************************************************************************/
#if defined(_LINUX_)
EXPORT_SYMBOL(tca_ckc_init);
EXPORT_SYMBOL(tca_ckc_getpll);
EXPORT_SYMBOL(tca_ckc_getcpu);
EXPORT_SYMBOL(tca_ckc_getbus);
//EXPORT_SYMBOL(tca_ckc_gclkctrlx);
//EXPORT_SYMBOL(tca_ckc_gclkctrly);
EXPORT_SYMBOL(tca_ckc_setfbusctrl);
EXPORT_SYMBOL(tca_ckc_getfbusctrl);
//EXPORT_SYMBOL(tca_ckc_gpllxcfg);
//EXPORT_SYMBOL(tca_ckc_pll);
EXPORT_SYMBOL(tca_ckc_validpll);
EXPORT_SYMBOL(tca_ckc_setpll);
EXPORT_SYMBOL(tca_ckc_setpmupwroff);
EXPORT_SYMBOL(tca_ckc_getpmupwroff);
//EXPORT_SYMBOL(tca_ckc_gpckxxx);
//EXPORT_SYMBOL(tca_ckc_gpckyyy);
EXPORT_SYMBOL(tca_ckc_setperi);
//EXPORT_SYMBOL(tca_ckc_gperi);
EXPORT_SYMBOL(tca_ckc_getperi);
EXPORT_SYMBOL(tca_ckc_set_iobus_swreset);
EXPORT_SYMBOL(tca_ckc_setswreset);
EXPORT_SYMBOL(tca_ckc_setiobus);
EXPORT_SYMBOL(tca_ckc_getiobus);
EXPORT_SYMBOL(tca_ckc_setsmui2c);
EXPORT_SYMBOL(tca_ckc_getsmui2c);
EXPORT_SYMBOL(tca_ckc_setddipwdn);
EXPORT_SYMBOL(tca_ckc_getddipwdn);
EXPORT_SYMBOL(tca_ckc_setvideobuscfgpwdn);
EXPORT_SYMBOL(tca_ckc_getvideobuscfgpwdn);
EXPORT_SYMBOL(tca_ckc_setvideobuscfgswreset);
EXPORT_SYMBOL(tca_ckc_sethsiobus);
EXPORT_SYMBOL(tca_ckc_gethsiobus);
EXPORT_SYMBOL(tca_ckc_sethsiobusswreset);
EXPORT_SYMBOL(tca_ckc_setcommonhsiobus);
#endif

/* end of file */
