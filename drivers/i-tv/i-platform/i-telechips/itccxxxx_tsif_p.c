#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/poll.h>
#include <linux/spi/spi.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/dma.h>

#include <mach/bsp.h>
#include <linux/spi/tcc_tsif_module.h>
#include "../../../spi/tcc/tca_tsif_module_hwset.h"

#include <i-tv/itv_common.h>
#include "itccxxxx_tsif_p.h"

//20110321 koo : tsif/dma reg & input ts data debug option
//#define TSIF_DEBUG

#if defined(CONFIG_ARCH_TCC93XX)
static struct clk *gpsb_clk;
#endif

extern int tca_tsif_module_init(struct tcc_tsif_module_handle *h, volatile struct tcc_tsif_regs *regs, dma_alloc_f tea_dma_alloc, dma_free_f tea_dma_free, int dma_size, int tsif_ch, int dma_controller, int dma_ch, int port);
extern void tca_tsif_module_clean(struct tcc_tsif_module_handle *h);



typedef enum {
	P_TSIF_STATE_DEINIT, 
	P_TSIF_STATE_INIT, 
	P_TSIF_STATE_START, 
	P_TSIF_STATE_STOP 
} e_p_tsif_state;

typedef struct {
	e_p_tsif_state state;

	int mode;	// true : TSIF_Serial mode, false : TSIF_Parallel mode.
	struct tcc_tsif_module_handle handle;

	void *cb_data;
	void (*callback)(void *);
	
	#define PIDTABLE_MAXCNT		32
	
	unsigned short pidtable[PIDTABLE_MAXCNT];
} st_p_tsif;

st_p_tsif p_tsif_inst = {
	.state = P_TSIF_STATE_DEINIT, 
};

static void tsif_p_free_dma_buffer(struct tea_dma_buf *tdma)
{
	if(tdma) {
		if(tdma->v_addr)
			dma_free_writecombine(0, tdma->buf_size, tdma->v_addr, tdma->dma_addr);
		memset(tdma, 0, sizeof(struct tea_dma_buf));
	}
}

static int tsif_p_alloc_dma_buffer(struct tea_dma_buf *tdma, unsigned int size)
{
	int error = -1;

	if(tdma) {
		tsif_p_free_dma_buffer(tdma);
		tdma->buf_size = size;
		tdma->v_addr = dma_alloc_writecombine(0, tdma->buf_size, &tdma->dma_addr, GFP_KERNEL);
		printk("[%s] Alloc DMA buffer @0x%X(Phy=0x%X), size:%d\n", __func__, (unsigned int)tdma->v_addr,
				(unsigned int)tdma->dma_addr, tdma->buf_size);

		error = tdma->v_addr ? 0 : 1;
	}

	return error;
}

#ifdef TSIF_DEBUG
struct timeval time_;
unsigned int cur_time=0;
unsigned int base_time=0;
unsigned int debug_time=0;
unsigned int debug_cnt=0;
#define CHK_TIME			10	//sec
struct task_struct *debug_thread = NULL;
#endif

static irqreturn_t tsif_p_dma_handler(int irq, void *dev_id)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_module_handle *handle = (struct tcc_tsif_module_handle *)dev_id;

	handle->tsif_isr(handle);

#ifdef GDMA
	handle->cur_q_pos = (int)handle->dma_phy_curpos + (int)(handle->rx_dma.v_addr - handle->rx_dma.dma_addr);
#else
	handle->q_pos = (int)(handle->rx_dma.v_addr + (handle->cur_q_pos * MPEG_PACKET_SIZE));
#endif

#ifdef TSIF_DEBUG
	{
		int cnt;
		unsigned char* pos;
 
#ifdef GDMA
		if(handle->cur_q_pos > handle->prev_q_pos) {
			pos = (int)handle->prev_q_pos;
			cnt = handle->cur_q_pos - (int)handle->prev_q_pos;
			
			do_gettimeofday(&time_);
			cur_time = ((time_.tv_sec * 1000) & 0x00ffffff) + (time_.tv_usec / 1000);
			if((cur_time - base_time) > (CHK_TIME * 1000)) {
				printk(">> [0x%08x - 0x%08x / %d] : 0x%02x\n", handle->prev_q_pos, handle->cur_q_pos, cnt, *pos);
				if(*pos != 0x47) {
					if((pos - (int)handle->rx_dma.v_addr) >= 2) {
						printk("\t[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n", *(pos-2), *(pos-1), *pos, *(pos+1), *(pos+2));
					} else {
						unsigned char* pos_addr;
						int pos_size;

						pos_size = pos - (int)handle->rx_dma.v_addr;

						if(pos_size == 0) {
							pos_addr = (TSIF_DMA_SIZE + (int)handle->rx_dma.v_addr) - 2;
							printk("\t[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n", *(pos_addr), *(pos_addr+1), *pos, *(pos+1), *(pos+2));
						} else if(pos_size == 1) {
							pos_addr = (TSIF_DMA_SIZE + (int)handle->rx_dma.v_addr) - 1;
							printk("\t[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n", *(pos_addr), *(pos-1), *pos, *(pos+1), *(pos+2));
						}
					}
				}
				base_time = cur_time;
			}

			handle->prev_q_pos += (cnt / MPEG_PACKET_SIZE) * MPEG_PACKET_SIZE;
		} else {
			int res, ovr;
			
			pos = (int)handle->prev_q_pos;
			res = (TSIF_DMA_SIZE + (int)handle->rx_dma.v_addr) - (int)handle->prev_q_pos;
			ovr = handle->cur_q_pos - (int)handle->rx_dma.v_addr;
			cnt = res + ovr;
			
			do_gettimeofday(&time_);
			cur_time = ((time_.tv_sec * 1000) & 0x00ffffff) + (time_.tv_usec / 1000);
			if((cur_time - base_time) > (CHK_TIME * 1000)) {
				printk(">> [0x%08x - 0x%08x / %d] : 0x%02x\n", handle->prev_q_pos, handle->cur_q_pos, cnt, *pos);
				if(*pos != 0x47) {
					if((pos - (int)handle->rx_dma.v_addr) >= 2) {
						printk("\t[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n", *(pos-2), *(pos-1), *pos, *(pos+1), *(pos+2));
					} else {
						unsigned char* pos_addr;
						int pos_size;

						pos_size = pos - (int)handle->rx_dma.v_addr;

						if(pos_size == 0) {
							pos_addr = (TSIF_DMA_SIZE + (int)handle->rx_dma.v_addr) - 2;
							printk("\t[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n", *(pos_addr), *(pos_addr+1), *pos, *(pos+1), *(pos+2));
						} else if(pos_size == 1) {
							pos_addr = (TSIF_DMA_SIZE + (int)handle->rx_dma.v_addr) - 1;
							printk("\t[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n", *(pos_addr), *(pos-1), *pos, *(pos+1), *(pos+2));
						}
					}
				}
				
				base_time = cur_time;
			}

			handle->prev_q_pos = (int)handle->rx_dma.v_addr + (ovr - (cnt % MPEG_PACKET_SIZE));
		}
#else	//#ifdef GDMA
		if(handle->q_pos > handle->prev_q_pos) {
			pos = (int)handle->prev_q_pos;
			cnt = handle->q_pos - (int)handle->prev_q_pos;
		} else {
			pos = (int)handle->prev_q_pos;
			cnt = ((handle->dma_total_packet_cnt * MPEG_PACKET_SIZE) + (int)handle->rx_dma.v_addr) - (int)handle->prev_q_pos;
		}

		do_gettimeofday(&time_);
		cur_time = ((time_.tv_sec * 1000) & 0x00ffffff) + (time_.tv_usec / 1000);
		if((cur_time - base_time) > (CHK_TIME * 1000)) {
			printk(">> [0x%08x - 0x%08x / %d] : 0x%02x\n", handle->prev_q_pos, handle->q_pos, cnt, *pos);
			if(*pos != 0x47) {
				if((pos - (int)handle->rx_dma.v_addr) >= 2) {
					printk("\t[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n", *(pos-2), *(pos-1), *pos, *(pos+1), *(pos+2));
				} else {
					unsigned char* pos_addr;
					int pos_size;

					pos_size = pos - (int)handle->rx_dma.v_addr;

					if(pos_size == 0) {
						pos_addr = (TSIF_DMA_SIZE + (int)handle->rx_dma.v_addr) - 2;
						printk("\t[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n", *(pos_addr), *(pos_addr+1), *pos, *(pos+1), *(pos+2));
					} else if(pos_size == 1) {
						pos_addr = (TSIF_DMA_SIZE + (int)handle->rx_dma.v_addr) - 1;
						printk("\t[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]\n", *(pos_addr), *(pos-1), *pos, *(pos+1), *(pos+2));
					}
				}
			}
			
			base_time = cur_time;
		}

		handle->prev_q_pos = handle->q_pos;
#endif	//#ifdef GDMA
	}
#endif	//#ifdef TSIF_DEBUG

	if(instance->callback)
		instance->callback(instance->cb_data);

	return IRQ_HANDLED;
}

int tcc_tsif_p_get_pos(void)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_module_handle *handle = &instance->handle;

#ifdef GDMA
	return handle->cur_q_pos;
#else
	return handle->q_pos;
#endif
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_get_pos);

void tcc_tsif_p_insert_pid(int pid)
{
#ifdef GDMA
#ifdef TSIF_PIDMATCH_USE
	st_p_tsif *instance = &p_tsif_inst;
	
	int i, pos=0;

	for(i=0; i<PIDTABLE_MAXCNT; i++) {
		if(instance->pidtable[i] == pid) {
			printk("[%s] pid table already insert val : %d\n", __func__, pid);
			return;
		}
	}

	for(i=0; i<PIDTABLE_MAXCNT; i++) {
		if(instance->pidtable[i] == 0xffff) {
			pos = i;
			break;
		}
		
		if(i == (PIDTABLE_MAXCNT - 1)) {
			printk("[%s] pid table full : %d\n", __func__, (i + 1));
			return;
		}
	}

#if defined(CONFIG_ARCH_TCC88XX)
	PTSIF	tsif_regs;
	tsif_regs = (volatile PTSIF)tcc_p2v((TSIF_CH == 1) ? HwTSIF1_BASE : HwTSIF0_BASE);

	tsif_regs->TSPID[pos] = 0;
	tsif_regs->TSPID[pos] = (pid & 0x1FFF) | Hw13;
#endif

	instance->pidtable[pos] = pid;
	
	//printk("[%s] pidtable insert : %d-%d\n", __func__, pos, pid);
#endif
#else
	st_p_tsif *instance = &p_tsif_inst;
	
	int i, pos=0;

	for(i=0; i<PIDTABLE_MAXCNT; i++) {
		if(instance->pidtable[i] == pid) {
			printk("[%s] pid table already insert val : %d\n", __func__, pid);
			return;
		}
	}

	for(i=0; i<PIDTABLE_MAXCNT; i++) {
		if(instance->pidtable[i] == 0xffff) {
			pos = i;
			break;
		}
		
		if(i == (PIDTABLE_MAXCNT - 1)) {
			printk("[%s] pid table full : %d\n", __func__, (i + 1));
			return;
		}
	}

#if defined(CONFIG_ARCH_TCC93XX)
	volatile unsigned long* PIDT;

	PIDT = (volatile unsigned long *)tcc_p2v(HwGPSB_PIDT(pos));
	*PIDT = 0x0;
	*PIDT = (pid & 0x1FFF) | ((TSIF_CH == 1) ? Hw30 : Hw29);
#elif defined(CONFIG_ARCH_TCC88XX)
#ifdef TSIF_PIDMATCH_USE
	PTSIF	tsif_regs;
	tsif_regs = (volatile PTSIF)tcc_p2v((TSIF_CH == 1) ? HwTSIF1_BASE : HwTSIF0_BASE);

	tsif_regs->TSPID[pos] = 0;
	tsif_regs->TSPID[pos] = (pid & 0x1FFF) | Hw13;
#else
	HwGPSB_PIDT(pos) = 0;
	HwGPSB_PIDT(pos) = (pid & 0x1FFF) | ((TSIF_CH == 1) ? Hw30 : Hw29);
#endif
#endif	

	instance->pidtable[pos] = pid;
	
	//printk("[%s] pidtable insert : %d-%d\n", __func__, pos, pid);
#endif
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_insert_pid);

void tcc_tsif_p_remove_pid(int pid)
{
#ifdef GDMA	
#ifdef TSIF_PIDMATCH_USE
	st_p_tsif *instance = &p_tsif_inst;
	
	int i, pos=0;

	for(i=0; i<PIDTABLE_MAXCNT; i++) {
		if(instance->pidtable[i] == pid) {
			pos = i;
			break;
		}
		
		if(i == (PIDTABLE_MAXCNT - 1)) {
			printk("[%s] pid table not found : %d\n", __func__, pid);
			return;
		}
	}

#if defined(CONFIG_ARCH_TCC88XX)
	PTSIF	tsif_regs;
	tsif_regs = (volatile PTSIF)tcc_p2v((TSIF_CH == 1) ? HwTSIF1_BASE : HwTSIF0_BASE);

	tsif_regs->TSPID[i] = 0;
#endif

	instance->pidtable[pos] = 0xffff;
	
	//printk("[%s] pidtable remove : %d-%d\n", __func__, pos, pid);
#endif
#else
	st_p_tsif *instance = &p_tsif_inst;
	
	int i, pos=0;

	for(i=0; i<PIDTABLE_MAXCNT; i++) {
		if(instance->pidtable[i] == pid) {
			pos = i;
			break;
		}
		
		if(i == (PIDTABLE_MAXCNT - 1)) {
			printk("[%s] pid table not found : %d\n", __func__, pid);
			return;
		}
	}

#if defined(CONFIG_ARCH_TCC93XX)
	volatile unsigned long* PIDT;

	PIDT = (volatile unsigned long *)tcc_p2v(HwGPSB_PIDT(pos));
	*PIDT = 0x0;
#elif defined(CONFIG_ARCH_TCC88XX)
#ifdef TSIF_PIDMATCH_USE
	PTSIF	tsif_regs;
	tsif_regs = (volatile PTSIF)tcc_p2v((TSIF_CH == 1) ? HwTSIF1_BASE : HwTSIF0_BASE);

	tsif_regs->TSPID[i] = 0;
#else
	HwGPSB_PIDT(i) = 0;
#endif
#endif

	instance->pidtable[pos] = 0xffff;
	
	//printk("[%s] pidtable remove : %d-%d\n", __func__, pos, pid);
#endif
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_remove_pid);

void tcc_tsif_p_set_packetcnt(int cnt)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_module_handle *handle = &instance->handle;
	
	handle->dma_intr_packet_cnt = cnt;
	handle->dma_total_packet_cnt = ((TSIF_DMA_SIZE / MPEG_PACKET_SIZE) / cnt) * cnt;
	
	printk("[%s] packet count set : %d\n", __func__, cnt);	
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_set_packetcnt);

#ifdef TSIF_DEBUG
void tcc_tsif_p_debug(int mod)
{
#if defined(CONFIG_ARCH_TCC88XX) || defined(CONFIG_ARCH_TCC93XX)
	PGPION	gpio_regs;
	PTSIF	tsif_regs;
	volatile unsigned int* tsif_port_reg;

#ifdef GDMA
	PGDMACTRL	gdma_regs;
#else
	PGPSB	gpsb_regs;
#endif

#if defined(CONFIG_ARCH_TCC93XX)
	tsif_port_reg = (volatile unsigned int *)tcc_p2v(HwTSIFPORTSEL_BASE);

#if (TSIF_GPIO_PORT == 0xD)
	gpio_regs = (volatile PGPION)tcc_p2v(HwGPIOD_BASE);
#elif (TSIF_GPIO_PORT == 0xE)
	gpio_regs = (volatile PGPION)tcc_p2v(HwGPIOE_BASE);
#elif (TSIF_GPIO_PORT == 0xF)
	gpio_regs = (volatile PGPION)tcc_p2v(HwGPIOF_BASE);
#endif

#if (TSIF_CH == 1)
	tsif_regs = (volatile PTSIF)tcc_p2v(HwTSIF1_BASE);
#else
	tsif_regs = (volatile PTSIF)tcc_p2v(HwTSIF0_BASE);
#endif

#ifdef GDMA
	gdma_regs = (volatile PGDMACTRL)tcc_p2v(HwGDMA1_BASE); 
#else
#if (TSIF_CH == 1)
	gpsb_regs = (volatile PGPSB)tcc_p2v(HwGPSBCH1_BASE);; 
#else
	gpsb_regs = (volatile PGPSB)tcc_p2v(HwGPSBCH0_BASE); 
#endif
#endif	
#elif defined(CONFIG_ARCH_TCC88XX)
	tsif_port_reg = (volatile unsigned int *)tcc_p2v(HwTSIF_PORTSEL_BASE);

	gpio_regs = (volatile PGPION)tcc_p2v(((TSIF_GPIO_PORT == 0xE) ? HwGPIOE_BASE : HwGPIOF_BASE));
	tsif_regs = (volatile PTSIF)tcc_p2v((TSIF_CH == 1) ? HwTSIF1_BASE : HwTSIF0_BASE);

#ifdef GDMA
	gdma_regs = (volatile PGDMACTRL)tcc_p2v(HwGDMA1_BASE); 
#else
	gpsb_regs = (volatile PGPSB)tcc_p2v((TSIF_CH == 1) ? HwGPSBCH1_BASE : HwGPSBCH0_BASE); 
#endif	
#endif
	if(mod == 0) {
		printk("\n####################################\n");
		printk("GP%cFN0 : 0x%08x > 0x%08x\n", ((TSIF_GPIO_PORT == 0xE) ? 'E' : 'F'), (int)&gpio_regs->GPFN0, gpio_regs->GPFN0);
		printk("GP%cFN1 : 0x%08x > 0x%08x\n", ((TSIF_GPIO_PORT == 0xE) ? 'E' : 'F'), (int)&gpio_regs->GPFN1, gpio_regs->GPFN1);
		printk("GP%cFN2 : 0x%08x > 0x%08x\n", ((TSIF_GPIO_PORT == 0xE) ? 'E' : 'F'), (int)&gpio_regs->GPFN2, gpio_regs->GPFN2);
		printk("GP%cFN3 : 0x%08x > 0x%08x\n\n", ((TSIF_GPIO_PORT == 0xE) ? 'E' : 'F'), (int)&gpio_regs->GPFN3, gpio_regs->GPFN3);
		printk("TSCR   : 0x%08x > 0x%08x\n", (int)&tsif_regs->TSCR, tsif_regs->TSCR);
		printk("TSIC   : 0x%08x > 0x%08x\n", (int)&tsif_regs->TSIC, tsif_regs->TSIC);
		printk("TSCHS  : 0x%08x > 0x%08x\n\n", (int)tsif_port_reg, *tsif_port_reg);
#ifdef GDMA
		printk("ST_SADR: 0x%08x > 0x%08x\n", (int)&gdma_regs->ST_SADR2, gdma_regs->ST_SADR2);
		printk("SPARAM : 0x%08x > 0x%08x\n", (int)&gdma_regs->SPARAM2, gdma_regs->SPARAM2);
		printk("ST_DADR: 0x%08x > 0x%08x\n", (int)&gdma_regs->ST_DADR2, gdma_regs->ST_DADR2);
		printk("DPARAM : 0x%08x > 0x%08x\n", (int)&gdma_regs->DPARAM2, gdma_regs->DPARAM2);
		printk("HCOUNT : 0x%08x > 0x%08x\n", (int)&gdma_regs->HCOUNT2, gdma_regs->HCOUNT2);
		printk("CHCTRL : 0x%08x > 0x%08x\n", (int)&gdma_regs->CHCTRL2, gdma_regs->CHCTRL2);
		printk("RPTCTRL: 0x%08x > 0x%08x\n", (int)&gdma_regs->RPTCTRL2, gdma_regs->RPTCTRL2);
		printk("EXTREQ : 0x%08x > 0x%08x\n", (int)&gdma_regs->EXTREQ2, gdma_regs->EXTREQ2);
#else
		printk("RXBASE : 0x%08x > 0x%08x\n", (int)&gpsb_regs->RXBASE, gpsb_regs->RXBASE);
		printk("PACKET : 0x%08x > 0x%08x\n", (int)&gpsb_regs->PACKET, gpsb_regs->PACKET);
		printk("DMACTR : 0x%08x > 0x%08x\n", (int)&gpsb_regs->DMACTR, gpsb_regs->DMACTR);
		printk("DMAICR : 0x%08x > 0x%08x\n", (int)&gpsb_regs->DMAICR, gpsb_regs->DMAICR);
#endif	
		printk("####################################\n\n");
	} else {
		printk("\nTSIS   : 0x%08x > 0x%08x\n", (int)&tsif_regs->TSIS, tsif_regs->TSIS);
#ifdef GDMA	
		printk("C_DADR : 0x%08x > 0x%08x\n", (int)&gdma_regs->C_DADR2, gdma_regs->C_DADR2);
		printk("HCOUNT : 0x%08x > 0x%08x\n", (int)&gdma_regs->HCOUNT2, gdma_regs->HCOUNT2);
#else
		printk("DMASTR : 0x%08x > 0x%08x\n",(int)&gpsb_regs->DMASTR, gpsb_regs->DMASTR);
		printk("DMAICR : 0x%08x > 0x%08x\n\n", (int)&gpsb_regs->DMAICR, gpsb_regs->DMAICR);
#endif
	}
#endif
}

int tcc_tsif_p_debug_thread(void *data)
{
	while (!kthread_should_stop()) {
		if(debug_time != 0) {
			if(debug_time == cur_time) {
				tcc_tsif_p_debug(1);

#if 0			
				if(debug_cnt++ > 5) {
					st_p_tsif *instance = &p_tsif_inst;
					struct tcc_tsif_module_handle *handle = &instance->handle;

					handle->dma_stop(handle);
					handle->dma_start(handle);

					printk("\n\t## dma restart...\n\n");
				}
#endif				
			} else {
				debug_cnt = 0;
			}
		}

		debug_time = cur_time;
		
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(CHK_TIME*HZ);
	}

	return 0;
}
#endif	//#ifdef TSIF_DEBUG

int tcc_tsif_p_start(void)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_module_handle *handle = &instance->handle;

#if defined(CONFIG_ARCH_TCC93XX)
	clk_enable(gpsb_clk);	
#endif

	if(instance->state == P_TSIF_STATE_DEINIT)
	{
		printk("[%s] Could not start : 0x%x\n", __func__, instance->state);
		return -EACCES;
	}

//20110526 koo : android tcc880x kernel에서는 module init 후에 tsif & gpsb1을 swret state 및 clk disable 상태로 만들기 때문에 tsif start 시 swreset cancel & clk enable 시켜줌
#if defined(CONFIG_ARCH_TCC88XX)
	tca_ckc_setioswreset(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), ON);
	tca_ckc_setioswreset(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), OFF);
	tca_ckc_setiobus(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), ENABLE);
#ifndef GDMA
	tca_ckc_setioswreset(RB_GPSBCONTROLLER1, ON);
	tca_ckc_setioswreset(RB_GPSBCONTROLLER1, OFF);
	tca_ckc_setiobus(RB_GPSBCONTROLLER1, ENABLE);
#endif
#endif

    handle->msb_first		= 0x01;

#ifdef GDMA
	handle->big_endian		= 0x01;		//1:big endian, 0:little endian
#else
	handle->big_endian		= 0x00;		//1:big endian, 0:little endian
#endif	
	handle->serial_mode		= instance->mode;
	handle->clk_polarity	= 0x00;
	handle->valid_polarity	= 0x01;
	handle->sync_polarity	= 0x00;
	handle->sync_delay		= 0x00;

	handle->dma_stop(handle);
	handle->tsif_set(handle);
	handle->clear_fifo_packet(handle);
	handle->dma_start(handle);
	handle->prev_q_pos = (int)handle->rx_dma.v_addr;
	
	instance->state = P_TSIF_STATE_START;

#ifdef TSIF_DEBUG
	tcc_tsif_p_debug(0);
	if(debug_thread == NULL) {
		debug_time = 0;
		debug_cnt = 0;
		debug_thread = kthread_run(tcc_tsif_p_debug_thread, NULL, "itv_debug_thread");
	}
#endif

	return 0;
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_start);

int tcc_tsif_p_stop(void)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_module_handle *handle = &instance->handle;

#ifdef TSIF_DEBUG
	if(debug_thread != NULL) {
		kthread_stop(debug_thread); 
		debug_thread = NULL;
	}
#endif

	if(instance->state != P_TSIF_STATE_START)
		return 0;

	handle->dma_stop(handle);

	instance->state = P_TSIF_STATE_STOP;

#if defined(CONFIG_ARCH_TCC93XX)
	clk_disable(gpsb_clk);	
#endif

//20110526 koo : tsif start 시 swreset cancel & clk enable 시켜주기 때문에 stop 시 다시 swreset state & clk disable 시켜줌.
#if defined(CONFIG_ARCH_TCC88XX)
	tca_ckc_setioswreset(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), ON);
	tca_ckc_setiobus(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), DISABLE);
#ifndef GDMA
	tca_ckc_setioswreset(RB_GPSBCONTROLLER1, ON);
	tca_ckc_setiobus(RB_GPSBCONTROLLER1, DISABLE);
#endif
#endif

	return 0;
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_stop);

int tcc_tsif_p_init(unsigned char **buffer_addr, unsigned int *real_buffer_size, unsigned int buffer_size, 
		unsigned int pkt_intr_cnt, unsigned int timing_mode, unsigned int pid_mode, unsigned int serial_mode, 
		void (*callback)(void *), void *cb_data) 
{
	int error = 0;

	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_module_handle *handle = &instance->handle;
	struct tcc_tsif_regs *reg_addr;

	if(instance->state != P_TSIF_STATE_DEINIT)
	{
		printk("[%s] Already Init : 0x%x\n", __func__, instance->state);
		return -EACCES;
	}

#if defined(CONFIG_ARCH_TCC88XX)
	reg_addr = (struct tcc_tsif_regs *)((TSIF_CH == 0) ? HwTSIF0_BASE : HwTSIF1_BASE);
#elif defined(CONFIG_ARCH_TCC93XX)
	reg_addr = (struct tcc_tsif_regs *)((TSIF_CH == 0) ? &(HwTSIF0_BASE) : &(HwTSIF1_BASE));
#endif

	memset(instance, 0x00, sizeof(st_p_tsif));

	instance->callback = callback;
	instance->cb_data = cb_data;
	instance->mode = serial_mode;

	handle->dma_intr_packet_cnt = TSIF_DMA_PACKET_CNT;
	handle->dma_total_packet_cnt = ((TSIF_DMA_SIZE / MPEG_PACKET_SIZE) / handle->dma_intr_packet_cnt) * handle->dma_intr_packet_cnt;
	printk("tsif > dma size:0x%x / intr_cnt:%d / total_intr_cnt:%d\n", TSIF_DMA_SIZE, handle->dma_intr_packet_cnt, handle->dma_total_packet_cnt);	
	
#if defined(CONFIG_ARCH_TCC93XX)
	tca_ckc_set_iobus_swreset(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), OFF);
	tca_ckc_set_iobus_swreset(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), ON);
	tca_ckc_setiobus(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), ENABLE);
		
#ifndef GDMA
	tca_ckc_set_iobus_swreset((TSIF_CH == 0) ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, OFF);
	tca_ckc_set_iobus_swreset((TSIF_CH == 0) ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, ON);
	tca_ckc_setiobus((TSIF_CH == 0) ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, ENABLE);
#endif	
		
	if(TSIF_CH == 0)	gpsb_clk = clk_get(NULL, "gpsb0");
    else		        gpsb_clk = clk_get(NULL, "gpsb1");
    
    if(gpsb_clk == NULL) {
        printk("gpsb clock error on tsif: cannot get clock\n");
        return -EINVAL;
    }
#elif defined(CONFIG_ARCH_TCC88XX)
	//20110321 koo : tcc93xx와 tcc88xx의 swreset value가 뒤바뀌어져 있지만 tca_ckc_set_iobus_swreset를 동일하게 사용하고 있음
	tca_ckc_setioswreset(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), ON);
	tca_ckc_setioswreset(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), OFF);
	tca_ckc_setiobus(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), ENABLE);
#ifndef GDMA
	//20110321 koo : tcc93xx와 tcc88xx의 swreset value가 뒤바뀌어져 있지만 tca_ckc_set_iobus_swreset를 동일하게 사용하고 있음
	tca_ckc_setioswreset((TSIF_CH == 0) ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, ON);
	tca_ckc_setioswreset((TSIF_CH == 0) ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, OFF);
	tca_ckc_setiobus((TSIF_CH == 0) ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, ENABLE);
#endif	
#else
    // reset
	tca_ckc_set_iobus_swreset(RB_TSIFCONTROLLER, OFF);
	tca_ckc_set_iobus_swreset(RB_TSIFCONTROLLER, ON);
	tca_ckc_setiobus(RB_TSIFCONTROLLER, ENABLE);
#endif

	if(tca_tsif_module_init(handle,
				reg_addr,
				tsif_p_alloc_dma_buffer,
				tsif_p_free_dma_buffer,
				TSIF_DMA_SIZE,
				TSIF_CH,
				TSIF_DMA_CONTROLLER,
				TSIF_DMA_CH,
				TSIF_GPIO_PORT
				)) {
		printk("[%s] SPI initialize failed\n", __func__);
		error = -EBUSY;
		goto err_tsif_p;
	}

	error = request_irq(handle->irq, tsif_p_dma_handler, IRQF_SHARED, 
			"tcc-tsif", handle);

	if(error) 
		goto err_irq; 

	*buffer_addr = handle->rx_dma.v_addr;

#ifdef GDMA
	*real_buffer_size = TSIF_DMA_SIZE;
#else
	*real_buffer_size = handle->dma_total_packet_cnt * MPEG_PACKET_SIZE;
#endif
	
	{
		int i;

		for(i=0; i<32; i++) {
			instance->pidtable[i] = 0xffff;
			
#if defined(CONFIG_ARCH_TCC93XX)			
			volatile unsigned long* PIDT;

			PIDT = (volatile unsigned long *)tcc_p2v(HwGPSB_PIDT(i));
			*PIDT = 0x0;
#elif defined(CONFIG_ARCH_TCC88XX)
			HwGPSB_PIDT(i) = 0;
#endif
		}
	}	

	handle->prev_q_pos = (int)handle->rx_dma.v_addr;
	instance->state = P_TSIF_STATE_INIT;

	return 0;

err_irq:
	free_irq(handle->irq, handle);

err_tsif_p:
	
	tca_tsif_module_clean(handle);
	
#if defined(CONFIG_ARCH_TCC93XX) || defined(CONFIG_ARCH_TCC88XX)
	tca_ckc_setiobus(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), DISABLE);
	tca_ckc_setiobus((TSIF_CH == 0) ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, DISABLE);
#else
	tca_ckc_setiobus(RB_TSIFCONTROLLER, DISABLE);
#endif	

	return error;
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_init);

void tcc_tsif_p_deinit(void)
{
	st_p_tsif *instance = &p_tsif_inst;
	struct tcc_tsif_module_handle *handle = &instance->handle;

#ifdef TSIF_DEBUG
	if(debug_thread != NULL) {
		kthread_stop(debug_thread); 
		debug_thread = NULL;
	}
#endif

	if(instance->state == P_TSIF_STATE_DEINIT)
		return;

	free_irq(handle->irq, handle);
	
	tca_tsif_module_clean(handle);
	
#if defined(CONFIG_ARCH_TCC93XX) || defined(CONFIG_ARCH_TCC88XX)
	tca_ckc_setiobus(((TSIF_CH == 0) ? RB_TSIF0CONTROLLER : RB_TSIF1CONTROLLER), DISABLE);
	tca_ckc_setiobus((TSIF_CH == 0) ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, DISABLE);
#if defined(CONFIG_ARCH_TCC93XX)
	clk_put(gpsb_clk);
#endif
#else
	tca_ckc_setiobus(RB_TSIFCONTROLLER, DISABLE);
#endif	

	instance->state = P_TSIF_STATE_DEINIT;
}
EXPORT_SYMBOL_GPL(tcc_tsif_p_deinit);
