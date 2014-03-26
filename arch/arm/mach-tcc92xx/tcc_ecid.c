/*
 * TCC CPU ID
 *
 * (C) 2007-2009 by smit Inc.
 * Author: jianjun jiang <jerryjianjun@gmail.com>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <asm/gpio.h>
#include <asm/delay.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <mach/bsp.h>
#include <linux/io.h>


#define ENABLE_FLAG		(0x80000000)
#define DELAY_LOOP		100

static unsigned gECID[2] = {0,0};
// ECID Code
// -------- 31 ------------- 15 ----------- 0 --------
// [0]     |****************|****************|    : '*' is valid
// [1]     |0000000000000000|****************|    : 
//

static void IO_UTIL_ReadECID (void)
{
	volatile PGPIO	pGPIO = (volatile PGPIO)tcc_p2v(HwGPIO_BASE);

	unsigned i;
	unsigned int CS, SIGDEV, FSET , PRCHG, PROG, MCK;

	gECID[0] = gECID[1] = 0;
	CS = 0;
	SIGDEV = 0;
	FSET = 0;
	PRCHG = 0;
	PROG = 0;
	MCK = 0;

	pGPIO->ECID0 = 0xe0010000;
	pGPIO->ECID0 = 0xe8010000;
	pGPIO->ECID0 = 0xe0010000;

	//start---
	pGPIO->ECID0 = (ENABLE_FLAG | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23));

	for(i=0; i<DELAY_LOOP; i++);
	PRCHG = 1;
	pGPIO->ECID0 = (ENABLE_FLAG | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23));

	for(i=0; i<DELAY_LOOP; i++);
	CS = 1;
	pGPIO->ECID0 = (ENABLE_FLAG | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23));
	PRCHG = 0;
	pGPIO->ECID0 = (ENABLE_FLAG | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23));
	SIGDEV = 1;
	pGPIO->ECID0 = (ENABLE_FLAG | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23));

	for(i=0; i<DELAY_LOOP; i++);
	FSET = 1;
	pGPIO->ECID0 = (ENABLE_FLAG | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23));

	for(i=0; i<DELAY_LOOP; i++);
	SIGDEV = 0;
	pGPIO->ECID0 = (ENABLE_FLAG | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23)); 

	for(i=0; i<DELAY_LOOP; i++);
	PRCHG = 1;
	pGPIO->ECID0 = (ENABLE_FLAG | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23));

	for(i=0; i<DELAY_LOOP; i++);
	FSET = 0;
	pGPIO->ECID0 = (ENABLE_FLAG | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23));

	for(i=0; i<DELAY_LOOP; i++);
	CS = 0;
	pGPIO->ECID0 = (ENABLE_FLAG | (CS<<30) | (FSET<<29) | (PRCHG<<27) | (PROG<<26) | (SIGDEV<<23));

	gECID[0] = pGPIO->ECID2;
	gECID[1] = pGPIO->ECID3;

	//printk(KERN_ERR "===Chips ID %08X%08X\n", gECID[0], gECID[1]);
	//delay50ns(10);

	pGPIO->ECID0  = 0x00000000;  // ECID Closed
}

static ssize_t cpu_id_read(struct device *dev, struct device_attribute *attr, char *buf)
{
	IO_UTIL_ReadECID();
	return sprintf(buf, "%08X%08X", gECID[0], gECID[1]);
}

static ssize_t cpu_id_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return 1;
}

static DEVICE_ATTR(chip_id, 0644, cpu_id_read, cpu_id_write);

static struct attribute * cpu_id_sysfs_entries[] = {
	&dev_attr_chip_id.attr,
	NULL,
};

static struct attribute_group cpu_id_attr_group = {
	.name	= NULL,
	.attrs	= cpu_id_sysfs_entries,
};

static int cpu_id_probe(struct platform_device *pdev)
{
	return sysfs_create_group(&pdev->dev.kobj, &cpu_id_attr_group);
	return 0;
}

static int cpu_id_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &cpu_id_attr_group);
	return 0;
}

#ifdef CONFIG_PM
static int cpu_id_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int cpu_id_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define cpu_id_suspend	NULL
#define cpu_id_resume	NULL
#endif

static struct platform_driver cpu_id_driver = {
	.probe		= cpu_id_probe,
	.remove		= cpu_id_remove,
	.suspend	= cpu_id_suspend,
	.resume		= cpu_id_resume,
	.driver		= {
		.name	= "cpu-id",
	},
};

static int cpu_id_init(void)
{
	return platform_driver_register(&cpu_id_driver);
}

static void cpu_id_exit(void)
{
	platform_driver_unregister(&cpu_id_driver);
}

module_init(cpu_id_init);
module_exit(cpu_id_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jianjun jiang <jerryjianjun@gmail.com>");
MODULE_DESCRIPTION("Get CPU ID");
