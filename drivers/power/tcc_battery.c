#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/power_supply.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/wakelock.h>
#include <linux/device.h>
#include <asm/gpio.h>
#include <linux/kthread.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <mach/bsp.h>
#include <linux/clk.h>
#include <linux/notifier.h>
#include <linux/usb.h>
#include <linux/spinlock.h>
#include <mach/bsp.h>
#if defined(CONFIG_ARCH_TCC92XX)
#include <mach/TCC92x_Structures.h>
#include <mach/TCC92x_Physical.h>
#elif defined(CONFIG_ARCH_TCC88XX)
#include <mach/TCC88xx_Structures.h>
#include <mach/TCC88xx_Physical.h>
#endif 
#include <asm/io.h>
#include <mach/tcc_adc.h>
#include <asm/mach-types.h>

#define ENABLE_START_EN (1<<0)
#define ECFLG_END (1<<15)

#define ESTIMATE_RATE 1000
#define EMPTLIMIT 20 //
#define INTR 1

#define BATTVOLSAMPLE 8
unsigned long   gAvrVoltage[8];
unsigned long   gIndex = 0;

#define TRACE_BATT 0 

static struct wake_lock vbus_wake_lock;
static struct work_struct batt_work;

#if TRACE_BATT
#define BATT(x...) printk(KERN_INFO "[BATT] " x)
#else
#define BATT(x...) do {} while (0)
#endif

typedef enum {
	CHARGER_BATTERY = 0,
	CHARGER_USB,
	CHARGER_AC
} charger_type_t;

typedef enum {
	DISABLE_CHG = 0,
	ENABLE_SLOW_CHG,
	ENABLE_FAST_CHG
} batt_ctl_t;

typedef struct {
	int voltage_low;
	int voltage_high;
	int percentage;
} tcc_batt_vol;

const char *charger_tags[] = {"none", "USB", "AC"};

struct battery_info_reply {
	u32 batt_id;			/* Battery ID from ADC */
	u32 batt_vol;			/* Battery voltage from ADC */
	u32 batt_temp;			/* Battery Temperature (C) from formula and ADC */
	u32 batt_current;		/* Battery current from ADC */
	u32 level;			/* formula */
	u32 charging_source;		/* 0: no cable, 1:usb, 2:AC */
	u32 charging_enabled;		/* 0: Disable, 1: Enable */
	u32 full_bat;			/* Full capacity of battery (mAh) */
};

struct tcc_battery_info {
	/* lock to protect the battery info */
	struct tcc_adc_client *client;
	struct platform_device *pdev;
	struct mutex lock;
	struct clk *clk;
	struct battery_info_reply rep;
	struct work_struct changed_work;
	int present;
	unsigned long update_time;
};

struct mutex batt_lock;
struct task_struct	*batt_thread_task;
// XXX
void usb_register_notifier(struct notifier_block *nb);


static int tcc_power_get_property(struct power_supply *psy, 
				  enum power_supply_property psp,
				  union power_supply_propval *val);

static int tcc_battery_get_property(struct power_supply *psy, 
				    enum power_supply_property psp,
				    union power_supply_propval *val);

static ssize_t tcc_battery_show_property(struct device *dev,
					 struct device_attribute *attr,
					 char *buf);

static struct tcc_battery_info tcc_batt_info;

static unsigned int cache_time = 1000;

static int tcc_battery_initial = 0;



static enum power_supply_property tcc_battery_properties[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
};

static enum power_supply_property tcc_power_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static char *supply_list[] = {
	"battery",
};

static struct power_supply tcc_power_supplies[] = {
	{
		.name = "battery",
		.type = POWER_SUPPLY_TYPE_BATTERY,
		.properties = tcc_battery_properties,
		.num_properties = ARRAY_SIZE(tcc_battery_properties),
		.get_property = tcc_battery_get_property,
	},
	{
		.name = "usb",
		.type = POWER_SUPPLY_TYPE_USB,
		.supplied_to = supply_list,
		.num_supplicants = ARRAY_SIZE(supply_list),
		.properties = tcc_power_properties,
		.num_properties = ARRAY_SIZE(tcc_power_properties),
		.get_property = tcc_power_get_property,
	},
	{
		.name = "ac",
		.type = POWER_SUPPLY_TYPE_MAINS,
		.supplied_to = supply_list,
		.num_supplicants = ARRAY_SIZE(supply_list),
		.properties = tcc_power_properties,
		.num_properties = ARRAY_SIZE(tcc_power_properties),
		.get_property = tcc_power_get_property,
	},
};

static void usb_status_notifier_func(struct notifier_block *self, unsigned long action, void *dev);
static int g_usb_online;
static int g_ac_plugin;

static struct notifier_block usb_status_notifier = {
	.notifier_call = usb_status_notifier_func,
};



//extern struct tcc_udc *the_controller;
// extern int USB_FLAG; // not use

/* not yet */
#define GPIO_BATTERY_DETECTION
#define GPIO_BATTERY_CHARGER_EN
#define GPIO_BATTERY_CHARGER_CURRENT
#define TIME_STEP (HZ)

/* SAMPLE Value */



const tcc_batt_vol tcc_battery_levels[] = {
#if 1
	{ 0, 4095, 100}
#else
#if defined(CONFIG_MACH_TCC9200) || defined(CONFIG_MACH_TCC9201)
	// low, high, persent
	{ 732, 752,  100},    // 4.1v ~ 4.2v
	{ 716, 731,  90},     // 4.0v ~ 4.1v
	{ 699, 715,  80},     // 3.9v ~ 4.0v
	{ 680, 698,  70},     // 3.8v ~ 3.9v
	{ 665, 679,  60},     // 3.7v ~ 3.8v
	{ 647, 664,  50},     // 3.6v ~ 3.7v
	{ 628, 646,  40},     // 3.5v ~ 3.6v
	{ 610, 627,  30},     // 3.4v ~ 3.5v    
	{ 593, 609,  20},     // 3.3v ~ 3.4v    
	{ 575, 592,  10},     // 3.2v ~ 3.3v // low battery level is under 10 percentage.
	{ 557, 574,  0},      // 3.1v ~ 3.2v // power off level is 0 percentage.
#elif defined(CONFIG_MACH_TCC8900)
	// low, high, persent
	{ 712, 723,  100},    // 4.1v ~ 4.2v
	{ 694, 711,  90},     // 4.0v ~ 4.1v
	{ 674, 693,  80},     // 3.9v ~ 4.0v
	{ 656, 673,  70},     // 3.8v ~ 3.9v
	{ 637, 655,  60},     // 3.7v ~ 3.8v
	{ 620, 636,  50},     // 3.6v ~ 3.7v
	{ 602, 619,  40},     // 3.5v ~ 3.6v
	{ 580, 601,  30},     // 3.4v ~ 3.5v    
	{ 562, 579,  20},     // 3.3v ~ 3.4v    
	{ 545, 561,  10},     // 3.2v ~ 3.3v // low battery level is under 10 percentage.
	{ 534, 544,  0},      // 3.1v ~ 3.2v // power off level is 0 percentage.
#else
	{ 0, 1600, 100}
#endif
#endif
};


#if defined(CONFIG_REGULATOR_AXP192)
extern int axp192_battery_voltage(void);
extern int axp192_acin_detect(void);

const tcc_batt_vol m801_88_battery_levels[] = {
        { 3707, 4095,  100},    // 4.2v ~ 
        { 3613, 3706,  90},     // 4.1v ~ 4.2v
        { 3522, 3612,  75},     // 4.0v ~ 4.1v
        { 3425, 3521,  60},     // 3.9v ~ 4.0v
        { 3330, 3424,  45},     // 3.8v ~ 3.9v
        { 3181, 3329,  30},     // 3.7v ~ 3.8v
        { 3043, 3180,  15},     // 3.55v ~ 3.7v
        { 500,  3042,  0},     // 3.4v ~ 3.55v
};
#else
const tcc_batt_vol m801_88_battery_levels[] = {
    // low, high, persent
#if 0
    	{ 0, 4095, 50 },
#else
	{ 2710, 2900,  100},    // 4.1v ~ 4.2v
	{ 2626, 2709,  90},     // 4.0v ~ 4.1v
	{ 2559, 2625,  75},     // 3.9v ~ 4.0v
	{ 2490, 2558,  60},     // 3.8v ~ 3.9v
	{ 2416, 2489,  45},     // 3.7v ~ 3.8v
	{ 2343, 2415,  30},     // 3.6v ~ 3.7v
	{ 2271, 2342,  15},     // 3.5v ~ 3.6v
	{ 545, 2270,  0},     // 3.4v ~ 3.5v    
//	{ 2179, 2196,  10},     // 3.3v ~ 3.4v    // low battery level is under 10 percentage.
//	{ 545, 2178,  0},     // 3.2v ~ 3.3v  power off level is zero percentage.
#endif
};
#endif

const tcc_batt_vol m57te_battery_levels[] = {
    // low, high, persent
    { 686, 702, 100}, // 4.1v ~ 4.2v
    { 669, 686,  90}, // 4.0v ~ 4.1v
    { 653, 669,  75}, // 3.9v ~ 4.0v
    { 634, 653,  60}, // 3.8v ~ 3.9v 
    { 616, 634,  45}, // 3.7v ~ 3.8v 
    { 601, 616,  30}, // 3.6v ~ 3.7v 
    { 583, 601,  15}, // 3.5v ~ 3.6v low battery level is under 15%
    {  50, 583,   0}, //      ~ 3.5v power off level is zero percentage.
};

static void tcc_set_fast_charge_mode(int onoff) {
    if( onoff ) {
        if( machine_is_m57te() ) {
	     gpio_set_value(TCC_GPE(2), 1);		
	     printk("set fast charge mode\n");
        }
    } else {
        if( machine_is_m57te() ) {
	     gpio_set_value(TCC_GPE(2), 0);		
           printk("clear fast charge mode\n");
        }
    }
}

static void tcc_set_charge_enable(int onoff) {
    if( onoff ) {
        if( machine_is_m57te() ) {
	     gpio_set_value(TCC_GPE(3), 1);		
           printk("set charge enable\n");
        }
    } else {
        if( machine_is_m57te() ) {
	      gpio_set_value(TCC_GPE(3), 0);		
	      printk("set charge disable\n");
        }
    }
}

static void tcc_is_full_charge(int onoff) {
    if( machine_is_m57te() ) {
        return !(gpio_get_value(TCC_GPE(5)));
    }
}


static unsigned long tcc_read_adc(void)
{
	unsigned long adcValue = 0;
	int i=0;
	
	struct tcc_adc_client *client = tcc_batt_info.client;
	
	if (client) { 

		if (machine_is_tcc8900() || machine_is_tcc9201() || machine_is_m57te() || machine_is_m801() ) {
			adcValue = tcc_adc_start(client, 1, 0);
		} else if (machine_is_tcc9200s()) {
			adcValue = tcc_adc_start(client, 2, 0);
		} else if (machine_is_tcc8800()) {
			adcValue = tcc_adc_start(client, 5, 0);
		} else if (machine_is_m801_88() || machine_is_m803()) {
#if defined(CONFIG_REGULATOR_AXP192)
			adcValue = axp192_battery_voltage();
#else
			adcValue = tcc_adc_start(client, 5, 0);
#endif
		}

		// If invalid value is read, Get value again
		if( adcValue < EMPTLIMIT) {
			if (machine_is_tcc8900() || machine_is_tcc9201() || machine_is_m57te() || machine_is_m801() ) {
				adcValue = tcc_adc_start(client, 1, 0);
			} else if (machine_is_tcc9200s()) {
				adcValue = tcc_adc_start(client, 2, 0);
			} else if (machine_is_tcc8800()) {
				adcValue = tcc_adc_start(client, 5, 0);
			} else if (machine_is_m801_88() || machine_is_m803()) {
#if defined(CONFIG_REGULATOR_AXP192)
                        adcValue = axp192_battery_voltage();
#else
                        adcValue = tcc_adc_start(client, 5, 0);
#endif
			}
		}

		if( adcValue < EMPTLIMIT) {
            if( machine_is_m57te() || machine_is_m801()) {
                adcValue = m57te_battery_levels[ARRAY_SIZE(m57te_battery_levels)-1].voltage_high;
            } 
		else if(machine_is_m801_88() || machine_is_m803())
		{
		   adcValue = m801_88_battery_levels[ARRAY_SIZE(m801_88_battery_levels)-1].voltage_high;
		}
			else {
    			adcValue = tcc_battery_levels[ARRAY_SIZE(tcc_battery_levels)-1].voltage_high;
            }
            
			BATT("dummy ADC Value = %x\n", adcValue);
		}

		//BATT("ADC Value = %x\n", adcValue);
		
		//msleep(30);
		gAvrVoltage[gIndex++%BATTVOLSAMPLE] = adcValue;
	}	

	return 0;
}

static int test = 25;
#define SAMPLE 100

struct battery_info_reply tcc_cur_battery_info =
{
	.batt_id = 0,			/* Battery ID from ADC */
	.batt_vol = 1100,			/* Battery voltage from ADC */
	.batt_temp = SAMPLE,			/* Battery Temperature (C) from formula and ADC */
	.batt_current = SAMPLE,		/* Battery current from ADC */
	.level = 100,				/* formula */
	.charging_source = 0,	/* 0: no cable, 1:usb, 2:AC */
	.charging_enabled  = 0,	/* 0: Disable, 1: Enable */
	.full_bat = SAMPLE			/* Full capacity of battery (mAh) */
};

/* ADC raw data Update to Android framework */
void tcc_read_battery(struct battery_info_reply *info)
{
	memcpy(info, &tcc_cur_battery_info, sizeof(struct battery_info_reply));

	BATT("tcc_read_battery read batt id=%d,voltage=%d, temp=%d, current=%d, level=%d, charging source=%d, charging_enable=%d, full=%d\n", 
	     info->batt_id, info->batt_vol, info->batt_temp, info->batt_current, info->level, info->charging_source, info->charging_enabled, info->full_bat);
	
	return 0;
}

int tcc_cable_status_update(int status)
{
	int rc = 0;
	unsigned last_source;
	//struct tcc_udc *pdev = the_controller;

	if (!tcc_battery_initial)
	     return 0;

	mutex_lock(&tcc_batt_info.lock);

	switch(status) {
	case CHARGER_BATTERY:
		BATT("cable NOT PRESENT\n");
 		tcc_batt_info.rep.charging_source = CHARGER_BATTERY;
		break;
	case CHARGER_USB:
		BATT("cable USB\n");
        if(tcc_batt_info.rep.charging_source != CHARGER_USB) {
    	    tcc_batt_info.rep.charging_source = CHARGER_USB;
        }
		break;
	case CHARGER_AC:
		BATT("cable AC\n");
        if(tcc_batt_info.rep.charging_source != CHARGER_AC) {
    		tcc_batt_info.rep.charging_source = CHARGER_AC;
        }
		break;
	default:
		BATT(KERN_ERR "%s: Not supported cable status received!\n", __FUNCTION__);
		rc = -EINVAL;
	}

	if ((status == CHARGER_USB) && (g_usb_online == 0)) {
		tcc_batt_info.rep.charging_source = CHARGER_AC;
	}

	if (machine_is_m801_88() || machine_is_m803()) {
		if ((status == CHARGER_BATTERY) && (g_usb_online == 1)) {
			g_usb_online = 0;
		}
	}

    if( g_ac_plugin || tcc_batt_info.rep.charging_source == CHARGER_AC ) {
        tcc_set_fast_charge_mode(1);
    } else {
        tcc_set_fast_charge_mode(0);
    }

    if( tcc_batt_info.rep.charging_source == CHARGER_AC
        || tcc_batt_info.rep.charging_source == CHARGER_USB ) {
        tcc_set_charge_enable(1);
    } else {
        tcc_set_charge_enable(0);
    }
    
	last_source = tcc_batt_info.rep.charging_source;
	mutex_unlock(&tcc_batt_info.lock);
	
/*
	//spin_lock(pdev->lock);
	if (USB_FLAG) {
		status = CHARGER_USB;
	} 
	//spin_unlock(pdev->lock);
*/ // not use

	if (last_source == CHARGER_USB) {
 		wake_lock(&vbus_wake_lock);
	} else {
		/* give userspace some time to see the uevent and update
	  	* LED state or whatnot...
	  	*/
	 	wake_lock_timeout(&vbus_wake_lock, HZ / 2);
	}

	/* if the power source changes, all power supplies may change state */
	power_supply_changed(&tcc_power_supplies[CHARGER_BATTERY]);
	power_supply_changed(&tcc_power_supplies[CHARGER_USB]);
	power_supply_changed(&tcc_power_supplies[CHARGER_AC]);

	return rc;
}


/* A9 reports USB charging when helf AC cable in and China AC charger. */
/* Work arround: notify userspace AC charging first,
and notify USB charging again when receiving usb connected notificaiton from usb driver. */
static void usb_status_notifier_func(struct notifier_block *self, unsigned long action, void *dev)
{
	mutex_lock(&tcc_batt_info.lock);

	if (action == USB_BUS_ADD) {
	 	if (!g_usb_online) {
			g_usb_online = 1;
#if 1
			BATT("USB CHARGER IN\n");
			mutex_unlock(&tcc_batt_info.lock);
			tcc_cable_status_update(CHARGER_USB);
			mutex_lock(&tcc_batt_info.lock);
#else
			if (action && tcc_batt_info.rep.charging_source == CHARGER_AC) {
				mutex_unlock(&tcc_batt_info.lock);
				tcc_cable_status_update(CHARGER_USB);
				mutex_lock(&tcc_batt_info.lock);
			} else if (action) {
				BATT("warning: usb connected but charging source=%d\n", tcc_batt_info.rep.charging_source);
			}
#endif
		}     
	} else if( action == USB_BUS_REMOVE ) {
		g_usb_online = 0;
		BATT("USB CHARGER OUT\n");
		mutex_unlock(&tcc_batt_info.lock);
		// XXX

        if( g_ac_plugin )
    		tcc_cable_status_update(CHARGER_AC);
        else
            tcc_cable_status_update(CHARGER_BATTERY);
        
		mutex_lock(&tcc_batt_info.lock);
	}
	mutex_unlock(&tcc_batt_info.lock);

	return NOTIFY_OK;
}

static int tcc_get_battery_info(struct battery_info_reply *buffer)
{
	struct battery_info_reply info;
	int rc;

	if (buffer == NULL) 
		return -EINVAL;

	tcc_read_battery(&info);
                             
	//mutex_lock(&tcc_batt_info.lock);
	buffer->batt_id                 = (info.batt_id);
	buffer->batt_vol                = (info.batt_vol);
	buffer->batt_temp               = (info.batt_temp);
	buffer->batt_current            = (info.batt_current);
	buffer->level                   = (info.level);
	/* Move the rules of charging_source to cable_status_update. */
	/* buffer->charging_source      = be32_to_cpu(rep.info.charging_source); */
	buffer->charging_enabled        = (info.charging_enabled);
	buffer->full_bat                = (info.full_bat);
	//mutex_unlock(&tcc_batt_info.lock);
	
	return 0;
}

static int tcc_power_get_property(struct power_supply *psy, 
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	charger_type_t charger;
	
	mutex_lock(&tcc_batt_info.lock);
	charger = tcc_batt_info.rep.charging_source;
	mutex_unlock(&tcc_batt_info.lock);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		if (psy->type == POWER_SUPPLY_TYPE_MAINS)
			val->intval = (charger ==  CHARGER_AC ? 1 : 0);//1
		else if (psy->type == POWER_SUPPLY_TYPE_USB)
		{
//#ifdef CONFIG_MACH_TCC8800              // 101125 jckim temporary code for preventing to enter deep sleep
#if (0)
			val->intval = 1;
#else
			val->intval = (charger ==  CHARGER_USB ? 1 : 0);
#endif
		}
		else
			val->intval = 0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


int tcc_battery_get_charging_status(void)
{
	u32 level;
	charger_type_t charger;	
	int ret;

	mutex_lock(&tcc_batt_info.lock);
	charger = tcc_batt_info.rep.charging_source;
	
	switch (charger) {
	case CHARGER_BATTERY:
		ret = POWER_SUPPLY_STATUS_NOT_CHARGING;
		break;
	case CHARGER_USB:
	case CHARGER_AC:
		level = tcc_batt_info.rep.level;

        if(machine_is_m57te() ) {
            // check CHARG_RDY
            if( gpio_get_value(TCC_GPD(9))) {
    			ret = POWER_SUPPLY_STATUS_FULL;
            } else {
    			ret = POWER_SUPPLY_STATUS_CHARGING;
            }
        } else {
    		if (level == 100)
    			ret = POWER_SUPPLY_STATUS_FULL;
    		else
    			ret = POWER_SUPPLY_STATUS_CHARGING;
        }
		break;
	default:
		ret = POWER_SUPPLY_STATUS_UNKNOWN;
	}

	mutex_unlock(&tcc_batt_info.lock);
	return ret;
}
EXPORT_SYMBOL(tcc_battery_get_charging_status);

static int tcc_battery_get_property(struct power_supply *psy, 
				    enum power_supply_property psp,
				    union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = tcc_battery_get_charging_status();
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = tcc_batt_info.present;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		mutex_lock(&tcc_batt_info.lock);
		val->intval = tcc_batt_info.rep.level;
		mutex_unlock(&tcc_batt_info.lock);
		break;
	default:		
		return -EINVAL;
	}
	
	return 0;
}



static int tcc_battery_suspend(struct platform_device *dev, pm_message_t state)
{
	printk("%s in\n", __func__);	
	/* flush all pending status updates */

	if(machine_is_m801_88()) {
		// for LED
		gpio_set_value(TCC_GPF(15), 0);
		gpio_set_value(TCC_GPF(16), 0);			
	}

	printk("%s out\n", __func__);	
	return 0;
}

static int tcc_battery_resume(struct platform_device *dev)
{
	printk("%s in\n", __func__);	
	/* things may have changed while we were away */
	/* wake thread */

	if(machine_is_m801_88()) {
		// for LED
		gpio_set_value(TCC_GPF(15), 1);
		gpio_set_value(TCC_GPF(16), 1);			
	}

	printk("%s out\n", __func__);	
	return 0;
}


#define TCC_BATTERY_ATTR(_name)						\
{									\
	.attr = { .name = #_name, .mode = S_IRUGO}, \
	.show = tcc_battery_show_property,				\
	.store = NULL,							\
}

static struct device_attribute tcc_battery_attrs[] = {
	TCC_BATTERY_ATTR(batt_id),
	TCC_BATTERY_ATTR(batt_vol),
	TCC_BATTERY_ATTR(batt_temp),
	TCC_BATTERY_ATTR(batt_current),
	TCC_BATTERY_ATTR(charging_source),
	TCC_BATTERY_ATTR(charging_enabled),
	TCC_BATTERY_ATTR(full_bat),
};

enum {
	BATT_ID = 0,
	BATT_VOL,
	BATT_TEMP,
	BATT_CURRENT,
	CHARGING_SOURCE,
	CHARGING_ENABLED,
	FULL_BAT,
};

static int tcc_battery_create_attrs(struct device * dev)
{
	int i, j, rc;
	for (i = 0; i < ARRAY_SIZE(tcc_battery_attrs); i++) {
		rc = device_create_file(dev, &tcc_battery_attrs[i]);
		if (rc)
			goto tcc_attrs_failed;
	}

	goto succeed;
     
tcc_attrs_failed:
	while (i--)
		device_remove_file(dev, &tcc_battery_attrs[i]);

succeed:        
	return rc;
}

static ssize_t tcc_battery_show_property(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	int i = 0;
	const ptrdiff_t off = attr - tcc_battery_attrs;

	mutex_lock(&tcc_batt_info.lock);
	/* check cache time to decide if we need to update */
	if (tcc_batt_info.update_time &&
	    time_before(jiffies, tcc_batt_info.update_time + msecs_to_jiffies(cache_time)))
		goto dont_need_update;

	if (tcc_get_battery_info(&tcc_batt_info.rep) < 0) {
		printk(KERN_ERR "%s: rpc failed!!!\n", __FUNCTION__);
	} else {
		tcc_batt_info.update_time = jiffies;
	}
dont_need_update:
	mutex_unlock(&tcc_batt_info.lock);

	mutex_lock(&tcc_batt_info.lock);
	switch (off) {
	case BATT_ID:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       tcc_batt_info.rep.batt_id);
		break;
	case BATT_VOL:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       tcc_batt_info.rep.batt_vol);
		break;
	case BATT_TEMP:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       tcc_batt_info.rep.batt_temp);
		break;
	case BATT_CURRENT:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       tcc_batt_info.rep.batt_current);
		break;
	case CHARGING_SOURCE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       tcc_batt_info.rep.charging_source);
		break;
	case CHARGING_ENABLED:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       tcc_batt_info.rep.charging_enabled);
		break;
	case FULL_BAT:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       tcc_batt_info.rep.full_bat);
		break;
	default:
		i = -EINVAL;
	}
	mutex_unlock(&tcc_batt_info.lock);

	return i;
}
void tcc_ac_charger_detect_process(void) 
{
	if(machine_is_m801_88() || machine_is_m803())
	{
		unsigned long adcValue = 0;
		  int i=0;
	
		struct tcc_adc_client *client = tcc_batt_info.client;
#if defined(CONFIG_REGULATOR_AXP192)
		if(axp192_acin_detect())
#else
		if(tcc_adc_start(client, 4, 0) > 1500) 
#endif
	            g_ac_plugin = 1;
        	else
            	    g_ac_plugin = 0;



	      if( g_ac_plugin ) {
	        if( tcc_batt_info.rep.charging_source == CHARGER_BATTERY ) {
	            tcc_cable_status_update(CHARGER_AC);
				//if( machine_is_m801() )
				//	HwGPIOD->GPDAT |= Hw11;
	        }
	      } else {
	        if( tcc_batt_info.rep.charging_source == CHARGER_AC ) {
	            tcc_cable_status_update(CHARGER_BATTERY);
				//if( machine_is_m801() )
				//	HwGPIOD->GPDAT &= ~Hw11;
	        	}
	      }
	}
	else if( machine_is_m57te() ) {
        static int CHRG_STBY = 0;
        static int CHRG = 0;


	  if( gpio_get_value(TCC_GPE(5)) != CHRG_STBY){
	  	CHRG_STBY = gpio_get_value(TCC_GPE(5));
            printk("CHRG_STBY# 0x%x ", gpio_get_value(TCC_GPE(5)));
	  }


	 if(gpio_get_value(TCC_GPD(9)) != CHRG){
            CHRG = gpio_get_value(TCC_GPD(9));
            printk("CHRG# 0x%x ", gpio_get_value(TCC_GPD(9)));        
        }
	 

        // check CHRG_STBY# PORT

	    if( (gpio_get_value(TCC_GPE(26))) && (g_ac_plugin == 0) ) {
            g_ac_plugin = 1;
            if(g_usb_online)
	            tcc_cable_status_update(CHARGER_USB);
            else
	            tcc_cable_status_update(CHARGER_AC);

	    } else if( !(gpio_get_value(TCC_GPE(26))) && (g_ac_plugin == 1) ) {
            g_ac_plugin = 0;
            if(g_usb_online)
	            tcc_cable_status_update(CHARGER_USB);
            else
	            tcc_cable_status_update(CHARGER_BATTERY);
	    }


    } else if( machine_is_m801() ) {
    
	    if(HwGPIOE->GPDAT & Hw26)
	        g_ac_plugin = 1;
        else 
            g_ac_plugin = 0;

	    if( g_ac_plugin ) {
	        if( tcc_batt_info.rep.charging_source == CHARGER_BATTERY ) {
	            tcc_cable_status_update(CHARGER_AC);
				if( machine_is_m801() )
					gpio_set_value(TCC_GPD(11), 1);
	        }
	    } else {
	        if( tcc_batt_info.rep.charging_source == CHARGER_AC ) {
	            tcc_cable_status_update(CHARGER_BATTERY);
				if( machine_is_m801() )
					gpio_set_value(TCC_GPD(11), 0);
	        }
	    }
	}

}

static int battery_percentage = 100;
int tcc_battery_percentage(void)
{
	return battery_percentage;
}
EXPORT_SYMBOL(tcc_battery_percentage);

void tcc_battery_process(void) 
{
	unsigned long adcValue;
	unsigned long BattVoltage;
	int temp;
	int i, source, level, count;
	struct battery_info_reply info;
	unsigned int battery_changed_flag = 0;
    int battery_level_count = 0;
    tcc_batt_vol *pbattery_levels;

	adcValue = 0;
	temp = 0;

	for (i=0; i<8; i++)
		adcValue += gAvrVoltage[i];

	if( adcValue )
		BattVoltage = adcValue/BATTVOLSAMPLE;
	else 
		adcValue = 0;

	//BATT("avr = %d\r\n", BattVoltage);
	//BATT("PERCENT = %d\n", (int)(100 - ( ( MAXCAPACITY -BattVoltage)*100)/(MAXCAPACITY- EMPTLIMIT) )); // 0~ 100 in the range
	//BATT("mV = %d\n", (BattVoltage * 1000) /ESTIMATE_RATE); // mV
	temp  = ((BattVoltage * 1000) /ESTIMATE_RATE);

	//mutex_lock(&tcc_batt_info.lock);
	info.batt_id                 	= 0;
	info.batt_vol                  	= temp;
	info.batt_temp					= SAMPLE;
	info.batt_current            	= SAMPLE;
	info.charging_source			= tcc_batt_info.rep.charging_source;
	source							= tcc_batt_info.rep.charging_source;

	info.level = 100;

    if( machine_is_m57te() || machine_is_m801()) {
        battery_level_count = ARRAY_SIZE(m57te_battery_levels);
        pbattery_levels = m57te_battery_levels;
    }
    else if(machine_is_m801_88() || machine_is_m803()){
        battery_level_count = ARRAY_SIZE(m801_88_battery_levels);
        pbattery_levels = m801_88_battery_levels;
    	}	
	else {
        battery_level_count = ARRAY_SIZE(tcc_battery_levels);
        pbattery_levels = tcc_battery_levels;        
    }
    
	for (count = 0; count < battery_level_count; count++)
	{
		if ((temp >= pbattery_levels[count].voltage_low) && (temp <= pbattery_levels[count].voltage_high))
			info.level = pbattery_levels[count].percentage;
	}

	if( temp < pbattery_levels[battery_level_count-1].voltage_low ) {
	    info.level = pbattery_levels[battery_level_count-1].percentage;
	}

	//BATT("Voltage : %d, level: %d\r\n", temp, info->level);
	/* Move the rules of charging_source to cable_status_update. */
	//info->charging_source      		= CHARGER_BATTERY;
	info.charging_enabled        	= 0;
	info.full_bat                	= SAMPLE;

    if( (source == CHARGER_USB || source == CHARGER_AC) ) {
        if( machine_is_m57te() ) {
        	// check CHRG# port 
        	if( !(HwGPIOD->GPDAT & Hw9)  ) {
        	    info.charging_enabled = 1;
        	} else {
        	    info.charging_enabled = 0;
        	}
        } else {
        	// source 
        	if(info.level < 100 ){
        		info.charging_enabled = 1;
        	} else {
        		info.charging_enabled = 0;
        	}
        	//mutex_unlock(&tcc_batt_info.lock);	
        }
    } else {
		info.charging_enabled = 0;
    }

	if( info.batt_id != tcc_cur_battery_info.batt_id )			
	{/* Battery ID from ADC */

	}

	if( info.charging_source != tcc_cur_battery_info.charging_source )
	{
		battery_changed_flag = 1;
	}

	if( info.level != tcc_cur_battery_info.level 
	    //|| info.batt_vol != tcc_cur_battery_info.batt_vol 
		)
	{
		battery_changed_flag = 1;
	}

	if( info.charging_enabled != tcc_cur_battery_info.charging_enabled )
	{
		battery_changed_flag = 1;
	}

	if( info.full_bat != tcc_cur_battery_info.full_bat)
	{
		battery_changed_flag = 1;
	}

	if( info.level < 20 || battery_changed_flag )
	{
		memcpy(&tcc_cur_battery_info, &info, sizeof(struct battery_info_reply));
		power_supply_changed(&tcc_power_supplies[info.charging_source]);
	}

	battery_percentage = info.level;

	BATT("batt id=%d,voltage=%d, temp=%d, current=%d, level=%d%%, charging source=%d, charging_enable=%d, full=%d ==============================\n", 
	     info.batt_id, info.batt_vol, info.batt_temp, info.batt_current, info.level, info.charging_source, info.charging_enabled, info.full_bat);
}

static int tcc_battery_thread(void *v)
{
	struct task_struct *tsk = current;	
	daemonize("tcc-battery");
	allow_signal(SIGKILL);

	while (!signal_pending(tsk)) {
        int i;

	if(gIndex >= (BATTVOLSAMPLE-1))
	{
	        gIndex = BATTVOLSAMPLE-1;
		for(i=0;i<BATTVOLSAMPLE-1;i++)
		{
			gAvrVoltage[i] = gAvrVoltage[i+1];
		}
		tcc_read_adc();
	}
	else
	{
		tcc_read_adc();
	}
   
        tcc_ac_charger_detect_process();
        tcc_battery_process();


        for( i=0; i< 10; i++ ) {
            msleep(10);
            tcc_ac_charger_detect_process();
        }
		
		msleep(1500);
		power_supply_changed(&tcc_power_supplies[CHARGER_BATTERY]);
	}

	return 0;
}

static void tcc_ts_select(unsigned int selected)
{
	/* dummy */
}

static int tcc_bat_standby_f = 0; // 0 : not standby, 1 : standby
int tcc_adc_battery_standby(void) // check battery driver loading
{
    return tcc_bat_standby_f;
}
EXPORT_SYMBOL(tcc_adc_battery_standby);

void batt_conv(void)
{
	/* dummy */
}


static int tcc_battery_probe(struct platform_device *pdev)
{
	int i, err, ret;
	unsigned short adc_data;
   	struct tcc_adc_client *client;


	
	BATT("%s\n",__func__);
	//struct tcc_udc *usb_dev = the_controller;

#if 0 // kch, not support
	if (machine_is_tcc9200s()) {
		volatile PGPIO  pGPIO  = (volatile PGPIO)tcc_p2v(HwGPIO_BASE);

		BITCSET(pGPIO->GPEFN3 , 0x00000F00, Hw8 );  // Function ADCIN
		BITCLR(pGPIO->GPEEN , Hw26); // Port as Input
	}
#endif
	
	tcc_batt_info.update_time = jiffies;
	tcc_battery_initial = 1;

	/* init power supplier framework */
	for (i = 0; i < ARRAY_SIZE(tcc_power_supplies); i++) {
		err = power_supply_register(&pdev->dev, &tcc_power_supplies[i]);
		if (err)
			printk(KERN_ERR "Failed to register power supply (%d)\n", err);
	}


//	usb_register_notifier(&usb_status_notifier);
	/* create tcc detail attributes */
	tcc_battery_create_attrs(tcc_power_supplies[CHARGER_BATTERY].dev);

	//spin_lock(usb_dev->lock);
	if( machine_is_m57te() ||machine_is_m801() ) {
	    if(gpio_get_value(TCC_GPE(26)))			
	        tcc_batt_info.rep.charging_source = CHARGER_AC;
	}


#if 0
	if( machine_is_m801_88()){
		if(tcc_adc_start(client, 4, 0) > 4000)	
		        tcc_batt_info.rep.charging_source = CHARGER_AC;
		}
#endif

/*
	if (USB_FLAG)
	{
		printk("USB_FLAG : %d\n", USB_FLAG);
		tcc_batt_info.rep.charging_source = CHARGER_USB;
	}
	else
		tcc_batt_info.rep.charging_source = CHARGER_BATTERY;
	//spin_unlock(usb_dev->lock);
*/ //not use

	tcc_battery_initial = 1;
	
	// Get current Battery info
	if (tcc_get_battery_info(&tcc_batt_info.rep) < 0)
		printk(KERN_ERR "%s: get info failed\n", __FUNCTION__);

	tcc_batt_info.present = 1;
	tcc_batt_info.update_time = jiffies;

	clk_enable(tcc_batt_info.clk);
	
	/* register adc client driver */
	client = tcc_adc_register(pdev, tcc_ts_select, batt_conv, 0);
	if (IS_ERR(client)) {
		printk("Battery client ADC fail%p", client);
		ret = PTR_ERR(client);
		return ret;
	}

	if( machine_is_m801_88() || machine_is_m803()){
		printk("dc input value = %d\n", tcc_adc_start(client, 4, 0));
#if defined(CONFIG_REGULATOR_AXP192)
		if(axp192_acin_detect())
#else		
		if(tcc_adc_start(client, 4, 0) > 1500)	
#endif
		        tcc_batt_info.rep.charging_source = CHARGER_AC;
		}
	/* init */
	tcc_batt_info.client = client;
	tcc_batt_info.pdev   = pdev;
	tcc_batt_info.rep.level = 100;
	
	// /* get adc raw data */
	// tcc_adc_start(client, 1, 0);
	
	tcc_batt_info.update_time = jiffies;

	gIndex = 0;

	for(i=0; i<BATTVOLSAMPLE; i++)
   		tcc_read_adc();	

    tcc_ac_charger_detect_process();
	tcc_battery_process();

	kernel_thread(tcc_battery_thread, NULL, CLONE_KERNEL);

	tcc_bat_standby_f = 1; // battery driver stanby 

	printk("TCC Battery Driver Load\n");
	return 0;
	
fail:
	/* free adc client */
   	tcc_adc_release(client);

    return 0;
}

static int tcc_battery_remove(struct platform_device *pdev)
{
	/* dummy */
	return 0;
}

static struct platform_driver tcc_battery_driver = {
	.driver.name	= "tcc-battery",
	.driver.owner	= THIS_MODULE,
	.probe			= tcc_battery_probe,
	.suspend		= tcc_battery_suspend,
	.resume			= tcc_battery_resume,
};

static int __init tcc_battery_init(void)
{
	struct clk *clk;

	printk("%s\n", __func__);

	wake_lock_init(&vbus_wake_lock, WAKE_LOCK_SUSPEND, "vbus_present");

	mutex_init(&tcc_batt_info.lock);
	clk  = clk_get(NULL, "adc");
	if (!clk) {
		printk(KERN_ERR "can't get ADC clock\n");
		return -ENODEV;
	}
	tcc_batt_info.clk = clk;

	if(machine_is_m801_88()) {
		gpio_request(TCC_GPF(15), "LED0");
		gpio_request(TCC_GPF(16), "LED1");
		tcc_gpio_config(TCC_GPF(15), GPIO_FN(0));
		tcc_gpio_config(TCC_GPF(16), GPIO_FN(0));
		gpio_direction_output(TCC_GPF(15),0);
		gpio_direction_output(TCC_GPF(16),0);		
		gpio_set_value(TCC_GPF(15), 1);
		gpio_set_value(TCC_GPF(16), 1);			
	}

	if( machine_is_m57te() ) {
		//GPIOE[2] CHRG_CTL O
		//GPIOE[3] CHRG_EN O
		//GPIOE[5] CHRG_STBY# I
		//GPIOE[26] DC_INPUT I
		//GPIOE[25] BAT_DET I	    
		//GPIOD[9] CHRG# I

		gpio_request(TCC_GPE(2), "CHRG_CTL");
		gpio_request(TCC_GPE(3), "CHRG_EN");	
		gpio_request(TCC_GPE(5), "CHRG_STBY");
		gpio_request(TCC_GPE(25), "BAT_DET");
		gpio_request(TCC_GPE(26), "DC INPUT");
		gpio_request(TCC_GPD(9), "CHRG#");
		tcc_gpio_config(TCC_GPE(2), GPIO_FN(0));
		tcc_gpio_config(TCC_GPE(3), GPIO_FN(0));
		tcc_gpio_config(TCC_GPE(5), GPIO_FN(0));
		tcc_gpio_config(TCC_GPE(25), GPIO_FN(0));
		tcc_gpio_config(TCC_GPE(26), GPIO_FN(0));
		tcc_gpio_config(TCC_GPD(9), GPIO_FN(0));
		gpio_direction_input(TCC_GPE(5));
		gpio_direction_input(TCC_GPE(25));	
		gpio_direction_input(TCC_GPE(26));
		gpio_direction_output(TCC_GPE(2), 0);
		gpio_direction_output(TCC_GPE(3), 0);
		gpio_direction_input(TCC_GPD(9));
	}

	if( machine_is_m801()) {
		//for AC cable
		gpio_request(TCC_GPE(26), "AC DET");
		tcc_gpio_config(TCC_GPE(26), GPIO_FN(0));
		gpio_direction_input(TCC_GPE(26));

		// for LED control
		gpio_request(TCC_GPD(11), "LED");
		tcc_gpio_config(TCC_GPD(11), GPIO_FN(0));
		gpio_direction_output(TCC_GPD(11), 1);
		gpio_set_value(TCC_GPD(11), 0);
	}

	// for usb cable detection
	// B090183... need to modification

	return platform_driver_register(&tcc_battery_driver);
}

static void __exit tcc_battery_exit(void)
{
	clk_put(tcc_batt_info.clk);

	platform_driver_unregister(&tcc_battery_driver);
//	usb_unregister_notify(&usb_status_notifier);
}

module_init(tcc_battery_init);
MODULE_DESCRIPTION("TCC Battery Driver");
MODULE_AUTHOR("Telechips, Inc");
MODULE_LICENSE("GPL");
