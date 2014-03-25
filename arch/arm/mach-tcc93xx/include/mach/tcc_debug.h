#ifndef __TCC_DEBUG_H__
#define __TCC_DEBUG_H__

/*
 * This program is debugging routines for Telechips TCC89/91/92 EVB.
 * For DEBUGGING ONLY!!!
 */
#include <bsp.h>

static inline void tcc_gpio_init(int init_val)
{
    /*
     * Hw8  : GPIO_E8  - DXB1_SFRM - JR21[8]
     * Hw9  : GPIO_E9  - DXB1_SCLK - JR21[10]
     * Hw10 : GPIO_E10 - DXB1_SDI  - JR21[12]
     * Hw11 : GPIO_E11 - DXB1_RST# - JR21[23]
     */
    BITCLR(*(volatile unsigned int *)(0xF0102128), Hw16-Hw0);			// gpio mode
    BITSET(*(volatile unsigned int *)(0xF0102104), Hw11|Hw10|Hw9|Hw8);	// output

	if (init_val) {
		BITSET(*(volatile unsigned int *)(0xF0102100), Hw11|Hw10|Hw9|Hw8);	// high
	} else {
		BITCLR(*(volatile unsigned int *)(0xF0102100), Hw11|Hw10|Hw9|Hw8);	// low
	}
}

static inline void tcc_gpio_ctrl(int port, int val)
{
    if (val) {
        BITSET(*(volatile unsigned int *)(0xF0102108), port);   // SET reg -> high
    } else {
        BITSET(*(volatile unsigned int *)(0xF010210C), port);   // CLEAR reg -> low
    }
}

static inline void tcc_gpio_toggle(int port)
{
	BITSET(*(volatile unsigned int *)(0xF0102110), port);	// XOR reg -> toggle
}

#endif /*__TCC_DEBUG_H__*/
