#ifndef __UOR_TS_H__
#define __UOR_TS_H__

/*
 * @gpio_int		interrupt gpio
 * @x_max		x-resolution
 * @y_max		y-resolution
 */
struct uor_ts_platdata {
	int gpio_int;

	unsigned int x_max;
	unsigned int y_max;
};

#endif
