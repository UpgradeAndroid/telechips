#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include <mach/TCC92x_Structures.h>
#include <mach/TCC92x_Physical.h>
#include <asm/mach-types.h>

#include <mach/tca_adconv.h>
#include "tsadc.h"


/************************************************************************************************
* Global Defines
************************************************************************************************/
#define MASK_ADCDATA(n)   ( (n)&0x3FF )
//#define MASK_ADCDATA(n)   ( (n)&0xFFF )

#define	BITSET(X, MASK)				((X) |= (unsigned long)(MASK) )
#define	BITSCLR(X, SMASK, CMASK)	((X) = ((((unsigned long)(X)) | ((unsigned long)(SMASK))) & ~((unsigned long)(CMASK))) )
#define	BITCSET(X, CMASK, SMASK)	((X) = ((((unsigned long)(X)) & ~((unsigned long)(CMASK))) | ((unsigned long)(SMASK))) )
#define	BITCLR(X, MASK)				((X) &= ~((unsigned long)(MASK)) )
#define	BITXOR(X, MASK)				((X) ^= (unsigned long)(MASK) )


 /************************************************************************************************
* Global Handle
************************************************************************************************/


/************************************************************************************************
* Type Define
************************************************************************************************/


/************************************************************************************************
* Global Variable define
************************************************************************************************/
//PGPIO			pGPIO;
PTSADC			pTSADC;
unsigned short	uADC_DATA[8];


/************************************************************************************************
* FUNCTION		: 
*
* DESCRIPTION	: 
*
************************************************************************************************/

unsigned int    tca_adc_adcinitialize(unsigned int devAddress, void* param )
{
	unsigned int    ret = 0;
	pTSADC = (PTSADC)devAddress;
	mdelay(5);

	if (machine_is_m801())
		pTSADC->ADCDLY = ADC_DELAY(130);
	else
		pTSADC->ADCDLY = ADC_DELAY(50);		// 10000 -> 50
#if defined(CONFIG_TOUCHSCREEN_TCCTS)
	HwTSADC->ADCTSC = 0x50;
#endif
	HwTSADC->ADCCON = RESSEL_10BIT| HwADCCON_PS_EN | HwADCCON_PS_VAL(11);	// prescale 12 (11+1),  PCK (12MHz)

	BITCLR(HwPMU->CONTROL, HwPMU_CONTROL_ASTM);
	BITSET(HwPMU->CONTROL, HwPMU_CONTROL_AISO);
	BITSET(HwPMU->CONTROL, HwPMU_CONTROL_APEN);

	return ret;
}

unsigned int    tca_adc_portinitialize(unsigned int devAddress, void* param)
{
	unsigned int    ret = 0;
	volatile PGPIO  pGPIO;
	pGPIO = (PGPIO)tcc_p2v(HwGPIO_BASE);
#if defined(CONFIG_TOUCHSCREEN_TCCTS)
	gpio_request(TCC_GPE(28), "ADC Touch Y-");
	gpio_request(TCC_GPE(29), "ADC Touch x-");
	gpio_request(TCC_GPE(30), "ADC Touch Y+");
	gpio_request(TCC_GPE(31), "ADC Touch x+");	
	tcc_gpio_config(TCC_GPE(28), GPIO_FN(1)|GPIO_CD(3));
	tcc_gpio_config(TCC_GPE(29), GPIO_FN(1)|GPIO_CD(3));
	tcc_gpio_config(TCC_GPE(30), GPIO_FN(1)|GPIO_CD(3));
	tcc_gpio_config(TCC_GPE(31), GPIO_FN(1)|GPIO_CD(3));	
	gpio_direction_input(TCC_GPE(28));
	gpio_direction_input(TCC_GPE(29));
	gpio_direction_input(TCC_GPE(30));
	gpio_direction_input(TCC_GPE(31));	
#endif
	return ret;
}


unsigned long tca_adc_read(unsigned int channel)
{
	unsigned long    ret = 0;
	unsigned int    uiCh = 0;

	switch(channel) {
	case ADC_CHANNEL0:
		uiCh = SEL_MUX_AIN0;
		break;
	case ADC_CHANNEL1:
		uiCh = SEL_MUX_AIN1;
		break;
	case ADC_CHANNEL2:
		uiCh = SEL_MUX_AIN2;
		break;
	case ADC_CHANNEL3:
		uiCh = SEL_MUX_AIN3;
		break;
	}

	//BITCLR(pTSADC->ADCCON, Hw2);	// wake-up
	//BITSET(pTSADC->ADCCON ,(uiCh|ENABLE_START_EN) );
	if(channel != ADC_TOUCHSCREEN) {
		BITCLR(pTSADC->ADCCON ,SEL_MUX_MASK );
		msleep(5);
	}
	BITSET(pTSADC->ADCCON ,(uiCh|ENABLE_START_EN) );
#ifdef CONFIG_INPUT_TCC_REMOTE
	if(channel != ADC_TOUCHSCREEN) {
		msleep(5);
	}
/*
	while (pTSADC->ADCCON & ENABLE_START_EN) {   // Wait for Start Bit Cleared
		if(channel != ADC_TOUCHSCREEN) {
			msleep(5);
		}
		else {
			ndelay(10);
		}
	}
*/
	while (!(pTSADC->ADCCON & ECFLG_END)) {   // Wait for ADC Conversion Ended
		if(channel != ADC_TOUCHSCREEN) {
			msleep(5);
		}
		else {
			ndelay(10);
		}
	}
#else
	while (pTSADC->ADCCON & ENABLE_START_EN) {   // Wait for Start Bit Cleared
		if(channel != ADC_TOUCHSCREEN) {
			msleep(5);
		}
		else {
			ndelay(10);
		}
	}
/*
	while (!(pTSADC->ADCCON & ECFLG_END)) {   // Wait for ADC Conversion Ended
		if(channel != ADC_TOUCHSCREEN) {
			msleep(5);
		}
		else {
			ndelay(10);
		}
	}
*/
#endif

	ret = MASK_ADCDATA( (pTSADC->ADCDAT0) );
	BITCSET(pTSADC->ADCCON, SEL_MUX_MASK ,ENABLE_START_EN);
	//BITSET(pTSADC->ADCCON, Hw2);

	return ret;
}
EXPORT_SYMBOL(tca_adc_read);

//#if defined(CONFIG_TOUCHSCREEN_TCCTS)
unsigned int    tca_adc_tsautoread(int* xpos, int* ypos)
{
	unsigned int    ret = 0;
	printk("->%s\n",__func__);
	BITSET( pTSADC->ADCTSC , ADCTSC_AUTO_ADC4);	// Auto Conversion
	BITSET( pTSADC->ADCCON , ENABLE_START_EN );	// ADC Conversion Start

	while (pTSADC->ADCCON & ENABLE_START_EN) {	// Wait for Start Bit Cleared
		msleep(1);
	}

	while (!(pTSADC->ADCCON & ECFLG_END)) {	// Wait for ADC Conversion Ended
		msleep(1);
	}

	//read x value
	*xpos= MASK_ADCDATA(pTSADC->ADCDAT0);

	//read y value
	*ypos= MASK_ADCDATA(pTSADC->ADCDAT1);

	BITCLR( pTSADC->ADCTSC , ADCTSC_AUTO_ADC5);	// Auto Conversion	

	pTSADC->ADCCLRINT = CLEAR_ADC_INT;
	pTSADC->ADCCLRUPDN = CLEAR_ADCWK_INT;
	pTSADC->ADCTSC = ADCTSC_WAIT_PENDOWN;

	return ret;
}

unsigned int    tca_adc_tsread(int* x, int* y, int *xp, int *ym)
{
	volatile PGPIO pGPIO = (volatile PGPIO)tcc_p2v(HwGPIO_BASE);

	//BITCLR(pTSADC->ADCCON, Hw2);//wakeup

////////// 1. Read X-Position
	pTSADC->ADCTSC = Hw3|Hw0; 
	//gpio setting - 30 high

	tcc_gpio_config(TCC_GPE(30), GPIO_FN(0));
	tcc_gpio_config(TCC_GPE(31), GPIO_FN(0));	
	gpio_direction_output(TCC_GPE(30), 0);
	gpio_direction_output(TCC_GPE(31), 0);
	gpio_set_value(TCC_GPE(31), 1);
	gpio_set_value(TCC_GPE(30), 0);
	
	BITCSET(pTSADC->ADCCON,Hw6-Hw3,  Hw5|Hw3);
	pTSADC->ADCCON |= ENABLE_START_EN;	// ADC Conversion Start
	while (pTSADC->ADCCON & ENABLE_START_EN)	// Wait for Start Bit Cleared
		;//msleep(1);
	while (!(pTSADC->ADCCON & ECFLG_END)) // Wait for ADC Conversion Ended
		;//msleep(1);
	//read x value
	*x= MASK_ADCDATA(pTSADC->ADCDAT0);

	tcc_gpio_config(TCC_GPE(28), GPIO_FN(1));
	tcc_gpio_config(TCC_GPE(29), GPIO_FN(1));
	tcc_gpio_config(TCC_GPE(30), GPIO_FN(1));
	tcc_gpio_config(TCC_GPE(31), GPIO_FN(1));
	
	pTSADC->ADCTSC = ADCTSC_AUTO_ADC5;	

////////// 2. Read Y-Position
	pTSADC->ADCTSC = Hw3|Hw1;

	tcc_gpio_config(TCC_GPE(28), GPIO_FN(0));
	tcc_gpio_config(TCC_GPE(29), GPIO_FN(0));	
	gpio_direction_output(TCC_GPE(28), 0);
	gpio_direction_output(TCC_GPE(29), 0);
	gpio_set_value(TCC_GPE(29), 1);
	gpio_set_value(TCC_GPE(28), 0);

	BITCSET(pTSADC->ADCCON,Hw6-Hw3,  Hw5|Hw4|Hw3);
	pTSADC->ADCCON |= ENABLE_START_EN;	// ADC Conversion Start
	while (pTSADC->ADCCON & ENABLE_START_EN)	// Wait for Start Bit Cleared
		;//Sleep(1);
	while (!(pTSADC->ADCCON & ECFLG_END))	// Wait for ADC Conversion Ended
		;//Sleep(1);
	//read y value
	*y= MASK_ADCDATA(pTSADC->ADCDAT1);
	pTSADC->ADCTSC = ADCTSC_AUTO_ADC5;

	tcc_gpio_config(TCC_GPE(28), GPIO_FN(1));
	tcc_gpio_config(TCC_GPE(29), GPIO_FN(1));
	tcc_gpio_config(TCC_GPE(30), GPIO_FN(1));
	tcc_gpio_config(TCC_GPE(31), GPIO_FN(1));
	

////////// 3. Read Z1
	pTSADC->ADCTSC = Hw3|Hw6|Hw7;	

	tcc_gpio_config(TCC_GPE(28), GPIO_FN(0));
	tcc_gpio_config(TCC_GPE(31), GPIO_FN(0));	
	gpio_direction_output(TCC_GPE(28), 0);
	gpio_direction_output(TCC_GPE(31), 0);
	gpio_set_value(TCC_GPE(31), 1);
	gpio_set_value(TCC_GPE(28), 0);

	BITCSET(pTSADC->ADCCON,Hw6-Hw3,  Hw5|Hw3);
	pTSADC->ADCCON |= ENABLE_START_EN;	// ADC Conversion Start
	while (pTSADC->ADCCON & ENABLE_START_EN)	// Wait for Start Bit Cleared
		;//Sleep(1);
	while (!(pTSADC->ADCCON & ECFLG_END))	// Wait for ADC Conversion Ended
		;//Sleep(1);
	//read x value
	*xp= MASK_ADCDATA(pTSADC->ADCDAT0);
	pTSADC->ADCTSC = ADCTSC_AUTO_ADC5;	

	tcc_gpio_config(TCC_GPE(28), GPIO_FN(1));
	tcc_gpio_config(TCC_GPE(29), GPIO_FN(1));
	tcc_gpio_config(TCC_GPE(30), GPIO_FN(1));
	tcc_gpio_config(TCC_GPE(31), GPIO_FN(1));

////////// 4. Read Z2
	pTSADC->ADCTSC = Hw3|Hw6|Hw7;	

	tcc_gpio_config(TCC_GPE(28), GPIO_FN(0));
	tcc_gpio_config(TCC_GPE(31), GPIO_FN(0));	
	gpio_direction_output(TCC_GPE(28), 0);
	gpio_direction_output(TCC_GPE(31), 0);
	gpio_set_value(TCC_GPE(31), 1);
	gpio_set_value(TCC_GPE(28), 0);
	
	BITCSET(pTSADC->ADCCON,Hw6-Hw3,  Hw5|Hw4);
	pTSADC->ADCCON |= ENABLE_START_EN;	// ADC Conversion Start
	while (pTSADC->ADCCON & ENABLE_START_EN)	// Wait for Start Bit Cleared
		;//Sleep(1);
	while (!(pTSADC->ADCCON & ECFLG_END))	// Wait for ADC Conversion Ended
		;//Sleep(1);
	//read x value
	*ym= MASK_ADCDATA(pTSADC->ADCDAT0);

	//*ym=0;
	//	BITSET(pTSADC->ADCCON, Hw2);//Standby
	BITCLR( pTSADC->ADCTSC , ADCTSC_AUTO_ADC5);	// Auto Conversion	
	pTSADC->ADCCLRINT = CLEAR_ADC_INT;
	pTSADC->ADCCLRUPDN = CLEAR_ADCWK_INT;
	pTSADC->ADCTSC = ADCTSC_WAIT_PENDOWN;

	tcc_gpio_config(TCC_GPE(28), GPIO_FN(1));
	tcc_gpio_config(TCC_GPE(29), GPIO_FN(1));
	tcc_gpio_config(TCC_GPE(30), GPIO_FN(1));
	tcc_gpio_config(TCC_GPE(31), GPIO_FN(1));
	
	return 1;
}
//#endif

unsigned int    tca_adc_powerdown(void)
{
	unsigned int    ret = 0;
	pTSADC->ADCCLRINT = Hw0;
#if defined(CONFIG_TOUCHSCREEN_TCCTS)
	pTSADC->ADCCLRUPDN = Hw0;
	pTSADC->ADCTSC |= ADCTSC_WAIT_PENDOWN;
#endif

	BITSET(pTSADC->ADCCON, STDBM_STANDBY);
	return ret;
}

unsigned int    tca_adc_powerup(void)
{
	unsigned int    ret = 0;
	BITCLR(pTSADC->ADCCON, STDBM_STANDBY); // Because STDBM_NORMAL is [ 0<<2 ] 
	return ret;
}

