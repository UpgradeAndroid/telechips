// ****************************************** //
// Temporary Driver for GPS on Android
// 
// Title  : UART5 and GPIO (GPS_ON)
// Target : TCC92xx
//
// Author : Hae Jung Kim
// ****************************************** //

#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/kernel.h> 
  
#include <linux/fs.h>            
#include <linux/mm.h>            
#include <linux/errno.h>         
#include <linux/types.h>         
#include <linux/fcntl.h>         
#include <linux/cdev.h>         
#include <linux/device.h>         
#include <linux/major.h>         
#include <linux/gpio.h>

#include <asm/uaccess.h>  
#include <asm/io.h>  
#include <asm/mach-types.h>

#include <mach/bsp.h>  
  
#define GPS_GPIO_DEV_NAME      "gps_gpio"   // 디바이스 이름
#define GPS_GPIO_DEV_MAJOR     240          // 임의로 지정한 디바이스 주번호
#define DEBUG_ON               0            // 디버깅용 메시지 ON
                                
#define gps_dbg(fmt,arg...)     if(DEBUG_ON) printk("== gps debug == " fmt, ##arg)

static int      gps_major = GPS_GPIO_DEV_MAJOR;
static dev_t    dev;
static struct   cdev gps_cdev;
static struct   class *gps_class;
int gps_k_flag;


//  ************************************************************ //
//  Device Open : 
//  오픈시 UART5를 실제 활성화 시키지 않는다.
//  GPIO는 기본 GPIO로 세팅하고 출력으로 하며 LOW상태로 만들어
//  GPS 장치가 비활성 상태로 유지하게 한다.
//  ************************************************************ //
static int gps_gpio_open (struct inode *inode, struct file *filp)  
{
    gps_k_flag = 0;   
    // Set the Port Configure for the UART5
    // GPIO SETTING
#if defined(CONFIG_MACH_TCC9300)||defined(CONFIG_MACH_TCC8800)
    if(machine_is_m801_88() || machine_is_m803()) // GPIOG[4]
    {
        gpio_set_value(TCC_GPG(4), 0);
    }
    else if(machine_is_tcc8800())
    {
        gpio_set_value(TCC_GPEXT1(6), 0);
    }
#elif defined(CONFIG_MACH_TCC8900)
    if(machine_is_tcc8900())
    {
        gps_dbg("machine_is_tcc8900 : gps_gpio_open\n\n");
        gpio_set_value(TCC_GPD(25), 0);    
    }
#else

#endif


    gps_dbg("tcc92xx : gps_gpio_open\n\n");
    return 0;  
}

//  ************************************************************ //
//  Device Release : 
//  디바이스 릴리즈 할때 UART5및 GPIO PORT D에 Pin15를
//  기본 디폴트 값으로 재 설정한다.
//  ************************************************************ //
static int gps_gpio_release (struct inode *inode, struct file *filp)  
{  
    gps_k_flag = 0;   
#if defined(CONFIG_MACH_TCC9300)||defined(CONFIG_MACH_TCC8800)
    if(machine_is_m801_88() || machine_is_m803()) // GPIOG[4]
    {
        gpio_set_value(TCC_GPG(4), 0);
    }
    else if(machine_is_tcc8800())
    {
        gpio_set_value(TCC_GPEXT1(6), 0);
    }
#elif defined(CONFIG_MACH_TCC8900)
    if(machine_is_tcc8900())
    {
        gpio_set_value(TCC_GPD(25), 0);    
    }
#else
    
#endif
    gps_dbg("tcc92xx : gps_gpio_close\n");
    return 0;  
}  
  
//  ************************************************************ //
//  Device Release : 
//  IO Control은 안드로이드의 HAL부분의 gps_tcc.c 코드에서
//  호출하며 gps프로그램 구동시 start, stop과 연동된다.
//  앞서 open에서 미리 세팅된 상태로
//  IOCTL에서 gps모뮬의 On/Off를 행하게 된다.
//  ************************************************************ //
static int gps_gpio_ioctl (struct inode *inode, struct file *filp,
                           unsigned int cmd, unsigned long arg)  
{
//    printk("gps_gpio_ioctl");
    switch( cmd )  
    {  
#if defined(CONFIG_MACH_TCC9300)||defined(CONFIG_MACH_TCC8800)
        case 0 : // GPS_On
            gps_k_flag = 1;   
            if(machine_is_m801_88() || machine_is_m803()) // GPIOG[4]
            {
                gpio_set_value(TCC_GPG(4), 1);
            }
            else if(machine_is_tcc8800())
            {
                gpio_set_value(TCC_GPEXT1(6), 1);
            }
            gps_dbg("tccxxxx : gps_gpio_on\n");
            break;   
        case 1 : // GPS_Off
            gps_k_flag = 0;   
            if(machine_is_m801_88() || machine_is_m803()) // GPIOG[4]
            {
                gpio_set_value(TCC_GPG(4), 0);
            }
            else if(machine_is_tcc8800()) 
            {
                gpio_set_value(TCC_GPEXT1(6), 0);
            }
            gps_dbg("tccxxxx : gps_gpio_off\n");
            break;
#elif defined(CONFIG_MACH_TCC8900)
 
        case 0 : // GPS_On
            gps_k_flag = 1;   
            if(machine_is_tcc8900())
            {
                gpio_set_value(TCC_GPD(25), 1);   
            }
            break;   
        case 1 : // GPS_Off
            gps_k_flag = 0;   
            if(machine_is_tcc8900())
            {
                gpio_set_value(TCC_GPD(25), 0);   
            }
            break;            
            
#else
        case 0 : // GPS_On
            gps_k_flag = 1;   

            break;   
        case 1 : // GPS_Off
            gps_k_flag = 0;   

            break;
#endif  // #if defined(CONFIG_MACH_TCC9300)
        default :
            break;
    };
    return 0;  
}  
  
//  ************************************************************ //
//  File Operation Struct :
//  디바이스 등록을 위한 open, release, ioctl을 설정한다.
//  ************************************************************ //
static struct file_operations gps_gpio_fops =  
{  
    .owner    = THIS_MODULE,  
    .ioctl    = gps_gpio_ioctl,  
    .open     = gps_gpio_open,       
    .release  = gps_gpio_release,    
};  

//  ************************************************************ //
//  Device Init :
//  디바이스 드라이버가 처음 등록시 자동으로 /dev 디렉토리에
//  gps_gpio라는 디바이스 파일을 등록하게끔 구성됨
//
//  ************************************************************ //
static int __init gps_gpio_init(void)  
{  
    int result;  
//        printk("gps_gpio_init"); 

  
	if (0 == gps_major)
	{
		/* auto select a major */
		result = alloc_chrdev_region(&dev, 0, 1, GPS_GPIO_DEV_NAME);
		gps_major = MAJOR(dev);
	}
	else
	{
		/* use load time defined major number */
		dev = MKDEV(gps_major, 0);
		result = register_chrdev_region(dev, 1, GPS_GPIO_DEV_NAME);
	}

	memset(&gps_cdev, 0, sizeof(gps_cdev));

	/* initialize our char dev data */
	cdev_init(&gps_cdev, &gps_gpio_fops);

	/* register char dev with the kernel */
	result = cdev_add(&gps_cdev, dev, 1);
    
	if (0 != result)
	{
		unregister_chrdev_region(dev, 1);
		printk("Error registrating mali device object with the kernel\n");
	}

    gps_class = class_create(THIS_MODULE, GPS_GPIO_DEV_NAME);
    device_create(gps_class, NULL, MKDEV(gps_major, MINOR(dev)), NULL,
                  GPS_GPIO_DEV_NAME);

    if (result < 0)
        return result;  

#if defined(CONFIG_MACH_TCC9300)||defined(CONFIG_MACH_TCC8800)
    if(machine_is_m801_88() || machine_is_m803()) // GPIOG[4]
    {
        gps_dbg("GPS_PWREN on\n");
        tcc_gpio_config(TCC_GPG(4), GPIO_FN(0));
        gpio_request(TCC_GPG(4), "GPIO_PWREN");
        gpio_direction_output(TCC_GPG(4), 0);
    }
    else if(machine_is_tcc8800()) 
    {
//        printk("gpio_direction_output__gps");
        gpio_direction_output(TCC_GPEXT1(6), 0);    // GPS Power On
    }
#elif defined(CONFIG_MACH_TCC8900)
    if(machine_is_tcc8900())
    {
        gps_dbg("GPS_8900_PWREN on");
        tcc_gpio_config(TCC_GPD(25), GPIO_FN(0));
        gpio_request(TCC_GPD(25), "GPIO_PWREN");
        gpio_set_value(TCC_GPD(25), 0);
    }
#else

#endif

    gps_dbg("GPS driver loaded\n");

    return 0;  
}  

//  ************************************************************ //
//  Device Exit :
//
//  자동으로 생성된 디바이스 파일과 디바이스를 언로드 함.
//  ************************************************************ //
static void __exit gps_gpio_exit(void)  
{  

//    printk("gps_gpio_exit");
	
    device_destroy(gps_class, MKDEV(gps_major, 0));
    class_destroy(gps_class);

    cdev_del(&gps_cdev);
    unregister_chrdev_region(dev, 1);

#if defined(CONFIG_MACH_TCC9300) || defined(CONFIG_MACH_TCC8800)
    // GPS Power off
    gps_dbg("GPS_PWREN off");
    if(machine_is_m801_88() || machine_is_m803()) // demo set
    {
        gpio_set_value(TCC_GPG(4), 0);
    }
    else if(machine_is_tcc8800()) 
    {
        gpio_direction_output(TCC_GPEXT1(6), 0);
    }
#elif defined(CONFIG_MACH_TCC8900)
    if(machine_is_tcc8900())    
    {
        gps_dbg("GPS_8900_PWREN off");
        gpio_set_value(TCC_GPD(25), 0);
    }
#else

#endif
    gps_dbg("GPS driver unloaded");
}  

EXPORT_SYMBOL(gps_k_flag);

module_init(gps_gpio_init);  
module_exit(gps_gpio_exit);  
  
MODULE_LICENSE("Dual BSD/GPL");  
