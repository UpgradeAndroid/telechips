/*
 * arch/arm/mach-tcc93xx/cpufreq.c
 *
 * TCC92xx cpufreq driver
 *
 * Copyright (C) 2010 Telechips, Inc.
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
#include <linux/power_supply.h>
#include <plat/nand.h>

#define TCC_TRANSITION_LATENCY		100

#if defined(CONFIG_DRAM_DDR2)
#define TCC_CPU_FREQ_ONDEMAND_MAX	800000
#endif

#if defined(CONFIG_DRAM_MDDR)
#include "tcc_clocktbl_lpddr.h"
#elif defined(CONFIG_DRAM_DDR2)
#include "tcc_clocktbl_ddr2.h"
#elif defined(CONFIG_DRAM_DDR3)
#include "tcc_clocktbl_ddr3.h"
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
	unsigned int hsio_freq;
	unsigned int cam_freq;
	unsigned int dsub_freq;
	unsigned int voltage;
};

typedef enum {
	TCC_CORE_A = 0,
	TCC_CORE_B,
	TCC_CORE_MAX
} tcc_core_type;

static struct tcc_pll_table_t tcc_pll_table[] = {
	{ 146000, 146000 },
	{ 204000, 204000 },
	{ 304000, 304000 },
	{ 400000, 400000 },
	{ 500000, 500000 },
	{ 600000, 600000 },
	{ 700000, 700000 },
	{ 800000, 800000 },
	{ 900000, 900000 },
	{ 996000, 996000 },
};

static struct tcc_voltage_table_t tcc_voltage_table[] = {
	/*   cpu     ddi     mem     gpu      io    vbus    vcod     smu    hsio     cam   d-sub      vol */
	{ 300000, 140000, 140000, 100000,  40000, 130000, 110000,  20000,  80000, 130000,  80000, 1000000 },
	{ 375000, 180000, 180000, 120000,  80000, 170000, 150000,  60000, 120000, 170000, 120000, 1050000 },
	{ 450000, 220000, 220000, 140000, 120000, 210000, 190000, 100000, 160000, 210000, 160000, 1100000 },
	{ 525000, 260000, 260000, 160000, 160000, 250000, 230000, 140000, 200000, 250000, 200000, 1150000 },
	{ 600000, 300000, 300000, 180000, 200000, 290000, 270000, 180000, 240000, 290000, 240000, 1200000 },
	{ 675000, 340000, 340000, 200000, 240000, 330000, 310000, 220000, 280000, 330000, 280000, 1150000 },
	{ 750000, 380000, 380000, 220000, 280000, 370000, 350000, 260000, 320000, 370000, 320000, 1300000 },
	{ 825000, 420000, 420000, 240000, 320000, 410000, 390000, 300000, 360000, 410000, 360000, 1350000 },
	{ 996000, 420000, 420000, 240000, 384000, 410000, 390000, 300000, 360000, 410000, 360000, 1450000 },
};

static struct tcc_freq_table_t tcc_freq_old_table = {
	       0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0
};

extern struct tcc_freq_table_t gtClockLimitTable[];

struct tcc_freq_limit_table_t {
	struct tcc_freq_table_t freq;
	int usecount;
};

static struct tcc_freq_limit_table_t tcc_freq_limit_table[TCC_FREQ_LIMIT_MAX] = {{{0,0,0,0,0,0,0,0,0,0,0}, 0},};
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
static struct clk *hsio_clk;
static struct clk *cam_clk;
static struct clk *dsub_clk;

#ifdef CONFIG_REGULATOR
static struct regulator *vdd_coreA;
static struct regulator *vdd_coreB;
#endif
static int curr_coreA_voltage = 0;
static int curr_coreB_voltage = 0;

extern int cpufreq_is_performace_governor(void);

#if defined(CONFIG_CPU_HIGHSPEED)

#define TCC_CPU_FREQ_HIGH_SPEED			996000
#define HIGHSPEED_TIMER_TICK			1000		// 1000 ms.

static struct workqueue_struct *cpufreq_wq;
static struct work_struct cpufreq_work;
static struct timer_list timer_highspeed;
static int tcc_cpufreq_set_clock_table(struct tcc_freq_table_t *curr_clk_tbl);

extern int tcc_battery_get_charging_status(void);
extern int tcc_battery_percentage(void);
extern int android_system_booting_finished;

static void cpufreq_work_func(struct work_struct *work)
{
	mutex_lock(&tcc_freq_mutex);

	tcc_freq_curr_limit_table.cpu_freq = gtClockLimitTable[NUM_FREQS - 1].cpu_freq;
	tcc_freq_curr_limit_table.mem_freq = gtClockLimitTable[NUM_FREQS - 1].mem_freq;
	
	tcc_cpufreq_set_clock_table(&tcc_freq_curr_limit_table);
	mutex_unlock(&tcc_freq_mutex);

	cpufreq_update_policy(0);
}

static void highspeed_timer_func(unsigned long data)
{
	if (android_system_booting_finished) {
		queue_work(cpufreq_wq, &cpufreq_work);
	}
	else {
		timer_highspeed.expires = jiffies + msecs_to_jiffies(HIGHSPEED_TIMER_TICK);	// 100 milisec.
		add_timer(&timer_highspeed);
	}
}
#endif


/**
 * Get current CPU frequency in kHz
 */
static unsigned int tcc_cpufreq_get(unsigned int cpu)
{
	return clk_get_rate(cpu_clk) / 1000;
}

#if defined(CONFIG_GPIO_CORE_VOLTAGE_CONTROL)
static int tcc_cpufreq_set_voltage_by_gpio(tcc_core_type core, int uV)
{
	if (core == TCC_CORE_A) {
		if (curr_coreA_voltage > uV)
			udelay(200);

		if (curr_coreA_voltage < uV)
			udelay(200);
	}
	else if (core == TCC_CORE_B) {
		if (curr_coreB_voltage > uV)
			udelay(200);

		if (curr_coreB_voltage < uV)
			udelay(200);
	}

	return 0;
}
#endif

static int tcc_cpufreq_set_voltage(tcc_core_type core, int uV)
{
	int ret = 0;

#if defined(CONFIG_REGULATOR)
	if (vdd_coreA && (core == TCC_CORE_A))
		ret = regulator_set_voltage(vdd_coreA, uV, tcc_voltage_table[NUM_VOLTAGES-1].voltage);

	if (vdd_coreB && (core == TCC_CORE_B))
		ret = regulator_set_voltage(vdd_coreB, uV, tcc_voltage_table[NUM_VOLTAGES-1].voltage);
#elif defined(CONFIG_GPIO_CORE_VOLTAGE_CONTROL)
	ret = tcc_cpufreq_set_voltage_by_gpio(core, uV);
#endif
	if (ret == 0) {
		if (core == TCC_CORE_A)
			curr_coreA_voltage = uV;
		else if (core == TCC_CORE_B)
			curr_coreB_voltage = uV;
	}

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

static int tcc_cpufreq_get_voltage_table(tcc_core_type core, struct tcc_freq_table_t *clk_tbl)
{
	int i;

	if (core == TCC_CORE_A) {
		for (i=0 ; i<NUM_VOLTAGES ; i++) {
			if (tcc_voltage_table[i].cpu_freq  >= clk_tbl->cpu_freq  &&
				tcc_voltage_table[i].ddi_freq  >= clk_tbl->ddi_freq  &&
				tcc_voltage_table[i].mem_freq  >= clk_tbl->mem_freq  &&
				tcc_voltage_table[i].io_freq   >= clk_tbl->io_freq   &&
				tcc_voltage_table[i].smu_freq  >= clk_tbl->smu_freq  &&
				tcc_voltage_table[i].hsio_freq >= clk_tbl->hsio_freq &&
				tcc_voltage_table[i].cam_freq  >= clk_tbl->cam_freq  &&
				tcc_voltage_table[i].dsub_freq  >= clk_tbl->dsub_freq)
				break;
		}
	}
	else if (core == TCC_CORE_B) {
		for (i=0 ; i<NUM_VOLTAGES ; i++) {
			if (tcc_voltage_table[i].gpu_freq  >= clk_tbl->gpu_freq  &&
				tcc_voltage_table[i].vbus_freq >= clk_tbl->vbus_freq &&
				tcc_voltage_table[i].vcod_freq >= clk_tbl->vcod_freq)
				break;
		}
	}
	else
		i = NUM_VOLTAGES-1;

	if (i >= NUM_VOLTAGES)
		i = NUM_VOLTAGES-1;

	return (tcc_voltage_table[i].voltage);
}

static int tcc_cpufreq_set_clock_table(struct tcc_freq_table_t *curr_clk_tbl)
{
	int new_coreA_voltage, new_coreB_voltage;
	int ret = 0;

#if defined(CONFIG_CPU_HIGHSPEED)
	if (android_system_booting_finished == 0) {
		curr_clk_tbl->cpu_freq = TCC_CPU_FREQ_HIGH_SPEED;
		curr_clk_tbl->mem_freq = (TCC_CPU_FREQ_HIGH_SPEED/2);
	}
#endif

	new_coreA_voltage = tcc_cpufreq_get_voltage_table(TCC_CORE_A, curr_clk_tbl);
	new_coreB_voltage = tcc_cpufreq_get_voltage_table(TCC_CORE_B, curr_clk_tbl);

//	printk("cpufreq: cpu:%u, ddi:%u, mem:%u, gpu:%u, io:%u, vbu:%u\n", curr_clk_tbl->cpu_freq, curr_clk_tbl->ddi_freq, curr_clk_tbl->mem_freq, curr_clk_tbl->gpu_freq, curr_clk_tbl->io_freq, curr_clk_tbl->vbus_freq);
//	printk("         vco:%u, hsio:%u, cam:%u, coreA:%d, coreB:%d, tbl:%x\n", curr_clk_tbl->vcod_freq, curr_clk_tbl->hsio_freq, curr_clk_tbl->cam_freq, new_coreA_voltage, new_coreB_voltage, tcc_limitclocktbl_flag);
	
	if (new_coreA_voltage > curr_coreA_voltage) {
		ret = tcc_cpufreq_set_voltage(TCC_CORE_A, new_coreA_voltage);
		if (ret != 0) {
			pr_err("cpufreq: regulator_set_voltage failed\n");
			return ret;;
		}
	}
	if (new_coreB_voltage > curr_coreB_voltage) {
		ret = tcc_cpufreq_set_voltage(TCC_CORE_B, new_coreB_voltage);
		if (ret != 0) {
			pr_err("cpufreq: regulator_set_voltage failed\n");
			return ret;;
		}
	}

	tcc_nand_lock();

	clk_set_rate(pll0_clk, tcc_cpufreq_get_pll_table(curr_clk_tbl->cpu_freq) * 1000);
	clk_set_rate(cpu_clk, curr_clk_tbl->cpu_freq * 1000);
#if defined(CONFIG_MEM_CLK_SYNC_MODE) && defined(CONFIG_CPU_HIGHSPEED)
	if (android_system_booting_finished)
#endif
		clk_set_rate(mem_clk, curr_clk_tbl->mem_freq * 1000);
	clk_set_rate(io_clk, curr_clk_tbl->io_freq * 1000);
//	clk_set_rate(smu_clk, curr_clk_tbl->smu_freq * 1000);
	clk_set_rate(ddi_clk, curr_clk_tbl->ddi_freq * 1000);
	clk_set_rate(gpu_clk, curr_clk_tbl->gpu_freq * 1000);
	clk_set_rate(vcod_clk, curr_clk_tbl->vcod_freq * 1000);
	clk_set_rate(vbus_clk, curr_clk_tbl->vbus_freq * 1000);
	clk_set_rate(hsio_clk, curr_clk_tbl->hsio_freq * 1000);
	clk_set_rate(cam_clk, curr_clk_tbl->cam_freq * 1000);
	clk_set_rate(dsub_clk, curr_clk_tbl->dsub_freq * 1000);

	tcc_nand_unlock();

	if (new_coreA_voltage < curr_coreA_voltage) {
		ret = tcc_cpufreq_set_voltage(TCC_CORE_A, new_coreA_voltage);
	}
	if (new_coreB_voltage < curr_coreB_voltage) {
		ret = tcc_cpufreq_set_voltage(TCC_CORE_B, new_coreB_voltage);
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
		if (tcc_freq_limit_table[idx].freq.hsio_freq > tcc_freq_curr_limit_table.hsio_freq)
			tcc_freq_curr_limit_table.hsio_freq = tcc_freq_limit_table[idx].freq.hsio_freq;
		if (tcc_freq_limit_table[idx].freq.cam_freq > tcc_freq_curr_limit_table.cam_freq)
			tcc_freq_curr_limit_table.cam_freq = tcc_freq_limit_table[idx].freq.cam_freq;
		if (tcc_freq_limit_table[idx].freq.dsub_freq > tcc_freq_curr_limit_table.dsub_freq)
			tcc_freq_curr_limit_table.dsub_freq = tcc_freq_limit_table[idx].freq.dsub_freq;

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

	if (cpufreq_frequency_table_target(policy, tcc_cpufreq_table,
					   target_freq, relation, &index)) {
		return -EINVAL;
	}

	mutex_lock(&tcc_freq_mutex);

	if (index > NUM_FREQS-1)
		index = NUM_FREQS-1;
	memcpy(&tcc_freq_curr_limit_table, &(gtClockLimitTable[index]), sizeof(struct tcc_freq_table_t));

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
			if (tcc_freq_limit_table[i].freq.hsio_freq > tcc_freq_curr_limit_table.hsio_freq)
				tcc_freq_curr_limit_table.hsio_freq = tcc_freq_limit_table[i].freq.hsio_freq;
			if (tcc_freq_limit_table[i].freq.cam_freq > tcc_freq_curr_limit_table.cam_freq)
				tcc_freq_curr_limit_table.cam_freq = tcc_freq_limit_table[i].freq.cam_freq;
			if (tcc_freq_limit_table[i].freq.dsub_freq > tcc_freq_curr_limit_table.dsub_freq)
				tcc_freq_curr_limit_table.dsub_freq = tcc_freq_limit_table[i].freq.dsub_freq;

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
	struct tcc_freq_table_t *freqs = &gtClockLimitTable[0];

	clk_set_rate(mem_clk, freqs->mem_freq * 1000);
	/* Sets minimum CPU frequency and voltage when suspend */
	clk_set_rate(pll0_clk, tcc_cpufreq_get_pll_table(freqs->cpu_freq) * 1000);
	clk_set_rate(cpu_clk, freqs->cpu_freq * 1000);
	clk_set_rate(io_clk, freqs->io_freq * 1000);
//	clk_set_rate(smu_clk, freqs->smu_freq * 1000);
	clk_set_rate(ddi_clk, freqs->ddi_freq * 1000);
	clk_set_rate(gpu_clk, freqs->gpu_freq * 1000);
	clk_set_rate(hsio_clk, freqs->hsio_freq * 1000);
	clk_set_rate(cam_clk, freqs->cam_freq * 1000);
	clk_set_rate(dsub_clk, freqs->dsub_freq * 1000);

	return 0;
}

static int tcc_cpufreq_resume(struct cpufreq_policy *policy)
{
	struct tcc_freq_table_t freqs;
	int ret;

	memcpy(&freqs, &gtClockLimitTable[NUM_FREQS - 1], sizeof(struct tcc_freq_table_t));

#if !defined(CONFIG_REGULATOR)
	ret = tcc_cpufreq_set_voltage(TCC_CORE_A, tcc_cpufreq_get_voltage_table(TCC_CORE_A, &freqs));
	if (ret != 0) {
		pr_err("cpufreq: regulator_set_voltage failed\n");
		return -1;
	}
	ret = tcc_cpufreq_set_voltage(TCC_CORE_B, tcc_cpufreq_get_voltage_table(TCC_CORE_B, &freqs));
	if (ret != 0) {
		pr_err("cpufreq: regulator_set_voltage failed\n");
		return -1;
	}
#endif

	clk_set_rate(mem_clk, freqs.mem_freq * 1000);
	clk_set_rate(pll0_clk, tcc_cpufreq_get_pll_table(freqs.cpu_freq) * 1000);
	clk_set_rate(cpu_clk, freqs.cpu_freq * 1000);
	clk_set_rate(io_clk, freqs.io_freq * 1000);
//	clk_set_rate(smu_clk, freqs.smu_freq * 1000);
	clk_set_rate(ddi_clk, freqs.ddi_freq * 1000);
	clk_set_rate(gpu_clk, freqs.gpu_freq * 1000);
	clk_set_rate(hsio_clk, freqs.hsio_freq * 1000);
	clk_set_rate(cam_clk, freqs.cam_freq * 1000);
	clk_set_rate(dsub_clk, freqs.dsub_freq * 1000);

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
	hsio_clk = clk_get(NULL, "hsio");
	if (IS_ERR(hsio_clk))
		return PTR_ERR(hsio_clk);
	cam_clk = clk_get(NULL, "cam");
	if (IS_ERR(cam_clk))
		return PTR_ERR(cam_clk);
	dsub_clk = clk_get(NULL, "ddisub");
	if (IS_ERR(dsub_clk))
		return PTR_ERR(dsub_clk);

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
    timer_highspeed.expires = jiffies + msecs_to_jiffies(HIGHSPEED_TIMER_TICK);
    add_timer(&timer_highspeed);
#endif

#ifdef CONFIG_REGULATOR
	if (vdd_coreA == NULL) {
		vdd_coreA = regulator_get(NULL, "vdd_coreA");
		if (IS_ERR(vdd_coreA)) {
			pr_warning("cpufreq: failed to obtain vdd_coreA\n");
			vdd_coreA = NULL;
		}
	}
	if (vdd_coreB == NULL) {
		vdd_coreB = regulator_get(NULL, "vdd_coreB");
		if (IS_ERR(vdd_coreB)) {
			pr_warning("cpufreq: failed to obtain vdd_coreB\n");
			vdd_coreB = NULL;
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
