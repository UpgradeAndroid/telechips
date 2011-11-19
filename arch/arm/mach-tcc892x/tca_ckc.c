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


#define CPU_SRC_CH  5
#define CPU_SRC_PLL DIRECTPLL5

#define MEM_SRC_CH  4
#define MEM_SRC_PLL DIRECTPLL4

typedef struct {
	unsigned int	uFpll;
	unsigned int	P;
	unsigned int	M;
	unsigned int	S;
	unsigned int	VSEL;
} sfPLL;

#define PLLFREQ(P, M, S)	(( 120000 * (M) )  / (P) ) >> (S) // 100Hz Unit..
#define FPLL_t(P, M, S, VSEL)	PLLFREQ(P,M,S), P, M, S, VSEL

static sfPLL pIO_CKC_PLL[] = {
	{FPLL_t(3, 400,  2,  1)},		// 400 MHz
	{FPLL_t(3, 432,  2,  1)},		// 432 MHz
	{FPLL_t(3, 500,  2,  1)},		// 500 MHz
	{FPLL_t(3, 297,  1,  0)},		// 594 MHz
	{FPLL_t(3, 300,  1,  0)},		// 600 MHz
	{FPLL_t(3, 336,  1,  0)},		// 672 MHz
	{FPLL_t(3, 350,  1,  0)},		// 700 MHz
	{FPLL_t(3, 384,  1,  1)},		// 768 MHz
	{FPLL_t(3, 400,  1,  1)},		// 800 MHz
	{FPLL_t(3, 450,  1,  1)},		// 900 MHz
	{FPLL_t(3, 498,  1,  1)},		// 996 MHz
	{FPLL_t(3, 251,  0,  0)},		// 1004 MHz
	{FPLL_t(3, 275,  0,  0)},		// 1100 MHz
	{FPLL_t(3, 300,  0,  0)},		// 1200 MHz
};

#define NUM_PLL				(sizeof(pIO_CKC_PLL)/sizeof(sfPLL))

#define	tca_wait()			{ volatile int i; for (i=0; i<0x2000; i++); }
#define iomap_p2v(pa)		((unsigned int)(pa))

/****************************************************************************************
* Local Variable
* ***************************************************************************************/
static PCKC				pCKC ;
static PPMU				pPMU ;
static PIOBUSCFG		pIOBUSCFG;
static PDDICONFIG		pDDIBUSCFG;
static PGRPBUSCFG		pGPUBUSCFG;
//static PVIDEOBUSCFG	pVIDEOBUSCFG;
static PHSIOBUSCFG		pHSIOBUSCFG;
static sfPLL			*pPLL;

/****************************************************************************************
* FUNCTION :void tca_ckc_init(void)
* DESCRIPTION :
* ***************************************************************************************/
void tca_ckc_init(void)
{
	pCKC = (CKC *)iomap_p2v(HwCKC_BASE);
	pPMU = (PMU *)iomap_p2v(HwPMU_BASE);
	pIOBUSCFG = (IOBUSCFG *)iomap_p2v(HwIOBUSCFG_BASE);
	pDDIBUSCFG = (DDICONFIG *)iomap_p2v(HwDDI_CONFIG_BASE);
	pGPUBUSCFG = (GRPBUSCFG *)iomap_p2v(HwGRPBUSCONFIG_BASE);
//	pVIDEOBUSCFG = (VIDEOBUSCFG *)iomap_p2v(HwVIDEOBUSCONFIG_BASE);
	pHSIOBUSCFG = (HSIOBUSCFG *)iomap_p2v(HwHSIOBUSCFG_BASE);

	/* IOBUS AHB2AXI: flushs prefetch buffer when bus state is IDLE or WRITE
	   enable:  A2XMOD1 (Audio DMA, GPSB, DMA2/3, EHI1)
	   disable: A2XMOD0 (USB1.1Host, USB OTG, SD/MMC, IDE, DMA0/1, MPEFEC, EHI0)
	*/
	pIOBUSCFG->IO_A2X.bREG.A2XMOD1 = 1;
	pIOBUSCFG->IO_A2X.bREG.A2XMOD0 = 1;
	pHSIOBUSCFG->HSIO_CFG.bREG.A2X_USB20H = 1;

	pCKC->CLKDIVC0.nREG	= 0x01010101;	// PLL0,PLL1,PLL2,PLL3
	pCKC->CLKDIVC1.nREG	= 0x01010101;	// PLL4,PLL5,XIN,XTIN

}

/****************************************************************************************
* FUNCTION :int tca_ckc_setpll(unsigned int pll, unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setpll(unsigned int pll, unsigned int ch)
{
	unsigned uCnt;

	if ((ch == CPU_SRC_CH) || (ch == MEM_SRC_CH))
		return -1;

	PLLCFG_TYPE *pPLLCFG = (PLLCFG_TYPE *)((&pCKC->PLL0CFG)+ch);

 	if(pll != 0) {
		pPLL = &pIO_CKC_PLL[0];
		for (uCnt = 0; uCnt < NUM_PLL; uCnt ++, pPLL++) {
			if (pPLL->uFpll <= pll)
				break;
		}

		if ((pPLL->uFpll < pll) && (uCnt > 0))
			pPLL--;

		if (uCnt >= NUM_PLL)
			uCnt = NUM_PLL - 1;

		pPLLCFG->bREG.EN = 0;
		pPLLCFG->bREG.VSEL = pPLL->VSEL;
		pPLLCFG->bREG.P = pPLL->P;
		pPLLCFG->bREG.M = pPLL->M;
		pPLLCFG->bREG.S = pPLL->S;
		pPLLCFG->bREG.EN = 1;
		tca_wait();
	}
	else {
		pPLLCFG->bREG.EN = 0;
	}
	return 0;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getpll(unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
unsigned int tca_ckc_getpll(unsigned int ch)
{
	unsigned int tPLL;
	unsigned int tPCO;
	unsigned	iP=0, iM=0, iS=0;

	switch(ch) {
		case 0:
			iP = pCKC->PLL0CFG.bREG.P;
			iM = pCKC->PLL0CFG.bREG.M;
			iS = pCKC->PLL0CFG.bREG.S;
			break;
		case 1:
			iP = pCKC->PLL1CFG.bREG.P;
			iM = pCKC->PLL1CFG.bREG.M;
			iS = pCKC->PLL1CFG.bREG.S;
			break;
		case 2:
			iP = pCKC->PLL2CFG.bREG.P;
			iM = pCKC->PLL2CFG.bREG.M;
			iS = pCKC->PLL2CFG.bREG.S;
			break;
		case 3:
			iP = pCKC->PLL3CFG.bREG.P;
			iM = pCKC->PLL3CFG.bREG.M;
			iS = pCKC->PLL3CFG.bREG.S;
			break;
		case 4:
			iP = pCKC->PLL4CFG.bREG.P;
			iM = pCKC->PLL4CFG.bREG.M;
			iS = pCKC->PLL4CFG.bREG.S;
			break;
		case 5:
			iP = pCKC->PLL5CFG.bREG.P;
			iM = pCKC->PLL5CFG.bREG.M;
			iS = pCKC->PLL5CFG.bREG.S;
			break;
	}

	tPCO = (120000 * iM ) / iP;
	tPLL= ((tPCO) >> (iS));

	return tPLL;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getdividpll(unsigned int ch)
* DESCRIPTION :
* ***************************************************************************************/
unsigned int tca_ckc_getdividpll(unsigned int ch)
{
	unsigned int tDIVPLL;
	unsigned int tPLL = tca_ckc_getpll(ch);
	unsigned int uiPDIV = 0;

	switch(ch) {
		case 0:
			uiPDIV = pCKC->CLKDIVC0.bREG.P0DIV;
			break;
		case 1:
			uiPDIV = pCKC->CLKDIVC0.bREG.P1DIV;
			break;
		case 2:
			uiPDIV = pCKC->CLKDIVC0.bREG.P2DIV;
			break;
		case 3:
			uiPDIV = pCKC->CLKDIVC0.bREG.P3DIV;
			break;
		case 4:
			uiPDIV = pCKC->CLKDIVC1.bREG.P4DIV;
			break;
		case 5:
			uiPDIV = pCKC->CLKDIVC1.bREG.P5DIV;
			break;
	}

	//Fdivpll Clock
	tDIVPLL = (unsigned int)tPLL/(uiPDIV+1);

	return tDIVPLL;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setcpu(unsigned int n)
* DESCRIPTION :  n is n/16
* example : CPU == PLL : n=16 - CPU == PLL/2 : n=8
* ***************************************************************************************/
static int tca_ckc_setcpu(unsigned int freq)
{
	unsigned uCnt;
	PLLCFG_TYPE *pPLLCFG = (PLLCFG_TYPE *)((&pCKC->PLL0CFG)+CPU_SRC_CH);

	if (freq == 0)
		return -1;;

	// 1. temporally change the cpu clock source.(XIN)
	pCKC->CLKCTRL0.bREG.SEL = DIRECTXIN;
	pCKC->CLKCTRL0.bREG.CONFIG = 0xFFFF;

	// 2. change pll(for cpu) clock.
	pPLL = &pIO_CKC_PLL[0];
	for (uCnt = 0; uCnt < NUM_PLL; uCnt ++, pPLL++) {
		if (pPLL->uFpll <= freq)
			break;
	}
	if ((pPLL->uFpll < freq) && (uCnt > 0))
		pPLL--;
	if (uCnt >= NUM_PLL)
		uCnt = NUM_PLL - 1;
	pPLLCFG->bREG.EN = 0;
	pPLLCFG->bREG.VSEL = pPLL->VSEL;
	pPLLCFG->bREG.P = pPLL->P;
	pPLLCFG->bREG.M = pPLL->M;
	pPLLCFG->bREG.S = pPLL->S;
	pPLLCFG->bREG.EN = 1;
	tca_wait();

	// 3. change th cpu clock source.
	pCKC->CLKCTRL0.bREG.CONFIG = 0xFFFF;
	pCKC->CLKCTRL0.bREG.SEL = CPU_SRC_PLL;

	return 0;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getcpu(void)
* DESCRIPTION :
* ***************************************************************************************/
static unsigned int tca_ckc_getcpu(void)
{
	unsigned int lcpu = 0;
	unsigned int lconfig = 0;
	unsigned int lcnt = 0;
	unsigned int li = 0;
	unsigned int lclksource = 0;

	lconfig = pCKC->CLKCTRL0.bREG.CONFIG;

	for(li = 0; li < 16; li++) {
		if((lconfig & Hw0) == 1)
			lcnt++;
		lconfig = (lconfig >> 1);
	}

	switch(pCKC->CLKCTRL0.bREG.SEL) {
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
			lclksource =  tca_ckc_getpll(5);
			break;
	}

	lcpu = (lclksource * lcnt)/16;

	return lcpu;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setfbusctrl(unsigned int clkname,unsigned int isenable,unsigned int freq, unsigned int sor)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setfbusctrl(unsigned int clkname, unsigned int isenable, unsigned int freq, unsigned int sor)
{
	CLKCTRL_TYPE *pCLKCTRL = (CLKCTRL_TYPE *)((&pCKC->CLKCTRL0)+clkname);
	unsigned int clkdiv = 0;
	unsigned int clksource = 0;

	if (clkname == FBUS_CPU)
		return tca_ckc_setcpu(freq);
//	else if (clkname == FBUS_MEM)	// Memory
//		return change_mem_clock(freq);

	switch(sor) {
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

	if (freq != 0) {
		clkdiv	= (clksource + (freq>>1)) / freq ;	// should be even number of division factor
		clkdiv -= 1;
	}
	else
		clkdiv	= 1;

	if(clkdiv == CLKDIV0) // The config value should not be "ZERO" = 1/(config+1)
		clkdiv = 1;

	if(isenable == 0)
		pCLKCTRL->bREG.EN = 0;
	else {
		pCLKCTRL->bREG.SEL = DIRECTXIN;
		pCLKCTRL->bREG.CONFIG = clkdiv;
		pCLKCTRL->bREG.SEL = sor;
		pCLKCTRL->bREG.EN = 1;
	}

	return 0;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_getfbusctrl(unsigned int clkname)
* DESCRIPTION :
* ***************************************************************************************/
unsigned int tca_ckc_getfbusctrl(unsigned int clkname)
{
	CLKCTRL_TYPE *pCLKCTRL = (CLKCTRL_TYPE *)((&pCKC->CLKCTRL0)+clkname);
	unsigned int clksource = 0;

	if(clkname == FBUS_CPU)
		return tca_ckc_getcpu();

	switch(pCLKCTRL->bREG.SEL) {
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

	return (clksource / (pCLKCTRL->bREG.CONFIG+1));
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setperi(unsigned int periname,unsigned int isenable, unsigned int freq, unsigned int sor)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setperi(unsigned int periname,unsigned int isenable, unsigned int freq, unsigned int sor)
{
	PCLK_XXX_TYPE *pPCLKCTRL_XXX = (PCLK_XXX_TYPE *)((&pCKC->PCLKCTRL00)+periname);
	PCLK_YYY_TYPE *pPCLKCTRL_YYY = (PCLK_YYY_TYPE *)((&pCKC->PCLKCTRL00)+periname);
	unsigned int lclksource = 0;
	unsigned int clkdiv;

	switch(sor) {
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
		default :
			lclksource =  tca_ckc_getpll(1);
			break;
	}

	if (freq != 0) {
		clkdiv	= (lclksource + (freq>>1)) / freq ;	// should be even number of division factor
		clkdiv -= 1;
	}
	else
		clkdiv	= 0;

	if (periname == PERI_HDMIA || periname == PERI_ADMA1 || periname == PERI_ADAM1 || periname == PERI_SPDIF1 ||
		periname == PERI_ADMA0 || periname == PERI_ADAM0 || periname == PERI_SPDIF0 || periname == PERI_ADC)
	{
		if (periname == PERI_HDMIA || periname == PERI_ADMA1 || periname == PERI_ADAM1 || periname == PERI_SPDIF1 ||
			periname == PERI_ADMA0 || periname == PERI_ADAM0 || periname == PERI_SPDIF0/* || periname == PERI_ADC*/) 
		{
			pPCLKCTRL_YYY->bREG.MD = 0;		// DCO mode

			if(freq >= 131071)
			{
				clkdiv = (freq *16384);
				clkdiv = clkdiv/lclksource;
				clkdiv <<= 2;
				clkdiv = clkdiv + 1;
			}
			else
			{
				clkdiv = (freq *32768);
				clkdiv = clkdiv/lclksource;
				clkdiv <<= 1;
				clkdiv = clkdiv + 1;
			}
		}
		else
			pPCLKCTRL_YYY->bREG.MD = 1;		// DIVIDER mode

		pPCLKCTRL_YYY->bREG.EN = 0;
		pPCLKCTRL_YYY->bREG.DIV = clkdiv;
		pPCLKCTRL_YYY->bREG.SEL = sor;

		if (isenable)
			pPCLKCTRL_YYY->bREG.EN = 1;
	}
	else {
		pPCLKCTRL_XXX->bREG.EN = 0;
		pPCLKCTRL_XXX->bREG.DIV = clkdiv;
		pPCLKCTRL_XXX->bREG.SEL = sor;

		if (isenable)
			pPCLKCTRL_XXX->bREG.EN = 1;
	}

	return 0;
}

/****************************************************************************************
* FUNCTION : int tca_ckc_getperi(unsigned int periname)
* DESCRIPTION :
* ***************************************************************************************/
unsigned int tca_ckc_getperi(unsigned int periname)
{
	PCLK_XXX_TYPE *pPCLKCTRL_XXX = (PCLK_XXX_TYPE *)((&pCKC->PCLKCTRL00)+periname);
	PCLK_YYY_TYPE *pPCLKCTRL_YYY = (PCLK_YYY_TYPE *)((&pCKC->PCLKCTRL00)+periname);
	unsigned int lclksource = 0;

	switch(pPCLKCTRL_XXX->bREG.SEL) {
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
		/*		
		case PCSATA :
			lclksource =  250000;
			break;
		case PCUSBPHY:
			lclksource =  480000;
			break;
		*/
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
		default :
			return 0;
	}

	if (periname == PERI_HDMIA || periname == PERI_ADMA1 || periname == PERI_ADAM1 || periname == PERI_SPDIF1 ||
		periname == PERI_ADMA0 || periname == PERI_ADAM0 || periname == PERI_SPDIF0 || periname == PERI_ADC)
	{
		if (pPCLKCTRL_YYY->bREG.MD) {
			return (lclksource/(pPCLKCTRL_YYY->bREG.DIV+1));
		}
		else {
			if (pPCLKCTRL_YYY->bREG.DIV > 32768)
				return ((lclksource * (65536 - pPCLKCTRL_YYY->bREG.DIV)) / 65536);
			else
				return ((lclksource * pPCLKCTRL_YYY->bREG.DIV) / 65536);
		}
	}
	else {
		return (lclksource/(pPCLKCTRL_XXX->bREG.DIV+1));
	}

	return 0;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setpmuippwdn( unsigned int sel , unsigned int ispwdn)
* DESCRIPTION :
*   - fbusname : IP Isolation index
*   - ispwdn : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_setippwdn( unsigned int sel, unsigned int ispwdn)
{
	unsigned int ctrl_value;

	if (ispwdn)
		ctrl_value = 0;
	else
		ctrl_value = 1;
	
	switch (sel) {
		case PMU_ISOL_OTP:
			pPMU->PMU_ISOL.bREG.OTP = ctrl_value;
			break;
		case PMU_ISOL_RTC:
			pPMU->PMU_ISOL.bREG.RTC = ctrl_value;
			break;
		case PMU_ISOL_PLL:
			pPMU->PMU_ISOL.bREG.PLL = ctrl_value;
			break;
		case PMU_ISOL_ECID:
			pPMU->PMU_ISOL.bREG.ECID = ctrl_value;
			break;
		case PMU_ISOL_HDMI:
			pPMU->PMU_ISOL.bREG.HDMI = ctrl_value;
			break;
		case PMU_ISOL_VDAC:
			pPMU->PMU_ISOL.bREG.VDAC = ctrl_value;
			break;
		case PMU_ISOL_TSADC:
			pPMU->PMU_ISOL.bREG.TSADC = ctrl_value;
			break;
		case PMU_ISOL_USBHP:
			pPMU->PMU_ISOL.bREG.USBHP = ctrl_value;
			break;
		case PMU_ISOL_USBOP:
			pPMU->PMU_ISOL.bREG.USBOP = ctrl_value;
			break;
		case PMU_ISOL_LVDS:
			pPMU->PMU_ISOL.bREG.LVDS = ctrl_value;
			break;
		default:
			return -1;
	}

	return 0;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setfbuspwdn( unsigned int fbusname , unsigned int ispwdn)
* DESCRIPTION :
*   - fbusname : CLKCTRL(n) index
*   - ispwdn : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_setfbuspwdn( unsigned int fbusname, unsigned int ispwdn)
{
	switch (fbusname) {
		case FBUS_MEM:
			if (ispwdn)
				pPMU->PWRDN_MBUS.bREG.DATA = 0;
			else
				pPMU->PWRUP_MBUS.bREG.DATA = 0;
			break;
		case FBUS_DDI:
			if (ispwdn)
				pPMU->PWRDN_DBUS.bREG.DATA = 0;
			else
				pPMU->PWRUP_DBUS.bREG.DATA = 0;
			break;
		case FBUS_GPU:
			if (ispwdn)
				pPMU->PWRDN_GBUS.bREG.DATA = 0;
			else
				pPMU->PWRUP_GBUS.bREG.DATA = 0;
			break;
		case FBUS_VBUS:
			if (ispwdn)
				pPMU->PWRDN_VBUS.bREG.DATA = 0;
			else
				pPMU->PWRUP_VBUS.bREG.DATA = 0;
			break;
		case FBUS_HSIO:
			if (ispwdn)
				pPMU->PWRDN_HSBUS.bREG.DATA = 0;
			else
				pPMU->PWRUP_HSBUS.bREG.DATA = 0;
			break;
		default:
			return -1;
	}

	return 0;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_getfbuspwdn( unsigned int fbusname)
* DESCRIPTION :
*   - fbusname : CLKCTRL(n) index
*   - return : 1:pwdn, 0:wkup
* ***************************************************************************************/
int tca_ckc_getfbuspwdn( unsigned int fbusname)
{
	int retVal = 0;

	switch (fbusname) {
		case FBUS_MEM:
			if (pPMU->PMU_PWRSTS.bREG.PU_MB)
				retVal = 1;
			break;
		case FBUS_DDI:
			if (pPMU->PMU_PWRSTS.bREG.PU_DB)
				retVal = 1;
			break;
		case FBUS_GPU:
			if (pPMU->PMU_PWRSTS.bREG.PU_GB)
				retVal = 1;
			break;
//		case FBUS_VCORE:
		case FBUS_VBUS:
			if (pPMU->PMU_PWRSTS.bREG.PU_VB)
				retVal = 1;
			break;
		case FBUS_HSIO:
			if (pPMU->PMU_PWRSTS.bREG.PU_HSB)
				retVal = 1;
			break;
		default :
			retVal = -1;
			break;
	}

 	return retVal;
}

/****************************************************************************************
* FUNCTION :void tca_ckc_setswreset(unsigned int fbus_name, unsigned int isreset)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setfbusswreset(unsigned int fbusname, unsigned int isreset)
{
	int ctrl_value;

	if (isreset)
		ctrl_value = 0;
	else
		ctrl_value = 1;

	switch (fbusname) {
		case FBUS_DDI:
			pCKC->SWRESET.bREG.DDI  = ctrl_value;
			break;
		case FBUS_GPU:
			pCKC->SWRESET.bREG.GPU  = ctrl_value;
			break;
		case FBUS_VBUS:
//		case FBUS_VCORE:  
			pCKC->SWRESET.bREG.VBUS = ctrl_value;
			pCKC->SWRESET.bREG.VCORE = ctrl_value;
			break;
		case FBUS_HSIO:
			pCKC->SWRESET.bREG.HSIO = ctrl_value;
			break;
		default:
			return -1;
	}

	return 0;
}

/****************************************************************************************
* FUNCTION :  int tca_ckc_setiopwdn(unsigned int sel, unsigned int ispwdn)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setiopwdn(unsigned int sel, unsigned int ispwdn)
{
	if (sel >= RB_MAX)
		return -1;

	if (pCKC->CLKCTRL4.bREG.EN == 0)
		return -2;

	if (ispwdn)
		pIOBUSCFG->HCLKEN.nREG &= ~(0x1 << sel);
	else
		pIOBUSCFG->HCLKEN.nREG |= (0x1 << sel);

	return 0;
}

/****************************************************************************************
* FUNCTION :  int tca_ckc_getiobus(unsigned int sel)
* DESCRIPTION :
*   - return : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_getiopwdn(unsigned int sel)
{
	int retVal = 0;

	if (sel >= RB_MAX)
		return -1;

	if (pCKC->CLKCTRL4.bREG.EN == 0)
		return -2;

	if (pIOBUSCFG->HCLKEN.nREG & (0x1 << sel))
		retVal = 0;
	else
		retVal = 1;

	return retVal;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_setioswreset(unsigned int sel, unsigned int isreset)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setioswreset(unsigned int sel, unsigned int isreset)
{
	if (sel >= RB_MAX)
		return -1;

	if (pCKC->CLKCTRL4.bREG.EN == 0)
		return -2;

	if (isreset)
		pIOBUSCFG->HRSTEN.nREG &= ~(0x1 << sel);
	else
		pIOBUSCFG->HRSTEN.nREG |= (0x1 << sel);

	return 0;
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setddipwdn(unsigned int sel , unsigned int ispwdn)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setddipwdn(unsigned int sel , unsigned int ispwdn)
{
	if (sel >= DDIBUS_MAX)
		return -1;

	if (pCKC->CLKCTRL2.bREG.EN == 0)
		return -2;

	if (ispwdn)
		pDDIBUSCFG->PWDN.nREG |= (0x1 << sel);
	else
		pDDIBUSCFG->PWDN.nREG &= ~(0x1 << sel);

	return 0;
}

/****************************************************************************************
* FUNCTION : int tca_ckc_getddipwdn(unsigned int sel)
* DESCRIPTION :
*   - return : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_getddipwdn(unsigned int sel)
{
	int retVal = 0;

	if (sel >= DDIBUS_MAX)
		return -1;

	if (pCKC->CLKCTRL2.bREG.EN == 0)
		return -2;

	if (pDDIBUSCFG->PWDN.nREG & (0x1 << sel))
		retVal = 1;
	else
		retVal = 0;

	return retVal;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_setddiswreset(unsigned int sel, unsigned int isreset)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setddiswreset(unsigned int sel, unsigned int isreset)
{
	if (sel >= DDIBUS_MAX)
		return -1;

	if (pCKC->CLKCTRL2.bREG.EN == 0)
		return -2;

	if (isreset)
		pDDIBUSCFG->SWRESET.nREG |= (0x1 << sel);
	else
		pDDIBUSCFG->SWRESET.nREG &= ~(0x1 << sel);

	return 0;
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setgpupwdn(unsigned int sel , unsigned int ispwdn)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setgpupwdn(unsigned int sel , unsigned int ispwdn)
{
	if (sel >= GPUBUS_MAX)
		return -1;

	if (pCKC->CLKCTRL3.bREG.EN == 0)
		return -2;

	if (ispwdn)
		pGPUBUSCFG->PWDN.nREG |= (0x1 << sel);
	else
		pGPUBUSCFG->PWDN.nREG &= ~(0x1 << sel);

	return 0;
}

/****************************************************************************************
* FUNCTION : int tca_ckc_getgpupwdn(unsigned int sel)
* DESCRIPTION :
*   - return : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_getgpupwdn(unsigned int sel)
{
	int retVal = 0;

	if (sel >= GPUBUS_MAX)
		return -1;

	if (pCKC->CLKCTRL3.bREG.EN == 0)
		return -2;

	if (pGPUBUSCFG->PWDN.nREG & (0x1 << sel))
		retVal = 1;
	else
		retVal = 0;

	return retVal;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_setgpuswreset(unsigned int sel, unsigned int isreset)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setgpuswreset(unsigned int sel, unsigned int isreset)
{
	if (sel >= GPUBUS_MAX)
		return -1;

	if (pCKC->CLKCTRL3.bREG.EN == 0)
		return -2;

	if (isreset)
		pGPUBUSCFG->SWRESET.nREG |= (0x1 << sel);
	else
		pGPUBUSCFG->SWRESET.nREG &= ~(0x1 << sel);

	return 0;
}

/****************************************************************************************
* FUNCTION : void tca_ckc_setvideopwdn(unsigned int sel , unsigned int ispwdn)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setvideopwdn(unsigned int sel , unsigned int ispwdn)
{
#if 0
	if (sel >= VIDEOBUS_MAX)
		return -1;

	if (pCKC->CLKCTRL5.bREG.EN == 0)
		return -2;

	if (ispwdn)
		pVIDEOBUSCFG->PWDN.nREG |= (0x1 << sel);
	else
		pVIDEOBUSCFG->PWDN.nREG &= ~(0x1 << sel);
#endif
	return 0;
}

/****************************************************************************************
* FUNCTION : int tca_ckc_getvideopwdn(unsigned int sel)
* DESCRIPTION :
*   - return : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_getvideopwdn(unsigned int sel)
{
	int retVal = 0;
#if 0
	if (sel >= VIDEOBUS_MAX)
		return -1;

	if (pCKC->CLKCTRL5.bREG.EN == 0)
		return -2;

	if (pVIDEOBUSCFG->PWDN.nREG & (0x1 << sel))
		retVal = 1;
	else
		retVal = 0;
#endif
	return retVal;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_setvideoswreset(unsigned int sel, unsigned int isreset)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_setvideoswreset(unsigned int sel, unsigned int isreset)
{
#if 0
	if (sel >= VIDEOBUS_MAX)
		return -1;

	if (pCKC->CLKCTRL5.bREG.EN == 0)
		return -2;

	if (isreset)
		pVIDEOBUSCFG->SWRESET.nREG |= (0x1 << sel);
	else
		pVIDEOBUSCFG->SWRESET.nREG &= ~(0x1 << sel);
#endif
	return 0;
}

/****************************************************************************************
* FUNCTION : void tca_ckc_sethsiopwdn(unsigned int sel , unsigned int ispwdn)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_sethsiopwdn(unsigned int sel , unsigned int ispwdn)
{
	if (sel >= HSIOBUS_MAX)
		return -1;

	if (pCKC->CLKCTRL7.bREG.EN == 0)
		return -2;

	if (ispwdn)
		pHSIOBUSCFG->PWDN.nREG &= ~(0x1 << sel);
	else
		pHSIOBUSCFG->PWDN.nREG |= (0x1 << sel);

	return 0;
}

/****************************************************************************************
* FUNCTION : int tca_ckc_gethsiopwdn(unsigned int sel)
* DESCRIPTION : Power Down Register of DDI_CONFIG
*   - return : (1:pwdn, 0:wkup)
* ***************************************************************************************/
int tca_ckc_gethsiopwdn(unsigned int sel)
{
	int retVal = 0;

	if (sel >= HSIOBUS_MAX)
		return -1;

	if (pCKC->CLKCTRL7.bREG.EN == 0)
		return -2;

	if (pHSIOBUSCFG->PWDN.nREG & (0x1 << sel))
		retVal = 0;
	else
		retVal = 1;

	return retVal;
}

/****************************************************************************************
* FUNCTION :unsigned int tca_ckc_sethsioswreset(unsigned int sel, unsigned int isreset)
* DESCRIPTION :
* ***************************************************************************************/
int tca_ckc_sethsioswreset(unsigned int sel, unsigned int isreset)
{
	if (sel >= HSIOBUS_MAX)
		return -1;

	if (pCKC->CLKCTRL7.bREG.EN == 0)
		return -2;

	if (isreset)
		pHSIOBUSCFG->SWRESET.nREG &= ~(0x1 << sel);
	else
		pHSIOBUSCFG->SWRESET.nREG |= (0x1 << sel);

	return 0;
}

int tca_ckc_fclk_enable(unsigned int fclk, unsigned int enable)
{
	volatile CLKCTRL_TYPE *pCLKCTRL;
	pCLKCTRL = (volatile CLKCTRL_TYPE *)((&pCKC->CLKCTRL0)+fclk);

	if (enable)
		pCLKCTRL->bREG.EN = 1;
	else
		pCLKCTRL->bREG.EN = 0;

	return 0;
}

int tca_ckc_pclk_enable(unsigned int pclk, unsigned int enable)
{
	volatile PCLK_XXX_TYPE *pPERI;
	pPERI = (volatile PCLK_XXX_TYPE *)((&pCKC->PCLKCTRL00)+pclk);

	if (enable)
		pPERI->bREG.EN = 1;
	else
		pPERI->bREG.EN = 0;

	return 0;
}

/****************************************************************************************
* EXPORT_SYMBOL clock functions for Linux
* ***************************************************************************************/
#if defined(_LINUX_)

EXPORT_SYMBOL(tca_ckc_init);
EXPORT_SYMBOL(tca_ckc_setpll);
EXPORT_SYMBOL(tca_ckc_getpll);
EXPORT_SYMBOL(tca_ckc_getdividpll);
EXPORT_SYMBOL(tca_ckc_setcpu);
EXPORT_SYMBOL(tca_ckc_getcpu);
EXPORT_SYMBOL(tca_ckc_setfbusctrl);
EXPORT_SYMBOL(tca_ckc_getfbusctrl);
EXPORT_SYMBOL(tca_ckc_setperi);
EXPORT_SYMBOL(tca_ckc_getperi);
EXPORT_SYMBOL(tca_ckc_setippwdn);
EXPORT_SYMBOL(tca_ckc_setfbuspwdn);
EXPORT_SYMBOL(tca_ckc_getfbuspwdn);
EXPORT_SYMBOL(tca_ckc_setfbusswreset);
EXPORT_SYMBOL(tca_ckc_setiopwdn);
EXPORT_SYMBOL(tca_ckc_getiopwdn);
EXPORT_SYMBOL(tca_ckc_setioswreset);
EXPORT_SYMBOL(tca_ckc_setddipwdn);
EXPORT_SYMBOL(tca_ckc_getddipwdn);
EXPORT_SYMBOL(tca_ckc_setddiswreset);
EXPORT_SYMBOL(tca_ckc_setgpupwdn);
EXPORT_SYMBOL(tca_ckc_getgpupwdn);
EXPORT_SYMBOL(tca_ckc_setgpuswreset);
#if 0
EXPORT_SYMBOL(tca_ckc_setvideopwdn);
EXPORT_SYMBOL(tca_ckc_getvideopwdn);
EXPORT_SYMBOL(tca_ckc_setvideoswreset);
#endif
EXPORT_SYMBOL(tca_ckc_sethsiopwdn);
EXPORT_SYMBOL(tca_ckc_gethsiopwdn);
EXPORT_SYMBOL(tca_ckc_sethsioswreset);
EXPORT_SYMBOL(tca_ckc_fclk_enable);
EXPORT_SYMBOL(tca_ckc_pclk_enable);

#endif

/* end of file */
