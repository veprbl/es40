/* ES40 emulator.
 * Copyright (C) 2007 by Camiel Vanderhoeven
 *
 * Website: www.camicom.com
 * E-mail : camiel@camicom.com
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * Although this is not required, the author would appreciate being notified of, 
 * and receiving any modifications you may make to the source code that might serve
 * the general public.
 */

/** 
 * \file
 * Contains the datatype definitions for use with Microsoft Visual C++ and Linux.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
 **/

#if !defined(INCLUDED_DATATYPES_H)
#define INCLUDED_DATATYPES_H

#if defined(_WIN32)

#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned long
#define u64 unsigned __int64

#define s8 signed char
#define s16 signed short
#define s32 signed long
#define s64 signed __int64

#define X64(a) 0x##a##ui64
#define LL "I64"

#else // defined(_WIN32)

#include <stdint.h>
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define s8 int8_t
#define s16 int16_t
#define s32 int32_t
#define s64 int64_t

#define X64(a) 0x##a##ll
#define LL "ll"

#endif // defined(_WIN32)

#define X32(a) 0x##a
#define X16(a) 0x##a
#define X8(a) 0x##a


#endif //INCLUDED_DATATYPES_H
