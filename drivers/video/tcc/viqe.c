#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <mach/vioc_viqe.h>
#include "viqe.h"
#include "tcc_vioc_viqe_interface.h"

viqe_queue_t		g_viqe_render_queue;

int
viqe_get_next_ptr (int cptr)
{
	int	nptr = cptr + 1;

	if ( nptr >= (VIQE_QUEUE_MAX_ENTRY * 2) ) nptr = 0;

	return (nptr);
}

int
viqe_get_index (int cindex)
{
	if ( cindex >= VIQE_QUEUE_MAX_ENTRY ) cindex -= VIQE_QUEUE_MAX_ENTRY;

	return (cindex);
}

int
viqe_queue_get_filled (viqe_queue_t *p_queue)
{
	int	w_ptr	= p_queue->w_ptr;
	int	r_ptr	= p_queue->r_ptr;

	if ( w_ptr >= r_ptr ) {
		return (w_ptr - r_ptr);
	} else {
		return (w_ptr + (VIQE_QUEUE_MAX_ENTRY*2 - r_ptr));
	}
}

int
viqe_queue_push_entry (viqe_queue_t *p_queue, viqe_queue_entry_t new_entry)
{
	int	new_idx;

	new_idx = viqe_get_index (p_queue->w_ptr);
	memcpy (&p_queue->entry[new_idx],&new_entry,sizeof(viqe_queue_entry_t));
	p_queue->w_ptr = viqe_get_next_ptr(p_queue->w_ptr);
}

viqe_queue_entry_t *
viqe_queue_show_entry (viqe_queue_t *p_queue)
{
	int	nRet;
	int	ridx;

	ridx = viqe_get_index (p_queue->r_ptr);
	return (&p_queue->entry[ridx]);
}

viqe_queue_entry_t *
viqe_queue_pop_entry (viqe_queue_t *p_queue)
{
	int	nRet;
	int	new_idx;

	new_idx = viqe_get_index (p_queue->r_ptr);
	//nRet = p_queue->entry[new_idx].data;
	p_queue->r_ptr = viqe_get_next_ptr(p_queue->r_ptr);
	return (&p_queue->entry[new_idx]);
}

int
viqe_queue_is_empty (viqe_queue_t *p_queue)
{
	return ( ( p_queue->r_ptr == p_queue->w_ptr ) );
}

int
viqe_queue_is_full (viqe_queue_t *p_queue)
{
	int	nRet;

	int	w_ptr = p_queue->w_ptr;
	int	r_ptr = p_queue->r_ptr;

	if ( w_ptr >= r_ptr ) {
		if ( (w_ptr - r_ptr) < VIQE_QUEUE_MAX_ENTRY ) {
			nRet = 0;
		} else {
			nRet = 1;
		}
	} else {
		if ( (w_ptr + (VIQE_QUEUE_MAX_ENTRY*2-r_ptr)) < VIQE_QUEUE_MAX_ENTRY ) {
			nRet = 0;
		} else {
			nRet = 1;
		}
	}

	return (nRet);
}

void
viqe_queue_show_entry_info (viqe_queue_entry_t *p_entry, char *prefix)
{
#if 0
	dbg_printf ("[%s] [0x%x,0x%x,0x%x], [BFIELD=%d], [TIMESTAMP=%d], [NEW:%d]"
			,prefix
			,p_entry->frame_base0
			,p_entry->frame_base1
			,p_entry->frame_base2
			,p_entry->bottom_field
			,p_entry->time_stamp
			,p_entry->new_field
		   );
#endif
}

#define	NORMAL_MODE		0		//normal mode
#define	DUPLI_MODE		1		//duplicate mode

void
viqe_swap_buffer (uint mode)
{
	uint tmp;
#if 0
	VIQE *pVIQE = (VIQE *)HwVIOC_VIQE0;
#if 1
	if (mode == DUPLI_MODE) {
		tmp	= pVIQE->cDEINTL_DMA.nDEINTL_BASE3.nREG;
		pVIQE->cDEINTL_DMA.nDEINTL_BASE3.nREG = pVIQE->cDEINTL_DMA.nDEINTL_BASE2.nREG;
		pVIQE->cDEINTL_DMA.nDEINTL_BASE2.nREG = pVIQE->cDEINTL_DMA.nDEINTL_BASE1.nREG;
		pVIQE->cDEINTL_DMA.nDEINTL_BASE1.nREG = pVIQE->cDEINTL_DMA.nDEINTL_BASE0.nREG;
		pVIQE->cDEINTL_DMA.nDEINTL_BASE0.nREG = tmp;
	}
#else
#endif
#else
	if (mode == DUPLI_MODE) {
		TCC_VIQE_DI_Swap60Hz();
	}

#endif
}

void
//viqe_render_init (void * p_rdma_ctrl)
viqe_render_init (void)
{
	memset (&g_viqe_render_queue, 0, sizeof(viqe_queue_t));
//	g_viqe_render_queue.p_rdma_ctrl = p_rdma_ctrl;
}

void
viqe_render_frame (unsigned int fbase0, unsigned int fbase1, unsigned int fbase2, int bottom_first, unsigned int field_interval, int curTime,
							int Frame_width, int Frame_height,
							int crop_top, int crop_bottom, int crop_left, int crop_right, 
							int Image_width, int Image_height, 
							int offset_x, int offset_y, int frameInfo_interlace)
{
	unsigned int		curr_time;
	viqe_queue_entry_t	p_entry;

//	printk ("render_frame ...");
	if ( viqe_queue_is_full (&g_viqe_render_queue) ) {
//		printk ("[REND] queue is full ...\n");
		return;
	}
	curr_time = curTime;
	p_entry.frame_base0 	= fbase0;
	p_entry.frame_base1 	= fbase1;
	p_entry.frame_base2 	= fbase2;
	p_entry.bottom_field 	= (bottom_first) ? 1 : 0;
	p_entry.time_stamp 		= curr_time + 0;
	p_entry.new_field		= 1;
	p_entry.Frame_width      = Frame_width;
	p_entry.Frame_height     = Frame_height;
	p_entry.crop_top 		= crop_top;
	p_entry.crop_bottom	= crop_bottom;
	p_entry.crop_left		= crop_left;
	p_entry.crop_right		= crop_right;
	p_entry.Image_width	= Image_width;
	p_entry.Image_height	= Image_height;
	p_entry.offset_x		= offset_x;
	p_entry.offset_y		= offset_y;
	p_entry.frameInfo_interlace	= frameInfo_interlace;
	viqe_queue_push_entry (&g_viqe_render_queue, p_entry);		// insert first field

	p_entry.bottom_field 	= (bottom_first) ? 0 : 1;
	p_entry.time_stamp 		= curr_time + field_interval;
	p_entry.new_field		= 1;
	viqe_queue_push_entry (&g_viqe_render_queue, p_entry);		// insert second field

	if ( !g_viqe_render_queue.ready ) {
		viqe_render_field (curr_time);
	}
}

void
viqe_render_field (int curTime)
{
	viqe_queue_entry_t *	p_entry;
	viqe_queue_entry_t *	p_entry_next;
	unsigned int			curr_time;
	unsigned int			time_stamp;
	int						next_frame_info;

//	printk ("@@render_field ...");
	curr_time = curTime;

	if ( viqe_queue_is_empty (&g_viqe_render_queue) ) {
		if ( !g_viqe_render_queue.ready ) {
			//printf ("[RDMA] empty ...\n");
			next_frame_info = VIQE_FIELD_SKIP;
		} else {
			p_entry = &g_viqe_render_queue.curr_entry;
			next_frame_info 	= VIQE_FIELD_DUP;
		}
	} else {
#if 0
		p_entry = &g_viqe_render_queue.curr_entry;
		next_frame_info = VIQE_FIELD_NEW;
		while (1) {
			if ( viqe_queue_is_empty (&g_viqe_render_queue) ) {
				break;
			}
			p_entry_next = viqe_queue_show_entry (&g_viqe_render_queue);
			if ( !g_viqe_render_queue.ready ) {
				g_viqe_render_queue.ready = 1;
				p_entry = p_entry_next;
				next_frame_info = VIQE_FIELD_NEW;
				viqe_queue_pop_entry (&g_viqe_render_queue);
				break;
			} else if ( (int)(p_entry_next->time_stamp - curr_time) <= 0 ) {
				if ( g_viqe_render_queue.curr_entry.bottom_field != p_entry_next->bottom_field) {
					p_entry = p_entry_next;
				}
				next_frame_info = VIQE_FIELD_NEW;
				viqe_queue_pop_entry (&g_viqe_render_queue);
			} else {
				break;
			}
		}
#else
		p_entry = &g_viqe_render_queue.curr_entry;
		next_frame_info = VIQE_FIELD_NEW;
		while (1) {
			p_entry_next = viqe_queue_show_entry (&g_viqe_render_queue);
			if ( viqe_queue_is_empty (&g_viqe_render_queue) ) {
				break;
			} else if ( !g_viqe_render_queue.ready ) {
				g_viqe_render_queue.ready = 1;
				p_entry = p_entry_next;
				next_frame_info = VIQE_FIELD_NEW;
				viqe_queue_pop_entry (&g_viqe_render_queue);
				break;
			} else {
				if ( g_viqe_render_queue.curr_entry.bottom_field != p_entry_next->bottom_field) {
					p_entry = p_entry_next;
				}
				next_frame_info = VIQE_FIELD_NEW;
				viqe_queue_pop_entry (&g_viqe_render_queue);
				if (( viqe_queue_get_filled (&g_viqe_render_queue)) >= 3 ) {
					continue;
				} else {
					break;
				}
			}
		}
#endif
		if ( p_entry->new_field == 0 )
			next_frame_info = VIQE_FIELD_DUP;
	}

	if ( next_frame_info == VIQE_FIELD_SKIP ) {
			// do nothing
//			printk ("[RDMA:SKIP]");
			return;
	}
	if ( g_viqe_render_queue.frame_cnt < 8 ) {
		next_frame_info = VIQE_FIELD_NEW;
	}
	switch (next_frame_info) {
		case VIQE_FIELD_SKIP :
			break;
		case VIQE_FIELD_NEW :
			viqe_queue_show_entry_info (p_entry, "RDMA:NEW");
			viqe_swap_buffer (0);
			break;
		case VIQE_FIELD_BROKEN :
			viqe_queue_show_entry_info (p_entry, "RDMA:BROKEN");
//			printk ("[ERROR] : BROKEN FIELD OCCURRED ....\n");
//			sim_fail ();
			break;
		case VIQE_FIELD_DUP :
			viqe_queue_show_entry_info (p_entry, "RDMA:DUP");
			viqe_swap_buffer (1);
			break;
	}

	if ( !g_viqe_render_queue.ready ) {
	} else {
		//viqe_queue_show_entry_info (p_entry, "RDMA:WRITE");
		if ( g_viqe_render_queue.frame_cnt < 8 ) {
			p_entry->bottom_field = g_viqe_render_queue.frame_cnt&0x1;
		}
		#if 0
		VIOC_RDMA_SetImageBase	 (g_viqe_render_queue.p_rdma_ctrl, p_entry->frame_base0, p_entry->frame_base1, p_entry->frame_base2);
		VIOC_RDMA_SetBFIELD   	 (g_viqe_render_queue.p_rdma_ctrl, p_entry->bottom_field);
		VIOC_RDMA_SetImageUpdate (g_viqe_render_queue.p_rdma_ctrl, 1);
		#else
		TCC_VIQE_DI_Run60Hz(1, p_entry->frame_base0, p_entry->frame_base1, p_entry->frame_base2,
							p_entry->Frame_width, p_entry->Frame_height,
							p_entry->crop_top,p_entry->crop_bottom, p_entry->crop_left, p_entry->crop_right, 
							p_entry->Image_width, p_entry->Image_height, 
							p_entry->offset_x, p_entry->offset_y, p_entry->bottom_field, p_entry->frameInfo_interlace);
		
		#endif
		g_viqe_render_queue.frame_cnt ++;
		if ( g_viqe_render_queue.frame_cnt >= 10 ) 
			g_viqe_render_queue.frame_cnt = 10;
	}

	p_entry->new_field = 0;
	memcpy (&g_viqe_render_queue.curr_entry,p_entry,sizeof(viqe_queue_entry_t));
}
