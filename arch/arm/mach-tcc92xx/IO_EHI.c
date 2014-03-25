/*===========================================================================

                        TCC EXTERNAL HOST INTERFACE

----------------------------------------------------------------------------
version     who      history
----------------------------------------------------------------------------
0908XX      Bruce    Created file.
----------------------------------------------------------------------------

===========================================================================*/
#ifndef __IO_EHI_C__
#define __IO_EHI_C__

/*===========================================================================

                         INCLUDE FILES FOR MODULE

===========================================================================*/
#if defined(CONFIG_ARCH_TCC92XX)

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <mach/bsp.h>
#endif

#include "IO_EHI.h"
#ifndef _EHI_HOST_SIDE_
#if !defined(CONFIG_ARCH_TCC92XX)
#include "IO_TCCXXX.h"
#endif
#endif


/*===========================================================================

                 DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/

#define TCC92XX

/*===========================================================================

                     TCC EHI Module Access Definition

===========================================================================*/

#ifdef _EHI_HOST_SIDE_
/*---------------------------------------------------------------------------
 - EBI Chip Select Address
---------------------------------------------------------------------------*/
#define EHI_HPCS_BASE_ADDR                0x40000000

/*---------------------------------------------------------------------------
 - EBI Address Bus Index
---------------------------------------------------------------------------*/
#define EHI_HPXA_ADDR_IDX                 0x00000080
#else
/*---------------------------------------------------------------------------
 - EHI Register Base Address
---------------------------------------------------------------------------*/
#if defined(TCC92XX)
#define EHI_REG_BASE_ADDR                 0xF0570000
#else
#error EHI Register Base Address not defined
#endif
#endif

/*---------------------------------------------------------------------------
 - EBI External Interrupt Detecting Method
---------------------------------------------------------------------------*/
#define EHI_EINT_DET_LOW

/*---------------------------------------------------------------------------
 - EHI Register offset
---------------------------------------------------------------------------*/
#define	EHI_ST_OFFSET                     0x00
#define	EHI_IINT_OFFSET                   0x04
#define	EHI_EINT_OFFSET                   0x08
#define	EHI_ADDR_OFFSET                   0x0C
#define	EHI_AM_OFFSET                     0x10
#define	EHI_DATA_OFFSET                   0x14
#define	EHI_SEM_OFFSET                    0x18
#define	EHI_CFG_OFFSET                    0x1C
#define	EHI_IND_OFFSET                    0x20
#define	EHI_RWCS_OFFSET                   0x24

/*---------------------------------------------------------------------------
 - EHI Register size
---------------------------------------------------------------------------*/
#define	EHI_ST_SIZE                       2
#define	EHI_IINT_SIZE                     2
#define	EHI_EINT_SIZE                     2
#define	EHI_ADDR_SIZE                     4
#define	EHI_AM_SIZE                       4
#define	EHI_DATA_SIZE                     4
#define	EHI_SEM_SIZE                      2
#define	EHI_CFG_SIZE                      2
#define	EHI_IND_SIZE                      2
#define	EHI_RWCS_SIZE                     2

/*---------------------------------------------------------------------------
 - EHI Register Field
---------------------------------------------------------------------------*/
#define EHI_ST_RDY                        0x00000080
#define EHI_ST_STATUS                     0x7F
#define EHI_ST_EHIBOOT_RDY                0x55
#define EHI_ST_WFIFO_BUSY                 0x00000800

#define	EHI_SEM_FLG_NOT                   0x00000000
#define	EHI_SEM_FLG_HOST                  0x00000001
#define	EHI_SEM_FLG_SLAVE                 0x00000002
#ifdef _EHI_HOST_SIDE_
#define EHI_SEM_TAKE_FLG                  EHI_SEM_FLG_HOST
#define EHI_SEM_TAKEN_FLG                 EHI_SEM_FLG_SLAVE
#else
#define EHI_SEM_TAKE_FLG                  EHI_SEM_FLG_SLAVE
#define EHI_SEM_TAKEN_FLG                 EHI_SEM_FLG_HOST
#endif
#define	EHI_SEM_FLG_MASK                  (EHI_SEM_FLG_HOST|EHI_SEM_FLG_SLAVE)

#define	EHI_RWCS_AI                       0x00000080
#define	EHI_RWCS_LOCK_ON                  0x00000040
#define	EHI_RWCS_LOCK_OFF                 (~0x00000040)
#define	EHI_RWCS_RW_WAHB                  0x00000010
#define	EHI_RWCS_RW_RAHB                  0x00000020
#define EHI_RWCS_FREE                     0x00000000

/*---------------------------------------------------------------------------
 - EHI Access Retry Count
---------------------------------------------------------------------------*/
#define EHI_ESCAPE_CNT                    1000

/*=========================================================================*/



/*===========================================================================

                     TCC EHI Register Access Function

===========================================================================*/

#ifdef _EHI_HOST_SIDE_
/*---------------------------------------------------------------------------
 - EHI FIFO size = 32 bit * 16 unit = 32 x 16(bus size)access = 64 byte
---------------------------------------------------------------------------*/
#define EHI_FIFO_SIZE                     64

/*---------------------------------------------------------------------------
 - EHI FIFO unit = 32 bit * 4 byte
---------------------------------------------------------------------------*/
#define EHI_FIFO_UNIT                     4

/*---------------------------------------------------------------------------
 - EHI Data bus size 
---------------------------------------------------------------------------*/
#define EHI_BUS_WIDTH                     2

/*---------------------------------------------------------------------------
 - EHI Slave Buf Address Align
---------------------------------------------------------------------------*/
#define EHI_SLAVE_ADDR_ALIGN              4

/*---------------------------------------------------------------------------
 - EHI Host Buf Address Align
---------------------------------------------------------------------------*/
#define EHI_HOST_ADDR_ALIGN               2

/*---------------------------------------------------------------------------
 - EHI Dest Address Align
---------------------------------------------------------------------------*/
#define EHI_ALIGN_BUF_SIZE                (64*1024)

/*---------------------------------------------------------------------------
 - EBI BUS Access by 16 bit
---------------------------------------------------------------------------*/
#define	EHI_HWORD_OF(X)                   ( *(volatile unsigned short *)((X)) )



/*===========================================================================
VARIABLE
===========================================================================*/
static unsigned int ehi_align_buf[EHI_ALIGN_BUF_SIZE/sizeof(unsigned int)];

/*===========================================================================
FUNCTION 
 - Input : addr = Address of EHI Register
           data = Data for EHI Register
           size = Register Size in byte.
===========================================================================*/
static void ehi_reg_write(unsigned int reg, unsigned int data, unsigned int size)
{
	unsigned int cnt = 0;

	do{
		EHI_HWORD_OF(EHI_HPCS_BASE_ADDR + EHI_HPXA_ADDR_IDX) = reg + cnt;
		EHI_HWORD_OF(EHI_HPCS_BASE_ADDR) = data >> (cnt * 8);
	}while((cnt += EHI_BUS_WIDTH) < size);
}

/*===========================================================================
FUNCTION 
 - Input  : addr = Address of EHI Register
            size = Register Size in byte.
 - Return : Register value
===========================================================================*/
static unsigned int ehi_reg_read(unsigned int reg, unsigned int size)
{
	unsigned int cnt = 0;
	unsigned int data = 0;

	do{
		EHI_HWORD_OF(EHI_HPCS_BASE_ADDR + EHI_HPXA_ADDR_IDX) = reg + cnt;
		data |= (EHI_HWORD_OF(EHI_HPCS_BASE_ADDR) << (cnt * 8));
	}while((cnt += EHI_BUS_WIDTH) < size);

	return data;
}

/*===========================================================================
 MACRO
===========================================================================*/
#define EHI_WRITE_EHST(X)   ehi_reg_write(EHI_ST_OFFSET,   X, EHI_ST_SIZE)
#define EHI_WRITE_EHIINT(X) ehi_reg_write(EHI_IINT_OFFSET, X, EHI_IINT_SIZE)
#define EHI_WRITE_EHEINT(X) ehi_reg_write(EHI_EINT_OFFSET, X, EHI_EINT_SIZE)
#define EHI_WRITE_EHA(X)    ehi_reg_write(EHI_ADDR_OFFSET, X, EHI_ADDR_SIZE)
#define EHI_WRITE_EHAM(X)   ehi_reg_write(EHI_AM_OFFSET,   X, EHI_AM_SIZE)
#define EHI_WRITE_EHD(X)    ehi_reg_write(EHI_DATA_OFFSET, X, EHI_DATA_SIZE)
#define EHI_WRITE_EHSEM(X)  ehi_reg_write(EHI_SEM_OFFSET,  X, EHI_SEM_SIZE)
#define EHI_WRITE_EHCFG(X)  ehi_reg_write(EHI_CFG_OFFSET,  X, EHI_CFG_SIZE)
#define EHI_WRITE_EHIND(X)  ehi_reg_write(EHI_IND_OFFSET,  X, EHI_IND_SIZE)
#define EHI_WRITE_EHRWCS(X) ehi_reg_write(EHI_RWCS_OFFSET, X, EHI_RWCS_SIZE)

#define EHI_READ_EHST()     EHI_HWORD_OF(EHI_HPCS_BASE_ADDR + EHI_HPXA_ADDR_IDX) //ehi_reg_read(EHI_ST_OFFSET,   EHI_ST_SIZE)
#define EHI_READ_EHIINT()   ehi_reg_read(EHI_IINT_OFFSET, EHI_IINT_SIZE)
#define EHI_READ_EHEINT()   ehi_reg_read(EHI_EINT_OFFSET, EHI_EINT_SIZE)
#define EHI_READ_EHA()      ehi_reg_read(EHI_ADDR_OFFSET, EHI_ADDR_SIZE)
#define EHI_READ_EHAM()     ehi_reg_read(EHI_AM_OFFSET,   EHI_AM_SIZE)
#define EHI_READ_EHD()      ehi_reg_read(EHI_DATA_OFFSET, EHI_DATA_SIZE)
#define EHI_READ_EHSEM()    ehi_reg_read(EHI_SEM_OFFSET,  EHI_SEM_SIZE)
#define EHI_READ_EHCFG()    ehi_reg_read(EHI_CFG_OFFSET,  EHI_CFG_SIZE)
#define EHI_READ_EHIND()    ehi_reg_read(EHI_IND_OFFSET,  EHI_IND_SIZE)
#define EHI_READ_EHRWCS()   ehi_reg_read(EHI_RWCS_OFFSET, EHI_RWCS_SIZE)

#else /* _EHI_HOST_SIDE_ */

#define EHI_WRITE_EHST(X)   *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_ST_OFFSET) = X
#define EHI_WRITE_EHIINT(X) *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_IINT_OFFSET) = X
#define EHI_WRITE_EHEINT(X) *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_EINT_OFFSET) = X
#define EHI_WRITE_EHA(X)    *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_ADDR_OFFSET) = X
#define EHI_WRITE_EHAM(X)   *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_AM_OFFSET) = X
#define EHI_WRITE_EHD(X)    *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_DATA_OFFSET) = X
#define EHI_WRITE_EHSEM(X)  *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_SEM_OFFSET) = X
#define EHI_WRITE_EHCFG(X)  *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_CFG_OFFSET) = X
#define EHI_WRITE_EHIND(X)  *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_IND_OFFSET) = X
#define EHI_WRITE_EHRWCS(X) *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_RWCS_OFFSET) = X

#define EHI_READ_EHST()     *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_ST_OFFSET)
#define EHI_READ_EHIINT()   *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_IINT_OFFSET)
#define EHI_READ_EHEINT()   *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_EINT_OFFSET)
#define EHI_READ_EHA()      *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_ADDR_OFFSET)
#define EHI_READ_EHAM()     *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_AM_OFFSET)
#define EHI_READ_EHD()      *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_DATA_OFFSET)
#define EHI_READ_EHSEM()    *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_SEM_OFFSET)
#define EHI_READ_EHCFG()    *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_CFG_OFFSET)
#define EHI_READ_EHIND()    *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_IND_OFFSET)
#define EHI_READ_EHRWCS()   *(volatile unsigned long *)(EHI_REG_BASE_ADDR+EHI_RWCS_OFFSET)

#endif /* _EHI_HOST_SIDE_ */
/*=========================================================================*/



/*===========================================================================

                        TCC EHI Interrupt Function

===========================================================================*/

#define EHI_INT_REQ                       0x00000001

#define EHI_SET_INT_SRC(x)                ((unsigned char)((x<<1)&0x000000FE))

#define EHI_GET_INT_SRC(x)                ((unsigned char)((x>>1)&0x0000007F))

/*===========================================================================
FUNCTION 
 - Description : Check EHI Interrupt
===========================================================================*/
int IO_EHI_CheckInterrupt(void)
{
	unsigned int  loop = EHI_ESCAPE_CNT;

	do{
#ifdef _EHI_HOST_SIDE_
		if(!(EHI_READ_EHIINT() & EHI_INT_REQ))
#else
#ifdef EHI_EINT_DET_LOW
		if(EHI_READ_EHEINT() & EHI_INT_REQ)
#else
		if(!(EHI_READ_EHEINT() & EHI_INT_REQ))
#endif
#endif
			return EHI_RET_SUCCESS;
	}while(loop--);

	return EHI_RET_FAIL;
}

#ifndef _IPC_DEBUG_MODE_
unsigned char tx_interrupt_log[5];
#endif

/*===========================================================================
FUNCTION 
 - Description : Generate EHI Interrupt
 - Input : Interrupt Source
===========================================================================*/
void IO_EHI_SetInterrupt(unsigned char src)
{
#ifndef _IPC_DEBUG_MODE_
	tx_interrupt_log[4] = tx_interrupt_log[3];
	tx_interrupt_log[3] = tx_interrupt_log[2];
	tx_interrupt_log[2] = tx_interrupt_log[1];
	tx_interrupt_log[1] = tx_interrupt_log[0];
	tx_interrupt_log[0] = src;
#endif

#ifdef _EHI_HOST_SIDE_
	EHI_WRITE_EHIINT(EHI_SET_INT_SRC(src) | EHI_INT_REQ);
#else
#ifdef EHI_EINT_DET_LOW
	EHI_WRITE_EHEINT(EHI_SET_INT_SRC(src));
#else
	EHI_WRITE_EHEINT(EHI_SET_INT_SRC(src) | EHI_INT_REQ);
#endif
#endif
}

/*===========================================================================
FUNCTION 
 - Description : Generate EHI Interrupt
 - Input : Interrupt Source
===========================================================================*/
void IO_EHI_ClearInterrupt(void)
{
#ifdef _EHI_HOST_SIDE_
	EHI_WRITE_EHIINT(0x0);
#else
#ifdef EHI_EINT_DET_LOW
	EHI_WRITE_EHEINT(EHI_INT_REQ);
#else
	EHI_WRITE_EHEINT(0x0);
#endif
#endif
}

#ifndef _IPC_DEBUG_MODE_
unsigned char rx_interrupt_log[5];
#endif

/*===========================================================================
FUNCTION 
 - Description : Read & Clear EHI Interrupt
 - Return : Interrupt Source
===========================================================================*/
unsigned char IO_EHI_GetInterrupt(void)
{
	unsigned char src = 0xFF;

#ifdef _EHI_HOST_SIDE_
	src = EHI_READ_EHEINT();
#ifdef EHI_EINT_DET_LOW
	EHI_WRITE_EHEINT(src | EHI_INT_REQ);
#else
	EHI_WRITE_EHEINT(src & 0xFE);
#endif
#else
	src = EHI_READ_EHIINT();
	EHI_WRITE_EHIINT(src & 0xFE);
#endif

#ifndef _IPC_DEBUG_MODE_
	rx_interrupt_log[4] = rx_interrupt_log[3];
	rx_interrupt_log[3] = rx_interrupt_log[2];
	rx_interrupt_log[2] = rx_interrupt_log[1];
	rx_interrupt_log[1] = rx_interrupt_log[0];
	rx_interrupt_log[0] = EHI_GET_INT_SRC(src);
#endif

	return EHI_GET_INT_SRC(src);
}
/*=========================================================================*/



/*===========================================================================

                      TCC EHI H/W Semaphore Function

===========================================================================*/

/*===========================================================================
FUNCTION 
 - Description : Obtain EHI Semaphore
 - Return : success : 1, fail : 0
===========================================================================*/
int IO_EHI_ObtainSemaphore(void)
{
	unsigned short stat;
	unsigned int loop = EHI_ESCAPE_CNT;

	do{
		stat = EHI_READ_EHSEM();
		stat &= EHI_SEM_FLG_MASK;
		
		if(stat == EHI_SEM_TAKE_FLG)
			return EHI_RET_SUCCESS;
		//if(stat == EHI_SEM_TAKEN_FLG)
		//	return EHI_RET_SEMA_OBTAIN_FAIL;
	}while(loop--);

	return EHI_RET_SEMA_OBTAIN_FAIL;
}

/*===========================================================================
FUNCTION 
 - Description : Release EHI Semaphore
===========================================================================*/
void IO_EHI_ReleaseSemaphore(void)
{
	EHI_WRITE_EHSEM(EHI_SEM_FLG_NOT);
}
/*=========================================================================*/



/*===========================================================================

                      TCC Memory Read/Write Function

===========================================================================*/

#ifdef _EHI_HOST_SIDE_
/*===========================================================================
FUNCTION 
===========================================================================*/
static int ehi_wait_fifo_ready(void)
{
	unsigned short data = 0xFFFF;
	unsigned int   loop = EHI_ESCAPE_CNT;

	do{
		data = EHI_READ_EHST();

		if(data & EHI_ST_RDY)
			return EHI_RET_SUCCESS;
	}while(loop--);

	return EHI_RET_FIFO_NOT_READY;
}

/*===========================================================================
MACRO
===========================================================================*/

#define EHI_WAIT_FIFO_READY() \
	if(ehi_wait_fifo_ready() != EHI_RET_SUCCESS)\
	{\
		EHI_WRITE_EHRWCS(EHI_RWCS_FREE);\
		return EHI_RET_FIFO_NOT_READY;\
	}

#define EHI_WAIT_FIFO_READY_EX(x, y, z) \
	z = 0;\
	while(ehi_wait_fifo_ready() != EHI_RET_SUCCESS)\
	{\
		EHI_WRITE_EHRWCS(EHI_RWCS_FREE);\
		if(z++ > EHI_ESCAPE_CNT)\
			return EHI_RET_FIFO_NOT_READY;\
		EHI_WRITE_EHA(x);\
		EHI_WRITE_EHRWCS(y);\
		EHI_WRITE_EHRWCS(EHI_RWCS_AI | EHI_RWCS_RW_RAHB | y);\
		EHI_HWORD_OF(EHI_HPCS_BASE_ADDR+EHI_HPXA_ADDR_IDX) = EHI_DATA_OFFSET;\
	}

/*===========================================================================
FUNCTION 
===========================================================================*/
static int ehi_wait_wfifo_empty(void)
{
	unsigned short data = 0xFFFF;
	unsigned int   loop = EHI_ESCAPE_CNT;

	do{
		data = EHI_READ_EHST();

		if(!(data & EHI_ST_WFIFO_BUSY))
			return EHI_RET_SUCCESS;
	}while(loop--);

	return EHI_RET_WFIFO_BUSY;
}

/*===========================================================================
MACRO
===========================================================================*/

#define EHI_WAIT_EHI_WFIFO_EMPTY() \
	if(ehi_wait_wfifo_empty() != EHI_RET_SUCCESS)\
	{\
		return EHI_RET_WFIFO_BUSY;\
	}

/*===========================================================================
FUNCTION 
 - Description : Read data from Memory of TCC
 - Input : addr = Address of TCC memory ** Must be aligned 4Byte.
           data = Pointer to store read data
===========================================================================*/
static int ehi_read_word(unsigned int addr, unsigned int* data)
{
	unsigned short tmp;

	EHI_WAIT_EHI_WFIFO_EMPTY();
	EHI_WRITE_EHA(addr);
	EHI_WRITE_EHRWCS(1); // BSIZE = 1
	EHI_WRITE_EHRWCS(EHI_RWCS_AI|EHI_RWCS_RW_RAHB|1);
	EHI_HWORD_OF(EHI_HPCS_BASE_ADDR+EHI_HPXA_ADDR_IDX) = EHI_DATA_OFFSET; // Index Reg <= Data Reg's index

	EHI_WAIT_FIFO_READY();

	*data = EHI_HWORD_OF(EHI_HPCS_BASE_ADDR);
	*data += (EHI_HWORD_OF(EHI_HPCS_BASE_ADDR)<<16) & 0xFFFF0000 ;

	tmp = EHI_HWORD_OF(EHI_HPCS_BASE_ADDR);
	tmp = EHI_HWORD_OF(EHI_HPCS_BASE_ADDR);

	EHI_WRITE_EHRWCS(EHI_RWCS_FREE);

	return EHI_RET_SUCCESS;
}

/*===========================================================================
FUNCTION 
 - Description : Write data to Memory of TCC
 - Input : addr    = Address of TCC memory ** Must be aligned 4Byte.
           uParam1 = Data to be written (4Byte)
===========================================================================*/
static int ehi_write_word(unsigned int addr, unsigned int data)
{
	EHI_WAIT_EHI_WFIFO_EMPTY();
	EHI_WRITE_EHA(addr);
	EHI_WRITE_EHRWCS(EHI_RWCS_FREE);
	EHI_WRITE_EHRWCS(EHI_RWCS_AI|EHI_RWCS_RW_WAHB);
	EHI_HWORD_OF(EHI_HPCS_BASE_ADDR+EHI_HPXA_ADDR_IDX) = EHI_DATA_OFFSET; // Index Reg <= Data Reg's index

	EHI_HWORD_OF(EHI_HPCS_BASE_ADDR) = (unsigned short)(data & 0xFFFF);
	EHI_HWORD_OF(EHI_HPCS_BASE_ADDR) = (unsigned short)(data>>16 & 0xFFFF);

	EHI_WAIT_FIFO_READY();

	EHI_WRITE_EHRWCS(EHI_RWCS_FREE);

	return EHI_RET_SUCCESS;
}

/*===========================================================================
FUNCTION
 - Description : Read data from Memory of TCC
 - Input : addr = Address of TCC memory. ** Must be aligned 4Byte.
           data = Pointer to store read data. ** Must be aligned 2Byte.
           len =  Data amount to be read (byte unit)
===========================================================================*/
static int ehi_read_aligned_data(unsigned int addr, void* data, unsigned int len)
{
	unsigned int    addr_ptr;
	unsigned short* data_ptr;
	unsigned int    cnt;
	unsigned int    i, j;
	unsigned int    size;
	unsigned int    retry_cnt;
	unsigned short  temp_buf[4];

	addr_ptr = addr;
	data_ptr = (unsigned short*)data;

	cnt = len/EHI_FIFO_SIZE;

	if(cnt)
	{
		EHI_WAIT_EHI_WFIFO_EMPTY();
		EHI_WRITE_EHA(addr_ptr);
		EHI_WRITE_EHRWCS(15); // BSIZE = MAX(64byte) : 15
		EHI_WRITE_EHRWCS(EHI_RWCS_AI | EHI_RWCS_RW_RAHB | 15);
		EHI_HWORD_OF(EHI_HPCS_BASE_ADDR+EHI_HPXA_ADDR_IDX) = EHI_DATA_OFFSET; // Index Reg <= Data Reg's index

		for(j=0;j<cnt;j++)
		{
			EHI_WAIT_FIFO_READY_EX(addr_ptr, 15, retry_cnt);

			for(i=0;i<EHI_FIFO_SIZE/EHI_FIFO_UNIT;i++)
			{
				*data_ptr++ = EHI_HWORD_OF(EHI_HPCS_BASE_ADDR);
				*data_ptr++ = EHI_HWORD_OF(EHI_HPCS_BASE_ADDR);
			}
		}

		EHI_WRITE_EHRWCS(EHI_RWCS_FREE);

		addr_ptr += (cnt*EHI_FIFO_SIZE);
	}

	cnt = len%EHI_FIFO_SIZE;

	if(cnt)
	{
		size = cnt/EHI_FIFO_UNIT;

		if(size>1)
		{
			EHI_WAIT_EHI_WFIFO_EMPTY();
			EHI_WRITE_EHA(addr_ptr);
			EHI_WRITE_EHRWCS(size-1);
			EHI_WRITE_EHRWCS(EHI_RWCS_AI | EHI_RWCS_RW_RAHB | size-1);
			EHI_HWORD_OF(EHI_HPCS_BASE_ADDR+EHI_HPXA_ADDR_IDX) = EHI_DATA_OFFSET; // Index Reg <= Data Reg's index

			EHI_WAIT_FIFO_READY_EX(addr_ptr, size-1, retry_cnt);

			for(i=0;i<size;i++)
			{
				*data_ptr++ = EHI_HWORD_OF(EHI_HPCS_BASE_ADDR);
				*data_ptr++ = EHI_HWORD_OF(EHI_HPCS_BASE_ADDR);
			}

			EHI_WRITE_EHRWCS(EHI_RWCS_FREE);
			
			addr_ptr += (size*EHI_FIFO_UNIT);
		}

		if(size == 1) size = EHI_FIFO_UNIT;
		else          size = 0;

		size += (cnt%EHI_FIFO_UNIT);

		if(size)
		{
			EHI_WAIT_EHI_WFIFO_EMPTY();
			EHI_WRITE_EHA(addr_ptr);
			EHI_WRITE_EHRWCS(1);
			EHI_WRITE_EHRWCS(EHI_RWCS_AI | EHI_RWCS_RW_RAHB | 1);
			EHI_HWORD_OF(EHI_HPCS_BASE_ADDR+EHI_HPXA_ADDR_IDX) = EHI_DATA_OFFSET; // Index Reg <= Data Reg's index

			EHI_WAIT_FIFO_READY_EX(addr_ptr, 1, retry_cnt);

			temp_buf[0] = EHI_HWORD_OF(EHI_HPCS_BASE_ADDR);
			temp_buf[1] = EHI_HWORD_OF(EHI_HPCS_BASE_ADDR);
			temp_buf[2] = EHI_HWORD_OF(EHI_HPCS_BASE_ADDR);
			temp_buf[3] = EHI_HWORD_OF(EHI_HPCS_BASE_ADDR);

			memcpy(data_ptr, temp_buf, size);

			//EHI_WRITE_EHRWCS(EHI_RWCS_FREE);
		}
	}

	EHI_WRITE_EHRWCS(EHI_RWCS_FREE);

	return EHI_RET_SUCCESS;
}

/*===========================================================================
FUNCTION
 - Description : Write data to Memory of TCC
 - Input : addr = Address of TCC memory. ** Must be aligned 4Byte.
           data = Pointer to data to be written. ** Must be aligned 2Byte.
           len =  Data amount to Transfer (byte unit)
===========================================================================*/
static int ehi_write_aligned_data(unsigned int addr, void* data, unsigned int len)
{
	unsigned int    addr_ptr;
	unsigned short* data_ptr;
	unsigned int    cnt;
	unsigned int    i, j;
	unsigned int    size;
	unsigned int    merge_buf;
	int  ret;

	addr_ptr = addr;
	data_ptr = (unsigned short*)data;

	cnt = len/EHI_FIFO_SIZE;

	if(cnt)
	{
		EHI_WAIT_EHI_WFIFO_EMPTY();
		EHI_WRITE_EHA(addr_ptr);
		EHI_WRITE_EHRWCS(15); // BSIZE = MAX(64byte) : 15
		EHI_WRITE_EHRWCS(EHI_RWCS_AI | EHI_RWCS_RW_WAHB | 15);
		EHI_HWORD_OF(EHI_HPCS_BASE_ADDR+EHI_HPXA_ADDR_IDX) = EHI_DATA_OFFSET; // Index Reg <= Data Reg's index

		for(j=0;j<cnt;j++)
		{
			for(i=0;i<EHI_FIFO_SIZE/EHI_FIFO_UNIT;i++)
			{
				EHI_HWORD_OF(EHI_HPCS_BASE_ADDR) = *data_ptr++;
				EHI_HWORD_OF(EHI_HPCS_BASE_ADDR) = *data_ptr++;
			}

			EHI_WAIT_FIFO_READY();
		}

		EHI_WRITE_EHRWCS(EHI_RWCS_FREE);

		addr_ptr += (cnt*EHI_FIFO_SIZE);
	}

	cnt = len%EHI_FIFO_SIZE;
	
	if(cnt)
	{
		size = cnt/EHI_FIFO_UNIT;

		if(size)
		{
			EHI_WAIT_EHI_WFIFO_EMPTY();
			EHI_WRITE_EHA(addr_ptr);
			EHI_WRITE_EHRWCS((size-1)); // BSIZE = ((cnt-1)+1)*2*16 Byte
			EHI_WRITE_EHRWCS(EHI_RWCS_AI | EHI_RWCS_RW_WAHB | (size-1));
			EHI_HWORD_OF(EHI_HPCS_BASE_ADDR+EHI_HPXA_ADDR_IDX) = EHI_DATA_OFFSET;

			for(i=0;i<size;i++)
			{
				EHI_HWORD_OF(EHI_HPCS_BASE_ADDR) = *data_ptr++;
				EHI_HWORD_OF(EHI_HPCS_BASE_ADDR) = *data_ptr++;
			}

			EHI_WAIT_FIFO_READY();

			addr_ptr += (size*EHI_FIFO_UNIT);
		}

		size = cnt%EHI_FIFO_UNIT;

		if(size)
		{
			if(EHI_RET_SUCCESS != (ret = (ehi_read_word(addr_ptr, &merge_buf))))
				return ret;

			if(size == 1)
				merge_buf = (merge_buf&0xFFFFFF00) | ((*data_ptr)&0x000000FF);
			else if(size == 2)
				merge_buf = (merge_buf&0xFFFF0000) | ((*data_ptr)&0x0000FFFF);
			else //if(size == 3)
				merge_buf = (merge_buf&0xFF000000) | ((*data_ptr)&0x0000FFFF) | (((*(data_ptr+1))<<16)&0x00FF0000);

			if(EHI_RET_SUCCESS != (ret = (ehi_write_word(addr_ptr, merge_buf))))
				return ret;
		}
	}

	EHI_WRITE_EHRWCS(EHI_RWCS_FREE);

	return EHI_RET_SUCCESS;
}
#endif /* _EHI_HOST_SIDE_ */

/*===========================================================================
FUNCTION
 - Description : Read data from Memory of TCC
 - Input : addr = Address of TCC memory.
           data = Pointer to store read data.
           len =  Data amount to be read (byte unit)
===========================================================================*/
int IO_EHI_Read(unsigned int addr, void* data, unsigned int len)
{
#ifdef _EHI_HOST_SIDE_
	int ret = EHI_RET_SUCCESS;
	unsigned char* curr_data  = data;
	unsigned int   curr_addr  = addr;
	unsigned int   remain_len = len;
	unsigned int   curr_len = 0;
	unsigned int   merge_buf;
	
	if(addr % EHI_SLAVE_ADDR_ALIGN) // unaligned addr
	{
		curr_len = EHI_SLAVE_ADDR_ALIGN - (addr % EHI_SLAVE_ADDR_ALIGN);
		
		if(EHI_RET_SUCCESS != (ret = (ehi_read_word((addr/EHI_SLAVE_ADDR_ALIGN)*EHI_SLAVE_ADDR_ALIGN, &merge_buf))))
			return ret;

		if(curr_len == 1)
			*curr_data = (unsigned char)((merge_buf>>24)&0xFF);
		else if(curr_len == 2)
		{
			*curr_data     = (unsigned char)((merge_buf>>16)&0xFF);
			*(curr_data+1) = (unsigned char)((merge_buf>>24)&0xFF);
		}
		else //if(curr_len == 3)
		{
			*curr_data     = (unsigned char)((merge_buf>>8)&0xFF);
			*(curr_data+1) = (unsigned char)((merge_buf>>16)&0xFF);
			*(curr_data+2) = (unsigned char)((merge_buf>>24)&0xFF);
		}

		curr_addr  += curr_len;
		curr_data  += curr_len;
		remain_len -= curr_len;
	}

	if((unsigned int)curr_data % EHI_HOST_ADDR_ALIGN) // unaligned data
	{
		while(remain_len && (ret == EHI_RET_SUCCESS))
		{
			if(remain_len < EHI_ALIGN_BUF_SIZE)
				curr_len = remain_len;
			else
				curr_len = EHI_ALIGN_BUF_SIZE;

			ret = ehi_read_aligned_data(curr_addr, (void*)ehi_align_buf, curr_len);

			memcpy((void*)curr_data, (void*)ehi_align_buf, curr_len);

			curr_addr  += curr_len;
			curr_data  += curr_len;
			remain_len -= curr_len;
		}
	}
	else
	{
		ret = ehi_read_aligned_data(curr_addr, (void*)curr_data, remain_len);
	}

	return ret;

#else /* _EHI_HOST_SIDE_ */
	memcpy((void*)data, (void*)addr, len);
	return EHI_RET_SUCCESS;
#endif /* _EHI_HOST_SIDE_ */
}

/*===========================================================================
FUNCTION
 - Description : Write data to Memory of TCC
 - Input : addr = Address of TCC memory.
           data = Pointer to data to be written.
           len =  Data amount to Transfer (byte unit)
===========================================================================*/
int IO_EHI_Write(unsigned int addr, void* data, unsigned int len)
{
#ifdef _EHI_HOST_SIDE_
	int ret = EHI_RET_SUCCESS;
	unsigned char* curr_data  = data;
	unsigned int   curr_addr  = addr;
	unsigned int   remain_len = len;
	unsigned int   curr_len = 0;
	unsigned int   merge_buf;
	
	if(addr % EHI_SLAVE_ADDR_ALIGN) // unaligned addr
	{
		curr_len = EHI_SLAVE_ADDR_ALIGN - (addr % EHI_SLAVE_ADDR_ALIGN);
		
		if(EHI_RET_SUCCESS != (ret = (ehi_read_word((addr/EHI_SLAVE_ADDR_ALIGN)*EHI_SLAVE_ADDR_ALIGN, &merge_buf))))
			return ret;

		if(curr_len == 1)
			merge_buf = (merge_buf&0x00FFFFFF) | (((*curr_data)<<24)&0xFF000000);
		else if(curr_len == 2)
			merge_buf = (merge_buf&0x0000FFFF) | (((*curr_data)<<16)&0x00FF0000) | (((*(curr_data+1))<<24)&0xFF000000);
		else //if(curr_len == 3)
			merge_buf = (merge_buf&0x000000FF) | (((*curr_data)<<8)&0x0000FF00) | (((*(curr_data+1))<<16)&0x00FF0000) | (((*(curr_data+2))<<24)&0xFF000000);
		
		if(EHI_RET_SUCCESS != (ret = (ehi_write_word((addr/EHI_SLAVE_ADDR_ALIGN)*EHI_SLAVE_ADDR_ALIGN, merge_buf))))
			return ret;

		curr_addr  += curr_len;
		curr_data  += curr_len;
		remain_len -= curr_len;
	}

	if((unsigned int)curr_data % EHI_HOST_ADDR_ALIGN) // unaligned data
	{
		while(remain_len && (ret == EHI_RET_SUCCESS))
		{
			if(remain_len < EHI_ALIGN_BUF_SIZE)
				curr_len = remain_len;
			else
				curr_len = EHI_ALIGN_BUF_SIZE;

			memcpy((void*)ehi_align_buf, (void*)curr_data, curr_len);

			ret = ehi_write_aligned_data(curr_addr, (void*)ehi_align_buf, curr_len);

			curr_addr  += curr_len;
			curr_data  += curr_len;
			remain_len -= curr_len;
		}
	}
	else
	{
		ret = ehi_write_aligned_data(curr_addr, (void*)curr_data, remain_len);
	}

	return ret;

#else /* _EHI_HOST_SIDE_ */
	memcpy((void*)addr, (void*)data, len);
	return EHI_RET_SUCCESS;
#endif /* _EHI_HOST_SIDE_ */
}

/*===========================================================================
FUNCTION
 - Description : Write data to Memory of TCC
 - Input : addr = Address of TCC memory.
           data = Pointer to data to be written.
           len =  Data amount to Transfer (byte unit)
===========================================================================*/
int IO_EHI_ReadAlignedWord(unsigned int addr, unsigned int* data)
{
#ifdef _EHI_HOST_SIDE_
	return ehi_read_word(addr, data);
#else
	*data = *(volatile unsigned int *)addr;
	return EHI_RET_SUCCESS;
#endif
}

/*===========================================================================
FUNCTION
 - Description : Write data to Memory of TCC
 - Input : addr = Address of TCC memory.
           data = Pointer to data to be written.
           len =  Data amount to Transfer (byte unit)
===========================================================================*/
int IO_EHI_WriteAlignedWord(unsigned int addr, unsigned int data)
{
#ifdef _EHI_HOST_SIDE_
	return ehi_write_word(addr, data);
#else
	*(volatile unsigned int *)addr = data;
	return EHI_RET_SUCCESS;
#endif
}
/*=========================================================================*/



/*===========================================================================

                          TCC EHI Initialize

===========================================================================*/

/*===========================================================================
FUNCTION 
-----------------------------------------------------------------------------
  Hw0 = 68000 (1), x86 (0) interface
  Hw2 = 8bit (1), 16bit (0) interface
  Hw3 = used as Ready signal (1), used as Interrupt signal (0)
  Hw4 = Active Low Ready signal (1), Active High Ready signal (0)
===========================================================================*/
void IO_EHI_Initialize(void)
{
	if (machine_is_tcc9200s()) {
		tca_ckc_setperi(PERI_EHI0,ENABLE,1000000, PCDIRECTPLL2);

		HwGPIOF->GPFN0 = 0x11111111;  // HPXD[7:0]
		HwGPIOF->GPFN1 = 0x11111111;  // HPXD[15:8]
		HwGPIOF->GPFN2 = 0x11011100;  // HPINT0, HPXA, -, HPCSN0, HPWRN, HPRDN, -, -
		HwGPIOF->GPFN3 = 0;
	}

#ifndef _EHI_HOST_SIDE_
	EHI_WRITE_EHCFG(0x4);
#ifdef EHI_EINT_DET_LOW
	EHI_WRITE_EHEINT(EHI_INT_REQ);
#else
	EHI_WRITE_EHEINT(0x0);
#endif
#if defined(TCC92XX)
	HwPIC->SEL0	|= HwINT0_EHI0;
	HwPIC->IEN0	|= HwINT0_EHI0;
#endif
#endif
}
/*=========================================================================*/

#undef __IO_EHI_C__
#endif /* __IO_EHI_C__ */
