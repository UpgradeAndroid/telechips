/* 
 * linux/drivers/char/sensor_bma220.c
 *
 * Author:  <linux@telechips.com>
 * Created: 10th Jun, 2008 
 * Description: Telechips Linux BACK-LIGHT DRIVER
 *
 * Copyright (c) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>


#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/mach-types.h>
#include <linux/delay.h>


//#if defined(CONFIG_ARCH_TCC92X) || defined (CONFIG_ARCH_TCC93XX)
#include <mach/bsp.h>
//#endif

#ifdef CONFIG_I2C
#include <linux/i2c.h>
#endif
#include <asm/gpio.h>
#include <linux/i2c/sensor_i2c.h>
#include <linux/input.h>

#include <mach/sensor_ioctl.h>

#define BMA220_DEBUG    0

#if BMA220_DEBUG
#define sensor_dbg(fmt, arg...)     printk(fmt, ##arg)
#else
#define sensor_dbg(arg...)
#endif

#define SENSOR_DEV_NAME			"sensor"
#define SENSOR_DEV_MAJOR		249
#define SENSOR_DEV_MINOR		1


typedef struct {
    int x;
    int y;
    int z;
	int resolution;
    int delay_time;
} tcc_sensor_accel_t;


#ifdef CONFIG_I2C
#define BMA220_I2C_ADDRESS      0x0B

static struct i2c_driver sensor_i2c_driver;
static struct i2c_client *sensor_i2c_client;

static struct timer_list *sensor_timer;
static int sensor_used_count=0;
volatile static unsigned int sensor_state_flag=0;
volatile static unsigned int sensor_duration=200;
volatile static tcc_sensor_accel_t tcc_sensor_accel;
static struct work_struct sensor_work_q;

static const struct i2c_device_id sensor_i2c_id[] = {
    { "tcc-accel-sensor", 0, },
    { }
};

struct sensor_i2c_chip_info {
    unsigned gpio_start;
    uint16_t reg_output;
    uint16_t reg_direction;

    struct i2c_client *client;
    struct gpio_chip gpio_chip;
};

static void sensor_timer_handler(unsigned long data)
{
    sensor_dbg("%s\n", __func__);
    if (schedule_work(&sensor_work_q) == 0) {
        sensor_dbg("cannot schedule work !!!\n");
    }
}

static void sensor_timer_registertimer(struct timer_list* ptimer, unsigned int timeover )
{
    sensor_dbg("%s\n", __func__);
    init_timer(ptimer);
    ptimer->expires = jiffies+msecs_to_jiffies(timeover);
    ptimer->data = (unsigned long)NULL;
    ptimer->function = sensor_timer_handler;

    add_timer(ptimer);
}

static void SENSOR_SEND_CMD(unsigned char reg, unsigned char val)
{
    unsigned char cmd[2];
    sensor_dbg("%s\n", __func__);
    cmd[0] = reg<<1;
    cmd[1] = val;    
    i2c_master_send(sensor_i2c_client, cmd, 2);
}


static unsigned char SENSOR_READ_DAT(unsigned char reg)
{
    unsigned char buf;
    unsigned char tmp;
    sensor_dbg("%s\n", __func__);
    tmp = reg<<1;
    i2c_master_send(sensor_i2c_client, &tmp, 1);
    i2c_master_recv(sensor_i2c_client, &buf, 1);

    return 	buf;
}


static int sensor_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct sensor_i2c_chip_info 	*chip;

	sensor_dbg("\n sensor_i2c_probe  :  %s \n", client->name);

	chip = kzalloc(sizeof(struct sensor_i2c_chip_info), GFP_KERNEL);
	if(chip == NULL)
	{
		sensor_dbg("\n tcc_sensor_i2c  :  no chip info. \n");
		return -ENOMEM;
	}

	chip->client = client;
	i2c_set_clientdata(client, chip);
	//sensor_i2c_client = client;

	return 0;
}

static int sensor_i2c_remove(struct i2c_client *client)
{
	struct sensor_i2c_chip_info 		*chip  = i2c_get_clientdata(client);

	kfree(chip);
	//sensor_i2c_client = NULL;
	
	return 0;
}


/* bma220 i2c control layer */
static struct i2c_driver sensor_i2c_driver = {
	.driver = {
		.name	= "tcc-accel-sensor",
       	.owner  = THIS_MODULE,
	},
	.probe		= sensor_i2c_probe,
	.remove		= sensor_i2c_remove,
	.id_table	= sensor_i2c_id,
};

static int bma220_i2c_register(void)
{
    struct i2c_board_info info;
    struct i2c_adapter *adapter;
    struct i2c_client *client;
    
    memset(&info, 0, sizeof(struct i2c_board_info));
    info.addr = BMA220_I2C_ADDRESS;
    strlcpy(info.type, "tcc-accel-sensor", I2C_NAME_SIZE);

    sensor_dbg(KERN_INFO "%s : bma220_i2c_register\n", __FUNCTION__);
    	  
#if defined(CONFIG_ARCH_TCC93XX)
    adapter = i2c_get_adapter(3);
#else
    if(machine_is_m57te()){
        adapter = i2c_get_adapter(1);
    }
    else if(machine_is_m801())  // 89_froyo
    {
        adapter = i2c_get_adapter(0);
    }
    else{  // 88_froyo (88_93 demo black board)  // M8801_8803
        sensor_dbg(KERN_INFO "%s : i2c_get_adapter(0)\n", __FUNCTION__);
        adapter = i2c_get_adapter(0);
    }
#endif    	  
    if (!adapter) 
    {
        sensor_dbg(KERN_ERR "can't get i2c adapter 0 for tcc-accel-sensor\n");
        return -ENODEV;
    }

    client = i2c_new_device(adapter, &info);
    i2c_put_adapter(adapter);
    if (!client) 
    {
        sensor_dbg(KERN_ERR "can't add i2c device at 0x%x\n", (unsigned int)info.addr);
        return -ENODEV;
    }

    sensor_i2c_client = client;

    return 0;
}

#endif  /* #ifdef CONFIG_I2C */

int tempdata[8][3];
int tunning_count = 0;
int offsetdata[3];
int olddata[3];

static int tcc_sensor_get_accel(void)
{
    int data[3];
	
    data[0] = SENSOR_READ_DAT(0x02);
    data[1] = SENSOR_READ_DAT(0x03);
    data[2] = SENSOR_READ_DAT(0x04);


    if(machine_is_m801())
    {
        tcc_sensor_accel.x = ((signed char)data[1]);	
        tcc_sensor_accel.x = -tcc_sensor_accel.x >> 2;
        tcc_sensor_accel.y = ((signed char)data[0]);
        tcc_sensor_accel.y = -tcc_sensor_accel.y >> 2;
        tcc_sensor_accel.z = ((signed char)data[2]);
        tcc_sensor_accel.z = tcc_sensor_accel.z >> 2;
    }
    else if(machine_is_m801_88()) // demo set
    {
        tcc_sensor_accel.x = ((signed char)data[1]);	
        tcc_sensor_accel.x = -(tcc_sensor_accel.x >> 2);
        tcc_sensor_accel.y = ((signed char)data[0]);
        tcc_sensor_accel.y = (tcc_sensor_accel.y >> 2);
        tcc_sensor_accel.z = ((signed char)data[2]);
        tcc_sensor_accel.z = (tcc_sensor_accel.z >> 2);    
        
        if(tunning_count<8){ 
            tempdata[tunning_count][0] = tcc_sensor_accel.x;
            tempdata[tunning_count][1] = tcc_sensor_accel.y;
            tempdata[tunning_count][2] = tcc_sensor_accel.z;

            tunning_count++;
        }else if(tunning_count == 8)
        {
            int i = 0;
            int dx = 0;
            int dy = 0;
            int dz = 0;
            int oridata[3] = {0, 0, 0};


            for(i = 0; i < 8; i++){
                if(i == 0){
                    oridata[0] = tempdata[i][0];
                    oridata[1] = tempdata[i][1];
                    oridata[2] = tempdata[i][2];
                }else{
                    oridata[0] += tempdata[i][0];
                    oridata[1] += tempdata[i][1];
                    oridata[2] += tempdata[i][2];
                }
            }
            dx =  (oridata[0]>>3);
            dy =  (oridata[1]>>3);
            dz =  ((oridata[2]>>3)-16);
     
            if(dx != 0) offsetdata[0] = -dx;
            if(dy != 0) offsetdata[1] = -dy;
            if(dz != 0) offsetdata[2] = -dz;

            tunning_count++;
        }   

        tcc_sensor_accel.x = olddata[0] + tcc_sensor_accel.x;
        tcc_sensor_accel.y = olddata[1] + tcc_sensor_accel.y;
        tcc_sensor_accel.z = olddata[2] + tcc_sensor_accel.z;                
        
        tcc_sensor_accel.x = olddata[0] = (tcc_sensor_accel.x>>1);
        tcc_sensor_accel.y = olddata[1] = (tcc_sensor_accel.y>>1);
        tcc_sensor_accel.z = olddata[2] = (tcc_sensor_accel.z>>1);
			
        tcc_sensor_accel.x = tcc_sensor_accel.x + offsetdata[0];
        tcc_sensor_accel.y = tcc_sensor_accel.y + offsetdata[1];
        tcc_sensor_accel.z = tcc_sensor_accel.z + offsetdata[2];
                        
        if((tcc_sensor_accel.x <= 2) && (tcc_sensor_accel.x >= -2))
    	      tcc_sensor_accel.x = 0;
        if((tcc_sensor_accel.y <= 2) && (tcc_sensor_accel.y >= -2))
            tcc_sensor_accel.y = 0;    

    }   
    else
    {
        tcc_sensor_accel.x = ((signed char)data[0]);	
        tcc_sensor_accel.x = tcc_sensor_accel.x >> 2;
        tcc_sensor_accel.y = ((signed char)data[1]);
        tcc_sensor_accel.y = tcc_sensor_accel.y >> 2;
        tcc_sensor_accel.z = ((signed char)data[2]);
        tcc_sensor_accel.z = tcc_sensor_accel.z >> 2;
    }
    return 0;
}

static void sensor_fetch_thread(struct work_struct *work)
{
    sensor_dbg("%s: sensor_duration =%d \n", __func__, sensor_duration);
    tcc_sensor_get_accel();
    sensor_timer_registertimer( sensor_timer, sensor_duration );
}

static ssize_t tcc_sensor_write(struct file *file, const char __user *user, size_t size, loff_t *o)
 {
	sensor_dbg("%s\n", __func__);
	return 0;
 }

static ssize_t tcc_sensor_read(struct file *file, char __user *user, size_t size, loff_t *o)
{
    sensor_dbg("%s\n", __func__);    
    sensor_dbg("%s: IOCTL_read_SENSORS_ACCELERATION  %d, %d, %d\n", __func__, tcc_sensor_accel.x, tcc_sensor_accel.y, tcc_sensor_accel.z);

//    tcc_sensor_get_accel();   //test  reverse
    if(copy_to_user(( tcc_sensor_accel_t*) user, (const void *)&tcc_sensor_accel, sizeof( tcc_sensor_accel_t))!=0)
    {
        sensor_dbg("tcc_gsensor_read error\n");
    }
    return 0;
}

static int tcc_sensor_ioctl(struct inode *inode, struct file *filp, 
							unsigned int cmd, void *arg)
{
//    if (!sensor_used_count)
//        return -1;

    sensor_dbg("%s  (0x%x)  \n", __FUNCTION__, cmd);

    switch (cmd) {

        case IOCTL_SENSOR_GET_DATA_ACCEL:

            if(copy_to_user((tcc_sensor_accel_t*)arg, (const void *)&tcc_sensor_accel, sizeof(tcc_sensor_accel_t))!=0)
            {
                sensor_dbg("copy_to error\n");
            }
            sensor_dbg("%s: IOCTL_SENSOR_GET_DATA_ACCEL %d, %d, %d\n", __func__, tcc_sensor_accel.x, tcc_sensor_accel.y, tcc_sensor_accel.z);
            break;

        case IOCTL_SENSOR_SET_DELAY_ACCEL:
            if(copy_from_user((void *)&sensor_duration, (unsigned int*) arg, sizeof(unsigned int))!=0)
            {
                sensor_dbg("copy_from error\n");
            }					
            sensor_dbg(KERN_INFO "%s:  IOCTL_SENSOR_SET_DELAY_ACCEL (0x%x) %d \n", __FUNCTION__, cmd, sensor_duration);
            tcc_sensor_accel.delay_time = sensor_duration;
            break;

        case IOCTL_SENSOR_GET_STATE_ACCEL:
            sensor_state_flag = 1;
            if(copy_to_user((unsigned int*) arg, (const void *)&sensor_state_flag, sizeof(unsigned int))!=0)
            {
                sensor_dbg("copy_to error\n");
            }			
			
            sensor_dbg(KERN_INFO "%s: IOCTL_SENSOR_GET_STATE_ACCEL  (0x%x) %d \n", __FUNCTION__, cmd, sensor_state_flag);	
            break;
			
        case IOCTL_SENSOR_SET_STATE_ACCEL:
        //    arg = sensor_state_flag;
	
            if(copy_from_user((void *)&sensor_state_flag, (unsigned int*) arg, sizeof(unsigned int))!=0)
            {
                sensor_dbg("copy_from error\n");
            }			
            sensor_dbg(KERN_INFO "%s: IOCTL_SENSOR_SET_STATE_ACCEL  (0x%x) %d \n", __FUNCTION__, cmd, sensor_state_flag);			
            break;
			
        default:
            sensor_dbg("sensor: unrecognized ioctl (0x%x)\n", cmd); 
            return -EINVAL;
            break;
    }
    return 0;
}

static int tcc_sensor_release(struct inode *inode, struct file *filp)
{
    sensor_dbg("%s (%d)\n", __FUNCTION__, sensor_used_count);
    sensor_used_count--;
    if (sensor_used_count < 0) {
        sensor_dbg("sensor: release error (over)\n"); 
        sensor_used_count = 0;
    }

    if (sensor_used_count == 0) {
        flush_scheduled_work();      
        del_timer_sync( sensor_timer );
        //del_timer( sensor_timer);
        kfree( sensor_timer);

#ifdef CONFIG_I2C
        i2c_unregister_device(sensor_i2c_client);
        i2c_del_driver(&sensor_i2c_driver);
        sensor_i2c_client = NULL;
#endif
    }
    return 0;
}

static int tcc_sensor_open(struct inode *inode, struct file *filp)
{
    int ret;
    unsigned char old_ctrl;
    //int num = MINOR(inode->i_rdev);

    volatile PGPION pGPIOF = (volatile PGPION)tcc_p2v(HwGPIOF_BASE);
    volatile PGPION pGPIOB = (volatile PGPION)tcc_p2v(HwGPIOB_BASE);
    sensor_dbg("%s : \n", __FUNCTION__);
    if (sensor_used_count == 0) {
        if(machine_is_m57te()){  // 89_froyo m57te
            //output gpio_f17
            BITCSET(pGPIOF->GPFN2, HwPORTCFG_GPFN1_MASK,	 HwPORTCFG_GPFN1(0));  // GSEN_CSB
            BITCSET(pGPIOF->GPEN, Hw17, Hw17);
            BITCSET(pGPIOF->GPDAT, Hw17, 0);

            //Input gpio_f24
            BITCSET(pGPIOF->GPFN3, HwPORTCFG_GPFN0_MASK,	 HwPORTCFG_GPFN0(0));  // GSEN_INT
            BITCSET(pGPIOF->GPEN, Hw24, 0);
            BITCSET(pGPIOF->GPDAT, Hw24, 0);	
        }
        else if(machine_is_m801()){  // 89_froyo M801
            BITCSET(pGPIOF->GPFN3, HwPORTCFG_GPFN0_MASK,	 HwPORTCFG_GPFN0(0));  // GSEN_INT
            BITCSET(pGPIOF->GPEN, Hw24, 0);
        }
        else if(machine_is_m801_88()){  // M801_88 board
            BITCSET(pGPIOB->GPFN3, HwPORTCFG_GPFN4_MASK,	 HwPORTCFG_GPFN4(0));  // GSEN_INT
            BITCSET(pGPIOB->GPEN, Hw28, 0);
            
            sensor_duration=60;  // case of, togather AK(compass)
        }else {
            sensor_dbg("%s : machine is demo board \n", __FUNCTION__);
        #if defined(CONFIG_ARCH_TCC88XX)
            // 88통합 보드 88_D2_6.0 version GPIOF(25)
            BITCSET(pGPIOF->GPFN3, HwPORTCFG_GPFN1_MASK,	 HwPORTCFG_GPFN1(0));  // GSEN_INT
            BITCSET(pGPIOF->GPEN, Hw25, 0);
        #endif
        }
#ifdef CONFIG_I2C
        // Initialize I2C driver for BMA220
        ret = i2c_add_driver(&sensor_i2c_driver);
        if(ret < 0) 
        {
            sensor_dbg("%s() [Error] failed i2c_add_driver() = %d\n", __func__, ret);
            return ret;
        }
        ret = bma220_i2c_register();
        if(ret < 0) 
        {
            sensor_dbg("%s() [Error] Failed register i2c client driver for bma220, return is %d\n", __func__, ret);
            return ret;
        }
        sensor_dbg("%s: post bma220_i2c_register : %x\n", __func__, old_ctrl);
#endif

        //synaptics_io_init();
        old_ctrl = SENSOR_READ_DAT(0x00);
        if (old_ctrl == 0xFF) {
            sensor_dbg("%s: No such device or address\n", __func__);
#ifdef CONFIG_I2C
            i2c_unregister_device(sensor_i2c_client);
            i2c_del_driver(&sensor_i2c_driver);
            sensor_i2c_client = NULL;
#endif			
            return -ENXIO;
        }
        sensor_dbg("%s: identification : %x\n", __func__, old_ctrl);

        old_ctrl = SENSOR_READ_DAT(0x01);
        sensor_dbg("%s: ASIC revition ID : %x\n", __func__, old_ctrl);

        sensor_timer= kmalloc( sizeof( struct timer_list ), GFP_KERNEL );      // test
        if (sensor_timer == NULL){
            sensor_dbg("%s: mem alloc fail\n", __func__);
#ifdef CONFIG_I2C
            i2c_unregister_device(sensor_i2c_client);
            i2c_del_driver(&sensor_i2c_driver);
            sensor_i2c_client = NULL;
#endif
            return -ENOMEM;
        }
        memset(sensor_timer, 0, sizeof(struct timer_list));      // test
        sensor_timer_registertimer( sensor_timer, sensor_duration );      // test
    }
    sensor_used_count++;	
    sensor_dbg("%s out... \n", __FUNCTION__);
    return 0;
}

struct file_operations tcc_sensor_fops =
{
	.owner		= THIS_MODULE,
	.open		= tcc_sensor_open,
	.release		= tcc_sensor_release,
	.ioctl			= tcc_sensor_ioctl,
	.read      		= tcc_sensor_read,
	.write		= tcc_sensor_write,	
};

static struct class *sensor_class;

int __init tcc_sensor_init(void)
{
    int ret;

    sensor_dbg(KERN_INFO "tcc_sensor_init \n", __FUNCTION__);
    ret = register_chrdev(SENSOR_DEV_MAJOR, SENSOR_DEV_NAME, &tcc_sensor_fops);
    if (ret < 0)
        return ret;

    sensor_class = class_create(THIS_MODULE, SENSOR_DEV_NAME);
    device_create(sensor_class,NULL,MKDEV(SENSOR_DEV_MAJOR,SENSOR_DEV_MINOR),NULL,SENSOR_DEV_NAME);

    INIT_WORK(&sensor_work_q, sensor_fetch_thread);

    sensor_dbg(KERN_INFO "%s\n", __FUNCTION__);
    return 0;
}

void __exit tcc_sensor_exit(void)
{
    sensor_dbg(KERN_INFO "%s\n", __FUNCTION__);
    unregister_chrdev(SENSOR_DEV_MAJOR, SENSOR_DEV_NAME);
}

module_init(tcc_sensor_init);
module_exit(tcc_sensor_exit);

MODULE_AUTHOR("Telechips Inc. c2-g2-2 linux@telechips.com");
MODULE_DESCRIPTION("TCCxxx accel-gsensor driver");
MODULE_LICENSE("GPL");

