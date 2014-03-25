/*
 * linux/drivers/video/tcc_phisical_mem.H
 *
 * Author:  <linux@telechips.com>
 * Created: November 12, 2010
 * Description: TCC Physical Memory Allocation Driver
 *
 * Copyright (C) 20010-2011 Telechips 
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
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
 
#ifndef	_TCC_PHYSICAL_MEM_H_
#define	_TCC_PHYSICAL_MEM_H_  

enum
{
	MEM_TYPE_PMEM_SURF_BASE = 0,
	MEM_TYPE_PMEM_VIQE_BASE,
	MEM_TYPE_PMEM_CAM_BASE,
	MEM_TYPE_OVERLAY_MEM_BASE,
	MEM_TYPE_VIDEO_MEM_BASE,
	MEM_TYPE_COPYBIT_MEM_BASE,
	MEM_TYPE_FB_VIDEO_MEM_BASE,
	MEM_TYPE_FB_MEM_BASE,
	MEM_TYPE_STREAM_MEM_BASE
};

typedef struct
{
	unsigned int mem_addr;
	unsigned int mem_size;
 }stTCC_PHYSICAL_MEM_TYPE;

#endif //_TCC_PHYSICAL_MEM_H_
