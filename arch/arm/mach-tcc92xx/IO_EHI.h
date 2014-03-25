/*===========================================================================

                        TCC EXTERNAL HOST INTERFACE

----------------------------------------------------------------------------
version     who      history
----------------------------------------------------------------------------
0908XX      Bruce    Created file.
----------------------------------------------------------------------------

===========================================================================*/
#ifndef __IO_EHI_H__
#define __IO_EHI_H__

/*===========================================================================

                         INCLUDE FILES FOR MODULE

===========================================================================*/



/*===========================================================================

                 DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/
//#define _EHI_HOST_SIDE_  

/*---------------------------------------------------------------------------
 - Return Type
---------------------------------------------------------------------------*/

#define EHI_RET_SUCCESS           0

#define EHI_RET_FAIL             -1

#define EHI_RET_FIFO_NOT_READY   -2

#define EHI_RET_WFIFO_BUSY       -3

#define EHI_RET_SEMA_OBTAIN_FAIL -4

/*===========================================================================

                        TCC EHI Interrupt Function

===========================================================================*/

extern int IO_EHI_CheckInterrupt(void);

extern void IO_EHI_SetInterrupt(unsigned char src);

extern void IO_EHI_ClearInterrupt(void);

extern unsigned char IO_EHI_GetInterrupt(void);

/*===========================================================================

                      TCC EHI H/W Semaphore Function

===========================================================================*/

extern int IO_EHI_ObtainSemaphore(void);

extern void IO_EHI_ReleaseSemaphore(void);

/*===========================================================================

                      TCC Memory Read/Write Function

===========================================================================*/

extern int IO_EHI_Read(unsigned int addr, void* data, unsigned int len);

extern int IO_EHI_Write(unsigned int addr, void* data, unsigned int len);

extern int IO_EHI_ReadAlignedWord(unsigned int addr, unsigned int* data);

extern int IO_EHI_WriteAlignedWord(unsigned int addr, unsigned int data);

/*===========================================================================

                      TCC Memory Read/Write Function

===========================================================================*/

extern void IO_EHI_Initialize(void);

/*=========================================================================*/

#endif /* __IO_EHI_H__ */
