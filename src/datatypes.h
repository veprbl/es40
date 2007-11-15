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
 * X-1.11       Camiel Vanderhoeven                             15-NOV-2007
 *      Use typedefs in stead of #define's
 *
 * X-1.10       Camiel Vanderhoeven                             14-NOV-2007
 *      Added BSD-style definitions.
 *
 * X-1.9        Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.8        Camiel Vanderhoeven                             1-MAR-2007
 *      Used typedef's instead of #define's (conflict with Solaris' libc)
 *
 * X-1.7        Camiel Vanderhoeven                             27-FEB-2007
 *      OpenVMS support added.
 *
 * X-1.6        Camiel Vanderhoeven                             16-FEB-2007
 *      Added definition of "LL" for printf and scanf statements.
 *
 * X-1.5        Camiel Vanderhoeven                             12-FEB-2007
 *      Added comments.
 *
 * X-1.4        Camiel Vanderhoeven                             7-FEB-2007
 *      Added comments.
 *
 * X-1.3        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.2        Brian Wheeler                                   3-FEB-2007
 *      Linux support added.
 *
 * X-1.1        Camiel Vanderhoeven                             19-JAN-2007
 *      Initial version in CVS.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#if !defined(INCLUDED_DATATYPES_H)
#define INCLUDED_DATATYPES_H

#if defined(_WIN32)

typedef unsigned char     u8;
typedef unsigned short    u16;
typedef unsigned long     u32;
typedef unsigned __int64  u64;

typedef signed char       s8;
typedef signed short      s16;
typedef signed long       s32;
typedef signed __int64    s64;

#define X64(a) 0x##a##ui64
#define LL "I64"

#else // defined(_WIN32)

#if defined(__VMS)
#include <inttypes.h>
#else
#include <stdint.h>
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define X64(a) 0x##a##ll
#define LL "ll"

#endif // defined(_WIN32)

typedef u8 u_int8_t;
typedef u16 u_int16_t;
typedef u32 u_int32_t;
typedef u64 u_int64_t;

typedef u8 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef s8 int8_t;
typedef s16 int16_t;
typedef s32 int32_t;
typedef s64 int64_t;

#define HAVE_U_INT8_T 1
#define HAVE_INT8_T 1
#define HAVE_U_INT16_T 1
#define HAVE_INT16_T 1
#define HAVE_U_INT32_T 1
#define HAVE_INT32_T 1
#define HAVE_U_INT64_T 1
#define HAVE_INT64_T 1


#define X32(a) 0x##a
#define X16(a) 0x##a
#define X8(a) 0x##a


#endif //INCLUDED_DATATYPES_H
