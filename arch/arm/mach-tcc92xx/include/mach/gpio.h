/*
 * arch/arm/mach-tcc92xx/include/mach/gpio.h
 *
 * Written by <linux@telechips.com>
 * Modified: March 10, 2009
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __ASM_ARCH_TCC92XX_GPIO_H
#define __ASM_ARCH_TCC92XX_GPIO_H

#define GPIO_PORTA	(0 << 5)
#define GPIO_PORTB	(1 << 5)
#define GPIO_PORTC	(2 << 5)
#define GPIO_PORTD	(3 << 5)
#define GPIO_PORTE	(4 << 5)
#define GPIO_PORTF	(5 << 5)
#define GPIO_PORTEXT1	(6 << 5)
#define GPIO_PORTEXT2	(7 << 5)
#define GPIO_PORTEXT3	(8 << 5)

#define TCC_GPA(x)	(GPIO_PORTA | (x))
#define TCC_GPB(x)	(GPIO_PORTB | (x))
#define TCC_GPC(x)	(GPIO_PORTC | (x))
#define TCC_GPD(x)	(GPIO_PORTD | (x))
#define TCC_GPE(x)	(GPIO_PORTE | (x))
#define TCC_GPF(x)	(GPIO_PORTF | (x))
#define TCC_GPG(x)	(GPIO_PORTF | (x)) // not used
#define TCC_GPEXT1(x)	(GPIO_PORTEXT1 | (x))
#define TCC_GPEXT2(x)	(GPIO_PORTEXT2 | (x))
#define TCC_GPEXT3(x)	(GPIO_PORTEXT3 | (x))
#define TCC_GPEXT4(x)	0
#define TCC_GPEXT5(x)	0

#define GPIO_PULLUP	0x0100
#define GPIO_PULLDOWN	0x0200
#define GPIO_PULL_DISABLE 0x0400
#define GPIO_CD_BITMASK	0x0F000000
#define GPIO_CD_SHIFT	24
#define GPIO_CD(x)	(((x) + 1) << GPIO_CD_SHIFT)
#define GPIO_FN_BITMASK	0xF0000000
#define GPIO_FN_SHIFT	28
#define GPIO_FN(x)	(((x) + 1) << GPIO_FN_SHIFT)

#define gpio_get_value	__gpio_get_value
#define gpio_set_value	__gpio_set_value
#define gpio_cansleep	__gpio_cansleep

int gpio_to_irq(unsigned gpio);
int tcc_gpio_config(unsigned gpio, unsigned flags);

#define ARCH_NR_GPIOS	TCC_GPEXT3(15)

struct board_gpio_irq_config {
	unsigned gpio;
	unsigned irq;
};

extern struct board_gpio_irq_config *board_gpio_irqs;

#include <asm-generic/gpio.h>

#endif
