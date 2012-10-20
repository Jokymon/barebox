/*
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _IMX_REGS_H
#define _IMX_REGS_H

/* ------------------------------------------------------------------------
 *  Motorola IMX system registers
 * ------------------------------------------------------------------------
 */

# ifndef __ASSEMBLY__
# define __REG(x)	(*((volatile u32 *)(x)))
# define __REG16(x)     (*(volatile u16 *)(x))
# define __REG2(x,y)    (*(volatile u32 *)((u32)&__REG(x) + (y)))
# else
#  define __REG(x) (x)
#  define __REG16(x) (x)
#  define __REG2(x,y) ((x)+(y))
#endif

#ifdef CONFIG_ARCH_IMX1
# include <mach/imx1-regs.h>
#elif defined CONFIG_ARCH_IMX21
# include <mach/imx21-regs.h>
#elif defined CONFIG_ARCH_IMX27
# include <mach/imx27-regs.h>
#elif defined CONFIG_ARCH_IMX31
# include <mach/imx31-regs.h>
#elif defined CONFIG_ARCH_IMX35
# include <mach/imx35-regs.h>
#elif defined CONFIG_ARCH_IMX25
# include <mach/imx25-regs.h>
#elif defined CONFIG_ARCH_IMX51
# include <mach/imx51-regs.h>
#elif defined CONFIG_ARCH_IMX53
# include <mach/imx53-regs.h>
#elif defined CONFIG_ARCH_IMX6
# include <mach/imx6-regs.h>
#else
# error "unknown i.MX soc type"
#endif

/* There's a off-by-one betweem the gpio bank number and the gpiochip */
/* range e.g. GPIO_1_5 is gpio 5 under linux */
#define IMX_GPIO_NR(bank, nr)		(((bank) - 1) * 32 + (nr))

#define GPIO_PIN_MASK 0x1f

#define GPIO_PORT_SHIFT 5
#define GPIO_PORT_MASK (0x7 << GPIO_PORT_SHIFT)

#define GPIO_PORTA (0 << GPIO_PORT_SHIFT)
#define GPIO_PORTB (1 << GPIO_PORT_SHIFT)
#define GPIO_PORTC (2 << GPIO_PORT_SHIFT)
#define GPIO_PORTD (3 << GPIO_PORT_SHIFT)
#define GPIO_PORTE (4 << GPIO_PORT_SHIFT)
#define GPIO_PORTF (5 << GPIO_PORT_SHIFT)

#define GPIO_OUT   (1 << 8)
#define GPIO_IN    (0 << 8)
#define GPIO_PUEN  (1 << 9)

#define GPIO_PF    (1 << 10)
#define GPIO_AF    (1 << 11)

#define GPIO_OCR_SHIFT 12
#define GPIO_OCR_MASK (3 << GPIO_OCR_SHIFT)
#define GPIO_AIN   (0 << GPIO_OCR_SHIFT)
#define GPIO_BIN   (1 << GPIO_OCR_SHIFT)
#define GPIO_CIN   (2 << GPIO_OCR_SHIFT)
#define GPIO_GPIO  (3 << GPIO_OCR_SHIFT)

#define GPIO_AOUT_SHIFT 14
#define GPIO_AOUT_MASK (3 << GPIO_AOUT_SHIFT)
#define GPIO_AOUT     (0 << GPIO_AOUT_SHIFT)
#define GPIO_AOUT_ISR (1 << GPIO_AOUT_SHIFT)
#define GPIO_AOUT_0   (2 << GPIO_AOUT_SHIFT)
#define GPIO_AOUT_1   (3 << GPIO_AOUT_SHIFT)

#define GPIO_BOUT_SHIFT 16
#define GPIO_BOUT_MASK (3 << GPIO_BOUT_SHIFT)
#define GPIO_BOUT      (0 << GPIO_BOUT_SHIFT)
#define GPIO_BOUT_ISR  (1 << GPIO_BOUT_SHIFT)
#define GPIO_BOUT_0    (2 << GPIO_BOUT_SHIFT)
#define GPIO_BOUT_1    (3 << GPIO_BOUT_SHIFT)

#define GPIO_GIUS      (1<<16)

/* silicon revisions  */
#define IMX_CHIP_REV_1_0	0x10
#define IMX_CHIP_REV_1_1	0x11
#define IMX_CHIP_REV_1_2	0x12
#define IMX_CHIP_REV_1_3	0x13
#define IMX_CHIP_REV_2_0	0x20
#define IMX_CHIP_REV_2_1	0x21
#define IMX_CHIP_REV_2_2	0x22
#define IMX_CHIP_REV_2_3	0x23
#define IMX_CHIP_REV_3_0	0x30
#define IMX_CHIP_REV_3_1	0x31
#define IMX_CHIP_REV_3_2	0x32

#endif				/* _IMX_REGS_H */
