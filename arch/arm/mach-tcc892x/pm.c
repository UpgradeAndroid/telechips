/*
 * arch/arm/mach-tcc892x/pm.c
 *
 * Author:  <linux@telechips.com>
 * Created: November, 2011
 * Description: LINUX POWER MANAGEMENT FUNCTIONS
 *
 * Copyright (C) 2011 Telechips
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/*===========================================================================

                         INCLUDE FILES FOR MODULE

===========================================================================*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/suspend.h>
#include <linux/io.h>
#include <linux/reboot.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include <asm/tlbflush.h>
#include <linux/syscalls.h>		// sys_sync()
#include <asm/cacheflush.h>		// local_flush_tlb_all(), flush_cache_all();
#include <mach/system.h>
#include <mach/pm.h>

#include <mach/tcc_ddr.h>



/*===========================================================================

                  DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/

/*===========================================================================

                                 Shut-down

===========================================================================*/

/*===========================================================================
VARIABLE
===========================================================================*/

/*===========================================================================
FUNCTION
===========================================================================*/
static void shutdown(void)
{
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void io_restore(void)
{
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void shutdown_mode(void)
{
}
/*=========================================================================*/



/*===========================================================================

                               SDRAM Init

===========================================================================*/

/*===========================================================================
FUNCTION
===========================================================================*/
static void sdram_init(void)
{
}
/*=========================================================================*/



/*===========================================================================

                        Power Management Driver

===========================================================================*/

/*===========================================================================
FUNCTION
	mode 0: sleep mode, mode 1: shut down mode
===========================================================================*/
static int tcc_pm_enter(suspend_state_t state)
{
	unsigned long flags;
//	unsigned reg_backup[20];

// -------------------------------------------------------------------------
// disable interrupt
	local_irq_save(flags);
	local_irq_disable();

// -------------------------------------------------------------------------
// replace pm functions
	memcpy((void*)SRAM_BOOT_ADDR,       (void*)SRAM_Boot,  SRAM_BOOT_SIZE);
	memcpy((void*)SHUTDOWN_FUNC_ADDR,   (void*)shutdown,   SHUTDOWN_FUNC_SIZE);
	memcpy((void*)WAKEUP_FUNC_ADDR,     (void*)wakeup,     WAKEUP_FUNC_SIZE);
	memcpy((void*)SDRAM_INIT_FUNC_ADDR, (void*)sdram_init, SDRAM_INIT_FUNC_SIZE);

// -------------------------------------------------------------------------
// enter shutdown mode
	shutdown_mode();

// -------------------------------------------------------------------------
// enable interrupt
	local_irq_restore(flags);

	return 0;
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void tcc_pm_power_off(void)
{
#ifdef CONFIG_RTC_DISABLE_ALARM_FOR_PWROFF_STATE		//Disable the RTC Alarm during the power off state
	extern volatile void tca_alarm_disable(unsigned int rtcbaseaddress);

	tca_alarm_disable(tcc_p2v(HwRTC_BASE));
#endif
}

/*===========================================================================
VARIABLE
===========================================================================*/
static struct platform_suspend_ops tcc_pm_ops = {
	.valid   = suspend_valid_only_mem,
	.enter   = tcc_pm_enter,
};

/*===========================================================================
VARIABLE
===========================================================================*/
static uint32_t restart_reason = 0x776655AA;

/*===========================================================================
FUNCTION
===========================================================================*/
static void tcc_pm_restart(char str)
{
	/* store restart_reason to USTS register */
	if (restart_reason != 0x776655AA)
		((PPMU)tcc_p2v(HwPMU_BASE))->PMU_USSTATUS.nREG = restart_reason;

	//printk(KERN_DEBUG "reboot: reason=0x%x\n", restart_reason);

	arch_reset(str, NULL);
}

/*===========================================================================
FUNCTION
===========================================================================*/
static int tcc_reboot_call(struct notifier_block *this, unsigned long code, void *cmd)
{
	/* XXX: convert reboot mode value because USTS register
	 * hold only 8-bit value
	 */
	if (code == SYS_RESTART) {
		if (cmd) {
			if (!strcmp(cmd, "bootloader")) {
				restart_reason = 1;	/* fastboot mode */
			} else if (!strcmp(cmd, "recovery")) {
				restart_reason = 2;	/* recovery mode */
			} else {
				restart_reason = 0;
			}
		} else {
			restart_reason = 0;
		}
	}
	return NOTIFY_DONE;
}

/*===========================================================================
VARIABLE
===========================================================================*/
static struct notifier_block tcc_reboot_notifier = {
	.notifier_call = tcc_reboot_call,
};

/*===========================================================================
FUNCTION
===========================================================================*/
static int __init tcc_pm_init(void)
{
	pm_power_off = tcc_pm_power_off;
	arm_pm_restart = tcc_pm_restart;

	register_reboot_notifier(&tcc_reboot_notifier);

	suspend_set_ops(&tcc_pm_ops);
	return 0;
}

__initcall(tcc_pm_init);

