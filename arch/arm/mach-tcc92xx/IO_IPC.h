/*===========================================================================

                       TCC Inter-Processor Communication

----------------------------------------------------------------------------
version     who      history
----------------------------------------------------------------------------
0908XX      Bruce    Created file.
----------------------------------------------------------------------------

===========================================================================*/
#ifndef __IO_IPC_H__
#define __IO_IPC_H__

/*===========================================================================

                         INCLUDE FILES FOR MODULE

===========================================================================*/



/*===========================================================================

                 DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/

//#define _IPC_MODEM_SIDE_

#ifdef _IPC_MODEM_SIDE_
#define _IPC_OS_UCOS2_
#else
//#define _IPC_OS_NUCLEUS_
#define _IPC_OS_LINUX_
#endif

/*---------------------------------------------------------------------------
 - Return Type
---------------------------------------------------------------------------*/

#define IPC_RET_SUCCESS                  0

#define IPC_RET_FAIL                     -21

#define IPC_RET_CH_USING                 -22

#define IPC_RET_RX_CBF_NOT_REGISTERED    -23

#define IPC_RET_NO_RX_DATA               -24

#define IPC_RET_NO_ACK                   -25

/*---------------------------------------------------------------------------
 - Communication Line Type
---------------------------------------------------------------------------*/

typedef struct
{
  unsigned char  cts;
  unsigned char  dsr;
#ifdef _IPC_MODEM_SIDE_
  unsigned char  ri;
  unsigned char  dcd;
#endif
} IPC_TX_LINE_TYPE;

typedef struct
{
  unsigned char  rts;
  unsigned char  dtr;
#ifndef _IPC_MODEM_SIDE_ // AP Side
  unsigned char  ri;
  unsigned char  dcd;
#endif
} IPC_RX_LINE_TYPE;

/*---------------------------------------------------------------------------
 - IPC RX Data CallBack Function Type
---------------------------------------------------------------------------*/

typedef void (*IO_IPC_RX_CBF_TYPE)(unsigned int ch, IPC_RX_LINE_TYPE* line, void* data, unsigned int len);

/*---------------------------------------------------------------------------
 - Assert Fail
---------------------------------------------------------------------------*/

#define IPC_ASSERT(e)  \
do{                      \
	if(!(e)) {           \
		while(1);        \
	}				     \
}while(0)

/*=========================================================================*/



/*===========================================================================

                             IPC API Functions

===========================================================================*/

extern void IO_IPC_Initialize(void);

extern void IO_IPC_Handshaking(void);

extern void IO_IPC_RegisterRxCbf(unsigned int ch, IO_IPC_RX_CBF_TYPE cbf);

extern void IO_IPC_RxNotify(void);

extern int IO_IPC_SetLine(unsigned int ch, IPC_TX_LINE_TYPE* line);

extern int IO_IPC_GetLine(unsigned int ch, IPC_TX_LINE_TYPE* tx_line, IPC_RX_LINE_TYPE* rx_line);

extern int IO_IPC_Write(unsigned int ch, IPC_TX_LINE_TYPE* line, void* data, unsigned int len);

extern int IO_IPC_WriteData(unsigned int ch, void* data, unsigned int len);

#if 1
extern int IO_IPC_Read_avail(unsigned int ch);

extern int IO_IPC_Write_avail(unsigned int ch);


#endif 

/*=========================================================================*/

#endif /* __IO_IPC_H__ */
