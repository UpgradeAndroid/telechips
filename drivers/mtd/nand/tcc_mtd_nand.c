#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/tcc_mtd_nand.h>
#include <asm/mach/flash.h>
#include <asm/io.h>
#include <mach/bsp.h>
#include <mach/gpio.h>
#include <plat/nand.h>

#include <linux/reboot.h>

#if defined(__USE_MTD_NAND_ISR__) 
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/blkpg.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/mutex.h> 

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/scatterlist.h>
#include <asm/mach-types.h>
#include <asm/memory.h>
#include <asm/dma.h>

#include <linux/delay.h>

#define TCC_MTD_IO_IRQ_STATE_NONE			0
#define TCC_MTD_IO_IRQ_STATE_WRITE			1
#define TCC_MTD_IO_IRQ_STATE_READ			2
#define TCC_MTD_IO_IRQ_STATE_WRITE_RB		3
#define TCC_MTD_IO_IRQ_STATE_READ_RB		4

typedef struct _tag_TCC_MTD_ISR_INFO_T {
	TCC_MTD_IO_DEVINFO	*pNandIoDevInfo;
	TCC_MTD_IO_ERROR 	error;
	wait_queue_head_t 	wait_q;
	unsigned char 		*pbPageBuffer;
	unsigned char 		*pbEccBuffer;
	unsigned char 		*pbSpareBuffer;
	unsigned char 		ubIsRun;
	unsigned short 		usCurrentPPage;
	unsigned short 		usStartPPage;
	unsigned short 		usPPagesLeft;
	unsigned int		iEccOnOff;
	unsigned int 		uiState;	
	
	volatile int 		wait_complete;
	
} TCC_MTD_ISR_INFO_T;
static TCC_MTD_ISR_INFO_T sTCC_MTD_IO_NandIsrInfo;

static unsigned int sTCC_MTD_IO_IRQ_fEnable = 0;
#endif

//=============================================================================
//*
//*
//*                           [ GLOBAL VARIABLE DEFINE ]
//*
//*
//=============================================================================
#define TNFTL_V7_INCLUDE

#if defined(TCC_MTD_DMA_ACCESS)
#define TCC_MTD_DMA_BUFFER
#define NAND_IO_USE_DMA_ACCESS
//#define NAND_IO_USE_MCU_ACCESS
#elif defined(__USE_MTD_NAND_ISR__)
#define TCC_MTD_DMA_BUFFER
#endif

#define TCC_MTD_SPARE_ECC
//#define TCC_MTD_DEBUG
//#define TCC_MTD_DEBUG_FUNC
//#define TCC_MTD_DEBUG_SPARE
//#define TCC_MTD_DEBUG_INPUT_OOB
#define DEF_TCC_MTD_EXTEND_PAGE
#define CE_FIX

//=============================================================================
//
// Version Signature
//
//=============================================================================
#define TCC_MTD_IO_VERSION		'V','7','0','2','4'

#define MTD_GLUE_DRIVER_VERSION		0x0001

static const unsigned char 	TCC_MTD_IO_Library_Version[] = 
{ 	
	SIGBYAHONG, 
	TCC_MTD_IO_SIGNATURE, 
	SIGN_OS, 
	SIGN_CHIPSET, 
	TCC_MTD_IO_VERSION, 
	NULL
};


#if defined(TCC89XX) || defined(TCC92XX) || defined(TCC93XX) || defined(TCC88XX)
#include <mach/tca_ckc.h>
//#include "TC_DRV.h"
#endif

#if defined(TCC_MTD_DMA_BUFFER)
#include <linux/kernel.h>
#include <linux/string.h>

struct mtd_dma_buf {
	void *v_addr;
	unsigned int dma_addr;
	int buf_size;
};

struct mtd_dma_buf mtd_dma_t;
#endif

extern struct mutex mutex_tccnfc;
extern unsigned int tcc_get_maximum_io_clock(void);

static struct tcc_nand_platform_data *nand_platform_data;
//#define MUTEX_LOG

#ifdef CONFIG_MTD_PARTITIONS
static const char *part_probes[] = { "cmdlinepart", NULL, };
#endif

static const TCC_MTD_GLUE_DRV_T *s_pTccMtdGlueDrv = NULL;

//==============================================
// Define partitions for flash device
//==============================================
#define MTD_SYSTEM_PART_SIZE			120
#define MTD_CACHE_PART_SIZE				60

#define ASM_NOP {					\
	__asm__ __volatile__ ("nop");	\
	__asm__ __volatile__ ("nop");	\
	__asm__ __volatile__ ("nop");	\
	__asm__ __volatile__ ("nop");	\
	__asm__ __volatile__ ("nop");	\
	__asm__ __volatile__ ("nop");	\
}
 
const unsigned char TNFTL_HIDDEN_AREA_Signature[] =
{ 
#if defined(TCC77X) || defined(TCC87XX) || defined(TCC82XX) || defined(TCC78X) || defined(TCC83XX) || defined(TCC79X)
	"TNFTLHIDDENSIGNATURE"	// ECC: RS
#elif defined(TCC81XX)|| defined(TCC80XX)|| defined(TCC92XX)|| defined(TCC89XX) || defined(TCC93XX) || defined(TCC88XX)
	"TNFTLHIDDENSIG_BCH01"	// ECC: BCH
#endif
};



unsigned char	gSpareBuffer[512] __attribute__((aligned(8)));
unsigned char	gTCC_MTD_IO_TempBuffer[ 44 ] __attribute__((aligned(8)));
unsigned char	gTCC_MTD_IO_ShareEccBuffer[ 1344 ] __attribute__((aligned(8)));
unsigned char 	gTCC_MTD_IO_PreEnDecodeEccBuffer[1044] __attribute__((aligned(8)));
unsigned char 	gTCC_MTD_PageBuffer[TCC_MTD_MAX_SUPPORT_NAND_IO_PAGE_SIZE + TCC_MTD_MAX_SUPPORT_NAND_IO_SPARE_SIZE] __attribute__((aligned(8)));
unsigned char 	gTCC_MTD_ShareBuffer[TCC_MTD_MAX_SUPPORT_NAND_IO_PAGE_SIZE + TCC_MTD_MAX_SUPPORT_NAND_IO_SPARE_SIZE] __attribute__((aligned(8)));

//=============================================================================
//*
//*
//*                           [ CONST DATA DEFINE ]
//*
//*
//=============================================================================
const TCC_MTD_IO_FEATURE	TOSHIBA_NAND_DevInfo[] =
{
   //*=======================================================================================================================================================
    //*[        DEVICE CODE       ][           SIZE          ][               Cycle               ][                  ATTRIBUTE                  ]
    //*-------------------------------------------------------------------------------------------------------------------------------------------------------
    //* 1st, 2nd,  3rd,  4th,  5th,  6th,  PBpV,BBpZ, PpB, Page,Spare,Col,Low,Twc, Ws, Wp, Wh, Rs, Rp, Rh
    //*=======================================================================================================================================================

    // [ 32MB] TC58DVM82A1FT
    { {{0x98, 0x75, 0x00, 0x00, 0x00, 0x00}}, 2048,  20,  32,  512,   16,  1,  2, 50,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_SLC	  	|A_SMALL|S_NOR)			   	},
    // [ 64MB] TC58DVM92A1FT
    { {{0x98, 0x76, 0x00, 0x00, 0x00, 0x00}}, 4096,  20,  32,  512,   16,  1,  3, 50,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_SLC	  	|A_SMALL|S_NOR)			  	},
    // [ 64MB] TC58NWM9S3B
    { {{0x98, 0xF0, 0x00, 0x00, 0x00, 0x00}},  512,  10,  64, 2048,   64,  2,  2, 50,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR)			   	},
    // [128MB] TC58DVG02A1FT
    { {{0x98, 0x79, 0x00, 0x00, 0x00, 0x00}}, 8192,  20,  32,  512,   16,  1,  3, 50,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_SLC	  	|A_SMALL|S_NOR)			   	},	
    // [128MB] TC58NVG0S3AFT
    { {{0x98, 0xF1, 0x00, 0x00, 0x00, 0x00}}, 1024,  20,  64, 2048,   64,  2,  2, 50,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR)			   	},	
    // [256MB] TH58NVG1S3AFT
    { {{0x98, 0xDA, 0x00, 0x00, 0x00, 0x00}}, 2048,  20,  64, 2048,   64,  2,  3, 50,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR)			   	},
    // [512MB] THGVN0G4D1DTG00
    { {{0x98, 0xDC, 0x00, 0x15, 0x00, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 50,	 0, 25, 15,  0, 35, 15, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_LBA)		},
    // [512MB] TH58NYG2S8C
    { {{0x98, 0xBC, 0x91, 0xD5, 0x49, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 50,	 0, 25, 15,  0, 35, 15, (A_16BIT|A_SLC	  	|A_BIG  |S_NOR)			   	},
    // [512MB] TH58NYG2S8E
    { {{0x98, 0xBC, 0x90, 0x55, 0x76, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 50,	 0, 25, 15,  0, 35, 15, (A_16BIT|A_SLC	  	|A_BIG  |S_NOR)			   	},
	// [ 1GB] TC58DVG3S0ET
    { {{0x98, 0xD3, 0x90, 0x26, 0x76, 0x14}}, 4096,  20,  64, 4096,  128,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [512MB] TH58NVG2D4CTG00
    { {{0x98, 0xDC, 0x84, 0xA5, 0x60, 0x00}}, 2048,  40, 128, 2048,   64,  2,  3, 50,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP2)	   	},
    // [512MB] TH58NVG2D4BFT00
    { {{0x98, 0xDC, 0x84, 0xA5, 0x54, 0x00}}, 2048,  40, 128, 2048,   64,  2,  3, 50,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR) 		   	},
    // [  1GB] TH58NVG3D4BFT00
    { {{0x98, 0xD3, 0x85, 0xA5, 0x58, 0x00}}, 4096,  40, 128, 2048,   64,  2,  3, 50,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR) 	  	    },
    // [  1GB] TC58NVG3D4CTG00
    { {{0x98, 0xD3, 0x84, 0xA5, 0x66, 0x00}}, 4096,  40, 128, 2048,   64,  2,  3, 30,	 0, 20, 10,  0, 20, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP) 		},
    // [  2GB] TH58NVG4D4CFT00	[  4GB] TH58NVG5D4CTG20
    { {{0x98, 0xD5, 0x85, 0xA5, 0x00, 0x00}}, 8192,  40, 128, 2048,   64,  2,  3, 30,	 0, 20, 10,  0, 20, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)    	},
    // [  1GB] TH58NVG3D1DTG00	// 4k Page
    { {{0x98, 0xD3, 0x94, 0xBA, 0x64, 0x00}}, 2048,  40, 128, 4096,  218,  2,  3, 30,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_8BIT	|A_BIG  |S_NOR|S_MP)		},
	// [  2GB] TH58NVG4D1DTG00	[  4GB] TH58NVG5D1DTG20	
    { {{0x98, 0xD5, 0x94, 0x00, 0x00, 0x00}}, 4096,  40, 128, 4096,  218,  2,  3, 30,	 0, 20, 10,  0, 15, 15, (A_08BIT|A_MLC_8BIT	|A_BIG  |S_NOR|S_MP)		},
    // [  8GB] TH58NVG6D1DTG20	// 4k Page
    { {{0x98, 0xD7, 0x00, 0x00, 0x00, 0x00}}, 8192,  40, 128, 4096,  218,  2,  3, 30,	 0, 20, 10,  0, 15, 15, (A_08BIT|A_MLC_8BIT	|A_BIG  |S_NOR|S_MP) 		}
};

const TCC_MTD_IO_FEATURE	TOSHIBA_LBA_NAND_DevInfo[] =
{
    //*=======================================================================================================================================================
    //*[        DEVICE CODE       ][           SIZE          ][               Cycle               ][                  ATTRIBUTE                  ]
    //*-------------------------------------------------------------------------------------------------------------------------------------------------------
    //* 1st, 2nd,  3rd,  4th,  5th,  6th,  PBpV,BBpZ, PpB, Page,Spare,Col,Low,Twc, Ws, Wp, Wh, Rs, Rp, Rh
    //*=======================================================================================================================================================
    // [  2GB] THGVN0G4D1DTG00
    { {{0x98, 0x21, 0x01, 0x55, 0xAA, 0x00}}, 8192,  40, 128, 4096,  218,  2,  3, 30,	 0, 20, 10,  0, 15, 15, (A_08BIT|A_MLC_8BIT	|A_BIG  |S_NOR|S_MP)		},
    // [  8GB] THGVN1G6D4ELA02
    { {{0x98, 0x21, 0x03, 0x55, 0xAA, 0x00}}, 8192,  40, 128, 4096,  218,  2,  3, 30,	 0, 20, 10,  0, 15, 15, (A_08BIT|A_MLC_8BIT	|A_BIG  |S_NOR|S_MP)		},
    // [  16GB] THGVN1G7D8ELA09
    { {{0x98, 0x21, 0x04, 0x55, 0xAA, 0x00}}, 8192,  40, 128, 4096,  218,  2,  3, 30,	 0, 20, 10,  0, 15, 15, (A_08BIT|A_MLC_8BIT	|A_BIG  |S_NOR|S_MP) 		}
};

const TCC_MTD_IO_FEATURE	HYNIX_NAND_DevInfo[] =
{
    //*=======================================================================================================================================================
    //*[        DEVICE CODE       ][           SIZE          ][               Cycle               ][                  ATTRIBUTE                  ]
    //*-------------------------------------------------------------------------------------------------------------------------------------------------------
    //* 1st, 2nd,  3rd,  4th,  5th,  6th,  PBpV,BBpZ, PpB, Page,Spare,Col,Low,Twc, Ws, Wp, Wh, Rs, Rp, Rh
    //*=======================================================================================================================================================
    // [ 32MB] HY27US08561M
    { {{0xAD, 0x75, 0x00, 0x00, 0x00, 0x00}}, 2048,  20,  32,  512,   16,  1,  2, 50,	 0, 35, 15,  0, 35, 15, (A_08BIT|A_SLC	  	|A_SMALL|S_NOR|S_CB)       	},
    // [ 64MB] HY27US08121M
    { {{0xAD, 0x76, 0x00, 0x00, 0x00, 0x00}}, 4096,  20,  32,  512,   16,  1,  3, 60,	 0, 40, 20,  0, 40, 20, (A_08BIT|A_SLC	  	|A_SMALL|S_NOR|S_CB)       	},
	// [ 64MB] HY27SS16122A
    { {{0xAD, 0x46, 0xAD, 0x46, 0xAD, 0x00}}, 4096,  20,  32,  512,   16,  1,  3, 60,	 0, 40, 20,  0, 50, 20, (A_16BIT|A_SLC	  	|A_SMALL|S_NOR|S_CB)       	},
    // [128MB] HY27UA081G1M
    { {{0xAD, 0x79, 0x00, 0x00, 0x00, 0x00}}, 8192,  20,  32,  512,   16,  1,  3, 60,	 0, 40, 15,  0, 40, 15, (A_08BIT|A_SLC	  	|A_SMALL|S_NOR|S_CB)    	},
    // [128MB] HY27UF081G2M, HY27UF081G2A 
    { {{0xAD, 0xF1, 0x00, 0x00, 0x00, 0x00}}, 1024,  20,  64, 2048,   64,  2,  2, 50,	 0, 35, 20,  0, 35, 20, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_CP ) 	},
	// [256MB] HY27UF082G2M, HY27UG082G2M
	{ {{0xAD, 0xDA, 0x80, 0x15, 0x00, 0x00}}, 2048,  20,  64, 2048,   64,  2,  3, 50,	 0, 35, 20,  0, 35, 20, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_CP)  	},
	// [256MB] HY27UF082G2A
	{ {{0xAD, 0xDA, 0x80, 0x1D, 0x00, 0x00}}, 2048,  20,  64, 2048,   64,  2,  3, 50,	 0, 35, 20,  0, 35, 20, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_CP ) 	},
	// [256MB] HY27UF082G2B
	{ {{0xAD, 0xDA, 0x10, 0x95, 0x44, 0x00}}, 2048,  20,  64, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [512MB] HY27UF084G2M
    { {{0xAD, 0xDC, 0x80, 0x95, 0x00, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 30,	 0, 20, 10,  0, 20, 10, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_CP)  	},
	// [512MB] HY27UF084G2B
    { {{0xAD, 0xDC, 0x10, 0x95, 0x00, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_MP)	},
    // [512MB] HY27UG084G2M, HY27UH084G2M
    { {{0xAD, 0xDC, 0x80, 0x15, 0x00, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 60,	 0, 35, 20,  0, 35, 20, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_CP)  	},
    // [512MB] HY27UT084G2M
    { {{0xAD, 0xDC, 0x84, 0x25, 0x00, 0x00}}, 2048,  40, 128, 2048,   64,  2,  3, 50,	 0, 35, 15,  0, 35, 15, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR)				},
    // [512MB] HY27UT084G2A
    { {{0xAD, 0xDC, 0x14, 0xA5, 0x24, 0x00}}, 2048,  25, 128, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [512MB] H8BCS0UN0MCR
    { {{0xAD, 0xBC, 0x10, 0x55, 0x54, 0x00}}, 2048,  25, 128, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [  1GB] HY27UH088G2M
    { {{0xAD, 0xD3, 0x80, 0x15, 0x00, 0x00}}, 8192,  20,  64, 2048,   64,  2,  3, 60,	 0, 35, 20,  0, 35, 20, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_CP)  	},
    // [  1GB] HY27UG088G2M  	[  2GB] HY27UH08AG5M
    { {{0xAD, 0xD3, 0xC1, 0x95, 0x00, 0x00}}, 8192,  20,  64, 2048,   64,  2,  3, 30,	 0, 20, 10,  0, 20, 10, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_CP) 	},
    // [  2GB] HY27UH08AG5B
    { {{0xAD, 0xD3, 0x51, 0x95, 0x58, 0x00}}, 8192,  20,  64, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_MP)	 	},
    // [  1GB] H27U8G8F2M
    { {{0xAD, 0xD3, 0x10, 0xA6, 0x34, 0x00}}, 4096,  20,  64, 4096,  128,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_MP)	 	},	
    // [  1GB] HY27UU088G2M
    { {{0xAD, 0xD3, 0x85, 0x25, 0x00, 0x00}}, 4096,  40, 128, 2048,   64,  2,  3, 50,	 0, 35, 15,  0, 35, 15, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR)				},
    // [  1GB] HY27UT088G2M  	[  2GB] HY27UU08AG5M
    { {{0xAD, 0xD3, 0x14, 0xA5, 0x64, 0xAD}}, 4096,  25, 128, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [  1GB] HY27UT088G2A
    { {{0xAD, 0xD3, 0x14, 0xA5, 0x34, 0x00}}, 4096,  25, 128, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)		},
	// [  1GB] HY27U8G8T2B
    { {{0xAD, 0xD3, 0x14, 0xB6, 0x34, 0x00}}, 2048,  30, 128, 4096,  128,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [  4GB] HY27UV08BG5M	 	[  8GB] HY27UW08CGFM
    { {{0xAD, 0xD5, 0x55, 0xA5, 0x68, 0x00}}, 8192,  25, 128, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [  4GB] HY27UV08BG5A	 
    { {{0xAD, 0xD5, 0x55, 0xA5, 0x38, 0x00}}, 8192,  25, 128, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)		},
	// [  2GB] HY27UAG8T2MTR
    { {{0xAD, 0xD5, 0x14, 0xB6, 0x44, 0x00}}, 4096,  25, 128, 4096,  128,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)	 	},
    // [  8GB] HY27UCG8V5MTR
    { {{0xAD, 0xD7, 0x55, 0xB6, 0x48, 0x00}}, 8192,  25, 128, 4096,  128,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)		},
	// [  4GB] HY27UBG8U5A
    { {{0xAD, 0xD5, 0x94, 0x25, 0x44, 0x41}}, 4096,  25, 128, 4096,  224,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_12BIT|A_BIG  |S_NOR|S_MCP)		},
	// [  8GB] HY27UCG8V5A
    { {{0xAD, 0xD7, 0x95, 0x25, 0x48, 0x41}}, 8192,  25, 128, 4096,  224,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_12BIT|A_BIG  |S_NOR|S_MCP)		},
	// [  4GB] H27UBG8T2A		[  8GB] H27UCG8U5(D)A		[ 16GB] H27UDG8V5(F/E)A       
    { {{0xAD, 0xD7, 0x94, 0x9A, 0x74, 0x42}}, 2048,  25, 256, 8192,  448,  2,  3, 25,	 0, 20, 10,  0, 15, 10, (A_08BIT|A_MLC_24BIT|A_BIG  |S_NOR|S_MP|S_EIL)	},
	// [  2GB] H27UAG8T2BTR	
    { {{0xAD, 0xD5, 0x94, 0x9A, 0x74, 0x42}}, 1024,  25, 256, 8192,  448,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_24BIT|A_BIG  |S_NOR|S_MP)	 	}
};

const TCC_MTD_IO_FEATURE	ST_NAND_DevInfo[] =
{
    //*=======================================================================================================================================================
    //*[        DEVICE CODE       ][           SIZE          ][               Cycle               ][                  ATTRIBUTE                  ]
    //*-------------------------------------------------------------------------------------------------------------------------------------------------------
    //* 1st, 2nd,  3rd,  4th,  5th,  6th,  PBpV,BBpZ, PpB, Page,Spare,Col,Low,Twc, Ws, Wp, Wh, Rs, Rp, Rh
    //*=======================================================================================================================================================
    // [ 32MB] NAND256W3A
    { {{0x20, 0x75, 0x00, 0x00, 0x00, 0x00}}, 2048,  20,  32,  512,   16,  1,  2, 50,  	 0, 35, 15,  0, 35, 15, (A_08BIT|A_SLC	  	|A_SMALL|S_NOR)			   	},
    // [ 64MB] NAND512W3A
    { {{0x20, 0x76, 0x00, 0x00, 0x00, 0x00}}, 4096,  20,  32,  512,   16,  1,  3, 50,	 0, 35, 15,  0, 35, 15, (A_08BIT|A_SLC	  	|A_SMALL|S_NOR)			   	},
    // [128MB] NAND01GW3A
    { {{0x20, 0x79, 0x00, 0x00, 0x00, 0x00}}, 8192,  20,  32,  512,   16,  1,  3, 50,	 0, 35, 20,  0, 25, 20, (A_08BIT|A_SLC	  	|A_SMALL|S_NOR|S_CB|S_CP)  	},
    // [128MB] NAND01GW3B2C
    { {{0x20, 0xF1, 0x00, 0x1D, 0x00, 0x00}}, 1024,  20,  64, 2048,   64,  2,  2, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR)			 	},
    // [128MB] NAND01GW3B
    { {{0x20, 0xF1, 0x00, 0x00, 0x00, 0x00}}, 1024,  20,  64, 2048,   64,  2,  2, 50,	 0, 35, 20,  0, 35, 20, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_CP)  	},
	// [256MB] NAND02GW3B2D
    { {{0x20, 0xDA, 0x10, 0x95, 0x44, 0x00}}, 2048,  20,  64, 2048,   64,  2,  3, 45,	 0, 25, 20,  0, 25, 20, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_MP)	   	},
    // [256MB] NAND02GW3B
    { {{0x20, 0xDA, 0x00, 0x00, 0x00, 0x00}}, 2048,  20,  64, 2048,   64,  2,  3, 50,	 0, 35, 20,  0, 35, 20, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_CP)  	},
    // [256MB] NAND02GR4B2DDI6 
    { {{0x20, 0xBA, 0x10, 0x55, 0x44, 0x00}}, 2048,  20,  64, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_16BIT|A_SLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [512MB] NAND04GR4B2DDI6 
    { {{0x20, 0xBC, 0x10, 0x55, 0x54, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 45,	 0, 30, 15,  0, 30, 15, (A_16BIT|A_SLC	  	|A_BIG  |S_NOR|S_MP)		},
	// [512MB] NAND04GW3B2D
    { {{0x20, 0xDC, 0x10, 0x95, 0x54, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [512MB] NAND04GW3B
    { {{0x20, 0xDC, 0x80, 0x95, 0x00, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 50,	 0, 35, 20,  0, 35, 20, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_CP)   },
    // [512MB] NAND04GW3C2A
    { {{0x20, 0xDC, 0x84, 0x25, 0x00, 0x00}}, 2048,  40, 128, 2048,   64,  2,  3, 60,	 0, 40, 20,  0, 40, 20, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR)				},
	// [  1GB] NAND08GW3B2CN6
    { {{0x20, 0xD3, 0x51, 0x95, 0x58, 0x00}}, 8192,  20,  64, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_MP	|S_IL)	},
    // [  1GB] NAND08GW3B
    { {{0x20, 0xD3, 0x85, 0x25, 0x00, 0x00}}, 8192,  20,  64, 2048,   64,  2,  3, 50,	 0, 35, 15,  0, 35, 15, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_CP)  	},
	// [  1GB] NAND08GW3C2A,	[ 2GB] NAND16GW3C4A 
    { {{0x20, 0xD3, 0x14, 0xA5, 0x00, 0x00}}, 4096,  20, 128, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)	  	}
};

const TCC_MTD_IO_FEATURE	SAMSUNG_NAND_DevInfo[] =
{
    //*=======================================================================================================================================================
    //*[        DEVICE CODE       ][           SIZE          ][               Cycle               ][                  ATTRIBUTE                  ]
	//*-------------------------------------------------------------------------------------------------------------------------------------------------------
    //* 1st, 2nd,  3rd,  4th,  5th,  6th,  PBpV,BBpZ, PpB, Page,Spare,Col,Low,Twc, Ws, Wp, Wh, Rs, Rp, Rh
    //*=======================================================================================================================================================
	// [ 32MB] K9F5608U0B/C/D ~TEST(C) ~TEST(D)   
    { {{0xEC, 0x75, 0x00, 0x00, 0x00, 0x00}}, 2048,  20,  32,  512,   16,  1,  2, 50,  	0, 35, 15,  0, 35, 15, (A_08BIT|A_SLC	 	|A_SMALL|S_NOR|S_CB)       	},
    // [ 64MB] K9F1208U0M/A/B
    { {{0xEC, 0x76, 0xA5, 0xC0, 0x00, 0x00}}, 4096,  20,  32,  512,   16,  1,  3, 50,  	0, 35, 15,  0, 35, 15, (A_08BIT|A_SLC	 	|A_SMALL|S_NOR|S_CB)   	 	},
    // [ 64MB] K9F1208U0C: ~TEST(C) WC: 42 WP: 21 WH: 15 
    { {{0xEC, 0x76, 0x5A, 0x3F, 0x00, 0x00}}, 4096,  20,  32,  512,   16,  1,  3, 50,  	0, 35, 15,  0, 35, 15, (A_08BIT|A_SLC	 	|A_SMALL|S_NOR)		   	 	},
    // [128MB] K9K1G08U0M/A/B: ~TEST(B) 
    { {{0xEC, 0x79, 0xA5, 0xC0, 0x00, 0x00}}, 8192,  20,  32,  512,   16,  1,  3, 50,  	0, 35, 15,  0, 35, 15, (A_08BIT|A_SLC	 	|A_SMALL|S_NOR|S_CB)   	 	},
    // [128MB] K9F1G08U0M,A
    { {{0xEC, 0xF1, 0x80, 0x15, 0x40, 0x00}}, 1024,  20,  64, 2048,   64,  2,  2, 50,  	0, 35, 15,  0, 35, 15, (A_08BIT|A_SLC	 	|A_BIG  |S_NOR|S_CB|S_CP)	},
    // [128MB] K9F1G08U0C	K9F1G08U0B[ Twc:50 Ws: 0 Wp: 35 Wh: 15]
    { {{0xEC, 0xF1, 0x00, 0x95, 0x40, 0x00}}, 1024,  20,  64, 2048,   64,  2,  2, 50,  	0, 35, 15,  0, 35, 15, (A_08BIT|A_SLC	 	|A_BIG  |S_NOR|S_CB) 	  	},
    // [256MB] KBE00S00AB	MCP MEMORY - Supply Voltage: Vcc 2.5 ~ 2.9 
    { {{0xEC, 0x71, 0x5A, 0x3F, 0x00, 0x00}},16384,  20,  32,  512,   16,  1,  3, 50,  	0, 35, 15,  0, 35, 15, (A_08BIT|A_SLC	 	|A_SMALL|S_NOR)     	 	},
	// [256MB] K9F2G08U0M, K9K2G08U0M/A ~TEST(A)
    { {{0xEC, 0xDA, 0x00, 0x15, 0x00, 0x00}}, 2048,  20,  64, 2048,   64,  2,  3, 50,  	0, 35, 15,  0, 35, 15, (A_08BIT|A_SLC	 	|A_BIG  |S_NOR|S_CB|S_CP)	},
	// [256MB] K9F2G08U0A
    { {{0xEC, 0xDA, 0x10, 0x95, 0x44, 0x00}}, 2048,  20,  64, 2048,   64,  2,  3, 25,  	0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC     	|A_BIG  |S_NOR|S_MP)  		},
    // [512MB] K524G2GACB (mddr MCP)	YKG038QX
    { {{0xEC, 0xBC, 0x00, 0x55, 0x54, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 40,  	0, 21, 10,  0, 21, 10, (A_16BIT|A_SLC	 	|A_BIG  |S_NOR)			   	},
    // [512MB] K524G2GACB (mddr MCP)	YKG040A3
    { {{0xEC, 0xBC, 0x00, 0x55, 0x56, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 40,  	0, 21, 10,  0, 21, 10, (A_16BIT|A_SLC	 	|A_BIG  |S_NOR) 		   	},
    // [512MB] K9K4G08U0M
    { {{0xEC, 0xDC, 0xC1, 0x15, 0x00, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 30,  	0, 20, 10,  0, 20, 10, (A_08BIT|A_SLC	 	|A_BIG  |S_NOR|S_CB|S_CP)  	},
	// [512MB] K9F4G08U0B
    { {{0xEC, 0xDC, 0x10, 0x95, 0x54, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 25,  	0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	 	|A_BIG  |S_NOR|S_MP)	    },
    // [512MB] K9F4G08U0M
    { {{0xEC, 0xDC, 0x10, 0x95, 0x00, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 25,  	0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	 	|A_BIG  |S_NOR|S_CB)	    },
    // [256MB] K9G2G08U0M
	{ {{0xEC, 0xDA, 0x14, 0x25, 0x44, 0x00}}, 1024,  25, 128, 2048,   64,  2,  3, 30,  	0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	 	|A_BIG  |S_NOR|S_MP) 		},	
    // [512MB] K9G4G08U0M/A		[  1GB] K9L8G08U1M
    { {{0xEC, 0xDC, 0x14, 0x25, 0x54, 0x00}}, 2048,  40, 128, 2048,   64,  2,  3, 30,  	0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	 	|A_BIG  |S_NOR|S_MP)    	},
    // [512MB] K9G4G08U0B
    { {{0xEC, 0xDC, 0x14, 0xA5, 0x54, 0x00}}, 2048,  25, 128, 2048,   64,  2,  3, 25,  	0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	 	|A_BIG  |S_NOR|S_MP)    	},
    // [  1GB] K9K8G08UOM		[  2GB] K9WAG08U1M 
    { {{0xEC, 0xD3, 0x51, 0x95, 0x00, 0x00}}, 8192,  20,  64, 2048,   64,  2,  3, 25,  	0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	 	|A_BIG  |S_NOR|S_CB)		},
    // [  1GB] K9K8G08U0B 	
    { {{0xEC, 0xDC, 0x51, 0x95, 0x58, 0x00}}, 8192,  20,  64, 2048,   64,  2,  3, 25,  	0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	 	|A_BIG  |S_NOR|S_MP|S_IL)	},	
    // [  1GB] K9L8G08UOM		[  2GB] K9HAG08U1M
    { {{0xEC, 0xD3, 0x55, 0x25, 0x00, 0x00}}, 4096,  40, 128, 2048,   64,  2,  3, 35,  	0, 25, 10,  0, 25, 10, (A_08BIT|A_MLC	 	|A_BIG  |S_NOR|S_MP|S_IL)  	},
    // [  1GB] K9G8G08UOM		[  2GB] K9LAG08U1M 
    { {{0xEC, 0xD3, 0x14, 0x25, 0x64, 0x00}}, 4096,  40, 128, 2048,   64,  2,  3, 35,  	0, 25, 10,  0, 25, 10, (A_08BIT|A_MLC	 	|A_BIG  |S_NOR|S_MP)		},
    // [  1GB] K9G8G08UOA/B		[  2GB] K9LAG08U1A 
    { {{0xEC, 0xD3, 0x14, 0xA5, 0x64, 0x00}}, 4096,  25, 128, 2048,   64,  2,  3, 50,  	0, 30, 20,  0, 30, 20, (A_08BIT|A_MLC	 	|A_BIG  |S_NOR|S_MP)		},	
    // [  2GB] K9LAG08UOM	  	[  4GB] K9HBG08U1M	  [  8GB] K9MCG08U5M
    { {{0xEC, 0xD5, 0x55, 0x25, 0x68, 0x00}}, 8192,  40, 128, 2048,   64,  2,  3, 30,  	0, 20, 10,  0, 20, 10, (A_08BIT|A_MLC	 	|A_BIG  |S_NOR|S_MP|S_IL)	},
	// [  2GB] K9LAG08UOA/B	 
	{ {{0xEC, 0xD5, 0x55, 0xA5, 0x68, 0x00}}, 8192,  25, 128, 2048,   64,  2,  3, 25,  	0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	 	|A_BIG  |S_NOR|S_MP|S_IL)	},
    // [  2GB] K9GAG08UOM	// 4K Page
    { {{0xEC, 0xD5, 0x14, 0xB6, 0x74, 0x00}}, 4096,  25, 128, 4096,  128,  2,  3, 25,  	0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	 	|A_BIG  |S_NOR|S_MP)		},
    // [  4GB] K9LBG08UOM		[  8GB] K9HCG08U1M    [ 16G] K9MDG08U5M
    { {{0xEC, 0xD7, 0x55, 0xB6, 0x78, 0x00}}, 8192,  25, 128, 4096,  128,  2,  3, 25,  	0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	 	|A_BIG  |S_NOR|S_MP|S_IL)	},
    // [  2GB] K9GAG08UOD		[ 4GB] K9LBG08U1D	  [ 8GB] K9HCG08U5D
    { {{0xEC, 0xD5, 0x94, 0x29, 0x34, 0x41}}, 4096,  25, 128, 4096,  218,  2,  3, 30,   0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_8BIT	|A_BIG  |S_NOR|S_MCP)		},
    // [  4GB] K9LBG08UOD		[  8GB] K9HCG08U1D	  [ 16G] K9MDG08U5D   [ 32G] K9PDG08U5D
    { {{0xEC, 0xD7, 0xD5, 0x29, 0x38, 0x41}}, 8192,  25, 128, 4096,  218,  2,  3, 30,  	0, 20, 10,  0, 20, 10, (A_08BIT|A_MLC_8BIT	|A_BIG  |S_NOR|S_MP|S_IL)	},
	// [  2GB] K9GAG08UOE		
    { {{0xEC, 0xD5, 0x84, 0x72, 0x50, 0x42}}, 2048,  30, 128, 8192,  436,  2,  3, 30,  	0, 20, 10,  0, 20, 10, (A_08BIT|A_MLC_24BIT	|A_BIG  |S_NOR)				},
	// [  4GB] K9LBG08UOE		[  8GB] K9HCG08U1E			
    { {{0xEC, 0xD7, 0xC5, 0x72, 0x54, 0x42}}, 4096,  30, 128, 8192,  436,  2,  3, 30,  	0, 20, 10,  0, 20, 10, (A_08BIT|A_MLC_24BIT	|A_BIG  |S_NOR|S_IL|S_EB)	},
    // [  4GB] K9GBG08U0M		[  8GB] K9LCG08U1M		[ 16GB] K9HDG08U5M
    { {{0xEC, 0xD7, 0x94, 0x72, 0x54, 0x42}}, 4096,  30, 128, 8192,  436,  2,  3, 30,  	0, 20, 10,  0, 20, 10, (A_08BIT|A_MLC_24BIT |A_BIG  |S_NOR|S_MP|S_EIL)	},
	// [  32GB] K9PFG08U5M
    { {{0xEC, 0xDE, 0xD5, 0x72, 0x58, 0x42}}, 8192,  30, 128, 8192,  436,  2,  3, 30,  	0, 20, 10,  0, 20, 10, (A_08BIT|A_MLC_24BIT |A_BIG  |S_NOR|S_MP|S_IL|S_EB)	}
};

const TCC_MTD_IO_FEATURE	MICRON_NAND_DevInfo[] =
{
    //*=======================================================================================================================================================
    //*[        DEVICE CODE       ][           SIZE          ][               Cycle               ][                  ATTRIBUTE                  ]
    //*-------------------------------------------------------------------------------------------------------------------------------------------------------
    //* 1st, 2nd,  3rd,  4th,  5th,  6th,  PBpV,BBpZ, PpB, Page,Spare,Col,Low,Twc, Ws, Wp, Wh, Rs, Rp, Rh
    //*=======================================================================================================================================================
    // [256MB] 29F2G08AAC		
    { {{0x2C, 0xDA, 0x00, 0x00, 0x00, 0x00}}, 2048,  20,  64, 2048,   64,  2,  3, 30,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CP)   	    },
    // [512MB] 29F4G08BAC,		[  1GB] 29F8G08FAC
    { {{0x2C, 0xDC, 0x80, 0x15, 0x50, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 30,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CP)	    },
    // [512MB] 29F4G08AAA/C		[  1GB] 29F8G08DAA
    { {{0x2C, 0xDC, 0x90, 0x95, 0x54, 0x00}}, 4096,  20,  64, 2048,   64,  2,  3, 30,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [  2GB] 29F16G08FAA
    { {{0x2C, 0xD3, 0xD1, 0x95, 0x58, 0x00}}, 8192,  20,  64, 2048,   64,  2,  3, 30,	 0, 25, 15,  0, 25, 15, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_CB|S_CP)	},
    // [  1GB] MT29F8G08ABA (4Bit/540Bytes)		
    { {{0x2C, 0x38, 0x00, 0x26, 0x85, 0x00}}, 2048,  20, 128, 4096,  224,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [  1GB] MT29F16G08ABA (4Bit/540Bytes)		
    { {{0x2C, 0x48, 0x00, 0x26, 0x89, 0x00}}, 4096,  20, 128, 4096,  224,  2,  3, 25, 	 0, 15, 10,  0, 15, 10, (A_08BIT|A_SLC	  	|A_BIG  |S_NOR|S_MP)	 	},
    // [  1GB] 29F08G08MAA   	[  2GB] 29F16G08QAA
    { {{0x2C, 0xD3, 0x94, 0xA5, 0x64, 0x00}}, 4096,  40, 128, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [  4GB] 29F32G08TAA
    { {{0x2C, 0xD5, 0xD5, 0xA5, 0x68, 0x00}}, 8192,  40, 128, 2048,   64,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC	  	|A_BIG  |S_NOR|S_MP)		},
    // [  2GB] 29F16G08MAA		[  4GB] 29F32G08QAA 
	{ {{0x2C, 0xD5, 0x94, 0x3E, 0x74, 0x00}}, 4096,  25, 128, 4096,  218,  2,  3, 30,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_8BIT	|A_BIG  |S_NOR|S_MP)		},
	// [  8GB] 29F64G08TAA
	{ {{0x2C, 0xD7, 0xD5, 0x3E, 0x78, 0x00}}, 8192,  25, 128, 4096,  218,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_8BIT	|A_BIG  |S_NOR|S_MP|S_IL)	},
	// [  4GB] 29F32G08CBAAA, 	[  8GB] 29F64G08CFAAA 
	{ {{0x2C, 0xD7, 0x94, 0x3E, 0x84, 0x00}}, 8192,  25, 128, 4096,  218,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_12BIT|A_BIG  |S_NOR|S_MP)		},
	// [  4GB] 29F32G08CBABA, 	[  8GB] 29F64G08CFABA 
	{ {{0x2C, 0x68, 0x04, 0x46, 0x89, 0x00}}, 4096,  25, 256, 4096,  224,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_12BIT|A_BIG  |S_NOR|S_MCP)		},
	// [ 16GB] 29F128G08CJAA (2cs, 8bit)
	{ {{0x2C, 0xD9, 0xD5, 0x3E, 0x88, 0x00}},16384,  25, 128, 4096,  218,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_12BIT|A_BIG  |S_NOR|S_MP|S_IL)	},
	// [ 16GB] 29F128G08CJABA(2cs, 8bit), 29F128G08CKABA(1cs, 16bit)
	{ {{0x2C, 0x88, 0x05, 0xC6, 0x89, 0x00}}, 8192,  25, 256, 4096,  224,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_12BIT|A_BIG  |S_NOR|S_MP|S_IL)	},
	// [  4GB] 29F32G08CBACA, 	[  8GB]29F64G08CFACA - L73A_32G_64G_128G
	{ {{0x2C, 0x68, 0x04, 0x4A, 0xA9, 0x00}}, 4096,  25, 256, 4096,  224,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_24BIT|A_BIG  |S_NOR|S_MP|S_EIL)	},
	// [ 16GB] 29F128G08CFAAA(2cs, 8bit)
	{ {{0x2C, 0x88, 0x04, 0x4B, 0xA9, 0x00}}, 4096,  25, 256, 8192,  448,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_24BIT|A_BIG  |S_NOR|S_MP|S_EIL) }
};

const TCC_MTD_IO_FEATURE	INTEL_NAND_DevInfo[] =
{
	//*=======================================================================================================================================================
    //*[        DEVICE CODE       ][           SIZE            ][               Cycle                     ][               		   ATTRIBUTE                 ]
    //*-------------------------------------------------------------------------------------------------------------------------------------------------------
    //* 1st, 2nd,  3rd,  4th,  5th,  6th,  PBpV,BBpZ, PpB, Page,Spare,Col,Low,Twc, Ws, Wp, Wh, Rs, Rp, Rh
    //*=======================================================================================================================================================
    // [  4GB] JS29F32G08AAMD1 	[  8GB] JS29F64G08CAMD1 	[ 16GB] JS29F16B08JAMD1
	{ {{0x89, 0xD7, 0x94, 0x3E, 0x84, 0x00}}, 8192,  40, 128, 4096,  218,  2,  3, 25,	 0, 15, 10,  0, 15, 10, (A_08BIT|A_MLC_12BIT|A_BIG  |S_NOR|S_MCP)		}
};

const TCC_MTD_IO_MAKERINFO	NAND_SupportMakerInfo =
 {
	// MAXIMUM SUPPORT NANDFLASH
	{MAX_SUPPORT_SAMSUNG_NAND,
	MAX_SUPPORT_TOSHIBA_NAND,
	MAX_SUPPORT_HYNIX_NAND,
	MAX_SUPPORT_ST_NAND,
	MAX_SUPPORT_MICRON_NAND,
	MAX_SUPPORT_INTEL_NAND},
	// NAND MAKER ID
	{SAMSUNG_NAND_MAKER_ID,
	TOSHIBA_NAND_MAKER_ID,
	HYNIX_NAND_MAKER_ID,
	ST_NAND_MAKER_ID,
	MICRON_NAND_MAKER_ID,
	INTEL_NAND_MAKER_ID},

	// POINTER OF NANDFLASH INFOMATION
	{(TCC_MTD_IO_FEATURE*)SAMSUNG_NAND_DevInfo,
	(TCC_MTD_IO_FEATURE*)TOSHIBA_NAND_DevInfo,
	(TCC_MTD_IO_FEATURE*)HYNIX_NAND_DevInfo,
	(TCC_MTD_IO_FEATURE*)ST_NAND_DevInfo,
	(TCC_MTD_IO_FEATURE*)MICRON_NAND_DevInfo,
	(TCC_MTD_IO_FEATURE*)INTEL_NAND_DevInfo}
};

#define TCC_MTD_IO_MAX_SHIFT_FACTOR_FOR_MULTIPLY	16
const unsigned short int    TCC_MTD_IO_ShiftFactorForMultiplay[TCC_MTD_IO_MAX_SHIFT_FACTOR_FOR_MULTIPLY] =
{
    1,      //     1 = 2^0
    2,      //     2 = 2^1
    4,      //     4 = 2^2
    8,      //     8 = 2^3
    16,     //    16 = 2^4
    32,     //    32 = 2^5
    64,     //    64 = 2^6
    128,    //   128 = 2^7
    256,    //   256 = 2^8
    512,    //   512 = 2^9
    1024,   //  1024 = 2^10
    2048,   //  2048 = 2^11
    4096,   //  4096 = 2^12
    8192,   //  8192 = 2^13
    16384,  // 16384 = 2^14
    32768	// 32768 = 2^15
};

//=============================================================================
//*
//*
//*                           [ GLOBAL VARIABLE DEFINE ]
//*
//*
//=============================================================================

TCC_MTD_IO_CYCLE		WriteCycleTime;
TCC_MTD_IO_CYCLE		ReadCycleTime;
TCC_MTD_IO_CYCLE		CommCycleTime;

TCC_MTD_IO_ECC_INFO		gMLC_ECC_1Bit;
TCC_MTD_IO_ECC_INFO		gMLC_ECC_4Bit;
TCC_MTD_IO_ECC_INFO		gMLC_ECC_8Bit;
TCC_MTD_IO_ECC_INFO		gMLC_ECC_12Bit;
TCC_MTD_IO_ECC_INFO		gMLC_ECC_14Bit;
TCC_MTD_IO_ECC_INFO		gMLC_ECC_16Bit;
TCC_MTD_IO_ECC_INFO		gMLC_ECC_24Bit;

TCC_MTD_IO_DEVINFO 		gDevInfo[2];
 
unsigned int			*gpMTD_DMA_PhyBuffer0;
unsigned int			*gpMTD_DMA_WorkBuffer0;
unsigned int			*gpMTD_DMA_PhyBuffer1;
unsigned int			*gpMTD_DMA_WorkBuffer1;

PPIC 					pMTD_PIC;
PGPIO 					pMTD_GPIO;
PNFC					pMTD_NFC;
PECC					pMTD_ECC;
PIOBUSCFG				pIOBUSCFG_T;
PGDMANCTRL				pNAND_DMA;
unsigned char			gDummyDevID[4];

unsigned int			gMTDNAND_PORT_STATUS = 0;

const unsigned char	ALL_FF_ECC_BCH_04BIT_12[44] __attribute__((aligned(8))) =
{ 
	#if defined(TCC89XX) || defined(TCC92XX)
	0x48,0xF6,0x3C,0xC9,0xAA,0x45,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#elif defined(TCC93XX) || defined(TCC88XX)  
	0xCF,0xD3,0x97,0xE9,0x30,0x26,0x5A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#endif 
};

const unsigned char	ALL_FF_ECC_BCH_04BIT_16[44] __attribute__((aligned(8))) =
{ 
	#if defined(TCC89XX) || defined(TCC92XX)
	0xC4,0x2A,0xDC,0xB4,0x25,0xCC,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#elif defined(TCC93XX) || defined(TCC88XX)   
	0x8A,0x03,0x7D,0x52,0x1E,0x13,0xC2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#endif 
};

const unsigned char	ALL_FF_ECC_BCH_01BIT_512[44] __attribute__((aligned(8))) =
{ 
	#if defined(TCC89XX) || defined(TCC92XX)  
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#elif defined(TCC93XX) || defined(TCC88XX)  		 
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#endif 
};

const unsigned char	ALL_FF_ECC_BCH_04BIT_512[44] __attribute__((aligned(8))) =
{ 
	#if defined(TCC89XX) || defined(TCC92XX)
	0xEB,0x37,0xCC,0x63,0x96,0xCA,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#elif defined(TCC93XX) || defined(TCC88XX)  
	0x99,0x04,0x91,0x4F,0xF9,0xB2,0x76,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#endif
};

const unsigned char	ALL_FF_ECC_BCH_08BIT_512[44] __attribute__((aligned(8))) =
{ 
	#if defined(TCC89XX) || defined(TCC92XX)
	0x08,0x75,0x8B,0x6F,0x48,0x36,0xA6,0xBC,0x16,0x61,0x58,0xDB,0x52,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#elif defined(TCC93XX) || defined(TCC88XX)    
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF
	#endif  
};

const unsigned char	ALL_FF_ECC_BCH_12BIT_512[44] __attribute__((aligned(8)))= 
{ 
	#if defined(TCC89XX) || defined(TCC92XX)
	0x81,0xEC,0xE8,0x4E,0xE3,0x46,0x44,0xA1,0x3F,0x8A,0x29,0x06,0xD0,0x90,
	0x06,0x76,0x21,0x32,0x3E,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#elif defined(TCC93XX) || defined(TCC88XX)  
	0x25,0x1E,0xB0,0x67,0x00,0x58,0x97,0x7E,0xEC,0x9E,0x92,0x5F,0x0C,0x4F,
	0x6C,0xE2,0xDF,0xA5,0xD5,0x86,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#endif  
};

const unsigned char	ALL_FF_ECC_BCH_14BIT_512[44] __attribute__((aligned(8)))= 
{ 
	#if defined(TCC89XX) || defined(TCC92XX)
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF
	#elif defined(TCC93XX) || defined(TCC88XX)     	 
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF
	#endif 
};  

const unsigned char	ALL_FF_ECC_BCH_16BIT_512[44] __attribute__((aligned(8)))= 
{ 
	#if defined(TCC89XX) || defined(TCC92XX)
	0xA6,0x14,0x08,0x76,0xEE,0xFE,0x20,0x10,0x9F,0xA3,0xC5,0x06,0x6D,0xDB,
	0xF4,0x51,0xBF,0x38,0x65,0xF8,0xD8,0xC2,0x87,0xFB,0xF1,0x8B,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#elif defined(TCC93XX) || defined(TCC88XX)  		  
    0x24,0xDE,0x7C,0x49,0xD8,0xD0,0x1D,0xC1,0x1F,0x4D,0x6E,0x04,0x64,0xF8,
	0x7D,0x45,0x29,0x63,0xB0,0x0F,0x8A,0x9E,0x23,0x38,0x7D,0x74,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00
	#endif 
};

const unsigned char	ALL_FF_ECC_BCH_24BIT_512[44] __attribute__((aligned(8)))= 
{ 
	#if defined(TCC89XX) || defined(TCC92XX)
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF
	#elif defined(TCC93XX) || defined(TCC88XX)     
    0xC3,0xEC,0x7B,0xC8,0x8E,0x04,0xDA,0xDD,0xAF,0x82,0x36,0xB4,0xC6,0x36,
	0x21,0x95,0x27,0x26,0xD1,0xD3,0xB6,0x6C,0xCF,0x98,0x0E,0xE3,0x3A,0xE8,
	0x41,0x14,0x8B,0x69,0x47,0x4B,0xD0,0xCC,0xB8,0x33,0xDA,0xC2,0xE6,0x56,
	0x00,0x00
	#endif
};


//=============================================================================
//*
//*
//*                           [ LOCAL FUNCTIONS DEFINE ]
//*
//*
//=============================================================================
static int 							TCC_MTD_IO_Dev_ready( struct mtd_info *mtd );
static int 							TCC_MTD_IO_Waitfunc(struct mtd_info *mtd, struct nand_chip *this);
static void 						TCC_MTD_IO_Read_buf(struct mtd_info *mtd, u_char *buf, int len);
static u_char 						TCC_MTD_IO_Read_byte(struct mtd_info *mtd);
static void 						TCC_MTD_IO_Write_buf(struct mtd_info *mtd, const u_char *buf, int len);
static int 							TCC_MTD_IO_Write_page(struct mtd_info *mtd, struct nand_chip *chip, const uint8_t *buf, int page, int cached, int raw);
void 								TCC_MTD_IO_HwInit( void );
void 								TCC_MTD_IO_ReadID( int nMode );
static void 						TCC_MTD_IO_Select_chip(struct mtd_info *mtd, int chipnr);
static void 						TCC_MTD_IO_Cmdfunc(struct mtd_info *mtd, unsigned command, int column, int page_addr);

TCC_MTD_IO_ERROR 					TCC_MTD_IO_CallBackChangeWCtime( unsigned short int TotalMediaNum, TCC_MTD_IO_DEVINFO *nDevInfo );
TCC_MTD_IO_ERROR 					TCC_MTD_IO_locGetDeviceInfo( U16 nChipNo, TCC_MTD_IO_DEVINFO *nDevInfo );
TCC_MTD_IO_ERROR 					TCC_MTD_IO_locSetCycle( TCC_MTD_IO_DEVINFO *nDevInfo );
TCC_MTD_IO_ERROR 					TCC_MTD_IO_locReadPage( TCC_MTD_IO_DEVINFO *nDevInfo, U32 nPageAddr,U16 nStartPPage, U16 nReadPPSize,	U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff );
TCC_MTD_IO_ERROR 					TCC_MTD_IO_locReadPageForSignature( TCC_MTD_IO_DEVINFO *nDevInfo, U32 nPageAddr, U16 nStartPPage, U16 nReadPPSize, U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff );
TCC_MTD_IO_ERROR 					TCC_MTD_IO_locReadSpare( TCC_MTD_IO_DEVINFO *nDevInfo, U32 nPageAddr, U8 *nSpareBuffer );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locRead528Data( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nReadPPSize, U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff, int nSpareOnOff );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locRead512DataDoubleBuf( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nReadPPSize, U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff, int nSpareOnOff );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locRead512DataGoldenPage( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nReadPPSize, U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff, int nSpareOnOff );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locRead512Data( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nReadPPSize, U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff, int nSpareOnOff );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locReadSpareData( TCC_MTD_IO_DEVINFO *nDevInfo, U8 *nSpareBuffer, int nPageEccOnOff );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locGoldenReadSpareData( TCC_MTD_IO_DEVINFO *nDevInfo, U8 *nSpareBuffer, int nPageEccOnOff );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locWrite512DataDoubleBuf( struct mtd_info *mtd, U16 nStartPPage, U16 nWritePPSize, U8 *nPageBuffer, U8 *nSpareBuffer );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locWrite512Data( struct mtd_info *mtd, U16 nStartPPage, U16 nWritePPSize, U8 *nPageBuffer, U8 *nSpareBuffer );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locWriteSpareData( TCC_MTD_IO_DEVINFO *nDevInfo, U8 *nSpareBuffer, int nPageEccOnOff );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locPreEncodeDataForLastPage( TCC_MTD_IO_DEVINFO *nDevInfo, U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff );

static __inline void 				TCC_MTD_IO_locSetupECC( U16 nEccOnOff, U16 nEncDec, U16 nEccType, U16 nAccessType, U32 EccBaseAddr );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locEncodeECC( U16 nEccType, U8* nSpareBuffer );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locCorrectionSLC( U8* nPageBuffer, U8* nSpareBuffer );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locCorrectionMLC( U16 nEccType, U8* nPageBuffer, U8* nSpareBuffer, U16 nDataSize );

static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locReadStatus( void );
static __inline void 				TCC_MTD_IO_locWaitBusy(void);
void 								TCC_MTD_IO_locReset(int nMode );
void 								TCC_MTD_IO_locResetForReadID( U16 nChipNo, int nMode );

static __inline void 				TCC_MTD_IO_locDelay( void );
static __inline void 				TCC_MTD_IO_IRQ_Mask( void );
static __inline void 				TCC_MTD_IO_IRQ_UnMask( void );
static __inline void 				TCC_MTD_IO_locEnableChipSelect( U16 nChipNo );
static __inline void 				TCC_MTD_IO_locDisableChipSelect( void );
static __inline void 				TCC_MTD_IO_locEnableWriteProtect( void );
static __inline void 				TCC_MTD_IO_locDisableWriteProtect( void );

static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locGenerateRowColAddrForRead( U32 nPageAddr, U16 nColumnAddr, U32* rRowAddr, U32* rColumnAddr, TCC_MTD_IO_DEVINFO *nDevInfo );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locGenerateRowColAddrForWrite( U32 nPageAddr, U16 nColumnAddr, U32* rRowAddr, U32* rColumnAddr );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locGetShiftValueForFastMultiPly( U16 nValue, U16* rFactor );
static __inline void 				TCC_MTD_IO_locWriteBlockPageAddr( U32 nBlockPageAddr );
static __inline void 				TCC_MTD_IO_locWriteRowColAddr( U32 nRowAddr, U32 nColumnAddr );
static __inline void 				TCC_MTD_IO_locWriteColAddr( U32 nColumnAddr );

static __inline void 				TCC_MTD_IO_LineUpToBytesPerSector( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nPPSize, U16* rStartPPage, U16* rPPSize, U16* nFlagOfAlign );
static __inline void 				TCC_MTD_IO_LineUpToBufferAlignement( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nFlagOfAlign, U16 nStartPPage, U16 nLastPPage, U8 *nPageBuffer, U8 *nSpareBuffer, U8** pPageBuffer, U8** pSpareBuffer, U8 nMode );
static __inline void 				TCC_MTD_IO_LineUpBufferData( U16 nFlagOfAlign, U16 nStartPPage, U16 nLastPPage, U8 *nPageBuffer, U8 *nSpareBuffer, U8* pPageBuffer, U8* pSpareBuffer );

static __inline	void 				TCC_MTD_IO_locCheckForExtendBlockAccess( TCC_MTD_IO_DEVINFO *nDevInfo, U32* nPageAddr );

static __inline void 				TCC_MTD_IO_locSetDataWidth( U32 width );

static __inline void 				TCC_MTD_IO_locSetReadCycleTime(void);
static __inline void 				TCC_MTD_IO_locSetBasicCycleTime( void );
static __inline void 				TCC_MTD_IO_locSetCommCycleTime( void );
static __inline void 				TCC_MTD_IO_locSetWriteCycleTime(void);
void 								TCC_MTD_IO_locECC_InfoInit( void );
static __inline U32 				TCC_MTD_IO_locCheckReadyAndBusy( void );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EIL Function
static __inline void 				TCC_MTD_IO_locClearInterleaveStatus( TCC_MTD_IO_DEVINFO *nDevInfo );
static __inline TCC_MTD_IO_ERROR 	TCC_MTD_IO_locReadStatusForExternalInterleave( TCC_MTD_IO_DEVINFO *nDevInfo );
static __inline void				TCC_MTD_IO_locWaitBusyForProgramAndErase( TCC_MTD_IO_DEVINFO *nDevInfo );
static __inline TCC_MTD_IO_ERROR	TCC_MTD_IO_locWaitBusyForInterleave( TCC_MTD_IO_DEVINFO *nDevInfo, U32 nPageAddr );

#if defined(__USE_MTD_NAND_ISR__)
///////////////////////////////////////////////////////////////////////////
// ISR Function
static irqreturn_t					TCC_MTD_IO_ISR(int irq, void *dev_id);
TCC_MTD_IO_ERROR					TCC_MTD_IO_IRQ_Init();
TCC_MTD_IO_ERROR 					TCC_MTD_IO_IRQ_Write512DataPostProcess( TCC_MTD_IO_DEVINFO *nDevInfo, U8* nECCBuffer );
void								TCC_MTD_IO_IRQ_Enable( unsigned int fEnable );
void								TCC_MTD_IO_IRQ_SetupDMA( void *pDST, int nMode,  int nDSize  );
static __inline unsigned int		TCC_MTD_IO_IRQ_IsEnabled( void );
#endif

#if defined(TCC_MTD_DMA_BUFFER)
void TCC_MTD_IO_InitDMABuffer( TCC_MTD_IO_DEVINFO *nDevInfo )
{	
	mtd_dma_t.buf_size = ( nDevInfo->BytesPerSector * 2);
	mtd_dma_t.v_addr 	= dma_alloc_writecombine(0, mtd_dma_t.buf_size, &mtd_dma_t.dma_addr, GFP_KERNEL);

	printk("[MTD TCC] DMA Alloc Addr:0x%x, Size:%d\n", mtd_dma_t.dma_addr, mtd_dma_t.buf_size );

	gpMTD_DMA_PhyBuffer0 	= (unsigned int*)mtd_dma_t.dma_addr;
	gpMTD_DMA_WorkBuffer0	= (unsigned int*)mtd_dma_t.v_addr;
	gpMTD_DMA_PhyBuffer1 	= (unsigned int*)((unsigned char*)gpMTD_DMA_PhyBuffer0 + nDevInfo->BytesPerSector );
	gpMTD_DMA_WorkBuffer1	= (unsigned int*)((unsigned char*)gpMTD_DMA_WorkBuffer0 + nDevInfo->BytesPerSector );
}
#endif

#if defined(__USE_MTD_NAND_ISR__)
/**************************************************************************
*  FUNCTION NAME : 
*  
*      TCC_MTD_IO_ERROR TCC_MTD_IO_IRQ_Init()
*  
*  DESCRIPTION : Init IRQ Function
*  				 [ Add ISR Function, Add Wait Queue, Reset Ready/Busy Pin interrupt ]
*  INPUT:
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= If init success return 0 else return error flag.
*  
**************************************************************************/
TCC_MTD_IO_ERROR TCC_MTD_IO_IRQ_Init()
{
	int ret;

	//printk("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! MTD ISR Init !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );
	
	ret = request_irq(MTD_IRQ_NFC,					// Add Interrupt Service Routine (Shared IRQ Number)
					  TCC_MTD_IO_ISR,
					  IRQF_SHARED,   				/* flags */
					  "mtd-isr",    				/* in /proc/interrupts */
					  &sTCC_MTD_IO_NandIsrInfo); 	/* user data passed to ISR */
					  
	if( ret != SUCCESS )
	{
		printk(" !!!! request_irq Fail !!! \n");
		return ret;
	}

	/* Init ISR Struct */
	memset( &sTCC_MTD_IO_NandIsrInfo, 0x00, sizeof(TCC_MTD_ISR_INFO_T) );
	init_waitqueue_head(&(sTCC_MTD_IO_NandIsrInfo.wait_q));	

	#if 0
	#if defined(TCC89XX) || defined(TCC92XX)
    BITSET(pMTD_PIC->CLR0, (1 << NAND_IRQ_READY));			// Interrupt Status Clear
	NAND_IO_IRQ_ExtInterruptClear(NAND_IRQ_READY);
    BITCLR(pMTD_PIC->IEN0, (1 << NAND_IRQ_READY));			// Interrupt Disable

	#if defined(TCC_NAND_RDY_B28)
	pMTD_GPIO->GPBFN3 |= 0x00010000;		// GPIO_B28 --> EDI
	#elif defined(TCC_NAND_RDY_B29)
	pMTD_GPIO->GPBFN3 |= 0x00100000;		// GPIO_B29 --> EDI
	#elif defined(TCC_NAND_RDY_B31)
	pMTD_GPIO->GPBFN3 &= ~0x10000000;		// ND_RDY[GPIO_B31]			
	BITCLR(pMTD_GPIO->GPBEN, Hw31);
	#endif
	#endif
	#endif

	return ret;
	
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*       void TCC_MTD_IO_IRQ_Enable( void );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
void TCC_MTD_IO_IRQ_Enable( unsigned int fEnable )
{
	sTCC_MTD_IO_IRQ_fEnable = fEnable;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline unsigned int TCC__MTD_IO_IRQ_IsEnabled( void );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline unsigned int TCC_MTD_IO_IRQ_IsEnabled( void )
{
	return sTCC_MTD_IO_IRQ_fEnable;
}


/**************************************************************************
*  FUNCTION NAME : 
*  
*      void TCC_MTD_IO_IRQ_SetupDMA( void *pDST, int nMode, int nDSize )
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nMode	= Read or Write
*			pDST	= Destination Buffer Address
			nDSize  = Data Size 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
void TCC_MTD_IO_IRQ_SetupDMA( void *pDST, int nMode, int nDSize )
{
	unsigned int	nSourceAddr, nDestAddr;
	unsigned		uCHCTRL;
	unsigned int	uTmp;	
	unsigned int	uSrcInc, uSrcMask;
	unsigned int	uDstInc, uDstMask;

	//printk("+DMA+\n");
	if ( nMode == NAND_IO_DMA_WRITE )
	{
		uSrcInc 	= 4;
		uSrcMask 	= 0;
		uDstInc		= 0;
		uDstMask 	= 0;
	}
	else
	{
		uSrcInc 	= 0;
		uSrcMask 	= 0;
		uDstInc		= 4;
		uDstMask 	= 0;
	}

	if ( nMode == NAND_IO_DMA_WRITE )
	{
		#if defined(TCC89XX) || defined(TCC92XX)				//HwNFC_CTRL_DEN_EN: NANDFLASH DMA Request Enable
		BITCSET(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_8 | HwNFC_CTRL_DEN_EN | HwNFC_CTRL_PSIZE_512 );
		#elif defined(TCC93XX)	|| defined(TCC88XX)				//HwNFC_CTRL_DEN_EN: DMA Acknowledge Enable 
		BITCSET(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_8 | HwNFC_CTRL_PSIZE_512 );
		#endif 
		
		// Target Physical Address- for DMA H/W Control Set
		nSourceAddr	= (unsigned int)pDST;
		nDestAddr 	= (unsigned int)&NAND_IO_HwLDATA_PA;

		//============================================================
		// DMA Control Register Set
		//============================================================
		uCHCTRL =	
	//				HwCHCTRL_SYNC_ON		|
	//				HwCHCTRL_HRD_W			|
					HwCHCTRL_BST_BURST		|
					HwCHCTRL_TYPE_SINGL		|
				   	HwCHCTRL_HRD_WR			|
	//				HwCHCTRL_BST_BURST		|
					HwCHCTRL_BSIZE_8		|
					HwCHCTRL_WSIZE_32		|
					HwCHCTRL_FLAG			|
					HwCHCTRL_EN_ON			|
					0;
	}
	else
	{
		// pSRC: NFC_LDATA
		// pDST: Buffer Address
		
		#if defined(TCC89XX) || defined(TCC92XX)				//HwNFC_CTRL_DEN_EN: NANDFLASH DMA Request Enable
		BITCSET(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_8 | HwNFC_CTRL_DEN_EN | HwNFC_CTRL_PSIZE_512 );
		#elif defined(TCC93XX)	|| defined(TCC88XX)				//HwNFC_CTRL_DEN_EN: DMA Acknowledge Enable 
		BITCSET(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_8 | HwNFC_CTRL_PSIZE_512 );
		#endif 
		
		nSourceAddr	= (unsigned int)&NAND_IO_HwLDATA_PA;
		nDestAddr 	= (unsigned int)pDST;

		//============================================================
		// DMA Control Register Set
		//============================================================
		uCHCTRL =	
	//				HwCHCTRL_SYNC_ON		|
	//				HwCHCTRL_HRD_W			|
					HwCHCTRL_BST_BURST		|
					HwCHCTRL_TYPE_SINGL		|
				   	HwCHCTRL_HRD_RD			|
	//				HwCHCTRL_BST_BURST		|
					HwCHCTRL_BSIZE_8		|
					HwCHCTRL_WSIZE_32		|
					HwCHCTRL_FLAG			|
					HwCHCTRL_EN_ON			|
					0;
	}
	
	//============================================================
	// Set Source Address & Source Parameter (mask + increment)
	//============================================================
	pNAND_DMA->ST_SADR 	= nSourceAddr;
	#if defined(_WINCE_) || defined(_LINUX_)
	pNAND_DMA->SPARAM[0] = (uSrcInc | (uSrcMask << 4));
	#else
	pNAND_DMA->SPARAM	 = (uSrcInc | (uSrcMask << 4));
	#endif
	//============================================================
	// Set Dest Address & Dest Parameter (mask + increment)
	//============================================================
	pNAND_DMA->ST_DADR 	= nDestAddr;  
	#if defined(_WINCE_) || defined(_LINUX_)
	pNAND_DMA->DPARAM[0] = (uDstInc | (uDstMask << 4));
	#else
	pNAND_DMA->DPARAM	 = (uDstInc | (uDstMask << 4));
	#endif
	//============================================================
	// Calculate byte size per 1 Hop transfer
	//============================================================
	uTmp	= (uCHCTRL & (Hw5+Hw4)) >> 4;			// calc log2(word size)
	uTmp	= uTmp + ( (uCHCTRL & (Hw7+Hw6)) >> 6);	// calc log2(word * burst size)

	//============================================================
	// Set External DMA Request Register
	//============================================================
	pNAND_DMA->EXTREQ = Hw18;		// NFC

	//============================================================
	// Set Hcount
	//============================================================
	if (uTmp)
		pNAND_DMA->HCOUNT	= (nDSize + (1 << uTmp) - 1) >> uTmp;
	else
		pNAND_DMA->HCOUNT	= nDSize;
	
	//============================================================
	// Set & Enable DMA
	//============================================================
	pNAND_DMA->CHCTRL	= uCHCTRL;

	//============================================================
	// Set NFC DSize & IREQ Clear
	//============================================================
	pMTD_NFC->NFC_DSIZE		= nDSize;

	//============================================================
	// DMA Transfer Start
	//============================================================
	#ifdef NAND_GPIO_DEBUG
	BITSET(pMTD_GPIO->GPFDAT, Hw21);
	#endif	
	if ( nMode == NAND_IO_DMA_WRITE )
	{
		#if defined(TCC89XX) || defined(TCC92XX)	
		if ( pMTD_NFC->NFC_CTRL1 & Hw31 )
			BITCLR( pMTD_NFC->NFC_CTRL1, Hw31 );
		pMTD_NFC->NFC_IREQ		= 0x77;	// HwNFC_IREQ_FLAG1;
		pMTD_NFC->NFC_PSTART	= 0;
		#elif defined(TCC93XX)	|| defined(TCC88XX)	
		BITSET(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_PRSEL_P);
		pMTD_NFC->NFC_PRSTART	= 0;
		#endif 
	}
	else
	{
		#if defined(TCC89XX) || defined(TCC92XX)	
		pMTD_NFC->NFC_IREQ		= 0x77;	// HwNFC_IREQ_FLAG1;
		pMTD_NFC->NFC_RSTART	= 0;
		#elif defined(TCC93XX) || defined(TCC88XX)
		BITCLR(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_PRSEL_P);
		pMTD_NFC->NFC_PRSTART	= 0;
		#endif
	}
}


/**************************************************************************
*  FUNCTION NAME : 
*  
*      TCC_MTD_IO_ERROR 	TCC_MTD_IO_IRQ_Write512DataPostProcess( TCC_MTD_IO_DEVINFO *nDevInfo, U8* nECCBuffer )
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDevInfo	= 
*			nECCBuffer	= 
*  
*  OUTPUT:	NAND_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
TCC_MTD_IO_ERROR 	TCC_MTD_IO_IRQ_Write512DataPostProcess( TCC_MTD_IO_DEVINFO *nDevInfo, U8* nECCBuffer )
{ 
	unsigned int		i;
	#ifdef _LINUX_
	unsigned char		nTempBuffer[44]__attribute__((aligned(8)));
	#else
	unsigned char		nTempBuffer[44];
	#endif
	unsigned char		*pSpare, *pEccB;
	TCC_MTD_IO_ERROR	res = (TCC_MTD_IO_ERROR)SUCCESS;

	//============================
	// Get ECC Code
	//============================
	#if defined(TCC89XX) || defined(TCC92XX)
	if ( pMTD_NFC->NFC_CTRL1 & Hw30 )
		BITSET( pMTD_NFC->NFC_CTRL1, Hw31 );
	#endif 

	if ( ( nDevInfo->Feature.MediaType  & A_BIG ) || (( nDevInfo->Feature.MediaType  & A_SMALL ) && ( nDevInfo->Feature.MediaType  & A_PARALLEL )))
	{
		if ( ( nDevInfo->Feature.MediaType  & A_MLC_16BIT ) || ( nDevInfo->Feature.MediaType  & A_MLC_24BIT ) )
		{
			pSpare = (unsigned char*)nECCBuffer;
			pEccB  = (unsigned char*)nTempBuffer;
			res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pEccB );
			for ( i = 0; i < nDevInfo->EccDataSize; ++i )
				pSpare[i] = pEccB[i];
		}
		else
		{
			res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, nECCBuffer );
		}
	}
	else
	{
		pSpare = (unsigned char*)nECCBuffer;
		pEccB  = (unsigned char*)nTempBuffer;
		res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pEccB );
		for ( i = 0; i < nDevInfo->EccDataSize; ++i )
			pSpare[i] = pEccB[i];
	}
				
	return res;
}


/**************************************************************************
*  FUNCTION NAME : 
*  
*      TCC_MTD_IO_ERROR TCC_MTD_IO_IRQ_Init()
*  
*  DESCRIPTION : Init IRQ Function
*  				 [ Add ISR Function, Add Wait Queue, Reset Ready/Busy Pin interrupt ]
*  INPUT:
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= If init success return 0 else return error flag.
*  
**************************************************************************/
static irqreturn_t		TCC_MTD_IO_ISR(int irq, void *dev_id)
{
	unsigned char *pSrcEccBuffer;

	if( sTCC_MTD_IO_NandIsrInfo.ubIsRun == TRUE )
	{
		//===============================================
		// DMA Read IRQ
		//===============================================
	    #if defined(TCC89XX) || defined(TCC92XX)
	    if ( pMTD_NFC->NFC_IREQ & Hw0 )				// READ INTERRUPT 
	    #elif defined(TCC93XX) || defined(TCC88XX)
	    if ( pMTD_NFC->NFC_IRQ &  HwNFC_IRQ_RDIRQ )	// READ INTERRUPT 
	    #endif 
		{
			#if defined(TCC89XX) || defined(TCC92XX)
			BITSET(pMTD_NFC->NFC_IREQ, Hw0 );
			#elif defined(TCC93XX) || defined(TCC88XX)
			BITSET( pMTD_NFC->NFC_IRQ, HwNFC_IRQ_RDIRQ );
			#endif 

			if(sTCC_MTD_IO_NandIsrInfo.uiState!= TCC_MTD_IO_IRQ_STATE_READ)
			{
				printk("[TCC_MTD] wrong state with read irq. (state=%u)\n",sTCC_MTD_IO_NandIsrInfo.uiState);
				return;
			}

			if(sTCC_MTD_IO_NandIsrInfo.usPPagesLeft==0)
			{
				printk("[TCC_MTD] no pages left to read\n");
				return;
			}
			#if 0
			if( ( sTCC_MTD_IO_NandIsrInfo.usCurrentPPage + sTCC_MTD_IO_NandIsrInfo.usStartPPage ) 
					== (sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->UsablePPages - 1 ) )
			{
			
				//printk("isr out [S:%d] [P:%d] \n", sTCC_MTD_IO_NandIsrInfo.usStartPPage, sTCC_MTD_IO_NandIsrInfo.usPPagesLeft);
				memcpy(sTCC_MTD_IO_NandIsrInfo.pbPageBuffer, gTCC_MTD_IO_PreEnDecodeEccBuffer, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector );
				
				sTCC_MTD_IO_NandIsrInfo.wait_complete = 1;
			    wake_up(&(sTCC_MTD_IO_NandIsrInfo.wait_q));

				return IRQ_HANDLED;
			}
			#endif
			
			if ( ( sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->Feature.MediaType  & A_BIG ) || (( sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->Feature.MediaType  & A_SMALL ) && ( sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->Feature.MediaType  & A_PARALLEL )))
			{
				// NOP	
			}
			else if ( sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->Feature.MediaType & A_SMALL )
			{
				/* Read 16Bytes spare data */
				pSrcEccBuffer= &sTCC_MTD_IO_NandIsrInfo.pbEccBuffer[0];
				
				unsigned int i = 4;
				do {
					if ( !((int)sTCC_MTD_IO_NandIsrInfo.pbEccBuffer & 0x3) )
					{
						*((unsigned int*)sTCC_MTD_IO_NandIsrInfo.pbEccBuffer) = pMTD_NFC->NFC_WDATA;
					}	
					else
					{
						DWORD_BYTE uDWordByte;
						uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
						sTCC_MTD_IO_NandIsrInfo.pbEccBuffer[0] = uDWordByte.BYTE[0];
						sTCC_MTD_IO_NandIsrInfo.pbEccBuffer[1] = uDWordByte.BYTE[1];
						sTCC_MTD_IO_NandIsrInfo.pbEccBuffer[2] = uDWordByte.BYTE[2];
						sTCC_MTD_IO_NandIsrInfo.pbEccBuffer[3] = uDWordByte.BYTE[3];
					}
					sTCC_MTD_IO_NandIsrInfo.pbEccBuffer = &sTCC_MTD_IO_NandIsrInfo.pbEccBuffer[4];
				}while(--i);

				sTCC_MTD_IO_NandIsrInfo.pbEccBuffer = pSrcEccBuffer;
				sTCC_MTD_IO_NandIsrInfo.pbEccBuffer = &sTCC_MTD_IO_NandIsrInfo.pbEccBuffer[NAND_IO_SPARE_SIZE_SMALL];
			}
	
			//unsigned char *temp;
			memcpy( sTCC_MTD_IO_NandIsrInfo.pbPageBuffer, gpMTD_DMA_WorkBuffer0, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector );

			/* Check and Correct ECC code */
			if ( sTCC_MTD_IO_NandIsrInfo.iEccOnOff == ECC_ON )
			{
				if ( sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->EccType == SLC_ECC_TYPE )
				{
					sTCC_MTD_IO_NandIsrInfo.error |= TCC_MTD_IO_locCorrectionSLC( sTCC_MTD_IO_NandIsrInfo.pbPageBuffer, sTCC_MTD_IO_NandIsrInfo.pbEccBuffer );
				}
				else
				{
					sTCC_MTD_IO_NandIsrInfo.error |= TCC_MTD_IO_locCorrectionMLC( sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->EccType, sTCC_MTD_IO_NandIsrInfo.pbPageBuffer, sTCC_MTD_IO_NandIsrInfo.pbEccBuffer, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector );
				}

				sTCC_MTD_IO_NandIsrInfo.pbEccBuffer = &sTCC_MTD_IO_NandIsrInfo.pbEccBuffer[sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->EccDataSize];
			}

			sTCC_MTD_IO_NandIsrInfo.pbPageBuffer = &sTCC_MTD_IO_NandIsrInfo.pbPageBuffer[sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector];
			sTCC_MTD_IO_NandIsrInfo.usCurrentPPage++;
			sTCC_MTD_IO_NandIsrInfo.usPPagesLeft--;

			if( ( sTCC_MTD_IO_NandIsrInfo.usCurrentPPage + sTCC_MTD_IO_NandIsrInfo.usStartPPage ) 
					== (sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->UsablePPages - 1 ) )
			{
			
				//printk("isr out [S:%d] [P:%d] \n", sTCC_MTD_IO_NandIsrInfo.usStartPPage, sTCC_MTD_IO_NandIsrInfo.usPPagesLeft);
				memcpy(sTCC_MTD_IO_NandIsrInfo.pbPageBuffer, gTCC_MTD_IO_PreEnDecodeEccBuffer, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector );
				
				sTCC_MTD_IO_NandIsrInfo.wait_complete = 1;
			    wake_up(&(sTCC_MTD_IO_NandIsrInfo.wait_q));
				
			} 
			else if(sTCC_MTD_IO_NandIsrInfo.usPPagesLeft>0)
			{
				if ( sTCC_MTD_IO_NandIsrInfo.iEccOnOff == ECC_ON )
				{
					/* Setup ECC Block */
					#if defined(_WINCE_) || defined(_LINUX_)
					TCC_MTD_IO_locSetupECC( ECC_ON, ECC_DECODE, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->EccType, NAND_DMA_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
					#else
					TCC_MTD_IO_locSetupECC( ECC_ON, ECC_DECODE, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->EccType, NAND_DMA_ACCESS, (U32)&pMTD_NFC->NFC_LDATA );
					#endif
					pMTD_ECC->ECC_CTRL	|= ( sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
					pMTD_ECC->ECC_CLEAR	= 0x00000000;
				}

				TCC_MTD_IO_IRQ_SetupDMA(gpMTD_DMA_PhyBuffer0, NAND_IO_DMA_READ, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector);
			} 			
	    }
		//===============================================
		// DMA Write IRQ
		//===============================================
		#if defined(TCC89XX) || defined(TCC92XX)
		else if( pMTD_NFC->NFC_IREQ & Hw1 )
		#elif defined(TCC93XX) || defined(TCC88XX)
		else if( pMTD_NFC->NFC_IRQ &  HwNFC_IRQ_PIRQ )	
		#endif 
		{
			#if defined(TCC89XX) || defined(TCC92XX)
			BITSET( pMTD_NFC->NFC_IREQ, Hw1 );
			#elif defined(TCC93XX) || defined(TCC88XX)
			BITSET( pMTD_NFC->NFC_IRQ, HwNFC_IRQ_PIRQ );
			#endif 
			
			if(sTCC_MTD_IO_NandIsrInfo.uiState != TCC_MTD_IO_IRQ_STATE_WRITE)
			{
				//printk("[TCC_MTD] wrong state with write irq. (state=%u)\n",sTCC_MTD_IO_NandIsrInfo.uiState);
				return;
			}

			TCC_MTD_IO_IRQ_Write512DataPostProcess(sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo, sTCC_MTD_IO_NandIsrInfo.pbEccBuffer);

			if(sTCC_MTD_IO_NandIsrInfo.usCurrentPPage & ( ( sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->Feature.PageSize / sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector )-1)) 
			{
				// continue data transfer for 1 page
				sTCC_MTD_IO_NandIsrInfo.pbEccBuffer = &sTCC_MTD_IO_NandIsrInfo.pbEccBuffer[sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->EccDataSize];
				if(sTCC_MTD_IO_NandIsrInfo.usPPagesLeft>0)
				{
					memcpy(gpMTD_DMA_WorkBuffer0,sTCC_MTD_IO_NandIsrInfo.pbPageBuffer,sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector);
					sTCC_MTD_IO_NandIsrInfo.pbPageBuffer = &sTCC_MTD_IO_NandIsrInfo.pbPageBuffer[sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector];
					sTCC_MTD_IO_NandIsrInfo.usPPagesLeft--;
				}
				else
				{
					// Null Sector Write
					memset(gpMTD_DMA_WorkBuffer0,0xFF,sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector);
				}
				sTCC_MTD_IO_NandIsrInfo.usCurrentPPage++;

				if ( sTCC_MTD_IO_NandIsrInfo.iEccOnOff == ECC_ON )
				{
					/* Setup ECC Block */
					#if defined(_WINCE_) || defined(_LINUX_)
					TCC_MTD_IO_locSetupECC( (U16)sTCC_MTD_IO_NandIsrInfo.iEccOnOff, ECC_ENCODE, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->EccType, NAND_DMA_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
					#else
					TCC_MTD_IO_locSetupECC( (U16)sTCC_MTD_IO_NandIsrInfo.iEccOnOff, ECC_ENCODE, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->EccType, NAND_DMA_ACCESS, (U32)&pMTD_NFC->NFC_LDATA );
					#endif
					pMTD_ECC->ECC_CTRL	|= ( sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
					pMTD_ECC->ECC_CLEAR	= 0x00000000;
				}

				TCC_MTD_IO_IRQ_SetupDMA(gpMTD_DMA_PhyBuffer0,NAND_IO_DMA_WRITE, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector );
			}
			else
			{
				{	
					//printk("[TCC_MTD] Write ISR OK !!! \n");
					sTCC_MTD_IO_NandIsrInfo.wait_complete = 1;
				    wake_up(&(sTCC_MTD_IO_NandIsrInfo.wait_q));
				}
			}
		}
		else
		{
			#if defined(TCC89XX) || defined(TCC92XX)
			printk("[TCC_MTD]unknown mtd ireq : %08X\n",pMTD_NFC->NFC_IREQ);
			pMTD_NFC->NFC_IREQ = pMTD_NFC->NFC_IREQ;
			#elif defined(TCC93XX) || defined(TCC88XX)
	        printk("[TCC_MTD]unknown mtd ireq : %08X\n",pMTD_NFC->NFC_IRQ);
			pMTD_NFC->NFC_IRQ = pMTD_NFC->NFC_IRQ&pMTD_NFC->NFC_IRQCFG;
	        #endif
		}
	}	

	return IRQ_HANDLED;
}
#endif


/**************************************************************************
*  FUNCTION NAME : 
*  
*      static int __init TCC_MTD_IO_Init(struct tcc_nand_info *info);
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	__init - Return Type
*  			= 
*  
**************************************************************************/
static int TCC_MTD_IO_Init(struct tcc_nand_info *info)
{
	struct					nand_chip *this;
	int 					retval = 0;
	unsigned int			i = 0;
	unsigned int			nStartOffSet = 0;
	unsigned int			nPartSize = 0;
	unsigned int			nMediaNum = 0;
	TCC_MTD_IO_DEVINFO 		sDevInfo, sTmpDevInfo[4];
	

	//==============================================
	// NAND IO Init
	//==============================================	
	//TCC_MTD_IO_HwInit();

	//==============================================
	// Read ID & Get DeviceInfo
	//==============================================
	nMediaNum = 0;
	
	for ( i = 0; i < 4; ++i )
	{	
		if ( TCC_MTD_IO_locGetDeviceInfo( i, &sDevInfo ) == SUCCESS )
		{
			sTmpDevInfo[nMediaNum]	=	sDevInfo;
			++nMediaNum;
		}
	}

	#if defined(__USE_MTD_NAND_ISR__)
	TCC_MTD_IO_ERROR		res;
	
	printk(" [ISR: ON] ");
		
	res = TCC_MTD_IO_IRQ_Init();

	if( res != SUCCESS )
	{
		printk(" !!! MTD IRQ Init FAIL !!!!\n");
		//return -1;
	}
	#else
	printk(" [ISR: OFF] ");
	#endif
	
    if( nMediaNum == 0 )
	{
		printk("No NAND device found-tcc!!!\n");
		return -ENOMEM;
	}
	else
	{
		memcpy( &gDevInfo[0], &sTmpDevInfo[0], sizeof(TCC_MTD_IO_DEVINFO) );

		#ifdef __USE_MTD_EIL__
		if( ( gDevInfo[0].Feature.MediaType & S_EIL )  && ( nMediaNum > 1 ) )
		{
			printk("[EIL: ON]\n");			
			
			memcpy( &gDevInfo[1], &sTmpDevInfo[1], sizeof(TCC_MTD_IO_DEVINFO) );

			gDevInfo[0].ExtInterleaveUsable = TRUE;
			gDevInfo[1].ExtInterleaveUsable = TRUE;			

			#if defined(TCC88XX)
				#if defined(TCC88D_88M_BOARD)
				if(system_rev == 0x0602 || system_rev == 0x0612)
					nand_platform_data->SetFlags( FALSE );
				else
					nand_platform_data->SetFlags( TRUE );				
				#endif
			#endif
			
		}
		else
		#endif
		{
			printk("[EIL: OFF]\n");
			
			nMediaNum  = 1;
			gDevInfo[0].ExtInterleaveUsable = FALSE;
			gDevInfo[1].ExtInterleaveUsable = FALSE;		
		}
	}
		
	#if defined(TCC_MTD_DMA_BUFFER)
	TCC_MTD_IO_InitDMABuffer(&gDevInfo[0]);
	#endif
	
	TCC_MTD_IO_locSetCycle( &gDevInfo[0] );

	for ( i = 0;i < nMediaNum; ++i )
		TCC_MTD_IO_CallBackChangeWCtime( nMediaNum, &gDevInfo[i] );

	//==============================================
	// Get pointer to private data
	//==============================================
	this = &info->tcc_nand;

	//==============================================
	// Link the private data with the MTD structure
	//==============================================
	info->mtd.priv		= this;
	info->mtd.owner		= THIS_MODULE;

	//==============================================
	// Link the private data with the tcc_nand_info structure
	//==============================================
	this->priv			= info;
	//==============================================
	// Set address of NAND I/O function
	//==============================================
	this->chip_delay 	= 30;	/* 30 us command delay time */
	this->waitfunc 		= TCC_MTD_IO_Waitfunc;
	this->dev_ready 	= TCC_MTD_IO_Dev_ready;
	this->select_chip 	= TCC_MTD_IO_Select_chip;
	this->cmdfunc		= TCC_MTD_IO_Cmdfunc;
	this->cmd_ctrl  	= NULL;
	//==============================================
	this->read_buf 		= TCC_MTD_IO_Read_buf;
	this->read_byte 	= TCC_MTD_IO_Read_byte;
	//==============================================
	this->write_page 	= TCC_MTD_IO_Write_page;
	this->write_buf 	= TCC_MTD_IO_Write_buf;
	//==============================================
	this->verify_buf 	= NULL;
	
	if (( gDevInfo[0].Feature.MediaType & A_PARALLEL) || (gDevInfo[0].Feature.PageSize > 4096 ))
		this->options |= NAND_BUSWIDTH_16;

	if (( gDevInfo[0].Feature.PpB == 256 ))	
	{
		if( !( gDevInfo[0].Feature.MediaType & A_PARALLEL ) )
			this->options &= ~NAND_BUSWIDTH_16;
		
		this->options |= NAND_PPB_256;
	}
		
	this->ecc.mode 	= NAND_ECC_NONE;
	this->IO_ADDR_R = this->IO_ADDR_W = (void *) pMTD_NFC->NFC_SDATA;

	//===========================================================
	// Get MTD Partition Start OffSet
	//===========================================================	
	//TCC_MTD_IO_locMTDAreaScanInfo( info, &gDevInfo[0], nMediaNum, &nStartOffSet, &nPartSize );
	if(s_pTccMtdGlueDrv)
		s_pTccMtdGlueDrv->MTDAreaScanInfo( info, &gDevInfo[0], nMediaNum, &nStartOffSet, &nPartSize, NAND_SB_BOOT_PAGE_ECC_SIZE );	

	if ( nPartSize == 0 )
		goto outmem;
	
	return 0;

 outmem:
 	return retval;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      void TCC_MTD_IO_HwInit( void );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
void TCC_MTD_IO_HwInit( void )
{
	unsigned int	i;
	
	PEDI			pEDI;
	pMTD_GPIO		= (PGPIO)tcc_p2v(HwGPIO_BASE);
	pEDI 			= (PEDI)tcc_p2v(HwEDI_BASE);
	pMTD_NFC 		= (PNFC)tcc_p2v(HwNFC_BASE);
	pMTD_ECC 		= (PECC)tcc_p2v(HwECC_BASE);
	pIOBUSCFG_T 	= (PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE);
	pMTD_PIC		= (PPIC)tcc_p2v(HwPIC_BASE);
	pNAND_DMA		= (PGDMANCTRL)tcc_p2v(HwGDMA0_BASE);

	/*************************************/
	/*Don't remove NAND_IO_Delay Function*/
	/*************************************/
	for( i = 0; i < 5000; ++i )
		TCC_MTD_IO_locDelay();
	
	gMTDNAND_PORT_STATUS = ( TCC_MTD_IO_PORT_NAND_ON_CS0 | TCC_MTD_IO_PORT_NAND_ON_CS1 | TCC_MTD_IO_PORT_DATA_WITDH_8BIT );

	#if defined(TCC89XX) || defined(TCC92XX)
	if ( pMTD_GPIO->GPBFN0 == 0x11111111 )
	#elif defined(TCC93XX) || defined(TCC88XX)
	if ( pMTD_GPIO->GPBFN1 == 0x11111111 )
	#endif
		gMTDNAND_PORT_STATUS |= TCC_MTD_IO_PORT_DATA_WITDH_16BIT;
	
	#if defined(TCC89XX) || defined(TCC92XX)
	if ( pMTD_GPIO->GPBFN3 & 0x00000010 )
	#elif defined(TCC93XX) 
	if ( pMTD_GPIO->GPBFN3 & 0x00001000 )
	#elif defined(TCC88XX)
	if ( pMTD_GPIO->GPBFN3 & 0x00000001 )		
	#endif
		gMTDNAND_PORT_STATUS |= TCC_MTD_IO_PORT_NAND_ON_CS2;
	
	#if defined(TCC89XX) || defined(TCC92XX)
	if ( pMTD_GPIO->GPBFN3 & 0x00000100 )
	#elif defined(TCC93XX)
	if ( pMTD_GPIO->GPBFN3 & 0x00000010 )		
	#elif defined(TCC88XX)
	if ( pMTD_GPIO->GPBFN3 & 0x00000010 )
	#endif
		gMTDNAND_PORT_STATUS |= TCC_MTD_IO_PORT_NAND_ON_CS3;


	printk("[MTD TCC] [PORT CONFIG - CS[0, 1");

	if ( gMTDNAND_PORT_STATUS & TCC_MTD_IO_PORT_NAND_ON_CS2 )
		printk(", 2");

	if ( gMTDNAND_PORT_STATUS & TCC_MTD_IO_PORT_NAND_ON_CS3 )
		printk(", 3");

	printk("] ");
		
	if ( gMTDNAND_PORT_STATUS & TCC_MTD_IO_PORT_DATA_WITDH_16BIT )
		printk("[NAND Data Port: GPIO_B Hw0 ~ Hw15]");
	else
		printk("[NAND Data Port: GPIO_B Hw4 ~ Hw11]");

	//printk("\n");

	//---------------------------------------------------------------------------------------------------
	// [TCC89x] NAND DATA BUS WITDH
	// NAND GPIO_B Port (sND_xx : shared port)
	// pGPIO->GPBFN0 = { ND_D[07], ND_D[06], ND_D[05], ND_D[04], ND_D[11], ND_D[10], ND_D[09], ND_D[08] }
	// pGPIO->GPBFN1 = { ND_D[15], ND_D[14], ND_D[13], ND_D[12], ND_D[03], ND_D[02], ND_D[01], ND_D[00] }
	// pGPIO->GPBFN2 = { ND_CS#0,  sND_WP,   ND_ALE,   ND_CLE, 	 ND_OE,    XXXXXXXX, ND_WE,    XXXXXXXX	}
	// pGPIO->GPBFN3 = { ND_WP,    XXXXXXXX, XXXXXXXX, ND_RDY, 	 XXXXXXX,  sND_CS#3, sND_CS#2, ND_CS#1  }
	//---------------------------------------------------------------------------------------------------

	//---------------------------------------------------------------------------------------------------
	// [TCC88x] NAND DATA BUS WITDH
	//---------------------------------------------------------------------------------------------------
	// NAND GPIO_B Port(GPIIO_B[0] ~ GPIO_B[15]) - SET(1)
	// pGPIO->GPBFN0 = { ND_D[07], ND_D[06], ND_D[05], ND_D[04], ND_D[03], ND_D[02], ND_D[01], ND_D[00] }
	// pGPIO->GPBFN1 = { ND_D[15], ND_D[14], ND_D[13], ND_D[12], ND_D[11], ND_D[10], ND_D[09], ND_D[08] }		
	// pGPIO->GPBFN2 = { ND_CS[1], XXXXXXXX,   ND_ALE,   ND_CLE, 	ND_OE,  ND_RY[3], 	ND_WE, ND_RY[2]	}
	// pGPIO->GPBFN3 = { XXXXXXXX, ND_CS[0], ND_RY[0], ND_RY[1],    ND_WP, XXXXXXXX, ND_CS[3], ND_CS[2] }
	// NAND GPIO_F PORT (GPIIO_F[12] ~ GPIO_F[27]) - FUNC(3) - 32BIT MODE I/O
	// pGPIO->GPFFN1 = { ND_D[19], ND_D[18], ND_D[17], ND_D[16], XXXXXXXX, XXXXXXXX, XXXXXXXX, XXXXXXXX }
	// pGPIO->GPFFN2 = { ND_D[27], ND_D[26], ND_D[25], ND_D[24], ND_D[23], ND_D[22], ND_D[21], ND_D[20] }
	// pGPIO->GPFFN3 = { XXXXXXXX, XXXXXXXX, XXXXXXXX, XXXXXXXX, ND_D[31], ND_D[30], ND_D[29], ND_D[28] }
	//---------------------------------------------------------------------------------------------------
			
	//---------------------------------------------------------------------------------------------------
	// [TCC93x] NAND DATA BUS WITDH
	// NAND GPIO_B Port(GPIIO_B[0] ~ GPIO_B[15]) - SET(1)
	// pGPIO->GPBFN0 = { ND_D[07], ND_D[06], ND_D[05], ND_D[04], ND_D[03], ND_D[02], ND_D[01], ND_D[00] }
	// pGPIO->GPBFN1 = { ND_D[15], ND_D[14], ND_D[13], ND_D[12], ND_D[11], ND_D[10], ND_D[09], ND_D[08] }		
	// pGPIO->GPBFN2 = { XXXXXXXX, XXXXXXXX,   ND_ALE,   ND_CLE, 	ND_OE, ND_RY[3], 	ND_WE, ND_RY[2]	}
	// pGPIO->GPBFN3 = { ND_CS[1], ND_CS[0], ND_RY[0], ND_RY[1], ND_CS[2], XXXXXXXX, ND_CS[3], XXXXXXXX }
	// NAND GPIO_F Port(GPIIO_F[14] ~ GPIO_F[29]) - SET(7) - 32Bit Mode
	// pGPIO->GPFFN2 = { ND_D[16], ND_D[17], ND_D[18], ND_D[19], ND_D[20], ND_D[21], ND_D[22], ND_D[23] }
	// pGPIO->GPFFN3 = { ND_D[24], ND_D[25], ND_D[26], ND_D[27], ND_D[28], ND_D[29], ND_D[30], ND_D[31] }
	//---------------------------------------------------------------------------------------------------

	#if defined(TCC89XX) || defined(TCC92XX)
	pEDI->EDI_RDYCFG 		= 0x00000001;
	BITCSET(pEDI->EDI_CSNCFG0, 0xFFFF, 0x8765 );//pEDI->EDI_CSNCFG0 	= 0x00043215;
	#elif defined(TCC93XX) || defined(TCC88XX)
		#if defined(TCC930X_STB_BOARD)
		BITCSET(pEDI->EDI_RDYCFG, 	0x000FFFFF, 0x00011112 );
		BITCSET(pEDI->EDI_CSNCFG0, 	0x00FF0F00, 0x00570800 );
		BITCSET(pEDI->EDI_CSNCFG1, 	0x0000000F, 0x00000006 );
		#elif defined(TCC89D_93M_BOARD)
		BITCSET(pEDI->EDI_RDYCFG, 	0x000FFFFF, 0x00011112 );
		BITCSET(pEDI->EDI_CSNCFG0, 	0x00FF0F00, 0x00570800 );
		BITCSET(pEDI->EDI_CSNCFG1, 	0xFFFFFFFF, 0x00000006 );
		#elif defined(TCC8803_M801_BOARD)
		BITCSET(pEDI->EDI_RDYCFG,	0x000FFFFF, 0x00011112 );
		BITCSET(pEDI->EDI_CSNCFG0, 	0x00F00FFF, 0x00500876 );
		#elif defined(TCC88D_88M_BOARD)
		if(system_rev == 0x0602 || system_rev == 0x0612){
		BITCSET(pEDI->EDI_RDYCFG,	0x000FFFFF, 0x00011112 );
		BITCSET(pEDI->EDI_CSNCFG0, 	0x00F00FFF, 0x00500876 );
		}else {
		BITCSET(pEDI->EDI_RDYCFG, 	0x000FFFFF, 0x00032014 );
		BITCSET(pEDI->EDI_CSNCFG0, 	0x00F00FFF, 0x00500876 );
		}
		#else
		BITCSET(pEDI->EDI_RDYCFG, 	0x000FFFFF, 0x00054012 );
		BITCSET(pEDI->EDI_CSNCFG0, 	0x00FF0F00, 0x00570800 );
		BITCSET(pEDI->EDI_CSNCFG1, 	0x0000000F, 0x00000006 );
		#endif
	#endif

	// NAND Data Port setup
	if ( gMTDNAND_PORT_STATUS & TCC_MTD_IO_PORT_DATA_WITDH_16BIT )
	{
		// Set NAND Data Witdh 16bit
		BITCSET(pMTD_GPIO->GPBFN0, 0xFFFFFFFF, 0x11111111);
		BITCSET(pMTD_GPIO->GPBFN1, 0xFFFFFFFF, 0x11111111);
	}
	else
	{
		//TCC89x TCC93x 8Bit Mode Setting 
		#if defined(TCC89XX) || defined(TCC92XX)
		BITCSET(pMTD_GPIO->GPBFN0, 0xFFFF0000, 0x11110000);
		BITCSET(pMTD_GPIO->GPBFN1, 0x0000FFFF, 0x00001111);
		#elif defined(TCC93XX) || defined(TCC88XX)
		BITCSET(pMTD_GPIO->GPBFN0, 0xFFFFFFFF, 0x11111111);
		#endif
	}

	// SET CS#/RDY/WP PORT CONTROL 
	#if defined(TCC89XX) || defined(TCC92XX)
	// Set ND_CS#0, Control Port
	BITCSET(pMTD_GPIO->GPBFN2, 0xF0FFF0F0, 0x10111010);
	// Set ND_WP, ND_RDY, ND_CS#1
	BITCSET(pMTD_GPIO->GPBFN3, 0x0000000F, 0x00000001);
	#elif defined(TCC93XX)
		#if defined(TCC930X_STB_BOARD)
		BITCSET(pMTD_GPIO->GPBFN2, 0x00FFF0F0, 0x00111010);
		BITCSET(pMTD_GPIO->GPBFN3, 0xFFFF0000, 0x11110000);
		#else
		BITCSET(pMTD_GPIO->GPBFN2, 0x00FFF0F0, 0x00111010);
		BITCSET(pMTD_GPIO->GPBFN3, 0xFFFF0000, 0x11110000);
		#endif
	#elif defined(TCC88XX)
	BITCSET(pMTD_GPIO->GPBFN2, 0xF0FFF0F0, 0x10111010);
	BITCSET(pMTD_GPIO->GPBFN3, 0x0FFFF000, 0x01111000);
	#endif

	if ( gMTDNAND_PORT_STATUS & TCC_MTD_IO_PORT_NAND_ON_CS2 )
	{
		// TCC93x: ND_CS#2 GPIO_B[27]
		#if defined(TCC89XX) || defined(TCC92XX)
		BITCSET(pMTD_GPIO->GPBFN3, 0x000000F0, 0x00000010);
		#elif defined(TCC93XX)
			#if defined(TCC930X_STB_BOARD)
	        #else
			BITCSET(pMTD_GPIO->GPBFN2, 0x0000000F, 0x00000006);
			BITCSET(pMTD_GPIO->GPBFN3, 0x0000F000, 0x00001000);
			#endif
		#elif defined(TCC88XX)
		BITCSET(pMTD_GPIO->GPBFN2, 0x0000000F, 0x00000006);
		BITCSET(pMTD_GPIO->GPBFN3, 0x0000000F, 0x00000001);
		#endif
	}
		
	if ( gMTDNAND_PORT_STATUS & TCC_MTD_IO_PORT_NAND_ON_CS3 )
	{
		// TCC93x: ND_CS#3 GPIO_B[25]
		#if defined(TCC89XX) || defined(TCC92XX)
		BITCSET(pMTD_GPIO->GPBFN3, 0x00000F00, 0x00000100);
		#elif defined(TCC93XX) || defined(TCC88XX)
			#if defined(TCC930X_STB_BOARD)
            #else
			BITCSET(pMTD_GPIO->GPBFN2, 0x00000F00, 0x00000600);
			BITCSET(pMTD_GPIO->GPBFN3, 0x000000F0, 0x00000010);
			#endif
		#endif
	}

	// SET GPIO PULL UP
	#if defined(TCC89XX) || defined(TCC92XX)
	if( gMTDNAND_PORT_STATUS & TCC_MTD_IO_PORT_DATA_WITDH_16BIT )
		BITCSET(pMTD_GPIO->GPBPD0, 0xFFFFFFFF, 0x55555555);
	else
		BITCSET(pMTD_GPIO->GPBPD0, 0x0000FFFF, 0x00005555);	
	#elif defined(TCC88XX)
	if( gMTDNAND_PORT_STATUS & TCC_MTD_IO_PORT_DATA_WITDH_16BIT )
		BITCSET(pMTD_GPIO->GPBPD0, 0xFFFFFFFF, 0x55555555);
	else
		BITCSET(pMTD_GPIO->GPBPD0, 0x0000FFFF, 0x00005555);	
		
	#elif defined(TCC93XX)
	if( gMTDNAND_PORT_STATUS & TCC_MTD_IO_PORT_DATA_WITDH_16BIT )
		BITCSET(pMTD_GPIO->GPBPD0, 0xFFFFFFFF, 0x55555555);
	else
		BITCSET(pMTD_GPIO->GPBPD0, 0x0000FFFF, 0x00005555);	
	#endif
			
	/* Platform device driver  */
	// WP, RDY Init	
	nand_platform_data->init();

	/* Make Reset */
	pMTD_NFC->NFC_RST = 0;

    	/* Set Default NFC Configuration */
	#if defined(TCC89XX) || defined(TCC92XX)
		#ifdef __USE_MTD_NAND_ISR__
	    pMTD_NFC->NFC_CTRL	= HwNFC_CTRL_PROGIEN_EN |
		                      HwNFC_CTRL_READIEN_EN |
		                      HwNFC_CTRL_DEN_EN		|
					          HwNFC_CTRL_CFG_NOACT	|
					          HwNFC_CTRL_BSIZE_1	|
						      (4 << 4)				|		// pw = 5
					          (1 << 0);						// hold = 1

		#else
		pMTD_NFC->NFC_CTRL	= HwNFC_CTRL_DEN_EN		|
					          HwNFC_CTRL_CFG_NOACT	|
				          	  HwNFC_CTRL_BSIZE_1	|
		                      (4 << 4)				|		// pw = 5
					          (1 << 0);						// hold = 1
		#endif
		//---------------------------------------------------
	    // [TCC89X NFC IRQ]:			INTERRUPT ENABLE
	    // [TCC88X/93X NFC IRQCFG]: 	READY INTERRUPT ENABLE
	    //---------------------------------------------------
		pMTD_NFC->NFC_CTRL1 |= Hw31;
		pMTD_NFC->NFC_CTRL1 |= Hw30;	

		pMTD_NFC->NFC_IREQ = 0x77;						// Clear Interrupt
		BITSET( pMTD_PIC->CLR1,		HwINT1_NFC );
	    BITSET( pMTD_PIC->SEL1,	  	HwINT1_NFC);		// Set NFC as IRQ interrupt
	    BITSET( pMTD_PIC->MODE1, 	HwINT1_NFC );		// Level type for NFC interrupt, IO_INT_HwNFC );	// Level type for NFC interrupt
	#elif defined(TCC93XX) || defined(TCC88XX)
		pMTD_NFC->NFC_CTRL	|= 	HwNFC_CTRL_CFG_NOACT;
		pMTD_NFC->NFC_IRQCFG =  //HwNFC_IRQCFG_RDYIEN	 |
								HwNFC_IRQCFG_PIEN  		 |
								HwNFC_IRQCFG_RDIEN;
		
		//-----------------------------------------
		//[NDMA]DISABLE BURST OPERATION 
		//-----------------------------------------
		//BITCLR(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BMODE_BURST );
	
		//-----------------------------------------
		//[WRITE/READ] STP: 0 PW: 4 HLD: 2 
		//-----------------------------------------
		BITCSET(pMTD_NFC->NFC_WRCYC, 0xF0F0F, 0x402); 
		BITCSET(pMTD_NFC->NFC_RECYC, 0xF0F0F, 0x402);

		//-----------------------------------------
		//[DMA Mode Set]: GDMA
		//[SET DMA ACKKNOWLEDGE]
		//-----------------------------------------
		BITCLR( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_DMODE_NDMA );
		BITCLR( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_DACK_EN );
		
		//--------------------------------------------
		//[WRITE/READ] STP: 0 PW: 4 HLD: 2 
		//--------------------------------------------	
		BITCSET(pMTD_NFC->NFC_WRCYC, 0xF0F0F, 0x402); 
		BITCSET(pMTD_NFC->NFC_RECYC, 0xF0F0F, 0x402);
	#endif

	//----------------------------------------
	// Setup Variable about ECC 
	//  ECC Control Register 
	//  Base Address for ECC Calculation
	//	Address mask for ECC area
	//----------------------------------------
	#if defined(TCC89XX) || defined(TCC92XX)
	pMTD_ECC->ECC_CTRL	= 0x04000000;		/* ECC Control Register */
	pMTD_ECC->ECC_BASE	= pMTD_NFC->NFC_WDATA;	/* Base Address for ECC Calculation */
	pMTD_ECC->ECC_MASK	= 0x00000000;		/* Address mask for ECC area */
	#endif
    
    TCC_MTD_IO_locECC_InfoInit();
}


/******************************************************************************
*
*	NAND_IO_ERROR	NAND_IO_CallBackChangeWCtime
*
*	Input	:
*	Output	:
*	Return	:
*
*	Description :
*
*******************************************************************************/
TCC_MTD_IO_ERROR TCC_MTD_IO_CallBackChangeWCtime( unsigned short int TotalMediaNum, TCC_MTD_IO_DEVINFO *nDevInfo )
{
	if ( nDevInfo->IoStatus == ENABLE )
	{
		//**************************************************************
		// Case on K9NBG08U5M Samsung NANDFLASH
		//**************************************************************
		if ( nDevInfo->Feature.DeviceID.Code[0] == 0xEC &&
			 nDevInfo->Feature.DeviceID.Code[1] == 0xD3 &&
			 nDevInfo->Feature.DeviceID.Code[2] == 0x51 &&
			 nDevInfo->Feature.DeviceID.Code[3] == 0x95 )
		{
			if ( TotalMediaNum == 4 )
			{
				nDevInfo->Feature.WCtime 	= 45;

				nDevInfo->Feature.WriteSTP 	= 0;
				nDevInfo->Feature.WriteWP 	= 30;
				nDevInfo->Feature.WriteHLD 	= 15;

				nDevInfo->Feature.ReadSTP	= 0;
				nDevInfo->Feature.ReadPW 	= 30;
				nDevInfo->Feature.ReadHLD 	= 15;
				
				TCC_MTD_IO_locSetCycle( nDevInfo );
			}
		}
		//**************************************************************
		// Case on K9MBG08U5M Samsung NANDFLASH
		//**************************************************************
		else if ( nDevInfo->Feature.DeviceID.Code[0] == 0xEC &&
			 	  nDevInfo->Feature.DeviceID.Code[1] == 0xD3 &&
				  nDevInfo->Feature.DeviceID.Code[2] == 0x55 &&
				  nDevInfo->Feature.DeviceID.Code[3] == 0x25 )
		{
			if ( TotalMediaNum == 4 )
			{
				nDevInfo->Feature.WCtime = 45;

				nDevInfo->Feature.WriteSTP 	= 0;
				nDevInfo->Feature.WriteWP 	= 30;
				nDevInfo->Feature.WriteHLD 	= 15;

				nDevInfo->Feature.ReadSTP	= 0;
				nDevInfo->Feature.ReadPW 	= 30;
				nDevInfo->Feature.ReadHLD 	= 15;

				TCC_MTD_IO_locSetCycle( nDevInfo );
			}
		}
		//**************************************************************
		// Case on K9MCG08U5M Samsung NANDFLASH
		//**************************************************************
		else if ( nDevInfo->Feature.DeviceID.Code[0] == 0xEC &&
			 	  nDevInfo->Feature.DeviceID.Code[1] == 0xD5 &&
			 	  nDevInfo->Feature.DeviceID.Code[2] == 0x55 &&
			 	  nDevInfo->Feature.DeviceID.Code[3] == 0x25 &&
			 	  nDevInfo->Feature.DeviceID.Code[4] == 0x68 )
		{
			if ( TotalMediaNum == 4 )
			{
				nDevInfo->Feature.WCtime = 45;

				nDevInfo->Feature.WriteSTP 	= 0;
				nDevInfo->Feature.WriteWP 	= 30;
				nDevInfo->Feature.WriteHLD 	= 15;

				nDevInfo->Feature.ReadSTP	= 0;
				nDevInfo->Feature.ReadPW 	= 30;
				nDevInfo->Feature.ReadHLD 	= 15;

				TCC_MTD_IO_locSetCycle( nDevInfo );
			}
		}
		//**************************************************************
		// Case on K9MDG08U5M Samsung NANDFLASH
		//**************************************************************
		else if ( nDevInfo->Feature.DeviceID.Code[0] == 0xEC &&
			 	  nDevInfo->Feature.DeviceID.Code[1] == 0xD7 &&
			 	  nDevInfo->Feature.DeviceID.Code[2] == 0x55 &&
			 	  nDevInfo->Feature.DeviceID.Code[3] == 0xB6 &&
			 	  nDevInfo->Feature.DeviceID.Code[4] == 0x78 )
		{
			if ( TotalMediaNum == 4 )
			{
				nDevInfo->Feature.WCtime = 45;

				nDevInfo->Feature.WriteSTP 	= 0;
				nDevInfo->Feature.WriteWP 	= 30;
				nDevInfo->Feature.WriteHLD 	= 15;

				nDevInfo->Feature.ReadSTP	= 0;
				nDevInfo->Feature.ReadPW 	= 30;
				nDevInfo->Feature.ReadHLD 	= 15;

				TCC_MTD_IO_locSetCycle( nDevInfo );
			}
		}
		//**************************************************************
		// Case on K9MDG08U5D Samsung NANDFLASH
		//**************************************************************
		else if ( nDevInfo->Feature.DeviceID.Code[0] == 0xEC &&
			 	  nDevInfo->Feature.DeviceID.Code[1] == 0xD7 &&
			 	  nDevInfo->Feature.DeviceID.Code[2] == 0xD5 &&
			 	  nDevInfo->Feature.DeviceID.Code[3] == 0x29 &&
			 	  nDevInfo->Feature.DeviceID.Code[4] == 0x38 )
		{
			if ( TotalMediaNum == 4 )
			{
				nDevInfo->Feature.WCtime = 45;

				nDevInfo->Feature.WriteSTP 	= 0;
				nDevInfo->Feature.WriteWP 	= 30;
				nDevInfo->Feature.WriteHLD 	= 15;

				nDevInfo->Feature.ReadSTP	= 0;
				nDevInfo->Feature.ReadPW 	= 30;
				nDevInfo->Feature.ReadHLD 	= 15;

				TCC_MTD_IO_locSetCycle( nDevInfo );
			}
		}
		//**************************************************************
		// Case on K9HDG08U5M Samsung NANDFLASH
		//**************************************************************
		else if ( nDevInfo->Feature.DeviceID.Code[0] == 0xEC &&
			 	  nDevInfo->Feature.DeviceID.Code[1] == 0xD7 &&
			 	  nDevInfo->Feature.DeviceID.Code[2] == 0x94 &&
			 	  nDevInfo->Feature.DeviceID.Code[3] == 0x72 &&
			 	  nDevInfo->Feature.DeviceID.Code[4] == 0x54 && 
			 	  nDevInfo->Feature.DeviceID.Code[5] == 0x42)
		{
			if ( TotalMediaNum == 4 )
			{
				nDevInfo->Feature.WCtime = 45;

				nDevInfo->Feature.WriteSTP 	= 0;
				nDevInfo->Feature.WriteWP 	= 30;
				nDevInfo->Feature.WriteHLD 	= 15;

				nDevInfo->Feature.ReadSTP	= 0;
				nDevInfo->Feature.ReadPW 	= 30;
				nDevInfo->Feature.ReadHLD 	= 15;

				TCC_MTD_IO_locSetCycle( nDevInfo );
			}
		}		
		//**************************************************************
		// Case on K9PFG08U5M Samsung NANDFLASH
		//**************************************************************
		else if ( nDevInfo->Feature.DeviceID.Code[0] == 0xEC &&
			 	  nDevInfo->Feature.DeviceID.Code[1] == 0xDE &&
			 	  nDevInfo->Feature.DeviceID.Code[2] == 0xD5 &&
			 	  nDevInfo->Feature.DeviceID.Code[3] == 0x72 &&
			 	  nDevInfo->Feature.DeviceID.Code[4] == 0x58 && 
			 	  nDevInfo->Feature.DeviceID.Code[5] == 0x42)
		{
			if ( ( TotalMediaNum >= 2 ) || ( nDevInfo->Feature.MediaType & A_PARALLEL ) )
			{
				nDevInfo->Feature.WCtime = 45;

				nDevInfo->Feature.WriteSTP 	= 0;
				nDevInfo->Feature.WriteWP 	= 30;
				nDevInfo->Feature.WriteHLD 	= 15;

				nDevInfo->Feature.ReadSTP	= 0;
				nDevInfo->Feature.ReadPW 	= 30;
				nDevInfo->Feature.ReadHLD 	= 15;

				TCC_MTD_IO_locSetCycle( nDevInfo );
			}
		}
		//**************************************************************
		// Case on TH58NVG5D4CTG20 Toshiba NANDFLASH
		//**************************************************************
		else if ( nDevInfo->Feature.DeviceID.Code[0] == 0x98 &&
			 	  nDevInfo->Feature.DeviceID.Code[1] == 0xD5 &&
			 	  nDevInfo->Feature.DeviceID.Code[2] == 0x85 &&
			 	  nDevInfo->Feature.DeviceID.Code[3] == 0xA5 )
		{
			if ( TotalMediaNum == 2 )
			{
				nDevInfo->Feature.WCtime = 30;

				nDevInfo->Feature.WriteSTP 	= 0;
				nDevInfo->Feature.WriteWP 	= 20;
				nDevInfo->Feature.WriteHLD 	= 10;

				nDevInfo->Feature.ReadSTP	= 0;
				nDevInfo->Feature.ReadPW 	= 20;
				nDevInfo->Feature.ReadHLD 	= 10;

				TCC_MTD_IO_locSetCycle( nDevInfo );
			}
		}

		//**************************************************************
		// Case on 29F64G08CFAAA Micron NANDFLASH
		//**************************************************************
		else if ( nDevInfo->Feature.DeviceID.Code[0] == 0x2C &&
			 	  nDevInfo->Feature.DeviceID.Code[1] == 0xD7 &&
			 	  nDevInfo->Feature.DeviceID.Code[2] == 0x94 &&
			 	  nDevInfo->Feature.DeviceID.Code[3] == 0x3E && 
  			 	  nDevInfo->Feature.DeviceID.Code[4] == 0x84 )
		{
			if ( TotalMediaNum == 2 )
			{
				nDevInfo->Feature.WCtime = 45;

				nDevInfo->Feature.WriteSTP 	= 0;
				nDevInfo->Feature.WriteWP 	= 30;
				nDevInfo->Feature.WriteHLD 	= 15;

				nDevInfo->Feature.ReadSTP	= 0;
				nDevInfo->Feature.ReadPW 	= 30;
				nDevInfo->Feature.ReadHLD 	= 15;

				TCC_MTD_IO_locSetCycle( nDevInfo );
			}
		}

		//**************************************************************
		// Case on 29F64G08CFABA Micron NANDFLASH
		//**************************************************************
		else if ( nDevInfo->Feature.DeviceID.Code[0] == 0x2C &&
			 	  nDevInfo->Feature.DeviceID.Code[1] == 0x68 &&
			 	  nDevInfo->Feature.DeviceID.Code[2] == 0x04 &&
			 	  nDevInfo->Feature.DeviceID.Code[3] == 0x46 && 
  			 	  nDevInfo->Feature.DeviceID.Code[4] == 0x89 )
		{
			if ( TotalMediaNum == 2 )
			{
				nDevInfo->Feature.WCtime = 45;

				nDevInfo->Feature.WriteSTP 	= 0;
				nDevInfo->Feature.WriteWP 	= 40;
				nDevInfo->Feature.WriteHLD 	= 15;

				nDevInfo->Feature.ReadSTP	= 0;
				nDevInfo->Feature.ReadPW 	= 30;
				nDevInfo->Feature.ReadHLD 	= 15;

				TCC_MTD_IO_locSetCycle( nDevInfo );
			}
		}

		//**************************************************************
		// Case on 29F128G08CJAA Micron NANDFLASH
		//**************************************************************
		else if ( nDevInfo->Feature.DeviceID.Code[0] == 0x2C &&
			 	  nDevInfo->Feature.DeviceID.Code[1] == 0xD9 &&
			 	  nDevInfo->Feature.DeviceID.Code[2] == 0xD5 &&
			 	  nDevInfo->Feature.DeviceID.Code[3] == 0x3E && 
  			 	  nDevInfo->Feature.DeviceID.Code[4] == 0x88 )
		{
			if ( ( TotalMediaNum == 2 ) || ( nDevInfo->Feature.MediaType & A_PARALLEL ) )
			{
				nDevInfo->Feature.WCtime = 45;

				nDevInfo->Feature.WriteSTP 	= 0;
				nDevInfo->Feature.WriteWP 	= 40;
				nDevInfo->Feature.WriteHLD 	= 15;

				nDevInfo->Feature.ReadSTP	= 0;
				nDevInfo->Feature.ReadPW 	= 30;
				nDevInfo->Feature.ReadHLD 	= 15;

				TCC_MTD_IO_locSetCycle( nDevInfo );
			}
		}

		//**************************************************************
		// Case on 29F128G08CJABA(2CS, 8bit), 29F128G08CKABA (1CS, 16bit) Micron NANDFLASH
		//**************************************************************
		else if ( nDevInfo->Feature.DeviceID.Code[0] == 0x2C &&
			 	  nDevInfo->Feature.DeviceID.Code[1] == 0x88 &&
			 	  nDevInfo->Feature.DeviceID.Code[2] == 0x05 &&
			 	  nDevInfo->Feature.DeviceID.Code[3] == 0xC6 && 
  			 	  nDevInfo->Feature.DeviceID.Code[4] == 0x89 )
		{
			if (( TotalMediaNum == 2 ) || ( nDevInfo->Feature.MediaType & A_PARALLEL ))
			{
				nDevInfo->Feature.WCtime = 45;

				nDevInfo->Feature.WriteSTP 	= 0;
				nDevInfo->Feature.WriteWP 	= 30;
				nDevInfo->Feature.WriteHLD 	= 15;

				nDevInfo->Feature.ReadSTP	= 0;
				nDevInfo->Feature.ReadPW 	= 30;
				nDevInfo->Feature.ReadHLD 	= 15;

				TCC_MTD_IO_locSetCycle( nDevInfo );
			}
		}
	}

	return (TCC_MTD_IO_ERROR)SUCCESS;
}


/**************************************************************************
*  FUNCTION NAME : 
*  
*      static void TCC_MTD_IO_Cmdfunc(struct mtd_info *mtd, unsigned command, int column, int page_addr);
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			column	= 
*			command	= 
*			mtd	= 
*			page_addr	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static void TCC_MTD_IO_Cmdfunc(struct mtd_info *mtd, unsigned command, int column, int page_addr)
{
	struct nand_chip *tcc_nand = mtd->priv;
	struct tcc_nand_info *info = tcc_nand->priv;
	
	unsigned int	 		nPageAddr = 0, nChipNo = 0;
	TCC_MTD_IO_DEVINFO		*sDevInfo;
	TCC_MTD_IO_ERROR		res;

	sDevInfo = &gDevInfo[0];
	
	//#define NAND_CMD_PAGEPROG		0x10

	//#define NAND_CMD_READ0		0
	//#define NAND_CMD_READ1		1
	//#define NAND_CMD_RNDOUT		5
	//#define NAND_CMD_READOOB		0x50

	//#define NAND_CMD_ERASE1		0x60
	//#define NAND_CMD_ERASE2		0xd0
	
	//#define NAND_CMD_STATUS		0x70
	//#define NAND_CMD_STATUS_MULTI	0x71

	//#define NAND_CMD_SEQIN		0x80
	//#define NAND_CMD_RNDIN		0x85

	//#define NAND_CMD_READID		0x90
	//#define NAND_CMD_RESET		0xff

	#ifdef TCC_MTD_DEBUG
	printk("\nNAND MTD CMD:0x%02X", command);
	#endif

	switch (command)
	{
		//----------------------------------
		// Page Program
		//----------------------------------
		case NAND_CMD_SEQIN:
		{
			sDevInfo->CmdBuf.CmdStatus	= TCC_MTD_CMD_PROGRAM_START;
			sDevInfo->CmdBuf.ColAddr 	= column;
			sDevInfo->CmdBuf.RowAddr	= page_addr;
			break;
		}
		case NAND_CMD_PAGEPROG:
		{	
			//mutex_lock(&mutex_tccnfc);
			#ifdef MUTEX_LOG
			printk("+in[%s,%d]\n", __FUNCTION__, __LINE__);
			#endif

			/* Command Page Program #2 [ 0x10 ] */
			pMTD_NFC->NFC_CMD = sDevInfo->CmdMask & 0x1010;

			#ifdef MUTEX_LOG
			printk("-out[%s,%d]\n", __FUNCTION__, __LINE__);
			#endif
			//mutex_lock(&mutex_tccnfc);
			break;
		}
		case NAND_CMD_RNDIN:
		{
			// Random Data Input: [0x85] - <ColAddr1> - <ColAddr2>
			//mutex_lock(&mutex_tccnfc);
			#ifdef MUTEX_LOG
			printk("+in[%s,%d]\n", __FUNCTION__, __LINE__);
			#endif

			#ifdef TCC_MTD_DEBUG_SPARE
			printk("[MTD]NAND_CMD_RNDIN\n");
			#endif			
			
			pMTD_NFC->NFC_CMD = sDevInfo->CmdMask & 0x8585;
			TCC_MTD_IO_locWriteColAddr( column );

			#ifdef MUTEX_LOG
			printk("-out[%s,%d]\n", __FUNCTION__, __LINE__);
			#endif			
			//mutex_lock(&mutex_tccnfc);

			break;			
		}
		//----------------------------------
		// Read
		//----------------------------------
		case NAND_CMD_READ0:
		{
			sDevInfo->CmdBuf.CmdStatus	= TCC_MTD_CMD_READ_PAGE;
			sDevInfo->CmdBuf.ColAddr 	= column;
			sDevInfo->CmdBuf.RowAddr	= page_addr;
			break;				
		}
		case NAND_CMD_RNDOUT:
		{
			break;
		}
		case NAND_CMD_READOOB:
		{
			sDevInfo->CmdBuf.CmdStatus	= TCC_MTD_CMD_READ_SPARE;
			sDevInfo->CmdBuf.ColAddr 	= 0;
			sDevInfo->CmdBuf.RowAddr	= page_addr;
			break;
		}
		case NAND_CMD_ERASE1:
		{
			//mutex_lock(&mutex_tccnfc);

			#ifdef MUTEX_LOG
			printk("+in[%s,%d]\n", __FUNCTION__, __LINE__);
			#endif
			TCC_MTD_IO_locSetCommCycleTime();
			//res = TCC_MTD_IO_locGetPhyBlkAddr( sDevInfo, page_addr, &nPageAddr, &nChipNo );

			if(s_pTccMtdGlueDrv)
			{
				res = s_pTccMtdGlueDrv->locGetPhyBlkAddr( &info->glue_info, sDevInfo, page_addr, &nPageAddr, &nChipNo );
				info->glue_info.gTCC_MTD_CurrCS	=	nChipNo;
				sDevInfo						= &gDevInfo[nChipNo];
			}				
			else
				res = ERR_TCC_MTD_IO_FAILED_NO_GLUE_DRV;

			if ( res == SUCCESS )
			{
				TCC_MTD_IO_locCheckForExtendBlockAccess( sDevInfo, &nPageAddr );

				if( sDevInfo->ExtInterleaveUsable == TRUE )
				{
					TCC_MTD_IO_locWaitBusyForInterleave( sDevInfo, nPageAddr );
				}
				else
				{
					TCC_MTD_IO_locEnableChipSelect( 0 );
					TCC_MTD_IO_locDisableWriteProtect();
				}
				
				#ifdef TCC_MTD_DEBUG_SPARE
				printk("[MTD-Erase]Blk:%04d-%04d\n", page_addr >> sDevInfo->ShiftPpB, nPageAddr >> sDevInfo->ShiftPpB );
				#endif
				
				pMTD_NFC->NFC_CMD = sDevInfo->CmdMask & 0x6060;
				TCC_MTD_IO_locWriteBlockPageAddr( nPageAddr );				
			}
			#ifdef MUTEX_LOG
			printk("-out[%s,%d]\n", __FUNCTION__, __LINE__);
			#endif
			
			//mutex_lock(&mutex_tccnfc);

			break;				
		}
		case NAND_CMD_ERASE2:
		{
			#ifdef TCC_MTD_DEBUG
			printk("\nBlockErase:0xD0\n");
			#endif

			#ifdef MUTEX_LOG
			printk("+in[%s,%d]\n", __FUNCTION__, __LINE__);
			#endif
			//mutex_lock(&mutex_tccnfc);
			TCC_MTD_IO_locSetCommCycleTime();			
			pMTD_NFC->NFC_CMD = sDevInfo->CmdMask & 0xD0D0;

			#ifdef MUTEX_LOG
			printk("-out[%s,%d]\n", __FUNCTION__, __LINE__);
			#endif
			
			//mutex_lock(&mutex_tccnfc);
				
			break;
		}
		case NAND_CMD_READID:
		{
			sDevInfo = &gDevInfo[0];
			
			if (( sDevInfo->Feature.MediaType & A_16BIT ) || 
				( sDevInfo->Feature.PBpV == 16384 ) || 
				( sDevInfo->Feature.PpB > 128 ) || 
				( sDevInfo->Feature.PageSize > 2048 ))
			{
				sDevInfo->CmdBuf.CmdStatus	= TCC_MTD_CMD_READ_ID_1ST;		// Read ID: 2 Byte
			}
			else
			{
				sDevInfo->CmdBuf.CmdStatus	= TCC_MTD_CMD_CLEAR;
			}
			
			TCC_MTD_IO_ReadID( TCC_MTD_IO_PARALLEL_COMBINATION_MODE );
			break;
		}
		case NAND_CMD_STATUS:
		{
			//mutex_lock(&mutex_tccnfc);
			#ifdef MUTEX_LOG
			printk("+in[%s,%d]\n", __FUNCTION__, __LINE__);
			#endif
			TCC_MTD_IO_locSetCommCycleTime();
			pMTD_NFC->NFC_CMD = sDevInfo->CmdMask & 0x7070;

			// Delay : more than 200nS
		   	TCC_MTD_IO_locDelay();
			
			#ifdef MUTEX_LOG
			printk("-out[%s,%d]\n", __FUNCTION__, __LINE__);
			#endif
			
			//mutex_lock(&mutex_tccnfc);
			break;
		}
		case NAND_CMD_RESET:
		{
			//mutex_lock(&mutex_tccnfc);
			#ifdef MUTEX_LOG
			printk("+in[%s,%d]\n", __FUNCTION__, __LINE__);
			#endif
			TCC_MTD_IO_locReset( TCC_MTD_IO_PARALLEL_COMBINATION_MODE );
			//mutex_lock(&mutex_tccnfc);			

			#ifdef MUTEX_LOG
			printk("-out[%s,%d]\n", __FUNCTION__, __LINE__);
			#endif
			return;			
		}
			/* This applies to read commands */
		default:
			/*
			 * If we don't have access to the busy pin, we apply the given
			 * command delay
			 */
			return;
	}

	memcpy( &sDevInfo[1].CmdBuf, &sDevInfo[0].CmdBuf, sizeof(TCC_MTD_IO_CMDBUF) );
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static void TCC_MTD_IO_Select_chip(struct mtd_info *mtd, int chipnr);
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			chipnr	= 
*			mtd	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static void TCC_MTD_IO_Select_chip(struct mtd_info *mtd, int chipnr)
{
	if (chipnr == -1 )
	{
		TCC_MTD_IO_locEnableWriteProtect();
		TCC_MTD_IO_locDisableChipSelect();		
	}
	else if ( chipnr < 4 )
	{

		TCC_MTD_IO_locDisableWriteProtect();
		#ifdef CE_FIX
		TCC_MTD_IO_locEnableChipSelect(0);
		#else
		TCC_MTD_IO_locEnableChipSelect(chipnr);
		#endif	// EC_FIX
	}

	return;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      void TCC_MTD_IO_ReadID( int nMode );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nMode	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
void TCC_MTD_IO_ReadID( int nMode )
{
	/* Pre Process */
	//TCC_MTD_IO_PreProcess();

	/* Set Setup Time and Hold Time */
	TCC_MTD_IO_locSetBasicCycleTime();

	/* Enable Chip Select */
	TCC_MTD_IO_locEnableChipSelect(0);

	/* Set Data Bus as 16Bit */
	TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	
	/* Parallel Composition */
	if ( nMode == TCC_MTD_IO_PARALLEL_COMBINATION_MODE )
	{
		pMTD_NFC->NFC_CMD		= 0x9090;	/* Command READ ID [ 0x90 ] */
		pMTD_NFC->NFC_SADDR		= 0x0000;	/* Address [ 0x00 ] */
	}
	/* Serial Composition */
	else
	{
		pMTD_NFC->NFC_CMD		= 0x0090;	/* Command READ ID [ 0x90 ] */
		pMTD_NFC->NFC_SADDR		= 0x0000;	/* Address [ 0x00 ] */
	}
	
	/* Delay : tAR1[READID] Max 200nS */
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static void TCC_MTD_IO_Read_buf(struct mtd_info *mtd, u_char *buf, int len);
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			buf	= 
*			len	= 
*			mtd	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static void TCC_MTD_IO_Read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	struct nand_chip *tcc_nand = mtd->priv;
	struct tcc_nand_info *info = tcc_nand->priv;
	
	#ifdef TCC_MTD_DEBUG_INPUT_OOB
	unsigned int			j;
	#endif

	#ifdef TCC_MTD_DEBUG_SPARE
	unsigned int			nBlockAddr, nPpB;
	#endif
	unsigned int	 		nPageAddr = 0, nChipNo = 0;
  	unsigned short int		nStartPPage, nReadPPSize;
  	unsigned char			*nPageBuffer, *nSpareBuffer;
	TCC_MTD_IO_DEVINFO 		*nDevInfo;
	TCC_MTD_IO_ERROR		res;

	nDevInfo = &gDevInfo[0];

	#ifdef TCC_MTD_DEBUG_FUNC
	printk("\nRead_buf:%02x\n", nDevInfo->CmdBuf.CmdStatus);
	#endif

	//mutex_lock(&mutex_tccnfc);
	#ifdef MUTEX_LOG
	printk("+in[%s,%d]\n", __FUNCTION__, __LINE__);
	#endif

	if ( nDevInfo->CmdBuf.CmdStatus == TCC_MTD_CMD_READ_PAGE )
	{
		nStartPPage = nDevInfo->CmdBuf.ColAddr >> 9;

		nReadPPSize = len >> 9;
		if ( len < 512 )
			++nReadPPSize;

		nPageBuffer		= (unsigned char*)buf;
		nSpareBuffer	= (unsigned char*)gSpareBuffer;

		memset( (void*)nSpareBuffer, 0xFF, 64);

		if(s_pTccMtdGlueDrv)
		{
			res = s_pTccMtdGlueDrv->locGetPhyBlkAddr( &info->glue_info, nDevInfo, nDevInfo->CmdBuf.RowAddr, &nPageAddr, &nChipNo );
			info->glue_info.gTCC_MTD_CurrCS = nChipNo;
			nDevInfo = &gDevInfo[nChipNo];
		}
		else
			res = ERR_TCC_MTD_IO_FAILED_NO_GLUE_DRV;
		
		if ( res == SUCCESS )
		{
			#ifdef TCC_MTD_DEBUG_SPARE
			nBlockAddr =  nPageAddr >> nDevInfo->ShiftPpB;
			nPpB = nPageAddr - (nBlockAddr <<nDevInfo->ShiftPpB);
			printk("[MTD-Read-P]CS:%d,Blk:%04d-%04d,PpB:%03d,StPPage:%d,len:%d\n", nChipNo, nDevInfo->CmdBuf.RowAddr>>nDevInfo->ShiftPpB, nBlockAddr, nPpB, nStartPPage, len);
			#endif

			#if defined(__USE_MTD_NAND_ISR__)
			TCC_MTD_IO_IRQ_Enable( 1 );
			#endif
	
			res = TCC_MTD_IO_locReadPage( nDevInfo, nPageAddr, nStartPPage, nReadPPSize, nPageBuffer, nSpareBuffer, ECC_ON );

			#if defined(__USE_MTD_NAND_ISR__)
			TCC_MTD_IO_IRQ_Enable( 0 );
			#endif
				
			if( res != SUCCESS )
			{
				printk("[MTD-ReadPage-Fail] [ChipNo:%d][BlkNum:%d] [PageAddr:%d] [Error:0x%08x]\n"
						, nChipNo, nPageAddr>>nDevInfo->ShiftPpB, nPageAddr, res );
		
			}
			
		}
		
		nDevInfo->CmdBuf.CmdStatus = TCC_MTD_CMD_READ_SPARE_SEQ;

		if( nChipNo == 1 )
			gDevInfo[0].CmdBuf.CmdStatus =TCC_MTD_CMD_READ_SPARE_SEQ;
	}
	else if ( nDevInfo->CmdBuf.CmdStatus == TCC_MTD_CMD_READ_SPARE )
	{
		nSpareBuffer = (unsigned char*)gSpareBuffer;

		memset( (void*)nSpareBuffer, 0xFF, len);

		if(s_pTccMtdGlueDrv)
		{
			res = s_pTccMtdGlueDrv->locGetPhyBlkAddr( &info->glue_info, nDevInfo, nDevInfo->CmdBuf.RowAddr, &nPageAddr, &nChipNo );

			if( ( nChipNo != 0 ) && ( nChipNo != 1 ) )
				return ;

			if( nDevInfo->ExtInterleaveUsable == TRUE )
			{
				info->glue_info.gTCC_MTD_CurrCS = nChipNo;
				nDevInfo = &gDevInfo[nChipNo];
			}
			else
			{
				info->glue_info.gTCC_MTD_CurrCS = 0;
				nDevInfo = &gDevInfo[0];
			}
			
		}
		else
			res = ERR_TCC_MTD_IO_FAILED_NO_GLUE_DRV;
		
		if ( res == SUCCESS )
		{
			#ifdef TCC_MTD_DEBUG_SPARE
			nBlockAddr =  nPageAddr >> nDevInfo->ShiftPpB;
			nPpB = nPageAddr - (nBlockAddr <<nDevInfo->ShiftPpB);
			printk("[MTD-Read-S]CS:%d,Blk:%04d-%04d,PpB:%03d,len:%d\n", nChipNo, nDevInfo->CmdBuf.RowAddr>>nDevInfo->ShiftPpB, nBlockAddr, nPpB, 64);
			#endif

			res = TCC_MTD_IO_locReadSpare( nDevInfo, nPageAddr, nSpareBuffer );
		
			if( res != SUCCESS )
			{
				printk("[MTD-ReadSpare-Fail] [ChipNo:%d][BlkNum:%d] [Error:0x%08x] \n"
						, nChipNo, nPageAddr>>nDevInfo->ShiftPpB , res);
			}
			
		}

		memcpy(	(void*)buf, (void*)nSpareBuffer, len);

		//if( ( gMTDEdBlkAddr - gMTDStBlkAddr) < ( nDevInfo->CmdBuf.RowAddr >> nDevInfo->ShiftPpB ) )
		if( ( info->glue_info.gMTDEdBlkAddr - info->glue_info.gMTDStBlkAddr) < ( nDevInfo->CmdBuf.RowAddr >> nDevInfo->ShiftPpB ) )
		{
			memset(	&buf[0], 0xFF, len);
		}
	}
	else if ( nDevInfo->CmdBuf.CmdStatus == TCC_MTD_CMD_READ_SPARE_SEQ )
	{
		#ifdef TCC_MTD_DEBUG_SPARE
		printk("[MTD-Read-mem-Spare] len:%d\n", len);
		#endif
		
		//spare Buffer Copy
		nSpareBuffer = (unsigned char*)gSpareBuffer;
		memcpy(	(void*)buf, (void*)nSpareBuffer, len);

		#ifdef TCC_MTD_DEBUG_INPUT_OOB
		for ( j = 0; j < 40; ++j )
		{
			printk("%02X", nSpareBuffer[j] );
		}
		printk("\n");	
		#endif		
	}

	#ifdef MUTEX_LOG
	printk("-out[%s,%d]\n", __FUNCTION__, __LINE__);
	#endif	
	
	//mutex_lock(&mutex_tccnfc);
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static u_char TCC_MTD_IO_Read_byte(struct mtd_info *mtd);
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			mtd	= 
*  
*  OUTPUT:	u_char - Return Type
*  			= 
*  
**************************************************************************/
static u_char TCC_MTD_IO_Read_byte(struct mtd_info *mtd)
{
	#ifdef TCC_MTD_DEBUG_FUNC
	printk("\nRead_byte\n");
	#endif
	TCC_MTD_IO_DEVINFO 	*nDevInfo;
	u_char ret;

	nDevInfo = &gDevInfo[0];
	//mutex_lock(&mutex_tccnfc);
	#ifdef MUTEX_LOG
	printk("+in[%s,%d]\n", __FUNCTION__, __LINE__);
	#endif		

	if ( nDevInfo->Feature.MediaType & A_16BIT )
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	else
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

	if ( nDevInfo->CmdBuf.CmdStatus == TCC_MTD_CMD_READ_ID_1ST )
	{
		ret = pMTD_NFC->NFC_SDATA & 0x000000FF;
		ret = gDummyDevID[0];
		nDevInfo->CmdBuf.CmdStatus = TCC_MTD_CMD_READ_ID_2ND;
	}
	else if ( nDevInfo->CmdBuf.CmdStatus == TCC_MTD_CMD_READ_ID_2ND )
	{
		ret = pMTD_NFC->NFC_SDATA & 0x000000FF;
		ret = gDummyDevID[1];		// 2K Page NAND ID
		nDevInfo->CmdBuf.CmdStatus = TCC_MTD_CMD_READ_ID_3RD;
	}
	else if ( nDevInfo->CmdBuf.CmdStatus == TCC_MTD_CMD_READ_ID_3RD )
	{
		ret = pMTD_NFC->NFC_SDATA & 0x000000FF;
		ret = gDummyDevID[2];		// 2K Page NAND ID
		nDevInfo->CmdBuf.CmdStatus = TCC_MTD_CMD_READ_ID_4TH;
	}
	else if ( nDevInfo->CmdBuf.CmdStatus == TCC_MTD_CMD_READ_ID_4TH )
	{
		ret = pMTD_NFC->NFC_SDATA & 0x000000FF;
		ret = gDummyDevID[3];		// 2K Page NAND ID
		nDevInfo->CmdBuf.CmdStatus = TCC_MTD_CMD_CLEAR;
	}	
	else
	{

		#ifdef TCC_MTD_DEBUG_SPARE
		printk("\nRead_byte\n");
		#endif
			
		ret = pMTD_NFC->NFC_SDATA & 0x000000FF;
	}

	#ifdef MUTEX_LOG
	printk("-out[%s,%d]\n", __FUNCTION__, __LINE__);
	#endif	
	//mutex_lock(&mutex_tccnfc);

	return ret;
}
 
/**************************************************************************
*  FUNCTION NAME : 
*  
*      static void TCC_MTD_IO_Write_buf(struct mtd_info *mtd, const u_char *buf, int len);
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			buf	= 
*			len	= 
*			mtd	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static void TCC_MTD_IO_Write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int 	i;
	
//	#ifdef TCC_MTD_DEBUG_FUNC
	printk("\n-TCC_MTD_IO_Write_buf-len:%d\n",  len);
//	#endif
	//mutex_lock(&mutex_tccnfc);

	#ifdef MUTEX_LOG
	printk("+in[%s,%d]\n", __FUNCTION__, __LINE__);
	#endif	
	
	for (i = 0; i < len; i++)
	{
		pMTD_NFC->NFC_SDATA = 0x000000FF & buf[i];
	}

	#ifdef MUTEX_LOG
	printk("-out[%s,%d]\n", __FUNCTION__, __LINE__);
	#endif	
	//mutex_lock(&mutex_tccnfc);
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static int TCC_MTD_IO_Write_page(struct mtd_info *mtd, struct nand_chip *chip,
*      			   const uint8_t *buf, int page, int cached, int raw);
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			buf	= 
*			cached	= 
*			chip	= 
*			mtd	= 
*			page	= 
*			raw	= 
*  
*  OUTPUT:	int - Return Type
*  			= 
*  
**************************************************************************/
static int TCC_MTD_IO_Write_page(struct mtd_info *mtd, struct nand_chip *chip,
			  					 const uint8_t *buf, int page, int cached, int raw)
{
	struct nand_chip *tcc_nand = mtd->priv;
	struct tcc_nand_info *info = tcc_nand->priv;

	#ifdef TCC_MTD_DEBUG_INPUT_OOB
	unsigned int		i, j;
	#endif
	unsigned int		nPageAddr = 0, nChipNo = 0;
	unsigned int		RowAddr, ColumnAddr;
	unsigned short int	nFlagOfAlign = DISABLE;
	unsigned short int	rStartPPage, rWritePPSize;
	unsigned short int	nStartPPage, nWritePPSize;
	unsigned char		*cPageBuffer = 0, *cSpareBuffer = 0;
	unsigned char		*nDataBuffer, *nSpareBuffer;
	TCC_MTD_IO_DEVINFO 	*nDevInfo;
	TCC_MTD_IO_ERROR	res;

	nDevInfo = &gDevInfo[0];

	// Page Data :  buf, 	mtd->writesize
	// Spare Data: chip->oob_poi, mtd->oobsize

	#if defined(__USE_MTD_NAND_ISR__)
	TCC_MTD_IO_IRQ_Enable( 1 );
	#endif
	//=============================================	
	// Check Device and Parameter
	//=============================================	
	if ( mtd->writesize == 0 )
		return ERR_TCC_MTD_IO_WRONG_PARAMETER;
	
	if ( !( nDevInfo->IoStatus & TCC_MTD_IO_STATUS_ENABLE ))
		return ERR_TCC_MTD_IO_NOT_READY_DEVICE_IO;
		
	//mutex_lock(&mutex_tccnfc);
	#ifdef MUTEX_LOG
	printk("+in[%s,%d]\n", __FUNCTION__, __LINE__);
	#endif	

	if(s_pTccMtdGlueDrv)
	{
		res = s_pTccMtdGlueDrv->locGetPhyBlkAddr( &info->glue_info, nDevInfo, page, &nPageAddr, &nChipNo );
		info->glue_info.gTCC_MTD_CurrCS = nChipNo;
		nDevInfo	= &gDevInfo[nChipNo];
		
	}
	else
		res = ERR_TCC_MTD_IO_FAILED_NO_GLUE_DRV;
	
	if ( res != SUCCESS )
		goto ErrorWritePage;

	#ifdef TCC_MTD_DEBUG_SPARE	
	unsigned int		nBlockAddr, nPpB;
	nBlockAddr 	=  nPageAddr >> nDevInfo->ShiftPpB;
	nPpB 		= nPageAddr - (nBlockAddr <<nDevInfo->ShiftPpB);
	printk("[MTD-WritePage]CS:%d, Blk:%04d-%04d,PpB:%03d,len:%d\n", nChipNo, page >> nDevInfo->ShiftPpB, nBlockAddr, nPpB, mtd->writesize);
	#endif
		
	nStartPPage 	= 0;
	nWritePPSize   	= mtd->writesize >> 9;

	nDataBuffer 	= (unsigned char*)buf;
	nSpareBuffer 	= (unsigned char*)chip->oob_poi;

	#ifdef TCC_MTD_DEBUG_INPUT_OOB
	for ( j = 0; j < 20; ++j )
	{
		printk("%02X", nSpareBuffer[j] );
	}
	printk("\n");	
	#endif

	TCC_MTD_IO_LineUpToBytesPerSector( nDevInfo, nStartPPage, nWritePPSize, &rStartPPage, &rWritePPSize, &nFlagOfAlign );	
	if ( ( rStartPPage + rWritePPSize ) > nDevInfo->UsablePPages )
		return ERR_TCC_MTD_IO_WRONG_PARAMETER;
	TCC_MTD_IO_LineUpToBufferAlignement( nDevInfo, nFlagOfAlign, nStartPPage, nWritePPSize, nDataBuffer, nSpareBuffer, &cPageBuffer, &cSpareBuffer, TCC_MTD_IO_WRITE_MODE );
	
	//=============================================
	// PreProcess
	// Set Setup Time and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP HIGH
	//=============================================	
	TCC_MTD_IO_locSetCommCycleTime();
	//TCC_MTD_IO_locEnableChipSelect(0);
	//TCC_MTD_IO_locDisableWriteProtect();

	TCC_MTD_IO_locCheckForExtendBlockAccess( nDevInfo, &nPageAddr );

	if( gDevInfo[nChipNo].ExtInterleaveUsable == TRUE )
	{
		res = TCC_MTD_IO_locWaitBusyForInterleave( nDevInfo, nPageAddr );
		if( res != SUCCESS )
		{
			printk(" !!!!! TCC_MTD_IO_locWaitBusyForInterleave Failed [0x%08X] !!!! \n", res);
		}
	}
	else
	{
		TCC_MTD_IO_locEnableChipSelect(0);
		TCC_MTD_IO_locDisableWriteProtect();
	}

	//=============================================
	// Write Data
	//=============================================
	res = TCC_MTD_IO_locPreEncodeDataForLastPage( nDevInfo, cPageBuffer, cSpareBuffer, ECC_ON );

	if( res == ERR_TCC_MTD_IO_ATTEMPT_REWRITE )
	{
		printk(" !!! TCC_MTD_IO_locPreEncodeDataForLastPage Func FAIL !!! \n");
		printk(" !!! Attempt ReWRite FUNC \n ");

		TCC_MTD_IO_locEnableWriteProtect();
		emergency_restart();
	
		return (TCC_MTD_IO_ERROR)SUCCESS;			
	}
	
	if ( res != SUCCESS )
		goto ErrorWritePage;
		
	//=============================================
	// Write Data
	//=============================================
	/* Generate Row and Column Address */
	res = TCC_MTD_IO_locGenerateRowColAddrForWrite( nPageAddr, ( rStartPPage << nDevInfo->ShiftBytesPerSector ), &RowAddr, &ColumnAddr );
	if ( res != SUCCESS )
		goto ErrorWritePage;

	/* Command Page Program #1 [ 0x80 ] */
	pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0x8080;

	/* Write Row and Column Address */
	TCC_MTD_IO_locWriteRowColAddr( RowAddr, ColumnAddr );

	/* Change Cycle */
	TCC_MTD_IO_locSetWriteCycleTime();

	/* Write Data to NAND FLASH */
	#if defined(TCC_MTD_DMA_ACCESS) && !defined(__USE_MTD_NAND_ISR__)
	res = TCC_MTD_IO_locWrite512DataDoubleBuf(  mtd,
												rStartPPage,
												rWritePPSize,
												cPageBuffer,
												cSpareBuffer);
	#else
	res = TCC_MTD_IO_locWrite512Data(   mtd,
										rStartPPage,
										rWritePPSize,
										cPageBuffer,
										cSpareBuffer);
	#endif

	if( res == ERR_TCC_MTD_IO_ATTEMPT_REWRITE )
	{
		printk(" !!! TCC_MTD_IO_locWrite512DataDoubleBuf Func FAIL !!! \n");
		printk(" !!! Attempt ReWRite FUNC \n ");
		TCC_MTD_IO_locSetCommCycleTime();
		pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0x1010;

		TCC_MTD_IO_locEnableWriteProtect();
		emergency_restart();
	
		return (TCC_MTD_IO_ERROR)SUCCESS;			
	}
	
	/* Change Cycle */
	TCC_MTD_IO_locSetCommCycleTime();

	if ( res != SUCCESS )
		goto ErrorWritePage;
	
	/* Command Page Program #2 [ 0x10 ] */
	pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0x1010;
	
    /* Wait until it is ready */
    TCC_MTD_IO_locWaitBusy();

	/* Check Status */
	res = TCC_MTD_IO_locReadStatus();	   

ErrorWritePage:
	//=============================================
	// FORCE TO SET WP LOW
	// Disable Chip Select
	// PostProcess
	//=============================================

	if( !( nDevInfo->ExtInterleaveUsable == TRUE ) )
		TCC_MTD_IO_locEnableWriteProtect();
	
	TCC_MTD_IO_locDisableChipSelect();
	//NAND_IO_PostProcess();

	#ifdef TCC_MTD_DEBUG
	printk("\n-[MTD-WritePage]");
	#endif	

	#ifdef MUTEX_LOG
	printk("-out[%s,%d]\n", __FUNCTION__, __LINE__);
	#endif	
	//mutex_lock(&mutex_tccnfc);

	#if defined(__USE_MTD_NAND_ISR__)
	TCC_MTD_IO_IRQ_Enable( 0 );
	#endif
	if ( res != SUCCESS )
	{
		printk("[TCC_MTD] Write Page Operation Fail !! \n");
		return res;
	}

   	return (TCC_MTD_IO_ERROR)SUCCESS;
}


/**************************************************************************
*  FUNCTION NAME : 
*      static __inline void NAND_IO_WaitBusyForInterleave( NAND_IO_DEVINFO *nDevInfo, U32 nPageAddr );
*  
*  DESCRIPTION : 
*  INPUT:
*			nDevInfo	= 
*			nPageAddr	= 
*  
*  OUTPUT:	void - Return Type
*  REMARK  :	
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locWaitBusyForInterleave( TCC_MTD_IO_DEVINFO *nDevInfo, U32 nPageAddr )
{
	TCC_MTD_IO_ERROR		res;

	TCC_MTD_IO_locDelay();
	//=============================================	
	// External Inter Leave
	//=============================================	

	TCC_MTD_IO_locEnableChipSelect( nDevInfo->ChipNo );
	TCC_MTD_IO_locDisableWriteProtect();

	res = TCC_MTD_IO_locReadStatusForExternalInterleave( nDevInfo );
	
	return res;
	
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline NAND_IO_ERROR TCC_MTD_IO_locPreEncodeDataForLastPage();
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nEccType	= 
*			nSpareBuffer	= 
*  
*  OUTPUT:	NAND_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locPreEncodeDataForLastPage( TCC_MTD_IO_DEVINFO *nDevInfo, U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff )
{
	unsigned int		i, j;
	unsigned char		*pPageB, *pSpareB;
	unsigned char		*pDataBuffer,*pSpareBuffer;
	unsigned int		nDataSize;
	DWORD_BYTE			uDWordByte;
	TCC_MTD_IO_ERROR	res;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );

	res = SUCCESS;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================	
	if ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	else
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

	//=========================================================================
	// Get Buffer Pointer
	//=========================================================================
	/* Adapt type of address */
	pPageB			= (unsigned char*)nPageBuffer;
	pSpareB			= (unsigned char*)nSpareBuffer;

	/* Get Buffer pointer */
	pDataBuffer		= (unsigned char*)pPageB;
	pSpareBuffer 	= (unsigned char*)pSpareB;

	//####################################################
	//Dummy Write Page Data
	//DataSize: 512(Last_PPage) + 20(Yaffs Tag) 
	//
	//
	//####################################################
	pDataBuffer += ( ( nDevInfo->BytesPerSector ) * ( nDevInfo->UsablePPages- 1 ) );
	
	nDataSize = ( nDevInfo->BytesPerSector + 20 );					
	memset( gTCC_MTD_IO_PreEnDecodeEccBuffer, 0xFF, nDataSize );
	memcpy( gTCC_MTD_IO_PreEnDecodeEccBuffer, pDataBuffer, nDevInfo->BytesPerSector );
	memcpy( &gTCC_MTD_IO_PreEnDecodeEccBuffer[nDevInfo->BytesPerSector], pSpareBuffer, 20);

	pDataBuffer		= (unsigned char*)gTCC_MTD_IO_PreEnDecodeEccBuffer;
	
	//----------------------------------------------
	//	MCU ACCESS
	//----------------------------------------------
	#if defined(TCC89XX) || defined(TCC92XX)
	if (!( pMTD_NFC->NFC_CTRL1 & Hw30 ))
	#elif defined(TCC93XX) || defined(TCC88XX)
	if (!( pMTD_NFC->NFC_CTRL & HwNFC_CTRL_DACK_EN ))
	#endif 
	{	
		if ( nEccOnOff == ECC_ON )
		{
			/* Setup ECC Block */
			TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
			pMTD_ECC->ECC_CTRL	|= ( nDataSize << ECC_SHIFT_DATASIZE );
			pMTD_ECC->ECC_CLEAR	= 0x00000000;

			#if defined(TCC93XX) || defined(TCC88XX)
			pMTD_NFC->NFC_CTRL		|= HwNFC_CTRL_ECCON_EN;		// NFC ECC Encode/Decode Enable
			#endif
		}
    	
		BITCSET( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_1 );	// 1R/W Burst Size
		pMTD_NFC->NFC_DSIZE	= nDataSize;								// NFC DSIZE SETTING
		#if defined(TCC89XX) || defined(TCC92XX) 
		pMTD_NFC->NFC_IREQ	 	= 0x77;											// pNFC->NFC_IREQ_FLAG1;
		#elif defined(TCC93XX)|| defined(TCC88XX)
		pMTD_NFC->NFC_CTRL 	 |= HwNFC_CTRL_PRSEL_P;
		pMTD_NFC->NFC_IRQCFG |= HwNFC_IRQCFG_PIEN;
		#endif

		#if defined(TCC89XX) || defined(TCC92XX)
		TCC_MTD_IO_IRQ_Mask();
		pMTD_NFC->NFC_PSTART  = 0;
		#elif defined(TCC93XX) || defined(TCC88XX)
		TCC_MTD_IO_IRQ_Mask();
		pMTD_NFC->NFC_PRSTART = 0;
		#endif 
			
		i = ( nDataSize >> 2 );
		do {
			#if defined(TCC89XX) || defined(TCC92XX)
			while (!( pMTD_NFC->NFC_CTRL & HwNFC_CTRL_FS_RDY ));
			#elif defined(TCC93XX) || defined(TCC88XX)	
			
			#if 1 //def FOR_LOOP
			for( j = 0; j < 10000000; j++ )
			{
				if( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY )
					break;

				ASM_NOP;
			}

			if( !(pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY) )
			{
				printk(" !!! MTD Write Fail [HwNFC_CTRL_FS_RDY]!!!\n ReWrite Attempt [Line:%d]\n", __LINE__);				
				TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, &gTCC_MTD_IO_PreEnDecodeEccBuffer[0] );
				return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
			}
			
			#else
			while (!( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY ));
			#endif
			
			#endif
				
			uDWordByte.BYTE[0] = *pDataBuffer;++pDataBuffer;
			uDWordByte.BYTE[1] = *pDataBuffer;++pDataBuffer;
			uDWordByte.BYTE[2] = *pDataBuffer;++pDataBuffer;
			uDWordByte.BYTE[3] = *pDataBuffer;++pDataBuffer;
			pMTD_NFC->NFC_LDATA	= uDWordByte.DWORD;
		}while(--i);

		#if defined(TCC89XX) || defined(TCC92XX) 
		while (ISZERO( pMTD_NFC->NFC_IREQ, HwNFC_IREQ_FLAG1 ));
		TCC_MTD_IO_IRQ_UnMask();
		#elif defined(TCC93XX) || defined(TCC88XX)

		#if 1 //def FOR_LOOP
		for( j = 0; j < 10000000; j++ )
		{
			if( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG )
				break;

			ASM_NOP;

		}

		if( !( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG ) )
		{
			printk(" !!! MTD Write Fail [HwNFC_IRQ_PFG]!!!\n ReWrite Attempt [Line:%d]\n", __LINE__);		
			TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, &gTCC_MTD_IO_PreEnDecodeEccBuffer[0] );
			return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
		}
		#else
		while (ISZERO( pMTD_NFC->NFC_IRQ, HwNFC_IRQ_PFG));
		
		#endif
		
		pMTD_NFC->NFC_IRQ 	|= HwNFC_IRQ_PFG;
		TCC_MTD_IO_IRQ_UnMask();
		#endif 
	}
	else
	{
		if ( nEccOnOff == ECC_ON )
		{
			/* Setup ECC Block */
			TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwDATA_PA );
			pMTD_ECC->ECC_CTRL	|= ( ( nDevInfo->BytesPerSector + 20 ) << ECC_SHIFT_DATASIZE );
			pMTD_ECC->ECC_CLEAR	 = 0x00000000;

			#if defined(TCC93XX) || defined(TCC88XX)
			pMTD_NFC->NFC_CTRL		|= HwNFC_CTRL_ECCON_EN;		// NFC ECC Encode/Decode Enable
			#endif
		}

		i = ( nDataSize >> 2 );
		do {
			uDWordByte.BYTE[0] = *pDataBuffer;++pDataBuffer;
			uDWordByte.BYTE[1] = *pDataBuffer;++pDataBuffer;
			uDWordByte.BYTE[2] = *pDataBuffer;++pDataBuffer;
			uDWordByte.BYTE[3] = *pDataBuffer;++pDataBuffer;
			pMTD_NFC->NFC_WDATA	= uDWordByte.DWORD;
		}while(--i);
	}
	
	/*	Load ECC code from ECC block */
	if ( nEccOnOff == ECC_ON )
	{
		res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, &gTCC_MTD_IO_PreEnDecodeEccBuffer[0] );
		if ( res != SUCCESS )
			goto ErrorWrite512Data;
	}

	/* Disable ECC Block */
	if ( nEccOnOff == ECC_ON )
		TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );
	
	//=========================================================================
	// Return
	//=========================================================================
ErrorWrite512Data:
	return res;
}


/**************************************************************************
*  FUNCTION NAME : 
*  
*      static int TCC_MTD_IO_Dev_ready( struct mtd_info *mtd );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			mtd	= 
*  
*  OUTPUT:	int - Return Type
*  			= 
*  
**************************************************************************/
static int TCC_MTD_IO_Dev_ready( struct mtd_info *mtd )
{
   	TCC_MTD_IO_locDelay();
	//mutex_lock(&mutex_tccnfc);
	#ifdef MUTEX_LOG
	printk("+in[%s,%d]\n", __FUNCTION__, __LINE__);
	#endif		
	while (TCC_MTD_IO_locCheckReadyAndBusy());	


	#ifdef MUTEX_LOG
	printk("-out[%s,%d]\n", __FUNCTION__, __LINE__);
	#endif
	
	//mutex_lock(&mutex_tccnfc);
	return 0;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static int TCC_MTD_IO_Waitfunc(struct mtd_info *mtd, struct nand_chip *this);
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			mtd	= 
*			this	= 
*  
*  OUTPUT:	int - Return Type
*  			= 
*  
**************************************************************************/
static int TCC_MTD_IO_Waitfunc(struct mtd_info *mtd, struct nand_chip *this)
{
	unsigned int	res;

	//mutex_lock(&mutex_tccnfc);

	#ifdef MUTEX_LOG
	printk("+in[%s,%d]\n", __FUNCTION__, __LINE__);
	#endif	
	
	res = TCC_MTD_IO_locReadStatus();

	#ifdef MUTEX_LOG
	printk("-out[%s,%d]\n", __FUNCTION__, __LINE__);
	#endif	

	//mutex_lock(&mutex_tccnfc);
	#ifdef TCC_MTD_DEBUG
	printk("\nTCC_MTD_ReadStatus:%02d", res);
	#endif

	if ( res == 0 )
		return 0;
	else
		return 0x01;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      void TCC_MTD_IO_locReadID( U16 nChipNo, TCC_MTD_IO_DEVID *nDeviceCode, int nMode );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nChipNo	= 
*			nDeviceCode	= 
*			nMode	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
void TCC_MTD_IO_locReadID( U16 nChipNo, TCC_MTD_IO_DEVID *nDeviceCode, int nMode )
{
	/* Pre Process */
	//TCC_MTD_IO_PreProcess();

	/* Set Setup Time and Hold Time */
	TCC_MTD_IO_locSetBasicCycleTime();

	/* Enable Chip Select */
	TCC_MTD_IO_locEnableChipSelect( nChipNo );

	/* Set Data Bus as 16Bit */
	TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	
	/* Parallel Composition */
	if ( nMode == TCC_MTD_IO_PARALLEL_COMBINATION_MODE )
	{
		pMTD_NFC->NFC_CMD		= 0x9090;	/* Command READ ID [ 0x90 ] */
		pMTD_NFC->NFC_SADDR		= 0x0000;	/* Address [ 0x00 ] */
	}
	/* Serial Composition */
	else
	{
		pMTD_NFC->NFC_CMD		= 0x0090;	/* Command READ ID [ 0x90 ] */
		pMTD_NFC->NFC_SADDR		= 0x0000;	/* Address [ 0x00 ] */
	}

	#if defined(TCC93XX) || defined(TCC88XX)
	pMTD_NFC->NFC_CTRL	|= HwNFC_CTRL_RDNDLY_7;
	#endif
	
	/* Delay : tAR1[READID] Max 200nS */
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;


	/* Parallel Composition */
	if ( nMode == TCC_MTD_IO_PARALLEL_COMBINATION_MODE )
	{
		nDeviceCode->Code[0] = (U16)pMTD_NFC->NFC_SDATA;
		nDeviceCode->Code[1] = (U16)pMTD_NFC->NFC_SDATA;
		nDeviceCode->Code[2] = (U16)pMTD_NFC->NFC_SDATA;
		nDeviceCode->Code[3] = (U16)pMTD_NFC->NFC_SDATA;
		nDeviceCode->Code[4] = (U16)pMTD_NFC->NFC_SDATA;
		nDeviceCode->Code[5] = (U16)pMTD_NFC->NFC_SDATA;
	}	
	/* Serial Composition */
	else
	{
		nDeviceCode->Code[0] = (U8)( pMTD_NFC->NFC_SDATA & 0xFF );
		nDeviceCode->Code[1] = (U8)( pMTD_NFC->NFC_SDATA & 0xFF );
		nDeviceCode->Code[2] = (U8)( pMTD_NFC->NFC_SDATA & 0xFF );
		nDeviceCode->Code[3] = (U8)( pMTD_NFC->NFC_SDATA & 0xFF );
		nDeviceCode->Code[4] = (U8)( pMTD_NFC->NFC_SDATA & 0xFF );
		nDeviceCode->Code[5] = (U8)( pMTD_NFC->NFC_SDATA & 0xFF );
	}	

	#if defined(TCC93XX) || defined(TCC88XX)
	BITCLR(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_RDNDLY_7);
	#endif 
	 
	/* Disable Chip Select */
	TCC_MTD_IO_locDisableChipSelect();

	/* Post Process */
	//NAND_IO_PostProcess();
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      TCC_MTD_IO_ERROR TCC_MTD_IO_locGetDeviceInfo( U16 nChipNo, TCC_MTD_IO_DEVINFO *nDevInfo );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nChipNo	= 
*			nDevInfo	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
TCC_MTD_IO_ERROR TCC_MTD_IO_locGetDeviceInfo( U16 nChipNo, TCC_MTD_IO_DEVINFO *nDevInfo )
{

	unsigned short int		j,k,l;
	unsigned char			bFindMedia;
	unsigned char			bFindMakerNo;
	unsigned char			bMatchCount;
	TCC_MTD_IO_DEVID		sDeviceCode1,sDeviceCode2;
	TCC_MTD_IO_FEATURE		*sTempFeatureInfo;
	TCC_MTD_IO_FEATURE		*sFindFeatureInfo;
	TCC_MTD_IO_ERROR		res;
	
	bFindMedia 				= FALSE;
	nDevInfo->IoStatus		= 0; 
	nDevInfo->ChipNo		= 0xFF;

	// Init Variable
	sTempFeatureInfo		= (TCC_MTD_IO_FEATURE*)NAND_SupportMakerInfo.DevInfo[0];
	sFindFeatureInfo		= (TCC_MTD_IO_FEATURE*)NAND_SupportMakerInfo.DevInfo[0];

	if (!( gMTDNAND_PORT_STATUS & ( 1 << nChipNo )))
		return ERR_TCC_MTD_IO_FAILED_GET_DEVICE_INFO;
	
	//=====================================================================
	// Search Matched NANDFLASH
	//=====================================================================
	for ( j = 0; j < 3; ++j )	/* Check Read ID during 3 turn */
	{
		BITCLR( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_MASK_EN );
		/* Read Device CODE */
		TCC_MTD_IO_locResetForReadID( nChipNo, TCC_MTD_IO_SERIAL_COMBINATION_MODE );
		TCC_MTD_IO_locReadID( nChipNo, &sDeviceCode1, TCC_MTD_IO_SERIAL_COMBINATION_MODE );

		/* Check Maker ID */
		bFindMakerNo = 0xFF;
		for ( k = 0; k < MAX_SUPPORT_MAKER_NAND; ++k )
		{
			if ( sDeviceCode1.Code[0] == NAND_SupportMakerInfo.MakerID[k] )
			{
				bFindMakerNo		= (unsigned char)k;
				sTempFeatureInfo	= (TCC_MTD_IO_FEATURE*)NAND_SupportMakerInfo.DevInfo[k];
				break;
			}	
		}
			
		if ( bFindMakerNo >= MAX_SUPPORT_MAKER_NAND )
			continue;

		/* Check Device ID */
		for ( k = 0; k < NAND_SupportMakerInfo.MaxSupportNAND[bFindMakerNo]; ++k )
		{
			bMatchCount = 0;
			
			for ( l = 0; l < 5; ++l )
			{
				if ( sTempFeatureInfo->DeviceID.Code[l+1] == 0x00 )
					++bMatchCount;
				else if ( sDeviceCode1.Code[l+1] == sTempFeatureInfo->DeviceID.Code[l+1] )
					++bMatchCount;
			}

			/* Found NAND Device */
			if ( bMatchCount >= 5 )
			{
				bFindMedia = TRUE;
				sFindFeatureInfo = sTempFeatureInfo;
				break;
			}
			else
				++sTempFeatureInfo;
		}

		/* Found NAND Device */
		if ( bFindMedia == TRUE )
		{
			gMTDNAND_PORT_STATUS |= TCC_MTD_IO_PORT_DATA_WITDH_8BIT;
			break;
		}
	}

	if ( ( bFindMedia != TRUE ) && ( gMTDNAND_PORT_STATUS & TCC_MTD_IO_PORT_DATA_WITDH_16BIT ) )
	{
		//=====================================================================
		// Search Matched NANDFLASH (x16 Bit Serial NAND)
		//=====================================================================
		for ( j = 0; j < 3; ++j )	/* Check Read ID during 3 turn */
		{
			//IO_CKC_EnableBUS( IO_CKC_BUS_NFC );
			// 16Bit NAND Mask Enable
			BITSET( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_MASK_EN );

		    /* Read Device CODE */
			TCC_MTD_IO_locResetForReadID( nChipNo, TCC_MTD_IO_PARALLEL_COMBINATION_MODE );			// x16 Bit NAND Command
			TCC_MTD_IO_locReadID( nChipNo, &sDeviceCode1, TCC_MTD_IO_PARALLEL_COMBINATION_MODE );	// x16 Bit NAND Search
    
		    /* Check Maker ID */
		    bFindMakerNo = 0xFF;
		    for ( k = 0; k < MAX_SUPPORT_MAKER_NAND; ++k )
		    {
			    if ( sDeviceCode1.Code[0] == NAND_SupportMakerInfo.MakerID[k] )
			    {
				    bFindMakerNo		= (unsigned char)k;
				    sTempFeatureInfo	= (TCC_MTD_IO_FEATURE*)NAND_SupportMakerInfo.DevInfo[k];
				    break;
			    }	
		    }
			    
		    if ( bFindMakerNo >= MAX_SUPPORT_MAKER_NAND )
			    continue;
    
		    /* Check Device ID */
		    for ( k = 0; k < NAND_SupportMakerInfo.MaxSupportNAND[bFindMakerNo]; ++k )
		    {
			    bMatchCount = 0;
			    
			    for ( l = 0; l < 5; ++l )
			    {
				    if ( sTempFeatureInfo->DeviceID.Code[l+1] == 0x00 )
					    ++bMatchCount;
				    else if ( sDeviceCode1.Code[l+1] == sTempFeatureInfo->DeviceID.Code[l+1] )
					    ++bMatchCount;
			    }
    
			    /* Found NAND Device */
			    if ( bMatchCount >= 5 )
			    {
				    bFindMedia = TRUE;
				    sFindFeatureInfo = sTempFeatureInfo;
				    break;
			    }
			    else
				    ++sTempFeatureInfo;
		    }
    
		    /* Found NAND Device */
		    if ( bFindMedia == TRUE )
			{
				gMTDNAND_PORT_STATUS |= TCC_MTD_IO_PORT_DATA_WITDH_16BIT;
			    break;
			}
	    }
	}
	
	//=====================================================================
	// If Media is founded
	//=====================================================================
	if ( bFindMedia == TRUE )
	{
		/* Get NAND Feature Info */
		memcpy( (void*)&nDevInfo->Feature,
			 	(void*)sFindFeatureInfo,
				sizeof(TCC_MTD_IO_FEATURE) );

		/* Get ECC Type Info */
		if ( nDevInfo->Feature.MediaType & A_SLC )
		{
			nDevInfo->EccType = TYPE_ECC_FOR_4BIT_MLC_NANDFLASH;
			nDevInfo->EccDataSize = 8;
		}
		else if ( nDevInfo->Feature.MediaType & A_MLC )
		{
			nDevInfo->EccType = TYPE_ECC_FOR_4BIT_MLC_NANDFLASH;
			nDevInfo->EccDataSize = 8;
		}
		#if 0	//TODO: TCC93X 6Bit ECC
		else if ( nDevInfo->Feature.MediaType & A_MLC_6BIT )
		{
			nDevInfo->EccType = TYPE_ECC_FOR_8BIT_MLC_NANDFLASH;
			nDevInfo->EccDataSize = 20;
			nDevInfo->Feature.SpareSize = 192;
		}
		#endif
		else if ( nDevInfo->Feature.MediaType & A_MLC_8BIT )
		{
			nDevInfo->EccType = TYPE_ECC_FOR_8BIT_MLC_NANDFLASH;
			nDevInfo->EccDataSize = 20;
			nDevInfo->Feature.SpareSize = 200;
		}
		else if ( nDevInfo->Feature.MediaType & A_MLC_12BIT )
		{
			nDevInfo->EccType = TYPE_ECC_FOR_12BIT_MLC_NANDFLASH;
			nDevInfo->EccDataSize = 20;
			nDevInfo->Feature.SpareSize = 200;
		}
		else if ( nDevInfo->Feature.MediaType & A_MLC_16BIT )
		{
			nDevInfo->EccType = TYPE_ECC_FOR_16BIT_MLC_NANDFLASH;
			nDevInfo->EccDataSize = 26;
			nDevInfo->Feature.SpareSize = 436;
		}
		else if ( nDevInfo->Feature.MediaType & A_MLC_24BIT )
		{
			#if defined(TCC89XX) || defined(TCC92XX)
			nDevInfo->Feature.MediaType &= ~A_MLC_24BIT;
			nDevInfo->Feature.MediaType |= A_MLC_16BIT;
			nDevInfo->EccType = TYPE_ECC_FOR_16BIT_MLC_NANDFLASH;
			nDevInfo->EccDataSize = 26;
			nDevInfo->Feature.SpareSize = 436;
			#elif defined(TCC93XX) || defined(TCC88XX)
			nDevInfo->EccType = TYPE_ECC_FOR_24BIT_MLC_NANDFLASH;
			nDevInfo->EccDataSize = 42;
			#endif 
		}

		if ( nDevInfo->Feature.MediaType & A_MLC_24BIT )
			nDevInfo->BytesPerSector = 1024;
		else
			nDevInfo->BytesPerSector = 512;
		
		if ( nDevInfo->Feature.MediaType & A_08BIT )
		{
			if ( gMTDNAND_PORT_STATUS & TCC_MTD_IO_PORT_DATA_WITDH_16BIT )
			{
				/* Check if compositin of NAND is parallel or serial */				
				TCC_MTD_IO_locResetForReadID( nChipNo, TCC_MTD_IO_PARALLEL_COMBINATION_MODE );
				TCC_MTD_IO_locReadID( nChipNo, &sDeviceCode2, TCC_MTD_IO_PARALLEL_COMBINATION_MODE );

				if ( ((sDeviceCode2.Code[0] & 0xFF) == ((sDeviceCode2.Code[0] >> 8) & 0xFF)) &&
					 ((sDeviceCode2.Code[1] & 0xFF) == ((sDeviceCode2.Code[1] >> 8) & 0xFF)) &&
					 ((sDeviceCode2.Code[2] & 0xFF) == ((sDeviceCode2.Code[2] >> 8) & 0xFF)) &&
					 ((sDeviceCode2.Code[3] & 0xFF) == ((sDeviceCode2.Code[3] >> 8) & 0xFF)) &&
					 ((sDeviceCode2.Code[4] & 0xFF) == ((sDeviceCode2.Code[4] >> 8) & 0xFF)) )
				{
					gMTDNAND_PORT_STATUS |= TCC_MTD_IO_PORT_DATA_WITDH_16BIT;
					nDevInfo->Feature.MediaType |= A_PARALLEL;
					nDevInfo->Feature.MediaType |= A_DATA_WITDH_16BIT;				
					nDevInfo->Feature.PageSize 	*= 2;
					nDevInfo->Feature.SpareSize *= 2;
					nDevInfo->CmdMask = 0xFFFF;

				}
				else
				{
					gMTDNAND_PORT_STATUS &= ~TCC_MTD_IO_PORT_DATA_WITDH_16BIT;
					nDevInfo->CmdMask = 0x00FF;
				}				
			}
			else
			{
				nDevInfo->CmdMask = 0x00FF;
			}			
		}
		else
		{
			//IO_CKC_EnableBUS( IO_CKC_BUS_NFC );
			// 16Bit NAND Mask Enable
			BITSET( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_MASK_EN );
			nDevInfo->Feature.MediaType |= A_DATA_WITDH_16BIT;
			nDevInfo->CmdMask = 0x00FF;			
		}
		
		/* Get Total Partial Page [512+16Bytes] */
		nDevInfo->PPages = ( nDevInfo->Feature.PageSize / 512 );
		nDevInfo->UsablePPages = ( nDevInfo->Feature.PageSize / nDevInfo->BytesPerSector );
		
		/* Get Shift Factors of PBpV, PpB, PageSize, SpareSize, PPages */
		res = (TCC_MTD_IO_ERROR)SUCCESS;
		res |= TCC_MTD_IO_locGetShiftValueForFastMultiPly( nDevInfo->Feature.PBpV, &nDevInfo->ShiftPBpV );
		res |= TCC_MTD_IO_locGetShiftValueForFastMultiPly( nDevInfo->Feature.PpB, &nDevInfo->ShiftPpB );
		res |= TCC_MTD_IO_locGetShiftValueForFastMultiPly( nDevInfo->Feature.PageSize, &nDevInfo->ShiftPageSize );
		res |= TCC_MTD_IO_locGetShiftValueForFastMultiPly( nDevInfo->PPages, &nDevInfo->ShiftPPages );
		res |= TCC_MTD_IO_locGetShiftValueForFastMultiPly( nDevInfo->UsablePPages, &nDevInfo->ShiftUsablePPages );
		res |= TCC_MTD_IO_locGetShiftValueForFastMultiPly( nDevInfo->BytesPerSector, &nDevInfo->ShiftBytesPerSector );	
		nDevInfo->EccWholeDataSize =  ( nDevInfo->EccDataSize << nDevInfo->ShiftUsablePPages );
		if ( res != SUCCESS )
			return res;

		gMTDNAND_PORT_STATUS |= ( 1 << nChipNo );
	}
	//=====================================================================
	// Not Found
	//=====================================================================
	else
	{
		gMTDNAND_PORT_STATUS &= ~( 1 << nChipNo );
		return ERR_TCC_MTD_IO_FAILED_GET_DEVICE_INFO;
	}

	//=========================================================================
	//				|Page Size	(s:serial, p:parallel)
	//-------------------------------------------------------------------------
	// NAND Feature	|512 |	2048	|	4096	|	8192		|	16384
	// MTD NAND		|512 |	2048(s) |	4096(s)	|	4096*2 (p)	|	4096*2 (p)
	//=========================================================================

	// Remap 4k Page NAND to 2K Page NAND ID
	if (( nDevInfo->Feature.MediaType & A_16BIT ) || 
		( nDevInfo->Feature.PBpV == 16384 ) || 
		( nDevInfo->Feature.PpB > 128 ) || 
		( nDevInfo->Feature.PageSize > 2048) )
	{
		gDummyDevID[0] = 0xEC;

		if ( ( nDevInfo->Feature.PpB == 256 ) || ( nDevInfo->Feature.PageSize == 16384 ) )
		{	
			if( nDevInfo->Feature.PageSize == 4096 ) 
			{
				gDummyDevID[1] = 0xD7;
				gDummyDevID[2] = 0xD5;
				gDummyDevID[3] = 0x29;
			}
			else
			{
				gDummyDevID[1] = 0xD7;
				gDummyDevID[2] = 0x94;
				gDummyDevID[3] = 0x72;
			}
			
		}
		else if ( nDevInfo->Feature.PBpV == 2048 )
		{
			if( nDevInfo->Feature.PageSize == 8192 )
			{
				// Page: 4K, PBpV: 8192
				gDummyDevID[1] = 0xD5;
				gDummyDevID[2] = 0x14;
				gDummyDevID[3] = 0xB6;
			}
			else
			{
				gDummyDevID[1] = 0xDC;
				gDummyDevID[2] = 0x14;
				gDummyDevID[3] = 0x25;	
			}
		}
		else if ( nDevInfo->Feature.PBpV == 4096 )
		{
			if ( nDevInfo->Feature.PageSize == 2048 )
			{
				// Page: 2K, PBpV: 4096
				if ( nDevInfo->Feature.PpB == 64 )
				{
					gDummyDevID[1] = 0xDC;
					gDummyDevID[2] = 0x10;
					gDummyDevID[3] = 0x95;
				}
				else if ( nDevInfo->Feature.PpB == 128 )
				{
					gDummyDevID[1] = 0xD3;
					gDummyDevID[2] = 0x55;
					gDummyDevID[3] = 0x25;
				}
			}
			else if( ( nDevInfo->Feature.PageSize == 4096 ) && ( nDevInfo->Feature.PpB == 64 ) )
			{
				gDummyDevID[1] = 0xD3;
				gDummyDevID[2] = 0x10;
				gDummyDevID[3] = 0xA6;
			}
			else
			{
				// Page: 4K, PBpV: 4096
				gDummyDevID[1] = 0xD5;
				gDummyDevID[2] = 0x14;
				gDummyDevID[3] = 0xB6;
			}
		}
		else if ( ( nDevInfo->Feature.PBpV == 8192 ) || ( nDevInfo->Feature.PBpV == 16384 ) )
		{
			if ( nDevInfo->Feature.PageSize == 2048 )
			{
				// Page: 2K, PBpV: 8192
				gDummyDevID[1] = 0xD5;
				gDummyDevID[2] = 0x55;
				gDummyDevID[3] = 0x25;
			}
			else
			{
				// Page: 4K, PBpV: 8192
				gDummyDevID[1] = 0xD5;
				gDummyDevID[2] = 0x14;
				gDummyDevID[3] = 0xB6;
			}
		}
		else
			return ERR_TCC_MTD_IO_FAILED_GET_DEVICE_INFO;

		nDevInfo->EccWholeDataSize =  ( nDevInfo->EccDataSize << nDevInfo->ShiftPPages );
	}

	nDevInfo->IoStatus	= TCC_MTD_IO_STATUS_ENABLE;
	nDevInfo->ChipNo	= nChipNo;

//	TCC_MTD_IO_locSetCycle( nDevInfo );

	return SUCCESS;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      TCC_MTD_IO_ERROR TCC_MTD_IO_locReadSpare( TCC_MTD_IO_DEVINFO *nDevInfo, U32 nPageAddr, U8 *nSpareBuffer );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDevInfo	= 
*			nPageAddr	= 
*			nSpareBuffer	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
TCC_MTD_IO_ERROR TCC_MTD_IO_locReadSpare( TCC_MTD_IO_DEVINFO *nDevInfo, U32 nPageAddr, U8 *nSpareBuffer )
{
	unsigned int		RowAddr, ColumnAddr;
	TCC_MTD_IO_ERROR	res;

	//=============================================	
	// Check Device and Parameter
	//=============================================	
	if ( !( nDevInfo->IoStatus & TCC_MTD_IO_STATUS_ENABLE ))
		return ERR_TCC_MTD_IO_NOT_READY_DEVICE_IO;

	//=============================================
	// PreProcess
	// Set Setup and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP LOW
	//=============================================	
	//NAND_IO_PreProcess();
	TCC_MTD_IO_locSetCommCycleTime();
	//TCC_MTD_IO_locEnableChipSelect(0);

	if( nDevInfo->ExtInterleaveUsable == TRUE )
		TCC_MTD_IO_locClearInterleaveStatus( nDevInfo );
	else
		TCC_MTD_IO_locEnableChipSelect( 0 );	
	
	TCC_MTD_IO_locEnableWriteProtect();
	TCC_MTD_IO_locCheckForExtendBlockAccess( nDevInfo, &nPageAddr );

	//=============================================
	// Read Data
	//=============================================	
	/* Generate Row and Column Address */
	if( nPageAddr >> nDevInfo->ShiftPpB )
		res = TCC_MTD_IO_locGenerateRowColAddrForRead( nPageAddr, ( ( nDevInfo->UsablePPages - 1 ) << nDevInfo->ShiftBytesPerSector ),  &RowAddr, &ColumnAddr, nDevInfo );
	else
		res = TCC_MTD_IO_locGenerateRowColAddrForRead( nPageAddr, nDevInfo->Feature.PageSize,  &RowAddr, &ColumnAddr, nDevInfo );
	
	if ( res != SUCCESS )
		goto ErrorReadSpare;

	/* Write Row and Column Address	*/
	TCC_MTD_IO_locWriteRowColAddr( RowAddr, ColumnAddr );

	/* Command READ2 [ 0x30 ] for Advance NandFlash */
	if ( nDevInfo->Feature.MediaType & A_BIG )
		pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0x3030;
	
	/* Wait until it is ready */
	TCC_MTD_IO_locWaitBusy();
	
	/* Read Spare data from NANDFLASH */
	if( nPageAddr >> nDevInfo->ShiftPpB )
		res = TCC_MTD_IO_locReadSpareData( 	nDevInfo, 
										 	nSpareBuffer, 
										 	PAGE_ECC_ON );
	else
		res = TCC_MTD_IO_locGoldenReadSpareData( nDevInfo, 
												 nSpareBuffer, 
												 PAGE_ECC_ON );

	if ( res != SUCCESS )
		goto ErrorReadSpare;
							
ErrorReadSpare:
	//=============================================
	// Disable Chip Select
	// PostProcess
	//=============================================	
	TCC_MTD_IO_locDisableChipSelect();
	//NAND_IO_PostProcess();
	
	if ( res != SUCCESS )
		return res;

   	return (TCC_MTD_IO_ERROR)SUCCESS;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      TCC_MTD_IO_ERROR TCC_MTD_IO_locReadPage( TCC_MTD_IO_DEVINFO *nDevInfo, U32 nPageAddr,
*      										 U16 nStartPPage, U16 nReadPPSize,
*      										 U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDevInfo	= 
*			nEccOnOff	= 
*			nPageAddr	= 
*			nPageBuffer	= 
*			nReadPPSize	= 
*			nSpareBuffer	= 
*			nStartPPage	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
TCC_MTD_IO_ERROR TCC_MTD_IO_locReadPage( TCC_MTD_IO_DEVINFO *nDevInfo, U32 nPageAddr,
										 U16 nStartPPage, U16 nReadPPSize,
										 U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff )
{
	unsigned short int	nFlagOfAlign = DISABLE;
	unsigned short int	rStartPPage, rReadPPSize;
	unsigned char		*cPageBuffer = 0, *cSpareBuffer = 0;
	unsigned int		RowAddr, ColumnAddr;
	unsigned int		nSpareOnOff;
	TCC_MTD_IO_ERROR	res;

	//=============================================	
	// Check Device and Parameter
	//=============================================	
	if ( !( nDevInfo->IoStatus & TCC_MTD_IO_STATUS_ENABLE ) )
		return ERR_TCC_MTD_IO_NOT_READY_DEVICE_IO;

	TCC_MTD_IO_LineUpToBytesPerSector( nDevInfo, nStartPPage, nReadPPSize, &rStartPPage, &rReadPPSize, &nFlagOfAlign );		
	if ( ( rStartPPage + rReadPPSize ) > nDevInfo->UsablePPages )
		return ERR_TCC_MTD_IO_WRONG_PARAMETER;
	TCC_MTD_IO_LineUpToBufferAlignement( nDevInfo, nFlagOfAlign, nStartPPage, nReadPPSize, nPageBuffer, nSpareBuffer, &cPageBuffer, &cSpareBuffer, TCC_MTD_IO_READ_MODE );
	
	//=============================================
	// PreProcess
	// Set Setup and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP LOW
	//=============================================	
	//NAND_IO_PreProcess();
	TCC_MTD_IO_locSetCommCycleTime();
	//TCC_MTD_IO_locEnableChipSelect(0);

	if( gDevInfo->ExtInterleaveUsable ==  TRUE )
		TCC_MTD_IO_locClearInterleaveStatus( nDevInfo );
	else
		TCC_MTD_IO_locEnableChipSelect( 0 );
	
	TCC_MTD_IO_locEnableWriteProtect();

	TCC_MTD_IO_locCheckForExtendBlockAccess( nDevInfo, &nPageAddr );
	
	//=============================================
	// Read Data
	//=============================================	
	/* Generate Row and Column Address */
	if ( ( nDevInfo->Feature.MediaType  & A_BIG ) || (( nDevInfo->Feature.MediaType  & A_SMALL ) && ( nDevInfo->Feature.MediaType  & A_PARALLEL )))
	{
		if( nPageAddr >> nDevInfo->ShiftPpB )
		{
			res = TCC_MTD_IO_locGenerateRowColAddrForRead( nPageAddr, ( ( nDevInfo->UsablePPages - 1 ) << nDevInfo->ShiftBytesPerSector ), &RowAddr, &ColumnAddr, nDevInfo );
		}
		else
			res = TCC_MTD_IO_locGenerateRowColAddrForRead( nPageAddr, nDevInfo->Feature.PageSize, &RowAddr, &ColumnAddr, nDevInfo );
	}		
	else
		res = TCC_MTD_IO_locGenerateRowColAddrForRead( nPageAddr, ( rStartPPage << nDevInfo->ShiftBytesPerSector ), &RowAddr, &ColumnAddr, nDevInfo );
	
	if ( res != SUCCESS )
		goto ErrorReadPage;

	/* Write Row and Column Address	*/
	TCC_MTD_IO_locWriteRowColAddr( RowAddr, ColumnAddr );

	/* Command READ2 [ 0x30 ] for Advance NandFlash */
	if ( nDevInfo->Feature.MediaType & A_BIG )
		pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0x3030;
	
	/* Wait until it is ready */
	TCC_MTD_IO_locWaitBusy();
	/* Change Cycle */
	TCC_MTD_IO_locSetReadCycleTime();

	/* Read Page Size data from NANDFLASH */
	nSpareOnOff = TNFTL_READ_SPARE_ON;

	if ( nDevInfo->Feature.MediaType & A_BIG )
	{
		
		if( nPageAddr >> nDevInfo->ShiftPpB )
			res = TCC_MTD_IO_locReadSpareData( nDevInfo, cSpareBuffer, PAGE_ECC_ON );
		else
			res = TCC_MTD_IO_locGoldenReadSpareData( nDevInfo, cSpareBuffer, PAGE_ECC_ON );

		if ( res != SUCCESS )
			goto ErrorReadPage;

		/* Change Cycle */
		TCC_MTD_IO_locSetCommCycleTime();

		/* Command Random Data Output [ 0x05 ] for Advance NandFlash */
		pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0x0505;
		
		ColumnAddr = ( rStartPPage << nDevInfo->ShiftBytesPerSector );
		ColumnAddr	= ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT ) ? ( ColumnAddr >> 1 ) : ColumnAddr;
		TCC_MTD_IO_locWriteColAddr( ColumnAddr );
		
		/* Command Random Data Output [ 0xE0 ] for Advance NandFlash */
		pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0xE0E0;

		/* Change Cycle */
		TCC_MTD_IO_locSetReadCycleTime();
	}
	else if (( nDevInfo->Feature.MediaType  & A_SMALL ) && ( nDevInfo->Feature.MediaType  & A_PARALLEL ))
	{
		res = TCC_MTD_IO_locReadSpareData( nDevInfo, cSpareBuffer, PAGE_ECC_ON );
		if ( res != SUCCESS )
			goto ErrorReadPage;

		/* Change Cycle */
		TCC_MTD_IO_locSetCommCycleTime();

		res = TCC_MTD_IO_locGenerateRowColAddrForRead( nPageAddr, ( rStartPPage << nDevInfo->ShiftBytesPerSector ), &RowAddr, &ColumnAddr, nDevInfo );
		if ( res != SUCCESS )
			goto ErrorReadPage;

		/* Write Row and Column Address	*/
		TCC_MTD_IO_locWriteRowColAddr( RowAddr, ColumnAddr );

		/* Wait until it is ready */
		TCC_MTD_IO_locWaitBusy();

		/* Change Cycle */
		TCC_MTD_IO_locSetReadCycleTime();
	}

	#if defined(TCC_MTD_DMA_ACCESS) && !defined(__USE_MTD_NAND_ISR__)
	if( nPageAddr >> nDevInfo->ShiftPpB )
		res = TCC_MTD_IO_locRead512DataDoubleBuf( nDevInfo,
												  rStartPPage,
												  rReadPPSize,
												  cPageBuffer,
												  cSpareBuffer,
												  nEccOnOff,
												  TNFTL_READ_SPARE_ON );
	else
		res = TCC_MTD_IO_locRead512DataGoldenPage( nDevInfo,
												   rStartPPage,
												   rReadPPSize,
												   cPageBuffer,
												   cSpareBuffer,
												   nEccOnOff,
												   TNFTL_READ_SPARE_ON );
		
	#else	
	if( nPageAddr >> nDevInfo->ShiftPpB )
		res = TCC_MTD_IO_locRead512Data( nDevInfo,
									     rStartPPage,
									     rReadPPSize,
									     cPageBuffer,
									     cSpareBuffer,
									     nEccOnOff,
									     TNFTL_READ_SPARE_ON );
	else
		res = TCC_MTD_IO_locRead512DataGoldenPage( nDevInfo,
												   rStartPPage,
												   rReadPPSize,
												   cPageBuffer,
												   cSpareBuffer,
												   nEccOnOff,
												   TNFTL_READ_SPARE_ON );
	#endif

	if( res == ERR_TCC_MTD_IO_ATTEMPT_REREAD )
	{
		printk("[%s] TCC_MTD_IO Read Page Fail !!!!!! \n", __FUNCTION__ );

		emergency_restart();	
	}
	/* Change Cycle */
	TCC_MTD_IO_locSetCommCycleTime();
	TCC_MTD_IO_LineUpBufferData( nFlagOfAlign, nStartPPage, nReadPPSize, cPageBuffer, cSpareBuffer, nPageBuffer, nSpareBuffer );						
ErrorReadPage:
	//=============================================
	// Disable Chip Select
	// PostProcess
	//=============================================	
	TCC_MTD_IO_locDisableChipSelect();
	//NAND_IO_PostProcess();

	if ( res != SUCCESS )
		return res;

   	return (TCC_MTD_IO_ERROR)SUCCESS;
}
EXPORT_SYMBOL(TCC_MTD_IO_locReadPage);

/**************************************************************************
*  FUNCTION NAME : 
*  
*      TCC_MTD_IO_ERROR TCC_MTD_IO_locReadPage( TCC_MTD_IO_DEVINFO *nDevInfo, U32 nPageAddr,
*      										 U16 nStartPPage, U16 nReadPPSize,
*      										 U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDevInfo	= 
*			nEccOnOff	= 
*			nPageAddr	= 
*			nPageBuffer	= 
*			nReadPPSize	= 
*			nSpareBuffer	= 
*			nStartPPage	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
TCC_MTD_IO_ERROR TCC_MTD_IO_locReadPageForSignature( TCC_MTD_IO_DEVINFO *nDevInfo, U32 nPageAddr,
										 U16 nStartPPage, U16 nReadPPSize,
										 U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff )
{
	unsigned int			RowAddr, ColumnAddr;
	unsigned int			nSpareOnOff;
	TCC_MTD_IO_ERROR		res;
	
	//=============================================	
	// Check Device and Parameter
	//=============================================	
	if ( !( nDevInfo->IoStatus & TCC_MTD_IO_STATUS_ENABLE ) )
		return ERR_TCC_MTD_IO_NOT_READY_DEVICE_IO;

	if ( ( nStartPPage + nReadPPSize ) > ( nDevInfo->Feature.PageSize >> 9 ) )
		return ERR_TCC_MTD_IO_WRONG_PARAMETER;
	
	//=============================================
	// PreProcess
	// Set Setup and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP LOW
	//=============================================	
	//NAND_IO_PreProcess();
	TCC_MTD_IO_locSetCommCycleTime();
	TCC_MTD_IO_locEnableChipSelect( nDevInfo->ChipNo );
	TCC_MTD_IO_locEnableWriteProtect();

	TCC_MTD_IO_locCheckForExtendBlockAccess( nDevInfo, &nPageAddr );

	//=============================================
	// Read Data
	//=============================================	
	/* Generate Row and Column Address */
	res = TCC_MTD_IO_locGenerateRowColAddrForRead( nPageAddr, 0, &RowAddr, &ColumnAddr, nDevInfo );
	if ( res != SUCCESS )
		goto ErrorReadPage;

	/* Write Row and Column Address	*/
	TCC_MTD_IO_locWriteRowColAddr( RowAddr, ColumnAddr );

	/* Command READ2 [ 0x30 ] for Advance NandFlash */
	if ( nDevInfo->Feature.MediaType & A_BIG )
		pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0x3030;

	/* Wait until it is ready */
	TCC_MTD_IO_locWaitBusy();
	
	/* Change Cycle */
	TCC_MTD_IO_locSetReadCycleTime();

	/* Read Page Size data from NANDFLASH */
	nSpareOnOff = TNFTL_READ_SPARE_ON;

	res = TCC_MTD_IO_locRead528Data( nDevInfo,
								   nStartPPage,
								   nReadPPSize,
								   nPageBuffer,
								   nSpareBuffer,
								   nEccOnOff,
								   TNFTL_READ_SPARE_ON );		
	/* Change Cycle */
	TCC_MTD_IO_locSetCommCycleTime();
							
ErrorReadPage:
	//=============================================
	// Disable Chip Select
	// PostProcess
	//=============================================	
	TCC_MTD_IO_locDisableChipSelect();
	//NAND_IO_PostProcess();

	if ( res != SUCCESS )
		return res;

   	return (TCC_MTD_IO_ERROR)SUCCESS;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void NAND_IO_SetupDMADoubleBuf(  int nMode, int nDMACh );
*  
*  DESCRIPTION : 
*  
*  INPUT:
*			nDMACh	= 
*			nMode	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_SetupDMADoubleBuf(  int nMode, int nDMACh, int nDSize )
{
	unsigned int	*pDMA_PhyBuffer;
	unsigned int	*pDMA_WorkBuffer;
	unsigned int	nSourceAddr, nDestAddr;
	unsigned		uCHCTRL;
	unsigned int	uTmp;
	unsigned int	uSrcInc, uSrcMask;
	unsigned int	uDstInc, uDstMask;

	if ( nMode == NAND_IO_DMA_WRITE )
	{
		uSrcInc 	= 4;
		uSrcMask 	= 0;
		uDstInc		= 0;
		uDstMask 	= 0;
	}
	else
	{
		uSrcInc 	= 0;
		uSrcMask 	= 0;
		uDstInc		= 4;
		uDstMask 	= 0;
	}
	
	if ( nDMACh & 1 )
	{
		pDMA_PhyBuffer 	= gpMTD_DMA_PhyBuffer0;			// Working Address
		pDMA_WorkBuffer = gpMTD_DMA_WorkBuffer0;		// Physical Address
	}
	else
	{
		pDMA_PhyBuffer 	= gpMTD_DMA_PhyBuffer1;			// Working Address
		pDMA_WorkBuffer = gpMTD_DMA_WorkBuffer1;		// Physical Address
	}
	
	if ( nMode == NAND_IO_DMA_WRITE )
	{
		// pSRC: Buffer Address
		// pDST: NFC_LDATA
		#if defined(TCC89XX) || defined(TCC92XX)	//HwNFC_CTRL_DEN_EN: NANDFLASH DMA Request Enable
		BITCSET(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_8 | HwNFC_CTRL_DEN_EN | HwNFC_CTRL_PSIZE_512 );
		#elif defined(TCC93XX) || defined(TCC88XX)
		BITCSET(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_8 | HwNFC_CTRL_PSIZE_512 );
		#endif

		//pSRC --> pDMA_WorkBuffer
		//memcpy( pDMA_WorkBuffer, pSRC, 512 );

		// Target Physical Address- for DMA H/W Control Set
		nSourceAddr	= (unsigned int)pDMA_PhyBuffer;
		#if defined(_WINCE_) || defined(_LINUX_)
		nDestAddr 	= (unsigned int)&NAND_IO_HwLDATA_PA;
		#else
		nDestAddr 	= (unsigned int)&pMTD_NFC->NFC_LDATA;
		#endif
		
		//============================================================
		// DMA Control Register Set
		//============================================================
		uCHCTRL =	
	//				HwCHCTRL_SYNC_ON		|
	//				HwCHCTRL_HRD_W			|
					HwCHCTRL_BST_BURST		|
					HwCHCTRL_TYPE_SINGL		|
				   	HwCHCTRL_HRD_WR			|
	//				HwCHCTRL_BST_BURST		|
					HwCHCTRL_BSIZE_8		|
					HwCHCTRL_WSIZE_32		|
					HwCHCTRL_FLAG			|
					HwCHCTRL_EN_ON			|
					0;

	}
	else	// NAND_IO_DMA_READ
	{			
		#if defined(TCC89XX) || defined(TCC92XX)	//HwNFC_CTRL_DEN_EN: NANDFLASH DMA Request Enable
		BITCSET(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_8 | HwNFC_CTRL_DEN_EN | HwNFC_CTRL_PSIZE_512 );
		#elif defined(TCC93XX) || defined(TCC88XX)
		BITCSET(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_8 | HwNFC_CTRL_PSIZE_512 );
		#endif

		#if defined(_WINCE_) || defined(_LINUX_)
		nSourceAddr = (unsigned int)&NAND_IO_HwLDATA_PA;				// NFC_LDATA Physical Address: ex_TCC89,92XX: 0XF050b0020
		#else
		nSourceAddr = (unsigned int)&pMTD_NFC->NFC_LDATA;
		#endif
		nDestAddr 	= (unsigned int)pDMA_PhyBuffer;

		//============================================================
		// DMA Control Register Set
		//============================================================
		uCHCTRL =	
	//				HwCHCTRL_SYNC_ON		|
	//				HwCHCTRL_HRD_W			|
					HwCHCTRL_BST_BURST		|
					HwCHCTRL_TYPE_SINGL		|
				   	HwCHCTRL_HRD_RD			|
	//				HwCHCTRL_BST_BURST		|
					HwCHCTRL_BSIZE_8		|
					HwCHCTRL_WSIZE_32		|
					HwCHCTRL_FLAG			|
					HwCHCTRL_EN_ON			|
					0;
	}

	//============================================================
	// Set Source Address & Source Parameter (mask + increment)
	//============================================================
	pNAND_DMA->ST_SADR 	= nSourceAddr;
	#if defined(_WINCE_) || defined(_LINUX_)
	pNAND_DMA->SPARAM[0] = (uSrcInc | (uSrcMask << 4));
	#else
	pNAND_DMA->SPARAM	 = (uSrcInc | (uSrcMask << 4));
	#endif
	//============================================================
	// Set Dest Address & Dest Parameter (mask + increment)
	//============================================================
	pNAND_DMA->ST_DADR 	= nDestAddr;  
	#if defined(_WINCE_) || defined(_LINUX_)
	pNAND_DMA->DPARAM[0] = (uDstInc | (uDstMask << 4));
	#else
	pNAND_DMA->DPARAM	 = (uDstInc | (uDstMask << 4));
	#endif
	//============================================================
	// Calculate byte size per 1 Hop transfer
	//============================================================
	uTmp	= (uCHCTRL & (Hw5+Hw4)) >> 4;			// calc log2(word size)
	uTmp	= uTmp + ( (uCHCTRL & (Hw7+Hw6)) >> 6);	// calc log2(word * burst size)

	//============================================================
	// Set External DMA Request Register
	//============================================================
	pNAND_DMA->EXTREQ = Hw18;		// NFC

	//============================================================
	// Set Hcount
	//============================================================
	if (uTmp)
		pNAND_DMA->HCOUNT	= (nDSize + (1 << uTmp) - 1) >> uTmp;
	else
		pNAND_DMA->HCOUNT	= nDSize;

	//============================================================
	// Set & Enable DMA
	//============================================================
	pNAND_DMA->CHCTRL		= uCHCTRL;

	//============================================================
	// Set NFC DSize & IREQ Clear
	//============================================================
	pMTD_NFC->NFC_DSIZE		= nDSize;
	#if defined(TCC89XX) || defined(TCC92XX)
	pMTD_NFC->NFC_IREQ		= 0x77;	// HwNFC_IREQ_FLAG1;
	#elif defined(TCC93XX) || defined(TCC88XX)
	if ( nMode == NAND_IO_DMA_WRITE )
	{
		BITSET(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_PRSEL_P);
		//pMTD_NFC->NFC_IRQCFG |= HwNFC_IRQCFG_PIEN;
	}
	else
	{
		BITCLR(pMTD_NFC->NFC_CTRL, HwNFC_CTRL_PRSEL_P);
		//pMTD_NFC->NFC_IRQCFG |= HwNFC_IRQCFG_RDIEN;
	}
	#endif
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locSetupECC( U16 nEccOnOff, U16 nEncDec, U16 nEccType, U16 nAccessType, U32 EccBaseAddr );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			EccBaseAddr	= 
*			nAccessType	= 
*			nEccOnOff	= 
*			nEccType	= 
*			nEncDec	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locSetupECC( U16 nEccOnOff, U16 nEncDec, U16 nEccType, U16 nAccessType, U32 EccBaseAddr )
{
	if ( nEccOnOff == ECC_OFF )
	{
		//IO_CKC_EnableBUS( IO_CKC_BUS_ECC );
		#if defined(TCC89XX) || defined(TCC92XX)
		BITCLR(pIOBUSCFG_T->HRSTEN0, Hw24);
		BITSET(pIOBUSCFG_T->HRSTEN0, Hw24);
		pMTD_ECC->ECC_BASE = 0xF05B0010;	/* Base Address for ECC Calculation */
		pMTD_ECC->ECC_MASK	= 0x00000000;				/* Address mask for ECC area */
		pMTD_ECC->ECC_CTRL &= HwECC_CTRL_EN_DIS;
		#elif defined(TCC93XX) || defined(TCC88XX)
		pMTD_NFC->NFC_CTRL		&= ~HwNFC_CTRL_ECCON_EN;	
		pMTD_ECC->ECC_CTRL 		&= HwECC_CTRL_EN_DIS;			
		BITCLR(pMTD_ECC->ECC_CTRL, HwECC_CTRL_CORPASS_EN );
		#endif

	}
	else if ( nEccOnOff == ECC_ON )
	{

		#if defined(TCC93XX) || defined(TCC88XX)
		pMTD_NFC->NFC_CTRL	|= HwNFC_CTRL_ECCON_EN;					// NFC ECC Encode/Decode Enable
		BITCSET( pMTD_ECC->ECC_CTRL, 0x07FF003F, 0x00000000 ); 		//ECC CTRL: ECC_DSIZE & CP/MODE & ECC_EN[3:0]	
		#endif

		if ( nEncDec == ECC_DECODE )
		{
			//==========================================================
			//
			// ECC Decode Setup
			//
			//==========================================================
			if ( nAccessType == NAND_MCU_ACCESS )
			{
				#if defined(TCC89XX) || defined(TCC92XX)
				pIOBUSCFG_T->STORAGE = HwIOBUSCFG_STORAGE_NFC;
				pMTD_ECC->ECC_BASE	= (0x000FFFFF & EccBaseAddr);
				pMTD_ECC->ECC_MASK	= 0x00000000;				/* Address mask for ECC area */
				#endif
			}
			else if ( nAccessType == NAND_DMA_ACCESS )
			{
				#if defined(TCC89XX) || defined(TCC92XX)
				pIOBUSCFG_T->STORAGE = HwIOBUSCFG_STORAGE_NFC;
				pMTD_ECC->ECC_BASE	= EccBaseAddr;
				pMTD_ECC->ECC_MASK	= 0x00000000;				/* Address mask for ECC area */
				#endif
			}			

			if ( nEccType == SLC_ECC_TYPE )
			{
				#if defined(TCC89XX) || defined(TCC92XX)
				pMTD_ECC->ECC_CTRL	|= 	HwECC_CTRL_EN_SLCDE;
				#elif defined(TCC93XX) || defined(TCC88XX)
				pMTD_ECC->ECC_CTRL	|=	HwECC_CTRL_EN_SLCDE;
				#endif
			}
			else if (nEccType == MLC_ECC_4BIT_TYPE )
			{
				#if defined(TCC89XX) || defined(TCC92XX)
				pMTD_ECC->ECC_CTRL	|= 	HwECC_CTRL_IEN_MECC4_EN;				
				pMTD_ECC->ECC_CTRL	|= 	HwECC_CTRL_EN_MCL4DE;
				#elif defined(TCC93XX) || defined(TCC88XX)
				//pMTD_NFC->NFC_IRQCFG	|=  HwNFC_IRQ_MEDIRQ;
				pMTD_ECC->ECC_CTRL		|= 	HwECC_CTRL_EN_MCL4DE;
				#endif 
			}
			#if 0 //TCC93xx TODO 6Bit ECC 
			else if (nEccType == MLC_ECC_6BIT_TYPE )
			{
				#if defined(TCC89XX) || defined(TCC92XX)
				//NOT USE ECC I/P
				#elif defined(TCC93XX) || defined(TCC88XX)
				pMTD_NFC->NFC_IRQCFG	|=  HwNFC_IRQ_MEDIRQ;
				pMTD_ECC->ECC_CTRL		|= 	HwECC_CTRL_EN_MCL6DE;
				#endif
			}
			#endif 
			else if (nEccType == MLC_ECC_8BIT_TYPE )
			{
				#if defined(TCC89XX) || defined(TCC92XX)
				pMTD_ECC->ECC_CTRL	|= HwECC_CTRL_IEN_MECC8_EN;
				pMTD_ECC->ECC_CTRL	|= HwECC_CTRL_EN_MCL8DE;
				#elif defined(TCC93XX) || defined(TCC88XX)
				//NOT USE ECC I/P
				#endif
			}
			else if (nEccType == MLC_ECC_12BIT_TYPE )
			{
				#if defined(TCC89XX) || defined(TCC92XX)
				pMTD_ECC->ECC_CTRL	|= HwECC_CTRL_IEN_MECC12_EN;
				pMTD_ECC->ECC_CTRL	|= 	HwECC_CTRL_EN_MCL12DE;
				#elif defined(TCC93XX) || defined(TCC88XX)
				//pMTD_NFC->NFC_IRQCFG	|=  HwNFC_IRQ_MEDIRQ;
				pMTD_ECC->ECC_CTRL		|= 	HwECC_CTRL_EN_MCL12DE;
				#endif 
			}
			else if (nEccType == MLC_ECC_16BIT_TYPE )
			{
				#if defined(TCC89XX) || defined(TCC92XX)
				pMTD_ECC->ECC_CTRL	|= HwECC_CTRL_IEN_MECC16_EN;
				pMTD_ECC->ECC_CTRL	|= 	HwECC_CTRL_EN_MCL16DE;
				#elif defined(TCC93XX) || defined(TCC88XX)
				//pMTD_NFC->NFC_IRQCFG	|=  HwNFC_IRQ_MEDIRQ;
				pMTD_ECC->ECC_CTRL		|= 	HwECC_CTRL_EN_MCL16DE;
				#endif 
			}
			else if (nEccType == MLC_ECC_24BIT_TYPE )
			{
				#if defined(TCC93XX) || defined(TCC88XX)
				//pMTD_NFC->NFC_IRQCFG	|=  HwNFC_IRQ_MEDIRQ;
				pMTD_ECC->ECC_CTRL		|= 	HwECC_CTRL_EN_MCL24DE;
				#endif	
			}						
		}
		else if ( nEncDec == ECC_ENCODE )
		{			
			pMTD_ECC->ECC_CLEAR = 0x00000000;				/* Address mask for ECC area */
			//==========================================================
			//
			// ECC Encode Setup
			//
			//==========================================================
			if ( nAccessType == NAND_MCU_ACCESS )
			{
				#if defined(TCC89XX) || defined(TCC92XX)
				pIOBUSCFG_T->STORAGE = HwIOBUSCFG_STORAGE_NFC;
				pMTD_ECC->ECC_BASE	= (0x000FFFFF &EccBaseAddr);
				#endif
			}
			else if ( nAccessType == NAND_DMA_ACCESS )
			{
				#if defined(TCC89XX) || defined(TCC92XX)
				pIOBUSCFG_T->STORAGE = HwIOBUSCFG_STORAGE_NFC;
				pMTD_ECC->ECC_BASE	= EccBaseAddr;				
				#endif
			}

			pMTD_ECC->ECC_MASK	= 0x00000000;				/* Address mask for ECC area */	
			
			if ( nEccType == SLC_ECC_TYPE )
				pMTD_ECC->ECC_CTRL	|= 	HwECC_CTRL_EN_SLCEN;
			else if (nEccType == MLC_ECC_4BIT_TYPE )
				pMTD_ECC->ECC_CTRL	|= 	HwECC_CTRL_EN_MCL4EN;
			#if 0 //TCC93X TODO 6Bit ECC 
			else if (nEccType == MLC_ECC_6BIT_TYPE )
				pMTD_ECC->ECC_CTRL	|= 	HwECC_CTRL_EN_MCL6EN;
			#endif
			else if (nEccType == MLC_ECC_8BIT_TYPE )
			{
				#if defined(TCC89XX) || defined(TCC92XX)
				pMTD_ECC->ECC_CTRL	|= 	HwECC_CTRL_EN_MCL8EN;
				#endif
			}				
			else if (nEccType == MLC_ECC_12BIT_TYPE )
				pMTD_ECC->ECC_CTRL	|= 	HwECC_CTRL_EN_MCL12EN;
			else if (nEccType == MLC_ECC_16BIT_TYPE )
				pMTD_ECC->ECC_CTRL	|= 	HwECC_CTRL_EN_MCL16EN;
			else if (nEccType == MLC_ECC_24BIT_TYPE )
			{
				#if defined(TCC93XX) || defined(TCC88XX)
				pMTD_ECC->ECC_CTRL	|= 	HwECC_CTRL_EN_MCL24EN;
				#endif 
			}
		}
	}
	
	pMTD_ECC->ECC_CLEAR	= 0x00000000;					/* Clear ECC Block		*/
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locEncodeECC( U16 nEccType, U8* nSpareBuffer );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nEccType	= 
*			nSpareBuffer	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locEncodeECC( U16 nEccType, U8* nSpareBuffer )
{
	unsigned int		nECC_CODE;
	unsigned char		*pcECC;
	unsigned int		*pSpareDW;

	if ( nEccType == SLC_ECC_TYPE )
	{
		//==================================================================
		//	[ TNFTL V1.0 ]
		//==================================================================	
		// 520th	: P64-P8		of ECC Area-1 [ DATA 256-511 ]
		// 521th	: P1024-P128	of ECC Area-1 [ DATA 256-511 ]
		// 522th	: P4-P1			of ECC Area-1 [ DATA 256-511 ]
		// 525th	: P64-P8		of ECC Area-0 [ DATA 0-255 ]
		// 526th	: P1024-P128	of ECC Area-0 [ DATA 0-255 ]
		// 527th	: P4-P1			of ECC Area-0 [ DATA 0-255 ]
		//==================================================================
		
		pcECC = (unsigned char* )&pMTD_ECC->ECC_CODE0;

		/* Area-1 */
		nSpareBuffer[2]	= pcECC[0];		// P4-P1
		nSpareBuffer[1]	= pcECC[1];		// P1024-P128
		nSpareBuffer[0]	= pcECC[2];		// P64-P8
		
		/* Area-0 */
		nSpareBuffer[5]	= pcECC[4];		// P4-P1
		nSpareBuffer[4]	= pcECC[5];		// P1024-128
		nSpareBuffer[3]	= pcECC[6];		// P64-P8	
	}
	else
	{
		pSpareDW = (unsigned int *)nSpareBuffer;

		nECC_CODE 	= pMTD_ECC->ECC_CODE0;
		*pSpareDW	= nECC_CODE; ++pSpareDW;

		nECC_CODE 	= pMTD_ECC->ECC_CODE1;
		*pSpareDW	= nECC_CODE; ++pSpareDW;

		nECC_CODE 	= pMTD_ECC->ECC_CODE2;
		*pSpareDW	= nECC_CODE; ++pSpareDW;

		nECC_CODE 	= pMTD_ECC->ECC_CODE3;
		*pSpareDW	= nECC_CODE; ++pSpareDW;

		nECC_CODE 	= pMTD_ECC->ECC_CODE4;
		*pSpareDW	= nECC_CODE; ++pSpareDW;

		nECC_CODE 	= pMTD_ECC->ECC_CODE5;
		*pSpareDW	= nECC_CODE; ++pSpareDW;

		nECC_CODE 	= pMTD_ECC->ECC_CODE6;
		*pSpareDW	= nECC_CODE; ++pSpareDW;

		#if defined(TCC93XX) || defined(TCC88XX)
		nECC_CODE 	= pMTD_ECC->ECC_CODE7;
		*pSpareDW	= nECC_CODE; ++pSpareDW;

		nECC_CODE 	= pMTD_ECC->ECC_CODE8;
		*pSpareDW	= nECC_CODE; ++pSpareDW;

		nECC_CODE 	= pMTD_ECC->ECC_CODE9;
		*pSpareDW	= nECC_CODE; ++pSpareDW;

		nECC_CODE 	= pMTD_ECC->ECC_CODE10;
		*pSpareDW	= nECC_CODE; ++pSpareDW;
		#endif
	}
	
	return (TCC_MTD_IO_ERROR)SUCCESS;	
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locCorrectionSLC( U8* nPageBuffer, U8* nSpareBuffer );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nPageBuffer	= 
*			nSpareBuffer	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locCorrectionSLC( U8* nPageBuffer, U8* nSpareBuffer )
{
	unsigned int		uErrorStatus;
	unsigned int		uSLCECC0, uSLCECC1;
	TCC_MTD_IO_ERROR	res;

	//IO_CKC_EnableBUS( IO_CKC_BUS_ECC );
	
	//==================================================================
	//	[ TNFTL V1.0 ]
	//==================================================================	
	// 520th	: P64-P8		of ECC Area-1 [ DATA 256-511 ]
	// 521th	: P1024-P128	of ECC Area-1 [ DATA 256-511 ]
	// 522th	: P4-P1			of ECC Area-1 [ DATA 256-511 ]
	// 525th	: P64-P8		of ECC Area-0 [ DATA 0-255 ]
	// 526th	: P1024-P128	of ECC Area-0 [ DATA 0-255 ]
	// 527th	: P4-P1			of ECC Area-0 [ DATA 0-255 ]
	//==================================================================

	/* Load SLC ECC Code for Area 1 */
	uSLCECC0	= ( nSpareBuffer[0] << 16 ) +
				  ( nSpareBuffer[1] <<  8 ) +
				  ( nSpareBuffer[2] );
				  
	/* Load SLC ECC Code for Area 0 */
	uSLCECC1	= ( nSpareBuffer[3] << 16 ) +
				  ( nSpareBuffer[4] <<  8 ) +
				  ( nSpareBuffer[5] );

	res = (TCC_MTD_IO_ERROR)SUCCESS;
	
	/* Correction Area 0 */
	pMTD_ECC->ECC_CODE1 = uSLCECC1;
	uErrorStatus = pMTD_ECC->ERRNUM & 0x7;
	
	if ( uErrorStatus == HwERR_NUM_ERR1 )
	{
		nPageBuffer[pMTD_ECC->ECC_EADDR0 >> 3] ^= (1 << (pMTD_ECC->ECC_EADDR0 & 0x07));
		res = (TCC_MTD_IO_ERROR)SUCCESS;
	}	
	else if ( uErrorStatus != HwERR_NUM_NOERR )
	{
		res	= ERR_TCC_MTD_IO_FAILED_CORRECTION_SLC_ECC;
		goto ErrorCorrectionSLC;
	}	

	res = (TCC_MTD_IO_ERROR)SUCCESS;
	
	/* Correction Area 1 */
	pMTD_ECC->ECC_CODE0 = uSLCECC0;
	uErrorStatus = pMTD_ECC->ERRNUM & 0x7;
	
	if ( uErrorStatus == HwERR_NUM_ERR1 )
	{
		nPageBuffer[pMTD_ECC->ECC_EADDR0 >> 3] ^= (1 << (pMTD_ECC->ECC_EADDR0 & 0x07));
		res = (TCC_MTD_IO_ERROR)SUCCESS;
	}
	else if ( uErrorStatus != HwERR_NUM_NOERR )
	{
		res = ERR_TCC_MTD_IO_FAILED_CORRECTION_SLC_ECC;
		goto ErrorCorrectionSLC;		
	}

ErrorCorrectionSLC:

	//IO_CKC_DisableBUS( IO_CKC_BUS_ECC );
	
	return res;
}	

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locCorrectionMLC( U16 nEccType, U8* nPageBuffer, U8* nSpareBuffer, U16 nDataSize );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDataSize	= 
*			nEccType	= 
*			nPageBuffer	= 
*			nSpareBuffer	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locCorrectionMLC( U16 nEccType, U8* nPageBuffer, U8* nSpareBuffer, U16 nDataSize )
{
	unsigned int		i;
	unsigned int		uErrAddr;
	unsigned int		uErrorStatus;
	unsigned int		*pSpareDW;
	#ifdef _LINUX_
	unsigned char		nECCBuffer[44]__attribute__((aligned(8)));
	#else
	unsigned char		nECCBuffer[44];
	#endif
	TCC_MTD_IO_ECC_INFO	*pECC_Info;
	TCC_MTD_IO_DEVINFO	*nDevInfo;
	TCC_MTD_IO_ERROR	res;

	nDevInfo = &gDevInfo[0];

	if ( nEccType == MLC_ECC_4BIT_TYPE )
		pECC_Info = &gMLC_ECC_4Bit;
	#if 0 //TODO: 6Bit ECC IP  Verification  
	else if ( nEccType == MLC_ECC_6BIT_TYPE )
		pECC_Info = &gMLC_ECC_6Bit;
	#endif
	else if ( nEccType == MLC_ECC_8BIT_TYPE )
		pECC_Info = &gMLC_ECC_8Bit;
	else if ( nEccType == MLC_ECC_12BIT_TYPE )
		pECC_Info = &gMLC_ECC_12Bit;
	else if ( nEccType == MLC_ECC_14BIT_TYPE )
		pECC_Info = &gMLC_ECC_14Bit;
	else if ( nEccType == MLC_ECC_16BIT_TYPE )
		pECC_Info = &gMLC_ECC_16Bit;
	else if ( nEccType == MLC_ECC_24BIT_TYPE )
		pECC_Info = &gMLC_ECC_24Bit;
	else
		return ERR_TCC_MTD_IO_WRONG_PARAMETER;

	for ( i = 0; i < pECC_Info->EccDataSize; ++i )
		nECCBuffer[i] = nSpareBuffer[i];

	//============================================ 
	// Get Buffer Pointer
	//============================================
	if ( memcmp( nECCBuffer, gTCC_MTD_IO_TempBuffer, pECC_Info->EccDataSize ) == 0 )
	{
		if ( ( nDataSize == 512 ) || ( nDataSize == 1024 ) )
			pSpareDW	= (unsigned int *)pECC_Info->All_FF_512_ECC_Code;
		else if ( nDataSize == 12 )
			pSpareDW	= (unsigned int *)&ALL_FF_ECC_BCH_04BIT_12;
		else if ( nDataSize == 16 )
			pSpareDW	= (unsigned int *)&ALL_FF_ECC_BCH_04BIT_16;
		else
			pSpareDW	= (unsigned int *)&ALL_FF_ECC_BCH_04BIT_512;
	}
	else
	{
		pSpareDW	= (unsigned int *)nECCBuffer;
	}		

	//IO_CKC_EnableBUS( IO_CKC_BUS_ECC );
	
	pMTD_ECC->ECC_CODE0	= *pSpareDW; ++pSpareDW;
	pMTD_ECC->ECC_CODE1	= *pSpareDW; ++pSpareDW;
	pMTD_ECC->ECC_CODE2	= *pSpareDW; ++pSpareDW;
	pMTD_ECC->ECC_CODE3	= *pSpareDW; ++pSpareDW;
	pMTD_ECC->ECC_CODE4	= *pSpareDW; ++pSpareDW;
	pMTD_ECC->ECC_CODE5	= *pSpareDW; ++pSpareDW;
	pMTD_ECC->ECC_CODE6	= *pSpareDW; ++pSpareDW;
	#if defined(TCC93XX) || defined(TCC88XX)
	pMTD_ECC->ECC_CODE7  = *pSpareDW; ++pSpareDW; 
	pMTD_ECC->ECC_CODE8  = *pSpareDW; ++pSpareDW;
	pMTD_ECC->ECC_CODE9  = *pSpareDW; ++pSpareDW;
	pMTD_ECC->ECC_CODE10 = *pSpareDW; ++pSpareDW;
	#endif
	
	/* Wait MLC ECC Correction */
	#if defined(TCC89XX) || defined(TCC92XX)
	while ( !(pMTD_ECC->ECC_IREQ & pECC_Info->DecodeFlag ) );
	#elif defined(TCC93XX) || defined(TCC88XX)
	while( !( pMTD_NFC->NFC_IRQ & Hw7 ) );
	pMTD_NFC->NFC_IRQ		|= Hw7;
	pMTD_NFC->NFC_CTRL		&= ~HwNFC_CTRL_ECCON_EN; 
	#endif

	res = (TCC_MTD_IO_ERROR)SUCCESS;

	/* Correction */
	uErrorStatus = pMTD_ECC->ERRNUM & 0x1F;

	if ( uErrorStatus > pECC_Info->ErrorNum )
	{
		#ifdef NAND_IO_ECC_ERROR_LOG
		PRINTF("\n\n[TCC_MTD]ErrorNum[%02d],DataSize[%03d] - Correction Fail", uErrorStatus, nDataSize );
		#endif
		#ifdef TCC_MTD_DEBUG
		printk("\n[TCC_MTD]ErrorNum[%02d],DataSize[%03d] - Correction Fail", uErrorStatus, nDataSize );

		#if 0		/* 09.04.27 */
		printk("\nData:");
		for ( i = 0; i< 16; ++i )
		{
			printk("0x%02X ", nPageBuffer[i]);
		}

		printk("\nECC:");
		for ( i = 0; i< 8; ++i )
		{
			printk("0x%02X ", nSpareBuffer[i]);
		}
		printk("\n");
		#endif /* 0 */
		#endif

		#if 0
		printk("\nData:");
		for ( i = 0; i< 512; ++i )
		{
			printk("%02X", nPageBuffer[i]);
		}

		printk("\nECC:");
		for ( i = 0; i< 8; ++i )
		{
			printk("%02X", nSpareBuffer[i]);
		}
		printk("\n");

		#endif

		res = ERR_TCC_MTD_IO_FAILED_CORRECTION_MLC_ECC;
		goto ErrorCorrectionMLC;
	}
	else if ( uErrorStatus == HwERR_NUM_NOERR )
	{
		res = (TCC_MTD_IO_ERROR)SUCCESS;
	}
	else
	{
		for ( i = 0; i < uErrorStatus; ++i )
		{
			uErrAddr = *(unsigned long int*)(&pMTD_ECC->ECC_EADDR0+i);

			if ( ( uErrAddr >> 3 ) < nDataSize )
				nPageBuffer[uErrAddr>>3] ^= (1<<(uErrAddr &0x7));
		}
	}

ErrorCorrectionMLC:
	/* Disable MLC ECC */
	//IO_CKC_DisableBUS( IO_CKC_BUS_ECC );
	
	return res;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locRead512Data( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nReadPPSize,
*      												  	 U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff, int nSpareOnOff );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDevInfo	= 
*			nEccOnOff	= 
*			nPageBuffer	= 
*			nReadPPSize	= 
*			nSpareBuffer	= 
*			nSpareOnOff	= 
*			nStartPPage	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locRead528Data( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nReadPPSize,
												  	 		U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff, int nSpareOnOff )
{
	unsigned int		i, j;
	unsigned char		bAlignAddr;
	unsigned char		*pPageB = 0, *pSpareB, *pEccB;
	unsigned int		*pPageDW = 0, *pSpareDW;
	unsigned char		*pDataBuffer, *pSpareBuffer;
	#ifdef _LINUX_
	unsigned char		nECCBuffer[44]__attribute__((aligned(8)));
	#else
	unsigned char		nECCBuffer[44];
	#endif
	DWORD_BYTE			uDWordByte;
	TCC_MTD_IO_ERROR	res = (TCC_MTD_IO_ERROR)SUCCESS;

		
	if ( ( nStartPPage + nReadPPSize ) > ( nDevInfo->Feature.PageSize >> 9 ) )
		return ERR_TCC_MTD_IO_WRONG_PARAMETER;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	res = (TCC_MTD_IO_ERROR)SUCCESS;
	nSpareOnOff = TNFTL_READ_SPARE_ON;

	//=========================================================================
	// Check Align of PageBuffer Address
	//=========================================================================
	bAlignAddr = ( (unsigned int)nPageBuffer & 3 ) ? 0 : 1;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================	
	if ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	else
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

	if ( nEccOnOff == ECC_ON )
		TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );
	
	//=========================================================================
	// Get Buffer Pointer
	//=========================================================================
	/* Adapt type of address */
	if ( bAlignAddr )
	{
		pPageDW		= (unsigned int*)nPageBuffer;
		pSpareDW	= (unsigned int*)nSpareBuffer;		
	}
	else
	{
		pPageB		= (unsigned char*)nPageBuffer;
		pSpareB		= (unsigned char*)nSpareBuffer;
	}

	//----------------------------------------------
	//	Read Data as 512Bytes repeatly
	//----------------------------------------------
	for ( j = 0; j < nReadPPSize; ++j )
	{
		/* Set Data Buffer */
		pDataBuffer = ( bAlignAddr ) ? (unsigned char*)pPageDW : (unsigned char*)pPageB;

		//####################################################
		//#	Read 512 Page Data
		//####################################################
		//----------------------------------------------
		//	MCU ACCESS
		//----------------------------------------------
		//#if defined( NAND_IO_USE_MCU_ACCESS )
		/* Setup ECC Block */
		if ( nEccOnOff == ECC_ON )
		{
			TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_DECODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwDATA_PA );
			pMTD_ECC->ECC_CTRL		|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
			pMTD_ECC->ECC_CLEAR	 = 0x00000000;			/* Clear ECC Block		*/
		}

		/* Read 512 Data Area */
		i = ( nDevInfo->BytesPerSector >> 2);
		do {
			if ( bAlignAddr )
			{
				*pPageDW = pMTD_NFC->NFC_WDATA;++pPageDW;
			}
			else
			{
				uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
				*pPageB = uDWordByte.BYTE[0];++pPageB;
				*pPageB = uDWordByte.BYTE[1];++pPageB;
				*pPageB = uDWordByte.BYTE[2];++pPageB;
				*pPageB = uDWordByte.BYTE[3];++pPageB;
			}
		}while(--i);

		//----------------------------------------------
		//	DMA ACCESS
		//----------------------------------------------
		//#elif defined( NAND_IO_USE_DMA_ACCESS )		
		///* Setup ECC Block */
		//if ( nEccOnOff == ECC_ON )
		//{
		//	#if defined(_WINCE_) || defined(_LINUX_)
		//	NAND_IO_SetupECC( (U16)nEccOnOff, ECC_DECODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
		//	#else
		//	NAND_IO_SetupECC( (U16)nEccOnOff, ECC_DECODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&pMTD_NFC->NFC_LDATA );
		//	#endif
		//	pMTD_ECC->ECC_CTRL		|= ( 512 << ECC_SHIFT_DATASIZE );
		//	pMTD_ECC->ECC_CLEAR	 = 0x00000000;			/* Clear ECC Block		*/
		//}
		//
		//#ifdef READ_SPEED_CHECK
		//NAND_IO_GPIO_Toggle(Hw13);
		//#endif /* READ_SPEED_CHECK */
		//
		//
		//#ifdef READ_SPEED_CHECK
		//BITCLR(pMTD_GPIO->GPBDAT, Hw15);
		//#endif
		//
		///* Start DMA on NFC BUS */
		//#if defined(_LINUX_) || defined(_WINCE_)
		//NAND_IO_SetupDMA( (void*)&NAND_IO_HwLDATA_PA, 0, 0,
		//				  (void*)pDataBuffer, 4, 0,
		//				  NAND_IO_DMA_READ );
		//#else
		//NAND_IO_SetupDMA( (void*)&pMTD_NFC->NFC_LDATA, 0, 0,
		//				  (void*)pDataBuffer, 4, 0,
		//				  NAND_IO_DMA_READ );
		//#endif
		//
		//#ifdef READ_SPEED_CHECK
		//NAND_IO_GPIO_Toggle(Hw15);
		//#endif
		//
		//if ( bAlignAddr )
		//	pPageDW += 128;
		//else
		//	pPageB += 512;
		//
		//#ifdef READ_SPEED_CHECK		
		//NAND_IO_GPIO_Toggle(Hw13);	//------------------------------------->> DMA Transfer
		//#endif
		//
		//#endif
		//####################################################
		//####################################################
		/* Read 'n'Bytes ECC data */

		if ( ( nDevInfo->Feature.MediaType & A_SLC ) || ( nDevInfo->Feature.MediaType & A_MLC ) )
			i = 10;
		else
			i = nDevInfo->EccDataSize;

		// Clear ECC Buffer
		memset( nECCBuffer, 0xFF, 44 );
		
		/* Adapt type of address */
		pEccB = (unsigned char*)nECCBuffer;

		while ( i )
		{
			/* Read as DWORD */
			if ( i >= 4 )
			{
				uDWordByte.DWORD = WORD_OF(pMTD_NFC->NFC_WDATA);
				*pEccB = uDWordByte.BYTE[0];++pEccB;
				*pEccB = uDWordByte.BYTE[1];++pEccB;
				*pEccB = uDWordByte.BYTE[2];++pEccB;
				*pEccB = uDWordByte.BYTE[3];++pEccB;
				i -= 4;
			}
			/* Read as WORD */
			else if ( i >= 2 )
			{
				*pEccB = (unsigned char)pMTD_NFC->NFC_SDATA; ++pEccB;
				*pEccB = (unsigned char)pMTD_NFC->NFC_SDATA; ++pEccB;			
				i -= 2;
			}
			/* Read as BYTE */
			else
			{
				*pEccB = (unsigned char)pMTD_NFC->NFC_SDATA; ++pEccB;			
				i -= 1;
			}
		}

		/* Check and Correct ECC code */
		if ( nEccOnOff == ECC_ON )
		{
			if ( nDevInfo->EccType == SLC_ECC_TYPE )
			{
				//===================================
				// SLC ECC Correction
				//===================================				
				pSpareBuffer += 8;
				res |= TCC_MTD_IO_locCorrectionSLC( pDataBuffer, nECCBuffer );
			}
			else
			{
				res |= TCC_MTD_IO_locCorrectionMLC( nDevInfo->EccType, pDataBuffer, nECCBuffer, nDevInfo->BytesPerSector );

				pSpareB += nDevInfo->EccDataSize;
			}
		}
	}

	//=========================================================================
	// Return
	//=========================================================================
	if ( nEccOnOff == ECC_ON )
		TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );

	return res;

}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locRead512DataDoubleBuf( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nReadPPSize,
*      												  	 				U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff, int nSpareOnOff );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDevInfo	= 
*			nEccOnOff	= 
*			nPageBuffer	= 
*			nReadPPSize	= 
*			nSpareBuffer	= 
*			nSpareOnOff	= 
*			nStartPPage	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locRead512DataDoubleBuf( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nReadPPSize,
												  	 				U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff, int nSpareOnOff )
{
	unsigned int		i, j, k;
	unsigned char		bAlignAddr;
	unsigned char		*pPageB = 0, *pSpareB;
	unsigned int		*pPageDW = 0, *pSpareDW;
	unsigned char		*pDataBuffer, *pSpareBuffer;
	unsigned char		*pPrDataBuffer = 0;
	DWORD_BYTE			uDWordByte;
	TCC_MTD_IO_ERROR	res = (TCC_MTD_IO_ERROR)SUCCESS;
	
	if ( ( nStartPPage + nReadPPSize ) > nDevInfo->UsablePPages )
		return ERR_TCC_MTD_IO_WRONG_PARAMETER;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	res			= (TCC_MTD_IO_ERROR)SUCCESS;
	nSpareOnOff = TNFTL_READ_SPARE_ON;

	//=========================================================================
	// Check Align of PageBuffer Address
	//=========================================================================
	bAlignAddr	= ( (unsigned int)nPageBuffer & 3 ) ? 0 : 1;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================	
	if ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	else
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

	if ( nEccOnOff == ECC_ON )
		TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );
	//=========================================================================
	// Get Buffer Pointer
	//=========================================================================
	/* Adapt type of address */
	if ( bAlignAddr )
	{
		pPageDW		= (unsigned int*)nPageBuffer;
		pSpareDW	= (unsigned int*)nSpareBuffer;		
	}
	else
	{
		pPageB		= (unsigned char*)nPageBuffer;
		pSpareB		= (unsigned char*)nSpareBuffer;
	}

	// Set SpareBuffer Pointer =>> ECCBuffer
	if ( ( nDevInfo->Feature.MediaType  & A_BIG ) || (( nDevInfo->Feature.MediaType  & A_SMALL ) && ( nDevInfo->Feature.MediaType  & A_PARALLEL )))
	{
		
		pSpareB		= (unsigned char*)&gTCC_MTD_IO_ShareEccBuffer[0];
		
		//=========================================================================
		// Empty Page ECCBuffer Pointer Increment
		//=========================================================================
		for ( j = 0; j < nStartPPage; ++j )
			pSpareB += nDevInfo->EccDataSize;
	}

	//----------------------------------------------
	//	Read Data as 512Bytes repeatly
	//----------------------------------------------
	for ( j = 0; j < nReadPPSize; ++j )
	{
		/* Set Data Buffer */
		pDataBuffer = ( bAlignAddr ) ? (unsigned char*)pPageDW : (unsigned char*)pPageB;

		//####################################################
		//#	Read 512 Page Data
		//####################################################
		//----------------------------------------------
		//	MCU ACCESS
		//----------------------------------------------
		#if defined( NAND_IO_USE_MCU_ACCESS )
		
        if ( ( nStartPPage + j ) == ( nDevInfo->UsablePPages - 1 ) )
		{
			memcpy( pDataBuffer, gTCC_MTD_IO_PreEnDecodeEccBuffer, nDevInfo->BytesPerSector );
			continue;
		}
		
		/* Setup ECC Block */
		if ( nEccOnOff == ECC_ON )
		{
			TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_DECODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwDATA_PA );
			pMTD_ECC->ECC_CTRL		|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
			pMTD_ECC->ECC_CLEAR		= 0x00000000;			/* Clear ECC Block		*/
		}

		/* Read 512 Data Area */
		i = ( nDevInfo->BytesPerSector >> 2 );
		do {
			if ( bAlignAddr )
			{
				*pPageDW = pMTD_NFC->NFC_WDATA;++pPageDW;
			}
			else
			{
				uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
				*pPageB = uDWordByte.BYTE[0];++pPageB;
				*pPageB = uDWordByte.BYTE[1];++pPageB;
				*pPageB = uDWordByte.BYTE[2];++pPageB;
				*pPageB = uDWordByte.BYTE[3];++pPageB;
			}
		}while(--i);

		//----------------------------------------------
		//	DMA ACCESS
		//----------------------------------------------
		#elif defined( NAND_IO_USE_DMA_ACCESS )		
		/* Setup ECC Block */

		if ( ( nStartPPage + j ) == ( nDevInfo->UsablePPages - 1 ) )
		{
			nEccOnOff = ECC_OFF;			
		}
		
		if ( nEccOnOff == ECC_ON )
		{
			TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_DECODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
			pMTD_ECC->ECC_CTRL		|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
			pMTD_ECC->ECC_CLEAR	 	= 0x00000000;			/* Clear ECC Block		*/
		}
		/* Disable DMA Ahead */
		/* Start DMA on NFC BUS */
		TCC_MTD_IO_SetupDMADoubleBuf( NAND_IO_DMA_READ, j, nDevInfo->BytesPerSector );

		#if defined(TCC89XX) || defined(TCC92XX)
		TCC_MTD_IO_IRQ_Mask();
		pMTD_NFC->NFC_RSTART	= 0;
		#elif defined(TCC93XX)  || defined(TCC88XX)
		TCC_MTD_IO_IRQ_Mask();
		pMTD_NFC->NFC_PRSTART	= 0;		
		#endif 

		if ( j != 0 )
		{
			if ( j & 1 )
				memcpy( pPrDataBuffer, gpMTD_DMA_WorkBuffer1, nDevInfo->BytesPerSector );
			else
				memcpy( pPrDataBuffer, gpMTD_DMA_WorkBuffer0, nDevInfo->BytesPerSector );
				
		}

		#if defined(TCC89XX) || defined(TCC92XX)
		while ( ISZERO(pMTD_NFC->NFC_IREQ, HwNFC_IREQ_FLAG0) );
		TCC_MTD_IO_IRQ_UnMask();
		#elif defined(TCC93XX) || defined(TCC88XX)
		
		#if 1 //def FOR_LOOP
		for( k = 0; k < 10000000; k++ )
		{
			if( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_RDFG )
				break;

			ASM_NOP;
		}

		if( !(pMTD_NFC->NFC_IRQ & HwNFC_IRQ_RDFG ) )
		{
			printk("[%s] Read End Flag Check Fail !!!!!!!!!!!!!!!!!!!!! \n", __FUNCTION__ );
			return ERR_TCC_MTD_IO_ATTEMPT_REREAD;
		}		
		#else
		while (ISZERO(pMTD_NFC->NFC_IRQ, HwNFC_IRQ_RDFG ));
		#endif

		pMTD_NFC->NFC_IRQ 	|= HwNFC_IRQ_RDFG;
		TCC_MTD_IO_IRQ_UnMask();
		#endif		

		if ( j == (unsigned int)( nReadPPSize - 1 ) )
		{	/*
			if ( j & 1 )
				memcpy( pDataBuffer, gpMTD_DMA_WorkBuffer0, 512 );
			else
				memcpy( pDataBuffer, gpMTD_DMA_WorkBuffer1, 512 );
			*/
			memcpy( pDataBuffer, gTCC_MTD_IO_PreEnDecodeEccBuffer, nDevInfo->BytesPerSector );
		}
		else
		{
			pPrDataBuffer = pDataBuffer;	// Buffer Pointer Backup

			if ( j & 1 )
				pDataBuffer =(unsigned char *)gpMTD_DMA_WorkBuffer0;			
			else
				pDataBuffer =(unsigned char *)gpMTD_DMA_WorkBuffer1;
		}

		if ( bAlignAddr )
			pPageDW += ( nDevInfo->BytesPerSector >> 2 );
		else
			pPageB += nDevInfo->BytesPerSector;

		#endif
		
		//####################################################
		//####################################################

		if ( ( nDevInfo->Feature.MediaType  & A_BIG ) || (( nDevInfo->Feature.MediaType  & A_SMALL ) && ( nDevInfo->Feature.MediaType  & A_PARALLEL )))
		{
			// NOP	
		}
		else if ( nDevInfo->Feature.MediaType & A_SMALL )
		{
			/* Set Spare Buffer */
			pSpareBuffer = ( bAlignAddr ) ? (unsigned char*)pSpareDW : (unsigned char*)pSpareB;

			/* Read 16Bytes spare data */
			i = 4;
			do {
				if ( bAlignAddr )
				{
					*pSpareDW = pMTD_NFC->NFC_WDATA;++pSpareDW;
				}	
				else
				{
					uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
					*pSpareB = uDWordByte.BYTE[0];++pSpareB;
					*pSpareB = uDWordByte.BYTE[1];++pSpareB;
					*pSpareB = uDWordByte.BYTE[2];++pSpareB;
					*pSpareB = uDWordByte.BYTE[3];++pSpareB;
				}
			}while(--i);


			pSpareB = (unsigned char*)pSpareBuffer;
			pSpareB +=	8;
		}

			/* Check and Correct ECC code */
			if ( nEccOnOff == ECC_ON )
			{
				if ( ( nDevInfo->EccType == SLC_ECC_TYPE )  && ( nDevInfo->Feature.MediaType & A_SMALL ) )
				{
					//===================================
					// SLC ECC Correction
					//===================================				
					pSpareBuffer += 8;
					res |= TCC_MTD_IO_locCorrectionSLC( pDataBuffer, pSpareBuffer );
				}
				if ( 	( pSpareB[0] == 0xFF ) && ( pSpareB[1] == 0xFF ) &&
						( pSpareB[2] == 0xFF ) && ( pSpareB[3] == 0xFF ) &&
						( pSpareB[4] == 0xFF ) && ( pSpareB[5] == 0xFF ) &&
						( pSpareB[6] == 0xFF ) && ( pSpareB[7] == 0xFF ))
				{
#if defined(TCC89XX) || defined(TCC92XX)
					ASM_NOP;
#elif defined(TCC93XX) || defined(TCC88XX)
#if 0
					BITSET( pMTD_ECC->ECC_CTRL, HwECC_CTRL_CORPASS_EN );
					pMTD_ECC->ECC_CODE0		= 0;
					pMTD_ECC->ECC_CODE1		= 0;
					pMTD_ECC->ECC_CODE2		= 0;
					pMTD_ECC->ECC_CODE3		= 0;
					pMTD_ECC->ECC_CODE4		= 0;
					pMTD_ECC->ECC_CODE5		= 0;
					pMTD_ECC->ECC_CODE6		= 0;
					pMTD_ECC->ECC_CODE7		= 0;
					pMTD_ECC->ECC_CODE8		= 0;
					pMTD_ECC->ECC_CODE9		= 0;
					pMTD_ECC->ECC_CODE10	= 0;
					BITCLR( pMTD_ECC->ECC_CTRL, HwECC_CTRL_CORPASS_EN );
#else
					res |= TCC_MTD_IO_locCorrectionMLC( nDevInfo->EccType, pDataBuffer, pSpareB, nDevInfo->BytesPerSector );
					res = SUCCESS; 
#endif 
#endif
				}	
				else
				{	
					res |= TCC_MTD_IO_locCorrectionMLC( nDevInfo->EccType, pDataBuffer, pSpareB, nDevInfo->BytesPerSector );
					if( res != SUCCESS ){

						int j;

						for ( j = 0; j < 20; ++j )
						{
							printk("%02X", pDataBuffer[j] );
						}
						printk("\n");

						for ( j = 0; j < 20; ++j )
						{
							printk("%02X", pSpareB[j] );
						}
						printk("\n");

							printk("[DoubleBuf] [EccType:%d] [res:%x]\n", nDevInfo->EccType ,res);

						}
					

					pSpareB += nDevInfo->EccDataSize;
				}
			}
		}
	

	//=========================================================================
	// Return
	//=========================================================================
	if ( nEccOnOff == ECC_ON )
		TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );

	return res;

}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locRead512Data( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nReadPPSize,
*      												  	 U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff, int nSpareOnOff );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDevInfo	= 
*			nEccOnOff	= 
*			nPageBuffer	= 
*			nReadPPSize	= 
*			nSpareBuffer	= 
*			nSpareOnOff	= 
*			nStartPPage	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locRead512Data( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nReadPPSize,
												  	 U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff, int nSpareOnOff )
{
	unsigned int		i, j;
	unsigned char		bAlignAddr;
	unsigned int		nECCDataSize = 0;
	unsigned char		*pPageB = 0, *pSpareB;
	unsigned int		*pPageDW = 0, *pSpareDW;
	unsigned char		*pDataBuffer;
	DWORD_BYTE			uDWordByte;
	TCC_MTD_IO_ERROR	res = (TCC_MTD_IO_ERROR)SUCCESS;
	
	if ( ( nStartPPage + nReadPPSize ) > nDevInfo->UsablePPages )
		return ERR_TCC_MTD_IO_WRONG_PARAMETER;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	res			= (TCC_MTD_IO_ERROR)SUCCESS;
	nSpareOnOff = TNFTL_READ_SPARE_ON;

	//=========================================================================
	// Check Align of PageBuffer Address
	//=========================================================================
	bAlignAddr	= ( (unsigned int)nPageBuffer & 3 ) ? 0 : 1;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================	
	if ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	else
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

	if ( nEccOnOff == ECC_ON )
		TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );

	if (( nDevInfo->Feature.MediaType & A_MLC ) ||( nDevInfo->Feature.MediaType & A_SLC ))
		nECCDataSize = 8;
	else if ( ( nDevInfo->Feature.MediaType & A_MLC_8BIT ) || ( nDevInfo->Feature.MediaType & A_MLC_12BIT ) )
		nECCDataSize = 20;
	else if ( nDevInfo->Feature.MediaType & A_MLC_16BIT ) 
		nECCDataSize = 26;
	else if ( nDevInfo->Feature.MediaType & A_MLC_24BIT ) 
		nECCDataSize = 42;
	
	//=========================================================================
	// Get Buffer Pointer
	//=========================================================================
	/* Adapt type of address */
	if ( bAlignAddr )
	{
		pPageDW		= (unsigned int*)nPageBuffer;
		pSpareDW	= (unsigned int*)nSpareBuffer;		
	}
	else
	{
		pPageB		= (unsigned char*)nPageBuffer;
		pSpareB		= (unsigned char*)nSpareBuffer;
	}

	// Set SpareBuffer Pointer =>> ECCBuffer
	if ( ( nDevInfo->Feature.MediaType  & A_BIG ) || (( nDevInfo->Feature.MediaType  & A_SMALL ) && ( nDevInfo->Feature.MediaType  & A_PARALLEL )))
	{

		pSpareB		= (unsigned char*)&gTCC_MTD_IO_ShareEccBuffer[0];

		//=========================================================================
		// Empty Page ECCBuffer Pointer Increment
		//=========================================================================
		for ( j = 0; j < nStartPPage; ++j )
			pSpareB += nECCDataSize;
	}
#if defined(__USE_MTD_NAND_ISR__)
	if( TCC_MTD_IO_IRQ_IsEnabled() )
	{
		int ret;

		sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo 	= nDevInfo;
		sTCC_MTD_IO_NandIsrInfo.pbPageBuffer 	= nPageBuffer;
		sTCC_MTD_IO_NandIsrInfo.pbEccBuffer 	= pSpareB;
		sTCC_MTD_IO_NandIsrInfo.pbSpareBuffer 	= nSpareBuffer;
		sTCC_MTD_IO_NandIsrInfo.usCurrentPPage 	= nStartPPage;
		sTCC_MTD_IO_NandIsrInfo.usStartPPage 	= nStartPPage;
		sTCC_MTD_IO_NandIsrInfo.usPPagesLeft 	= nReadPPSize;
		sTCC_MTD_IO_NandIsrInfo.iEccOnOff 		= nEccOnOff;
		sTCC_MTD_IO_NandIsrInfo.wait_complete 	= 0;
		sTCC_MTD_IO_NandIsrInfo.uiState 		= TCC_MTD_IO_IRQ_STATE_READ;
		sTCC_MTD_IO_NandIsrInfo.error 			= SUCCESS;
		sTCC_MTD_IO_NandIsrInfo.ubIsRun			= TRUE;
		
		if ( sTCC_MTD_IO_NandIsrInfo.iEccOnOff == ECC_ON )
		{
			/* Setup ECC Block */
			#if defined(_WINCE_) || defined(_LINUX_)
			TCC_MTD_IO_locSetupECC( ECC_ON, ECC_DECODE, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->EccType, NAND_DMA_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
			#else
			TCC_MTD_IO_locSetupECC( ECC_ON, ECC_DECODE, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->EccType, NAND_DMA_ACCESS, (U32)&pNFC->NFC_LDATA );
			#endif
			pMTD_ECC->ECC_CTRL	|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
			pMTD_ECC->ECC_CLEAR	= 0x00000000;
		}

		TCC_MTD_IO_IRQ_SetupDMA(gpMTD_DMA_PhyBuffer0, NAND_IO_DMA_READ, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector );

		do
		{	
			ret = wait_event_interruptible(sTCC_MTD_IO_NandIsrInfo.wait_q, sTCC_MTD_IO_NandIsrInfo.wait_complete);

			if( ret == 0)
			{
				// ok
				//printk("[TCC_MTD] Read ISR OK \n");
				res = sTCC_MTD_IO_NandIsrInfo.error;
			}
			else if( abs(ret) == ERESTARTSYS )
			{
				//printk("\n!! RE Wait Sleep !!\n");
			}		
			else 
			{
				// fail
				printk("[TCC_MTD] mtd:wait_event_interruptible error!(ret=%d),[error:0x%08X] [complete:%d]\n"
								,ret, sTCC_MTD_IO_NandIsrInfo.error, sTCC_MTD_IO_NandIsrInfo.wait_complete);
				//res = ERR_TCC_MTD_IO_FAILED_READ;
			}
		} while( !sTCC_MTD_IO_NandIsrInfo.wait_complete );

		sTCC_MTD_IO_NandIsrInfo.uiState = TCC_MTD_IO_IRQ_STATE_NONE;
		sTCC_MTD_IO_NandIsrInfo.ubIsRun	= FALSE;
	}
	else
#endif
	{
		//----------------------------------------------
		//	Read Data as 512Bytes repeatly
		//----------------------------------------------
		for ( j = 0; j < nReadPPSize; ++j )
		{
			/* Set Data Buffer */
			pDataBuffer = ( bAlignAddr ) ? (unsigned char*)pPageDW : (unsigned char*)pPageB;

			if ( ( nStartPPage + j ) == ( nDevInfo->UsablePPages - 1 ) )
			{
				memcpy( pDataBuffer, gTCC_MTD_IO_PreEnDecodeEccBuffer, nDevInfo->BytesPerSector );
				continue;
			}
			 
			{		
				//####################################################
				//#	Read 512 Page Data
				//####################################################
				//----------------------------------------------
				//	MCU ACCESS
				//----------------------------------------------
				//#if defined( NAND_IO_USE_MCU_ACCESS )
				/* Setup ECC Block */
				if ( nEccOnOff == ECC_ON )
				{
					TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_DECODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwDATA_PA );
					pMTD_ECC->ECC_CTRL		|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
					pMTD_ECC->ECC_CLEAR	 	= 0x00000000;			/* Clear ECC Block		*/
				}

				/* Read 512 Data Area */
				i = ( nDevInfo->BytesPerSector >> 2);
				do {
					if ( bAlignAddr )
					{
						*pPageDW = pMTD_NFC->NFC_WDATA;++pPageDW;
					}
					else
					{
						uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
						*pPageB = uDWordByte.BYTE[0];++pPageB;
						*pPageB = uDWordByte.BYTE[1];++pPageB;
						*pPageB = uDWordByte.BYTE[2];++pPageB;
						*pPageB = uDWordByte.BYTE[3];++pPageB;
					}
				}while(--i);

				//----------------------------------------------
				//	DMA ACCESS
				//----------------------------------------------
				//#elif defined( NAND_IO_USE_DMA_ACCESS )		
				///* Setup ECC Block */
				//if ( nEccOnOff == ECC_ON )
				//{
				//	#if defined(_WINCE_) || defined(_LINUX_)
				//	NAND_IO_SetupECC( (U16)nEccOnOff, ECC_DECODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
				//	#else
				//	NAND_IO_SetupECC( (U16)nEccOnOff, ECC_DECODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&pMTD_NFC->NFC_LDATA );
				//	#endif
				//	pMTD_ECC->ECC_CTRL		|= ( 512 << ECC_SHIFT_DATASIZE );
				//	pMTD_ECC->ECC_CLEAR	 = 0x00000000;			/* Clear ECC Block		*/
				//}
				//
				//#ifdef READ_SPEED_CHECK
				//NAND_IO_GPIO_Toggle(Hw13);
				//#endif /* READ_SPEED_CHECK */
				//
				//
				//#ifdef READ_SPEED_CHECK
				//BITCLR(pMTD_GPIO->GPBDAT, Hw15);
				//#endif
				//
				///* Start DMA on NFC BUS */
				//#if defined(_LINUX_) || defined(_WINCE_)
				//NAND_IO_SetupDMA( (void*)&NAND_IO_HwLDATA_PA, 0, 0,
				//				  (void*)pDataBuffer, 4, 0,
				//				  NAND_IO_DMA_READ );
				//#else
				//NAND_IO_SetupDMA( (void*)&pMTD_NFC->NFC_LDATA, 0, 0,
				//				  (void*)pDataBuffer, 4, 0,
				//				  NAND_IO_DMA_READ );
				//#endif
				//
				//#ifdef READ_SPEED_CHECK
				//NAND_IO_GPIO_Toggle(Hw15);
				//#endif
				//
				//if ( bAlignAddr )
				//	pPageDW += 128;
				//else
				//	pPageB += 512;
				//
				//#ifdef READ_SPEED_CHECK		
				//NAND_IO_GPIO_Toggle(Hw13);	//------------------------------------->> DMA Transfer
				//#endif
				//
				//#endif
				//####################################################
				//####################################################

				if ( ( nDevInfo->Feature.MediaType  & A_BIG ) || (( nDevInfo->Feature.MediaType  & A_SMALL ) && ( nDevInfo->Feature.MediaType  & A_PARALLEL )))
				{
					// NOP	
				}
				else if ( nDevInfo->Feature.MediaType & A_SMALL )
				{
					/* Read 16Bytes spare data */
					i = 4;
					do {
						if ( bAlignAddr )
						{
							*pSpareDW = pMTD_NFC->NFC_WDATA;++pSpareDW;
						}	
						else
						{
							uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
							*pSpareB = uDWordByte.BYTE[0];++pSpareB;
							*pSpareB = uDWordByte.BYTE[1];++pSpareB;
							*pSpareB = uDWordByte.BYTE[2];++pSpareB;
							*pSpareB = uDWordByte.BYTE[3];++pSpareB;
						}
					}while(--i);

					pSpareB =	&pSpareB[NAND_IO_SPARE_SIZE_SMALL];
				}

				/* Check and Correct ECC code */
				if ( nEccOnOff == ECC_ON )
				{
					if ( nDevInfo->EccType == SLC_ECC_TYPE )
					{
						//===================================
						// SLC ECC Correction
						//===================================				
						res |= TCC_MTD_IO_locCorrectionSLC( pDataBuffer, pSpareB );
					}
					
					if ( ( pSpareB[0] == 0xFF ) && ( pSpareB[1] == 0xFF ) &&
					 	 ( pSpareB[2] == 0xFF ) && ( pSpareB[3] == 0xFF ) &&
					 	 ( pSpareB[4] == 0xFF ) && ( pSpareB[5] == 0xFF ) &&
					 	 ( pSpareB[6] == 0xFF ) && ( pSpareB[7] == 0xFF ))
					{
						#if defined(TCC89XX) || defined(TCC92XX)
						ASM_NOP;
						#elif defined(TCC93XX) || defined(TCC88XX)
#if 0						
						BITSET( pMTD_ECC->ECC_CTRL, HwECC_CTRL_CORPASS_EN );
						pMTD_ECC->ECC_CODE0		= 0;
						pMTD_ECC->ECC_CODE1		= 0;
						pMTD_ECC->ECC_CODE2		= 0;
						pMTD_ECC->ECC_CODE3		= 0;
						pMTD_ECC->ECC_CODE4		= 0;
						pMTD_ECC->ECC_CODE5		= 0;
						pMTD_ECC->ECC_CODE6		= 0;
						pMTD_ECC->ECC_CODE7		= 0;
						pMTD_ECC->ECC_CODE8		= 0;
						pMTD_ECC->ECC_CODE9		= 0;
						pMTD_ECC->ECC_CODE10	= 0;
						BITCLR( pMTD_ECC->ECC_CTRL, HwECC_CTRL_CORPASS_EN );
#else	
						res |= TCC_MTD_IO_locCorrectionMLC( nDevInfo->EccType, pDataBuffer, pSpareB, nDevInfo->BytesPerSector );
						res = SUCCESS; 
#endif 
						#endif
					}				
					else
					{
						res |= TCC_MTD_IO_locCorrectionMLC( nDevInfo->EccType, pDataBuffer, pSpareB, nDevInfo->BytesPerSector );
						if( res != SUCCESS )
						{
							int j;
							for ( j = 0; j < 40; ++j )
							{
								printk("%02X", pSpareB[j] );
							}
							printk("\n");

							printk("[Buf] [EccType:%d] [res:%x]\n", nDevInfo->EccType ,res);
						}	
						
						
						
						pSpareB += nECCDataSize;
					}
				}
			}
		}
	}
	
	//=========================================================================
	// Return
	//=========================================================================
	if ( nEccOnOff == ECC_ON )
		TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );

	return res;

}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locRead512Data( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nReadPPSize,
*      												  	 U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff, int nSpareOnOff );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDevInfo	= 
*			nEccOnOff	= 
*			nPageBuffer	= 
*			nReadPPSize	= 
*			nSpareBuffer	= 
*			nSpareOnOff	= 
*			nStartPPage	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locRead512DataGoldenPage( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nReadPPSize,
												  	 U8 *nPageBuffer, U8 *nSpareBuffer, int nEccOnOff, int nSpareOnOff )
{
	unsigned int		i, j;
	unsigned char		bAlignAddr;
	unsigned int		nECCDataSize = 0;
	unsigned char		*pPageB = 0, *pSpareB;
	unsigned int		*pPageDW = 0, *pSpareDW;
	unsigned char		*pDataBuffer, *pSpareBuffer;
	DWORD_BYTE			uDWordByte;
	TCC_MTD_IO_ERROR	res = (TCC_MTD_IO_ERROR)SUCCESS;
	
	if ( ( nStartPPage + nReadPPSize ) > nDevInfo->UsablePPages )
		return ERR_TCC_MTD_IO_WRONG_PARAMETER;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	res = (TCC_MTD_IO_ERROR)SUCCESS;
	nSpareOnOff = TNFTL_READ_SPARE_ON;

	//=========================================================================
	// Check Align of PageBuffer Address
	//=========================================================================
	bAlignAddr = ( (unsigned int)nPageBuffer & 3 ) ? 0 : 1;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//========================================================================`	=	
	if ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	else
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

	if ( nEccOnOff == ECC_ON )
		TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );

	if (( nDevInfo->Feature.MediaType & A_MLC ) ||( nDevInfo->Feature.MediaType & A_SLC ))
		nECCDataSize = 8;
	else if ( ( nDevInfo->Feature.MediaType & A_MLC_8BIT ) || ( nDevInfo->Feature.MediaType & A_MLC_12BIT ) )
		nECCDataSize = 20;
	else if ( nDevInfo->Feature.MediaType & A_MLC_16BIT ) 
		nECCDataSize = 26;
	else if ( nDevInfo->Feature.MediaType & A_MLC_24BIT ) 
		nECCDataSize = 42;
	
	//=========================================================================
	// Get Buffer Pointer
	//=========================================================================
	/* Adapt type of address */
	if ( bAlignAddr )
	{
		pPageDW		= (unsigned int*)nPageBuffer;
		pSpareDW	= (unsigned int*)nSpareBuffer;		
	}
	else
	{
		pPageB		= (unsigned char*)nPageBuffer;
		pSpareB		= (unsigned char*)nSpareBuffer;
	}

	// Set SpareBuffer Pointer =>> ECCBuffer
	if ( ( nDevInfo->Feature.MediaType  & A_BIG ) || (( nDevInfo->Feature.MediaType  & A_SMALL ) && ( nDevInfo->Feature.MediaType  & A_PARALLEL )))
	{
	
		pSpareB		= (unsigned char*)&gTCC_MTD_IO_ShareEccBuffer[0];
		
		//=========================================================================
		// Empty Page ECCBuffer Pointer Increment
		//=========================================================================
		for ( j = 0; j < nStartPPage; ++j )
			pSpareB += nECCDataSize;
	}
	
	//----------------------------------------------
	//	Read Data as 512Bytes repeatly
	//----------------------------------------------
	for ( j = 0; j < nReadPPSize; ++j )
	{
		/* Set Data Buffer */
		pDataBuffer = ( bAlignAddr ) ? (unsigned char*)pPageDW : (unsigned char*)pPageB;

		{		
			//####################################################
			//#	Read 512 Page Data
			//####################################################
			//----------------------------------------------
			//	MCU ACCESS
			//----------------------------------------------
			//#if defined( NAND_IO_USE_MCU_ACCESS )
			/* Setup ECC Block */
			if ( nEccOnOff == ECC_ON )
			{
				TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_DECODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwDATA_PA );
				pMTD_ECC->ECC_CTRL		|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
				pMTD_ECC->ECC_CLEAR	 	= 0x00000000;			/* Clear ECC Block		*/
			}

			/* Read 512 Data Area */
			i = ( nDevInfo->BytesPerSector >> 2 );
			do {
				if ( bAlignAddr )
				{
					*pPageDW = pMTD_NFC->NFC_WDATA;++pPageDW;
				}
				else
				{
					uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
					*pPageB = uDWordByte.BYTE[0];++pPageB;
					*pPageB = uDWordByte.BYTE[1];++pPageB;
					*pPageB = uDWordByte.BYTE[2];++pPageB;
					*pPageB = uDWordByte.BYTE[3];++pPageB;
				}
			}while(--i);

			//----------------------------------------------
			//	DMA ACCESS
			//----------------------------------------------
			//#elif defined( NAND_IO_USE_DMA_ACCESS )		
			///* Setup ECC Block */
			//if ( nEccOnOff == ECC_ON )
			//{
			//	#if defined(_WINCE_) || defined(_LINUX_)
			//	NAND_IO_SetupECC( (U16)nEccOnOff, ECC_DECODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
			//	#else
			//	NAND_IO_SetupECC( (U16)nEccOnOff, ECC_DECODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&pMTD_NFC->NFC_LDATA );
			//	#endif
			//	pMTD_ECC->ECC_CTRL		|= ( 512 << ECC_SHIFT_DATASIZE );
			//	pMTD_ECC->ECC_CLEAR	 = 0x00000000;			/* Clear ECC Block		*/
			//}
			//
			//#ifdef READ_SPEED_CHECK
			//NAND_IO_GPIO_Toggle(Hw13);
			//#endif /* READ_SPEED_CHECK */
			//
			//
			//#ifdef READ_SPEED_CHECK
			//BITCLR(pMTD_GPIO->GPBDAT, Hw15);
			//#endif
			//
			///* Start DMA on NFC BUS */
			//#if defined(_LINUX_) || defined(_WINCE_)
			//NAND_IO_SetupDMA( (void*)&NAND_IO_HwLDATA_PA, 0, 0,
			//				  (void*)pDataBuffer, 4, 0,
			//				  NAND_IO_DMA_READ );
			//#else
			//NAND_IO_SetupDMA( (void*)&pMTD_NFC->NFC_LDATA, 0, 0,
			//				  (void*)pDataBuffer, 4, 0,
			//				  NAND_IO_DMA_READ );
			//#endif
			//
			//#ifdef READ_SPEED_CHECK
			//NAND_IO_GPIO_Toggle(Hw15);
			//#endif
			//
			//if ( bAlignAddr )
			//	pPageDW += 128;
			//else
			//	pPageB += 512;
			//
			//#ifdef READ_SPEED_CHECK		
			//NAND_IO_GPIO_Toggle(Hw13);	//------------------------------------->> DMA Transfer
			//#endif
			//
			//#endif
			//####################################################
			//####################################################

			if ( ( nDevInfo->Feature.MediaType  & A_BIG ) || (( nDevInfo->Feature.MediaType  & A_SMALL ) && ( nDevInfo->Feature.MediaType  & A_PARALLEL )))
			{
				// NOP	
			}
			else if ( nDevInfo->Feature.MediaType & A_SMALL )
			{
				/* Set Spare Buffer */
				pSpareBuffer = ( bAlignAddr ) ? (unsigned char*)pSpareDW : (unsigned char*)pSpareB;

				/* Read 16Bytes spare data */
				i = 4;
				do {
					if ( bAlignAddr )
					{
						*pSpareDW = pMTD_NFC->NFC_WDATA;++pSpareDW;
					}	
					else
					{
						uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
						*pSpareB = uDWordByte.BYTE[0];++pSpareB;
						*pSpareB = uDWordByte.BYTE[1];++pSpareB;
						*pSpareB = uDWordByte.BYTE[2];++pSpareB;
						*pSpareB = uDWordByte.BYTE[3];++pSpareB;
					}
				}while(--i);


				pSpareB = (unsigned char*)pSpareBuffer;
				pSpareB +=	NAND_IO_SPARE_SIZE_SMALL;
			}

			/* Check and Correct ECC code */
			if ( nEccOnOff == ECC_ON )
			{
				if ( ( nDevInfo->EccType == SLC_ECC_TYPE ) && ( nDevInfo->Feature.MediaType & A_SMALL ) )
				{
					//===================================
					// SLC ECC Correction
					//===================================				
					pSpareBuffer += NAND_IO_SPARE_SIZE_SMALL;
					res |= TCC_MTD_IO_locCorrectionSLC( pDataBuffer, pSpareBuffer );
				}
				else
				{
					res |= TCC_MTD_IO_locCorrectionMLC( nDevInfo->EccType, pDataBuffer, pSpareB, nDevInfo->BytesPerSector );
					if( res != SUCCESS )
						printk("[Buf] [EccType:%d] [EccDataSize:%d] [res:%x] \n", nDevInfo->EccType , nECCDataSize, res);	

					pSpareB += nECCDataSize;
				}
			}
		}
	}
	//=========================================================================
	// Return
	//=========================================================================
	if ( nEccOnOff == ECC_ON )
		TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );

	return res;

}


/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locReadSpareData( TCC_MTD_IO_DEVINFO *nDevInfo, U8 *nSpareBuffer, int nPageEccOnOff );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDevInfo	= 
*			nPageEccOnOff	= 
*			nSpareBuffer	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locGoldenReadSpareData( TCC_MTD_IO_DEVINFO *nDevInfo, U8 *nSpareBuffer, int nPageEccOnOff )
{
	unsigned int		i;

	unsigned int		nSpareTotalSize;
	unsigned char		bAlignAddr;
	unsigned char		*pSpareB = 0;
	unsigned int		*pSpareDW = 0;
	unsigned char		*pSpareBuffer;
	unsigned int		nReadSize;
	unsigned char		*pEccB = 0;

	#ifdef _LINUX_
	//unsigned char		nSpareDataBuf[64]__attribute__((aligned(8)));	
	unsigned char		nECCBuffer[44]__attribute__((aligned(8)));
	#endif
	DWORD_BYTE			uDWordByte;
	TCC_MTD_IO_ERROR	res;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	res = (TCC_MTD_IO_ERROR)SUCCESS;

	if ( ( nDevInfo->Feature.MediaType & A_MLC_16BIT ) || ( nDevInfo->Feature.MediaType & A_MLC_24BIT ) )
	{
		#if defined(TCC89XX) || defined(TCC92XX)
		if ( ( nDevInfo->Feature.PageSize == 4096 ) &&  ( (nDevInfo->Feature.MediaType & A_PARALLEL ) == 0 ) )
			nSpareTotalSize = 8;
		else
		#endif
			nSpareTotalSize = 12;
	}
	else
		nSpareTotalSize = 16;

	//=========================================================================
	// Check Align of PageBuffer Address
	//=========================================================================
	bAlignAddr = ( (unsigned int)nSpareBuffer & 3 ) ? 0 : 1;

	TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );
	
	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================	
	if ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	else
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

	//=========================================================================
	// Get Buffer Pointer
	//=========================================================================
	/* Adapt type of address */
	if ( bAlignAddr )
		pSpareDW	= (unsigned int*)nSpareBuffer;
	else
		pSpareB		= (unsigned char*)nSpareBuffer;
	
	if ( nDevInfo->Feature.MediaType & A_SMALL )
	{
		//----------------------------------------------
		//	Read Small Page Spare Data: 16Byte
		//----------------------------------------------
		i = 4;
		
		do {
			if ( bAlignAddr )
			{
				*pSpareDW = pMTD_NFC->NFC_WDATA;++pSpareDW;
			}	
			else
			{
				uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
				*pSpareB = uDWordByte.BYTE[0];++pSpareB;
				*pSpareB = uDWordByte.BYTE[1];++pSpareB;
				*pSpareB = uDWordByte.BYTE[2];++pSpareB;
				*pSpareB = uDWordByte.BYTE[3];++pSpareB;
			}
		}while(--i);

		if ( nDevInfo->Feature.MediaType & A_PARALLEL )
		{
			if ( nPageEccOnOff == PAGE_ECC_ON )
			{
				memset( gTCC_MTD_IO_ShareEccBuffer, 0xFF, 16 );
				
				//=========================================================================
				// Check Align of PageBuffer Address
				//=========================================================================
				bAlignAddr = ( (unsigned int)&gTCC_MTD_IO_ShareEccBuffer & 3 ) ? 0 : 1;
				
				//=========================================================================
				// Get Buffer Pointer
				//=========================================================================
				/* Adapt type of address */
				if ( bAlignAddr )
					pSpareDW	= (unsigned int*)&gTCC_MTD_IO_ShareEccBuffer[0];
				else
					pSpareB		= (unsigned char*)&gTCC_MTD_IO_ShareEccBuffer[0];
				
				//----------------------------------------------
				//	Read Spare Data
				//----------------------------------------------
				i = 4;
				do {
					if ( bAlignAddr )
					{
						*pSpareDW = pMTD_NFC->NFC_WDATA;++pSpareDW;
					}	
					else
					{
						uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
						*pSpareB = uDWordByte.BYTE[0];++pSpareB;
						*pSpareB = uDWordByte.BYTE[1];++pSpareB;
						*pSpareB = uDWordByte.BYTE[2];++pSpareB;
						*pSpareB = uDWordByte.BYTE[3];++pSpareB;
					}
				}while(--i);
			}
		}

		return SUCCESS;
	}

	/* Setup ECC Block */
	TCC_MTD_IO_locSetupECC( ECC_ON, ECC_DECODE, MLC_ECC_4BIT_TYPE, NAND_MCU_ACCESS, (U32)&NAND_IO_HwDATA_PA );
	pMTD_ECC->ECC_CTRL	|= ( nSpareTotalSize << ECC_SHIFT_DATASIZE );
	pMTD_ECC->ECC_CLEAR	= 0x00000000;		// Clear ECC Block
	
	/* Set Spare Buffer */
	pSpareBuffer = ( bAlignAddr ) ? (unsigned char*)pSpareDW : (unsigned char*)pSpareB;

	//----------------------------------------------
	//	Read Spare Data
	//----------------------------------------------
	i = ( nSpareTotalSize >> 2 );
	do {
		if ( bAlignAddr )
		{
			*pSpareDW = pMTD_NFC->NFC_WDATA;++pSpareDW;
		}	
		else
		{
			uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
			*pSpareB = uDWordByte.BYTE[0];++pSpareB;
			*pSpareB = uDWordByte.BYTE[1];++pSpareB;
			*pSpareB = uDWordByte.BYTE[2];++pSpareB;
			*pSpareB = uDWordByte.BYTE[3];++pSpareB;
		}
	}while(--i);

	/* Read 4Bit ECC data */
	nReadSize = 8;
	
	/* Adapt type of address */
	pEccB = (unsigned char*)nECCBuffer;

	//----------------------------------------------
	// Read ECC Code
	//----------------------------------------------
	while ( nReadSize )
	{
		/* Read as DWORD */
		if ( nReadSize >= 4 )
		{
			uDWordByte.DWORD = WORD_OF(pMTD_NFC->NFC_WDATA);
			*pEccB = uDWordByte.BYTE[0];++pEccB;
			*pEccB = uDWordByte.BYTE[1];++pEccB;
			*pEccB = uDWordByte.BYTE[2];++pEccB;
			*pEccB = uDWordByte.BYTE[3];++pEccB;
			nReadSize -= 4;
		}
		/* Read as WORD */
		else if ( nReadSize >= 2 )
		{
			*pEccB = (unsigned char)pMTD_NFC->NFC_SDATA; ++pEccB;
			*pEccB = (unsigned char)pMTD_NFC->NFC_SDATA; ++pEccB;			
			nReadSize -= 2;
		}
		/* Read as BYTE */
		else
		{
			*pEccB = (unsigned char)pMTD_NFC->NFC_SDATA; ++pEccB;			
			nReadSize -= 1;
		}
	}
	
	/* Check and Correct ECC code */
	//===================================
	// 4 Bit MLC ECC Correction
	//===================================
	if ( ( nECCBuffer[0] == 0x00 ) && ( nECCBuffer[1] == 0x00 ) &&
		 ( nECCBuffer[2] == 0x00 ) && ( nECCBuffer[3] == 0x00 ) &&
		 ( nECCBuffer[4] == 0x00 ) && ( nECCBuffer[5] == 0x00 ) &&
		 ( nECCBuffer[6] == 0x00 ) && ( nECCBuffer[7] == 0x00 ))
	{
		res |= TCC_MTD_IO_locCorrectionMLC( MLC_ECC_4BIT_TYPE, pSpareBuffer, nECCBuffer, nSpareTotalSize );
	 	res |= ERR_TCC_MTD_IO_FAILED_CORRECTION_MLC_ECC;

	}
	else if ( ( nECCBuffer[0] == 0xFF ) && ( nECCBuffer[1] == 0xFF ) &&
		 	  ( nECCBuffer[2] == 0xFF ) && ( nECCBuffer[3] == 0xFF ) &&
		 	  ( nECCBuffer[4] == 0xFF ) && ( nECCBuffer[5] == 0xFF ) &&
		 	  ( nECCBuffer[6] == 0xFF ) && ( nECCBuffer[7] == 0xFF ))
 	{
		#if defined(TCC89XX) || defined(TCC92XX)
		ASM_NOP;
		#elif defined(TCC93XX)  || defined(TCC88XX)
#if 0
		BITSET( pMTD_ECC->ECC_CTRL, HwECC_CTRL_CORPASS_EN );
		pMTD_ECC->ECC_CODE0		= 0;
		pMTD_ECC->ECC_CODE1		= 0;
		pMTD_ECC->ECC_CODE2		= 0;
		pMTD_ECC->ECC_CODE3		= 0;
		pMTD_ECC->ECC_CODE4		= 0;
		pMTD_ECC->ECC_CODE5		= 0;
		pMTD_ECC->ECC_CODE6		= 0;
		pMTD_ECC->ECC_CODE7		= 0;
		pMTD_ECC->ECC_CODE8		= 0;
		pMTD_ECC->ECC_CODE9		= 0;
		pMTD_ECC->ECC_CODE10	= 0;
		BITCLR( pMTD_NFC->NFC_CTRL, HwECC_CTRL_CORPASS_EN );
#else
		res |= TCC_MTD_IO_locCorrectionMLC( MLC_ECC_4BIT_TYPE, pSpareBuffer, nECCBuffer, nSpareTotalSize );
		res = SUCCESS;
#endif 
		#endif
	}
	else
	{
		res |= TCC_MTD_IO_locCorrectionMLC( MLC_ECC_4BIT_TYPE, pSpareBuffer, nECCBuffer, nSpareTotalSize );
	}


#if 0		/* 010.02.10 */
	#ifdef TCC_MTD_DEBUG_SPARE
	//for ( i = 0; i < 4; ++i )
	//{
	//	printk("\n");
	//	for ( j = 0; j < 16; ++j )
	//	{
	//		printk("%02x ", nSpareBuffer[(i<<4) + j] );	
	//	}
	//}
	printk("-->");
	printk("\n");
	for ( j = 0; j < 32; ++j )
	{
		printk("%02X ", pSpareBuffer[j] );	
	}
	printk("\n");
	#endif
#endif /* 0 */

	//=========================================================================
	// Ecc Clear
	//=========================================================================
	TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );

	if ( nPageEccOnOff == PAGE_ECC_ON )
	{
		memset( gTCC_MTD_IO_ShareEccBuffer, 0xFF, nDevInfo->EccWholeDataSize );
		
		//=========================================================================
		// Check Align of PageBuffer Address
		//=========================================================================
		bAlignAddr = ( (unsigned int)&gTCC_MTD_IO_ShareEccBuffer& 3 ) ? 0 : 1;
		
		//=========================================================================
		// Get Buffer Pointer
		//=========================================================================
		/* Adapt type of address */
		if ( bAlignAddr )
			pSpareDW	= (unsigned int*)&gTCC_MTD_IO_ShareEccBuffer[0];
		else
			pSpareB		= (unsigned char*)&gTCC_MTD_IO_ShareEccBuffer[0];
		
		//----------------------------------------------
		//	Read Spare Data
		//----------------------------------------------
		i = ( nDevInfo->EccWholeDataSize >> 2 );
		do {
			if ( bAlignAddr )
			{
				*pSpareDW = pMTD_NFC->NFC_WDATA;++pSpareDW;
			}	
			else
			{
				uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
				*pSpareB = uDWordByte.BYTE[0];++pSpareB;
				*pSpareB = uDWordByte.BYTE[1];++pSpareB;
				*pSpareB = uDWordByte.BYTE[2];++pSpareB;
				*pSpareB = uDWordByte.BYTE[3];++pSpareB;
			}
		}while(--i);
	}

	return res;
}


/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locReadSpareData( TCC_MTD_IO_DEVINFO *nDevInfo, U8 *nSpareBuffer, int nPageEccOnOff );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDevInfo	= 
*			nPageEccOnOff	= 
*			nSpareBuffer	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locReadSpareData( TCC_MTD_IO_DEVINFO *nDevInfo, U8 *nSpareBuffer, int nPageEccOnOff )
{
	unsigned int		i;

	unsigned int		nSpareTotalSize;
	unsigned char		bAlignAddr;
	unsigned char		*pSpareB = 0;
	unsigned int		*pSpareDW = 0;
	unsigned char		*pSpareBuffer;
	unsigned int		nECCDataSize = 8;
	
	#ifdef _LINUX_
	unsigned char		nECCBuffer[44]__attribute__((aligned(8)));
	#else
	unsigned char		nECCBuffer[44];
	#endif
	DWORD_BYTE			uDWordByte;
	TCC_MTD_IO_ERROR	res;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	res = (TCC_MTD_IO_ERROR)SUCCESS;
	nSpareTotalSize = ( nDevInfo->BytesPerSector + 20 );

	//=========================================================================
	// Check Align of PageBuffer Address
	//=========================================================================
	bAlignAddr = ( (unsigned int)gTCC_MTD_IO_PreEnDecodeEccBuffer & 3 ) ? 0 : 1;

	TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );
	
	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================	
	if ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	else
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

	//=========================================================================
	// Get Buffer Pointer
	//=========================================================================

	if ( bAlignAddr )
		pSpareDW	= (unsigned int*)gTCC_MTD_IO_PreEnDecodeEccBuffer;
	else
		pSpareB		= (unsigned char*)gTCC_MTD_IO_PreEnDecodeEccBuffer;

	/* Setup ECC Block */
	TCC_MTD_IO_locSetupECC( ECC_ON, ECC_DECODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwDATA_PA );
	pMTD_ECC->ECC_CTRL	|= ( nSpareTotalSize << ECC_SHIFT_DATASIZE );
	pMTD_ECC->ECC_CLEAR	= 0x00000000;		// Clear ECC Block
	
	/* Set Spare Buffer */
	pSpareBuffer = ( bAlignAddr ) ? (unsigned char*)pSpareDW : (unsigned char*)pSpareB;

	//----------------------------------------------
	//	Read 512 (Last PPage) + 20(Yaffs Tags)
	//----------------------------------------------
	
	i = ( nSpareTotalSize >> 2 );
	do {
		if ( bAlignAddr )
		{
			*pSpareDW = pMTD_NFC->NFC_WDATA;++pSpareDW;
		}	
		else
		{
			uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
			*pSpareB = uDWordByte.BYTE[0];++pSpareB;
			*pSpareB = uDWordByte.BYTE[1];++pSpareB;
			*pSpareB = uDWordByte.BYTE[2];++pSpareB;
			*pSpareB = uDWordByte.BYTE[3];++pSpareB;
		}
	}while(--i);
	
	if (( nDevInfo->Feature.MediaType & A_MLC ) || ( nDevInfo->Feature.MediaType & A_SLC ))
		nECCDataSize = 8;
	else if ( ( nDevInfo->Feature.MediaType & A_MLC_8BIT ) || ( nDevInfo->Feature.MediaType & A_MLC_12BIT ) )
		nECCDataSize = 20;
	else if ( nDevInfo->Feature.MediaType & A_MLC_16BIT ) 
		nECCDataSize = 26;
	else if ( nDevInfo->Feature.MediaType & A_MLC_24BIT ) 
		nECCDataSize = 42;
	
	if ( nPageEccOnOff == PAGE_ECC_ON )
	{
		memset( gTCC_MTD_IO_ShareEccBuffer, 0xFF, ( nECCDataSize << nDevInfo->ShiftPPages) );
		//=========================================================================
		// Check Align of PageBuffer Address
		//=========================================================================
		bAlignAddr = ( (unsigned int)&gTCC_MTD_IO_ShareEccBuffer & 3 ) ? 0 : 1;
		
		//=========================================================================
		// Get Buffer Pointer
		//=========================================================================
		/* Adapt type of address */
		if ( bAlignAddr )
			pSpareDW	= (unsigned int*)&gTCC_MTD_IO_ShareEccBuffer[0];
		else
			pSpareB		= (unsigned char*)&gTCC_MTD_IO_ShareEccBuffer[0];
		
		//----------------------------------------------
		//	Read Spare Data
		//----------------------------------------------
		i = ( ( nECCDataSize << nDevInfo->ShiftPPages) >> 2 );
		do {
			if ( bAlignAddr )
			{
				*pSpareDW = pMTD_NFC->NFC_WDATA;++pSpareDW;
			}	
			else
			{
				uDWordByte.DWORD = pMTD_NFC->NFC_WDATA;
				*pSpareB = uDWordByte.BYTE[0];++pSpareB;
				*pSpareB = uDWordByte.BYTE[1];++pSpareB;
				*pSpareB = uDWordByte.BYTE[2];++pSpareB;
				*pSpareB = uDWordByte.BYTE[3];++pSpareB;
			}
		}while(--i);
		
	}
	memcpy(nECCBuffer, &gTCC_MTD_IO_ShareEccBuffer[((nDevInfo->UsablePPages- 1)*nDevInfo->EccDataSize)], nDevInfo->EccDataSize );

	//----------------------------------------------
	// ECC Correction 512(Last PPage ) + 20 ( Yaffs Tag)
	//----------------------------------------------
	if ( ( nECCBuffer[0] == 0x00 ) && ( nECCBuffer[1] == 0x00 ) &&
		 ( nECCBuffer[2] == 0x00 ) && ( nECCBuffer[3] == 0x00 ) &&
		 ( nECCBuffer[4] == 0x00 ) && ( nECCBuffer[5] == 0x00 ) &&
		 ( nECCBuffer[6] == 0x00 ) && ( nECCBuffer[7] == 0x00 ))
	{
		res |= TCC_MTD_IO_locCorrectionMLC( nDevInfo->EccType, pSpareBuffer, nECCBuffer, nSpareTotalSize );
		res |= ERR_TCC_MTD_IO_FAILED_CORRECTION_MLC_ECC;
	}
	else if ( memcmp( nECCBuffer, gTCC_MTD_IO_TempBuffer, nDevInfo->EccDataSize ) == 0 )
 	{
		#if defined(TCC89XX) || defined(TCC92XX)
		ASM_NOP;
		#elif defined(TCC93XX) || defined(TCC88XX)
#if 0
		BITSET( pMTD_ECC->ECC_CTRL, HwECC_CTRL_CORPASS_EN );
		pMTD_ECC->ECC_CODE0		= 0;
		pMTD_ECC->ECC_CODE1		= 0;
		pMTD_ECC->ECC_CODE2		= 0;
		pMTD_ECC->ECC_CODE3		= 0;
		pMTD_ECC->ECC_CODE4		= 0;
		pMTD_ECC->ECC_CODE5		= 0;
		pMTD_ECC->ECC_CODE6		= 0;
		pMTD_ECC->ECC_CODE7		= 0;
		pMTD_ECC->ECC_CODE8		= 0;
		pMTD_ECC->ECC_CODE9		= 0;
		pMTD_ECC->ECC_CODE10	= 0;
		BITCLR( pMTD_ECC->ECC_CTRL, HwECC_CTRL_CORPASS_EN );
#else
		res |= TCC_MTD_IO_locCorrectionMLC( nDevInfo->EccType, pSpareBuffer, nECCBuffer, nSpareTotalSize );	
		res = SUCCESS;
#endif 
		#endif
	}
	else
	{
		res |= TCC_MTD_IO_locCorrectionMLC( nDevInfo->EccType, pSpareBuffer, nECCBuffer, nSpareTotalSize );	
	}

	memcpy( &nSpareBuffer[0], &gTCC_MTD_IO_PreEnDecodeEccBuffer[nDevInfo->BytesPerSector], 20);

	return SUCCESS;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locWrite512Data( U16 nStartPPage, U16 nWritePPSize, U8 *nPageBuffer, U8 *nSpareBuffer );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nPageBuffer	= 
*			nSpareBuffer	= 
*			nStartPPage	= 
*			nWritePPSize	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locWrite512DataDoubleBuf(struct mtd_info *mtd, U16 nStartPPage, U16 nWritePPSize, U8 *nPageBuffer, U8 *nSpareBuffer )
{
	struct nand_chip *tcc_nand = mtd->priv;
	struct tcc_nand_info *info = tcc_nand->priv;
	unsigned int		i, j, k;
	unsigned char		bAlignAddr;
	unsigned int		ColumnAddr;
	unsigned int		nEccOnOff = ECC_ON;
	unsigned char		nECCBuffer[44]__attribute__((aligned(8)));	
	unsigned char		*pPageB = 0, *pSpareB = 0;
	unsigned int		*pPageDW = 0;
	unsigned char		*pDataBuffer;
//	unsigned char		nDummyPageBuffer[512];
	unsigned char		*pEccB, *pSpare;
	#ifdef NAND_IO_USE_MCU_ACCESS
	DWORD_BYTE			uDWordByte;
	#endif
	TCC_MTD_IO_ECC_INFO	*pECC_Info;
	TCC_MTD_IO_DEVINFO	*nDevInfo;
	TCC_MTD_IO_ERROR	res;

	nDevInfo 		= &gDevInfo[info->glue_info.gTCC_MTD_CurrCS];
	
	if ( ( nStartPPage + nWritePPSize ) > nDevInfo->UsablePPages )
		return ERR_TCC_MTD_IO_WRONG_PARAMETER;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	res 			= (TCC_MTD_IO_ERROR)SUCCESS;

	//=========================================================================
	// Check Align of PageBuffer Address
	//=========================================================================
	bAlignAddr		= ( (unsigned int)nPageBuffer & 3 ) ? 0 : 1;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	if ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	else
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

	TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );

	//=========================================================================
	// Get Buffer Pointer
	//=========================================================================
	/* Adapt type of address */
	if ( bAlignAddr )
		pPageDW		= (unsigned int*)nPageBuffer;
	else
		pPageB		= (unsigned char*)nPageBuffer;

	// Set SpareBuffer Pointer =>> ECCBuffer
	if ( ( nDevInfo->Feature.MediaType  & A_BIG ) || (( nDevInfo->Feature.MediaType  & A_SMALL ) && ( nDevInfo->Feature.MediaType  & A_PARALLEL )))
	{
		memset( gTCC_MTD_IO_ShareEccBuffer, 0xFF, nDevInfo->EccWholeDataSize );
		pSpareB		= (unsigned char*)&gTCC_MTD_IO_ShareEccBuffer[0];
	}
	else
	{
		pSpareB		= (unsigned char*)nSpareBuffer;
		pSpareB 	+= 8;
	}

	//=========================================================================
	// Empty Page ECCBuffer Pointer Increment
	//=========================================================================
	for ( j = 0; j < nStartPPage; ++j )
	{
		if ( nDevInfo->EccType == MLC_ECC_4BIT_TYPE )
			pECC_Info = &gMLC_ECC_4Bit;
		else if ( nDevInfo->EccType == MLC_ECC_8BIT_TYPE )
			pECC_Info = &gMLC_ECC_8Bit;
		else if ( nDevInfo->EccType == MLC_ECC_12BIT_TYPE )
			pECC_Info = &gMLC_ECC_12Bit;
		else if ( nDevInfo->EccType == MLC_ECC_14BIT_TYPE )
			pECC_Info = &gMLC_ECC_14Bit;
		else if ( nDevInfo->EccType == MLC_ECC_16BIT_TYPE )
			pECC_Info = &gMLC_ECC_16Bit;
		else if ( nDevInfo->EccType == MLC_ECC_24BIT_TYPE )
			pECC_Info = &gMLC_ECC_24Bit;
		else
			return ERR_TCC_MTD_IO_WRONG_PARAMETER;

		if ( ( nDevInfo->Feature.MediaType  & A_MLC_16BIT ) || ( nDevInfo->Feature.MediaType  & A_MLC_24BIT ) )
		{
			pSpare = (unsigned char*)pSpareB;
			
			for ( i = 0; i < nDevInfo->EccDataSize; ++i )
				pSpare[i] = pECC_Info->All_FF_512_ECC_Code[i];
		}
		else
		{
			memcpy(	(void*)pSpareB, (void*)pECC_Info->All_FF_512_ECC_Code, nDevInfo->EccDataSize);			
		}

		pSpareB += nDevInfo->EccDataSize;
	}

	//----------------------------------------------
	//	Write Data as 512Bytes repeatly
	//----------------------------------------------
	for ( j = 0; j < nWritePPSize; ++j )
	{
		/* Get Data Buffer */
		pDataBuffer = ( bAlignAddr ) ? (unsigned char*)pPageDW : (unsigned char*)pPageB;

		//####################################################
		//#	Write 512 Page Data
		//####################################################
		//----------------------------------------------
		//	MCU ACCESS
		//----------------------------------------------
		#if defined( NAND_IO_USE_MCU_ACCESS )
		#if defined(TCC89XX)  || defined(TCC92XX)
		if (!( pMTD_NFC->NFC_CTRL1 & Hw30 ))
		#elif defined(TCC93XX)  || defined(TCC88XX)
		if (!( pMTD_NFC->NFC_CTRL & HwNFC_CTRL_DACK_EN ))
		#endif 
		{
		    if ( nEccOnOff == ECC_ON )
		    {
			    /* Setup ECC Block */
			    TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
			    pMTD_ECC->ECC_CTRL	|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
			    pMTD_ECC->ECC_CLEAR	= 0x00000000;
		    }
    
		    /* Write 512 Data Area */
		    BITCSET( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_1 );	// 1R/W Burst Size
		    pMTD_NFC->NFC_DSIZE		= nDevInfo->BytesPerSector;
			#if defined(TCC89XX) || defined(TCC92XX) 
		    pMTD_NFC->NFC_IREQ 		= 0x77;	// pMTD_NFC->NFC_IREQ_FLAG1;
		    #elif defined(TCC93XX)  || defined(TCC88XX)
			pMTD_NFC->NFC_CTRL 	 |= HwNFC_CTRL_PRSEL_P;
			pMTD_NFC->NFC_IRQCFG |= HwNFC_IRQCFG_PIEN;
			#endif

			#if defined(TCC89XX) || defined(TCC92XX) 	
			TCC_MTD_IO_IRQ_Mask();
		    pMTD_NFC->NFC_PSTART 	= 0;
			#elif defined(TCC93XX)  || defined(TCC88XX)
			TCC_MTD_IO_IRQ_Mask();
			pMTD_NFC->NFC_PRSTART = 0; 
			#endif
		    
		    i = ( nDevInfo->BytesPerSector >> 2);
		    do {
				#if defined(TCC89XX) || defined(TCC92XX)
			    while (!( pMTD_NFC->NFC_CTRL & HwNFC_CTRL_FS_RDY ));
				#elif defined(TCC93XX) || defined(TCC88XX)

				#if 1//def FOR_LOOP
				for( k = 0; k < 10000000; k++ )
				{
					if ( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY )
						break;

					ASM_NOP;
				}

				if( !( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY ) )
				{
					printk(" !!! MTD Write Fail [HwNFC_IRQ_PFG]!!!\n ReWrite Attempt [Line:%d]\n", __LINE__);		
					TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pSpareB );
					return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
				}
				#else
				while (!( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY ));
				#endif
				
				#endif
				
			    if ( bAlignAddr )
			    {
				    pMTD_NFC->NFC_LDATA = *pPageDW;++pPageDW;
			    }	
			    else
			    {
				    uDWordByte.BYTE[0] = *pPageB;++pPageB;
				    uDWordByte.BYTE[1] = *pPageB;++pPageB;
				    uDWordByte.BYTE[2] = *pPageB;++pPageB;
				    uDWordByte.BYTE[3] = *pPageB;++pPageB;
				    pMTD_NFC->NFC_LDATA	= uDWordByte.DWORD;
			    }
		    }while(--i);

			#if defined(TCC89XX) || defined(TCC92XX) 
		    while (ISZERO( pMTD_NFC->NFC_IREQ, HwNFC_IREQ_FLAG1 ));
			TCC_MTD_IO_IRQ_UnMask();
			#elif defined(TCC93XX)  || defined(TCC88XX)

			#if 1 //def FOR_LOOP
			for( k = 0; k < 10000000; k++ )
			{
				if ( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG )
					break;

				ASM_NOP;
			}

			if( !( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG ) )
			{
				printk(" !!! MTD Write Fail [HwNFC_IRQ_PFG]!!!\n ReWrite Attempt [Line:%d]\n", __LINE__);			
				TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pSpareB );
				return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
			}
			#else
			while (ISZERO( pMTD_NFC->NFC_IRQ, HwNFC_IRQ_PFG));
			#endif
			
			pMTD_NFC->NFC_IRQ 	|= HwNFC_IRQ_PFG;
			TCC_MTD_IO_IRQ_UnMask();
			#endif
		}
		else
		{
			if ( nEccOnOff == ECC_ON )
			{
				/* Setup ECC Block */
				TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwDATA_PA );
				pMTD_ECC->ECC_CTRL	|= ( nDevInfo->BytesPerSector<< ECC_SHIFT_DATASIZE );
				pMTD_ECC->ECC_CLEAR	= 0x00000000;
			}

			/* Write 512 Data Area */
			i = ( nDevInfo->BytesPerSector >> 2 );
			do {				
				if ( bAlignAddr )
				{
					pMTD_NFC->NFC_WDATA = *pPageDW;++pPageDW;
				}	
				else
				{
					uDWordByte.BYTE[0] = *pPageB;++pPageB;
					uDWordByte.BYTE[1] = *pPageB;++pPageB;
					uDWordByte.BYTE[2] = *pPageB;++pPageB;
					uDWordByte.BYTE[3] = *pPageB;++pPageB;
					pMTD_NFC->NFC_WDATA	= uDWordByte.DWORD;
				}
			}while(--i);	
		}
		//----------------------------------------------
		//	DMA ACCESS
		////----------------------------------------------
		#elif defined( NAND_IO_USE_DMA_ACCESS )
		if ( nEccOnOff == ECC_ON )
		{
			/* Setup ECC Block */
			TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
			pMTD_ECC->ECC_CTRL	|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
			pMTD_ECC->ECC_CLEAR	= 0x00000000;
		}

		if ( j == 0 )
			memcpy( gpMTD_DMA_WorkBuffer1, pDataBuffer, nDevInfo->BytesPerSector );

		TCC_MTD_IO_SetupDMADoubleBuf( NAND_IO_DMA_WRITE, j, nDevInfo->BytesPerSector );

		#if defined(TCC89XX) || defined(TCC92XX)
		if ( pMTD_NFC->NFC_CTRL1 & Hw31 )
			BITCLR( pMTD_NFC->NFC_CTRL1, Hw31 );
		#endif
		TCC_MTD_IO_IRQ_Mask();		
		#if defined(TCC89XX) || defined(TCC92XX)
		pMTD_NFC->NFC_PSTART	= 0;
		#elif defined(TCC93XX) || defined(TCC88XX)
		pMTD_NFC->NFC_PRSTART	= 0;
		#endif

		if ( j != (unsigned int)( nWritePPSize - 1 ) ) 
		{
			if ( j & 1 )
				memcpy( gpMTD_DMA_WorkBuffer1, (void *)(pDataBuffer + nDevInfo->BytesPerSector), nDevInfo->BytesPerSector );
			else
				memcpy( gpMTD_DMA_WorkBuffer0, (void *)(pDataBuffer + nDevInfo->BytesPerSector), nDevInfo->BytesPerSector );
		}

		#if defined(TCC89XX)|| defined(TCC92XX)
		while (ISZERO( pMTD_NFC->NFC_IREQ, HwNFC_IREQ_FLAG1 ));
		#endif
		
		#if defined(TCC89XX) || defined(TCC92XX)
		TCC_MTD_IO_IRQ_UnMask();		
		if ( pMTD_NFC->NFC_CTRL1 & Hw30 )
			BITSET( pMTD_NFC->NFC_CTRL1, Hw31 );
		#elif defined(TCC93XX) || defined(TCC88XX)

		#if 1 //def FOR_LOOP
		for( k = 0; k < 10000000; k++ )
		{
			if( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG )
				break;

			ASM_NOP;
		}

		if( !( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG ) )
		{
			printk(" !!! MTD Write Fail [HwNFC_IRQ_PFG]!!!\n ReWrite Attempt [Line:%d]\n", __LINE__);
			TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pSpareB );
			return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
		}
		
		#else
		while (ISZERO( pMTD_NFC->NFC_IRQ, HwNFC_IRQ_PFG));
		#endif
		
		pMTD_NFC->NFC_IRQ 	|= HwNFC_IRQ_PFG;
		TCC_MTD_IO_IRQ_UnMask();		
		#endif
	
		if ( bAlignAddr )
			pPageDW += ( nDevInfo->BytesPerSector >> 2);
		else
			pPageB += nDevInfo->BytesPerSector;
		#endif
		//####################################################
		//####################################################

		/*	Load ECC code from ECC block */
		if ( nEccOnOff == ECC_ON )
		{
			if ( ( nDevInfo->Feature.MediaType  & A_MLC_16BIT ) || ( nDevInfo->Feature.MediaType  & A_MLC_24BIT ) )
			{
				pSpare = (unsigned char*)pSpareB;
				pEccB = (unsigned char*)nECCBuffer;
				res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pEccB );
				for ( i = 0; i < nDevInfo->EccDataSize; ++i )
					pSpare[i] = pEccB[i];
			}
			else
			{
				res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pSpareB );				
			}

			if ( res != SUCCESS )
				goto ErrorWrite512Data;
		}

		pSpareB += nDevInfo->EccDataSize;	
	}

	//=========================================================================
	// Empty PPage Write
	//=========================================================================
	if ( ( nStartPPage + nWritePPSize ) != nDevInfo->UsablePPages )
	{
		if ( nDevInfo->Feature.MediaType & A_BIG )
		{
			/* Change Cycle */
			TCC_MTD_IO_locSetCommCycleTime();
			
			//---------------------------------
			// Random Data Input [ 0x85 ]
			//---------------------------------
			pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0x8585;

			//---------------------------------
			// Write Column Address
			//---------------------------------
			ColumnAddr = nDevInfo->Feature.PageSize;
			ColumnAddr	= ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT ) ? (ColumnAddr >> 1) : ColumnAddr;
			TCC_MTD_IO_locWriteColAddr( ColumnAddr );

			//---------------------------------
			// ECC Data Clear
			//---------------------------------
			for ( j = 0; j < (U16)( nDevInfo->UsablePPages - ( nStartPPage + nWritePPSize ) ); ++j )
			{
				if ( nDevInfo->EccType == MLC_ECC_4BIT_TYPE )
					pECC_Info = &gMLC_ECC_4Bit;
				else if ( nDevInfo->EccType == MLC_ECC_8BIT_TYPE )
					pECC_Info = &gMLC_ECC_8Bit;
				else if ( nDevInfo->EccType == MLC_ECC_12BIT_TYPE )
					pECC_Info = &gMLC_ECC_12Bit;
				else if ( nDevInfo->EccType == MLC_ECC_14BIT_TYPE )
					pECC_Info = &gMLC_ECC_14Bit;
				else if ( nDevInfo->EccType == MLC_ECC_16BIT_TYPE )
					pECC_Info = &gMLC_ECC_16Bit;
				else if ( nDevInfo->EccType == MLC_ECC_24BIT_TYPE )
					pECC_Info = &gMLC_ECC_24Bit;
				else
					return ERR_TCC_MTD_IO_WRONG_PARAMETER;

				if ( ( nDevInfo->Feature.MediaType  & A_MLC_16BIT ) || ( nDevInfo->Feature.MediaType  & A_MLC_24BIT ) )
				{
					pSpare = (unsigned char*)pSpareB;
					
					for ( i = 0; i < nDevInfo->EccDataSize; ++i )
						pSpare[i] = pECC_Info->All_FF_512_ECC_Code[i];
				}
				else
				{
					memcpy(	(void*)pSpareB, (void*)pECC_Info->All_FF_512_ECC_Code, nDevInfo->EccDataSize);			
				}
				
				pSpareB += nDevInfo->EccDataSize;	
			}
		}
		else
		{

			//Write Dummy Data
			for ( j = 0; j < (U16)( nDevInfo->UsablePPages - ( nStartPPage + nWritePPSize ) ); ++j )
			{
				//####################################################
				//#	Write 512 Page Data
				//####################################################
				//----------------------------------------------
				//	MCU ACCESS
				//----------------------------------------------
				#if defined( NAND_IO_USE_MCU_ACCESS )
				/* Setup ECC Block */
				/*
				#if defined(TCC89XX)  || defined(TCC92XX)
				if (!( pMTD_NFC->NFC_CTRL1 & Hw30 ))
				#elif defined(TCC93XX)
				if (!( pMTD_NFC->NFC_CTRL & HwNFC_CTRL_DACK_EN ))
				#endif
				{*/
					if ( nEccOnOff == ECC_ON )
					{
						pMTD_NFC->NFC_CTRL		|= HwNFC_CTRL_ECCON_EN;		
						
						TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
						pMTD_ECC->ECC_CTRL	|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );		// Data Size...
						pMTD_ECC->ECC_CLEAR	= 0x00000000;
					}

					/* Write 512 Data Area */
					BITCSET( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_1 );	// 1R/W Burst Size
					#if defined(TCC89XX) || defined(TCC92XX)
					pMTD_NFC->NFC_DSIZE	= nDevInfo->BytesPerSector;
					pMTD_NFC->NFC_IREQ = 0x77;	// pMTD_NFC->NFC_IREQ_FLAG1;		
					TCC_MTD_IO_IRQ_Mask();
					pMTD_NFC->NFC_PSTART = 0;
					#elif defined(TCC93XX) || defined(TCC88XX)
					pMTD_NFC->NFC_DSIZE	= nDevInfo->BytesPerSector;
					TCC_MTD_IO_IRQ_Mask();
					pMTD_NFC->NFC_PRSTART = 0;
					#endif
					
					i = ( nDevInfo->BytesPerSector >> 2);
					do
					{
						#if defined(TCC89XX) || defined(TCC92XX)
						while (!( pMTD_NFC->NFC_CTRL & HwNFC_CTRL_FS_RDY ));
						#elif defined(TCC93XX) || defined(TCC88XX)

						#if 1 //def FOR_LOOP
						for( k = 0; k < 10000000; k++ )
						{
							if( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY )
								break;

							ASM_NOP;
						}

						if( !(pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY) )
						{
							return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
						}
						#else
						while (!( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY ));
						#endif
						
						#endif
						pMTD_NFC->NFC_LDATA = 0xFFFFFFFF;
					}while(--i);

					#if defined(TCC89XX) || defined(TCC92XX) 
					while (ISZERO( pMTD_NFC->NFC_IREQ, HwNFC_IREQ_FLAG1 ));					
					TCC_MTD_IO_IRQ_UnMask();
					#elif defined(TCC93XX) || defined(TCC88XX)

					#if 1//def FOR_LOOP
					for( k = 0; k < 10000000; k++ )
					{
						if( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG )
							break;

						ASM_NOP;
					}

					if( !(pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG) )
					{
						return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
					}
					#else
					while (ISZERO( pMTD_NFC->NFC_IRQ, HwNFC_IRQ_PFG));
					#endif
					
					pMTD_NFC->NFC_IRQ 	|= HwNFC_IRQ_PFG;
					TCC_MTD_IO_IRQ_UnMask();
					#endif

					#endif
					/*
				}
				else
				{
					if ( nEccOnOff == ECC_ON )
					{
						TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwDATA_PA );
						pMTD_ECC->ECC_CTRL	|= ( 512 << ECC_SHIFT_DATASIZE );		// Data Size...
						pMTD_ECC->ECC_CLEAR	= 0x00000000;
					}


					i = 128;
					do
					{
						pMTD_NFC->NFC_WDATA = 0xFFFFFFFF;
					}while(--i);					
				}
				
				*/				
				//----------------------------------------------
				//	DMA ACCESS
				//----------------------------------------------
				//#elif defined( NAND_IO_USE_DMA_ACCESS )
				///* Setup ECC Block */
				//if ( nEccOnOff == ECC_ON )
				//{
				//	#if defined(_WINCE_) || defined(_LINUX_)
				//	NAND_IO_SetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
				//	#else
				//	NAND_IO_SetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&pMTD_NFC->NFC_LDATA );
				//	#endif
				//	pMTD_ECC->ECC_CTRL	|= ( 512 << ECC_SHIFT_DATASIZE );		// Data Size...
				//	pMTD_ECC->ECC_CLEAR	= 0x00000000;
				//}
				//
				//#if defined(_LINUX_) || defined(_WINCE_)
				//NAND_IO_SetupDMA( (void*)nDummyPageBuffer, 4, 0,
				//				  (void*)&NAND_IO_HwLDATA_PA, 0, 0,
				//				  NAND_IO_DMA_WRITE );
				//#else
				//NAND_IO_SetupDMA( (void*)nDummyPageBuffer, 4, 0,
				//				  (void*)&pMTD_NFC->NFC_LDATA, 0, 0,
				//				  NAND_IO_DMA_WRITE );
				//#endif
				//
				//#endif
				//####################################################
				//####################################################
				/*	Load ECC code from ECC block */
				if ( nEccOnOff == ECC_ON )
				{
					if ( ( nDevInfo->Feature.MediaType  & A_MLC_16BIT ) || ( nDevInfo->Feature.MediaType  & A_MLC_24BIT ) )
					{
						pSpare = (unsigned char*)pSpareB;
						pEccB = (unsigned char*)nECCBuffer;
						res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pEccB );
						for ( i = 0; i < nDevInfo->EccDataSize; ++i )
							pSpare[i] = pEccB[i];
					}
					else
					{
						res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pSpareB );
					}
					
					if ( res != SUCCESS )
						goto ErrorWrite512Data;
				}

				pSpareB += nDevInfo->EccDataSize;		
			}			
		}
	}
	
	//=========================================================================
	// Write Spare Data
	//=========================================================================
	/* Change Cycle */
	TCC_MTD_IO_locSetWriteCycleTime();

	TCC_MTD_IO_locWriteSpareData( nDevInfo, nSpareBuffer, PAGE_ECC_ON );

	//=========================================================================
	// Return
	//=========================================================================

ErrorWrite512Data:
	return res;

}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locWrite512Data( U16 nStartPPage, U16 nWritePPSize, U8 *nPageBuffer, U8 *nSpareBuffer );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nPageBuffer	= 
*			nSpareBuffer	= 
*			nStartPPage	= 
*			nWritePPSize	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locWrite512Data( struct mtd_info *mtd, U16 nStartPPage, U16 nWritePPSize, U8 *nPageBuffer, U8 *nSpareBuffer )
{	
	struct nand_chip *tcc_nand = mtd->priv;
	struct tcc_nand_info *info = tcc_nand->priv;
	
	TCC_MTD_IO_ERROR	res;
	
#if defined(__USE_MTD_NAND_ISR__)
	if(TCC_MTD_IO_IRQ_IsEnabled())
	{
		unsigned int		nEccOnOff = ECC_ON;
		unsigned char		*pSpareB;
		unsigned int		i;
		int					ret;
		TCC_MTD_IO_DEVINFO	*nDevInfo;	

		nDevInfo = &gDevInfo[info->glue_info.gTCC_MTD_CurrCS];

		//=========================================================================
		// DATA BUS WIDTH Setting
		//=========================================================================
		if ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )
			TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
		else
			TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

		if ( nEccOnOff == ECC_ON )
			TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );

		//=========================================================================
		// Set SpareBuffer Pointer =>> ECCBuffer
		//=========================================================================
		if ( ( nDevInfo->Feature.MediaType  & A_BIG )
			|| (( nDevInfo->Feature.MediaType  & A_SMALL ) && ( nDevInfo->Feature.MediaType  & A_PARALLEL )))
		{
			memset( gTCC_MTD_IO_ShareEccBuffer, 0xFF, nDevInfo->EccWholeDataSize );
			pSpareB		= (unsigned char*)&gTCC_MTD_IO_ShareEccBuffer[0];
		}
		else
		{
			pSpareB		= (unsigned char*)nSpareBuffer;
			pSpareB 	+= NAND_IO_SPARE_SIZE_SMALL;
		}

		//=========================================================================
		// Empty Page ECCBuffer Pointer Increment
		//=========================================================================
		for ( i = 0; i < nStartPPage; ++i )
		{
			TCC_MTD_IO_ECC_INFO *pECC_Info;

			if ( nDevInfo->EccType == MLC_ECC_4BIT_TYPE )
				pECC_Info = &gMLC_ECC_4Bit;
			else if ( nDevInfo->EccType == MLC_ECC_8BIT_TYPE )
				pECC_Info = &gMLC_ECC_8Bit;
			else if ( nDevInfo->EccType == MLC_ECC_12BIT_TYPE )
				pECC_Info = &gMLC_ECC_12Bit;
			else if ( nDevInfo->EccType == MLC_ECC_14BIT_TYPE )
				pECC_Info = &gMLC_ECC_14Bit;
			else if ( nDevInfo->EccType == MLC_ECC_16BIT_TYPE )
				pECC_Info = &gMLC_ECC_16Bit;
			else if ( nDevInfo->EccType == MLC_ECC_24BIT_TYPE )
				pECC_Info = &gMLC_ECC_24Bit;
			else
				return ERR_TCC_MTD_IO_WRONG_PARAMETER;

			memcpy(	(void*)pSpareB, (void*)pECC_Info->All_FF_512_ECC_Code, nDevInfo->EccDataSize);
			
			pSpareB += nDevInfo->EccDataSize;
		}

		sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo	= nDevInfo;
		sTCC_MTD_IO_NandIsrInfo.pbPageBuffer 	= nPageBuffer;
		sTCC_MTD_IO_NandIsrInfo.pbEccBuffer 	= pSpareB;
		sTCC_MTD_IO_NandIsrInfo.pbSpareBuffer 	= nSpareBuffer;
		sTCC_MTD_IO_NandIsrInfo.usCurrentPPage	= nStartPPage;
		sTCC_MTD_IO_NandIsrInfo.usPPagesLeft 	= nWritePPSize;
		sTCC_MTD_IO_NandIsrInfo.iEccOnOff 		= nEccOnOff;
		sTCC_MTD_IO_NandIsrInfo.wait_complete 	= 0;
		sTCC_MTD_IO_NandIsrInfo.uiState 		= TCC_MTD_IO_IRQ_STATE_WRITE;
		sTCC_MTD_IO_NandIsrInfo.error 			= SUCCESS;
		sTCC_MTD_IO_NandIsrInfo.ubIsRun			= TRUE;		

		if(sTCC_MTD_IO_NandIsrInfo.usPPagesLeft>0)
		{
			memcpy(gpMTD_DMA_WorkBuffer0,sTCC_MTD_IO_NandIsrInfo.pbPageBuffer,nDevInfo->BytesPerSector );
			sTCC_MTD_IO_NandIsrInfo.pbPageBuffer = &sTCC_MTD_IO_NandIsrInfo.pbPageBuffer[nDevInfo->BytesPerSector];
			sTCC_MTD_IO_NandIsrInfo.usPPagesLeft--;
		}
		else
		{
			// Null Sector Write
			memset(gpMTD_DMA_WorkBuffer0,0xFF,nDevInfo->BytesPerSector );
		}
		sTCC_MTD_IO_NandIsrInfo.usCurrentPPage++;

		if ( sTCC_MTD_IO_NandIsrInfo.iEccOnOff == ECC_ON )
		{
			/* Setup ECC Block */
			#if defined(_WINCE_) || defined(_LINUX_)
			TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
			#else
			TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&pNFC->NFC_LDATA );
			#endif
			pMTD_ECC->ECC_CTRL	|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
			pMTD_ECC->ECC_CLEAR	= 0x00000000;
		}

		TCC_MTD_IO_IRQ_SetupDMA(gpMTD_DMA_PhyBuffer0, NAND_IO_DMA_WRITE, sTCC_MTD_IO_NandIsrInfo.pNandIoDevInfo->BytesPerSector);
		
		do
		{
			ret = wait_event_interruptible(sTCC_MTD_IO_NandIsrInfo.wait_q, sTCC_MTD_IO_NandIsrInfo.wait_complete);
			if( ret == 0)			
			{
				// ok
				res = sTCC_MTD_IO_NandIsrInfo.error;
			}
			else if ( abs(ret) == ERESTARTSYS )
			{
				printk("[TCC_MTD] Write Op. RE Sleep !! \n");
			}
			else
			{
				// fail
				printk("[TCC_MTD] mtd:wait_event_interruptible error!(ret=%d)\n",ret);
				res = ERR_TCC_MTD_IO_FAILED_WRITE;
			}	
		} while( !sTCC_MTD_IO_NandIsrInfo.wait_complete );

		sTCC_MTD_IO_NandIsrInfo.uiState = TCC_MTD_IO_IRQ_STATE_NONE;
		sTCC_MTD_IO_NandIsrInfo.ubIsRun	= FALSE;

		if(res == SUCCESS)
			TCC_MTD_IO_locWriteSpareData( nDevInfo, nSpareBuffer, PAGE_ECC_ON );

//BITCLR(pGPIO->GPFDAT, Hw16);
	}
	else
#endif //defined(__USE_NAND_ISR__) && defined(_LINUX_)
	{
		unsigned int		i, j, k;
		unsigned char		bAlignAddr;
		unsigned int		ColumnAddr;
		unsigned int		nECCDataSize = 8;
		unsigned int		nEccOnOff = ECC_ON;
		unsigned char		nECCBuffer[44]__attribute__((aligned(8)));
		unsigned char		*pPageB = 0, *pSpareB = 0;
		unsigned int		*pPageDW = 0;
		unsigned char		*pDataBuffer;
		unsigned char		*pEccB, *pSpare;
	//	unsigned char		nDummyPageBuffer[512];
		DWORD_BYTE			uDWordByte;
		TCC_MTD_IO_ECC_INFO	*pECC_Info;
		TCC_MTD_IO_DEVINFO	*nDevInfo;

		nDevInfo = &gDevInfo[info->glue_info.gTCC_MTD_CurrCS];
		
		if ( ( nStartPPage + nWritePPSize ) > nDevInfo->UsablePPages )
			return ERR_TCC_MTD_IO_WRONG_PARAMETER;

		//=========================================================================
		// Initial Setting
		//=========================================================================
		res = (TCC_MTD_IO_ERROR)SUCCESS;

		//=========================================================================
		// Check Align of PageBuffer Address
		//=========================================================================
		bAlignAddr		= ( (unsigned int)nPageBuffer & 3 ) ? 0 : 1;

		//=========================================================================
		// DATA BUS WIDTH Setting
		//=========================================================================
		if ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )
			TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
		else
			TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

		TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );

		//=========================================================================
		// Get Buffer Pointer
		//=========================================================================
		/* Adapt type of address */
		if ( bAlignAddr )
			pPageDW		= (unsigned int*)nPageBuffer;
		else
			pPageB		= (unsigned char*)nPageBuffer;

		if (( nDevInfo->Feature.MediaType & A_MLC ) ||( nDevInfo->Feature.MediaType & A_SLC ))
			nECCDataSize = 8;
		else if ( ( nDevInfo->Feature.MediaType & A_MLC_8BIT ) || ( nDevInfo->Feature.MediaType & A_MLC_12BIT ) )
			nECCDataSize = 20;
		else if ( nDevInfo->Feature.MediaType & A_MLC_16BIT )
			nECCDataSize = 26;
		else if ( nDevInfo->Feature.MediaType & A_MLC_24BIT )
			nECCDataSize = 42;
		
		// Set SpareBuffer Pointer =>> ECCBuffer
		if ( ( nDevInfo->Feature.MediaType  & A_BIG ) || (( nDevInfo->Feature.MediaType  & A_SMALL ) && ( nDevInfo->Feature.MediaType  & A_PARALLEL )))
		{
			memset( gTCC_MTD_IO_ShareEccBuffer, 0xFF, nDevInfo->EccWholeDataSize );
			pSpareB		= (unsigned char*)&gTCC_MTD_IO_ShareEccBuffer[0];
		}
		else
		{
			pSpareB		= (unsigned char*)nSpareBuffer;
			pSpareB 	+= NAND_IO_SPARE_SIZE_SMALL;
		}

		//=========================================================================
		// Empty Page ECCBuffer Pointer Increment
		//=========================================================================
		for ( j = 0; j < nStartPPage; ++j )
		{
			if ( nDevInfo->EccType == MLC_ECC_4BIT_TYPE )
				pECC_Info = &gMLC_ECC_4Bit;
			#if 0 //TCC93XX TODO 6Bit ECC 
			else if ( nDevInfo->EccType == MLC_ECC_6BIT_TYPE )
				pECC_Info = &gMLC_ECC_6Bit;
			#endif
			else if ( nDevInfo->EccType == MLC_ECC_8BIT_TYPE )
				pECC_Info = &gMLC_ECC_8Bit;
			else if ( nDevInfo->EccType == MLC_ECC_12BIT_TYPE )
				pECC_Info = &gMLC_ECC_12Bit;
			else if ( nDevInfo->EccType == MLC_ECC_14BIT_TYPE )
				pECC_Info = &gMLC_ECC_14Bit;
			else if ( nDevInfo->EccType == MLC_ECC_16BIT_TYPE )
				pECC_Info = &gMLC_ECC_16Bit;
			else if ( nDevInfo->EccType == MLC_ECC_24BIT_TYPE )
				pECC_Info = &gMLC_ECC_24Bit;
			else
				return ERR_TCC_MTD_IO_WRONG_PARAMETER;

			if ( ( nDevInfo->Feature.MediaType  & A_MLC_16BIT ) || ( nDevInfo->Feature.MediaType  & A_MLC_24BIT ) )
			{
				pSpare = (unsigned char*)pSpareB;
				
				for ( i = 0; i < nECCDataSize; ++i )
					pSpare[i] = pECC_Info->All_FF_512_ECC_Code[i];
			}
			else
			{
				memcpy(	(void*)pSpareB, (void*)pECC_Info->All_FF_512_ECC_Code, nDevInfo->EccDataSize);			
			}
			pSpareB += nECCDataSize;
		}

		//----------------------------------------------
		//	Write Data as 512Bytes repeatly
		//----------------------------------------------
		for ( j = 0; j < nWritePPSize; ++j )
		{
			/* Get Data Buffer */
			pDataBuffer = ( bAlignAddr ) ? (unsigned char*)pPageDW : (unsigned char*)pPageB;

			//####################################################
			//#	Write 512 Page Data
			//####################################################
			//----------------------------------------------
			//	MCU ACCESS
			//----------------------------------------------
			//#if defined( NAND_IO_USE_MCU_ACCESS )
			#if defined(TCC89XX) || defined(TCC92XX)
			if (!( pMTD_NFC->NFC_CTRL1 & Hw30 ))
			#elif defined(TCC93XX)  || defined(TCC88XX)
			if (!( pMTD_NFC->NFC_CTRL & HwNFC_CTRL_DACK_EN ))
			#endif		
			{
				if ( nEccOnOff == ECC_ON )
				{
					/* Setup ECC Block */
					TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
					pMTD_ECC->ECC_CTRL	|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
					pMTD_ECC->ECC_CLEAR	= 0x00000000;
				}

				/* Write 512 Data Area */
				BITCSET( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_1 );	// 1R/W Burst Size
				pMTD_NFC->NFC_DSIZE		= nDevInfo->BytesPerSector;
				#if defined(TCC89XX) || defined(TCC92XX) 
				pMTD_NFC->NFC_IREQ 		= 0x77;	// pMTD_NFC->NFC_IREQ_FLAG1;
				#elif defined(TCC93XX) || defined(TCC88XX)
				pMTD_NFC->NFC_CTRL 	|= HwNFC_CTRL_PRSEL_P;
				pMTD_NFC->NFC_IRQCFG 	|= HwNFC_IRQCFG_PIEN;
				#endif

				#if defined(TCC89XX) || defined(TCC92XX)
				TCC_MTD_IO_IRQ_Mask();
				pMTD_NFC->NFC_PSTART	= 0;
				#elif defined(TCC93XX) || defined(TCC88XX)
				TCC_MTD_IO_IRQ_Mask();
				pMTD_NFC->NFC_PRSTART	= 0;
				#endif

				i = ( nDevInfo->BytesPerSector >> 2);
				do {
					#if defined(TCC89XX) || defined(TCC92XX)
					while (!( pMTD_NFC->NFC_CTRL & HwNFC_CTRL_FS_RDY ));
					#elif defined(TCC93XX) || defined(TCC88XX)

					#if 1 //def FOR_LOOP
					for( k = 0; k < 10000000; k++ )
					{
						if( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY )
							break;

						ASM_NOP;
					}

					if( !( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY ) )
					{
						printk(" !!! MTD Write Fail [HwNFC_CTRL_FS_RDY]!!!\n ReWrite Attempt [Line:%d]\n", __LINE__);
						res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pSpareB );
						return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
					}
					#else					
					while (!( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY ));
					#endif
					
					#endif
					
					if ( bAlignAddr )
					{
						pMTD_NFC->NFC_LDATA = *pPageDW;++pPageDW;
					}	
					else
					{
						uDWordByte.BYTE[0] = *pPageB;++pPageB;
						uDWordByte.BYTE[1] = *pPageB;++pPageB;
						uDWordByte.BYTE[2] = *pPageB;++pPageB;
						uDWordByte.BYTE[3] = *pPageB;++pPageB;
						pMTD_NFC->NFC_LDATA	= uDWordByte.DWORD;
					}
				}while(--i);

				#if defined(TCC89XX) || defined(TCC92XX) 
				while (ISZERO( pMTD_NFC->NFC_IREQ, HwNFC_IREQ_FLAG1 ));
				TCC_MTD_IO_IRQ_UnMask();
				#elif defined(TCC93XX) || defined(TCC88XX)

				#if 1 //def FOR_LOOP
				for( k = 0; k < 10000000; k++ )
				{
					if( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG )
						break;

					ASM_NOP;
				}

				if( !( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG ) )
				{
					printk(" !!! MTD Write Fail [HwNFC_IRQ_PFG]!!!\n ReWrite Attempt [Line:%d]\n", __LINE__);				
					res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pSpareB );
					return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
				}
				
				#else
				
				while (ISZERO( pMTD_NFC->NFC_IRQ, HwNFC_IRQ_PFG));
				#endif
				
				pMTD_NFC->NFC_IRQ 	|= HwNFC_IRQ_PFG;
				TCC_MTD_IO_IRQ_UnMask();			
				#endif
			}
			else
			{
				if ( nEccOnOff == ECC_ON )
				{
					/* Setup ECC Block */
					TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwDATA_PA );
					pMTD_ECC->ECC_CTRL	|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );
					pMTD_ECC->ECC_CLEAR	= 0x00000000;

					#if defined(TCC93XX) || defined(TCC88XX)
					pMTD_NFC->NFC_CTRL		|= HwNFC_CTRL_ECCON_EN;		// NFC ECC Encode/Decode Enable
					#endif
				}

				/* Write 512 Data Area */
				i = ( nDevInfo->BytesPerSector >> 2);
				do {				
					if ( bAlignAddr )
					{
						pMTD_NFC->NFC_WDATA = *pPageDW;++pPageDW;
					}	
					else
					{
						uDWordByte.BYTE[0] = *pPageB;++pPageB;
						uDWordByte.BYTE[1] = *pPageB;++pPageB;
						uDWordByte.BYTE[2] = *pPageB;++pPageB;
						uDWordByte.BYTE[3] = *pPageB;++pPageB;
						pMTD_NFC->NFC_WDATA	= uDWordByte.DWORD;
					}
				}while(--i);	
			}

			//----------------------------------------------
			//	DMA ACCESS
			////----------------------------------------------
			//#elif defined( NAND_IO_USE_DMA_ACCESS )
			//if ( nEccOnOff == ECC_ON )
			//{
			//	/* Setup ECC Block */
			//	#if defined(_WINCE_) || defined(_LINUX_)
			//	NAND_IO_SetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
			//	#else
			//	NAND_IO_SetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&pMTD_NFC->NFC_LDATA );
			//	#endif
			//	pECC->ECC_CTRL	|= ( 512 << ECC_SHIFT_DATASIZE );
			//	pECC->ECC_CLEAR	= 0x00000000;
			//}
			//
			//#if defined(_LINUX_) || defined(_WINCE_)
			//NAND_IO_SetupDMA( (void*)pDataBuffer, 4, 0,
			//				  (void*)&NAND_IO_HwLDATA_PA, 0, 0,
			//				  NAND_IO_DMA_WRITE );
			//#else
			//NAND_IO_SetupDMA( (void*)pDataBuffer, 4, 0,
			//				  (void*)&pMTD_NFC->NFC_LDATA, 0, 0,
			//				  NAND_IO_DMA_WRITE );
			//#endif
			//
			//if ( bAlignAddr )
			//	pPageDW += 128;
			//else
			//	pPageB += 512;
			//
			//#endif
			//####################################################
			//####################################################

			/*	Load ECC code from ECC block */
			if ( nEccOnOff == ECC_ON )
			{
				if ( ( nDevInfo->Feature.MediaType  & A_MLC_16BIT ) || ( nDevInfo->Feature.MediaType  & A_MLC_24BIT ) )
				{
					pSpare = (unsigned char*)pSpareB;
					pEccB = (unsigned char*)nECCBuffer;
					res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pEccB );
					for ( i = 0; i < nDevInfo->EccDataSize; ++i )
						pSpare[i] = pEccB[i];
				}
				else
				{
					res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pSpareB );				
				}
				
				if ( res != SUCCESS )
					goto ErrorWrite512Data;
			}

			pSpareB += nDevInfo->EccDataSize;
			
		}

		//=========================================================================
		// Empty PPage Write
		//=========================================================================
		if ( ( nStartPPage + nWritePPSize ) != nDevInfo->UsablePPages )
		{
			if ( nDevInfo->Feature.MediaType & A_BIG )
			{
				/* Change Cycle */
				TCC_MTD_IO_locSetCommCycleTime();
				
				//---------------------------------
				// Random Data Input [ 0x85 ]
				//---------------------------------
				pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0x8585;

				//---------------------------------
				// Write Column Address
				//---------------------------------
				ColumnAddr  = nDevInfo->Feature.PageSize;
				ColumnAddr	= ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT ) ? (ColumnAddr >> 1) : ColumnAddr;
				TCC_MTD_IO_locWriteColAddr( ColumnAddr );

				//---------------------------------
				// ECC Data Clear
				//---------------------------------
				for ( j = 0; j < (U16)( nDevInfo->UsablePPages - ( nStartPPage + nWritePPSize ) ); ++j )
				{
					if ( nDevInfo->EccType == MLC_ECC_4BIT_TYPE )
						pECC_Info = &gMLC_ECC_4Bit;
					#if 0	//TCC93XX TODO 6Bit ECC 
					else if ( nDevInfo->EccType == MLC_ECC_6BIT_TYPE )
						pECC_Info = &gMLC_ECC_6Bit;
					#endif
					else if ( nDevInfo->EccType == MLC_ECC_8BIT_TYPE )
						pECC_Info = &gMLC_ECC_8Bit;
					else if ( nDevInfo->EccType == MLC_ECC_12BIT_TYPE )
						pECC_Info = &gMLC_ECC_12Bit;
					else if ( nDevInfo->EccType == MLC_ECC_14BIT_TYPE )
						pECC_Info = &gMLC_ECC_14Bit;
					else if ( nDevInfo->EccType == MLC_ECC_16BIT_TYPE )
						pECC_Info = &gMLC_ECC_16Bit;
					else if ( nDevInfo->EccType == MLC_ECC_24BIT_TYPE )
						pECC_Info = &gMLC_ECC_24Bit;
					else
						return ERR_TCC_MTD_IO_WRONG_PARAMETER;

					if ( ( nDevInfo->Feature.MediaType  & A_MLC_16BIT ) || ( nDevInfo->Feature.MediaType  & A_MLC_24BIT ) )
					{
						pSpare = (unsigned char*)pSpareB;
						
						for ( i = 0; i < nECCDataSize; ++i )
							pSpare[i] = pECC_Info->All_FF_512_ECC_Code[i];
					}
					else
					{
						memcpy(	(void*)pSpareB, (void*)pECC_Info->All_FF_512_ECC_Code, nECCDataSize);			
					}
					pSpareB += nECCDataSize;	
				}
			}
			else
			{
	//			memset( nDummyPageBuffer, 0xFF, 512 );
				//Write Dummy Data
				for ( j = 0; j < (U16)( nDevInfo->UsablePPages - ( nStartPPage + nWritePPSize ) ); ++j )
				{
					//####################################################
					//#	Write 512 Page Data
					//####################################################
					//----------------------------------------------
					//	MCU ACCESS
					//----------------------------------------------
					//#if defined( NAND_IO_USE_MCU_ACCESS )
					#if defined(TCC89XX)  || defined(TCC92XX)
					if (!( pMTD_NFC->NFC_CTRL1 & Hw30 ))
					#elif defined(TCC93XX) || defined(TCC88XX)
					if (!( pMTD_NFC->NFC_CTRL & HwNFC_CTRL_DACK_EN ))
					#endif
					{
				
						if ( nEccOnOff == ECC_ON )
						{
							TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
							pMTD_ECC->ECC_CTRL	|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );		// Data Size...
							pMTD_ECC->ECC_CLEAR	= 0x00000000;
						}

						/* Write 512 Data Area */
						BITCSET( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_1 );	// 1R/W Burst Size
						#if defined(TCC89XX) || defined(TCC92XX)
						pMTD_NFC->NFC_DSIZE		= nDevInfo->BytesPerSector;
						pMTD_NFC->NFC_IREQ		= 0x77;	// pMTD_NFC->NFC_IREQ_FLAG1;		
						TCC_MTD_IO_IRQ_Mask();
						pMTD_NFC->NFC_PSTART	= 0;
						#elif defined(TCC93XX) || defined(TCC88XX)
						pMTD_NFC->NFC_DSIZE		= nDevInfo->BytesPerSector;
						pMTD_NFC->NFC_CTRL		|= HwNFC_CTRL_ECCON_EN;		
						TCC_MTD_IO_IRQ_Mask();
						pMTD_NFC->NFC_PRSTART	= 0;
						#endif
						
						i = ( nDevInfo->BytesPerSector >> 2);
						do
						{
							#if defined(TCC89XX) || defined(TCC92XX)
							while (!( pMTD_NFC->NFC_CTRL & HwNFC_CTRL_FS_RDY ));
							#elif defined(TCC93XX) || defined(TCC88XX)

							#if 1//def FOR_LOOP
							for( k = 0; k < 10000000; k++ )
							{
								if( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY )
									break;

								ASM_NOP;
							}

							if( !(pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY) )
							{
								return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
							}
							
							#else
							while (!( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY ));
							#endif
							
							#endif
							pMTD_NFC->NFC_LDATA = 0xFFFFFFFF;
						}while(--i);

						#if defined(TCC89XX) || defined(TCC92XX) 
						while (ISZERO( pMTD_NFC->NFC_IREQ, HwNFC_IREQ_FLAG1 ));					
						TCC_MTD_IO_IRQ_UnMask();
						#elif defined(TCC93XX) || defined(TCC88XX)

						#if 1//def FOR_LOOP
						for( k = 0; k < 10000000; k++ )
						{
							if( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG)
								break;
							
							ASM_NOP;
						}

						if( !(pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG ) )
						{
							return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
						}
						
						#else
						while (ISZERO( pMTD_NFC->NFC_IRQ, HwNFC_IRQ_PFG));
						#endif
						
						pMTD_NFC->NFC_IRQ 	|= HwNFC_IRQ_PFG;
						TCC_MTD_IO_IRQ_UnMask();
						#endif

					}
					else
					{
						if ( nEccOnOff == ECC_ON )
						{
							TCC_MTD_IO_locSetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, (U32)&NAND_IO_HwDATA_PA );
							pMTD_ECC->ECC_CTRL	|= ( nDevInfo->BytesPerSector << ECC_SHIFT_DATASIZE );		// Data Size...
							pMTD_ECC->ECC_CLEAR	= 0x00000000;
						}

						/* Write 512 Data Area */
						i = ( nDevInfo->BytesPerSector >> 2);
						do
						{
							pMTD_NFC->NFC_WDATA = 0xFFFFFFFF;
						}while(--i);					

					}
					
					
					//----------------------------------------------
					//	DMA ACCESS
					//----------------------------------------------
					//#elif defined( NAND_IO_USE_DMA_ACCESS )
					///* Setup ECC Block */
					//if ( nEccOnOff == ECC_ON )
					//{
					//	#if defined(_WINCE_) || defined(_LINUX_)
					//	NAND_IO_SetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&NAND_IO_HwLDATA_PA );
					//	#else
					//	NAND_IO_SetupECC( (U16)nEccOnOff, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, (U32)&pMTD_NFC->NFC_LDATA );
					//	#endif
					//	pMTD_ECC->ECC_CTRL	|= ( 512 << ECC_SHIFT_DATASIZE );		// Data Size...
					//	pMTD_ECC->ECC_CLEAR	= 0x00000000;
					//}
					//
					//#if defined(_LINUX_) || defined(_WINCE_)
					//NAND_IO_SetupDMA( (void*)nDummyPageBuffer, 4, 0,
					//				  (void*)&NAND_IO_HwLDATA_PA, 0, 0,
					//				  NAND_IO_DMA_WRITE );
					//#else
					//NAND_IO_SetupDMA( (void*)nDummyPageBuffer, 4, 0,
					//				  (void*)&pMTD_NFC->NFC_LDATA, 0, 0,
					//				  NAND_IO_DMA_WRITE );
					//#endif
					//
					//#endif
					//####################################################
					//####################################################
					/*	Load ECC code from ECC block */
					if ( nEccOnOff == ECC_ON )
					{
						if ( ( nDevInfo->Feature.MediaType  & A_MLC_16BIT ) || ( nDevInfo->Feature.MediaType  & A_MLC_24BIT ) )
						{
							pSpare = (unsigned char*)pSpareB;
							pEccB = (unsigned char*)nECCBuffer;
							res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pEccB );
							for ( i = 0; i < nDevInfo->EccDataSize; ++i )
								pSpare[i] = pEccB[i];
						}
						else
						{
							res = TCC_MTD_IO_locEncodeECC( nDevInfo->EccType, pSpareB );
						}
						if ( res != SUCCESS )
							goto ErrorWrite512Data;
					}

					pSpareB += nDevInfo->EccDataSize;		
				}			
			}
		}
		
		//=========================================================================
		// Write Spare Data
		//=========================================================================
		/* Change Cycle */
		TCC_MTD_IO_locSetWriteCycleTime();
			
		TCC_MTD_IO_locWriteSpareData( nDevInfo, nSpareBuffer, PAGE_ECC_ON );
	}
	//=========================================================================
	// Return
	//=========================================================================

ErrorWrite512Data:
	return res;

}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locWriteSpareData( TCC_MTD_IO_DEVINFO *nDevInfo, U8 *nSpareBuffer, int nPageEccOnOff );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDevInfo	= 
*			nPageEccOnOff	= 
*			nSpareBuffer	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locWriteSpareData( TCC_MTD_IO_DEVINFO *nDevInfo, U8 *nSpareBuffer, int nPageEccOnOff )
{
	unsigned int		i, j;
	unsigned int		nSpareTotalSize;
	unsigned char		bAlignAddr;
	unsigned char		*pSpareB = 0;
	unsigned int		*pSpareDW = 0;

	DWORD_BYTE			uDWordByte;
	TCC_MTD_IO_ERROR	res;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	res 			= (TCC_MTD_IO_ERROR)SUCCESS;

	//Yaffs Tags 20Bytes 
	nSpareTotalSize = 20;
	
	//=========================================================================
	// Check Align of nSpareBuffer Address
	//=========================================================================
	bAlignAddr		= ( (unsigned int)nSpareBuffer & 3 ) ? 0 : 1;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	if ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	else
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

	TCC_MTD_IO_locSetupECC( ECC_OFF, 0, 0, 0, 0 );

	//=========================================================================
	// Get Buffer Pointer
	//=========================================================================
	/* Adapt type of address */
	if ( bAlignAddr )
		pSpareDW	= (unsigned int*)nSpareBuffer;
	else
		pSpareB		= (unsigned char*)nSpareBuffer;

	//==============================================================
	// Write Spare
	//==============================================================
	if ( nDevInfo->Feature.MediaType & A_BIG )
	{
		#if defined(TCC89XX) || defined(TCC92XX)
		if (!( pMTD_NFC->NFC_CTRL1 & Hw30 ))
		#elif defined(TCC93XX) || defined(TCC88XX)
		if (!( pMTD_NFC->NFC_CTRL & HwNFC_CTRL_DACK_EN ))
		#endif
		{
			/* Write 20Bytes spare data */
			BITCSET( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_1 );	// 1R/W Burst Size
			pMTD_NFC->NFC_DSIZE		= nSpareTotalSize;
			//#if (!( pNFC->NFC_CTRL & HwNFC_CTRL_DACK_EN ))
			#if defined(TCC89XX) || defined(TCC92XX) 
			pMTD_NFC->NFC_IREQ	 	= 0x77;	// pMTD_NFC->NFC_IREQ_FLAG1;
			#elif defined(TCC93XX) || defined(TCC88XX)
			pMTD_NFC->NFC_CTRL 	 |= HwNFC_CTRL_PRSEL_P;
			pMTD_NFC->NFC_IRQCFG |= HwNFC_IRQCFG_PIEN;
			#endif

			#if defined(TCC89XX) || defined(TCC92XX)
			TCC_MTD_IO_IRQ_Mask();
			pMTD_NFC->NFC_PSTART 	= 0;
			#elif defined(TCC93XX) || defined(TCC88XX)
			TCC_MTD_IO_IRQ_Mask();
			pMTD_NFC->NFC_PRSTART	= 0;
			#endif

			i = ( nSpareTotalSize >> 2 );
			do {
				#if defined(TCC89XX) || defined(TCC92XX)
				while (!( pMTD_NFC->NFC_CTRL & HwNFC_CTRL_FS_RDY ));
				#elif defined(TCC93XX) || defined(TCC88XX)

				#if 1//def FOR_LOOP
				for( j = 0; j < 10000000; j++ )
				{
					if ( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY )
						break;
					
					ASM_NOP;
				}

				if( !( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY ) )
				{
					printk(" !!! MTD Write Fail [HwNFC_IRQ_PFG]!!!\n ReWrite Attempt [Line:%d]\n", __LINE__);				
					return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
				}
				#else
				while (!( pMTD_NFC->NFC_STATUS & HwNFC_CTRL_FS_RDY ));
				#endif
				
				#endif
				
				if ( bAlignAddr )
				{
					pMTD_NFC->NFC_LDATA = *pSpareDW;++pSpareDW;
				}
				else
				{
					uDWordByte.BYTE[0]	= *pSpareB;++pSpareB;
					uDWordByte.BYTE[1]	= *pSpareB;++pSpareB;
					uDWordByte.BYTE[2]	= *pSpareB;++pSpareB;
					uDWordByte.BYTE[3]	= *pSpareB;++pSpareB;
					pMTD_NFC->NFC_LDATA = uDWordByte.DWORD;
				}
			}while(--i);

			#if defined(TCC89XX) || defined(TCC92XX) 
			while (ISZERO( pMTD_NFC->NFC_IREQ, HwNFC_IREQ_FLAG1 ));
			TCC_MTD_IO_IRQ_UnMask();		
			#elif defined(TCC93XX) || defined(TCC88XX)

			#if 1//def FOR_LOOP
			for( j = 0; j < 10000000; j++ )
			{
				if( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG )
					break;
				
				ASM_NOP;
			}

			if( !( pMTD_NFC->NFC_IRQ & HwNFC_IRQ_PFG ) )
			{
				printk(" !!! MTD Write Fail [HwNFC_IRQ_PFG]!!!\n ReWrite Attempt [Line:%d]\n", __LINE__);			
				return ERR_TCC_MTD_IO_ATTEMPT_REWRITE;
			}
			
			#else
			while (ISZERO( pMTD_NFC->NFC_IRQ, HwNFC_IRQ_PFG));
			#endif
			
			pMTD_NFC->NFC_IRQ 	|= HwNFC_IRQ_PFG;
			TCC_MTD_IO_IRQ_UnMask();
			#endif
		}
		else
		{
			/* Write 16Bytes spare data */
			i = ( nSpareTotalSize >> 2 );
			do {
				if ( bAlignAddr )
				{
					pMTD_NFC->NFC_WDATA = *pSpareDW;++pSpareDW;
				}
				else
				{
					uDWordByte.BYTE[0]	= *pSpareB;++pSpareB;
					uDWordByte.BYTE[1]	= *pSpareB;++pSpareB;
					uDWordByte.BYTE[2]	= *pSpareB;++pSpareB;
					uDWordByte.BYTE[3]	= *pSpareB;++pSpareB;
					pMTD_NFC->NFC_WDATA 	= uDWordByte.DWORD;
				}
			}while(--i);
		}

		if ( nPageEccOnOff == PAGE_ECC_ON )
		{
			//=========================================================================
			// Check Align of PageBuffer Address
			//=========================================================================

			memcpy( &gTCC_MTD_IO_ShareEccBuffer[((nDevInfo->UsablePPages- 1)*nDevInfo->EccDataSize)], &gTCC_MTD_IO_PreEnDecodeEccBuffer[0], nDevInfo->EccDataSize );
			
			bAlignAddr 	= ( (unsigned int)&gTCC_MTD_IO_ShareEccBuffer & 3 ) ? 0 : 1;
			
			//=========================================================================
			// Get Buffer Pointer
			//=========================================================================
			/* Adapt type of address */
			if ( bAlignAddr )
				pSpareDW	= (unsigned int*)&gTCC_MTD_IO_ShareEccBuffer[0];
			else
				pSpareB		= (unsigned char*)&gTCC_MTD_IO_ShareEccBuffer[0];

			//----------------------------------------------
			//	Read Spare Data
			//----------------------------------------------
			i = ( nDevInfo->EccWholeDataSize >> 2 );
			do {
				if ( bAlignAddr )
				{
					pMTD_NFC->NFC_WDATA = *pSpareDW;++pSpareDW;
				}	
				else
				{
					uDWordByte.BYTE[0]	= *pSpareB;++pSpareB;
					uDWordByte.BYTE[1]	= *pSpareB;++pSpareB;
					uDWordByte.BYTE[2]	= *pSpareB;++pSpareB;
					uDWordByte.BYTE[3]	= *pSpareB;++pSpareB;
					pMTD_NFC->NFC_WDATA = uDWordByte.DWORD;
				}
			}while(--i);
		}
	}
	else
	{
		if (( nDevInfo->Feature.MediaType & A_SMALL ) && ( nDevInfo->Feature.MediaType & A_PARALLEL ))
		{
			if ( nPageEccOnOff == PAGE_ECC_ON )
			{
				//=========================================================================
				// Copy ECC Data (Last PPage + Yaffs Tage) 
				//=========================================================================
				memcpy( &gTCC_MTD_IO_ShareEccBuffer[((nDevInfo->UsablePPages - 1)*nDevInfo->EccDataSize)], &gTCC_MTD_IO_PreEnDecodeEccBuffer[nDevInfo->BytesPerSector], nDevInfo->EccDataSize );
				
				//=========================================================================
				// Check Align of PageBuffer Address
				//=========================================================================
				bAlignAddr = ( (unsigned int)&gTCC_MTD_IO_ShareEccBuffer & 3 ) ? 0 : 1;
				
				//=========================================================================
				// Get Buffer Pointer
				//=========================================================================
				/* Adapt type of address */
				if ( bAlignAddr )
					pSpareDW	= (unsigned int*)&gTCC_MTD_IO_ShareEccBuffer[0];
				else
					pSpareB		= (unsigned char*)&gTCC_MTD_IO_ShareEccBuffer[0];
				
				//----------------------------------------------
				//	Read Spare Data
				//----------------------------------------------
				i = ( nDevInfo->EccWholeDataSize >> 2 );
				do {
					if ( bAlignAddr )
					{
						pMTD_NFC->NFC_WDATA = *pSpareDW;++pSpareDW;
					}	
					else
					{
						uDWordByte.BYTE[0] = *pSpareB;++pSpareB;
						uDWordByte.BYTE[1] = *pSpareB;++pSpareB;
						uDWordByte.BYTE[2] = *pSpareB;++pSpareB;
						uDWordByte.BYTE[3] = *pSpareB;++pSpareB;
						pMTD_NFC->NFC_WDATA = uDWordByte.DWORD;
					}
				}while(--i);
			}			
		}
	}
	
	//=========================================================================
	// Return
	//=========================================================================

	return res;

}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locGenerateRowColAddrForWrite( U32 nPageAddr, U16 nColumnAddr,
*      										  				  		  U32* rRowAddr, U32* rColumnAddr );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nColumnAddr	= 
*			nPageAddr	= 
*			rColumnAddr	= 
*			rRowAddr	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locGenerateRowColAddrForWrite( U32 nPageAddr, U16 nColumnAddr,
										  				  		 		   U32* rRowAddr, U32* rColumnAddr )
{
	unsigned long int	RowAddr;
	unsigned long int	ColumnAddr;
	TCC_MTD_IO_DEVINFO	*nDevInfo;
	
	nDevInfo = &gDevInfo[0];
	
	if ( nColumnAddr > ( nDevInfo->Feature.PageSize + nDevInfo->Feature.SpareSize ) )
		return ERR_TCC_MTD_IO_WRONG_PARAMETER_ROW_COL_ADDRESS;

	//==================================================
	// Generate Column & Row Address	
	//==================================================
	if ( nDevInfo->Feature.MediaType & A_SMALL )
	{
		// ColumnAddr	ADR[7:0]	==> nColumnAddr
		// RowAddr		ADR[25:9]	==> (nPageAddr)*512
		ColumnAddr	= ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )  ? (nColumnAddr>>1) : nColumnAddr;
		RowAddr		= nPageAddr;

		if ( nDevInfo->Feature.MediaType & A_08BIT )
		{
		/* Command READ for SMALL NAND */
		if ( ColumnAddr < 256 )
		{
			pMTD_NFC->NFC_CMD	= nDevInfo->CmdMask & 0x0000;
			ColumnAddr	= ColumnAddr;
		}
		else if ( ColumnAddr < 512 )
		{
			pMTD_NFC->NFC_CMD	= nDevInfo->CmdMask & 0x0101;
			ColumnAddr  = ColumnAddr-256;
		}
		else
		{
			pMTD_NFC->NFC_CMD	= nDevInfo->CmdMask & 0x5050;
			ColumnAddr	= ColumnAddr-512;
		}
		}
		else if ( nDevInfo->Feature.MediaType & A_16BIT )
		{
			/* Command READ for SMALL NAND */
			if ( ColumnAddr < 256 )
			{
				pMTD_NFC->NFC_CMD	= nDevInfo->CmdMask & 0x0000;
				ColumnAddr	= ColumnAddr;
			}
			else
			{
				pMTD_NFC->NFC_CMD	= nDevInfo->CmdMask & 0x5050;
				ColumnAddr	= ColumnAddr;
			}
		}		
	}
	else
	{
		// ColumnAddr   ADR[11:0]   ==> nColumnAddr
		// RowAddr      ADR[31:12]  ==> nPageAddr
		ColumnAddr	= ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT ) ? (nColumnAddr>>1) : nColumnAddr;
		RowAddr		= nPageAddr;
	}			

	*rRowAddr		= RowAddr;
	*rColumnAddr	= ColumnAddr;

	return (TCC_MTD_IO_ERROR)SUCCESS;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locGenerateRowColAddrForRead( U32 nPageAddr, U16 nColumnAddr,
*      												  				  		  U32* rRowAddr, U32* rColumnAddr,
*      																  		  TCC_MTD_IO_DEVINFO *nDevInfo );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nColumnAddr	= 
*			nDevInfo	= 
*			nPageAddr	= 
*			rColumnAddr	= 
*			rRowAddr	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locGenerateRowColAddrForRead( U32 nPageAddr, U16 nColumnAddr,
												  				  		  U32* rRowAddr, U32* rColumnAddr,
																  		  TCC_MTD_IO_DEVINFO *nDevInfo )
{
	unsigned long int	RowAddr;
	unsigned long int	ColumnAddr;
	
	if ( nColumnAddr > ( nDevInfo->Feature.PageSize + nDevInfo->Feature.SpareSize ) )
		return ERR_TCC_MTD_IO_WRONG_PARAMETER_ROW_COL_ADDRESS;

	//==================================================
	// Generate Column & Row Address	
	//==================================================
	if ( nDevInfo->Feature.MediaType & A_SMALL )
	{
		// ColumnAddr	ADR[7:0]	==> nColumnAddr
		// RowAddr		ADR[25:9]	==> (nPageAddr)*512
		ColumnAddr	= ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT )  ? (nColumnAddr>>1) : nColumnAddr;
		RowAddr		= nPageAddr;

		if ( nDevInfo->Feature.MediaType & A_08BIT )
		{
			/* Command READ for SMALL NAND */
			if ( ColumnAddr < 256 )
			{
				pMTD_NFC->NFC_CMD	= nDevInfo->CmdMask & 0x0000;
				ColumnAddr	= ColumnAddr;
			}
			else if ( ColumnAddr < 512 )
			{
				pMTD_NFC->NFC_CMD	= nDevInfo->CmdMask & 0x0101;
				ColumnAddr  = ColumnAddr-256;
			}
			else
			{
				pMTD_NFC->NFC_CMD	= nDevInfo->CmdMask & 0x5050;
				ColumnAddr	= ColumnAddr-512;
			}			
		}
		else if ( nDevInfo->Feature.MediaType & A_16BIT )
		{
			/* Command READ for SMALL NAND For 16Bit */
			if ( ColumnAddr < 256 )
			{
				pMTD_NFC->NFC_CMD	= nDevInfo->CmdMask & 0x0000;
				ColumnAddr	= ColumnAddr;
			}
			else
			{
				pMTD_NFC->NFC_CMD	= nDevInfo->CmdMask & 0x5050;
				ColumnAddr	= ColumnAddr;
			}
		}
	}
	else
	{
		/* Command READ [ 0x00 ] */
		pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0x0000;

		// ColumnAddr   ADR[11:0]   ==> nColumnAddr
		// RowAddr      ADR[31:12]  ==> nPageAddr
		ColumnAddr	= ( nDevInfo->Feature.MediaType & A_DATA_WITDH_16BIT ) ? (nColumnAddr>>1) : nColumnAddr;
		RowAddr		= nPageAddr;

	}			

	*rRowAddr		= RowAddr;
	*rColumnAddr	= ColumnAddr;

	return (TCC_MTD_IO_ERROR)SUCCESS;
}	

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locWriteBlockPageAddr( U32 nBlockPageAddr );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nBlockPageAddr	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locWriteBlockPageAddr( U32 nBlockPageAddr )
{
	unsigned int		i;
	TCC_MTD_IO_DEVINFO 	*nDevInfo;

	nDevInfo = &gDevInfo[0];
	
	//==================================================
	// Write Block Address
	//==================================================
	for ( i = 0; i < nDevInfo->Feature.RowCycle; ++i )
	{
		pMTD_NFC->NFC_SADDR	= nDevInfo->CmdMask & (((nBlockPageAddr<<8)&0xFF00)|(nBlockPageAddr&0x00FF));
		nBlockPageAddr = nBlockPageAddr >> 8;
	}
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locWriteRowColAddr( U32 nRowAddr, U32 nColumnAddr );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nColumnAddr	= 
*			nRowAddr	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locWriteRowColAddr( U32 nRowAddr, U32 nColumnAddr )
{
	unsigned int		nTempAddr;
	unsigned int		i;
	TCC_MTD_IO_DEVINFO	*nDevInfo;
	
	nDevInfo = &gDevInfo[0];
	
	//==================================================
	// Write Column Address
	//==================================================
	for ( i = 0; i < nDevInfo->Feature.ColCycle; ++i )
	{
		nTempAddr = nDevInfo->CmdMask & (((nColumnAddr<<8)&0xFF00)|(nColumnAddr&0x00FF));
		pMTD_NFC->NFC_SADDR = nTempAddr;
		
		nColumnAddr = nColumnAddr >> 8;
	}

	//==================================================
	// Write Row Address
	//==================================================	
	nRowAddr = nRowAddr;

	for ( i = 0; i < nDevInfo->Feature.RowCycle; ++i )
	{
		nTempAddr = nDevInfo->CmdMask & (((nRowAddr<<8)&0xFF00)|(nRowAddr&0x00FF));
		pMTD_NFC->NFC_SADDR = nTempAddr;

		nRowAddr = nRowAddr >> 8;
	}
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locWriteColAddr( U32 nColumnAddr );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nColumnAddr	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locWriteColAddr( U32 nColumnAddr )
{
	unsigned int		i;
	TCC_MTD_IO_DEVINFO 	*nDevInfo;

	nDevInfo = &gDevInfo[0];
		
	//==================================================
	// Write Column Address
	//==================================================
	for ( i = 0; i < nDevInfo->Feature.ColCycle; ++i )
	{
		pMTD_NFC->NFC_SADDR	= nDevInfo->CmdMask & (((nColumnAddr<<8)&0xFF00)|(nColumnAddr&0x00FF));
		nColumnAddr = nColumnAddr >> 8;
	}
}

static __inline void TCC_MTD_IO_LineUpToBytesPerSector( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nStartPPage, U16 nPPSize, U16* rStartPPage, U16* rPPSize, U16* nFlagOfAlign )
{
	*rStartPPage	= nStartPPage;
	*rPPSize 	  	= nPPSize;
		
	if ( nDevInfo->Feature.MediaType & A_MLC_24BIT ) 
	{
		//------------------------------
		// Start PPageNo°¡ Even 
		//------------------------------
		if ( (nStartPPage & 1 ) == 0  && ( nPPSize & 1 ) == 0 )
			*nFlagOfAlign = DISABLE; 
		else
			*nFlagOfAlign = ENABLE; 

		if ( *nFlagOfAlign == ENABLE )
		{
			*rStartPPage = ( nStartPPage >> 1 );
			
			if ( ( nStartPPage + nPPSize ) % 2 )
				*rPPSize =  ( ( nPPSize >> 1 ) + 1 );
			else
			{
				if ( ( nPPSize != 1 ) && ( nPPSize != 0 ) )
					*rPPSize = ( ( nPPSize >> 1 ) + 1 );
			}
		}
		else
		{
			*rStartPPage = ( nStartPPage >> 1 );
			*rPPSize = ( nPPSize >> 1);
		}
	}
}

static __inline void TCC_MTD_IO_LineUpToBufferAlignement( TCC_MTD_IO_DEVINFO *nDevInfo, U16 nFlagOfAlign, U16 nStartPPage, U16 nLastPPage, U8 *nPageBuffer, U8 *nSpareBuffer, U8** pPageBuffer, U8** pSpareBuffer, U8 nMode )
{
	if ( nMode == TCC_MTD_IO_READ_MODE )
	{
		if ( nFlagOfAlign == ENABLE )
		{		
			*pPageBuffer  = ( unsigned char* )&gTCC_MTD_ShareBuffer[0];
			*pSpareBuffer = ( unsigned char* )&gTCC_MTD_ShareBuffer[nDevInfo->Feature.PageSize];
		}
		else
		{
			*pPageBuffer  = ( unsigned char* )nPageBuffer;
			*pSpareBuffer = ( unsigned char* )nSpareBuffer;
		}
	}
	else
	{
		if ( nFlagOfAlign == ENABLE )
		{
			memset( gTCC_MTD_ShareBuffer, 0xFF, (TCC_MTD_MAX_SUPPORT_NAND_IO_PAGE_SIZE + TCC_MTD_MAX_SUPPORT_NAND_IO_SPARE_SIZE) );
			memcpy( &gTCC_MTD_ShareBuffer[(nStartPPage<<9)], nPageBuffer, ( nLastPPage << 9 ) );
			memcpy( &gTCC_MTD_ShareBuffer[nDevInfo->Feature.PageSize], nSpareBuffer, 20 );
			
			*pPageBuffer  = &gTCC_MTD_ShareBuffer[0];
			*pSpareBuffer = &gTCC_MTD_ShareBuffer[nDevInfo->Feature.PageSize];
		}
		else
		{
			*pPageBuffer  = ( unsigned char* )nPageBuffer;
			*pSpareBuffer = ( unsigned char* )nSpareBuffer;
		}
	}
}

static __inline void TCC_MTD_IO_LineUpBufferData( U16 nFlagOfAlign, U16 nStartPPage, U16 nLastPPage, U8 *nPageBuffer, U8 *nSpareBuffer, U8*  pPageBuffer, U8*  pSpareBuffer )
{
	if ( nFlagOfAlign == ENABLE )
	{
		if ( nStartPPage % 2 )
			memcpy( pPageBuffer,  &nPageBuffer[512], ( nLastPPage << 9 ) );
		else
			memcpy( pPageBuffer,  &nPageBuffer[0], ( nLastPPage << 9 ) );
		
		memcpy( pSpareBuffer, &nSpareBuffer[0], 20 );
	}
}

static __inline	void TCC_MTD_IO_locCheckForExtendBlockAccess( TCC_MTD_IO_DEVINFO *nDevInfo, U32* nPageAddr )
{
	unsigned long int		dwPhyBlkNo;
	unsigned long int		dwPhyPageNo;
	unsigned long int		dwPHYPageAddr;
	
	if ( nDevInfo->Feature.MediaType & S_EB )
	{
		dwPHYPageAddr = *nPageAddr;
		dwPhyBlkNo	  = dwPHYPageAddr >> nDevInfo->ShiftPpB;
		dwPhyPageNo   = dwPHYPageAddr - ( dwPhyBlkNo << nDevInfo->ShiftPpB );
		
		if ( dwPhyBlkNo >= ( nDevInfo->Feature.PBpV >> 1 ) )
		{
			dwPhyBlkNo   += ( nDevInfo->Feature.PBpV >> 1 );
			dwPHYPageAddr = ( dwPhyBlkNo << nDevInfo->ShiftPpB ) + dwPhyPageNo;

			*nPageAddr = dwPHYPageAddr;
		}
	}
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      TCC_MTD_IO_ERROR TCC_MTD_IO_locSetCycle( TCC_MTD_IO_DEVINFO *nDevInfo );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nDevInfo	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
TCC_MTD_IO_ERROR TCC_MTD_IO_locSetCycle( TCC_MTD_IO_DEVINFO *nDevInfo )
{
	unsigned int	nMaxBusClk;
	unsigned int	nMaxBusClkMHZ;
	unsigned int	nCycleTick;
	unsigned int	nTickRange = 10;
	unsigned int	nDevSTP, nDevPW, nDevHLD;
	unsigned int	nMCycleSTP, nMCyclePW, nMCycleHLD;
	unsigned int	nSCycleSTP, nSCyclePW, nSCycleHLD;
	int				nRemainSTP, nRemainPW, nRemainHLD;
	int				nTempResult;
	unsigned int	nReCompRatioSTP, nReCompRatioHLD, nReCompRatioPW;
	unsigned int	nWrCompRatioSTP, nWrCompRatioHLD, nWrCompRatioPW;
	unsigned int	nCoCompRatioSTP, nCoCompRatioHLD, nCoCompRatioPW;	
	unsigned int	nReGateDelay, nWrGateDelay;

	//===================================
	// Get Max Bus CLK
	//===================================
	nMaxBusClk = tcc_get_maximum_io_clock();

	/* Convert MHZ Value of Max Bus Clock */
	if (!( nMaxBusClk / 1000 ))
		return ERR_TCC_MTD_IO_WRONG_PARAMETER;
	nMaxBusClkMHZ = ( nMaxBusClk / 1000 );

	/* Get Cycke Tick */
	nCycleTick = ( 1000 * nTickRange ) / nMaxBusClkMHZ;
	if (( 1000 * nTickRange ) % nMaxBusClkMHZ )
		++nCycleTick;

	//===================================
	// Set Cycle
	//===================================

	/* Basis Setting */
	nReGateDelay	= 15;	
	nWrGateDelay	= 0;

	nReCompRatioSTP	= 6;
	nReCompRatioHLD	= 0;
	nReCompRatioPW	= 0;
	
	nWrCompRatioSTP	= 6;
	nWrCompRatioHLD	= 0;
	nWrCompRatioPW	= 0;

	nCoCompRatioSTP	= 0;
	nCoCompRatioHLD	= 0;
	nCoCompRatioPW	= 0;

	/* Read Cycle */
	nDevSTP				= nDevInfo->Feature.ReadSTP * nTickRange;
	nDevPW				= nDevInfo->Feature.ReadPW * nTickRange;
	nDevHLD				= nDevInfo->Feature.ReadHLD * nTickRange;
	
	nMCycleSTP			= ( nDevSTP / nCycleTick ) + (( nDevSTP && !( nDevSTP / nCycleTick )) ? 1 : 0 );
	nMCycleHLD			= ( nDevHLD / nCycleTick ) + (( nDevHLD && !( nDevHLD / nCycleTick )) ? 1 : 0 );
	nRemainSTP			= ( nMCycleSTP * nCycleTick ) - nDevSTP;
	nRemainHLD			= ( nMCycleHLD * nCycleTick ) - nDevHLD;
	nSCycleSTP			= (( nRemainSTP < 0 ) && ((( nRemainSTP >= 0 ) ? nRemainSTP : -nRemainSTP ) > (int)(( nCycleTick / 10 ) * nReCompRatioSTP ))) ? 1 : 0;
	nSCycleHLD			= (( nRemainHLD < 0 ) && ((( nRemainHLD >= 0 ) ? nRemainHLD : -nRemainHLD ) > (int)(( nCycleTick / 10 ) * nReCompRatioHLD ))) ? 1 : 0;
	nTempResult			= ( nDevPW + ( nReGateDelay * nTickRange )) + (( nRemainSTP >= 0 ) ? 0 : -nRemainSTP ) + (( nRemainHLD >= 0 ) ? 0 : -nRemainHLD );
	nMCyclePW			= ( nTempResult >= (int)nCycleTick ) ? ( nTempResult / nCycleTick ) : 1;
	nRemainPW			= ( nMCyclePW * nCycleTick ) - ( nDevPW + ( nReGateDelay * nTickRange ));
	nSCyclePW			= (( nRemainPW < 0 ) && ((( nRemainPW >= 0 ) ? nRemainPW : -nRemainPW ) > (int)(( nCycleTick / 10 ) * nReCompRatioPW ))) ? 1 : 0;

	ReadCycleTime.STP	= (U8)(nMCycleSTP + nSCycleSTP);
	ReadCycleTime.HLD	= (U8)(nMCycleHLD + nSCycleHLD);
	ReadCycleTime.PW	= (U8)(nMCyclePW + nSCyclePW + (( nDevInfo->Feature.MediaType & A_PARALLEL ) ? 1: 0 ));

	/* Write Cycle */
	nDevSTP				= nDevInfo->Feature.WriteSTP * nTickRange;
	nDevPW				= nDevInfo->Feature.WriteWP * nTickRange;
	nDevHLD				= nDevInfo->Feature.WriteHLD * nTickRange;
	
	nMCycleSTP			= ( nDevSTP / nCycleTick ) + (( nDevSTP && !( nDevSTP / nCycleTick )) ? 1 : 0 );
	nMCycleHLD			= ( nDevHLD / nCycleTick ) + (( nDevHLD && !( nDevHLD / nCycleTick )) ? 1 : 0 );
	nRemainSTP			= ( nMCycleSTP * nCycleTick ) - nDevSTP;
	nRemainHLD			= ( nMCycleHLD * nCycleTick ) - nDevHLD;
	nSCycleSTP			= (( nRemainSTP < 0 ) && ((( nRemainSTP >= 0 ) ? nRemainSTP : -nRemainSTP ) > (int)(( nCycleTick / 10 ) * nWrCompRatioSTP ))) ? 1 : 0;
	nSCycleHLD			= (( nRemainHLD < 0 ) && ((( nRemainHLD >= 0 ) ? nRemainHLD : -nRemainHLD ) > (int)(( nCycleTick / 10 ) * nWrCompRatioHLD ))) ? 1 : 0;
	nTempResult			= ( nDevPW + ( nWrGateDelay * nTickRange )) + (( nRemainSTP >= 0 ) ? 0 : -nRemainSTP ) + (( nRemainHLD >= 0 ) ? 0 : -nRemainHLD );
	nMCyclePW			= ( nTempResult >= (int)nCycleTick ) ? ( nTempResult / nCycleTick ) : 1;
	nRemainPW			= ( nMCyclePW * nCycleTick ) - ( nDevPW + ( nWrGateDelay * nTickRange ));
	nSCyclePW			= (( nRemainPW < 0 ) && ((( nRemainPW >= 0 ) ? nRemainPW : -nRemainPW ) > (int)(( nCycleTick / 10 ) * nWrCompRatioPW ))) ? 1 : 0;

	WriteCycleTime.STP	= (U8)(nMCycleSTP + nSCycleSTP);
	WriteCycleTime.HLD	= (U8)(nMCycleHLD + nSCycleHLD);
	WriteCycleTime.PW	= (U8)(nMCyclePW + nSCyclePW);
	
	/* Comm Cycle */
	nDevSTP				= 10 * nTickRange;
	nDevPW				= 80 * nTickRange;
	nDevHLD				= 40 * nTickRange;
	
	nMCycleSTP			= ( nDevSTP / nCycleTick ) + (( nDevSTP && !( nDevSTP / nCycleTick )) ? 1 : 0 );
	nMCycleHLD			= ( nDevHLD / nCycleTick ) + (( nDevHLD && !( nDevHLD / nCycleTick )) ? 1 : 0 );
	nRemainSTP			= ( nMCycleSTP * nCycleTick ) - nDevSTP;
	nRemainHLD			= ( nMCycleHLD * nCycleTick ) - nDevHLD;
	nSCycleSTP			= (( nRemainSTP < 0 ) && ((( nRemainSTP >= 0 ) ? nRemainSTP : -nRemainSTP ) > (int)(( nCycleTick / 10 ) * nCoCompRatioSTP ))) ? 1 : 0;
	nSCycleHLD			= (( nRemainHLD < 0 ) && ((( nRemainHLD >= 0 ) ? nRemainHLD : -nRemainHLD ) > (int)(( nCycleTick / 10 ) * nCoCompRatioHLD ))) ? 1 : 0;
	nTempResult			= ( nDevPW + ( nReGateDelay * nTickRange )) + (( nRemainSTP >= 0 ) ? 0 : -nRemainSTP ) + (( nRemainHLD >= 0 ) ? 0 : -nRemainHLD );
	nMCyclePW			= ( nTempResult >= (int)nCycleTick ) ? ( nTempResult / nCycleTick ) : 1;
	nRemainPW			= ( nMCyclePW * nCycleTick ) - ( nDevPW + ( nReGateDelay * nTickRange ));
	nSCyclePW			= (( nRemainPW < 0 ) && ((( nRemainPW >= 0 ) ? nRemainPW : -nRemainPW ) > (int)(( nCycleTick / 10 ) * nCoCompRatioPW ))) ? 1 : 0;

	CommCycleTime.STP	= (U8)(nMCycleSTP + nSCycleSTP);
	CommCycleTime.HLD	= (U8)(nMCycleHLD + nSCycleHLD);
	CommCycleTime.PW	= (U8)(nMCyclePW + nSCyclePW);

	if (WriteCycleTime.STP >= 16)
		WriteCycleTime.STP	= 15;
	if (WriteCycleTime.PW >= 16)
		WriteCycleTime.PW	= 15;
	if (WriteCycleTime.HLD >= 16)
		WriteCycleTime.HLD	= 15;
	if (WriteCycleTime.HLD == 0)
		WriteCycleTime.HLD	= 1;	

	if (ReadCycleTime.STP >= 16)
		ReadCycleTime.STP	= 15;
	if (ReadCycleTime.PW >= 16)
		ReadCycleTime.PW	= 15;
	if (ReadCycleTime.HLD >= 16)
		ReadCycleTime.HLD	= 15;

	if (CommCycleTime.STP >= 16)
		CommCycleTime.STP	= 15;
	if (CommCycleTime.PW >= 16)
		CommCycleTime.PW	= 15;

	CommCycleTime.HLD	= 15;

	#if defined(TCC89XX) || defined(TCC92XX)
	WriteCycleTime.RegValue	= ( WriteCycleTime.STP << 8 ) + ( WriteCycleTime.PW << 4 ) + WriteCycleTime.HLD;
	ReadCycleTime.RegValue	= ( ReadCycleTime.STP << 8 ) + ( ReadCycleTime.PW << 4 ) + ReadCycleTime.HLD;
	CommCycleTime.RegValue	= ( CommCycleTime.STP << 8 ) + ( CommCycleTime.PW << 4 ) + CommCycleTime.HLD;
	#elif defined(TCC93XX) || defined(TCC88XX)
	WriteCycleTime.RegValue	= ( WriteCycleTime.STP << 16 ) + ( WriteCycleTime.PW << 8 ) + WriteCycleTime.HLD;
	ReadCycleTime.RegValue	= ( ReadCycleTime.STP << 16 ) + ( ReadCycleTime.PW << 8 ) + ReadCycleTime.HLD;
	CommCycleTime.RegValue	= ( CommCycleTime.STP << 16 ) + ( CommCycleTime.PW << 8 ) + CommCycleTime.HLD;
	#endif

	if ( nDevInfo->ChipNo == 0 )
	{
		printk("[MTD TCC] [BClk %dMHZ][1Tick %d][RE-S:%d,P:%d,H:%d][WR-S:%d,P:%d,H:%d][COM-S:%d,P:%d,H:%d]\n",
		nMaxBusClkMHZ,nCycleTick,ReadCycleTime.STP,ReadCycleTime.PW,ReadCycleTime.HLD,
		WriteCycleTime.STP,WriteCycleTime.PW,WriteCycleTime.HLD,CommCycleTime.STP,CommCycleTime.PW,CommCycleTime.HLD );
	}

	return (TCC_MTD_IO_ERROR)SUCCESS;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locSetReadCycleTime(void);
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locSetReadCycleTime(void)
{
	#if defined(TCC89XX) || defined(TCC92XX)
	BITCSET( pMTD_NFC->NFC_CTRL, 0xFFF, ReadCycleTime.RegValue );
	#elif defined(TCC93XX) || defined(TCC88XX)
	BITCSET( pMTD_NFC->NFC_RECYC, 0xF0F0F, ReadCycleTime.RegValue );
	#endif
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locSetWriteCycleTime(void);
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locSetWriteCycleTime(void)
{
	#if defined(TCC89XX) || defined(TCC92XX)
	BITCSET( pMTD_NFC->NFC_CTRL, 0xFFF, WriteCycleTime.RegValue );
	#elif defined(TCC93XX)|| defined(TCC88XX)
	BITCSET( pMTD_NFC->NFC_WRCYC, 0xF0F0F, WriteCycleTime.RegValue );
	#endif
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locSetBasicCycleTime( void );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locSetBasicCycleTime( void )
{
	/* SETUP 1 PW 5 HOLD 1 */
	#if defined(TCC89XX) || defined(TCC92XX)
	BITCSET( pMTD_NFC->NFC_CTRL, 0xFFF, 0xEEE );
	#elif defined(TCC93XX) || defined(TCC88XX)
	BITCSET( pMTD_NFC->NFC_CACYC, 0xF0F0F, 0xE0E0E);
	#endif
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locSetCommCycleTime( void );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locSetCommCycleTime( void )
{
	while (TCC_MTD_IO_locCheckReadyAndBusy());
	
	/* SETUP 1 PW 5 HOLD 1 */
	#if defined(TCC89XX) || defined(TCC92XX) 
	BITCSET( pMTD_NFC->NFC_CTRL, 0xFFF, CommCycleTime.RegValue );
	#elif defined(TCC93XX) || defined(TCC88XX)
	BITCSET( pMTD_NFC->NFC_CACYC, 0xF0F0F, CommCycleTime.RegValue );
	#endif
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locEnableChipSelect( U16 nChipNo );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nChipNo	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locEnableChipSelect( U16 nChipNo )
{
	if ( nChipNo == 0 )
	{
		/* NAND_IO_SUPPORT_4CS */
		#if defined(NAND_2CS_ONLY)
		BITSCLR( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_CFG_nCS1, HwNFC_CTRL_CFG_nCS0 );
		#else
		BITSCLR( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_CFG_NOACT, HwNFC_CTRL_CFG_nCS0 );
		#endif
	}
	else if ( nChipNo == 1 )
	{
		/* NAND_IO_SUPPORT_4CS */
		#if defined(NAND_2CS_ONLY)
		BITSCLR( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_CFG_nCS0, HwNFC_CTRL_CFG_nCS1 );
		#else
		BITSCLR( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_CFG_NOACT, HwNFC_CTRL_CFG_nCS1 );
		#endif
	}
	else if ( nChipNo == 2 )
	{
		BITSCLR( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_CFG_NOACT, HwNFC_CTRL_CFG_nCS2 );
	}
	else if ( nChipNo == 3 )
	{
		BITSCLR( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_CFG_NOACT, HwNFC_CTRL_CFG_nCS3 );
	}
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locEnableWriteProtect( void );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locEnableWriteProtect( void )
{
	gpio_set_value( nand_platform_data->gpio_wp, 0 );	
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locDisableWriteProtect( void );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locDisableWriteProtect( void )
{
	gpio_set_value( nand_platform_data->gpio_wp, 1 );

}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locDisableChipSelect( void );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locDisableChipSelect( void )
{
	/* NAND_IO_SUPPORT_4CS */
	#if defined(NAND_2CS_ONLY)
	BITSET( pMTD_NFC->NFC_CTRL, ( HwNFC_CTRL_CFG_nCS0 | HwNFC_CTRL_CFG_nCS1 ) ); 
	#else
	BITSET( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_CFG_NOACT );
	#endif
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locSetDataWidth( U32 width );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			width	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locSetDataWidth( U32 width )
{
	if ( width == TCC_MTD_IO_DATA_WITDH_8BIT )
		BITCLR( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BW_16 );
	else
		BITSET( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_BW_16 );

	if ( gMTDNAND_PORT_STATUS & TCC_MTD_IO_PORT_DATA_WITDH_16BIT )
        BITCLR( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_MASK_EN );
    else
        BITSET( pMTD_NFC->NFC_CTRL, HwNFC_CTRL_MASK_EN );
	
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locWaitBusy(void);
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locWaitBusy(void)
{
	// Misc. Configuration Register(MCFG)
   	// 0 : represent that READY pin is low
   	// 1 :                             high
   	// Delay : 200nS
	
   	TCC_MTD_IO_locDelay();

	while (TCC_MTD_IO_locCheckReadyAndBusy());
}

/**************************************************************************
*  FUNCTION NAME : 
*      static __inline void TCC_MTD_IO_locClearInterleaveStatus( NAND_IO_DEVINFO *nDevInfo );
*  
*  DESCRIPTION : 
*  INPUT:
*			nDevInfo	= 
*  
*  OUTPUT:	void - Return Type
*  REMARK  :	
**************************************************************************/
static __inline void TCC_MTD_IO_locClearInterleaveStatus( TCC_MTD_IO_DEVINFO *nDevInfo )
{
	TCC_MTD_IO_locDelay();
	
	//if ( nDevInfo->ExtInterleaveUsable ==  TRUE )
	{
		//=============================================	
		// External Inter Leave
		//=============================================	

		if ( nDevInfo->ChipNo == 0 )
		{
			TCC_MTD_IO_locEnableChipSelect( 0 );
			TCC_MTD_IO_locDisableWriteProtect();

			#if defined(TCC89XX) || defined(TCC92XX) || defined(TCC93XX) || defined(TCC88XX)
			TCC_MTD_IO_locReadStatusForExternalInterleave( nDevInfo );
			#else
			TCC_MTD_IO_locWaitBusyForProgramAndErase( nDevInfo );	
			#endif

			//gInterLeaveIoStatus &= ~NAND_IO_STATUS_INTERLEAVING_CHIP1;

			TCC_MTD_IO_locDisableChipSelect();
		}

		if ( nDevInfo->ChipNo == 1 )
		{
			TCC_MTD_IO_locEnableChipSelect( 1 );
			TCC_MTD_IO_locDisableWriteProtect();

			#if defined(TCC89XX) || defined(TCC92XX) || defined(TCC93XX) || defined(TCC88XX)
			TCC_MTD_IO_locReadStatusForExternalInterleave( nDevInfo );
			#else
			TCC_MTD_IO_locWaitBusyForProgramAndErase( nDevInfo );	
			#endif

			//gInterLeaveIoStatus &= ~NAND_IO_STATUS_INTERLEAVING_CHIP2;

			TCC_MTD_IO_locDisableChipSelect();
		}

		TCC_MTD_IO_locEnableChipSelect( nDevInfo->ChipNo );
	}
}

/**************************************************************************
*  FUNCTION NAME : 
*      static __inline NAND_IO_ERROR NAND_IO_ReadStatusForInterleaveClear( NAND_IO_DEVINFO *nDevInfo );
*  
*  DESCRIPTION : 
*  INPUT:
*			nDevInfo	= 
*  
*  OUTPUT:	NAND_IO_ERROR - Return Type
*  			= 
*  REMARK  :	
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locReadStatusForExternalInterleave( TCC_MTD_IO_DEVINFO *nDevInfo )
{
	unsigned int		uStatus;
	unsigned int		uCheckBit;
	unsigned long int	timeout;
	TCC_MTD_IO_ERROR	res;	

	//================================
	//	Read IO Status
	//================================
 	timeout = 0xFFFFF;
 	while ( timeout )
 	{
		//================================
		//	Command READ STATUS [ 0x70 ]
		//================================
		pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0x7070;

		// Delay : more than 200nS
	   	TCC_MTD_IO_locDelay();
		
		//=============================================
		// DATA BUS WIDTH Setting
		//=============================================
		if ( nDevInfo->Feature.MediaType & A_PARALLEL )
			TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
		else
			TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

		uStatus		= nDevInfo->CmdMask & pMTD_NFC->NFC_SDATA;
		uCheckBit	= nDevInfo->CmdMask & 0x4040;
				
		/* Check if it is ready */
		if ( ( uStatus & uCheckBit ) == uCheckBit )
			break;

		if ( !timeout )
			return ERR_TCC_MTD_IO_TIME_OUT_READ_STATUS;
	}

	//================================
	//	Check Bit Status
	//================================
	uStatus		= nDevInfo->CmdMask & pMTD_NFC->NFC_SDATA;
	uCheckBit	= nDevInfo->CmdMask & 0x0101;

	res = (TCC_MTD_IO_ERROR)SUCCESS;
	
	if ( uStatus & uCheckBit )
	{
		if ( nDevInfo->Feature.MediaType & A_PARALLEL )
		{
			if (uStatus & ( uCheckBit & 0x0100 ) )
				res |= TCC_MTD_IO_STATUS_FAIL_CS1_PARALLEL;

			if (uStatus & ( uCheckBit & 0x0001 ) )
				res |= TCC_MTD_IO_STATUS_FAIL_CS0_PARALLEL;
		}
		else
		{
			if (uStatus & uCheckBit )
				res |= TCC_MTD_IO_STATUS_FAIL_CS0_SERIAL;
		}
	}
	
 	return res;

}


/**************************************************************************
*  FUNCTION NAME : 
*      static __inline void NAND_IO_WaitBusyForProgramAndErase( NAND_IO_DEVINFO *nDevInfo );
*  
*  DESCRIPTION : 
*  INPUT:
*			nDevInfo	= 
*  
*  OUTPUT:	void - Return Type
*  REMARK  :	
**************************************************************************/
static __inline void TCC_MTD_IO_locWaitBusyForProgramAndErase( TCC_MTD_IO_DEVINFO *nDevInfo )
{
	// Misc. Configuration Register(MCFG)
   	// 0 : represent that READY pin is low
   	// 1 :                             high
   	// Delay : 200nS
   	TCC_MTD_IO_locDelay();

	while (TCC_MTD_IO_locCheckReadyAndBusy()) ;
	
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline void TCC_MTD_IO_locDelay( void );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
static __inline void TCC_MTD_IO_locDelay( void )
{
 	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
 	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
 	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
 	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
	ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;ASM_NOP;
}

static __inline void TCC_MTD_IO_IRQ_Mask( void )
{

	#if defined(__USE_MTD_NAND_ISR__) || defined( TCC_MTD_DMA_ACCESS )
	unsigned int	irq = MTD_NAND_IRQ_NFC;

    if (irq < 32)
    {
		BITCLR(pMTD_PIC->IEN0, (1 << irq));
        BITCLR(pMTD_PIC->INTMSK0,   (1 << irq));
    } 
    else 
    {
  		BITCLR(pMTD_PIC->IEN1, (1 << (irq - 32)));
        BITCLR(pMTD_PIC->INTMSK1,   (1 << (irq - 32)));
    }
	#endif
}

static __inline void TCC_MTD_IO_IRQ_UnMask( void )
{
	#if defined(__USE_MTD_NAND_ISR__) || defined( TCC_MTD_DMA_ACCESS )
	unsigned int	irq = MTD_NAND_IRQ_NFC;

    #if defined(TCC89XX) || defined(TCC92XX)
	pMTD_NFC->NFC_IREQ		= 0x77;	// HwNFC_IREQ_FLAG1;
	#elif defined(TCC93XX) || defined(TCC88XX)
	//TODO TCC93XX
    #endif

	//--------------------------
	// VPIC 
	//--------------------------
	// INTERRUPT 	CLEAR REGISTER
	// CLR1[9]: 	NFC 
	// INTMSK[1]:	NFC
	if (irq < 32) 
    {
        BITSET(pMTD_PIC->INTMSK0,   (1 << irq));
        BITSET(pMTD_PIC->CLR0,      (1 << irq));
		BITSET(pMTD_PIC->IEN0, 		(1 << irq));
    } 
    else 
    {
        BITSET(pMTD_PIC->INTMSK1,   (1 << (irq - 32)));
        BITSET(pMTD_PIC->CLR1,      (1 << (irq - 32)));
		BITSET(pMTD_PIC->IEN1, 	 	(1 << (irq - 32)));
    }
	#endif
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      void TCC_MTD_IO_locReset(int nMode);
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nMode	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
void TCC_MTD_IO_locReset(int nMode)
{
	/* Pre Process */
	//NAND_IO_PreProcess();
	
	/* Set Setuo Time and Hold Time */
	TCC_MTD_IO_locSetBasicCycleTime();

	/* Enable Chip Select */
	TCC_MTD_IO_locEnableChipSelect(0);

	/* Set Data Bus as 16Bit */
	TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	
	/* Command RESET [ 0xFF ] */
	if ( nMode == TCC_MTD_IO_PARALLEL_COMBINATION_MODE )
		pMTD_NFC->NFC_CMD = 0xFFFF;
	else
		pMTD_NFC->NFC_CMD = 0x00FF;

	/* Wait until it is ready */
	TCC_MTD_IO_locWaitBusy();

	/* Disable Chip Select */
	TCC_MTD_IO_locDisableChipSelect();

	/* Post Process */
	//NAND_IO_PostProcess();
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locReadStatus( void );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locReadStatus( void )
{
	unsigned int		uStatus;
	unsigned int		uCheckBit;
	unsigned long int	timeout;
	TCC_MTD_IO_ERROR	res;	
	TCC_MTD_IO_DEVINFO	*nDevInfo;
	
	nDevInfo = &gDevInfo[0];
	
	//================================
	//	Command READ STATUS [ 0x70 ]
	//================================
	pMTD_NFC->NFC_CMD = nDevInfo->CmdMask & 0x7070;

	// Delay : more than 200nS
   	TCC_MTD_IO_locDelay();
	
	//=============================================
	// DATA BUS WIDTH Setting
	//=============================================
	if ( nDevInfo->Feature.MediaType & A_PARALLEL )
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );
	else
		TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_8BIT );

	//================================
	//	Read IO Status
	//================================
 	timeout = 0xFFFFF;
 	while ( timeout )
 	{
		uStatus		= nDevInfo->CmdMask & pMTD_NFC->NFC_SDATA;
		uCheckBit	= nDevInfo->CmdMask & 0x4040;
			
		/* Check if it is ready */
		if ( ( uStatus & uCheckBit ) == uCheckBit )
			break;
	}

	if ( !timeout )
		return ERR_TCC_MTD_IO_TIME_OUT_READ_STATUS;

	//================================
	//	Check Bit Status
	//================================
	uStatus		= nDevInfo->CmdMask & pMTD_NFC->NFC_SDATA;
	uCheckBit	= nDevInfo->CmdMask & 0x0101;

	res = (TCC_MTD_IO_ERROR)SUCCESS;
	
	if ( uStatus & uCheckBit )
	{
		if ( nDevInfo->Feature.MediaType & A_PARALLEL )
		{
			if (uStatus & ( uCheckBit & 0x0100 ) )
				res |= TCC_MTD_IO_STATUS_FAIL_CS1_PARALLEL;

			if (uStatus & ( uCheckBit & 0x0001 ) )
				res |= TCC_MTD_IO_STATUS_FAIL_CS0_PARALLEL;
		}
		else
		{
			if (uStatus & uCheckBit )
				res |= TCC_MTD_IO_STATUS_FAIL_CS0_SERIAL;
		}
	}
	
 	return res;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline U32 TCC_MTD_IO_locCheckReadyAndBusy( void );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	U32 - Return Type
*  			= 
*  
**************************************************************************/
static __inline U32 TCC_MTD_IO_locCheckReadyAndBusy( void )
{
	return nand_platform_data->ready();
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      void TCC_MTD_IO_locResetForReadID( U16 nChipNo, int nMode );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nChipNo	= 
*			nMode	= 
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
void TCC_MTD_IO_locResetForReadID( U16 nChipNo, int nMode )
{
	unsigned int	i, j;
	
	/* Pre Process */
	//TCC_MTD_IO_PreProcess();
	
	/* Set Setuo Time and Hold Time */
	TCC_MTD_IO_locSetBasicCycleTime();

	/* Enable Chip Select */
	TCC_MTD_IO_locEnableChipSelect( nChipNo );

	/* Set Data Bus as 16Bit */
	TCC_MTD_IO_locSetDataWidth( TCC_MTD_IO_DATA_WITDH_16BIT );

	/* Command RESET [ 0xFF ] */
	if ( nMode == TCC_MTD_IO_PARALLEL_COMBINATION_MODE )
		pMTD_NFC->NFC_CMD = 0xFFFF;
	else
		pMTD_NFC->NFC_CMD = 0x00FF;

	/* Wait until it is ready */
	for ( i = 0; i < 0x100; ++i )
	{
		for ( j = 0; j < 0x80; ++j )
		{
			ASM_NOP;
		}
	}

	TCC_MTD_IO_locWaitBusy();
	
	/* Disable Chip Select */
	TCC_MTD_IO_locDisableChipSelect();

	/* Post Process */
	//NAND_IO_PostProcess();
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locGetShiftValueForFastMultiPly( U16 nValue, U16* rFactor );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*			nValue	= 
*			rFactor	= 
*  
*  OUTPUT:	TCC_MTD_IO_ERROR - Return Type
*  			= 
*  
**************************************************************************/
static __inline TCC_MTD_IO_ERROR TCC_MTD_IO_locGetShiftValueForFastMultiPly( U16 nValue, U16* rFactor )
{
	unsigned short int	i;

	*rFactor = 0;
	
	for ( i = 0; i < TCC_MTD_IO_MAX_SHIFT_FACTOR_FOR_MULTIPLY; ++i )
	{
		if ( TCC_MTD_IO_ShiftFactorForMultiplay[i] == nValue )
		{
			*rFactor = i;
			break;
		}
	}

	if ( i >= TCC_MTD_IO_MAX_SHIFT_FACTOR_FOR_MULTIPLY )
		return ERR_TCC_MTD_IO_FAILED_GET_SHIFT_FACTOR_FOR_MULTIPLY;

	return (TCC_MTD_IO_ERROR)SUCCESS;	
}	

/**************************************************************************
*  FUNCTION NAME : 
*  
*      void TCC_MTD_IO_locECC_InfoInit( void );
*  
*  DESCRIPTION : You can add file description here.
*  
*  INPUT:
*  
*  OUTPUT:	void - Return Type
*  
**************************************************************************/
void TCC_MTD_IO_locECC_InfoInit( void )
{
	//--------------------------------------------------------------------
	//		TCC89/92x Support ECC I/P
	//		(1Bit/ 4Bit/ 8Bit/ 12Bit /14Bit /16Bit )
	//--------------------------------------------------------------------
	#if defined(TCC89XX) || defined(TCC92XX) 
	gMLC_ECC_1Bit.EccDataSize 			= 7;
	//gMLC_ECC_1Bit.EncodeFlag			= HwECC_IREQ_M1EF;
	//gMLC_ECC_1Bit.DecodeFlag			= HwECC_IREQ_M1DF;
	gMLC_ECC_1Bit.ErrorNum				= 1;
	gMLC_ECC_1Bit.All_FF_512_ECC_Code	= (U8 *)&ALL_FF_ECC_BCH_01BIT_512;
	
	gMLC_ECC_4Bit.EccDataSize 			= 7;
	gMLC_ECC_4Bit.EncodeFlag			= HwECC_IREQ_M4EF;
	gMLC_ECC_4Bit.DecodeFlag			= HwECC_IREQ_M4DF;
	gMLC_ECC_4Bit.ErrorNum				= 4;
	gMLC_ECC_4Bit.All_FF_512_ECC_Code	= (U8 *)&ALL_FF_ECC_BCH_04BIT_512;
		
	gMLC_ECC_8Bit.EccDataSize 			= 13;
	gMLC_ECC_8Bit.EncodeFlag			= HwECC_IREQ_M8EF;
	gMLC_ECC_8Bit.DecodeFlag			= HwECC_IREQ_M8DF;
	gMLC_ECC_8Bit.ErrorNum				= 8;
	gMLC_ECC_8Bit.All_FF_512_ECC_Code	= (U8 *)&ALL_FF_ECC_BCH_08BIT_512;

	gMLC_ECC_12Bit.EccDataSize 			= 20;
	gMLC_ECC_12Bit.EncodeFlag			= HwECC_IREQ_M12EF;
	gMLC_ECC_12Bit.DecodeFlag			= HwECC_IREQ_M12DF;
	gMLC_ECC_12Bit.ErrorNum 			= 12;
	gMLC_ECC_12Bit.All_FF_512_ECC_Code	= (U8 *)&ALL_FF_ECC_BCH_12BIT_512;

	gMLC_ECC_14Bit.EccDataSize 			= 23;
	gMLC_ECC_14Bit.EncodeFlag			= HwECC_IREQ_M14EF;
	gMLC_ECC_14Bit.DecodeFlag			= HwECC_IREQ_M14DF;
	gMLC_ECC_14Bit.ErrorNum 			= 14;
	gMLC_ECC_14Bit.All_FF_512_ECC_Code	= (U8 *)&ALL_FF_ECC_BCH_14BIT_512;

	gMLC_ECC_16Bit.EccDataSize 			= 26;
	gMLC_ECC_16Bit.EncodeFlag			= HwECC_IREQ_M16EF;
	gMLC_ECC_16Bit.DecodeFlag			= HwECC_IREQ_M16DF;
	gMLC_ECC_16Bit.ErrorNum 			= 16;
	gMLC_ECC_16Bit.All_FF_512_ECC_Code	= (U8 *)&ALL_FF_ECC_BCH_16BIT_512;

	//--------------------------------------------------------------------
	//		TCC93x Support ECC I/P
	//		(1Bit/ 4Bit/ 6Bit/ 12Bit /16Bit /24Bit  )
	//--------------------------------------------------------------------
	#elif defined(TCC93XX) || defined(TCC88XX)
	gMLC_ECC_1Bit.EccDataSize 			= 7;
	gMLC_ECC_1Bit.ErrorNum				= 1;
	//gMLC_ECC_1Bit.EncodeFlag			= HwECC_IREQ_SEDFG;
	//gMLC_ECC_1Bit.DecodeFlag			= HwECC_IREQ_SEDFG;
	gMLC_ECC_1Bit.All_FF_512_ECC_Code	= (U8 *)&ALL_FF_ECC_BCH_01BIT_512;
		
	gMLC_ECC_4Bit.EccDataSize 			= 7;
	gMLC_ECC_4Bit.EncodeFlag			= HwECC_IREQ_MEEFG;
	gMLC_ECC_4Bit.DecodeFlag			= HwECC_IREQ_MEDFG;
	gMLC_ECC_4Bit.ErrorNum				= 4;
	gMLC_ECC_4Bit.All_FF_512_ECC_Code	= (U8 *)&ALL_FF_ECC_BCH_04BIT_512;

	#if 0 //TODO: TCC93X 6Bit ECC NAND
	gMLC_ECC_6Bit.EccDataSize 			= 10;
	gMLC_ECC_6Bit.EncodeFlag			= HwECC_IREQ_MEEFG;
	gMLC_ECC_6Bit.DecodeFlag			= HwECC_IREQ_MEDFG;
	gMLC_ECC_6Bit.ErrorNum				= 6;
	gMLC_ECC_86Bit.All_FF_512_ECC_Code	= (U8 *)&ALL_FF_ECC_BCH_06BIT_512;
	#endif 
	
	gMLC_ECC_12Bit.EccDataSize 			= 20;
	gMLC_ECC_12Bit.EncodeFlag			= HwECC_IREQ_MEEFG;
	gMLC_ECC_12Bit.DecodeFlag			= HwECC_IREQ_MEDFG;
	gMLC_ECC_12Bit.ErrorNum 			= 12;
	gMLC_ECC_12Bit.All_FF_512_ECC_Code	= (U8 *)&ALL_FF_ECC_BCH_12BIT_512;

	gMLC_ECC_16Bit.EccDataSize 			= 26;
	gMLC_ECC_16Bit.EncodeFlag			= HwECC_IREQ_MEEFG;
	gMLC_ECC_16Bit.DecodeFlag			= HwECC_IREQ_MEDFG;
	gMLC_ECC_16Bit.ErrorNum 			= 16;
	gMLC_ECC_16Bit.All_FF_512_ECC_Code	= (U8 *)&ALL_FF_ECC_BCH_16BIT_512;

	gMLC_ECC_24Bit.EccDataSize 			= 42;
	gMLC_ECC_24Bit.EncodeFlag			= HwECC_IREQ_MEEFG;
	gMLC_ECC_24Bit.DecodeFlag			= HwECC_IREQ_MEDFG;
	gMLC_ECC_24Bit.ErrorNum 			= 24;
	gMLC_ECC_24Bit.All_FF_512_ECC_Code	= (U8 *)&ALL_FF_ECC_BCH_24BIT_512;
	#endif

	memset( gTCC_MTD_IO_TempBuffer, 0xFF, sizeof(gTCC_MTD_IO_TempBuffer) );
}

int __devinit tcc_nand_probe(struct platform_device *pdev)
{
	struct 					tcc_nand_info *info;
	struct 					tcc_nand_platform_data *pdata = pdev->dev.platform_data;
	unsigned int 			err, i, nMediaNum = 0;
	
	//printk("************************* %s ********************\n",__FUNCTION__);

	/* Allocate memory (with set to zero) for MTD device structure and private data */
	info = kzalloc(sizeof(struct tcc_nand_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->mtd.name		= dev_name(&pdev->dev);
	info->dev 			= &pdev->dev;
	nand_platform_data	= pdata;

	TCC_MTD_IO_HwInit();

	#if !defined( __USE_MTD_EIL__)
	nMediaNum = 1;
	#else
	TCC_MTD_IO_DEVINFO 		sDevInfo;

	for ( i = 0; i < 4; ++i )
	{
		if ( TCC_MTD_IO_locGetDeviceInfo( i, &sDevInfo ) == SUCCESS )
			++nMediaNum;
	}
	#endif	

	info->glue_info.gTCC_MTD_PageBuffer = gTCC_MTD_PageBuffer;
	//info->glue_info.gTCC_GoldenBadTable = kmalloc( 512 * 4, GFP_KERNEL);
	info->glue_info.gTCC_GoldenBadTable = (unsigned	 int **) kmalloc( sizeof(unsigned int*) * nMediaNum, GFP_KERNEL );
	for( i = 0; i < nMediaNum; ++i )
	{
		info->glue_info.gTCC_GoldenBadTable[i] = (unsigned int*) kmalloc( 512 * 4, GFP_KERNEL );

		if (!info->glue_info.gTCC_GoldenBadTable[i])
		{
			err = -ENOMEM;
			goto out_free_info;
		}
	}	
		
	info->glue_info.tcc_mtd_LPT		= (unsigned long*)kmalloc( (TCC_MTD_MAX_LPT_SIZE * 4), GFP_KERNEL );

	if (!info->glue_info.tcc_mtd_LPT)
	{
		err = -ENOMEM;
		goto out_free_info;
	}

	info->glue_info.tcc_mtd_LPT_CS	= (unsigned char*)kmalloc( (TCC_MTD_MAX_LPT_SIZE * 1), GFP_KERNEL );	
	if( !info->glue_info.tcc_mtd_LPT_CS )
	{
		err = -ENOMEM;
		goto out_free_info;
	}

	TCC_MTD_IO_Init(info);

	/* Scan to find existence of the device */
	if (nand_scan(&info->mtd, 1)) 		// Chip Limit: 1
	{
		err = -ENXIO;
		goto out_free_info;
	}

#ifdef CONFIG_MTD_PARTITIONS
	err = add_mtd_partitions(&info->mtd, pdata->parts, pdata->nr_parts);
#endif
	return 0;

out_free_info:
	for( i = 0; i < nMediaNum; ++i )
	{
		if( info->glue_info.gTCC_GoldenBadTable[i] )
			kfree( info->glue_info.gTCC_GoldenBadTable[i] );
	}

	if( info->glue_info.gTCC_GoldenBadTable )
		kfree( info->glue_info.gTCC_GoldenBadTable );		
	
	if (info->glue_info.tcc_mtd_LPT)
		kfree(info->glue_info.tcc_mtd_LPT);
	kfree(info);
	return err;
}

static int __devexit tcc_nand_remove(struct platform_device *pdev)
{
	struct tcc_nand_info *info = dev_get_drvdata(&pdev->dev);

	if (info) {
#ifdef CONFIG_MTD_PARTITIONS
		if (info->parts)
			del_mtd_partitions(&info->mtd);
#endif
		if( info->glue_info.gTCC_GoldenBadTable[1] )
		{
			kfree( info->glue_info.gTCC_GoldenBadTable[0] );
			kfree( info->glue_info.gTCC_GoldenBadTable[1] );
		}
		else
			kfree( info->glue_info.gTCC_GoldenBadTable[0] );
		
		if(info->glue_info.gTCC_GoldenBadTable)
			kfree(info->glue_info.gTCC_GoldenBadTable);
		if (info->glue_info.tcc_mtd_LPT)
			kfree(info->glue_info.tcc_mtd_LPT);
		kfree(info);
	}
	return 0;
}

static struct platform_driver tcc_nand_driver = {
	.probe	= tcc_nand_probe,
	.remove	= __devexit_p(tcc_nand_remove),
	.driver	= {
		.name	= "tcc_nand",
	},
};

void TCC_MTD_NAND_REGISTER(unsigned long version, const TCC_MTD_GLUE_DRV_T *pDriver)
{
	//printk("************************* %s ********************\n",__FUNCTION__);
	// check MTD GLUE Driver Version
	if(version == MTD_GLUE_DRIVER_VERSION)
	{
		s_pTccMtdGlueDrv = pDriver;
		platform_driver_register(&tcc_nand_driver);
	}
	else
	{
		printk("%s: MTD Glue Driver mismatch!!! (drv=%X/needed=%X)\n",__FUNCTION__,(unsigned int)version,MTD_GLUE_DRIVER_VERSION);
	}
}
EXPORT_SYMBOL(TCC_MTD_NAND_REGISTER);

static int __init tcc_nand_init(void)
{
	//return platform_driver_register(&tcc_nand_driver);
	return 0;
}
module_init(tcc_nand_init);

static void __exit tcc_nand_exit(void)
{
	platform_driver_unregister(&tcc_nand_driver);
}
module_exit(tcc_nand_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Telechips, Hosi.Roh");
MODULE_DESCRIPTION("Board-specific glue layer for NAND flash on Telechips Dev board");
