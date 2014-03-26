/*
 * linux/arch/arm/mach-tcc92xx/gpio.c
 *
 * Copyright (C) 2009, 2010 Telechips, Inc.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <mach/irqs.h>
#include <mach/gpio.h>
#include <asm/mach/irq.h>

#include <mach/bsp.h>

#define RMWREG32(addr, startbit, width, val) writel((readl(addr) & ~(((1<<(width)) - 1) << (startbit))) | ((val) << (startbit)), addr)

struct board_gpio_irq_config *board_gpio_irqs;

int gpio_to_irq(unsigned gpio)
{
	struct board_gpio_irq_config *gpio_irq = board_gpio_irqs;

	if (gpio_irq) {
		while (gpio_irq->irq != 0) {	// irq 0 is INT_TC0
			if (gpio_irq->gpio == gpio)
				return gpio_irq->irq;
			++gpio_irq;
		}
	}
	return -EINVAL;
}
EXPORT_SYMBOL(gpio_to_irq);

int tcc_gpio_config(unsigned gpio, unsigned flags)
{
	GPION *reg;
	unsigned bit = gpio & 0x1f;

	if (gpio < GPIO_PORTB)
		reg = HwGPIOA;
	else if (gpio < GPIO_PORTC)
		reg = HwGPIOB;
	else if (gpio < GPIO_PORTD)
		reg = HwGPIOC;
	else if (gpio < GPIO_PORTE)
		reg = HwGPIOD;
	else if (gpio < GPIO_PORTF)
		reg = HwGPIOE;
	else if (gpio < GPIO_PORTEXT1)
		reg = HwGPIOF;
	else
		return -EINVAL;

	if (flags & GPIO_FN_BITMASK) {
		unsigned fn = ((flags & GPIO_FN_BITMASK) >> GPIO_FN_SHIFT) - 1;

		if (bit < 8)
			RMWREG32(&reg->GPFN0, bit*4, 4, fn);
		else if (bit < 16)
			RMWREG32(&reg->GPFN1, (bit-8)*4, 4, fn);
		else if (bit < 24)
			RMWREG32(&reg->GPFN2, (bit-16)*4, 4, fn);
		else
			RMWREG32(&reg->GPFN3, (bit-24)*4, 4, fn);
	}

	if (flags & GPIO_PULLUP) {
		if (bit < 16)
			RMWREG32(&reg->GPPD0, bit*2, 2, 0x1);
		else
			RMWREG32(&reg->GPPD1, (bit-16)*2, 2, 0x1);
	} else if (flags & GPIO_PULLDOWN) {
		if (bit < 16)
			RMWREG32(&reg->GPPD0, bit*2, 2, 0x2);
		else
			RMWREG32(&reg->GPPD1, (bit-16)*2, 2, 0x2);
	} else if (flags & GPIO_PULL_DISABLE) {
		if (bit < 16)
			RMWREG32(&reg->GPPD0, bit*2, 2, 0);
		else
			RMWREG32(&reg->GPPD1, (bit-16)*2, 2, 0);
    }

	if (flags & GPIO_CD_BITMASK) {
		unsigned cd = ((flags & GPIO_CD_BITMASK) >> GPIO_CD_SHIFT) - 1;

		if (bit < 16)
			RMWREG32(&reg->GPCD0, bit*2, 2, cd);
		else
			RMWREG32(&reg->GPCD1, (bit-16)*2, 2, cd);
	}
	return 0;
}
EXPORT_SYMBOL(tcc_gpio_config);

static inline GPION *tcc92xx_gpio_to_reg(struct gpio_chip *chip)
{
	if (chip->base == GPIO_PORTA)
		return HwGPIOA;
	if (chip->base == GPIO_PORTB)
		return HwGPIOB;
	if (chip->base == GPIO_PORTC)
		return HwGPIOC;
	if (chip->base == GPIO_PORTD)
		return HwGPIOD;
	if (chip->base == GPIO_PORTE)
		return HwGPIOE;
	if (chip->base == GPIO_PORTF)
		return HwGPIOF;
	return 0;
}

static int tcc92xx_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	GPION *reg = tcc92xx_gpio_to_reg(chip);

	return (readl(&reg->GPDAT) >> offset) & 1;
}

static void tcc92xx_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	GPION *reg = tcc92xx_gpio_to_reg(chip);

	if (value)
		writel(readl(&reg->GPDAT) | (1 << offset), &reg->GPDAT);
	else
		writel(readl(&reg->GPDAT) & ~(1 << offset), &reg->GPDAT);
}

static int tcc92xx_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	GPION *reg = tcc92xx_gpio_to_reg(chip);

	writel(readl(&reg->GPEN) & ~(1 << offset), &reg->GPEN);
	return 0;
}

static int tcc92xx_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
	GPION *reg = tcc92xx_gpio_to_reg(chip);

	writel(readl(&reg->GPEN) | (1 << offset), &reg->GPEN);
	tcc92xx_gpio_set(chip, offset, value);
	return 0;
}

static struct gpio_chip tcc92xx_gpio_chips[] = {
	{
		.label = "gpio_a",
		.base = GPIO_PORTA,
		.ngpio = 16,
	},
	{
		.label = "gpio_b",
		.base = GPIO_PORTB,
		.ngpio = 32,
	},
	{
		.label = "gpio_c",
		.base = GPIO_PORTC,
		.ngpio = 32,
	},
	{
		.label = "gpio_d",
		.base = GPIO_PORTD,
		.ngpio = 26,
	},
	{
		.label = "gpio_e",
		.base = GPIO_PORTE,
		.ngpio = 32,
	},
	{
		.label = "gpio_f",
		.base = GPIO_PORTF,
		.ngpio = 28,
	},
};

int __init tcc92xx_gpio_init(void)
{
	struct gpio_chip *chip;
	int i;

	for (i = 0; i < ARRAY_SIZE(tcc92xx_gpio_chips); i++) {
		chip = &tcc92xx_gpio_chips[i];

		chip->direction_input = tcc92xx_gpio_direction_input;
		chip->direction_output = tcc92xx_gpio_direction_output;
		chip->set = tcc92xx_gpio_set;
		chip->get = tcc92xx_gpio_get;

		gpiochip_add(chip);
	}
	return 0;
}

postcore_initcall(tcc92xx_gpio_init);
