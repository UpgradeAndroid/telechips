#ifndef EGALAX_I2C_H
#define EGALAX_I2C_H

struct egalax_i2c_platform_data {
	unsigned int gpio_int;
	unsigned int gpio_en;
	unsigned int gpio_rst;
};

#endif /* EGALAX_I2C_H */
