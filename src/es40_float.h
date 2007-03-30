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
 * Contains floating point conversion code used by AlphaCPU.cpp.
 * We've chosen to keep the floating point values as 64-bit integers in the floating
 * point registers, and to convert them to/from the host's native floating point 
 * format when required.
 *
 * X-1.3        Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.2        Camiel Vanderhoeven                             28-MAR-2007
 *      Properly put in all 64-bit constants as X64(...) instead of 0x...
 *
 * X-1.1        Camiel Vanderhoeven                             18-MAR-2007
 *      File created to support basic floating point operations.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#include <math.h>

/**
 * VAX (G or F) floating point to host conversion.
 * Converts the register-form of F and G foating point values to a double.
 **/

inline double v2f(u64 val)
{
  int s = (val & X64(8000000000000000))?1:0;
  int e = (val & X64(7ff0000000000000))>>52;
  s64 f = (val & X64(000fffffffffffff));

  double res;

  if (e==0)
    res = 0.0;
  else
    res = (s?-1.0:1.0) * ldexp (1.0 + ((double)f / (double)((s64)X64(10000000000000))), e-1023) * 4.0;

  printf("v2f: %016" LL "x ==> %g\n", val, res);

  return res;
}

/**
 * IEEE (S or T) floating point to host conversion.
 * Converts the register-form of S and T foating point values to a double.
 **/

inline double i2f(u64 val)
{
  int s = (val & X64(8000000000000000))?1:0;
  int e = (val & X64(7ff0000000000000))>>52;
  s64 f = (val & X64(000fffffffffffff));
  double res;

  if (e==2047) {
    if (f)
      res = (s?-0.0:0.0) / 0.0;	// NaN
    else
      res = (s?-1.0:1.0) / 0.0;	// +/- Inf
  } else if (e==0) {
    if (f)
      res = (s?-1.0:1.0) * ldexp((double)f / (double)((s64)X64(10000000000000)), -1022);
    else
      res = (s?-1.0:1.0) * 0.0;
  } else {
      res = (s?-1.0:1.0) * ldexp (1.0 + ((double)f / (double)((s64)X64(10000000000000))), e-1023);
  }

  printf("i2f: %016" LL "x ==> %g\n", val, res);

  return res;
}


/**
 * Check IEEE floating point value for NaN.
 **/

inline bool i_isnan(u64 val)
{
  int e = (val & X64(7ff0000000000000))>>52;
  s64 f = (val & X64(000fffffffffffff));

  return (e==2047) && f;
}

/**
 * Host to VAX (G or F) floating point conversion.
 * Converts a double to the register-form of F and G foating point values.
 **/

inline u64 f2v(double val)
{
  double v = val * 0.25;
  int s = (v<0.0)?1:0;
  if (s) v *= -1.0;
  int e = log(v) / log(2);
  double fr = v / pow(2,e) - 1;

  e += 1023;

  u64 f = fr * (double)X64(10000000000000);

  f =                (s?X64(800000000000000):0) | 
	  (((u64)e << 52) & X64(7ff0000000000000)) |
	  (f              & X64(000fffffffffffff));

  printf("f2v: %016" LL "x <== %g\n", f, val);
  return f;
}

/**
 * Host to IEEE (S or T) floating point conversion.
 * Converts a double to the register-form of S and T foating point values.
 **/

inline u64 f2i(double val)
{
  double v = val;
  int s = (v<0.0)?1:0;
  if (s) v *= -1.0;
  int e = log(v) / log(2);
  double fr = v / pow(2,e) - 1;

  e += 1023;
  
  u64 f = fr * (double)X64(10000000000000);

  f =                (s?X64(800000000000000):0) | 
	  (((u64)e << 52) & X64(7ff0000000000000)) |
	  (f              & X64(000fffffffffffff));

  printf("f2i: %016" LL "x <== %g\n", f, val);
  return f;
}

/**
 * Map an 8-bit VAX (F) exponent to an 11-bit VAX (G) exponent.
 **/

inline u64 map_f(u32 val)
{
   if (val==0)
     return 0;
   else if (val & 0x80)
     return (val & 0x7f) | 0x400;
   else
     return (val & 0x7f) | 0x380;
}

/**
 * Map an 8-bit IEEE (S) exponent to an 11-bit IEEE (T) exponent.
 **/

inline u64 map_s(u32 val)
{
   if (val==0)
     return 0;
   else if (val==0xff)
     return 0x7ff;
   else if (val & 0x80)
     return (val & 0x7f) | 0x400;
   else
     return (val & 0x7f) | 0x380;
}

/**
 * Perform the VAX-byte ordering swap necessary to load 32-bit VAX (F) floating point values from memory.
 **/

inline u32 swap_f(u32 val)
{
  return ((val & 0x0000ffff) << 16) | ((val & 0xffff0000) >> 16); 
}

/**
 * Perform the VAX-byte ordering swap necessary to load 64-bit VAX (G) floating point values from memory.
 **/

inline u64 swap_g(u64 val)
{
  return ((val & X64(0000ffff0000ffff)) << 16) | ((val & X64(ffff0000ffff0000)) >> 16);
}

/**
 * VAX 32-bit to 64-bit (F to G) floating point conversion.
 **/

inline u64 f2g(u32 val)
{
  return  ((val & X64(80000000)) << 32)		// sign
	| (map_f((val>>23) & 0xff) << 52)	// exp
	| ((val & X64(  7fffff)) << 29);
}

/**
 * IEEE 32-bit to 64-bit (S to T) floating point conversion.
 **/

inline u64 s2t(u32 val)
{
  return  ((val & X64(80000000)) << 32)		// sign
	| (map_s((val>>23) & 0xff) << 52)	// exp
	| ((val & X64(  7fffff)) << 29);
}

/**
 * VAX 64-bit to 32-bit (G to F) floating point conversion.
 **/

inline u32 g2f(u64 val)
{
  return ((u32)(val>>32) & 0xc0000000) |
	 ((u32)(val>>29) & 0x3fffffff);
}

/**
 * IEEE 64-bit to 32-bit (F to G) floating point conversion.
 **/

inline u32 t2s(u64 val)
{
  return ((u32)(val>>32) & 0xc0000000) |
	 ((u32)(val>>29) & 0x3fffffff);
}
