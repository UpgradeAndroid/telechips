#ifndef __TCC_ADCONV_H__
#define __TCC_ADCONV_H__

unsigned int    tcc_adc_adcinitialize(unsigned int devAddress, void* param);
unsigned int    tcc_adc_portinitialize(unsigned int devAddress, void* param);
unsigned int    tcc_adc_read(unsigned int channel);
unsigned int    tcc_adc_tsautoread(int* xpos, int* ypos);
unsigned int    tcc_adc_powerdown(void);
unsigned int    tcc_adc_powerup(void);

#endif //__TCC_ADCONV_H__
