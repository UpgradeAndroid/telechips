#include <mach/TCC92x_Structures.h>
#include <mach/TCC92x_Physical.h>
#include <asm/io.h>
#include <mach/tca_adconv.h>

/************************************************************************************************
* Global Defines
************************************************************************************************/

/************************************************************************************************
* Global Handle
************************************************************************************************/

/************************************************************************************************
* Type Define
************************************************************************************************/

/************************************************************************************************
* Global Variable define
************************************************************************************************/
 
 /************************************************************************************************
* FUNCTION		: 
*
* DESCRIPTION	: 
*
************************************************************************************************/
unsigned int    tcc_adc_adcinitialize(unsigned int devAddress, void* param)
{
    unsigned int    ret = 0;
	ret = tca_adc_adcinitialize(devAddress, param);
    return ret;
}

unsigned int    tcc_adc_portinitialize(unsigned int devAddress, void* param)
{
    unsigned int    ret = 0;
	ret = tca_adc_portinitialize(devAddress, param);
    return ret;
}


unsigned int    tcc_adc_read(unsigned int channel, unsigned short *data)
{
    unsigned long  ret = 0;
	ret = tca_adc_read(channel);
    return ret;
}

unsigned int    tcc_adc_tsautoread(int* xpos, int* ypos)
{
    unsigned int    ret = 0;
	ret = tca_adc_tsautoread(xpos, ypos);
    return ret;
}

unsigned int    tcc_adc_powerdown(void)
{
    unsigned int    ret = 0;
	ret = tca_adc_powerdown();
    return ret;
}

unsigned int    tcc_adc_powerup(void)
{
    unsigned int    ret = 0;
	ret = tca_adc_powerup();
    return ret;

}
