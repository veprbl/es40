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
 * Contains macro's for byte-swapping on big-endian host architectures.
 *
 * X-1.2        Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.1        Brian Wheeler                                   8-MAR-2007
 *      Renamed this file from endian.h to es40_endian.h to avoid conflicts
 *      with system-include files.
 *
 * X-A1-1.2     Camiel Vanderhoeven                             7-MAR-2007
 *      Properly handle OpenVMS
 *
 * X-A1-1.1     Camiel Vanderhoeven                             1-MAR-2007
 *      File created to support the Solaris/SPARC port.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#if !defined(INCLUDED_ENDIAN_H)
#define INCLUDED_ENDIAN_H

#if !defined(ES40_LITTLE_ENDIAN) && !defined(ES40_BIG_ENDIAN)

#if defined(_WIN32) || defined(__VMS)
#define ES40_LITTLE_ENDIAN
#else // defined (_WIN32) || defined(__VMS)

#include <sys/param.h>
#if (defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN)) || defined(sparc)
#define ES40_BIG_ENDIAN
#else // assume little endian
#define ES40_LITTLE_ENDIAN
#endif

#endif // defined (_WIN32) || defined(__VMS)

#endif // !defined(ES40_LITTLE_ENDIAN) && !defined(ES40_BIG_ENDIAN)

#if defined(ES40_BIG_ENDIAN)

#define endian_64(x) (  (((x)&X64(00000000000000ff))<<56) |    \
                        (((x)&X64(000000000000ff00))<<40) |    \
                        (((x)&X64(0000000000ff0000))<<24) |    \
                        (((x)&X64(00000000ff000000))<<8)  |    \
                        (((x)&X64(000000ff00000000))>>8)  |    \
                        (((x)&X64(0000ff0000000000))>>24) |    \
                        (((x)&X64(00ff000000000000))>>40) |    \
                        (((x)&X64(ff00000000000000))>>56)   )

#define endian_32(x) (  (((x)&0x000000ff)<<24) |               \
                        (((x)&0x0000ff00)<<8)  |               \
                        (((x)&0x00ff0000)>>8)  |               \
                        (((x)&0xff000000)>>24)   )
                        
#define endian_16(x) (  (((x)&0x00ff)<<8)  |                   \
                        (((x)&0xff00)>>8)    )
                        
#else // defined(ES40_BIG_ENDIAN)

#define endian_64(x) (x)
#define endian_32(x) (x)
#define endian_16(x) (x)

#endif // defined(ES40_BIG_ENDIAN)

inline u64 endian_bits(u64 x, int numbits)
{
    switch(numbits)
    {
        case 64:
            return endian_64(x);
        case 32:
            return endian_32(x);
        case 16:
            return endian_16(x);
        default:
            return x;
    }
}
                        
#endif //INCLUDED_ENDIAN_H
