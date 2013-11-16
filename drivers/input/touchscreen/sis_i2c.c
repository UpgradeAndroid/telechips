//#define DEBUG

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/linkage.h>
#include <linux/slab.h>
#include <linux/earlysuspend.h>
#include <linux/gpio.h>

#include <linux/i2c/sis_i2c.h>

struct sis_point {
	int id;
	unsigned short x, y;
	bool pressure;
	bool width;
	bool touch, pre;
};

struct sis_touch_state {
	int id, fingers;
	bool valid;
	struct sis_point pt[MAX_FINGERS];
	uint16_t crc;
};

struct sis_touch_state *touch_state = NULL;
static struct workqueue_struct *sis_wq;
static struct timer_list *kbd_timer;
struct sis_i2c_driver_data *ts_bak = 0;

struct sis_i2c_driver_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	int is_ts_pendown;
	int button_down;	//if key0 pendown ,button_down = 0.... if no key pendown button_down = -1;
	struct work_struct work;
	struct early_suspend early_suspend;
};

struct sis_ts_platform_data *pdata = NULL;

int sis_read_packet(struct i2c_client *client, uint8_t cmd, uint8_t * buf);
static void sis_tpinfo_clear(struct sis_touch_state *touch_state, int max);
static int sis_ts_suspend(struct i2c_client *client, pm_message_t mesg);
static int sis_ts_resume(struct i2c_client *client);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void sis_ts_early_suspend(struct early_suspend *h)
{
	struct sis_i2c_driver_data *ts;
	ts = container_of(h, struct sis_i2c_driver_data, early_suspend);
}

static void sis_ts_late_resume(struct early_suspend *h)
{
	struct sis_i2c_driver_data *ts;
	ts = container_of(h, struct sis_i2c_driver_data, early_suspend);
}
#endif

int sis_read_packet(struct i2c_client *client, uint8_t cmd, uint8_t * buf)
{
	uint8_t tmpbuf[MAX_READ_BYTE_COUNT] = { 0 };
	int ret = -1;
	uint8_t offset = 0;
	bool re_read = false;
	int8_t byte_count = 0;
	uint8_t fingers = 0;
	struct i2c_msg msg[2];

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = (char *)&cmd;
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = MAX_READ_BYTE_COUNT;
	msg[1].buf = tmpbuf;

	do {
		ret = i2c_transfer(client->adapter, msg, 2);
		dev_dbg(&client->dev, "read: ret = %d tmpbuf: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", ret,
			tmpbuf[0], tmpbuf[1], tmpbuf[2], tmpbuf[3], tmpbuf[4], tmpbuf[5], tmpbuf[6], tmpbuf[7],
			tmpbuf[8], tmpbuf[9], tmpbuf[10], tmpbuf[11], tmpbuf[12], tmpbuf[13], tmpbuf[14], tmpbuf[15]);

		switch (tmpbuf[0]) {
		case NO_T:
		case SINGLE_T:
		case BUTTON_T:
		case LAST_ONE:
			re_read = false;
			break;
		case MULTI_T:
			fingers = (tmpbuf[PKTINFO] & MSK_TOUCHNUM);
			if ((fingers <= 0) || (fingers > MAX_FINGERS)) {
				return -1;
			}
			byte_count = 2 + (fingers * 5) + CRCCNT(fingers);
			byte_count = byte_count - MAX_READ_BYTE_COUNT;
			re_read = byte_count > 0 ? true : false;
			break;
		case LAST_TWO:
			byte_count = byte_count - MAX_READ_BYTE_COUNT;
			re_read = byte_count > 0 ? true : false;
			break;
		default:
			dev_warn(&client->dev, "Unknown bytecount = %d\n", tmpbuf[0]);
			return -1;
			break;
		}

		memcpy(&buf[offset], &tmpbuf[CMD_BASE], tmpbuf[0]);
		offset += tmpbuf[0];
	} while (re_read);

	return ret;
}

static void sis_tpinfo_clear(struct sis_touch_state *touch_state, int max)
{
	int i = 0;

	for (i = 0; i < max; i++) {
		touch_state->pt[i].id = -1;
		touch_state->pt[i].touch = -1;
		touch_state->pt[i].x = 0;
		touch_state->pt[i].y = 0;
		touch_state->pt[i].pressure = 0;
		touch_state->pt[i].width = 0;
	}
	touch_state->crc = 0x0;
}

static void report_penup(unsigned long data)
{
	struct sis_touch_keys_button *buttons = pdata->buttons;
	struct sis_i2c_driver_data *ts = (struct sis_i2c_driver_data *)data;
	int ret;

	ret = gpio_get_value(pdata->intr);
	if (ret) {		/* charge to high level,pendown irq stoped */
		if (ts->is_ts_pendown == 1) {
			dev_dbg(&ts->client->dev, "Reported pen up\n");
			ts->is_ts_pendown = 0;
			input_report_abs(ts->input_dev, ABS_MT_PRESSURE, 0);
			input_sync(ts->input_dev);
		}
		if (ts->button_down >= 0 && ts->button_down < 4) {
			dev_dbg(&ts->client->dev, "report penup key:%d \n",ts->button_down);
			input_report_key(ts->input_dev,
					 buttons[ts->button_down].code, 0);
			input_sync(ts->input_dev);
			ts->button_down = -1;
		}
		return;
	}
	mod_timer(kbd_timer, jiffies + msecs_to_jiffies(5));
}

static int btoi(uint8_t a)
{
	return a & 1 ? 0 : (a & 2 ? 1 : (a & 4 ? 2 : (a & 8 ? 3 : -1)));
}

static void sis_ts_work_func(struct work_struct *work)
{
	struct sis_i2c_driver_data *ts = container_of(work, struct sis_i2c_driver_data, work);
	struct i2c_client *client = ts->client;
	int ret = -1;
	uint8_t buf[64] = { 0 };
	uint8_t i = 0, fingers = 0;
	uint8_t px = 0, py = 0, pstatus = 0;
	struct sis_touch_keys_button *buttons = NULL;

	buttons = pdata->buttons;
	ret = sis_read_packet(ts->client, SIS_CMD_NORMAL, buf);
	if (ret < 0) {
		enable_irq(client->irq);
		return;
	}

	sis_tpinfo_clear(touch_state, MAX_FINGERS);

	if (buf[1] == 0x70) {
		if (btoi(buf[2]) == -1) {	//data err
			enable_irq(client->irq);
			return;
		}

		if (ts->button_down >= 0 && ts->button_down < 4
		    && (ts->button_down != btoi(buf[2]))) {
			//already has a ts->button_down
			dev_dbg(&client->dev, "report penup key:%d \n",ts->button_down);
			input_report_key(ts->input_dev,
					 buttons[ts->button_down].code, 0);
			input_sync(ts->input_dev);
			ts->button_down = -1;
		}
		if (ts->button_down == -1) {
			//interrupt for this key down first time
			ts->button_down = btoi(buf[2]);
			dev_dbg(&client->dev, "report pendown key:%d\n",ts->button_down);
			mod_timer(kbd_timer, jiffies + msecs_to_jiffies(5));
			input_report_key(ts->input_dev,
					 buttons[ts->button_down].code, 1);
			input_sync(ts->input_dev);
			msleep(10);
			enable_irq(client->irq);
			return;
		}
	}

	if (ts->is_ts_pendown == 0) {
		ts->is_ts_pendown = 1;
		mod_timer(kbd_timer, jiffies + msecs_to_jiffies(5));
	}

	fingers = (buf[1] & MSK_TOUCHNUM);
	touch_state->fingers = fingers = (fingers > MAX_FINGERS ? 0 : fingers);

	for (i = 0; i < fingers; i++) {
		pstatus = 2 + (i * 5) + 2 * (i >> 1);
		px = pstatus + 1;
		py = px + 2;
		touch_state->pt[i].pressure =
		    (buf[pstatus] & MSK_PSTATE) == TOUCHDOWN ? 1 : 0;
		touch_state->pt[i].width =
		    (buf[pstatus] & MSK_PSTATE) == TOUCHDOWN ? 1 : 0;
		touch_state->pt[i].id = (buf[pstatus] & MSK_PID) >> 4;
		touch_state->pt[i].x =
		    (((buf[px] & 0xff) << 8) | (buf[px + 1] & 0xff));
		touch_state->pt[i].y =
		    ((((buf[py] & 0xff) << 8) | (buf[py + 1] & 0xff)));
		touch_state->pt[i].x = 4096 - touch_state->pt[i].x; //FIXME calibration hack
		touch_state->pt[i].y = 4096 - touch_state->pt[i].y; //FIXME calibration hack
		dev_dbg(&client->dev, "touch_state->pt[i].pressure = %d, touch_state->pt[i].id = %d, touch_state->pt[i].x = %d, touch_state->pt[i].y = %d\n",
		       touch_state->pt[i].pressure,touch_state->pt[i].id,touch_state->pt[i].x,touch_state->pt[i].y);
		input_report_abs(ts->input_dev, ABS_MT_PRESSURE,
				 touch_state->pt[i].pressure);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_X,
				 touch_state->pt[i].x);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y,
				 touch_state->pt[i].y);
		input_mt_sync(ts->input_dev);
	}
	input_sync(ts->input_dev);
//	msleep(10);
	enable_irq(client->irq);

	return;
}

static irqreturn_t sis_ts_irq_handler(int irq, void *dev_id)
{
	struct sis_i2c_driver_data *ts = dev_id;
	struct i2c_client *client = ts->client;
	int ret;

	if (!work_pending(&ts->work)) {
		disable_irq_nosync(client->irq);
		ret = queue_work(sis_wq, &ts->work);
	}

	return IRQ_HANDLED;
}

static inline void start_ts(void)
{
	struct i2c_client *client = ts_bak->client;

	if (gpio_is_valid(pdata->wakeup)) {
		gpio_direction_output(pdata->wakeup, 0);
		msleep(20);
	}

	if (gpio_is_valid(pdata->power)) {
		gpio_direction_output(pdata->power, 1);
		msleep(50);
	}

	if (gpio_is_valid(pdata->wakeup)) {
		gpio_set_value(pdata->wakeup, 1);
		msleep(20);
	}

	enable_irq(client->irq);
}

static inline void stop_ts(void)
{
	struct i2c_client *client = ts_bak->client;
	disable_irq(client->irq);
	if (gpio_is_valid(pdata->power))
		gpio_direction_output(pdata->power, 0);
}

static int sis_ts_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct sis_i2c_driver_data *ts = NULL;
	int ret = 0;
	struct sis_touch_keys_button *buttons = NULL;	//pdata->buttons;
	int nbuttons = 0;	//pdata->nbuttons;
	int i;

	touch_state = kzalloc(sizeof(struct sis_touch_state), GFP_KERNEL);
	if (touch_state == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
	ts_bak = ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	pdata = client->dev.platform_data;
	ts->client = client;
	i2c_set_clientdata(client, ts);

	start_ts();
	//workqueue
	INIT_WORK(&ts->work, sis_ts_work_func);
	sis_wq = create_singlethread_workqueue("sis_wq");
	if (!sis_wq) {
		ret = -ESRCH;
		goto exit_create_singlethread;
	}
	//input_dev
	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		goto err_input_dev_alloc_failed;
	}
	ts->input_dev->name = "SiS 81x family I2C touchscreen";

	set_bit(EV_ABS, ts->input_dev->evbit);

	buttons = pdata->buttons;
	nbuttons = pdata->nbuttons;

	if (nbuttons) {
		set_bit(EV_KEY, ts->input_dev->evbit);
		for (i = 0; i < nbuttons; i++) {
			if (buttons[i].code)
				set_bit(buttons[i].code, ts->input_dev->keybit);
		}
	}
	//set_bit(KEY_BACK, ts->input_dev->keybit);

	set_bit(ABS_MT_PRESSURE, ts->input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, ts->input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, ts->input_dev->absbit);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, 4096, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, 4096, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE, 0, 1, 0, 0);

	ret = input_register_device(ts->input_dev);
	if (ret) {
		dev_err(&client->dev, "Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}

	gpio_direction_input(pdata->intr);
	if (client->irq) {
		ret =
		    request_irq(client->irq, sis_ts_irq_handler, IRQF_DISABLED | IRQF_TRIGGER_FALLING,
				client->name, ts);
		if (ret < 0) {
			dev_err(&client->dev, "unable to request IRQ %d!\n", client->irq);
			goto err_request_irq;
		}
	}
	ts->is_ts_pendown = 0;
	ts->button_down = -1;

	kbd_timer = kzalloc(sizeof(struct timer_list), GFP_KERNEL);
	if (!kbd_timer)
		goto exit_timer_list_failed;
	setup_timer(kbd_timer, report_penup, (unsigned long)ts);

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = sis_ts_early_suspend;
	ts->early_suspend.resume = sis_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

	dev_info(&client->dev, "Start touchscreen %s\n", ts->input_dev->name);

	return 0;

exit_timer_list_failed:
err_request_irq:
err_input_register_device_failed:
	input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
	cancel_work_sync(&ts->work);
	destroy_workqueue(sis_wq);
exit_create_singlethread:
	kfree(ts);
err_alloc_data_failed:
	return ret;
}

static int sis_ts_remove(struct i2c_client *client)
{
	struct sis_i2c_driver_data *ts = i2c_get_clientdata(client);

	stop_ts();
	if (sis_wq) {
		destroy_workqueue(sis_wq);
	}
	unregister_early_suspend(&ts->early_suspend);
	free_irq(client->irq, ts);
	input_unregister_device(ts->input_dev);
	kfree(ts);

	return 0;
}

static int sis_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;
	struct sis_i2c_driver_data *ts = i2c_get_clientdata(client);

	stop_ts();
	disable_irq(client->irq);
	ret = cancel_work_sync(&ts->work);
	if (ret) {
		enable_irq(client->irq);
	}
	return 0;
}

static int sis_ts_resume(struct i2c_client *client)
{
	start_ts();
	enable_irq(client->irq);
	return 0;
}

static const struct i2c_device_id sis_ts_id[] = {
	{SIS_I2C_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, sis_ts_id);

static struct i2c_driver sis_ts_driver = {
	.probe = sis_ts_probe,
	.remove = sis_ts_remove,
	//#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = sis_ts_suspend,
	.resume = sis_ts_resume,
	//#endif
	.id_table = sis_ts_id,
	.driver = {
		   .name = SIS_I2C_NAME,
		   },
};

static int __devinit sis_ts_init(void)
{
	return i2c_add_driver(&sis_ts_driver);
}

static void __exit sis_ts_exit(void)
{
	i2c_del_driver(&sis_ts_driver);
}

module_init(sis_ts_init);
module_exit(sis_ts_exit);

MODULE_DESCRIPTION("SiS 81x Family Touchscreen Driver");
MODULE_LICENSE("GPL");
