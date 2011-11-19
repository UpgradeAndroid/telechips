#define VIQE_DEV_NAME		"viqe"
#define VIQE_DEV_MAJOR		230
#define VIQE_DEV_MINOR		0

#define IOCTL_VIQE_INITIALIZE		0xAF0
#define IOCTL_VIQE_EXCUTE			0xAF1
#define IOCTL_VIQE_DEINITIALIZE		0xAF2
#define IOCTL_VIQE_SETTING			0xAF3
/*****************************************************************************
*
* structures
*
******************************************************************************/
static int tcc_viqe_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

