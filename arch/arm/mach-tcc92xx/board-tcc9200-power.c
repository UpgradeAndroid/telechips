#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>

#include <asm/mach-types.h>

#include <mach/gpio.h>
#include <mach/bsp.h>

#include <mach/tcc_board_power.h>

/* Debugging stuff */
static int debug = 0;
#define dprintk(msg...)	if (debug) { printk( "tcc board power: " msg); }

struct tcc_power_ctrl {
		unsigned port;
		unsigned refcount;
		unsigned onoff;
		int (*set_init)(struct tcc_power_ctrl *power);
		const char	* name;
};

int tcc_v5p_init(struct tcc_power_ctrl *power)
{

    if (machine_is_tcc8900() || machine_is_tcc9201()) {
		dprintk("%s tcc8900 or tcc9201  \n", __func__);
		power->port = TCC_GPEXT3(2);

	}
	#if defined(CONFIG_MACH_TCC9300)
	else if (machine_is_tcc9300()) {
		printk("%s  tcc9300  pwr:%d \n", __func__);
		power->port =TCC_GPEXT14(0);
	}
	#endif//
}

static struct tcc_power_ctrl tcc_powers[TCC_POWER_MAX] = {
							{
							.port = 0,
							.refcount = 0,
							.onoff = 0,
							.set_init = tcc_v5p_init,
							.name ="hdmi_lvds_on"
							}
						};


int tcc_power_control(int power, char control)
{
	struct tcc_power_ctrl *tcc_power = &tcc_powers[power];
	dprintk("%s : power:%d , control:%d \n",__func__, power, control);

	switch(control)
	{
		case TCC_POWER_INIT:
		{
			if(tcc_power->refcount == 0)
			{
				if(tcc_power->set_init)
					tcc_power->set_init(tcc_power);
				
				gpio_request(tcc_power->port, tcc_power->name);
				gpio_direction_output(tcc_power->port, 1);
			}
			tcc_power->refcount++;
		}
		break;


		case TCC_POWER_ON:
		{
			if(tcc_power->onoff == 0)
				gpio_set_value(tcc_power->port, 1);

			tcc_power->onoff++;
		}
		break;

		case TCC_POWER_OFF:
		{
			if(tcc_power->onoff == 0)
				break;

			tcc_power->onoff--;

			if(tcc_power->onoff == 0)
				gpio_set_value(tcc_power->port, 0);
		}
		break;

		default:
			return -1;
	}
	return 1;
	
}



static int __init tcc_power_init_off(void)
{
	int i = 0;
	for(i = 0; i < TCC_POWER_MAX; i++)
	{
		if((tcc_powers[i].refcount > 0)  && (tcc_powers[i].onoff == 0))
			gpio_set_value(tcc_powers[i].port, 0);

	}

	return 0;
}

late_initcall_sync(tcc_power_init_off);
