/* 
 * Copyright (C) 2008, 2009 Telechips, Inc.
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

typedef enum
{
	TCC_DRMINFO_DATA_SET,
	TCC_DRMINFO_DATA_GET,	
	TCC_DRMINFO_DATA_CLEAR,
	TCC_DRMINFO_DATASIZE_SET,
	TCC_DRMINFO_DATASIZE_GET,
	TCC_DRMINFO_UID_CHECK,
	TCC_DRMINFO_MAX
} TccDRMInfoOpEnumType;

typedef struct
{
	unsigned int	sizeUID;
	char *			pUID;
}TccUIDInfoType;

typedef struct
{
	unsigned int	sizeDC;
	char *			pDataDC;
}TccDRMInfoType;

