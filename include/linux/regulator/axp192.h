/*
 * axp192.h  --  Voltage regulation for the KrossPower AXP192
 *
 * Copyright (C) 2011 by Telechips
 *
 * This program is free software
 *
 */

#ifndef REGULATOR_AXP192
#define REGULATOR_AXP192

#include <linux/regulator/machine.h>

enum {
	AXP192_ID_DCDC1 = 0,
	AXP192_ID_DCDC2,
	AXP192_ID_DCDC3,
#if 0
	AXP192_ID_LDO1,
#endif
	AXP192_ID_LDO2,
	AXP192_ID_LDO3,
	AXP192_ID_LDO4,
	AXP192_ID_MAX
};

/**
 * AXP192_subdev_data - regulator data
 * @id: regulator Id (either AXP192_V3 or AXP192_V6)
 * @name: regulator cute name (example for V3: "vcc_core")
 * @platform_data: regulator init data (constraints, supplies, ...)
 */
struct axp192_subdev_data {
	int                         id;
	char                        *name;
	struct regulator_init_data  *platform_data;
};

/**
 * axp192_platform_data - platform data for axp192
 * @num_subdevs: number of regulators used (may be 1 or 2)
 * @subdevs: regulator used
 *           At most, there will be a regulator for V3 and one for V6 voltages.
 * @init_irq: main chip irq initialize setting.
 */
struct axp192_platform_data {
	int num_subdevs;
	struct axp192_subdev_data *subdevs;
	int (*init_irq)(int irq_num);
};

#define AXP192_CHG_CURR_100mA	0x00
#define AXP192_CHG_CURR_190mA	0x01
#define AXP192_CHG_CURR_280mA	0x02
#define AXP192_CHG_CURR_360mA	0x03
#define AXP192_CHG_CURR_450mA	0x04
#define AXP192_CHG_CURR_550mA	0x05
#define AXP192_CHG_CURR_630mA	0x06
#define AXP192_CHG_CURR_700mA	0x07
#define AXP192_CHG_CURR_780mA	0x08
#define AXP192_CHG_CURR_880mA	0x09
#define AXP192_CHG_CURR_960mA	0x0A
#define AXP192_CHG_CURR_1000mA	0x0B
#define AXP192_CHG_CURR_1080mA	0x0C
#define AXP192_CHG_CURR_1160mA	0x0D

#define AXP192_CHG_OK			0x02
#define AXP192_CHG_ING			0x01
#define AXP192_CHG_NONE			0x00

extern int axp192_battery_voltage(void);
extern int axp192_acin_detect(void);
extern int axp192_vbus_voltage(void);
extern void axp192_power_off(void);
extern void axp192_charge_current(unsigned char val);
extern int axp192_charge_status(void);
#endif
