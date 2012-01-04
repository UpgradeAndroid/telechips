#ifndef TCC_VIOC_VIQE_INTERFACE_H_INCLUDED
#define TCC_VIOC_VIQE_INTERFACE_H_INCLUDED


 void TCC_VIQE_DI_Init(int scalerCh, unsigned int FrmWidth, unsigned int FrmHeight, int crop_top, int crop_bottom, int crop_left, int crop_right);
 void TCC_VIQE_DI_Run(int scalerCh, unsigned int FrmWidth, unsigned int FrmHeight, int crop_top, int crop_bottom, int crop_left, int crop_right);
 void TCC_VIQE_DI_DeInit(void);


#endif
