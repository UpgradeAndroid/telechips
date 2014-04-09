#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/clk.h>
#include <linux/log2.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define RTCCON		0x00
#define INTCON		0x04
#define RTCALM		0x08
#define ALMSEC		0x0c
#define ALMMIN		0x10
#define ALMHOUR		0x14
#define ALMDATE		0x18
#define ALMDAY		0x1c
#define ALMMON		0x20
#define ALMYEAR		0x24
#define BCDSEC		0x28
#define BCDMIN		0x2c
#define BCDHOUR		0x30
#define BCDDATE		0x34
#define BCDDAY		0x38
#define BCDMON		0x3c
#define BCDYEAR		0x40
#define RTCIM		0x44
#define RTCPEND		0x48
#define RTCSTR		0x4c

static int alarm_irq;
static void __iomem *regs;
static struct clk *clk;

static irqreturn_t tcc_alarm_interrupt(int irq, void *dev_id)
{
	writel(readl(regs + RTCCON) | BIT(1), regs + RTCCON);	// RTC Register write enabled
	writel(readl(regs + INTCON) | BIT(0), regs + INTCON);	// Interrupt Block Write Enable  

	writel(readl(regs + RTCIM) & ~(BIT(3)|BIT(1)|BIT(0)), regs + RTCIM); // INTMODE-DISABLE ALARM INTRRUPT & Operation mode - normal 

	writel(0, regs + RTCPEND); // PEND bit is cleared.

	writel(readl(regs + RTCSTR) | BIT(6) , regs + RTCSTR); // RTC Alarm interrupt pending clear
	writel(readl(regs + RTCSTR) | BIT(7) , regs + RTCSTR); // RTC wake-up interrupt pending clear

	pr_debug("RTCSTR[0x%x]", readl(regs + RTCSTR));
	pr_debug("RTCPEND[0x%x]", readl(regs + RTCPEND));	

	writel(readl(regs + INTCON) & ~BIT(0), regs + INTCON); // Interrupt Block Write disable  
	writel(readl(regs + RTCCON) & ~BIT(1), regs + RTCCON); // RTC Register write disable  

	rtc_update_irq(dev_id, 1, RTC_AF | RTC_IRQF);

	return IRQ_NONE;
}

static void tcc_rtc_enable(struct platform_device *pdev, int on)
{
	int timeout;
	u32 reg;

	writel(readl(regs + RTCCON) | BIT(1), regs + RTCCON);
	writel(readl(regs + INTCON) | BIT(0), regs + INTCON);

	for (timeout=1000; timeout > 0 && (readl(regs + INTCON) & 0x0000B700) != 0x00009000; timeout--) {
		writel(readl(regs + RTCCON) | BIT(0), regs + RTCCON);
		writel(readl(regs + INTCON) &  ~BIT(15), regs + INTCON);

		reg = readl(regs + INTCON);
		reg &= ~0x00003700;
		reg |= 0x00001000;
		writel(reg, regs + INTCON);
	}

	if ((readl(regs + INTCON) & BIT(15)) == 0) {
		writel(readl(regs + RTCCON) | BIT(0), regs + RTCCON);
		writel(readl(regs + INTCON) | BIT(15), regs + INTCON);
	}
		
	writel(readl(regs + RTCCON) & ~BIT(0), regs + RTCCON);
	writel(readl(regs + INTCON) & ~BIT(0), regs + INTCON);
	writel(readl(regs + RTCCON) & ~BIT(1), regs + RTCCON);
}

/* Update control registers */
static int tcc_rtc_setaie(struct device *dev, unsigned int enabled)
{
	u32 reg;

	writel(readl(regs + RTCCON) | BIT(1), regs + RTCCON);
	writel(readl(regs + INTCON) | BIT(0), regs + INTCON);

	reg = readl(regs + RTCALM) & ~BIT(7);
	if (enabled)
		reg |= BIT(7);
	writel(reg, regs + RTCALM);

	writel(readl(regs + INTCON) & ~BIT(0), regs + INTCON);
	writel(readl(regs + RTCCON) & ~BIT(1), regs + RTCCON);

	return 0;
}

static int tcc_rtc_gettime(struct device *dev, struct rtc_time *rtc_tm)
{
	writel(readl(regs + RTCCON) | BIT(1), regs + RTCCON);
	writel(readl(regs + INTCON) | BIT(0), regs + INTCON);
	writel(readl(regs + INTCON) & ~BIT(4), regs + INTCON);

	do {
		rtc_tm->tm_sec = readl(regs + BCDSEC) & 0x7f;
		rtc_tm->tm_min = readl(regs + BCDMIN) & 0x7f;
		rtc_tm->tm_hour = readl(regs + BCDHOUR) & 0x3f;
		rtc_tm->tm_wday = readl(regs + BCDDAY) & 0x07;
		rtc_tm->tm_mday = readl(regs + BCDDATE) & 0x3f;
		rtc_tm->tm_mon = readl(regs + BCDMON) & 0x1f;
		rtc_tm->tm_year = readl(regs + BCDYEAR) & 0xffff;
	} while(rtc_tm->tm_sec != (readl(regs + BCDSEC) & 0x7f));

	rtc_tm->tm_sec = bcd2bin(rtc_tm->tm_sec);
	rtc_tm->tm_min = bcd2bin(rtc_tm->tm_min);
	rtc_tm->tm_hour = bcd2bin(rtc_tm->tm_hour);
	rtc_tm->tm_wday = bcd2bin(rtc_tm->tm_wday);
	rtc_tm->tm_mday = bcd2bin(rtc_tm->tm_mday);
	rtc_tm->tm_mon = bcd2bin(rtc_tm->tm_mon);
	rtc_tm->tm_year = bcd2bin(rtc_tm->tm_year >> 8) * 100 +
		bcd2bin(rtc_tm->tm_year & 0xff);

	rtc_tm->tm_mon--;
	rtc_tm->tm_year -= 1900;

	writel(readl(regs + INTCON) & ~BIT(0), regs + INTCON);
	writel(readl(regs + INTCON) & ~BIT(1), regs + RTCCON);

	return 0;
}

static int tcc_rtc_settime(struct device *dev, struct rtc_time *tm)
{
	writel(readl(regs + RTCCON) | BIT(1), regs + RTCCON);
	writel(readl(regs + INTCON) | BIT(0), regs + INTCON);

	writel(readl(regs + RTCCON) | BIT(0), regs + RTCCON);	// RTC Start Halt
	writel(readl(regs + RTCCON) | BIT(4), regs + RTCCON);	// RTC Clock Count Reset
	writel(readl(regs + RTCCON) & ~BIT(4), regs + RTCCON);	// RTC Clock Count No Reset
	writel(readl(regs + RTCCON) | BIT(4), regs + RTCCON);	// RTC Clock Count Reset

	writel(readl(regs + INTCON) | BIT(15), regs + INTCON);	// RTC Protection - enable
	writel(readl(regs + INTCON) & ~BIT(15), regs + INTCON);	// RTC Protection - disable

	writel(bin2bcd(tm->tm_sec), regs + BCDSEC);
	writel(bin2bcd(tm->tm_min), regs + BCDMIN);
	writel(bin2bcd(tm->tm_hour), regs + BCDHOUR);
	writel(bin2bcd(tm->tm_wday), regs + BCDDAY);
	writel(bin2bcd(tm->tm_mday), regs + BCDDATE);
	writel(bin2bcd(tm->tm_mon +1), regs + BCDMON);
	writel(	(bin2bcd((tm->tm_year + 1900) / 100) << 8)
		| bin2bcd(tm->tm_year % 100), regs + BCDYEAR);

	writel(readl(regs + INTCON) | BIT(15), regs + INTCON);	// RTC Protection - enable
	writel(readl(regs + RTCCON) & ~BIT(0), regs + RTCCON);	// RTC Start RUN
	writel(readl(regs + INTCON) & ~BIT(0), regs + INTCON);	// Interrupt Block Write - disable
	writel(readl(regs + RTCCON) & ~BIT(1), regs + RTCCON);	// RTC Write - disable

	return 0;
}

static int tcc_rtc_getalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	u32 alm_en, alm_pnd;
	u32 almbits;

	writel(readl(regs + RTCCON) | BIT(1), regs + RTCCON);
	writel(readl(regs + INTCON) | BIT(0), regs + INTCON);

	alm_en = readl(regs + RTCALM);
	alm_pnd = readl(regs + RTCPEND);

	alrm->enabled = !!(alm_en & BIT(7));
	alrm->pending = !!(alm_pnd & BIT(0));

	pr_debug(" alrm->enabled = %d, alm_en = 0x%08x\n", alrm->enabled, alm_en);

	alrm->time.tm_sec	= readl(regs + ALMSEC);
	alrm->time.tm_min	= readl(regs + ALMMIN);
	alrm->time.tm_hour	= readl(regs + ALMHOUR);
	alrm->time.tm_mday	= readl(regs + ALMDATE);
	alrm->time.tm_wday	= readl(regs + ALMDAY);
	alrm->time.tm_mon	= readl(regs + ALMMON);
	alrm->time.tm_year	= readl(regs + ALMYEAR);

	almbits = readl(regs + RTCALM);

	writel(readl(regs + INTCON) & ~BIT(0), regs + INTCON);	// Interrupt Block Write Disable
	writel(readl(regs + RTCCON) & ~BIT(1), regs + RTCCON);	// RTC Register write Disable

	alrm->time.tm_sec  = (almbits & BIT(0)) ? bcd2bin(alrm->time.tm_sec) : 0;
	alrm->time.tm_min  = (almbits & BIT(1)) ? bcd2bin(alrm->time.tm_min) : 0;
	alrm->time.tm_hour = (almbits & BIT(2)) ? bcd2bin(alrm->time.tm_hour) : 0;
	alrm->time.tm_mday = (almbits & BIT(3)) ? bcd2bin(alrm->time.tm_mday) : 0;
	alrm->time.tm_wday = (almbits & BIT(4)) ? bcd2bin(alrm->time.tm_wday) : 0;
	alrm->time.tm_mon  = (almbits & BIT(5)) ? bcd2bin(alrm->time.tm_mon) -1 : 0;
	alrm->time.tm_year  = (almbits & BIT(6)) ? (bcd2bin(alrm->time.tm_year >> 8) * 100 +
							bcd2bin(alrm->time.tm_year & 0xff)) - 1900 : 0;

	writel(almbits, regs + RTCALM);

	writel(readl(regs + INTCON) & ~BIT(0), regs + INTCON);
	writel(readl(regs + RTCCON) & ~BIT(1), regs + RTCCON);

	return 0;
}

static int tcc_rtc_setalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	u32 reg;

	alrm->enabled = 1;

	writel(readl(regs + RTCCON) | BIT(1), regs + RTCCON);
	writel(readl(regs + INTCON) | BIT(0), regs + INTCON);

	reg = 0xef; // Not wDayOfWeek

	if (alrm->time.tm_sec  > 59) reg &= ~BIT(0); else writel(bin2bcd(alrm->time.tm_sec), regs + ALMSEC);
	if (alrm->time.tm_min  > 59) reg &= ~BIT(1); else writel(bin2bcd(alrm->time.tm_min), regs + ALMMIN);
	if (alrm->time.tm_hour > 23) reg &= ~BIT(2); else writel(bin2bcd(alrm->time.tm_hour), regs + ALMHOUR);
	if (alrm->time.tm_mday > 31 || alrm->time.tm_mday < 1) reg &= ~BIT(3); else writel(bin2bcd(alrm->time.tm_mday), regs + ALMDATE);
	if (alrm->time.tm_wday > 6)  reg &= ~BIT(4); else writel(bin2bcd(alrm->time.tm_wday +1), regs + ALMDAY);
	if (alrm->time.tm_mon  > 11) reg &= ~BIT(5); else writel(bin2bcd(alrm->time.tm_mon), regs + ALMMON);
	if (alrm->time.tm_year > 199) reg &= ~BIT(6); else writel((bin2bcd(alrm->time.tm_year + 1900) / 100) << 8 |
								bin2bcd(alrm->time.tm_year % 100), regs + ALMYEAR);
	// Enable ALARM
	writel(reg, regs + RTCALM);

#if 0
	#if 1	//for One-Shot write to INTMODE - 110106, hjbae
	if(machine_is_tcc8800() || machine_is_m801_88() || machine_is_m803())
	{
		#ifdef CONFIG_ARCH_TCC88XX
		if (tcc88xx_chip_rev() == TCC88XX_REV0) { // rev. 0X
			#if defined(TCC_PM_SLEEP_WFI_USED)
			BITSET(pRTC->RTCIM, Hw2|Hw1|Hw0);	//INTMODE : NormalMode / ActiveHigh / LevelTrigger
			#else
			BITSET(pRTC->RTCIM, Hw3|Hw2|Hw1|Hw0);	//INTMODE : PowerDownMode / ActiveHigh / LevelTrigger
			#endif
		}else{ // rev. AX
			#if defined(CONFIG_SLEEP_MODE) && defined(TCC_PM_SLEEP_WFI_USED)
			BITSET(pRTC->RTCIM, Hw2|Hw1|Hw0);	//INTMODE : NormalMode / ActiveHigh / LevelTrigger
			#else
			BITSET(pRTC->RTCIM, Hw3|Hw2|Hw1|Hw0);	//INTMODE : PowerDownMode / ActiveHigh / LevelTrigger
			#endif
		}
		#else
			BITSET(pRTC->RTCIM, Hw3|Hw2|Hw1|Hw0);	//INTMODE : PowerDownMode / ActiveHigh / LevelTrigger
		#endif
	}
	else
		BITSET(pRTC->RTCIM, Hw3|Hw2|Hw1|Hw0);	//INTMODE : PowerDownMode / ActiveHigh / LevelTrigger
	#else
	BITSET(pRTC->RTCIM, Hw1|Hw0);	//INTMODE : Supports on the level alarm interrupt.
	BITSET(pRTC->RTCIM, Hw3);		// Operation mode - Power Down Mode for PMU RTCWKUP
	BITSET(pRTC->RTCIM, Hw2);		// Wakeup mode selection bit - PMWKUP active high
	#endif
#endif

	/* alarm clear */
	writel(0, regs + RTCPEND);
	writel(readl(regs + RTCSTR) | BIT(0), regs + RTCSTR);

	// Alarm Interrupt Output Enable (Hw6) Wake-up Output Enable (Hw7)
	writel(readl(regs + RTCCON) | BIT(6) | BIT(7), regs + RTCCON);

	if (!(readl(regs + INTCON) & BIT(15))) {
		writel(readl(regs + RTCCON) | BIT(0), regs + RTCCON);
		writel(readl(regs + INTCON) | BIT(15), regs + RTCCON);
		writel(readl(regs + RTCCON) & ~BIT(0), regs + RTCCON);
	}

	#if 1	//for RMWKUP -> RTC Alarm Interrupt - 110106, hjbae
	writel(readl(regs + INTCON) & ~BIT(0), regs + INTCON);
	writel(readl(regs + RTCCON) & ~BIT(1), regs + RTCCON);
	#endif

	writel(readl(regs + RTCCON) | BIT(1), regs + RTCCON);
	writel(readl(regs + INTCON) | BIT(0), regs + INTCON);

	if (alrm->enabled) {
		writel(readl(regs + RTCALM) | BIT(7), regs + RTCALM);
		enable_irq_wake(alarm_irq);
	} else {
		writel(readl(regs + RTCALM) & ~BIT(7), regs + RTCALM);
		disable_irq_wake(alarm_irq);
	}

	writel(readl(regs + INTCON) & ~BIT(0), regs + INTCON);
	writel(readl(regs + RTCCON) & ~BIT(1), regs + RTCCON);

	return 0;
}

static int tcc_rtc_proc(struct device *dev, struct seq_file *seq)
{
	return 0;
}

static const struct rtc_class_ops tcc_rtcops = {
	.read_time	= tcc_rtc_gettime,
	.set_time	= tcc_rtc_settime,
	.read_alarm	= tcc_rtc_getalarm,
	.set_alarm	= tcc_rtc_setalarm,
	.proc		= tcc_rtc_proc,
	.alarm_irq_enable = tcc_rtc_setaie,
};

static int tcc_rtc_probe(struct platform_device *pdev)
{
	struct rtc_device *rtc;
	struct rtc_time rtc_tm;
	struct resource *res;
	int ret;

	/* find the IRQs */
	alarm_irq = platform_get_irq(pdev, 0);
	if (alarm_irq < 0) {
		dev_err(&pdev->dev, "no irq for alarm\n");
		return alarm_irq;
	}

	/* get the memory region */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(regs))
		return PTR_ERR(regs);

	clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "failed to find rtc clock source\n");
		ret = PTR_ERR(clk);
		clk = NULL;
		return ret;
	}
	clk_prepare_enable(clk);

	/* check to see if everything is setup correctly */
	tcc_rtc_enable(pdev, 1);
	device_init_wakeup(&pdev->dev, 1);

	/* register RTC and exit */
	rtc = devm_rtc_device_register(&pdev->dev, "rtc", &tcc_rtcops,
				  THIS_MODULE);
	if (IS_ERR(rtc)) {
		dev_err(&pdev->dev, "cannot attach rtc\n");
		ret = PTR_ERR(rtc);
		goto err_nortc;
	}

	/* Check RTC Time */
	tcc_rtc_gettime(NULL, &rtc_tm);
	if (rtc_valid_tm(&rtc_tm)) {
		rtc_tm.tm_year	= 100;
		rtc_tm.tm_mon	= 0;
		rtc_tm.tm_mday	= 1;
		rtc_tm.tm_hour	= 0;
		rtc_tm.tm_min	= 0;
		rtc_tm.tm_sec	= 0;
		tcc_rtc_settime(NULL, &rtc_tm);

		dev_warn(&pdev->dev, "warning: invalid RTC value so initializing it\n");
	}

	rtc->max_user_freq = 128;

	platform_set_drvdata(pdev, rtc);

	ret = devm_request_irq(&pdev->dev, alarm_irq, tcc_alarm_interrupt, 0,  "rtc-alarm", rtc);
	if (ret) {
		dev_err(&pdev->dev, "IRQ%d error %d\n", alarm_irq, ret);
		goto err_alarm_irq;
	}

	clk_disable(clk);

	return 0;

 err_alarm_irq:
	platform_set_drvdata(pdev, NULL);

 err_nortc:
	tcc_rtc_enable(pdev, 0);
	clk_disable_unprepare(clk);

	return ret;
}

static int tcc_rtc_remove(struct platform_device *dev)
{
	platform_set_drvdata(dev, NULL);

	tcc_rtc_setaie(&dev->dev, 0);

	clk_unprepare(clk);
	clk = NULL;

	return 0;
}

static const struct of_device_id tcc_rtc_dt_match[] = {
	{ .compatible = "tcc,tcc92xx-rtc", },
	{}
};

static struct platform_driver tcc_rtc_driver = {
	.probe		= tcc_rtc_probe,
	.remove		= tcc_rtc_remove,
	.driver		= {
		.name	= "tcc-rtc",
		.owner	= THIS_MODULE,
		.of_match_table	= of_match_ptr(tcc_rtc_dt_match),
	},
};

module_platform_driver(tcc_rtc_driver);

MODULE_DESCRIPTION("Telechips RTC Driver");
MODULE_AUTHOR("Ithamar R. Adema <ithamar@upgrade-android.com>");
MODULE_LICENSE("GPL");
