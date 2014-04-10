#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/vmalloc.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/module.h>

#include <asm/setup.h>

#define TCC_MAX_PARTITIONS 16
#define TCC_PART_NAME_LEN 16

#define GMC_NAND_NOMAL_BOOT_TYPE 0x54435342 // TCSB
#define GMC_NAND_SECURE_BOOT_TYPE 0x54435353 // TCSS

struct tcc_golden_info {
	u32 signature;		/* 'TCSB' / 'TCSS' */
	u16 page_size;
	u16 spare_size;
	u16 ppage;		/* Page size / 512 */
	u16 ppb;		/* Page per block */
	u32 ppb_for_address;
	u16 ecc_type;
	u16 ecc_data_size;
	u32 target_address;
	u32 bootloader_size;
	u32 last_block_ppb;
	u16 rom_copy_num;
	u16 rom_block_num;
	u32 rom_crc;
	u32 block_upper_limit;
	u32 block_lower_limit;
	u32 golden_info_crc;
	u32 secure_data[4];
};

struct tcc_ptbl_entry {
	char name[TCC_PART_NAME_LEN];
	u32 offset;
	u32 size;
	u32 flags;
};

static u8 partition_data[1024];
static int partition_data_size = 0;

static int directory = -1;

static int parse_tcc_partitions(struct mtd_info *master,
				struct mtd_partition **pparts,
				struct mtd_part_parser_data *data)
{
	struct tcc_golden_info *buf;
	unsigned long offset;
	size_t retlen;
	int ret;

	if ( directory < 0 ) {
		offset = 0;
		while (mtd_block_isbad(master, offset)) {
			offset += master->erasesize;
			if (offset == master->size) {
			nogood:
				printk(KERN_NOTICE "Failed to find a non-bad block to check for Telechips Golden Info table\n");
				return -EIO;
			}
		}
	} else {
		offset = directory * master->erasesize;
		while (mtd_block_isbad(master, offset)) {
			offset += master->erasesize;
			if (offset == master->size)
				goto nogood;
		}
	}
	buf = vmalloc(master->erasesize);

	if (!buf)
		return -ENOMEM;

	printk(KERN_NOTICE "Searching for Telechips Golden Info table in %s at offset 0x%lx\n",
	       master->name, offset);

	ret = mtd_read(master, offset, master->erasesize, &retlen,
		       (void *)buf);

	if (ret)
		goto out;

	if (retlen != master->erasesize) {
		ret = -EIO;
		goto out;
	}

	/* Check for standard or secure boot signature */
	if (buf->signature == GMC_NAND_NOMAL_BOOT_TYPE || buf->signature == GMC_NAND_SECURE_BOOT_TYPE) {
		/* Now use the ATAG info to build our final list of partitions */
		struct tcc_ptbl_entry *e = (void*)partition_data;
		int nrparts = partition_data_size / sizeof(*e);
		struct mtd_partition *parts;
		char *name;
		int i;

		pr_info("ppb: %d, page_size: %d, ppage: %d\n", buf->ppb, buf->page_size, buf->ppage);
		pr_info("partition_data_size: %d / nrparts: %d\n", partition_data_size, nrparts);

		parts = kzalloc((sizeof(*parts) + TCC_PART_NAME_LEN) * nrparts, GFP_KERNEL);
		if (!parts) {
			ret = -ENOMEM;
			goto out;
		}

		name = (char*)(parts + nrparts);
		for (i=0; i < nrparts; i++, name += TCC_PART_NAME_LEN) {
			strlcpy(name, e->name, TCC_PART_NAME_LEN);
			parts[i].name = name;
			parts[i].offset = e->offset * buf->ppb * buf->page_size;
			parts[i].size = e->size * buf->ppb * buf->page_size;
		}
		*pparts = parts;
		ret = nrparts;
	}

 out:
	vfree(buf);
	return ret;
}

#define ATAG_TCC_PARTITION 0x54434370 /* TCCp*/
static int __init parse_tag_tcc_partition(const struct tag *tag)
{
	partition_data_size = tag->hdr.size - 2;

	if (partition_data_size > sizeof(partition_data))
		partition_data_size = sizeof(partition_data);

	memcpy(partition_data, &tag->u, partition_data_size);

	pr_info("Found TCC partition ATAG: %d\n", partition_data_size);

	return 0;
}
__tagtable(ATAG_TCC_PARTITION, parse_tag_tcc_partition);

static struct mtd_part_parser tcc_parser = {
	.owner = THIS_MODULE,
	.parse_fn = parse_tcc_partitions,
	.name = "tcc",
};

static int __init tcc_parser_init(void)
{
	return register_mtd_parser(&tcc_parser);
}

static void __exit tcc_parser_exit(void)
{
	deregister_mtd_parser(&tcc_parser);
}

module_init(tcc_parser_init);
module_exit(tcc_parser_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ithamar R. Adema <ithamar@upgrade-android.com>");
MODULE_DESCRIPTION("Telechips NAND partitioning parser");
