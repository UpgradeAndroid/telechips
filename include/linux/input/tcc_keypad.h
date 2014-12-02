#ifndef TCC_KEYPAD_H
#define TCC_KEYPAD_H

struct tcc_key_info {
	// virtual key code
	int code;
	// start and end ADC values
	unsigned int start;
	unsigned int end;
};

struct tcc_keypad_platform_data {
	int adc_channel;
	struct tcc_key_info *keys;
	int num_keys;
};

#endif // TCC_KEYPAD_H
