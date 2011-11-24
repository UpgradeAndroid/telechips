/*
 * usbctrl.c: common usb control API
 *
 *  Copyright (C) 2011, Telechips, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/cpufreq.h>

#include <asm/io.h>
#include <asm/mach-types.h>

#include <mach/bsp.h>
#include <mach/gpio.h>
#include <mach/tcc_board_power.h>

#if !defined(CONFIG_ARCH_TCC92XX)
extern const struct tcc_freq_table_t gtHSIONormalClockLimitTable;
#endif

static struct clk *g_pOHCIClk = NULL;
static int is_usb_otg = 0;

void tcc_ohci_clkset(int on)
{
	char *pDev = NULL;
	struct clk* pClk = NULL;
	
	if (machine_is_tcc8900()){
		pDev = "usb_host"; 
	}
	else if (machine_is_tcc8800() || machine_is_tcc8800st()){
		pDev = "usb11h";
	}
	else{
		pDev = "unknown";
	}
	
	if (on)
	{
		if(g_pOHCIClk)
		{
			//printk("~~> [WARNING] OHCI clock is already turned on\n");
			return;
		}

		g_pOHCIClk = clk_get(NULL, (const char*)pDev);
		if (IS_ERR(g_pOHCIClk)){
			printk(KERN_ERR "ERR - usb_host clk_get fail.\n");
			return;
		}
		else{
			clk_enable(g_pOHCIClk);
			clk_set_rate(g_pOHCIClk, 48*1000*1000);
		}
	}
	else
	{
		if (g_pOHCIClk) {
		#if defined(CONFIG_TCC_DWC_OTG)
			if (!is_usb_otg)
		#endif
			{
				if(g_pOHCIClk){
					clk_disable(g_pOHCIClk);
				}
				g_pOHCIClk = NULL;
			}
		}
	}
}
EXPORT_SYMBOL(tcc_ohci_clkset);

static unsigned int g_iExec = 0;

/* [id] 0:OHCI , 1:EHCI's OHCI , -1:OTG */
void tcc_ohci_clock_control(int id, int on)
{
	if (id == 1){
		return;
	}

	if(g_iExec){
		printk("tcc_ohci_clock_control is still running...(%d)\n", id);
		return;
	}
	
	g_iExec = 1;
	
	if (on) {
		tcc_ohci_clkset(1);
		is_usb_otg = 1;
	} else {
	#if !defined(CONFIG_USB_OHCI_HCD) || !defined(CONFIG_USB_OHCI_HCD_MODULE)
		tcc_ohci_clkset(0);
	#endif
		is_usb_otg = 0;
	}
	
	g_iExec = 0;
}
EXPORT_SYMBOL(tcc_ohci_clock_control);

int USBHostVBUSControl(int id, int on)
{
	if (id == 1)
	{
		return 1;
	}
	else
	{
		if (machine_is_tcc8900()){
			int gpio_pwr_gp1, gpio_hvbus;

			gpio_pwr_gp1 = TCC_GPEXT3(4);
			gpio_hvbus = TCC_GPEXT3(1);

			gpio_request(gpio_pwr_gp1, "pwr_gp1");
			gpio_request(gpio_hvbus, "hvbus");

			if (on)
			{
				gpio_direction_output(gpio_pwr_gp1, 1);
				gpio_direction_output(gpio_hvbus, 1);
			}
			else
			{
				gpio_direction_output(gpio_hvbus, 0);
			}
		}
		else if(machine_is_tcc8800())
		{
			int gpio_fs_host_en, gpio_host_bus_en;
			
			tcc_power_control(TCC_V5P_POWER, (on)?TCC_POWER_ON:TCC_POWER_OFF);

			gpio_fs_host_en = TCC_GPEXT5(3);
			gpio_host_bus_en = TCC_GPEXT2(15);

			gpio_request(gpio_fs_host_en, "fs_host_en");
			gpio_request(gpio_host_bus_en, "host_bus_en");

			gpio_direction_output(gpio_fs_host_en, (on)?1:0);
			gpio_direction_output(gpio_host_bus_en, (on)?1:0);
		}
		else if(machine_is_m801_88() || machine_is_m803())
		{
			;
		}
	}
	return 1;
}
EXPORT_SYMBOL(USBHostVBUSControl);

int tcc_ohci_vbus_Init(void)
{
	if (machine_is_tcc8800()){
		tcc_power_control(TCC_V5P_POWER, TCC_POWER_INIT);
	}
	else if(machine_is_m801_88() || machine_is_m803()){
		;
	}

	return 0;
}
EXPORT_SYMBOL(tcc_ohci_vbus_Init);

/*************************************************************************************/
int tcc_ehci_vbus_Init(void)
{
	if (machine_is_tcc8800()){
		tcc_power_control(TCC_V5P_POWER, TCC_POWER_INIT);
	}
	else if(machine_is_tcc8800st())
	{
		;
	}
	else if(machine_is_m801_88() || machine_is_m803())
	{
		;
	}
	
	return 0;
}
EXPORT_SYMBOL(tcc_ehci_vbus_Init);

int tcc_ehci_vbus_ctrl(int on)
{
	if (machine_is_tcc8800())
	{
		int gpio_otg1_vbus_en, gpio_hs_host_en;

		tcc_power_control(TCC_V5P_POWER, (on)?TCC_POWER_ON:TCC_POWER_OFF);

		gpio_hs_host_en = TCC_GPEXT5(2);
		gpio_otg1_vbus_en = TCC_GPEXT2(14);	
		
		gpio_request(gpio_hs_host_en, "hs_host_en");
		gpio_request(gpio_otg1_vbus_en, "otg1_vbus_en");		

		/* Don't control gpio_hs_host_en because this power also supported in USB core. */
		gpio_direction_output(gpio_hs_host_en, 1);	
		gpio_direction_output(gpio_otg1_vbus_en, (on)?1:0);

		return 0;
	}
	else if(machine_is_tcc8800st())
	{
		int gpio_host1_vbus_en;
		gpio_host1_vbus_en = TCC_GPB(16);
		gpio_request(gpio_host1_vbus_en, "host1_vbus_en");
		gpio_direction_output(gpio_host1_vbus_en, (on)?1:0);

		return 0;
	}
	else if(machine_is_m801_88() || machine_is_m803())
	{
		return 0;
	}

	return -1;
}
EXPORT_SYMBOL(tcc_ehci_vbus_ctrl);


static struct clk *g_pEHCIClk;

void tcc_ehci_clkset(int on)
{
	if (machine_is_tcc8800st() || machine_is_tcc8800() || machine_is_m801_88() || machine_is_m803()) 
	{
		if(on)
		{
			if(g_pEHCIClk){
				//printk("~~> [WARNING] EHCI clock is already turned on\n");
				return;
			}

#if !defined(CONFIG_ARCH_TCC92XX)
			tcc_cpufreq_set_limit_table(&gtHSIONormalClockLimitTable, TCC_FREQ_LIMIT_EHCI, 1);
#endif
			
			g_pEHCIClk = clk_get(NULL, "usb20h");
			if (IS_ERR(g_pEHCIClk)){
				printk("ERR - usb20h clk_get fail.\n");	
				return;
			}

			clk_enable(g_pEHCIClk);
			clk_set_rate(g_pEHCIClk, 48*1000*1000);
		}
		else
		{
			if(g_pEHCIClk){
				clk_disable(g_pEHCIClk);
			}

#if !defined(CONFIG_ARCH_TCC92XX)
			tcc_cpufreq_set_limit_table(&gtHSIONormalClockLimitTable, TCC_FREQ_LIMIT_EHCI, 0);
#endif
			g_pEHCIClk = NULL;
		}
	}
}
EXPORT_SYMBOL(tcc_ehci_clkset);

