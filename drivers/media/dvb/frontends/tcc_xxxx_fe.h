/*
 *  Driver for TCC Frontend
 *
 *  Written by C2-G1-3T
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.=
 */

#ifndef TCC_XXXX_FE_H
#define TCC_XXXX_FE_H

#include <linux/dvb/frontend.h>
#include "dvb_frontend.h"

#if defined(CONFIG_DVB_TCC_XXXX) || (defined(CONFIG_DVB_TCC_XXXX_MODULE) && \
defined(MODULE))
extern struct dvb_frontend* tcc_xxxx_fe_ofdm_attach(void);
extern struct dvb_frontend* tcc_xxxx_fe_qpsk_attach(void);
extern struct dvb_frontend* tcc_xxxx_fe_qam_attach(void);
#else
static inline struct dvb_frontend *tcc_xxxx_fe_ofdm_attach(void)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
static inline struct dvb_frontend *tcc_xxxx_fe_qpsk_attach(void)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
static inline struct dvb_frontend *tcc_xxxx_fe_qam_attach(void)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif /* CONFIG_DVB_TCC_XXXX */

#endif // TCC_XXXX_FE_H
