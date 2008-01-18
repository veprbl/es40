/* ES40 emulator.
 * Copyright (C) 2007-2008 by the ES40 Emulator Project
 *
 * WWW    : http://sourceforge.net/projects/es40
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
 * $Id: datatypes.h,v 1.15 2008/01/18 20:58:20 iamcamiel Exp $
 *
 * X-1.15       Camiel Vanderhoeven                             18-JAN-2008
 *      Replaced sext_64 inlines with sext_u64_<bits> inlines for
 *      performance reasons (thanks to David Hittner for spotting this!)
 *
 * X-1.14       Camiel Vanderhoeven                             14-DEC-2007
 *      Added sext_32.
 *
 * X-1.13       Camiel Vanderhoeven                             2-DEC-2007
 *      Added various bitmask operations as inline functions.
 *
 * X-1.12       Camiel Vanderhoeven                             1-DEC-2007
 *      Use __int8, __int16 and __int32 on windows, instead of char, short, 
 *      long. This makes us compatible with SDL.
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

typedef unsigned __int8     u8;
typedef unsigned __int16    u16;
typedef unsigned __int32     u32;
typedef unsigned __int64  u64;

typedef signed __int8       s8;
typedef signed __int16      s16;
typedef signed __int32       s32;
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

inline u64 keep_bits_64(u64 x, int hibit, int lobit)
{
    u64 r = x;
    if (hibit<63)
      r &=  ((X64(1)<<(hibit+1))-X64(1));
    r   &= ~((X64(1)<<(lobit))-X64(1));
    return r;
}

inline u64 make_mask_64(int hibit, int lobit)
{
    u64 r = X64(ffffffffffffffff);
    if (hibit<63)
      r &=  ((X64(1)<<(hibit+1))-X64(1));
    r   &= ~((X64(1)<<(lobit))-X64(1));
    return r;
}

inline u64 move_bits_64(u64 x, int hibit, int lobit, int destlobit)
{
    u64 r = x;
    if (hibit<63)
      r &=  ((X64(1)<<(hibit+1))-X64(1));
    r   &= ~((X64(1)<<(lobit))-X64(1));
    r = (lobit<destlobit)?r<<(destlobit-lobit):r>>(lobit-destlobit);
    return r;
}

inline u64 extend_bit_64(u64 x, int hibit, int lobit, int srcbit)
{
   if (x & (X64(1)<<srcbit))
   {
     u64 r = X64(ffffffffffffffff);
     if (hibit<63)
        r &=  ((X64(1)<<(hibit+1))-X64(1));
      r   &= ~((X64(1)<<(lobit))-X64(1));
    return r;
   }
   return 0;
}

/**
 * Sign-extend an 8-bit value to 64 bits.
 **/
inline u64 sext_u64_8(u64 a) 
{
  return  (((a) & X64(0000000000000080)) ?
           ((a) | X64(ffffffffffffff00)) :
           ((a) & X64(00000000000000ff)));
}

/**
 * Sign-extend a 12-bit value to 64 bits.
 **/
inline u64 sext_u64_12(u64 a)
{
  return (((a) & X64(0000000000000800)) ?
          ((a) | X64(fffffffffffff000)) :
          ((a) & X64(0000000000000fff)));
}

/**
 * Sign-extend a 13-bit value to 64 bits.
 **/
inline u64 sext_u64_13(u64 a)
{
  return (((a) & X64(0000000000001000)) ?
          ((a) | X64(ffffffffffffe000)) :
          ((a) & X64(0000000000001fff)));
}

/**
 * Sign-extend an 16-bit value to 64 bits.
 **/
inline u64 sext_u64_16(u64 a)
{
  return (((a) & X64(0000000000008000)) ?
          ((a) | X64(ffffffffffff0000)) :
          ((a) & X64(000000000000ffff)));
}

/**
 * Sign-extend a 21-bit value to 64 bits.
 **/
inline u64 sext_u64_21(u64 a)
{
  return (((a) & X64(0000000000100000)) ? 
          ((a) | X64(ffffffffffe00000)) : 
          ((a) & X64(00000000001fffff)));
}

/**
 * Sign-extend a 32-bit value to 64 bits.
 **/
inline u64 sext_u64_32(u64 a)
{
  return (((a) & X64(0000000080000000)) ?
          ((a) | X64(ffffffff00000000)) :
          ((a) & X64(00000000ffffffff)));
}

/**
 * Sign-extend a 48-bit value to 64 bits.
 **/
inline u64 sext_u64_48(u64 a)
{
  return (((a) & X64(0000800000000000)) ?
          ((a) | X64(ffff000000000000)) :
          ((a) & X64(0000ffffffffffff)));
}

inline bool test_bit_64(u64 x, int bit)
{
  return (x & (X64(1)<<bit))?true:false;
}

inline u32 keep_bits_32(u32 x, int hibit, int lobit)
{
    u32 r = x;
    if (hibit<31)
      r &=  ((1<<(hibit+1))-1);
    r   &= ~((1<<(lobit))-1);
    return r;
}

inline u32 make_mask_32(int hibit, int lobit)
{
    u32 r = 0xffffffff;
    if (hibit<31)
      r &=  ((1<<(hibit+1))-1);
    r   &= ~((1<<(lobit))-1);
    return r;
}

inline u32 move_bits_32(u32 x, int hibit, int lobit, int destlobit)
{
    u32 r = x;
    if (hibit<31)
      r &=  ((1<<(hibit+1))-1);
    r   &= ~((1<<(lobit))-1);
    r = (lobit<destlobit)?r<<(destlobit-lobit):r>>(lobit-destlobit);
    return r;
}

inline u32 extend_bit_32(u32 x, int hibit, int lobit, int srcbit)
{
   if (x & (1<<srcbit))
   {
     u32 r = 0xffffffff;
     if (hibit<31)
        r &=  ((1<<(hibit+1))-1);
      r   &= ~((1<<(lobit))-1);
    return r;
   }
   return 0;
}

inline bool test_bit_32(u32 x, int bit)
{
  return (x & (1<<bit))?true:false;
}

/**
 * Sign-extend a 24-bit value to 32 bits.
 **/
inline u32 sext_u32_24(u32 a)
{
  return (((a) & 0x00800000) ?
          ((a) | 0xff000000) :
          ((a) & 0x00ffffff));
}

#endif //INCLUDED_DATATYPES_H
