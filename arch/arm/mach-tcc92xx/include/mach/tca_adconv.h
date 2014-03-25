
#ifndef __TCC_TSADC_H__
#define __TCC_TSADC_H__
//


enum {
    ADC_CHANNEL0 = 0,
    ADC_CHANNEL1,
    ADC_CHANNEL2,
    ADC_CHANNEL3,
    ADC_TOUCHSCREEN,
};

unsigned int    tca_adc_adcinitialize(unsigned int devAddress, void* param );
unsigned int    tca_adc_portinitialize(unsigned int devAddress, void* param);
unsigned long	tca_adc_read(unsigned int channel);
unsigned int    tca_adc_tsautoread(int* xpos, int* ypos);
unsigned int    tca_adc_tsread(int* xpos, int* ypos, int *xp, int *ym);
unsigned int    tca_adc_powerdown(void);
unsigned int    tca_adc_powerup(void);

#endif //__TCC_TSADC_H__
