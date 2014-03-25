#ifndef	_TCC_LCD_INTERFACE_H_
#define	_TCC_LCD_INTERFACE_H_

extern void LCD_Driver_mdelay(unsigned int delay);
extern void LCD_Driver_udelay(unsigned int usec);
extern void LCD_OS_DELAY(unsigned int delay);

extern void LCDC_IO_Set (char lcdctrl_num, unsigned bit_per_pixel);

extern void LCDC_IO_Disable (char lcdctrl_num, unsigned bit_per_pixel);

typedef enum{
	LCD_GPSB_PORT0 = 0,
	LCD_GPSB_PORT1,
	LCD_GPSB_PORT2,
	LCD_GPSB_PORT3,
	LCD_GPSB_PORT4,
	LCD_GPSB_PORT5,
	LCD_GPSB_PORT6,
	LCD_GPSB_PORT7,
	LCD_GPSB_PORT8,
	LCD_GPSB_PORT9,
	LCD_GPSB_PORT10,
	LCD_GPSB_PORT11,
	LCD_GPSB_PORT12,
	LCD_GPSB_PORTMAX
}DEV_LCD_GPSB_PORT;


typedef struct{
	unsigned int clock;
	unsigned int mode;
	DEV_LCD_GPSB_PORT port;
	unsigned int data_length;
}DEV_LCD_SPIO_Config_type;


/* -------------------------------------------------
GPSB Interface 사용
clock : GPSB clock  //100hz 단위.

---------------------------------------------------*/
extern void DEV_LCD_SPIO_Config(DEV_LCD_SPIO_Config_type *Lcd_gpsg);


extern unsigned int DEV_LCD_SPIO_TxRxData(void *pTxBuf, void *pRxBuf, unsigned uLength);


/* -------------------------------------------------
I2C Interface 사용
clock : GPSB clock  //100hz 단위.
---------------------------------------------------*/

/* I2C Controller */
#define GPA_BASE 			0xF0102000
#define I2C_CH0_BASE		0xF0530000	/* I2C master ch0 base address */
#define I2C_IRQSTR			0xF05300C0	/* only I2C */
#define CH1_BASE_OFFSET		0x40		/* I2C & SMU_I2C ch1 base offset */

/* SMU_I2C */
#define SMU_I2C_CH0_BASE	0xF0405000	/* SMU_I2C master ch0 base address */
#define I2C_ICLK_ADDRESS	0xF0405080	/* only SMU_I2C */

/* read/write bit */
#define I2C_WR	0
#define I2C_RD	1

enum {
	I2C_CH0 = 0,
	I2C_CH1,
	SMU_I2C_CH0,
	SMU_I2C_CH1,
};

extern int i2c_xfer(unsigned char slave_addr, 
		unsigned char out_len, unsigned char* out_buf, 
		unsigned char in_len, unsigned char* in_buf, 
		int ch);


#endif //_TCC_LCD_INTERFACE_H_

