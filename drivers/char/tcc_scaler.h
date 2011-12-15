/****************************************************************************
 *	 TCC Version 0.6
 *	 Copyright (c) telechips, Inc.
 *	 ALL RIGHTS RESERVED
 *
****************************************************************************/

#define TRUE   1
#define FALSE  0

static int tccxxx_scaler_mmap(struct file *file, struct vm_area_struct *vma);
long tccxxx_scaler_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int tccxxx_scaler_release(struct inode *inode, struct file *filp);
int tccxxx_scaler_open(struct inode *inode, struct file *filp);
extern void test_scaler_init(void);

