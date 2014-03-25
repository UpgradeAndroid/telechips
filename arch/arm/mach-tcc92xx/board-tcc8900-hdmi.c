#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>

#include <asm/mach-types.h>

#include <mach/gpio.h>
#include <mach/bsp.h>
#include <mach/tcc_board_hdmi.h>
#include <mach/tcc_board_power.h>

tcc_hdmi_power_s hdmipwr;
int tcc_hdmi_power(struct device *dev, tcc_hdmi_power_s pwr)
{
	hdmipwr = pwr;
	
    if (machine_is_tcc8900() || machine_is_tcc9201()) {
		printk("%s  tcc8900/tcc9201  pwr:%d \n", __func__, pwr);

		switch(pwr)
		{
			case TCC_HDMI_PWR_INIT:
				tcc_power_control(TCC_V5P_POWER, TCC_POWER_INIT);
//				gpio_request(TCC_GPEXT3(2), "hdmi_lvds_on");
//				gpio_direction_output(TCC_GPEXT3(2), 1);

				gpio_request(TCC_GPEXT2(7), "hdmi_on");
				gpio_direction_output(TCC_GPEXT2(7), 1);

				// VDD5D 
				gpio_request(TCC_GPEXT1(14), "vdd5d");
				gpio_direction_output(TCC_GPEXT1(14), 1);
				break;
				
			case TCC_HDMI_PWR_ON:	
				gpio_set_value(TCC_GPEXT1(14), 1);

				tcc_power_control(TCC_V5P_POWER, TCC_POWER_ON);
//				gpio_set_value(TCC_GPEXT3(2), 1);
				gpio_set_value(TCC_GPEXT2(7), 1);

				break;
			case TCC_HDMI_PWR_OFF:
				gpio_set_value(TCC_GPEXT1(14), 0);

				tcc_power_control(TCC_V5P_POWER, TCC_POWER_OFF);
//				gpio_set_value(TCC_GPEXT3(2), 0);
				gpio_set_value(TCC_GPEXT2(7), 0);
				break;
		}

	}
	else if (machine_is_tcc9200s()) {
		printk("%s  tcc9200s  pwr:%d \n", __func__, pwr);
		switch(pwr)
		{
			case TCC_HDMI_PWR_INIT:
				gpio_request(TCC_GPA(17), "hdmi_on");
				gpio_direction_output(TCC_GPA(17), 1);
				break;
			case TCC_HDMI_PWR_ON:		
				gpio_set_value(17, 1);
				break;
			case TCC_HDMI_PWR_OFF:
				gpio_set_value(17, 0);
				break;
		}
	}
	#if defined(CONFIG_MACH_TCC9300)
	else if (machine_is_tcc9300()) {
		printk("%s  tcc9300  pwr:%d \n", __func__, pwr);
		switch(pwr)
		{
			case TCC_HDMI_PWR_INIT:
				tcc_power_control(TCC_V5P_POWER, TCC_POWER_INIT);
				//gpio_request(TCC_GPEXT14(0), "hdmi_lvds_on");
				//gpio_direction_output(TCC_GPEXT14(0), 1);

				gpio_request(TCC_GPEXT14(3)), "hdmi_on");				
				gpio_direction_output(TCC_GPEXT14(3), 1);
				break;

			case TCC_HDMI_PWR_ON:		
				tcc_power_control(TCC_V5P_POWER, TCC_POWER_ON);
				//gpio_set_value(TCC_GPEXT14(0), 1);	
				gpio_set_value(TCC_GPEXT14(3), 1);
				break;

			case TCC_HDMI_PWR_OFF:
				tcc_power_control(TCC_V5P_POWER, TCC_POWER_OFF);
				//gpio_set_value(TCC_GPEXT14(0), 0);
				gpio_set_value(TCC_GPEXT14(3), 0);	
				break;
		}		
	}
	#endif//
	#if defined(CONFIG_MACH_TCC9300CM)
	else if(machine_is_tcc9300cm())
	{
		printk("%s  tcc9300cm  pwr:%d \n", __func__, pwr);
		switch(pwr)
		{
			case TCC_HDMI_PWR_INIT:
				gpio_request(TCC_GPEXT3(2), "hdmi_on");						
				gpio_direction_output(TCC_GPEXT3(2), 1);
				break;

			case TCC_HDMI_PWR_ON:		
				gpio_set_value(TCC_GPEXT3(2), 1);
				break;

			case TCC_HDMI_PWR_OFF:
				gpio_set_value(TCC_GPEXT3(2), 0)				
				break;
		}
	}
	#endif//
	#if defined(CONFIG_MACH_M57TE) || defined(CONFIG_MACH_M801)
	else if(machine_is_m57te() || machine_is_m801())
	{
		printk("%s m57te %d ~ \n", __func__, pwr);
		switch(pwr)
		{
			case TCC_HDMI_PWR_INIT:
				gpio_request(TCC_GPD(10), "hdmi_on");						
				gpio_direction_output(TCC_GPD(10), 1);
				break;
				
			case TCC_HDMI_PWR_ON:		
				gpio_set_value(TCC_GPD(10), 1);
				break;
			case TCC_HDMI_PWR_OFF:
				gpio_set_value(TCC_GPD(10), 0);
				break;
		}
	}
	#endif//
	#if defined(CONFIG_MACH_MT1308) 
	else if(machine_is_mt1308())
	{
		switch(pwr)
		{
			case TCC_HDMI_PWR_INIT:
				gpio_request(TCC_GPB(22), "hdmi_on");										
				gpio_direction_output(TCC_GPB(22), 1);
				break;
			case TCC_HDMI_PWR_ON:		
				gpio_set_value(TCC_GPB(22), 1);
				break;
			case TCC_HDMI_PWR_OFF:
				gpio_set_value(TCC_GPB(22), 0);
				break;
		}
	}	
	#endif//
	else {
		printk("ERROR : can not find HDMI POWER SETTING ");
	}	
	return 0;

}


static struct tcc_hdmi_platform_data hdmi_pdata = {
	.set_power	= tcc_hdmi_power,
};


struct platform_device tcc_hdmi_device = {
	.name	= "tcc_hdmi",
	.dev	= {
		.platform_data	= &hdmi_pdata,
	},
};

struct platform_device tcc_audio_device = {
	.name	= "tcc_hdmi_audio",
};


struct platform_device tcc_cec_device = {
	.name	= "tcc_hdmi_cec",
};

static struct tcc_hpd_platform_data hpd_pdata = {
	.hpd_port = TCC_GPA(14),
};

struct platform_device tcc_hpd_device = {
	.name	= "tcc_hdmi_hpd",
	.dev = {
		.platform_data = &hpd_pdata,
	}, 
};

static int __init tcc8900_init_hdmi(void)
{
	printk("%s (built %s %s)\n", __func__, __DATE__, __TIME__);

	platform_device_register(&tcc_hdmi_device);

	platform_device_register(&tcc_audio_device);

	platform_device_register(&tcc_cec_device);

	platform_device_register(&tcc_hpd_device);

	return 0;
}
device_initcall(tcc8900_init_hdmi);


