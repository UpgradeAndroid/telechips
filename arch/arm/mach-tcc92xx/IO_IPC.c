/*===========================================================================

                       TCC Inter-Processor Communication

----------------------------------------------------------------------------
version     who      history
----------------------------------------------------------------------------
0908XX      Bruce    Created file.
----------------------------------------------------------------------------

===========================================================================*/
#ifndef __IO_IPC_C__
#define __IO_IPC_C__

/*===========================================================================

                         INCLUDE FILES FOR MODULE

===========================================================================*/

#include "IO_IPC.h"
#include "IO_EHI.h"
#ifdef _IPC_OS_NUCLEUS_
#include "Nucleus.h"
#endif
#ifdef _IPC_OS_UCOS2_
#include "Ucos_ii.h"
#endif

#if defined(_IPC_OS_LINUX_) 
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/delay.h>

#include <linux/dma-mapping.h>

#endif

/*===========================================================================

                        DEFINITIONS & DECLARATIONS

===========================================================================*/

/*===========================================================================

                             IPC Interrupt

===========================================================================*/

#define IPC_INT_READY  0x1
#define IPC_INT_SEND   0x2
#define IPC_INT_ACK    0x3
#define IPC_INT_ERR    0x4



/*===========================================================================

                        IPC Communication Buffer

-----------------------------------------------------------------------------
 - Designed according to Infineon OneDRAM Buffer Architecture
===========================================================================*/

/*---------------------------------------------------------------------------
 - IPC Communication Buffer Address
---------------------------------------------------------------------------*/

#define IPC_COMMBUF_ADDR     0x4C700000   //199MB

#ifdef	_IPC_OS_LINUX_
unsigned char *vIPC_COMMBUF_ADDR;	/* virtual pointer */
#endif

/*---------------------------------------------------------------------------
 - IPC Channel Definition
---------------------------------------------------------------------------*/

#define IPC_CH_NUM           64

/*---------------------------------------------------------------------------
 - IPC Data Field Size Definition
---------------------------------------------------------------------------*/

#define IPC_DATA_FIELD_SIZE  2040

/*---------------------------------------------------------------------------
 - Padding
---------------------------------------------------------------------------*/

#define DUMMY_FILL_1_BYTE unsigned char:0; unsigned char:8; unsigned char:0;
#define DUMMY_FILL_2_BYTE unsigned char:0; unsigned char:8; unsigned char:8; unsigned char:0;
#define DUMMY_FILL_3_BYTE unsigned char:0; unsigned char:8; unsigned char:8; unsigned char:8; unsigned char:0;
#define DUMMY_FILL_4_BYTE unsigned char:0; unsigned char:8; unsigned char:8; unsigned char:8; unsigned char:8; unsigned char:0;

/*---------------------------------------------------------------------------
 - IPC Channel Mask Register
---------------------------------------------------------------------------*/

typedef struct
{
#ifdef _IPC_MODEM_SIDE_
  unsigned int  cmr; // Channel Mask Receive
  unsigned int  cmt; // Channel Mask Transmit
#else
  unsigned int  cmt;
  unsigned int  cmr;
#endif
  DUMMY_FILL_4_BYTE
  DUMMY_FILL_4_BYTE
} IPC_CH_MASK_TYPE;

/*---------------------------------------------------------------------------
 - IPC TX Buffer
---------------------------------------------------------------------------*/

typedef struct
{
  unsigned short len;
  unsigned char  cts;
  unsigned char  dsr;
#ifdef _IPC_MODEM_SIDE_
  unsigned char  ri;
  unsigned char  dcd;
#else
  DUMMY_FILL_1_BYTE
  DUMMY_FILL_1_BYTE
#endif
  DUMMY_FILL_2_BYTE
} IPC_TX_HDR_TYPE;

typedef struct
{
  IPC_TX_HDR_TYPE hdr;
  unsigned char   data[IPC_DATA_FIELD_SIZE];
} IPC_TX_BUF_TYPE;

/*---------------------------------------------------------------------------
 - IPC RX Buffer
---------------------------------------------------------------------------*/

typedef struct
{
  unsigned short len;
  unsigned char  rts;
  unsigned char  dtr;
#ifndef _IPC_MODEM_SIDE_ // AP Side
  unsigned char  ri;
  unsigned char  dcd;
#else
  DUMMY_FILL_1_BYTE
  DUMMY_FILL_1_BYTE
#endif
  DUMMY_FILL_2_BYTE
} IPC_RX_HDR_TYPE;

typedef struct
{
  IPC_RX_HDR_TYPE hdr;
  unsigned char   data[IPC_DATA_FIELD_SIZE];
} IPC_RX_BUF_TYPE;

/*---------------------------------------------------------------------------
 - IPC Buffer on Physical Memory
---------------------------------------------------------------------------*/

typedef struct
{
  IPC_CH_MASK_TYPE cm;
#ifdef _IPC_MODEM_SIDE_
  IPC_RX_BUF_TYPE  rx[IPC_CH_NUM];
  IPC_TX_BUF_TYPE  tx[IPC_CH_NUM];
#else
  IPC_TX_BUF_TYPE  tx[IPC_CH_NUM];
  IPC_RX_BUF_TYPE  rx[IPC_CH_NUM];
#endif
} IPC_COMM_BUF_TYPE;



/*===========================================================================

                            IPC Manager Type

===========================================================================*/

typedef struct
{
  IO_IPC_RX_CBF_TYPE cbf;
  IPC_TX_HDR_TYPE    tx;
  IPC_RX_BUF_TYPE    rx;
} IPC_CH_INFO_TYPE;

typedef enum
{
  IPC_STAT_NOT_READY = 0,
  IPC_STAT_INIT,
  IPC_STAT_HANDSHAKE,
  IPC_STAT_IDLE,
  IPC_STAT_SEND,
  IPC_STAT_ACK,
  IPC_STAT_ERR
} IPC_STATUS_TYPE;

typedef struct
{
  IPC_CH_MASK_TYPE cm;
  IPC_CH_INFO_TYPE ch[IPC_CH_NUM];
  IPC_STATUS_TYPE  stat;
} IPC_MANAGER_TYPE;

/*=========================================================================*/



/*===========================================================================

                                VARIABLES

===========================================================================*/

/*===========================================================================
VARIABLE
 - Pointer of IPC_Communication_Buffer on Physical Memory.
===========================================================================*/

static IPC_COMM_BUF_TYPE* IPC_CommBuf;

/*===========================================================================
VARIABLE
 - for IPC Management
===========================================================================*/

static IPC_MANAGER_TYPE   IPC_Manager;

/*=========================================================================*/



/*===========================================================================

                             System Resource

===========================================================================*/

/*===========================================================================
VARIABLE
===========================================================================*/
#ifdef _IPC_OS_NUCLEUS_
static NU_SEMAPHORE IPC_CS_SEM;
static NU_SEMAPHORE IPC_WAIT_SEM;
#endif
#ifdef _IPC_OS_UCOS2_
static OS_EVENT *IPC_CS_SEM;
static OS_EVENT *IPC_WAIT_SEM;
#endif

#ifdef _IPC_OS_LINUX_
//DECLARE_MUTEX(IPC_CS_SEM);
//DECLARE_MUTEX_LOCKED(IPC_WAIT_SEM);
struct semaphore IPC_CS_SEM;
struct semaphore IPC_WAIT_SEM;
#endif
/*===========================================================================
FUNCTION
===========================================================================*/
static void IPC_InitSysRes(void)
{
#ifdef _IPC_OS_NUCLEUS_
	IPC_ASSERT(NU_SUCCESS == NU_Create_Semaphore(&IPC_CS_SEM, "IPCCSSEMA", 1, NU_FIFO));
	IPC_ASSERT(NU_SUCCESS == NU_Create_Semaphore(&IPC_WAIT_SEM, "IPCWSEMA", 0, NU_FIFO));
#endif
#ifdef _IPC_OS_UCOS2_
	IPC_CS_SEM   = OSSemCreate(1);
	IPC_WAIT_SEM = OSSemCreate(0);
#endif

#ifdef _IPC_OS_LINUX_
	init_MUTEX(&IPC_CS_SEM);
	init_MUTEX_LOCKED(&IPC_WAIT_SEM);
#endif
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void IPC_Sleep(unsigned int ms)
{
#ifdef _IPC_OS_NUCLEUS_
	NU_Sleep((ms+4)/5);
#endif
#ifdef _IPC_OS_UCOS2_
	OSTimeDly((ms+4)/5);
#endif
#ifdef _IPC_OS_LINUX_
	msleep(ms);
#endif
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void IPC_EnterCriticalSection(void)
{
#ifdef _IPC_OS_NUCLEUS_
	IPC_ASSERT(NU_SUCCESS == NU_Obtain_Semaphore(&IPC_CS_SEM, NU_SUSPEND));
#endif
#ifdef _IPC_OS_UCOS2_
	unsigned char err;

	OSSemPend(IPC_CS_SEM, 0, &err);
#endif

#ifdef _IPC_OS_LINUX_
	down(&IPC_CS_SEM);
#endif
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void IPC_ExitCriticalSection(void)
{
#ifdef _IPC_OS_NUCLEUS_
	IPC_ASSERT(NU_SUCCESS == NU_Release_Semaphore(&IPC_CS_SEM));
#endif
#ifdef _IPC_OS_UCOS2_
	OSSemPost(IPC_CS_SEM);
#endif

#ifdef _IPC_OS_LINUX_
	up(&IPC_CS_SEM);
#endif

}

/*===========================================================================
FUNCTION
===========================================================================*/
static void IPC_WaitForAck(void)
{
#ifdef _IPC_OS_NUCLEUS_
	IPC_ASSERT(NU_SUCCESS == NU_Obtain_Semaphore(&IPC_WAIT_SEM, NU_SUSPEND));
#endif
#ifdef _IPC_OS_UCOS2_
	unsigned char err;

	OSSemPend(IPC_WAIT_SEM, 0, &err);
#endif

#ifdef _IPC_OS_LINUX_
	down(&IPC_WAIT_SEM);
#endif
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void IPC_FreeWaitingAck(void)
{
#ifdef _IPC_OS_NUCLEUS_
	IPC_ASSERT(NU_SUCCESS == NU_Release_Semaphore(&IPC_WAIT_SEM));
#endif
#ifdef _IPC_OS_UCOS2_
	OSSemPost(IPC_WAIT_SEM);
#endif

#ifdef _IPC_OS_LINUX_
	up(&IPC_WAIT_SEM);
#endif
}
/*=========================================================================*/



/*===========================================================================

                               IPC APIs

===========================================================================*/

/*===========================================================================
FUNCTION
===========================================================================*/
void IO_IPC_Initialize(void)
{
	IO_EHI_Initialize();

	memset(&IPC_Manager, 0x0, sizeof(IPC_MANAGER_TYPE));

#ifdef _IPC_MODEM_SIDE_
	IPC_CommBuf = (IPC_COMM_BUF_TYPE*)IPC_COMMBUF_ADDR;
#else
#ifdef _IPC_OS_LINUX_

	vIPC_COMMBUF_ADDR = ioremap_nocache(IPC_COMMBUF_ADDR,0x100000);	

	IPC_CommBuf = (IPC_COMM_BUF_TYPE*)vIPC_COMMBUF_ADDR;			

#else
	IPC_CommBuf = (IPC_COMM_BUF_TYPE*)IPC_COMMBUF_ADDR;
#endif
	memset(IPC_CommBuf, 0x0, sizeof(IPC_COMM_BUF_TYPE));
#endif

	IPC_InitSysRes();

	IPC_Manager.stat = IPC_STAT_INIT;
}

/*===========================================================================
FUNCTION
===========================================================================*/
void IO_IPC_Handshaking(void)
{
IPC_EnterCriticalSection();

	switch(IPC_Manager.stat)
	{
		case IPC_STAT_INIT:
			while(IPC_Manager.stat == IPC_STAT_INIT)
			{
#ifndef _IPC_MODEM_SIDE_ // AP Side
				IO_EHI_SetInterrupt((unsigned char)IPC_INT_READY); // Generate Ready_Interrupt.;
#endif
				IPC_Sleep(10);
#ifndef _IPC_MODEM_SIDE_ // AP Side
				IO_EHI_ClearInterrupt();
#endif
				IPC_Sleep(1);
			}
		case IPC_STAT_HANDSHAKE:
			IPC_Manager.stat = IPC_STAT_IDLE;
			break;
		case IPC_STAT_NOT_READY:
			IPC_ASSERT(0);
			break;
		default:
			break;
	}

IPC_ExitCriticalSection();
}

/*===========================================================================
FUNCTION
===========================================================================*/
static int IPC_RxData(unsigned int ch)
{
	int ret = IPC_RET_SUCCESS;
	IPC_RX_LINE_TYPE line;

	do{
		if(EHI_RET_SUCCESS != (ret = IO_EHI_Read((unsigned int)&IPC_CommBuf->rx[ch].hdr, (void*)&IPC_Manager.ch[ch].rx.hdr, sizeof(IPC_TX_HDR_TYPE))))
			break;

		if(IPC_Manager.ch[ch].rx.hdr.len)
			if(EHI_RET_SUCCESS != (ret = IO_EHI_Read((unsigned int)IPC_CommBuf->rx[ch].data, (void*)IPC_Manager.ch[ch].rx.data, IPC_Manager.ch[ch].rx.hdr.len)))
				break;

		line.rts = IPC_Manager.ch[ch].rx.hdr.rts;
		line.dtr = IPC_Manager.ch[ch].rx.hdr.dtr;
#ifndef _IPC_MODEM_SIDE_ // AP Side
		line.ri  = IPC_Manager.ch[ch].rx.hdr.ri;
		line.dcd = IPC_Manager.ch[ch].rx.hdr.dcd;
#endif

		if(IPC_Manager.ch[ch].cbf) // Upload rx data to upper layer..
		{
			if(IPC_Manager.ch[ch].rx.hdr.len)
				IPC_Manager.ch[ch].cbf(ch, &line, (void*)IPC_Manager.ch[ch].rx.data, IPC_Manager.ch[ch].rx.hdr.len);
			else
				IPC_Manager.ch[ch].cbf(ch, &line, (void*)0, (unsigned int)0);
		}
		else
			ret = IPC_RET_RX_CBF_NOT_REGISTERED;

	}while(0);

	return ret;
}
void dummy_func()
{
	IPC_RxData(NULL);
}
/*===========================================================================
FUNCTION
===========================================================================*/
void IO_IPC_RegisterRxCbf(unsigned int ch, IO_IPC_RX_CBF_TYPE cbf)
{
	IPC_Manager.ch[ch].cbf = cbf;
}

/*===========================================================================
FUNCTION
===========================================================================*/
void IO_IPC_RxNotify(void)
{
	unsigned int ch;
	unsigned char int_src;

	int_src = IO_EHI_GetInterrupt();

	switch(int_src)
	{
		case IPC_INT_SEND:
		{
			IO_EHI_ReadAlignedWord((unsigned int)&IPC_CommBuf->cm.cmr, &IPC_Manager.cm.cmr);

			for(ch=0 ; ch<IPC_CH_NUM ; ch++){
				if(IPC_Manager.cm.cmr & (1<<ch)){
					if(IPC_RxData(ch) == IPC_RET_SUCCESS)
					{
						IPC_Manager.cm.cmr &= (~(1<<ch));
					}						
				}
			}

			IO_EHI_WriteAlignedWord((unsigned int)&IPC_CommBuf->cm.cmr, IPC_Manager.cm.cmr); // Unmask Channel_Mask.
			IO_EHI_SetInterrupt((unsigned char)IPC_INT_ACK); // Generate ACK_Interrupt.
		}
		break;

		case IPC_INT_ACK:
		{
			if(IPC_Manager.stat == IPC_STAT_SEND)
			{
				IPC_Manager.stat = IPC_STAT_ACK;
				IPC_FreeWaitingAck();
			}
		}
		break;

		case IPC_INT_ERR:
		{
			if(IPC_Manager.stat == IPC_STAT_SEND)
			{
				IPC_Manager.stat = IPC_STAT_ERR;
				IPC_FreeWaitingAck();
			}
		}
		break;

		case IPC_INT_READY:
		{
#ifdef _IPC_MODEM_SIDE_
			IO_EHI_SetInterrupt((unsigned char)IPC_INT_READY); // Generate Ready_Interrupt.;
#endif
			IPC_Manager.stat = IPC_STAT_HANDSHAKE;
		}
		break;
	}
}

/*===========================================================================
FUNCTION
===========================================================================*/
int IO_IPC_SetLine(unsigned int ch, IPC_TX_LINE_TYPE* line)
{
	return IO_IPC_Write(ch, line, (void*)0x0, (unsigned int)0x0);
}

/*===========================================================================
FUNCTION
===========================================================================*/
int IO_IPC_GetLine(unsigned int ch, IPC_TX_LINE_TYPE* tx_line, IPC_RX_LINE_TYPE* rx_line)
{
	tx_line->cts = IPC_Manager.ch[ch].tx.cts;
	tx_line->dsr = IPC_Manager.ch[ch].tx.dsr;
#ifdef _IPC_MODEM_SIDE_
	tx_line->ri  = IPC_Manager.ch[ch].tx.ri;
	tx_line->dcd = IPC_Manager.ch[ch].tx.dcd;
#endif

	rx_line->rts = IPC_Manager.ch[ch].rx.hdr.rts;
	rx_line->dtr = IPC_Manager.ch[ch].rx.hdr.dtr;
#ifndef _IPC_MODEM_SIDE_ // AP Side
	rx_line->ri  = IPC_Manager.ch[ch].rx.hdr.ri;
	rx_line->dcd = IPC_Manager.ch[ch].rx.hdr.dcd;
#endif

	return IPC_RET_SUCCESS;
}
#ifdef _IPC_OS_LINUX_
/*===========================================================================
FUNCTION
===========================================================================*/
//FIXME
int IO_IPC_Read_avail(unsigned int ch)
{
	return 0;   
}

/*===========================================================================
FUNCTION
===========================================================================*/
int IO_IPC_Write_avail(unsigned int ch)
{
	return IPC_DATA_FIELD_SIZE;
}
#endif
/*===========================================================================
FUNCTION
===========================================================================*/
int IO_IPC_Write(unsigned int ch, IPC_TX_LINE_TYPE* line, void* data, unsigned int len)
{
	int ret = IPC_RET_SUCCESS;

IPC_EnterCriticalSection();

	// Obtain EHI Semaphore
	while(IO_EHI_ObtainSemaphore() != EHI_RET_SUCCESS)
		IPC_Sleep(5);

	// Check other side interrupt
	while(IO_EHI_CheckInterrupt() != EHI_RET_SUCCESS);

	// Send Data
	do{
		if(EHI_RET_SUCCESS != (ret = IO_EHI_ReadAlignedWord((unsigned int)&IPC_CommBuf->cm.cmt, &IPC_Manager.cm.cmt))) // Read Channel_Mask.
			break;

		// This channel's buffer is using.
		if(IPC_Manager.cm.cmt & (1<<ch)){
			ret = IPC_RET_CH_USING;
			break;
		}

		if(line)
		{
			IPC_Manager.ch[ch].tx.cts = line->cts;
			IPC_Manager.ch[ch].tx.dsr = line->dsr;
#ifdef _IPC_MODEM_SIDE_
			IPC_Manager.ch[ch].tx.ri  = line->ri;
			IPC_Manager.ch[ch].tx.dcd = line->dcd;
#endif
		}

		if(data)
			IPC_Manager.ch[ch].tx.len = len;
		else
			IPC_Manager.ch[ch].tx.len = 0;

		// ------------------------------------------------------------------
		do{
			if(EHI_RET_SUCCESS != (ret = IO_EHI_Write((unsigned int)&IPC_CommBuf->tx[ch].hdr, (void*)&IPC_Manager.ch[ch].tx, sizeof(IPC_TX_HDR_TYPE))))
				break;

			if(data)
				if(EHI_RET_SUCCESS != (ret = IO_EHI_Write((unsigned int)IPC_CommBuf->tx[ch].data, (void*)data, len)))
					break;

			// Mask Channel_Mask.
			IPC_Manager.cm.cmt |= (1<<ch);
			IO_EHI_WriteAlignedWord((unsigned int)&IPC_CommBuf->cm.cmt, IPC_Manager.cm.cmt);

			// Generate Send_Interrupt.
			IPC_Manager.stat = IPC_STAT_SEND;
			IO_EHI_SetInterrupt((unsigned char)IPC_INT_SEND);

			IPC_WaitForAck();

			if(IPC_Manager.stat != IPC_STAT_ACK)
				ret = IPC_RET_NO_ACK;

			IPC_Manager.stat = IPC_STAT_IDLE;
		}while(0);
		// ------------------------------------------------------------------

	}while(0);
	// ----------------------------------------------------------------------

	// Release EHI Semaphore
	IO_EHI_ReleaseSemaphore();

IPC_ExitCriticalSection();

	return ret;
}

/*===========================================================================
FUNCTION
===========================================================================*/
int IO_IPC_WriteData(unsigned int ch, void* data, unsigned int len)
{
	return IO_IPC_Write(ch, (IPC_TX_LINE_TYPE*)0x0, data, len);
}
/*=========================================================================*/

#undef __IO_IPC_C__
#endif /* __IO_IPC_C__ */
