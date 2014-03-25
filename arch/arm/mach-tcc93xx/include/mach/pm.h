/*
 * arch/arm/mach-tcc9300/pm.h
 *
 * Author:  <linux@telechips.com>
 * Created: April 21, 2008
 * Description: LINUX POWER MANAGEMENT FUNCTIONS
 *
 * Copyright (C) 2008-2009 Telechips 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#ifndef __TCC_PM_H__
#define __TCC_PM_H__

/*===========================================================================

                                  MODE

===========================================================================*/

#if defined(CONFIG_SHUTDOWN_MODE)
#define TCC_PM_SHUTDOWN_MODE
#elif defined(CONFIG_SLEEP_MODE)
#define TCC_PM_SLEEP_MODE
#define TCC_PM_SLEEP_WFI_USED
#endif

//#define TCC_MEMBUS_PWR_CTRL_USED

/*===========================================================================

                            Shut-down/Sleep MAP

===========================================================================*/

#ifdef TCC_PM_SHUTDOWN_MODE
/*---------------------------------------------------------------------------
 1) Shut-down (shut down + backup ram boot + sdram self-refresh)

     << sram1 >>
     0xD0000000(0xEFE00000) ------------------
                           |  Enter Shutdown  | 0x500
                     0x500  ------------------
                           |      Stack       | 0x100
                     0x600  ------------------

     << backup ram >>
     0xD8000000(0xEFC00000) ------------------
                           |    Boot code     | 0x400
                     0x400  ------------------
                           |   I/O restore    | 0x200
                     0x600  ------------------
                           |   SDRAM Init     | 0x500
                     0xB00  ------------------
                           | GPIO Backup Data | 0x300
                     0xE00  ------------------
                           | CPU Reg/MMU Data | 0x100
                     0xF00  ------------------

---------------------------------------------------------------------------*/

// SRAM1
#define SHUTDOWN_FUNC_ADDR       0xEFE00000
#define SHUTDOWN_FUNC_SIZE       0x00000500
#define SHUTDOWN_FUNC_PHY        0xD0000000

// Backup RAM
#define BACKUP_RAM_BOOT_ADDR     0xEFC00000
#define BACKUP_RAM_BOOT_SIZE     0x00000400

#define IO_RESTORE_FUNC_ADDR     0xEFC00400
#define IO_RESTORE_FUNC_SIZE     0x00000200

#define SDRAM_INIT_FUNC_ADDR     0xEFC00600
#define SDRAM_INIT_FUNC_SIZE     0x00000500
#define SDRAM_INIT_FUNC_PHY      0xD8000600

#define GPIO_BUF_ADDR            0xEFC00B00
#define GPIO_BUF_PHY             0xD8000B00

#define REG_MMU_DATA_ADDR        0xEFC00E00
#define REG_MMU_DATA_PHY         0xD8000E00

/*-------------------------------------------------------------------------*/
#endif

#ifdef TCC_PM_SLEEP_MODE
/*---------------------------------------------------------------------------
 2) sleep (sleep + sdram self-refresh)

     << sram1 >>
     0xD0000000(0xEFE00000) ------------------
                           |   Enter Sleep    | 0x500
                     0x500  ------------------
                           |      Stack       | 0x100
                     0x600  ------------------
                           |   SDRAM Init     | 0x500
                     0xB00  ------------------

---------------------------------------------------------------------------*/

// SRAM1
#define SLEEP_FUNC_ADDR          0xEFE00000
#define SLEEP_FUNC_SIZE          0x00000500

#define SDRAM_INIT_FUNC_ADDR     0xEFE00600
#define SDRAM_INIT_FUNC_SIZE     0x00000500

/*-------------------------------------------------------------------------*/
#endif

#ifndef hword_of
#define	hword_of(X)					( *(volatile unsigned short *)((X)) )
#endif

#ifndef byte_of
#define	byte_of(X)					( *(volatile unsigned char *)((X)) )
#endif
#ifndef word_of
#define	word_of(X)					( *(volatile unsigned int *)((X)) )
#endif


#ifdef TCC_PM_SHUTDOWN_MODE
/*===========================================================================

                        Shut-down Backup Registers

===========================================================================*/

typedef struct _TCC_REG_{
	CKC ckc;
	PIC pic;
	VIC vic;
	TIMER timer;
	PMU pmu;
	SMUCONFIG smuconfig;

	//GPIO gpio;

	I2CMASTER i2c0_0;
	I2CMASTER i2c0_1;
	I2CSLAVE	i2c0_slave;
	//I2CSTATUS i2c0_status;
	
	I2CMASTER i2c1_0;
	I2CMASTER i2c1_1;
	I2CSLAVE	i2c1_slave;
	//I2CSTATUS i2c1_status;
	
	I2CMASTER i2c2_0;
	I2CMASTER i2c2_1;
	I2CSLAVE	i2c2_slave;
	//I2CSTATUS i2c2_status;
	
	SMUI2CMASTER smui2c;
	SMUI2CMASTER smui2c_ch1;
	SMUI2CICLK smui2cclk;
	
    ADMA adma0;
    ADMADAI adma0_dai;
    ADMASPDIFTX adma0_spdiftx;
    
	char		UART0[sizeof(UART)];
	UARTPORTMUX uartportmux;
	TSADC tsadc;
	IOBUSCFG iobuscfg;
//	HSIOBUSCFG hsiobuscfg;
	NFC nfc;

	volatile unsigned int backup_peri_iobus0;
	volatile unsigned int backup_peri_iobus1;
} TCC_REG, *PTCC_REG;
#endif /* TCC_PM_SHUTDOWN_MODE */

#endif  /*__TCC_PM_H__*/
