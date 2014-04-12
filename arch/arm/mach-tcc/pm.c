#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/pm.h>
#include <linux/suspend.h>

#include <asm/system_misc.h>

#define CONTROL		0x00
#define WKUPEN		0x04
#define WKUPPOL		0x08
#define WATCHDOG	0x0c
#define CONFIG0		0x10
#define USERSTS		0x14
#define PWROFF		0x18

/* reboot modes */
#define FASTBOOT_MODE	0x77665500
#define NORMAL_MODE	0x77665501
#define RECOVERY_MODE	0x77665502

static int power_gpio;
static void __iomem *pmu_regs;

static int tcc_pm_enter(suspend_state_t state)
{
	return 0;
}

static int tcc_pm_prepare(void)
{
	return 0;
}

static void tcc_pm_finish(void)
{
}

static const struct platform_suspend_ops tcc_pm_ops = {
	.enter		= tcc_pm_enter,
	.prepare	= tcc_pm_prepare,
	.finish		= tcc_pm_finish,
	.valid		= suspend_valid_only_mem,
};

static void tcc_power_off(void)
{
	if (gpio_is_valid(power_gpio))
		gpio_direction_output(power_gpio, 1);

	while(true)
		cpu_relax();
}

static void tcc_restart(char mode, const char *cmd)
{
	u32 status;

	/* Set USERSTS to requested bootmode */
	if (!cmd)
		status = NORMAL_MODE;
	else if (strcmp(cmd, "bootloader") == 0)
		status = FASTBOOT_MODE;
	else if (strcmp(cmd, "recovery") == 0)
		status = RECOVERY_MODE;
	else {
		pr_warn("%s: unknown command '%s'\n",
			__func__, cmd);
		status = NORMAL_MODE;
	}
	writel(status, pmu_regs + USERSTS);

	/* software reset, Jump into ROM at address 0 */
	if (mode == 's')
		soft_restart(0);

	/* else setup watchdog and wait for it to trigger */
	writel(1 | BIT(31), pmu_regs + WATCHDOG);

	while(true)
		cpu_relax();
}


const static struct of_device_id pmu_of_match[] __initconst = {
	{ .compatible = "tcc,pmu", },
	{ },
};

static int __init tcc_pm_init(void)
{
	struct device_node *np = of_find_matching_node(NULL, pmu_of_match);
	if (!np) {
		pr_err("%s: No pmu in DT\n", __func__);
		return -ENODEV;
	}

	power_gpio = of_get_named_gpio(np, "power-gpio", 0);
	if (gpio_is_valid(power_gpio))
		gpio_request_one(power_gpio, GPIOF_OUT_INIT_LOW, "board_power_gpio");

	pmu_regs = of_iomap(np, 0);
	if (!pmu_regs)
		return -ENXIO;

	pm_power_off = tcc_power_off;
	arm_pm_restart = tcc_restart;

	suspend_set_ops(&tcc_pm_ops);
	return 0;
}
postcore_initcall(tcc_pm_init);
