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
 * Standard includes.
 * Include file for standard system include files,
 * or project specific include files that are used frequently, but
 * are changed infrequently.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
 **/

#if !defined(INCLUDED_STDAFX_H)
#define INCLUDED_STDAFX_H

#include "datatypes.h"
#include "endian.h"

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_DEPRECATE 1
#if _MSC_VER < 1400
#define WINVER 0x0400
#else
#define WINVER 0x0501
#endif
#include <afx.h>
#include <winsock.h>
#define strcasecmp(a,b) _stricmp(a,b)
#define strncasecmp(a,b,c) _strnicmp (a,b,c)

inline void sleep_ms(DWORD ms)
{
	Sleep(ms);
}

#else

#define _strdup strdup
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

inline void sleep_ms(int ms)
{
	usleep(ms*1000);
}

#endif

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <ctype.h>

#if (defined(_MSC_VER) && (_MSC_VER < 1400)) || !defined(_WIN32)
inline void gmtime_s(struct tm * t1, time_t * t2)
{
  struct tm * t3;
  t3 = gmtime(t2);
  memcpy(t1,t3,sizeof(struct tm));
}
#endif

#if defined(_WIN32) || defined(__DECCXX)
inline bool isblank(char c)
{
  if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' )
    return true;
  return false;
}
#endif

inline char printable(char c)
{
  if (isprint(c))
	  return c;
  return '.';
}

#endif // !defined(INCLUDED_STDAFX_H)
