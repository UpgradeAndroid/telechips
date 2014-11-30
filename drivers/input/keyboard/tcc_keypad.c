/*
 * linux/drivers/input/keyboard/tcc-keys.c
 *
 * Based on: drivers/input/keyboard/bf54x-keys.c
 * Author: <linux@telechips.com>
 * Created: June 10, 2008
 * Description: Keypad ADC driver on Telechips TCC Series
 *
 * Copyright (C) 2008-2009 Telechips 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input-polldev.h>
#include <linux/platform_device.h>
#include <linux/input/tcc_keypad.h>

#include <asm/mach-types.h>
#include <mach/tcc_adc.h>

/* For id.version */
#define TCCKEYVERSION	0x0001
#define DRV_NAME	"tcc-keypad"

#define KEY_RELEASED	0
#define KEY_PRESSED	1

struct tcc_private {
	struct input_dev *input_dev;
	struct input_polled_dev *poll_dev;
   	struct tcc_adc_client *client;
	struct platform_device *pdev;

	int key_pressed;
	int old_key;
	short status;
	//
	int adc_channel;
	int num_keys;
	struct tcc_key_info *keys;
};

struct tcc_private *tcc_private;
static struct tcc_adc_client *client;

int tcc_keypad_getkeycodebyscancode(unsigned int adcdata)
{
	int i, key = -1;

	for (i = 0; i < tcc_private->num_keys; i++)
		if (adcdata >= tcc_private->keys[i].start && adcdata <= tcc_private->keys[i].end)
			key = tcc_private->keys[i].code;

	return key;
}

static void tcc_keypad_poll_callback(struct input_polled_dev *dev)
{
	struct tcc_adc_client *client = tcc_private->client;
	int key = -1;

	if (client == NULL)
		return;

	key = tcc_keypad_getkeycodebyscancode(
		tcc_adc_start(client, tcc_private->adc_channel, 7) );
	if (key >= 0) {
		if (tcc_private->old_key == key) {
			tcc_private->key_pressed = key;
			input_report_key(tcc_private->poll_dev->input, tcc_private->key_pressed, KEY_PRESSED);
			tcc_private->status = KEY_PRESSED;
		} else {
			input_report_key(tcc_private->poll_dev->input, tcc_private->key_pressed, KEY_RELEASED);
			tcc_private->status = KEY_RELEASED;
		}
	} else {
		if (tcc_private->key_pressed >= 0) {
			input_report_key(tcc_private->poll_dev->input, tcc_private->key_pressed, KEY_RELEASED);
			tcc_private->key_pressed =  -1;
			tcc_private->status = KEY_RELEASED;
		}
	}

	input_sync(tcc_private->poll_dev->input);
	tcc_private->old_key = key;
}

static void tcc_keypadpad_select(unsigned int selected)
{
}

static void tcc_keypadpad_convert(unsigned int d0, unsigned int d1)
{
}

static int __devinit tcc_keypad_probe(struct platform_device *pdev)
{
	struct tcc_adc_client *client = NULL;
	struct input_polled_dev *poll_dev;
	struct input_dev *input_dev;
	int error;
	int  i;

	tcc_private = kzalloc(sizeof(struct tcc_private), GFP_KERNEL);
	poll_dev = input_allocate_polled_device();
	if (!tcc_private || !poll_dev) {
		error = -ENOMEM;
		goto fail;
	}

	platform_set_drvdata(pdev, tcc_private);

	client = tcc_adc_register(pdev, tcc_keypadpad_select, tcc_keypadpad_convert, 0);
	if (IS_ERR(client)) {
		error = PTR_ERR(client);
		goto fail;
	}
    
	poll_dev->private = tcc_private;
	poll_dev->poll = tcc_keypad_poll_callback;
	poll_dev->poll_interval = msecs_to_jiffies(20);

	input_dev = poll_dev->input;
	input_dev->evbit[0] = BIT(EV_KEY);
	input_dev->name = "telechips keypad";
	input_dev->phys = "tcc-keypad";
	input_dev->id.version = TCCKEYVERSION;

	for (i = 0; i < tcc_private->num_keys; i++)
		set_bit(tcc_private->keys[i].code, input_dev->keybit);

	tcc_private->poll_dev	= poll_dev;
	tcc_private->key_pressed= -1;
	tcc_private->input_dev	= input_dev;
	tcc_private->client	= client;
	tcc_private->pdev	= pdev;

	input_register_polled_device(tcc_private->poll_dev);

	tcc_adc_start(client, tcc_private->adc_channel, 7);

	return 0;

fail:
	kfree(tcc_private);
	tcc_adc_release(client);
	input_free_polled_device(poll_dev);

	return error;
}

static int __devexit tcc_keypad_remove(struct platform_device *pdev)
{
	input_unregister_polled_device(tcc_private->poll_dev);
	kfree(tcc_private);
	tcc_adc_release(client);
	return 0;
}

static struct platform_driver tcc_keypad_driver = {
       .driver         = {
	       .name   = "tcc-keypad",
	       .owner  = THIS_MODULE,
       },
       .probe          = tcc_keypad_probe,
       .remove         = tcc_keypad_remove,
};

int __init tcc_keypad_init(void)
{
	pr_info("Telechips ADC Keypad Driver\n");
	return platform_driver_register(&tcc_keypad_driver);
}
module_init(tcc_keypad_init);

void __exit tcc_keypad_exit(void)
{
	platform_driver_unregister(&tcc_keypad_driver);
}
module_exit(tcc_keypad_exit);

MODULE_AUTHOR("linux@telechips.com");
MODULE_DESCRIPTION("Telechips keypad driver");
MODULE_LICENSE("GPL");
