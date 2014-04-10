//#define DEBUG

#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/of_mtd.h>
#include <linux/delay.h>

#define NFC_CMD		0x00
#define NFC_LADDR	0x04
#define NFC_BADDR	0x08
#define NFC_SADDR	0x0c
#define NFC_WDATA	0x10
#define NFC_LDATA	0x20
#define NFC_SDATA	0x40
#define NFC_CTRL	0x50
#define NFC_PSTART	0x54
#define NFC_RSTART	0x58
#define NFC_DSIZE	0x5c
#define NFC_IREQ	0x60
#define NFC_RST		0x64
#define NFC_CTRL1	0x68
#define NFC_MDATA	0x70

#define ECC_CTRL	0x00
#define ECC_BASE	0x04
#define ECC_MASK	0x08
#define ECC_CLEAR	0x0c

struct tcc_nand {
	struct mtd_info	mtd;
	struct nand_chip chip;

	void __iomem *nfc;
	void __iomem *ecc;
	int irq;
};

#define to_tcc_nand(mtd)	container_of(mtd, struct tcc_nand, mtd)

static const char *part_probe_types[] = { "tcc", "ofpart", "cmdlinepart", NULL };

static void tcc_nand_inithw(struct tcc_nand *nand)
{
	/* Reset NFC controller */
	writel_relaxed(0, nand->nfc + NFC_RST);

	writel_relaxed(0
		| (1 << 28)	/* DMA Request Enable */
		| (15 << 22)	/* Disable all CSxSEL */
		| (1 << 19)	/* Burst size 1 */
		| (4 << 4)	/* Base Cycle for Pulse Width 4 */
		| (1 << 0)	/* Base Cycle for Hold Time 1 */
//		| (1 << 31)	/* Ready interrupt */
		| (1 << 30)	/* Program interrupt */
                | (1 << 29)	/* Read interrupt */
		, nand->nfc + NFC_CTRL);

	writel_relaxed(0
		| (1 << 30)	/* Burst Arbitration Enable */
		| (1 << 31)	/* DACK Acknowledge Enable */
		, nand->nfc + NFC_CTRL1);

	/* Clear interrupts */
	writel_relaxed(0x77, nand->nfc + NFC_IREQ);

	/* Setup ECC controller */
	writel_relaxed(0x4000000,
		nand->ecc + ECC_CTRL);
	writel_relaxed(nand->nfc + NFC_WDATA,
		nand->ecc + ECC_BASE);
	writel_relaxed(0, nand->ecc + ECC_MASK);
	// request_irq
	// dma_alloc
}

static irqreturn_t tcc_nand_interrupt(int irq, void *dev)
{
	struct tcc_nand *nand = dev;
	u32 status = readl_relaxed(nand->nfc + NFC_IREQ);
	if (status) {
		// BIT(0) = Read
		// BIT(1) = Program
		// BIT(2) = Ready
		// BIT(4) = Read data xfer
		// BIT(5) = Program data xfer
		// BIT(6) = Rising edge of Ready Signal
		writel_relaxed(status, nand->nfc + NFC_IREQ);
		pr_info("%s: %08x\n", __func__, status);
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

/* Select chip 'chip', or none if chip == -1 */
static void tcc_nand_select_chip(struct mtd_info *mtd, int chip)
{
	struct tcc_nand *nand = to_tcc_nand(mtd);
	u32 reg;

	reg = readl_relaxed(nand->nfc + NFC_CTRL);

	/* Disable all CSn */
	reg |= (15 << 22);
	/* Enable CSn if requested */
	if (chip >= 0)
		reg &= ~(BIT(chip) << 22);
	/* Set buswidth */
	if (nand->chip.options & NAND_BUSWIDTH_16)
		reg |= BIT(26);
	else
		reg |= BIT(15); // MASK

	reg |= 15; // bHLD
	reg |= (15 << 4); // bPW
	reg |= (2 << 8); //bSTP

	writel_relaxed(reg, nand->nfc + NFC_CTRL);
	pr_debug("select_chip: %08x\n", reg);
}

/* Return 1 if chip is ready, or 0 otherwise */
static int tcc_nand_devready(struct mtd_info *mtd)
{
	struct tcc_nand *nand = to_tcc_nand(mtd);
	u32 reg = readl_relaxed(nand->nfc + NFC_CTRL);
	pr_debug("devready: %08x\n", reg);
	return !!(reg & BIT(21));
}

static uint8_t tcc_nand_read_byte(struct mtd_info *mtd)
{
	struct tcc_nand *nand = to_tcc_nand(mtd);
	u32 reg = readl_relaxed(nand->nfc + NFC_SDATA);
	pr_debug("nand_read_byte: %08x\n", reg);
	return reg & 0xff;
}

static void tcc_nand_hwcontrol(struct mtd_info *mtd, int cmd,
				   unsigned int ctrl)
{
	struct tcc_nand *nand = to_tcc_nand(mtd);
	u32 mask = (nand->chip.options & NAND_BUSWIDTH_16) ? 0xffff : 0xff;
#if 0
	pr_info("%s: %x: %s %s %s %s\n", __func__, _cmd,
		(ctrl & NAND_NCE) ? "NCE" : "",
		(ctrl & NAND_CLE) ? "CLE" : "",
		(ctrl & NAND_ALE) ? "ALE" : "",
		(ctrl & NAND_CTRL_CHANGE) ? "*" : "");
#endif
	if (ctrl & NAND_ALE)
		writel_relaxed(cmd & mask, nand->nfc + NFC_SADDR);

	if (ctrl & NAND_CLE)
		writel_relaxed(cmd & mask, nand->nfc + NFC_CMD);
}

static void tcc_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	int i;
	for (i=0; i < len; i++)
		buf[i] = tcc_nand_read_byte(mtd);
}

static void tcc_nand_write_buf(struct mtd_info *mtd, const u_char *buf,
				   int len)
{
}

static void tcc_nand_initchip(struct tcc_nand *nand, struct device_node *np)
{
	struct nand_chip *chip = &nand->chip;

	chip->write_buf    = tcc_nand_write_buf;
	chip->read_buf     = tcc_nand_read_buf;
	chip->read_byte    = tcc_nand_read_byte;
	chip->select_chip  = tcc_nand_select_chip;
	chip->chip_delay   = 50;
	chip->priv	   = nand;
	chip->options	   = 0;

	chip->IO_ADDR_W = NULL;
	chip->IO_ADDR_R = nand->nfc + NFC_SDATA;

	chip->cmd_ctrl  = tcc_nand_hwcontrol;
	chip->dev_ready = tcc_nand_devready;

	chip->ecc.mode  = NAND_ECC_NONE;

	/* See if any nand config in DT */
	if (np) {
		int ecc_mode = of_get_nand_ecc_mode(np);
		if (ecc_mode >= 0)
			chip->ecc.mode = ecc_mode;

		if (of_get_nand_bus_width(np) == 16)
			chip->options |= NAND_BUSWIDTH_16;

		/* If we have a BBT on flash, skip bootup check too */
		if (of_get_nand_on_flash_bbt(np) == 1) {
			chip->options |= NAND_BBT_USE_FLASH;
			chip->options |= NAND_SKIP_BBTSCAN;
		}
	}
}


static const struct of_device_id tcc_nand_match[] = {
	{ .compatible = "tcc,tcc92xx-nfc", NULL },
	{},
};

MODULE_DEVICE_TABLE(of, tcc_nand_match);

static int tcc_nand_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct mtd_part_parser_data ppdata;
	struct tcc_nand *nand;
	int ret = 0;

	nand = kzalloc(sizeof(*nand), GFP_KERNEL);
	if (!nand) {
		ret = -ENOMEM;
		goto err_nockl;
	}

	nand->nfc = of_iomap(np, 0);
	if (!nand->nfc) {
		ret = -EIO;
		goto err_io;
	}

	nand->ecc = of_iomap(np, 1);
	if (!nand->ecc) {
		ret = -EIO;
		goto err_io;
	}

	nand->irq = irq_of_parse_and_map(np, 0);

	tcc_nand_initchip(nand, np);

	nand->mtd.priv = &nand->chip;
	nand->mtd.owner = THIS_MODULE;
	nand->mtd.name = dev_name(&pdev->dev);

	platform_set_drvdata(pdev, nand);

	ret = request_irq(nand->irq, tcc_nand_interrupt, 0, dev_name(&pdev->dev), nand);
	if (ret)
		goto err_io;

	tcc_nand_inithw(nand);

	/* Scan to find existence of the device */
	if (nand_scan(&nand->mtd, 1)) {
		ret = -ENXIO;
		goto err_io;
	}

	ppdata.of_node = pdev->dev.of_node;
	if (!mtd_device_parse_register(&nand->mtd, part_probe_types, &ppdata, NULL, -1))
		return 0;

	nand_release(&nand->mtd);

err_io:
	if (nand->ecc)
		iounmap(nand->ecc);
	if (nand->nfc)
		iounmap(nand->nfc);

	kfree(nand);
err_nockl:
	return ret;
}

static int tcc_nand_remove(struct platform_device *pdev)
{
	struct tcc_nand *nand = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);

	kfree(nand);

	return 0;
}

static struct platform_driver tcc_nand_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "tcc-nand",
		.of_match_table = tcc_nand_match,
	},
	.probe = tcc_nand_probe,
	.remove = tcc_nand_remove,
};

module_platform_driver(tcc_nand_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ithamar R. Adema <ithamar@upgrade-android.com>");
MODULE_DESCRIPTION("Telechips MTD NAND driver");
