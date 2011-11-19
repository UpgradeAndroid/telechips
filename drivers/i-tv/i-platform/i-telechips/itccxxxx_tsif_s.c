#include <linux/version.h>
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
#include <linux/spi/tcc_tsif.h>
#include <linux/proc_fs.h>
#include "tcc/tca_spi_hwset.h"

#include <i-tv/itv_common.h>
#include "itccxxxx_tsif_s.h"

#define S_TSIF_SPI_NUM 1

#define PROC_FILE_NAME		"tcc_s_tsif_gpio"
#define S_TSIF_GPIO			0x0E
#define LENGTH				sizeof(S_TSIF_GPIO)

typedef enum {
	S_TSIF_STATE_DEINIT, 
	S_TSIF_STATE_INIT, 
	S_TSIF_STATE_START, 
	S_TSIF_STATE_STOP 
} e_s_tsif_state;

typedef struct {
	e_s_tsif_state state;

	tca_spi_handle_t handle;
	struct tcc_tsif_param param;

	void *cb_data;
	void (*callback)(void *);
} st_s_tsif;

char s_tsif_gpio_data[LENGTH + 1];

struct proc_dir_entry *proc_file;
struct proc_dir_entry *proc_dir;

st_s_tsif s_tsif_inst = {
	.state = S_TSIF_STATE_DEINIT, 
};

static void tsif_s_free_dma_buffer(struct tea_dma_buf *tdma)
{
	if(tdma) {
		if(tdma->v_addr)
			dma_free_writecombine(0, tdma->buf_size, tdma->v_addr, tdma->dma_addr);
		memset(tdma, 0, sizeof(struct tea_dma_buf));
	}
}

static int tsif_s_alloc_dma_buffer(struct tea_dma_buf *tdma, unsigned int size)
{
	int error = -1;

	if(tdma) {
		tsif_s_free_dma_buffer(tdma);
		tdma->buf_size = size;
		tdma->v_addr = dma_alloc_writecombine(0, tdma->buf_size, &tdma->dma_addr, GFP_KERNEL);
		printk("[%s] Alloc DMA buffer @0x%X(Phy=0x%X), size:%d\n", __func__, (unsigned int)tdma->v_addr,
				(unsigned int)tdma->dma_addr, tdma->buf_size);
		error = tdma->v_addr ? 0 : 1;
	}

	return error;
}

static irqreturn_t tsif_s_dma_handler(int irq, void *dev_id)
{
	st_s_tsif *instance = &s_tsif_inst;
	tca_spi_handle_t *tspi = (tca_spi_handle_t *)dev_id;
	unsigned long dma_done_reg = 0;

	dma_done_reg = tspi->regs->DMAICR;

	if(dma_done_reg & (Hw28 | Hw29)) {
		BITSET(tspi->regs->DMAICR, (Hw29 | Hw28));
        tspi->cur_q_pos = (int)(tspi->regs->DMASTR >> 17);
		if(instance->callback)
			instance->callback(instance->cb_data);
	}

	return IRQ_HANDLED;
}

static int tcc_tsif_s_get_gpio(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;
	char *port = (char *)data;

	len = sprintf(page, "%s", port);

	return len;
}

static int tcc_tsif_s_set_gpio(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len, test;
	char *fb_data = (char *)data;

	if(count > LENGTH + 1)
		len = LENGTH + 1;
	else
		len = count;

	if(copy_from_user(fb_data, buffer, len))
		return -EFAULT;

	if(sscanf(fb_data, "0x%x", &test) == 0)
	{
		test = 0x0e;
		eprintk("Resource set fail!! default port 0x%02x\n", test);

		sprintf(fb_data, "0x%02x", S_TSIF_GPIO);
		len = LENGTH;
	}
	else
	{
		if((test < 0x0C) || (test > 0x0E))
		{
			test = 0x0e;
			eprintk("Resource set fail!! default port 0x%02x\n", test);

			sprintf(fb_data, "0x%02x", S_TSIF_GPIO);
			len = LENGTH;
		}
	}

	return len;
}

int tcc_tsif_s_get_pos(void)
{
	st_s_tsif *instance = &s_tsif_inst;
	tca_spi_handle_t *handle = &instance->handle;

	return handle->cur_q_pos;
}
EXPORT_SYMBOL_GPL(tcc_tsif_s_get_pos);

int tcc_tsif_s_start(void)
{
	st_s_tsif *instance = &s_tsif_inst;
	tca_spi_handle_t *handle = &instance->handle;
	struct tcc_tsif_param *param = &instance->param;

	if(instance->state == S_TSIF_STATE_DEINIT)
	{
		printk("[%s] Could not start : 0x%x\n", __func__, instance->state);
		return -EACCES;
	}
	
	handle->dma_stop(handle);

	tca_spi_setCPHA(handle->regs, param->mode & SPI_CPHA);
	tca_spi_setCPOL(handle->regs, param->mode & SPI_CPOL);
	tca_spi_setCS_HIGH(handle->regs, param->mode & SPI_CS_HIGH);
	tca_spi_setLSB_FIRST(handle->regs, param->mode & SPI_LSB_FIRST);

	handle->dma_mode = param->dma_mode;
	if(handle->dma_mode == 0)
		handle->set_mpegts_pidmode(handle, 0);

	handle->dma_total_packet_cnt = param->ts_total_packet_cnt;
	handle->dma_intr_packet_cnt = param->ts_intr_packet_cnt;

	handle->clear_fifo_packet(handle);
	handle->q_pos = handle->cur_q_pos = 0;

	handle->set_packet_cnt(handle, MPEG_PACKET_SIZE);
	handle->dma_start(handle);

	instance->state = S_TSIF_STATE_START;

	return 0;
}
EXPORT_SYMBOL_GPL(tcc_tsif_s_start);

int tcc_tsif_s_stop(void)
{
	st_s_tsif *instance = &s_tsif_inst;
	tca_spi_handle_t *handle = &instance->handle;

	if(instance->state != S_TSIF_STATE_START)
		return 0;

	handle->dma_stop(handle);

	instance->state = S_TSIF_STATE_STOP;

	return 0;
}
EXPORT_SYMBOL_GPL(tcc_tsif_s_stop);

int tcc_tsif_s_init(unsigned char **buffer_addr, unsigned int buffer_size, unsigned int pkt_intr_cnt, 
		unsigned int timing_mode, unsigned int pid_mode, void (*callback)(void *), void *cb_data) 
{
	int error = 0;
	int port;

	st_s_tsif *instance = &s_tsif_inst;
	tca_spi_handle_t *handle = &instance->handle;
	struct tcc_tsif_param *param = &instance->param;
	struct tca_spi_regs *reg_addr;

	if(instance->state != S_TSIF_STATE_DEINIT)
	{
		printk("[%s] Already Init : 0x%x\n", __func__, instance->state);
		return -EACCES;
	}

	printk("[%s] buffer_size: %d, pkt_intr_cnt: %d, timing_mode: 0x%x, pid_mode: 0x%x\n", __func__, 
			buffer_size, pkt_intr_cnt, timing_mode, pid_mode);

#if defined(CONFIG_MACH_TCC8800)
	reg_addr = (struct tca_spi_regs *)((S_TSIF_SPI_NUM == 0) ? HwGPSBCH0_BASE : HwGPSBCH1_BASE);
#elif defined(CONFIG_ARCH_TCC93XX)
	reg_addr = (struct tca_spi_regs *)((S_TSIF_SPI_NUM == 0) ? &(HwGPSBCH0_BASE) : &(HwGPSBCH1_BASE));
#endif
	
	memset(instance, 0x00, sizeof(st_s_tsif));

	instance->callback = callback;
	instance->cb_data = cb_data;

	param->ts_total_packet_cnt	= buffer_size / MPEG_PACKET_SIZE;
	param->ts_intr_packet_cnt	= pkt_intr_cnt;
	param->mode					= timing_mode;
	param->dma_mode				= pid_mode;

	// reset
	tca_ckc_set_iobus_swreset(S_TSIF_SPI_NUM == 0 ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, OFF);
	tca_ckc_set_iobus_swreset(S_TSIF_SPI_NUM == 0 ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, ON);

	tca_ckc_setiobus(S_TSIF_SPI_NUM == 0 ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, ENABLE);

	if(sscanf(s_tsif_gpio_data, "0x%x", &port) == 0) {
		port = 0x0E;
		eprintk("Serial tsif gpio resource get fail!! default port 0x%02x\n", port);
	}

	if (tca_spi_init(handle,
				reg_addr,
				(S_TSIF_SPI_NUM == 0) ? INT_GPSB0_DMA : INT_GPSB1_DMA,
				tsif_s_alloc_dma_buffer,
				tsif_s_free_dma_buffer,
				buffer_size,
				S_TSIF_SPI_NUM,
				1, port)) {
		printk("[%s] SPI initialize failed\n", __func__);
		error = -EBUSY;
		goto err_spi;
	}

	handle->clear_fifo_packet(handle);
	handle->dma_stop(handle);

	handle->dma_total_packet_cnt = handle->dma_total_size / TSIF_PACKET_SIZE;
	handle->dma_intr_packet_cnt = 1;

	handle->hw_init(handle);

	error = request_irq(handle->irq, tsif_s_dma_handler, IRQF_SHARED, 
			TSIF_DEV_NAME, handle);
	if(error) 
		goto err_irq; 

	handle->set_packet_cnt(handle, MPEG_PACKET_SIZE);

	*buffer_addr = handle->rx_dma.v_addr;

	instance->state = S_TSIF_STATE_INIT;

	return 0;

err_irq:
	free_irq(handle->irq, handle);

err_spi:
	tca_spi_clean(handle);
	tca_ckc_setiobus(S_TSIF_SPI_NUM == 0 ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, DISABLE);

	return error;
}
EXPORT_SYMBOL_GPL(tcc_tsif_s_init);

void tcc_tsif_s_deinit(void)
{
	st_s_tsif *instance = &s_tsif_inst;
	tca_spi_handle_t *handle = &instance->handle;

	if(instance->state == S_TSIF_STATE_DEINIT)
		return;

	remove_proc_entry(PROC_FILE_NAME, NULL);

	free_irq(handle->irq, handle);
	tca_spi_clean(handle);
	tca_ckc_setiobus(S_TSIF_SPI_NUM == 0 ? RB_GPSBCONTROLLER0 : RB_GPSBCONTROLLER1, DISABLE);

	instance->state = S_TSIF_STATE_DEINIT;
}
EXPORT_SYMBOL_GPL(tcc_tsif_s_deinit);

int tcc_tsif_s_init_proc(void)
{
	int ret = 0;

	proc_file = create_proc_entry(PROC_FILE_NAME, S_IFREG | S_IRWXU, NULL);

	if(proc_file == NULL)
	{
		ret = -ENODEV;
		eprintk("%s proc filesystem can't create!!!\n", PROC_FILE_NAME);
	}
	else
	{
		sprintf(s_tsif_gpio_data, "0x%02x", S_TSIF_GPIO);
		proc_file->data = (void *)s_tsif_gpio_data;
		proc_file->read_proc = tcc_tsif_s_get_gpio;
		proc_file->write_proc = tcc_tsif_s_set_gpio;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29)
		proc_file->owner = THIS_MODULE;
#endif
	}

	return ret;
}
EXPORT_SYMBOL_GPL(tcc_tsif_s_init_proc);
