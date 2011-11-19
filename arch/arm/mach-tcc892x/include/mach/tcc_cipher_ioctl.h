/* linux/arch/arm/mach-tcc92xx/include/mach/tcc_cipher_ioctl.h
 *
 * Copyright (C) 2009, 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
 
#ifndef _TCC_CIPHER_IOCTL_H_
#define _TCC_CIPHER_IOCTL_H_

/* Control Register */
#define HwCHIPHER_CTRL_ClearTxFIFO			Hw15
#define HwCHIPHER_CTRL_ClearRxFIFO			Hw14
#define HwCHIPHER_CTRL_ClearBlkCount		Hw13
#define HwCHIPHER_CTRL_InitVectorLoad		Hw12
#define HwCHIPHER_CTRL_KeyDataLoad			Hw11
#define HwCHIPHER_CTRL_KeyLength(x)			((x)*Hw9)
#define HwCHIPHER_CTRL_KeyLength_Mask		(Hw10+Hw9)
#define HwCHIPHER_CTRL_keyLength_128		HwCHIPHER_CTRL_KeyLength(0)
#define HwCHIPHER_CTRL_KeyLength_192		HwCHIPHER_CTRL_KeyLength(1)
#define HwCHIPHER_CTRL_KeyLength_256		HwCHIPHER_CTRL_KeyLength(2)
#define HwCHIPHER_CTRL_DESMode(x)			((x)*Hw7)
#define HwCHIPHER_CTRL_DESMode_Mask			(Hw8+Hw7)
#define HwCHIPHER_CTRL_DESMode_SingleDES	HwCHIPHER_CTRL_DESMode(0)
#define HwCHIPHER_CTRL_DESMode_DoubleDES	HwCHIPHER_CTRL_DESMode(1)
#define HwCHIPHER_CTRL_DESMode_TripleDES2	HwCHIPHER_CTRL_DESMode(2)
#define HwCHIPHER_CTRL_DESMode_TripleDES3	HwCHIPHER_CTRL_DESMode(3)
#define HwCHIPHER_CTRL_OperationMode(x)		((x)*Hw4)
#define HwCHIPHER_CTRL_OperationMode_Mask	(Hw6+Hw5+Hw4)
#define HwCHIPHER_CTRL_OperationMode_ECB	HwCHIPHER_CTRL_OperationMode(0)
#define HwCHIPHER_CTRL_OperationMode_CBC	HwCHIPHER_CTRL_OperationMode(1)
#define HwCHIPHER_CTRL_OperationMode_CFB	HwCHIPHER_CTRL_OperationMode(2)
#define HwCHIPHER_CTRL_OperationMode_OFB	HwCHIPHER_CTRL_OperationMode(3)
#define HwCHIPHER_CTRL_OperationMode_CTR	HwCHIPHER_CTRL_OperationMode(4)
#define HwCHIPHER_CTRL_ParityBit			Hw3
#define HwCHIPHER_CTRL_Encrytion			Hw2
#define HwCHIPHER_CTRL_Algorithm(x)			((x)*Hw0)
#define HwCHIPHER_CTRL_Algorithm_Mask		(Hw1+Hw0)
#define HwCHIPHER_CTRL_Algorithm_AES		HwCHIPHER_CTRL_Algorithm(0)
#define HwCHIPHER_CTRL_Algorithm_DES		HwCHIPHER_CTRL_Algorithm(1)
#define HwCHIPHER_CTRL_Algorithm_MULTI2		HwCHIPHER_CTRL_Algorithm(2)

/* DMA Control Register */
#define HwCHIPHER_DMACTR_RequestEnable		Hw31
#define HwCHIPHER_DMACTR_ByteEndian			Hw28
#define HwCHIPHER_DMACTR_AddrModeTx(x)		((x)*Hw16)
#define HwCHIPHER_DMACTR_AddrModeTx_Mask	(Hw17+Hw16)
#define HwCHIPHER_DMACTR_AddrModeTx_Multi	HwCHIPHER_DMACTR_AddrModeTx(0)
#define HwCHIPHER_DMACTR_AddrModeTx_Fixed	HwCHIPHER_DMACTR_AddrModeTx(1)
#define HwCHIPHER_DMACTR_AddrModeTx_Single	HwCHIPHER_DMACTR_AddrModeTx(2)
#define HwCHIPHER_DMACTR_AddrModeRx(x)		((x)*Hw14)
#define HwCHIPHER_DMACTR_AddrModeRx_Mask	(Hw15+Hw14)
#define HwCHIPHER_DMACTR_AddrModeRx_Multi	HwCHIPHER_DMACTR_AddrModeRx(0)
#define HwCHIPHER_DMACTR_AddrModeRx_Fixed	HwCHIPHER_DMACTR_AddrModeRx(1)
#define HwCHIPHER_DMACTR_AddrModeRx_Single	HwCHIPHER_DMACTR_AddrModeRx(2)
#define HwCHIPHER_DMACTR_ClearPacketCount	Hw2
#define HwCHIPHER_DMACTR_Enable				Hw0

/* IRQ Control Register */
#define HwCHIPHER_IRQCTR_DoneIrqStatus		Hw29
#define HwCHIPHER_IRQCTR_PacketIrqStatus	Hw28
#define HwCHIPHER_IRQCTR_SelectIrq			Hw20
#define HwCHIPHER_IRQCTR_EnableDoneIrq		Hw17
#define HwCHIPHER_IRQCTR_EnablePacketIrq	Hw16
#define HwCHIPHER_IRQCTR_PacketCountMask	(Hw13-Hw1)

/* The Key Length in AES */
enum
{
	TCC_CHIPHER_KEYLEN_128 = 0,
	TCC_CHIPHER_KEYLEN_192,
	TCC_CHIPHER_KEYLEN_256,
	TCC_CHIPHER_KEYLEN_256_1,
	TCC_CHIPHER_KEYLEN_MAX
};

/* The Mode in DES */
enum
{
	TCC_CHIPHER_DESMODE_SINGLE = 0,
	TCC_CHIPHER_DESMODE_DOUBLE,
	TCC_CHIPHER_DESMODE_TRIPLE2,
	TCC_CHIPHER_DESMODE_TRIPLE3,
	TCC_CHIPHER_DESMODE_MAX
};

/* The Operation Mode */ 
enum
{
	TCC_CHIPHER_OPMODE_ECB = 0,
	TCC_CHIPHER_OPMODE_CBC,
	TCC_CHIPHER_OPMODE_CFB,
	TCC_CHIPHER_OPMODE_OFB,
	TCC_CHIPHER_OPMODE_CTR,
	TCC_CHIPHER_OPMODE_CTR_1,
	TCC_CHIPHER_OPMODE_CTR_2,
	TCC_CHIPHER_OPMODE_CTR_3,
	TCC_CHIPHER_OPMODE_MAX
};

/* The Algorithm of the Encryption */
enum
{
	TCC_CHIPHER_ALGORITM_AES = 0,
	TCC_CHIPHER_ALGORITM_DES,
	TCC_CHIPHER_ALGORITM_MULTI2,
	TCC_CHIPHER_ALGORITM_MULTI2_1,
	TCC_CHIPHER_ALGORITM_MAX
};

/* The Base Address */ 
enum
{
	TCC_CHIPHER_BASEADDR_TX = 0,
	TCC_CHIPHER_BASEADDR_RX,
	TCC_CHIPHER_BASEADDR_MAX
};

/* The Packet Count */ 
enum
{
	TCC_CHIPHER_PACKETCOUNT_TX = 0,
	TCC_CHIPHER_PACKETCOUNT_RX,
	TCC_CHIPHER_PACKETCOUNT_MAX
};

/* The Packet Count */ 
enum
{
	TCC_CHIPHER_CLEARCOUNTER_TX 	= 0x01,
	TCC_CHIPHER_CLEARCOUNTER_RX		= 0x02,
	TCC_CHIPHER_CLEARCOUNTER_BLOCK	= 0x04,
	TCC_CHIPHER_CLEARCOUNTER_ALL	= 0x07,
	TCC_CHIPHER_CLEARCOUNTER_MAX
};

/* The Packet Count */ 
enum
{
	TCC_CHIPHER_DMA_ENDIAN_LITTLE = 0,
	TCC_CHIPHER_DMA_ENDIAN_BIG,
	TCC_CHIPHER_DMA_ENDIAN_MAX
};

/* The Packet Count */ 
enum
{
	TCC_CHIPHER_DMA_ADDRMODE_MULTI = 0,
	TCC_CHIPHER_DMA_ADDRMODE_FIXED,
	TCC_CHIPHER_DMA_ADDRMODE_SINGLE,
	TCC_CHIPHER_DMA_ADDRMODE_SINGLE_,
	TCC_CHIPHER_DMA_ADDRMODE_MAX
};

/* The Key Option for Multi2 */ 
enum
{
	TCC_CHIPHER_KEY_MULTI2_DATA = 0,
	TCC_CHIPHER_KEY_MULTI2_SYSTEM,
	TCC_CHIPHER_KEY_MULTI2_MAX
};

/* CIPHER IOCTL Command */
enum
{
	TCC_CHIPHER_IOCTL_SET_ALGORITHM = 0x100,
	TCC_CHIPHER_IOCTL_SET_KEY,
	TCC_CHIPHER_IOCTL_SET_VECTOR,
	TCC_CHIPHER_IOCTL_ENCRYPT,
	TCC_CHIPHER_IOCTL_DECRYPT,
	TCC_CHIPHER_IOCTL_MAX,
};

typedef struct
{
	unsigned uOperationMode;
	unsigned uAlgorithm;
	unsigned uArgument1;
	unsigned uArgument2;
} stCIPHER_ALGORITHM;

typedef struct
{
	unsigned char 	*pucData;
	unsigned 		uLength;
	unsigned 		uOption;
} stCIPHER_KEY;

typedef struct
{
	unsigned char 	*pucData;
	unsigned 		uLength;
} stCIPHER_VECTOR;

typedef struct
{
	unsigned char 	*pucSrcAddr;
	unsigned char 	*pucDstAddr;
	unsigned 		uLength;
} stCIPHER_ENCRYPTION;

typedef struct
{
	unsigned char 	*pucSrcAddr;
	unsigned char 	*pucDstAddr;
	unsigned 		uLength;
} stCIPHER_DECRYPTION;

#endif 
