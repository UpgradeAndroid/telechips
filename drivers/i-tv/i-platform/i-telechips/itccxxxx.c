#include <mach/bsp.h>
#include <mach/io.h>

#include <i-tv/itv_common.h>
#include <i-tv/itv_platform.h>

#include "itccxxxx_tsif_p.h"
#include "itccxxxx_tsif_s.h"

#if defined(CONFIG_ARCH_TCC93XX) || defined(CONFIG_ARCH_TCC88XX)
struct tcc_gpio_regs {
    volatile unsigned long GPDAT, GPEN, GPSET, GPCLR, GPXOR, GPCD0, GPCD1, GPPD0, GPPD1, GPFN0, GPFN1, GPFN2,GPFN3;
};
#endif

//20110321 koo : S5H1411&XC5000_ATSC_SV6.0 brd������ cam1�� ����ϹǷ� cam1_on on/off
#if defined(CONFIG_ARCH_TCC88XX)
#include <mach/gpio.h>
#endif

/* frontend operations */
static void itv_platform_demod_reset(void)
{
#if defined(CONFIG_ARCH_TCC93XX)
	struct tcc_gpio_regs *reg_addr;
	volatile struct tcc_gpio_regs *regs;

//20110126 koo : tcc9300_stb_board TS_PWDN# & TS_RST set
#if defined(CONFIG_MACH_TCC9300ST)
	reg_addr = (struct tcc_gpio_regs *)&HwGPIOD_BASE;
	regs = (volatile struct tcc_gpio_regs *)tcc_p2v(*reg_addr);
	
	BITCSET(regs->GPFN3, (Hw8-Hw0), 0);

	BITSET(regs->GPEN, Hw25);
	BITSET(regs->GPDAT, Hw25);

	msleep(10);
	
	BITSET(regs->GPEN, Hw24);
	BITSET(regs->GPDAT, Hw24);
	msleep(50);
	BITCLR(regs->GPDAT, Hw24);
	msleep(50);
	BITSET(regs->GPDAT, Hw24);
#else
	reg_addr = (struct tcc_gpio_regs *)&HwGPIOE_BASE;
	regs = (volatile struct tcc_gpio_regs *)tcc_p2v(*reg_addr);
	
	BITCSET(regs->GPFN1, (Hw16-Hw12), 0);
	BITSET(regs->GPEN, Hw11);
	BITSET(regs->GPDAT, Hw11);
	msleep(50);
	BITCLR(regs->GPDAT, Hw11);
	msleep(50);
	BITSET(regs->GPDAT, Hw11);
#endif	//#if defined(CONFIG_MACH_TCC9300ST)
#elif defined(CONFIG_MACH_TCC8800)		//TCC93&88_DEMO_V6.1���� xc5000�� ����� ���� cam1 interface�� �̿��ϰ�, fqd1136�� ����� ���� ts parallel interface�� �̿�.
#if defined(CONFIG_iTV_FE_TUNER_MODULE_XC5000) || defined(CONFIG_iTV_FE_TUNER_MODULE_XC5000_MODULE)
	volatile PGPION regs;

	gpio_request(TCC_GPEXT1(13), NULL);
	gpio_request(TCC_GPEXT3(6), NULL);
	gpio_direction_output(TCC_GPEXT1(13), 1);

	regs = (volatile PGPION)tcc_p2v(HwGPIOE_BASE);
	
	BITCSET(regs->GPFN0, (Hw12-Hw8), 0);
	BITSET(regs->GPEN, Hw2);
	BITSET(regs->GPDAT, Hw2);				//TS_PWDN

	msleep(10);

	gpio_direction_output(TCC_GPEXT3(6), 1);
	msleep(50);
	gpio_direction_output(TCC_GPEXT3(6), 0);
	msleep(50);
	gpio_direction_output(TCC_GPEXT3(6), 1);
#elif defined(CONFIG_iTV_FE_TUNER_MODULE_FQD1136) || defined(CONFIG_iTV_FE_TUNER_MODULE_FQD1136_MODULE)
	PGPION regs;

	gpio_request(TCC_GPEXT3(4), NULL);
	gpio_direction_output(TCC_GPEXT3(4), 1);	

	regs = (volatile PGPION)tcc_p2v(HwGPIOF_BASE);

	BITCSET(regs->GPFN2, (Hw28-Hw24), 0);
	BITSET(regs->GPEN, Hw22);
	BITSET(regs->GPDAT, Hw22);				//TS_PWDN

	msleep(10);
	
	BITCSET(regs->GPFN1, (Hw24-Hw20), 0);
	BITSET(regs->GPEN, Hw13);
	BITSET(regs->GPDAT, Hw13);				//TS_RST
	msleep(50);
	BITCLR(regs->GPDAT, Hw13);				
	msleep(50);
	BITSET(regs->GPDAT, Hw13);				
#endif	//#if defined(CONFIG_iTV_FE_TUNER_MODULE_XC5000) || defined(CONFIG_iTV_FE_TUNER_MODULE_XC5000_MODULE)
#elif defined(CONFIG_MACH_TCC8800ST)
	PGPION regs;

	//20110706 koo : i2c0-scl gpio_a0 conf�� clear �Ǿ� �ش� conf set
	regs = (volatile PGPION)tcc_p2v(HwGPIOA_BASE);
	if((regs->GPFN0 & 0x00000001) != 0x1)		BITCSET(regs->GPFN0, (Hw4-Hw0), Hw0);
	
	regs = (volatile PGPION)tcc_p2v(HwGPIOF_BASE);

	BITCSET(regs->GPFN3, (Hw8-Hw4), 0);
	BITSET(regs->GPEN, Hw25);
	BITSET(regs->GPDAT, Hw25);				//TS_PWDN	

	msleep(10);

	BITCSET(regs->GPFN1, (Hw16-Hw12), 0);
	BITSET(regs->GPEN, Hw11);
	BITSET(regs->GPDAT, Hw11);				//TS_RST
	msleep(50);
	BITCLR(regs->GPDAT, Hw11);				
	msleep(50);
	BITSET(regs->GPDAT, Hw11);				
#else
	BITCSET(HwGPIOE->GPFN1, (Hw16-Hw12), 0);
	BITSET(HwGPIOE->GPEN, Hw11);
	BITSET(HwGPIOE->GPDAT, Hw11);
	msleep(50);
	BITCLR(HwGPIOE->GPDAT, Hw11);
	msleep(50);
	BITSET(HwGPIOE->GPDAT, Hw11);
#endif

	//20110224 koo : reset �� �ٷ� i2c�� read �� ��� demod reset�� �Ϸ�Ǳ� ���� read�Ͽ� �߸��� data�� �����ͼ� reset �� delay add
	msleep(1);
}

static void itv_platform_demod_pwroff(int tsif_mod)
{
	//tsif stop
	if(tsif_mod)	tcc_tsif_p_stop();

#if defined(CONFIG_ARCH_TCC93XX)
	struct tcc_gpio_regs *reg_addr;
	volatile struct tcc_gpio_regs *regs;

#if defined(CONFIG_MACH_TCC9300ST)
	reg_addr = (struct tcc_gpio_regs *)&HwGPIOD_BASE;
	regs = (volatile struct tcc_gpio_regs *)tcc_p2v(*reg_addr);
	
	BITCLR(regs->GPDAT, Hw25);
	BITCLR(regs->GPDAT, Hw24);
#endif
#elif defined(CONFIG_MACH_TCC8800)		//TCC93&88_DEMO_V6.1���� xc5000�� ����� ���� cam1 interface�� �̿��ϰ�, fqd1136�� ����� ���� ts parallel interface�� �̿�.
#if defined(CONFIG_iTV_FE_TUNER_MODULE_XC5000) || defined(CONFIG_iTV_FE_TUNER_MODULE_XC5000_MODULE)
	PGPION regs;

	regs = (volatile PGPION)tcc_p2v(HwGPIOE_BASE);
	BITCLR(regs->GPDAT, Hw2);																				//TS_PWDN
	gpio_request(TCC_GPEXT1(13), NULL);
	gpio_request(TCC_GPEXT3(6), NULL);
	gpio_direction_output(TCC_GPEXT1(13), 0);																	//TS_PWR
	gpio_direction_output(TCC_GPEXT3(6), 0);																//TS_RST
#elif defined(CONFIG_iTV_FE_TUNER_MODULE_FQD1136) || defined(CONFIG_iTV_FE_TUNER_MODULE_FQD1136_MODULE)
	PGPION regs;

	regs = (volatile PGPION)tcc_p2v(HwGPIOF_BASE);
	BITCLR(regs->GPDAT, Hw22);																				//TS_PWDN
	BITCLR(regs->GPDAT, Hw13);																				//TS_RST	
	gpio_request(TCC_GPEXT3(4), NULL);
	gpio_direction_output(TCC_GPEXT3(4), 0);																	//TS_PWR
#endif
#elif defined(CONFIG_MACH_TCC8800ST)
	PGPION regs;

	regs = (volatile PGPION)tcc_p2v(HwGPIOF_BASE);
	BITCLR(regs->GPDAT, Hw25);																				//TS_PWDN
#endif
}

//20110126 koo : tcc9300_stb_board i2c1-0 using
#if defined(CONFIG_MACH_TCC9300ST)
#define ITV_TCCXXXX_I2C_CH		2
//20110324 koo : i2c ch0�� ����� ��� wm8731(addr : 0x1a)�� slave addr�� �����Ͽ� i2c ch1�� ����ϵ��� board �ù�.
#elif defined(CONFIG_MACH_TCC8800)
#define ITV_TCCXXXX_I2C_CH		1
#elif defined(CONFIG_MACH_TCC8800ST)
#define ITV_TCCXXXX_I2C_CH		0
#else
#define ITV_TCCXXXX_I2C_CH		1
#endif



static struct i2c_adapter *itv_platform_i2c_get_adapter(void)
{
	return i2c_get_adapter(ITV_TCCXXXX_I2C_CH);
}

static void itv_platform_i2c_put_adapter(struct i2c_adapter *p_i2c_adap)
{
	if(!p_i2c_adap)
		return;

	i2c_put_adapter(p_i2c_adap);
}

itv_platform_frontend_operations_t *itv_platform_get_frontend_operations(void)
{
	itv_platform_frontend_operations_t *p_fe_ops;

	p_fe_ops = kzalloc(sizeof(*p_fe_ops), GFP_KERNEL);
	if(!p_fe_ops) {
		eprintk("out of memory\n");
		return NULL;
	}

	// demodulator operations
	p_fe_ops->demod_reset = itv_platform_demod_reset;
	p_fe_ops->demod_pwroff = itv_platform_demod_pwroff;

	// i2c operations
	p_fe_ops->i2c_get_adapter = itv_platform_i2c_get_adapter;
	p_fe_ops->i2c_put_adapter = itv_platform_i2c_put_adapter;

	return p_fe_ops;
}
EXPORT_SYMBOL(itv_platform_get_frontend_operations);

void itv_platform_put_frontend_operations(itv_platform_frontend_operations_t *p_fe_ops)
{
	kfree(p_fe_ops);
}
EXPORT_SYMBOL(itv_platform_put_frontend_operations);

/* ts interface operations */
itv_platform_tsif_operations_t *itv_platform_get_tsif_operations(void)
{
	itv_platform_tsif_operations_t *p_tsif_ops;

	p_tsif_ops = kzalloc(sizeof(*p_tsif_ops), GFP_KERNEL);
	if(!p_tsif_ops) {
		eprintk("out of memory\n");
		return NULL;
	}

	// TS parallel interface operations
	p_tsif_ops->tsif_p_init 	= tcc_tsif_p_init;
	p_tsif_ops->tsif_p_deinit 	= tcc_tsif_p_deinit;
	p_tsif_ops->tsif_p_get_pos 	= tcc_tsif_p_get_pos;
	p_tsif_ops->tsif_p_start 	= tcc_tsif_p_start;
	p_tsif_ops->tsif_p_stop 	= tcc_tsif_p_stop;
	p_tsif_ops->tsif_p_insert_pid 		= tcc_tsif_p_insert_pid;
	p_tsif_ops->tsif_p_remove_pid 		= tcc_tsif_p_remove_pid;
	p_tsif_ops->tsif_p_set_packetcnt 	= tcc_tsif_p_set_packetcnt;

	// TS serial interface operations
	p_tsif_ops->tsif_s_init_proc 	= tcc_tsif_s_init_proc;
	p_tsif_ops->tsif_s_init 		= tcc_tsif_s_init;
	p_tsif_ops->tsif_s_deinit 		= tcc_tsif_s_deinit;
	p_tsif_ops->tsif_s_get_pos 		= tcc_tsif_s_get_pos;
	p_tsif_ops->tsif_s_start 		= tcc_tsif_s_start;
	p_tsif_ops->tsif_s_stop 		= tcc_tsif_s_stop;

	return p_tsif_ops;
}
EXPORT_SYMBOL(itv_platform_get_tsif_operations);

void itv_platform_put_tsif_operations(itv_platform_tsif_operations_t *p_tsif_ops)
{
	kfree(p_tsif_ops);
}
EXPORT_SYMBOL(itv_platform_put_tsif_operations);

MODULE_AUTHOR("JP");
MODULE_LICENSE("GPL");
