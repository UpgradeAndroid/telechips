#ifndef __ITCC89XX_TSIF_S_H__
#define __ITCC89XX_TSIF_S_H__

ITV_EXPORT(int, tcc_tsif_s_init_proc, (void));
ITV_EXPORT(int, tcc_tsif_s_init, (unsigned char **, unsigned int, unsigned int, unsigned int, unsigned int, void (*callback)(void *), void *));
ITV_EXPORT(void, tcc_tsif_s_deinit, (void));
ITV_EXPORT(int, tcc_tsif_s_get_pos, (void));
ITV_EXPORT(int, tcc_tsif_s_start, (void));
ITV_EXPORT(int, tcc_tsif_s_stop, (void));

#endif /* __ITCC89XX_TSIF_S_H__ */
