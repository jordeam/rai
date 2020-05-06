/* pbit.h - some macros to manipulate port bits */

/* Copyright (C) 2003 José Roberto Boffino de Almeida Monteiro
 *   <jrm@member.fsf.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 *
 */

/* $Id: pbit.h,v 1.5 2005/04/01 18:37:03 betoes Exp $ */

#ifndef _pbit_h
#define _pbit_h

#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000

/* Port bit operations: */
#define HIGH(pbit) _setpbit(pbit)
#define _setpbit(port,bit) port->BSRRL = BIT ## bit
#define LOW(pbit) _clrpbit(pbit)
#define _clrpbit(port,bit) port->BSRRH = BIT ## bit

#define PIN(pbit) _pbit(pbit)
#define _pbit(port,bit) BIT ## bit

#define PORT(pbit) _pbitoutport(pbit)
#define _pbitoutport(port,bit) port

/* reading pin input */
#define READBIT(pbit) _getpbit(pbit)
#define _getpbit(port,bit) (port->IDR & BIT ## bit)

#endif
