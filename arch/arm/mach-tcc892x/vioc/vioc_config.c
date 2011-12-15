/******************************************************************************
*
*  (C)Copyright All Rights Reserved by Telechips Inc.
*
*  This material is confidential and shall remain as such.
*  Any unauthorized use, distribution, reproduction is strictly prohibited.
*
*******************************************************************************
*
*  File Name   : Vioc_config.c
*
*  Description :
*
*******************************************************************************
*
*  yyyy/mm/dd     ver            descriptions                Author
*	---------	--------   ---------------       -----------------
*    2011/08/08     0.1            created                      hskim
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/delay.h>
#include <mach/bsp.h>
#include <mach/io.h>

#include <mach/vioc_plugin_tcc892x.h>
#include <mach/vioc_config.h>

#if 0
VIOC_CONFIG *gpConfig = (VIOC_CONFIG *)HwVIOC_PCONFIG;

void VIOC_CONFIG_SetWindowMixerPath(unsigned int nChannel, unsigned int nSub, unsigned int nValue)
{
	if (nSub == 0)
	{
		switch (nChannel)
		{
			case (VIOC_CONFIG_WMIX0) :
				gpConfig->uMISC.bReg.WMIX0_0 = nValue;
				gpConfig->uMISC.bReg.WMIX0_1 = nValue;
				break;
			case (VIOC_CONFIG_WMIX1) :
				gpConfig->uMISC.bReg.WMIX1_0 = nValue;
				gpConfig->uMISC.bReg.WMIX1_1 = nValue;
				break;
			case (VIOC_CONFIG_WMIX2) :
				gpConfig->uMISC.bReg.WMIX2_0 = nValue;
				gpConfig->uMISC.bReg.WMIX2_1 = nValue;
				break;
			default:
				break;
		}
	}
	else if (nSub == 1)
	{
		switch (nChannel)
		{
			case (VIOC_CONFIG_WMIX0) :
				gpConfig->uMISC.bReg.RDMA03 = nValue;
				break;
			default:
				break;
		}
	}
}
#endif

VIOC_CONFIG_PATH_u *VIOC_CONFIG_GetPathStruct (unsigned int nType)
{
	VIOC_CONFIG_PATH_u *pConfigPath = NULL;
	volatile PVIOC_IREQ_CONFIG pIREQConfig;
	pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

	switch(nType)
	{
		case (VIOC_SC0):
			//pConfigPath = &(pIREQConfig->uSC0.bREG);
			pConfigPath = &pIREQConfig->uSC0.nREG;
			break;
		case (VIOC_SC1):
			//pConfigPath = &(pIREQConfig->uSC0.bREG);
			pConfigPath = &pIREQConfig->uSC1.nREG;
			break;
		case (VIOC_SC2):
			//pConfigPath = &(pIREQConfig->uSC0.bREG);
			pConfigPath = &pIREQConfig->uSC2.nREG;
			break;
		case (VIOC_SC3):
			//pConfigPath = &(pIREQConfig->uSC0.bREG);
			pConfigPath = &pIREQConfig->uSC3.nREG;
			break;
		case (VIOC_VIQE):
			pConfigPath = &pIREQConfig->uVIQE.nREG;
			break;
		case (VIOC_DEINTLS):
			pConfigPath = &pIREQConfig->uDEINTLS.nREG;
			break;
		//case (VIOC_VIQE):
		//case (VIOC_DEINTLS):
		//case (VIOC_FILT2D   ) : return (&gpConfig->uFILT2D.bReg);
		//case (VIOC_FCDEC0   ) : return (&gpConfig->uFCDEC0.bReg);
		//case (VIOC_FCDEC1   ) : return (&gpConfig->uFCDEC1.bReg);
		//case (VIOC_FCDEC2   ) : return (&gpConfig->uFCDEC2.bReg);
		//case (VIOC_FCDEC3   ) : return (&gpConfig->uFCDEC3.bReg);
		//case (VIOC_FCENC0   ) : return (&gpConfig->uFCENC0.bReg);
		//case (VIOC_FCENC1   ) : return (&gpConfig->uFCENC1.bReg);
		//case (VIOC_FDELAY0  ) : return (&gpConfig->uFDELAY0.bReg);
		//case (VIOC_FDELAY1  ) : return (&gpConfig->uFDELAY1.bReg);
		//case (VIOC_DEBLOCK  ) : return (&gpConfig->uDEBLOCK.bReg);
		default:
			break;
	}

	return pConfigPath;
}

int VIOC_CONFIG_PlugIn (unsigned int nType, unsigned int nValue)
{
	VIOC_CONFIG_PATH_u *pConfigPath = NULL;
	unsigned int nStatus;

	pConfigPath = VIOC_CONFIG_GetPathStruct(nType);
	if(pConfigPath == NULL)
	{
		printk("Invalid Path Type ... \n");
		return (VIOC_DEVICE_INVALID);
	}

	//pConfigPath->SELECT = nValue;
	//pConfigPath->EN = 1;
	BITCSET(pConfigPath->nREG, 0xFF, nValue);
	BITCSET(pConfigPath->nREG, (0x1<<31), (0x1<<31));

	//if(pConfigPath->ERR)
	if(((pConfigPath->nREG>>18) & 0x1))
	{
		printk("Path Configuration Error : Device is Busy(1) ... \n");
		//pConfigPath->EN = 0;
		BITCSET(pConfigPath->nREG, (0x1<<31), (0x0<<31));
		return (VIOC_DEVICE_BUSY);
	}

	while(1) {
		//nStatus = pConfigPath->STATUS;
		nStatus = (pConfigPath->nREG>>16) & 0x3;
		if(nStatus == VIOC_PATH_CONNECTED ) {
			//printk("Path Configuration Done ... \n");
			break;
		} else {
			printk("Path Configuration Error : Device is Busy(2) ... \n");
			msleep(10);
		}
			
	}

	return (VIOC_DEVICE_CONNECTED);
}

int VIOC_CONFIG_PlugOut(unsigned int nType)
{
	VIOC_CONFIG_PATH_u *pConfigPath = NULL;
	unsigned int nStatus;
	unsigned int loop = 0x20000;
	pConfigPath = VIOC_CONFIG_GetPathStruct(nType);

	if(pConfigPath == NULL)
	{
		printk("Invalid Path Type ... \n");
		return (VIOC_DEVICE_INVALID);
	}

	//pConfigPath->EN = 0;
	BITCSET(pConfigPath->nREG, (0x1<<31), (0x0<<31));

	//if(pConfigPath->ERR)
	if(((pConfigPath->nREG>>18) & 0x1))
	{
		printk("Path Configuration Error : Device is Busy(1) ... \n");
		//pConfigPath->EN = 0;
		BITCSET(pConfigPath->nREG, (0x1<<31), (0x0<<31));
		return (VIOC_DEVICE_BUSY);
	}

	while (1)
	{
		//nStatus = pConfigPath->STATUS;
		nStatus = (pConfigPath->nREG>>16) & 0x3;
		#if 0 //[[ hskim_20110816_BEGIN -- .
		sim_value (nStatus);
		#endif //]] hskim_20110816_END -- .

		loop--;
		if(loop < 1)
		{
			printk("Path Configuration Error : Device is Busy(2) ... \n");
			return (VIOC_DEVICE_BUSY);
		}
		
		if(nStatus == VIOC_PATH_DISCONNECTED)
		{
			printk("Path Configuration Done ... \n");
			break;
		}
	}

	return (VIOC_DEVICE_CONNECTED);
}

void VIOC_CONFIG_RDMA12PathCtrl(unsigned int Path)
{
	/* Path - 0:  RDMA12 PATH , 	1:  VIDEOIN2 PATH */
	volatile PVIOC_IREQ_CONFIG pRDMAPath = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_CONFIG);
	//pRDMAPath->RDMA12 = Path;
	BITCSET(pRDMAPath->uMISC.nREG, (0x1<<30), (Path<<30));
}

void VIOC_CONFIG_RDMA14PathCtrl(unsigned int Path)
{
	/* Path - 0:  RDMA14 PATH , 	1:  VIDEOIN3 PATH */
	volatile PVIOC_IREQ_CONFIG pRDMAPath = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_CONFIG);
	//pRDMAPath->RDMA14 = Path;
	BITCSET(pRDMAPath->uMISC.nREG, (0x1<<31), (Path<<31));
}

void VIOC_CONFIG_WMIXPath(unsigned int Path, unsigned int Mode)
{
	/* Mode - 0: BY-PSSS PATH , 	1:  WMIX PATH */
	volatile PVIOC_IREQ_CONFIG pWMIXPath = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_CONFIG);

	switch(Path) {
		case WMIX00:
			//pWMIXPath->WMIX0_0 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<16), (Mode<<16));
			break;
		case WMIX03:
			//pWMIXPath->WMIX0_1 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<17), (Mode<<17));
			break;
		case WMIX10:
			//pWMIXPath->WMIX1_0 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<18), (Mode<<18));
			break;
		case WMIX13:
			//pWMIXPath->WMIX1_1 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<19), (Mode<<19));
			break;
		case WMIX30:
			//pWMIXPath->WMIX3_0 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<22), (Mode<<22));
			break;
		case WMIX40:
			//pWMIXPath->WMIX4_0 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<24), (Mode<<24));
			break;
		case WMIX50:
			//pWMIXPath->WMIX5_0 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<26), (Mode<<26));
			break;
		case WMIX60:
			//pWMIXPath->WMIX6_0 = Mode;
			BITCSET(pWMIXPath->uMISC.nREG, (0x1<<28), (Mode<<28));
			break;
	}
}



