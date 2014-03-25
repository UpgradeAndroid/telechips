/*
 * arch/arm/mach-tcc92x/cpufreq.c
 *
 * TCC92xx cpufreq driver
 *
 * Copyright (C) 2011 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/suspend.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/consumer.h>
#include <mach/bsp.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <linux/power_supply.h>
#include <plat/nand.h>

#define TCC_TRANSITION_LATENCY		100

#if defined(CONFIG_DRAM_MDDR)
#include "tcc_clocktbl_lpddr.h"
#elif defined(CONFIG_DRAM_DDR2)
#include "tcc_clocktbl_ddr2.h"
#else
#error Select the DDR type
#endif

struct tcc_pll_table_t {
	unsigned int cpu_freq;
	unsigned int pll_freq;
};

struct tcc_voltage_table_t {
	unsigned int cpu_freq;
	unsigned int ddi_freq;
	unsigned int mem_freq;
	unsigned int gpu_freq;
	unsigned int io_freq;
	unsigned int vbus_freq;
	unsigned int vcod_freq;
	unsigned int smu_freq;
	unsigned int voltage;
};

static struct tcc_pll_table_t tcc_pll_table[] = {
	{ 144000, 144000 },
	{ 294000, 294000 },
	{ 486000, 486000 },		// for TV-Out
	{ 500000, 500000 },
	{ 540000, 540000 },		// for TV-Out
	{ 600000, 600000 },
#if defined(CONFIG_DRAM_MDDR) 
#if defined(CONFIG_CPU_OVERCLOCKING_700MHZ)
	{ 704000, 704000 },
#endif
#elif defined(CONFIG_DRAM_DDR2)
	{ 660000, 660000 },
	{ 720000, 720000 },
#if defined(CONFIG_CPU_HIGHSPEED)
	{ 800000, 800000 },
	{ 900000, 900000 },
	{1000000,1000000 },
#endif
#else
#error
#endif
};

static struct tcc_voltage_table_t tcc_voltage_table[] = {
	/*   cpu     ddi     mem     gpu      io    vbus    vcod     smu      vol */
	{ 300000, 144000, 156000, 114000, 100000, 129000,  96000,  75000,  1000000 },
	{ 350000, 168000, 182000, 133000, 116500, 152000, 112000,  87500,  1050000 },
	{ 400000, 192000, 208000, 152000, 133000, 175000, 128000, 100000,  1100000 },
	{ 450000, 216000, 234000, 171000, 149500, 195000, 144000, 112500,  1150000 },
	{ 500000, 240000, 260000, 190000, 166000, 215000, 160000, 125000,  1200000 },
	{ 550000, 264000, 286000, 209000, 183000, 236500, 176000, 137500,  1250000 },
	{ 600000, 288000, 312000, 228000, 200000, 258000, 192000, 150000,  1300000 },
	{ 650000, 312000, 338000, 247000, 216000, 279500, 208000, 162500,  1350000 },
	{ 700000, 336000, 364000, 266000, 232000, 301000, 224000, 175000,  1400000 },
//	{ 720000, 345000, 374000, 274000, 266000, 310000, 230000, 180000,  1420000 },
	{ 720000, 345000, 374000, 274000, 266000, 330000, 230000, 180000,  1420000 },
#if defined(CONFIG_CPU_HIGHSPEED)
	{1000000, 345000, 374000, 274000, 266000, 330000, 230000, 250000,  1650000 },
#endif
};

static struct tcc_freq_table_t tcc_freq_old_table = {
	       0,      0,      0,      0,      0,      0,      0,      0
};

extern struct tcc_freq_table_t gtClockLimitTable[];

struct tcc_freq_limit_table_t {
	struct tcc_freq_table_t freq;
	int usecount;
};

static struct tcc_freq_limit_table_t tcc_freq_limit_table[TCC_FREQ_LIMIT_MAX] = {{{0,0,0,0,0,0,0,0}, 0},};
static struct tcc_freq_table_t tcc_freq_curr_limit_table;
static unsigned int tcc_limitclocktbl_flag = 0;

static unsigned int tcc_freq_mutex_init_flag = 0;
static struct mutex tcc_freq_mutex;

#define NUM_VOLTAGES	ARRAY_SIZE(tcc_voltage_table)
#define NUM_PLLS		ARRAY_SIZE(tcc_pll_table)
#define NUM_FREQS		ARRAY_SIZE(gtClockLimitTable)

static struct cpufreq_frequency_table tcc_cpufreq_table[NUM_FREQS + 1];

static struct clk *pll0_clk;
static struct clk *cpu_clk;
static struct clk *mem_clk;
static struct clk *io_clk;
static struct clk *ddi_clk;
static struct clk *gpu_clk;
static struct clk *smu_clk;
static struct clk *vcod_clk;
static struct clk *vbus_clk;

#ifdef CONFIG_REGULATOR
static struct regulator *vdd_core;
#endif
static int curr_voltage = 0;

extern int cpufreq_is_performace_governor(void);

#if defined(CONFIG_CPU_HIGHSPEED)
#define DEBUG_HIGHSPEED 0
#define dbg_highspeed(msg...)	if (DEBUG_HIGHSPEED) { printk( "TCC_HIGHSPEED: " msg); }

#define TCC_CPU_FREQ_HIGH_SPEED        1000000
#define TCC_CPU_FREQ_NORMAL_SPEED       720000
//#define TCC_CPU_FREQ_NORMAL_SPEED       660000

enum cpu_highspeed_status_t {
	CPU_FREQ_PROCESSING_NORMAL = 0,
	CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED,
	CPU_FREQ_PROCESSING_HIGHSPEED,
	CPU_FREQ_PROCESSING_MAX
};

#define HIGHSPEED_TIMER_TICK             100                                // 100 ms.
#define HIGHSPEED_LIMIT_CLOCK_PERIOD     (25000/HIGHSPEED_TIMER_TICK)       //  25 sec.	25000 ms.
#define HIGHSPEED_HIGER_CLOCK_PERIOD     (10000/HIGHSPEED_TIMER_TICK)       //  10 sec.	10000 ms.

static int highspeed_highclock_flag = 0;
static int highspeed_highclock_counter = 0;
static int highspeed_lowclock_flag = 0;
static int highspeed_lowclock_counter = 0;
static int highspeed_limit_highspeed_counter = 0;

static struct workqueue_struct *cpufreq_wq;
static struct work_struct cpufreq_work;
static enum cpu_highspeed_status_t cpu_highspeed_status = CPU_FREQ_PROCESSING_NORMAL;
static struct timer_list timer_highspeed;
static int tcc_cpufreq_set_clock_table(struct tcc_freq_table_t *curr_clk_tbl);
static unsigned int tcc_freq_curr_target_cpufreq = TCC_CPU_FREQ_HIGH_SPEED;

extern int tcc_battery_get_charging_status(void);
extern int tcc_battery_percentage(void);
extern int android_system_booting_finished;

static int tcc_cpufreq_is_limit_highspeed_status(void)
{
	if ( 0
	  || tcc_battery_get_charging_status() == POWER_SUPPLY_STATUS_CHARGING 	/* check charging status */
	  || tcc_battery_get_charging_status() == POWER_SUPPLY_STATUS_FULL		/* check charging status */
	  || tcc_freq_limit_table[TCC_FREQ_LIMIT_OVERCLOCK].usecount	/* check 3d gallery (from app.) */
	  || tcc_freq_limit_table[TCC_FREQ_LIMIT_VPU_ENC].usecount		/* check video encoding status */
	  || tcc_freq_limit_table[TCC_FREQ_LIMIT_VPU_DEC].usecount		/* check video decoding status */
	  || tcc_freq_limit_table[TCC_FREQ_LIMIT_CAMERA].usecount		/* check camera active status */
	  || tcc_battery_percentage() < 50			/* check battery level */
	  || android_system_booting_finished == 0	/* check boot complete */
	  || cpu_highspeed_status == CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED
//	  || tcc_freq_limit_table[TCC_FREQ_LIMIT_FB].usecount == 0		/* lcd off status */
	) {
		return 1;
	}

	return 0;
}

static void cpufreq_work_func(struct work_struct *work)
{
	dbg_highspeed("######### %s\n", __func__);

	mutex_lock(&tcc_freq_mutex);

	if (cpu_highspeed_status == CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED) {
		tcc_freq_curr_limit_table.cpu_freq = TCC_CPU_FREQ_NORMAL_SPEED;
	}
	else if(cpu_highspeed_status == CPU_FREQ_PROCESSING_HIGHSPEED) {
		if (tcc_cpufreq_is_limit_highspeed_status())
			tcc_freq_curr_limit_table.cpu_freq = TCC_CPU_FREQ_NORMAL_SPEED;
		else
			tcc_freq_curr_limit_table.cpu_freq = TCC_CPU_FREQ_HIGH_SPEED;
	}
	tcc_cpufreq_set_clock_table(&tcc_freq_curr_limit_table);
	mutex_unlock(&tcc_freq_mutex);

	cpufreq_update_policy(0);
}

static void highspeed_reset_setting_values(void)
{
 	highspeed_highclock_flag = 0;
	highspeed_highclock_counter = 0;
	highspeed_lowclock_flag = 0;
	highspeed_lowclock_counter = 0;
	highspeed_limit_highspeed_counter = 0;
	cpu_highspeed_status = CPU_FREQ_PROCESSING_NORMAL;
	tcc_freq_curr_limit_table.cpu_freq = tcc_freq_curr_target_cpufreq;
}

static void highspeed_timer_func(unsigned long data)
{
	int status_changed = 0;

	// increase counters
	 if (highspeed_highclock_flag && (tcc_freq_curr_limit_table.cpu_freq > TCC_CPU_FREQ_NORMAL_SPEED))
		highspeed_highclock_counter++;
	if (highspeed_lowclock_flag && (tcc_freq_curr_limit_table.cpu_freq <= TCC_CPU_FREQ_NORMAL_SPEED))
		highspeed_lowclock_counter++;
	if (cpu_highspeed_status == CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED)
		highspeed_limit_highspeed_counter++;

	// start overclock process
	if (cpu_highspeed_status == CPU_FREQ_PROCESSING_NORMAL && tcc_freq_curr_limit_table.cpu_freq > TCC_CPU_FREQ_NORMAL_SPEED) {
		cpu_highspeed_status = CPU_FREQ_PROCESSING_HIGHSPEED;
		highspeed_highclock_flag = 1;
		highspeed_highclock_counter = 0;
		highspeed_lowclock_flag = 1;
		highspeed_lowclock_counter = 0;
		dbg_highspeed("######### Change to highspeed status\n");
	}

	// during over clock status
	else if (cpu_highspeed_status == CPU_FREQ_PROCESSING_HIGHSPEED) {
		if (highspeed_highclock_counter >= HIGHSPEED_HIGER_CLOCK_PERIOD && highspeed_lowclock_counter < HIGHSPEED_HIGER_CLOCK_PERIOD) {
			cpu_highspeed_status = CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED;
			highspeed_limit_highspeed_counter = 0;
			dbg_highspeed("######### Change to limit highspeed status\n");
			status_changed = 1;
		}
		else if (highspeed_lowclock_counter >= HIGHSPEED_HIGER_CLOCK_PERIOD) {
			highspeed_reset_setting_values();
			dbg_highspeed("######### Change to normal status\n");
			status_changed = 1;
 		}
	}

	// during limit over clock status
	else if (cpu_highspeed_status == CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED) {
		if (highspeed_limit_highspeed_counter >= HIGHSPEED_LIMIT_CLOCK_PERIOD) {
			highspeed_reset_setting_values();
			dbg_highspeed("######### Change to highspeed status\n");
			status_changed = 1;
		}
 	}

	// unkown status
	else if (cpu_highspeed_status != CPU_FREQ_PROCESSING_NORMAL) {
		highspeed_reset_setting_values();
		dbg_highspeed("######### Change to normal status (dummy)\n");
		status_changed = 1;
	}

	if (status_changed)
		queue_work(cpufreq_wq, &cpufreq_work);

     timer_highspeed.expires = jiffies + msecs_to_jiffies(HIGHSPEED_TIMER_TICK);	// 100 milisec.
    add_timer(&timer_highspeed);
}
#endif


/**
 * Get current CPU frequency in kHz
 */
static unsigned int tcc_cpufreq_get(unsigned int cpu)
{
	return clk_get_rate(cpu_clk) / 1000;
}

static int tcc_cpufreq_set_voltage_by_gpio(int uV)
{
	volatile PGPIO  pGPIO;
	pGPIO = (PGPIO)tcc_p2v(HwGPIO_BASE);

	if (curr_voltage > uV)
		udelay(200);

	if (machine_is_m57te() || machine_is_m801()) {	
		BITCSET(pGPIO->GPDEN, Hw24|Hw25, Hw24|Hw25);	// VCORE_CTL0, VCORE_CTL1
	
#if defined(CONFIG_CPU_HIGHSPEED)
		if (uV <= 1100000)
			BITCSET(pGPIO->GPDDAT, Hw24|Hw25, 0);
	#if (TCC_CPU_FREQ_NORMAL_SPEED == 720000)
		else if (uV <= 1420000)
	#elif (TCC_CPU_FREQ_NORMAL_SPEED == 660000)
		else if (uV <= 1360000)
	#else
		#error Not defind 720MHz or 660MHz
	#endif
			BITCSET(pGPIO->GPDDAT, Hw24|Hw25, Hw25);
		else
			BITCSET(pGPIO->GPDDAT, Hw24|Hw25, Hw24);
#else
		if (uV <= 1100000)
			BITCSET(pGPIO->GPDDAT, Hw24|Hw25, 0);
		else if (uV <= 1200000)
			BITCSET(pGPIO->GPDDAT, Hw24|Hw25, Hw25);
		else
			BITCSET(pGPIO->GPDDAT, Hw24|Hw25, Hw24);
#endif
	}
	else if (machine_is_tcc8900()) {
		BITCSET(pGPIO->GPDEN, Hw24, Hw24);	// VCORE_CTL
		
#if defined(CONFIG_CPU_HIGHSPEED)
	#if (TCC_CPU_FREQ_NORMAL_SPEED == 720000)
		if (uV <= 1420000)
	#elif (TCC_CPU_FREQ_NORMAL_SPEED == 660000)
		if (uV <= 1360000)
	#else
		#error Not defind 720MHz or 660MHz
	#endif
			BITCSET(pGPIO->GPDDAT, Hw24, 0);
		else
			BITCSET(pGPIO->GPDDAT, Hw24, Hw24);
#else
		if (uV <= 1250000)
			BITCSET(pGPIO->GPDDAT, Hw24, 0);
		else
			BITCSET(pGPIO->GPDDAT, Hw24, Hw24);
#endif
	}

	if (curr_voltage < uV)
		udelay(200);

	return 0;
}

static int tcc_cpufreq_set_voltage(int uV)
{
	int ret = 0;

#if defined(CONFIG_REGULATOR)
	if (vdd_core)
		ret = regulator_set_voltage(vdd_core, uV, uV);
#else
	ret = tcc_cpufreq_set_voltage_by_gpio(uV);
#endif
	if (ret == 0)
		curr_voltage = uV;
	
	return ret;
}

int tcc_cpufreq_get_voltage(void)
{
	int ret = 0;

#if defined(CONFIG_REGULATOR)
	if (vdd_core)
		ret = regulator_get_voltage(vdd_core);
#endif

	return ret;
}

/****************************************************
unsigned int tcc_get_maximum_io_clock(void)
	return : io clock (kHz)
****************************************************/
unsigned int tcc_get_maximum_io_clock(void)
{
	return gtClockLimitTable[NUM_FREQS-1].io_freq;
}
EXPORT_SYMBOL(tcc_get_maximum_io_clock);

static unsigned int tcc_cpufreq_get_pll_table(unsigned int cpu_clk)
{
	int i;

	for (i=0 ; i<NUM_PLLS ; i++) {
		if (tcc_pll_table[i].cpu_freq >= cpu_clk) {
			i++;
			break;
		}
	}

	return tcc_pll_table[i-1].pll_freq;
}


static int tcc_cpufreq_get_voltage_table(struct tcc_freq_table_t *clk_tbl)
{
	int i;
	
	for (i=0 ; i<NUM_VOLTAGES ; i++) {
		if (tcc_voltage_table[i].cpu_freq  >= clk_tbl->cpu_freq  &&
			tcc_voltage_table[i].ddi_freq  >= clk_tbl->ddi_freq  &&
			tcc_voltage_table[i].mem_freq  >= clk_tbl->mem_freq  &&
			tcc_voltage_table[i].gpu_freq  >= clk_tbl->gpu_freq  &&
			tcc_voltage_table[i].io_freq   >= clk_tbl->io_freq   &&
			tcc_voltage_table[i].vbus_freq >= clk_tbl->vbus_freq &&
			tcc_voltage_table[i].vcod_freq >= clk_tbl->vcod_freq &&
			tcc_voltage_table[i].smu_freq  >= clk_tbl->smu_freq)
			break;
	}

	if (i >= NUM_VOLTAGES)
		i = NUM_VOLTAGES-1;

	return (tcc_voltage_table[i].voltage);
}

static int tcc_cpufreq_set_clock_table(struct tcc_freq_table_t *curr_clk_tbl)
{
	int new_voltage;
	int ret = 0;
	
	new_voltage = tcc_cpufreq_get_voltage_table(curr_clk_tbl);

#if defined(CONFIG_CPU_HIGHSPEED)
#if (DEBUG_HIGHSPEED)
	if (tcc_freq_old_table.cpu_freq > TCC_CPU_FREQ_NORMAL_SPEED && curr_clk_tbl->cpu_freq <= TCC_CPU_FREQ_NORMAL_SPEED ) {
		dbg_highspeed("change to %dMHz\n", TCC_CPU_FREQ_NORMAL_SPEED/1000);
	}
	else if (tcc_freq_old_table.cpu_freq <= TCC_CPU_FREQ_NORMAL_SPEED && curr_clk_tbl->cpu_freq > TCC_CPU_FREQ_NORMAL_SPEED ) {
		dbg_highspeed("change to %dMHz\n", TCC_CPU_FREQ_HIGH_SPEED/1000);
	}
#endif
#endif

//	printk("cpufreq: cpu:%u, ddi:%u, mem:%u, gpu:%u,\n", curr_clk_tbl->cpu_freq, curr_clk_tbl->ddi_freq, curr_clk_tbl->mem_freq, curr_clk_tbl->gpu_freq);
//	printk("         io :%u, vbu:%u, vco:%u, vol:%d, tbl:%x\n", curr_clk_tbl->io_freq, curr_clk_tbl->vbus_freq, curr_clk_tbl->vcod_freq, new_voltage, tcc_limitclocktbl_flag);
	
	if (new_voltage > curr_voltage) {
		ret = tcc_cpufreq_set_voltage(new_voltage);
		if (ret != 0) {
			pr_err("cpufreq: regulator_set_voltage failed\n");
			return ret;;
		}
	}

	tcc_nand_lock();

#if defined(CONFIG_VBUS_280MHZ_USE)
	if ((curr_clk_tbl->cpu_freq <= 660000) && (curr_clk_tbl->mem_freq == 330000 || curr_clk_tbl->mem_freq == 325000) && (
			tcc_freq_limit_table[TCC_FREQ_LIMIT_VPU_DEC].usecount || tcc_freq_limit_table[TCC_FREQ_LIMIT_VPU_ENC].usecount
		 || tcc_freq_limit_table[TCC_FREQ_LIMIT_JPEG].usecount)) {
		curr_clk_tbl->mem_freq = 325000;	// change mem_freq 330MHz to 325MHz, for check pll source.
		clk_set_rate(pll0_clk, tcc_cpufreq_get_pll_table(660000) * 1000);
		clk_set_rate(mem_clk, curr_clk_tbl->mem_freq * 1000);
		if (tcc_freq_old_table.mem_freq != curr_clk_tbl->mem_freq) {
			if (tcc_freq_old_table.ddi_freq == 330000)
				clk_set_rate(ddi_clk, 216000 * 1000);
			if (tcc_freq_old_table.vbus_freq == 330000)
				clk_set_rate(vbus_clk, 216000 * 1000);
		}
		clk_set_rate(cpu_clk, curr_clk_tbl->cpu_freq * 1000);
	}
	else {
		if (curr_clk_tbl->mem_freq == 325000)
			curr_clk_tbl->mem_freq = 330000;
		clk_set_rate(mem_clk, curr_clk_tbl->mem_freq * 1000);
		if (tcc_freq_old_table.mem_freq != curr_clk_tbl->mem_freq) {
			if (tcc_freq_old_table.ddi_freq == 330000)
				clk_set_rate(ddi_clk, 216000 * 1000);
			if (tcc_freq_old_table.vbus_freq == 330000)
				clk_set_rate(vbus_clk, 216000 * 1000);
		}
		clk_set_rate(pll0_clk, tcc_cpufreq_get_pll_table(curr_clk_tbl->cpu_freq) * 1000);
		clk_set_rate(cpu_clk, curr_clk_tbl->cpu_freq * 1000);
	}
#else
	clk_set_rate(pll0_clk, tcc_cpufreq_get_pll_table(curr_clk_tbl->cpu_freq) * 1000);
	clk_set_rate(cpu_clk, curr_clk_tbl->cpu_freq * 1000);
	clk_set_rate(mem_clk, curr_clk_tbl->mem_freq * 1000);

	if (tcc_freq_old_table.mem_freq != curr_clk_tbl->mem_freq) {
		if (tcc_freq_old_table.ddi_freq == 330000)
			clk_set_rate(ddi_clk, 216000 * 1000);
		if (tcc_freq_old_table.vbus_freq == 330000)
			clk_set_rate(vbus_clk, 216000 * 1000);
	}
#endif
	clk_set_rate(io_clk, curr_clk_tbl->io_freq * 1000);
//	clk_set_rate(smu_clk, curr_clk_tbl->smu_freq * 1000);
	clk_set_rate(ddi_clk, curr_clk_tbl->ddi_freq * 1000);
	clk_set_rate(gpu_clk, curr_clk_tbl->gpu_freq * 1000);
	clk_set_rate(vcod_clk, curr_clk_tbl->vcod_freq * 1000);
	clk_set_rate(vbus_clk, curr_clk_tbl->vbus_freq * 1000);

	tcc_nand_unlock();

	if (new_voltage < curr_voltage) {
		ret = tcc_cpufreq_set_voltage(new_voltage);
	}

	memcpy(&tcc_freq_old_table, curr_clk_tbl, sizeof(struct tcc_freq_table_t));

	return ret;
}

static int startup_cpufreq = 0;
/* idx: part idx (camera(0), vpu(1), jpge(2), hdmi(3), mali(4), ...)   flag : 1(TRUE) 0(FALSE) */
int tcc_cpufreq_set_limit_table(struct tcc_freq_table_t *limit_tbl, tcc_freq_limit_idx_t idx, int flag)
{
	if (idx >= TCC_FREQ_LIMIT_MAX)
		return -1;

	if (tcc_freq_mutex_init_flag == 0) {
		tcc_freq_mutex_init_flag = 1;
		mutex_init(&tcc_freq_mutex);
	}
	mutex_lock(&tcc_freq_mutex);

#if defined(CONFIG_CPU_HIGHSPEED)
	if (idx == TCC_FREQ_LIMIT_OVERCLOCK) {
		if (flag) {
			tcc_freq_limit_table[idx].usecount++;
			if (tcc_freq_curr_limit_table.cpu_freq > TCC_CPU_FREQ_NORMAL_SPEED)
				tcc_freq_curr_limit_table.cpu_freq = TCC_CPU_FREQ_NORMAL_SPEED;

			tcc_limitclocktbl_flag |= (1<<idx);
			tcc_cpufreq_set_clock_table(&tcc_freq_curr_limit_table);
		}
		else {
			tcc_limitclocktbl_flag &= ~(1<<idx);
			tcc_freq_limit_table[idx].usecount = 0;
		}
		mutex_unlock(&tcc_freq_mutex);
		return 0;
	}
#endif

	if (flag) {
		memcpy(&(tcc_freq_limit_table[idx].freq), limit_tbl, sizeof(struct tcc_freq_table_t));
		tcc_freq_limit_table[idx].usecount++;

		tcc_limitclocktbl_flag |= (1<<idx);
		pr_debug("tcc_cpufreq_set_limit_table idx:%d, cnt:%d\n", idx, tcc_freq_limit_table[idx].usecount);
	}
	else {
		#if 1
		memset(&(tcc_freq_limit_table[idx].freq), 0x0, sizeof(struct tcc_freq_table_t));
		tcc_freq_limit_table[idx].usecount = 0;
		#else
		if (tcc_freq_limit_table[idx].usecount == 0) {
			mutex_unlock(&tcc_freq_mutex);
			return -1;
		}

		if (tcc_freq_limit_table[idx].usecount == 1) {
			memset(&(tcc_freq_limit_table[idx].freq), 0x0, sizeof(struct tcc_freq_table_t));
			tcc_freq_limit_table[idx].usecount--;
		}
		#endif

		tcc_limitclocktbl_flag &= ~(1<<idx);
		pr_debug("tcc_cpufreq_set_limit_table idx:%d, cnt:%d\n", idx, tcc_freq_limit_table[idx].usecount);
	}

	if (flag && startup_cpufreq) {
		if (tcc_freq_limit_table[idx].freq.cpu_freq > tcc_freq_curr_limit_table.cpu_freq)
			tcc_freq_curr_limit_table.cpu_freq = tcc_freq_limit_table[idx].freq.cpu_freq;
		if (tcc_freq_limit_table[idx].freq.ddi_freq > tcc_freq_curr_limit_table.ddi_freq)
			tcc_freq_curr_limit_table.ddi_freq = tcc_freq_limit_table[idx].freq.ddi_freq;
		if (tcc_freq_limit_table[idx].freq.mem_freq > tcc_freq_curr_limit_table.mem_freq)
			tcc_freq_curr_limit_table.mem_freq = tcc_freq_limit_table[idx].freq.mem_freq;
		if (tcc_freq_limit_table[idx].freq.gpu_freq > tcc_freq_curr_limit_table.gpu_freq)
			tcc_freq_curr_limit_table.gpu_freq = tcc_freq_limit_table[idx].freq.gpu_freq;
		if (tcc_freq_limit_table[idx].freq.io_freq > tcc_freq_curr_limit_table.io_freq)
			tcc_freq_curr_limit_table.io_freq = tcc_freq_limit_table[idx].freq.io_freq;
		if (tcc_freq_limit_table[idx].freq.vbus_freq > tcc_freq_curr_limit_table.vbus_freq)
			tcc_freq_curr_limit_table.vbus_freq = tcc_freq_limit_table[idx].freq.vbus_freq;
		if (tcc_freq_limit_table[idx].freq.vcod_freq > tcc_freq_curr_limit_table.vcod_freq)
			tcc_freq_curr_limit_table.vcod_freq = tcc_freq_limit_table[idx].freq.vcod_freq;
		if (tcc_freq_limit_table[idx].freq.smu_freq > tcc_freq_curr_limit_table.smu_freq)
			tcc_freq_curr_limit_table.smu_freq = tcc_freq_limit_table[idx].freq.smu_freq;

		if (cpufreq_is_performace_governor() == 0) {
			// memory clock fixed rate
			if (tcc_freq_limit_table[TCC_FREQ_LIMIT_HDMI].usecount && tcc_freq_limit_table[TCC_FREQ_LIMIT_HDMI].freq.mem_freq)
				tcc_freq_curr_limit_table.mem_freq = tcc_freq_limit_table[TCC_FREQ_LIMIT_HDMI].freq.mem_freq;
			else if (tcc_freq_limit_table[TCC_FREQ_LIMIT_CAMERA].usecount) {
				if(tcc_freq_limit_table[TCC_FREQ_LIMIT_V2IP].usecount == 0)
					tcc_freq_curr_limit_table.mem_freq = tcc_freq_limit_table[TCC_FREQ_LIMIT_CAMERA].freq.mem_freq;
                 }	

			// cpu clock fixed rate
			if (tcc_freq_limit_table[TCC_FREQ_LIMIT_TV].usecount)
				tcc_freq_curr_limit_table.cpu_freq = tcc_freq_limit_table[TCC_FREQ_LIMIT_TV].freq.cpu_freq;
		}

#if defined(CONFIG_CPU_HIGHSPEED)
		if (tcc_freq_curr_limit_table.cpu_freq > TCC_CPU_FREQ_NORMAL_SPEED) {
			if (tcc_cpufreq_is_limit_highspeed_status())
				tcc_freq_curr_limit_table.cpu_freq = TCC_CPU_FREQ_NORMAL_SPEED;
		}
#endif
		tcc_cpufreq_set_clock_table(&tcc_freq_curr_limit_table);
	}

	mutex_unlock(&tcc_freq_mutex);
	cpufreq_update_policy(0);
	return 0;
}
EXPORT_SYMBOL(tcc_cpufreq_set_limit_table);

static int tcc_cpufreq_target(struct cpufreq_policy *policy,
			      unsigned int target_freq,
			      unsigned int relation)
{
	unsigned int index, i, limit_tbl_flag;
	struct cpufreq_freqs freqs;
	int ret = 0;
	cpufreq_verify_within_limits(policy, policy->cpuinfo.min_freq,
		policy->cpuinfo.max_freq);

	if (target_freq < policy->cpuinfo.min_freq)
		target_freq = policy->cpuinfo.min_freq;
	if (target_freq < policy->min)
		target_freq = policy->min;

	if (tcc_freq_limit_table[TCC_FREQ_LIMIT_FB].usecount == 0) {
		if (policy->cur < 72000 && target_freq >= 364500)
			target_freq = 72000; //144000;
//		else if (policy->cur < 90000 && target_freq >= 364500)
//			target_freq = 90000;
//		else if (policy->cur < 108000 && target_freq >= 364500)
//			target_freq = 108000;
		else if (policy->cur < 126000 && target_freq >= 364500)
			target_freq = 126000;
		else if (policy->cur < 144000 && target_freq >= 364500)
			target_freq = 144000;
		else if (policy->cur < 294000 && target_freq >= 364500)
			target_freq = 294000;
		else if (policy->cur < 364500 && target_freq >= 364500)
			target_freq = 364500;
	}

#if defined(CONFIG_CPU_HIGHSPEED)
	if (target_freq > TCC_CPU_FREQ_NORMAL_SPEED) {
		if (tcc_cpufreq_is_limit_highspeed_status())
			target_freq = TCC_CPU_FREQ_NORMAL_SPEED;
	}
#endif

	if (cpufreq_frequency_table_target(policy, tcc_cpufreq_table,
					   target_freq, relation, &index)) {
		return -EINVAL;
	}

	mutex_lock(&tcc_freq_mutex);

	memcpy(&tcc_freq_curr_limit_table, &(gtClockLimitTable[index]), sizeof(struct tcc_freq_table_t));
#if defined(CONFIG_CPU_HIGHSPEED)
	tcc_freq_curr_target_cpufreq = tcc_freq_curr_limit_table.cpu_freq;
#endif

	limit_tbl_flag = 0;
	for (i=0 ; i<TCC_FREQ_LIMIT_MAX ; i++) {
		if (tcc_freq_limit_table[i].usecount > 0) {
			if (tcc_freq_limit_table[i].freq.cpu_freq > tcc_freq_curr_limit_table.cpu_freq)
				tcc_freq_curr_limit_table.cpu_freq = tcc_freq_limit_table[i].freq.cpu_freq;
			if (tcc_freq_limit_table[i].freq.ddi_freq > tcc_freq_curr_limit_table.ddi_freq)
				tcc_freq_curr_limit_table.ddi_freq = tcc_freq_limit_table[i].freq.ddi_freq;
			if (tcc_freq_limit_table[i].freq.mem_freq > tcc_freq_curr_limit_table.mem_freq)
				tcc_freq_curr_limit_table.mem_freq = tcc_freq_limit_table[i].freq.mem_freq;
			if (tcc_freq_limit_table[i].freq.gpu_freq > tcc_freq_curr_limit_table.gpu_freq)
				tcc_freq_curr_limit_table.gpu_freq = tcc_freq_limit_table[i].freq.gpu_freq;
			if (tcc_freq_limit_table[i].freq.io_freq > tcc_freq_curr_limit_table.io_freq)
				tcc_freq_curr_limit_table.io_freq = tcc_freq_limit_table[i].freq.io_freq;
			if (tcc_freq_limit_table[i].freq.vbus_freq > tcc_freq_curr_limit_table.vbus_freq)
				tcc_freq_curr_limit_table.vbus_freq = tcc_freq_limit_table[i].freq.vbus_freq;
			if (tcc_freq_limit_table[i].freq.vcod_freq > tcc_freq_curr_limit_table.vcod_freq)
				tcc_freq_curr_limit_table.vcod_freq = tcc_freq_limit_table[i].freq.vcod_freq;
			if (tcc_freq_limit_table[i].freq.smu_freq > tcc_freq_curr_limit_table.smu_freq)
				tcc_freq_curr_limit_table.smu_freq = tcc_freq_limit_table[i].freq.smu_freq;

			limit_tbl_flag |= (1<<i);
		}
	}
	tcc_limitclocktbl_flag = limit_tbl_flag;

	if (cpufreq_is_performace_governor() == 0) {
		// memory clock fixed rate
		if (tcc_freq_limit_table[TCC_FREQ_LIMIT_HDMI].usecount && tcc_freq_limit_table[TCC_FREQ_LIMIT_HDMI].freq.mem_freq)
			tcc_freq_curr_limit_table.mem_freq = tcc_freq_limit_table[TCC_FREQ_LIMIT_HDMI].freq.mem_freq;
		else if (tcc_freq_limit_table[TCC_FREQ_LIMIT_CAMERA].usecount) {
			#if 0
			if (tcc_freq_limit_table[TCC_FREQ_LIMIT_VPU_ENC].usecount && tcc_freq_limit_table[TCC_FREQ_LIMIT_VPU_ENC].freq.mem_freq)
				tcc_freq_curr_limit_table.mem_freq = tcc_freq_limit_table[TCC_FREQ_LIMIT_VPU_ENC].freq.mem_freq;
			else if (tcc_freq_limit_table[TCC_FREQ_LIMIT_CAMERA].freq.mem_freq)
			#endif
			if(tcc_freq_limit_table[TCC_FREQ_LIMIT_V2IP].usecount == 0)
				tcc_freq_curr_limit_table.mem_freq = tcc_freq_limit_table[TCC_FREQ_LIMIT_CAMERA].freq.mem_freq;
		}

		// cpu clock fixed rate
		if (tcc_freq_limit_table[TCC_FREQ_LIMIT_TV].usecount)
			tcc_freq_curr_limit_table.cpu_freq = tcc_freq_limit_table[TCC_FREQ_LIMIT_TV].freq.cpu_freq;
	}

#if defined(CONFIG_CPU_HIGHSPEED)
	if (tcc_freq_curr_limit_table.cpu_freq > TCC_CPU_FREQ_NORMAL_SPEED) {
		if (tcc_cpufreq_is_limit_highspeed_status())
			tcc_freq_curr_limit_table.cpu_freq = TCC_CPU_FREQ_NORMAL_SPEED;
	}
#endif

	freqs.old = policy->cur;
	freqs.new = tcc_freq_curr_limit_table.cpu_freq;
	freqs.cpu = 0;

	cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

	pr_debug("cpufreq: changing clk to %u; target = %u, oldfreq = %u, mem = %u, mask = 0x%x\n",
		freqs.new, target_freq, freqs.old, tcc_freq_curr_limit_table.mem_freq, limit_tbl_flag);

	ret = tcc_cpufreq_set_clock_table(&tcc_freq_curr_limit_table);
	mutex_unlock(&tcc_freq_mutex);

	startup_cpufreq = 1;

	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

	return ret;
}

static int tcc_cpufreq_verify(struct cpufreq_policy *policy)
{
	if (policy->cpu != 0)
		return -EINVAL;

	return cpufreq_frequency_table_verify(policy, tcc_cpufreq_table);
}

static int tcc_cpufreq_suspend(struct cpufreq_policy *policy, pm_message_t pmsg)
{
	unsigned int memory_clock;
	struct tcc_freq_table_t *freqs = &gtClockLimitTable[0];

#ifdef CONFIG_DRAM_DDR2		// keep resume/suspend bug on samsung ddr2
	memory_clock = 166000;
#else
	memory_clock = freqs->mem_freq;
#endif

#if defined(CONFIG_CPU_HIGHSPEED)
	//del_timer(&timer_highspeed);
	highspeed_reset_setting_values();
#endif

	clk_set_rate(mem_clk, memory_clock * 1000);		
	/* Sets minimum CPU frequency and voltage when suspend */
	clk_set_rate(pll0_clk, tcc_cpufreq_get_pll_table(freqs->cpu_freq) * 1000);
	clk_set_rate(cpu_clk, freqs->cpu_freq * 1000);
	clk_set_rate(io_clk, freqs->io_freq * 1000);
//	clk_set_rate(smu_clk, freqs->smu_freq * 1000);
	clk_set_rate(ddi_clk, freqs->ddi_freq * 1000);
	clk_set_rate(gpu_clk, freqs->gpu_freq * 1000);

	return 0;
}

static int tcc_cpufreq_resume(struct cpufreq_policy *policy)
{
	struct tcc_freq_table_t freqs;
	int ret;
#if defined(CONFIG_REGULATOR)
	int i;

	for (i = NUM_FREQS-1 ; i >= 0 ; i--) {
		if ( gtClockLimitTable[i].cpu_freq  <= tcc_voltage_table[0].cpu_freq &&
			 gtClockLimitTable[i].ddi_freq  <= tcc_voltage_table[0].ddi_freq &&
			 gtClockLimitTable[i].mem_freq  <= tcc_voltage_table[0].mem_freq &&
			 gtClockLimitTable[i].gpu_freq  <= tcc_voltage_table[0].gpu_freq &&
			 gtClockLimitTable[i].io_freq   <= tcc_voltage_table[0].io_freq &&
			 gtClockLimitTable[i].vbus_freq <= tcc_voltage_table[0].vbus_freq &&
			 gtClockLimitTable[i].vcod_freq <= tcc_voltage_table[0].vcod_freq )
			 break;
	}
	memcpy(&freqs, &gtClockLimitTable[i], sizeof(struct tcc_freq_table_t));
#else
	memcpy(&freqs, &gtClockLimitTable[NUM_FREQS - 1], sizeof(struct tcc_freq_table_t));
#endif

#if defined(CONFIG_CPU_HIGHSPEED)
	//timer_highspeed.expires = jiffies + msecs_to_jiffies(HIGHSPEED_TIMER_TICK);	// 100 milisec.
	//add_timer(&timer_highspeed);

	if (freqs.cpu_freq > TCC_CPU_FREQ_NORMAL_SPEED)
		freqs.cpu_freq = TCC_CPU_FREQ_NORMAL_SPEED;
#endif

#if !defined(CONFIG_REGULATOR)
	ret = tcc_cpufreq_set_voltage(tcc_cpufreq_get_voltage_table(&freqs));
	if (ret != 0) {
		pr_err("cpufreq: regulator_set_voltage failed\n");
		return -1;
	}
#endif

	clk_set_rate(pll0_clk, tcc_cpufreq_get_pll_table(freqs.cpu_freq) * 1000);
	clk_set_rate(cpu_clk, freqs.cpu_freq * 1000);
	clk_set_rate(mem_clk, freqs.mem_freq * 1000);
	clk_set_rate(io_clk, freqs.io_freq * 1000);
//	clk_set_rate(smu_clk, freqs.smu_freq * 1000);
	clk_set_rate(ddi_clk, freqs.ddi_freq * 1000);
	clk_set_rate(gpu_clk, freqs.gpu_freq * 1000);

	memcpy(&tcc_freq_old_table, &freqs, sizeof(struct tcc_freq_table_t));
	return 0;
}

static int __init tcc_cpufreq_init(struct cpufreq_policy *policy)
{
	int i;

#if defined(CONFIG_CPU_HIGHSPEED)
	INIT_WORK(&cpufreq_work, cpufreq_work_func);
#endif

	if (policy->cpu != 0)
		return -EINVAL;

	pll0_clk = clk_get(NULL, "pll0");
	if (IS_ERR(pll0_clk))
		return PTR_ERR(pll0_clk);
	cpu_clk = clk_get(NULL, "cpu");
	if (IS_ERR(cpu_clk))
		return PTR_ERR(cpu_clk);
	io_clk = clk_get(NULL, "iobus");
	if (IS_ERR(io_clk))
		return PTR_ERR(io_clk);	
	smu_clk = clk_get(NULL, "smu");
	if (IS_ERR(smu_clk))
		return PTR_ERR(smu_clk);	
	mem_clk = clk_get(NULL, "membus");
	if (IS_ERR(mem_clk))
		return PTR_ERR(mem_clk);
	ddi_clk = clk_get(NULL, "ddi");
	if (IS_ERR(ddi_clk))
		return PTR_ERR(ddi_clk);
	gpu_clk = clk_get(NULL, "gpu");
		if (IS_ERR(gpu_clk))
			return PTR_ERR(gpu_clk);
	vcod_clk = clk_get(NULL, "vcore");
	if (IS_ERR(vcod_clk))
		return PTR_ERR(vcod_clk);
	vbus_clk = clk_get(NULL, "vbus");
		if (IS_ERR(vbus_clk))
			return PTR_ERR(vbus_clk);

	for (i = 0; i < NUM_FREQS; i++) {
		tcc_cpufreq_table[i].index = i;
		tcc_cpufreq_table[i].frequency = gtClockLimitTable[i].cpu_freq;
	}
	tcc_cpufreq_table[i].frequency = CPUFREQ_TABLE_END;

	policy->cur = policy->min = policy->max = clk_get_rate(cpu_clk);
	policy->cpuinfo.min_freq = tcc_cpufreq_table[0].frequency;
	policy->cpuinfo.max_freq = tcc_cpufreq_table[i - 1].frequency;
	policy->cpuinfo.transition_latency = 
		TCC_TRANSITION_LATENCY * NSEC_PER_USEC;

	cpufreq_frequency_table_cpuinfo(policy, tcc_cpufreq_table);

#if defined(CONFIG_CPU_HIGHSPEED)
    init_timer(&timer_highspeed);
    timer_highspeed.data = 0;
    timer_highspeed.function = highspeed_timer_func;
    timer_highspeed.expires = jiffies + msecs_to_jiffies(HIGHSPEED_TIMER_TICK);	// 100 milisec.
    add_timer(&timer_highspeed);
#endif

#ifdef CONFIG_REGULATOR
	if (vdd_core == NULL) {
		vdd_core = regulator_get(NULL, "vdd_core");
		if (IS_ERR(vdd_core)) {
			pr_warning("cpufreq: failed to obtain vdd_core\n");
			vdd_core = NULL;
		}
	}
#endif

	if (tcc_freq_mutex_init_flag == 0) {
		tcc_freq_mutex_init_flag = 1;
		mutex_init(&tcc_freq_mutex);
	}

	printk(KERN_INFO "TCC cpufreq driver initialized\n");
	return 0;
}

static struct cpufreq_driver tcc_cpufreq_driver = {
	.name	= "tcc",
	.owner	= THIS_MODULE,
	.flags	= CPUFREQ_STICKY | CPUFREQ_CONST_LOOPS,
	.init	= tcc_cpufreq_init,
	.verify	= tcc_cpufreq_verify,
	.target	= tcc_cpufreq_target,
	.get	= tcc_cpufreq_get,
	.suspend = tcc_cpufreq_suspend,
	.resume	= tcc_cpufreq_resume,
};

static int __init tcc_cpufreq_register(void)
{
#if defined(CONFIG_CPU_HIGHSPEED)
	cpufreq_wq = create_singlethread_workqueue("cpufreq_wq");
	if (!cpufreq_wq)
		return -ENOMEM;
#endif

	return cpufreq_register_driver(&tcc_cpufreq_driver);
}

static void __exit tcc_cpufreq_exit(void)
{
	cpufreq_unregister_driver(&tcc_cpufreq_driver);
}

MODULE_AUTHOR("Telechips, Inc.");
MODULE_DESCRIPTION("CPU frequency scaling driver for TCC92xx");

late_initcall(tcc_cpufreq_register);
module_exit(tcc_cpufreq_exit);
