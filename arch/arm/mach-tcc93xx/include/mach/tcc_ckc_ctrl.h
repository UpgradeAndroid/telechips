/* linux/arch/arm/mach-tcc8900/include/mach/tcc_ckc_ctrl.h

 * Author: <linux@telechips.com>
 * Created: June 10, 2008
 * Description: Header for code common to all Telechips TCC8900/TCC83x machines.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _TCC_CKC_CTRL_H
#define _TCC_CKC_CTRL_H

#include <linux/tcc_ioctl.h>

extern int arm_changestack(void);
extern void arm_restorestack(unsigned int rst);

extern void ckc_set_peri(struct ckc_ioctl st);
extern int ckc_get_peri(struct ckc_ioctl st);
extern int ckc_set_peribus(struct ckc_ioctl st);
extern int ckc_get_peribus(struct ckc_ioctl st);
extern void ckc_set_periswreset(struct ckc_ioctl st);
extern void ckc_set_fbusswreset(struct ckc_ioctl st);
extern void ckc_set_cpu(struct ckc_ioctl st);
extern void ckc_set_smui2c(struct ckc_ioctl st);
extern unsigned int ckc_get_cpu(void);
extern unsigned int ckc_get_bus(void);
extern void ckc_get_validpllinfo(struct ckc_ioctl st);
extern void ckc_set_fbus(struct ckc_ioctl st);
extern int ckc_get_fbus(struct ckc_ioctl st);
extern void ckc_set_pmupower(struct ckc_ioctl st);
extern unsigned int ckc_get_pmupower(struct ckc_ioctl st);
extern struct ckc_ioctl ckc_get_clockinfo(struct ckc_ioctl st);
extern void ckc_set_changefbus(struct ckc_ioctl st);
extern void ckc_set_changemem(struct ckc_ioctl st);
extern void ckc_set_changecpu(struct ckc_ioctl st);
extern void ckc_set_ddipwdn(struct ckc_ioctl st);
extern int ckc_get_ddipwdn(struct ckc_ioctl st);
extern void ckc_set_etcblock(struct ckc_ioctl st);
extern void ckc_set_videobuscfgpwdn(struct ckc_ioctl st);
extern int  ckc_get_videobuscfgpwdn(struct ckc_ioctl st);
extern void ckc_set_videobuscfgswreset(struct ckc_ioctl st);
extern int ckc_ioctl(unsigned int cmd, unsigned long arg);

#endif /* _TCC_CKC_CTRL_H*/

