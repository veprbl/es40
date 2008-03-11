/* ES40 emulator.
 * Copyright (C) 2007-2008 by Camiel Vanderhoeven
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
 * $Id: StdAfx.h,v 1.28 2008/03/11 09:10:40 iamcamiel Exp $
 *
 * X-1.28       Camiel Vanderhoeven                             11-MAR-2008
 *      Named, debuggable mutexes.
 *
 * X-1.27       Camiel Vanderhoeven                             05-MAR-2008
 *      Multi-threading version.
 *
 * X-1.26       Alex                                            20-FEB-2008
 *      GNU compiler support on Windows.
 *
 * X-1.25       Camiel Vanderhoeven                             19-JAN-2008
 *      Run CPU in a separate thread if CPU_THREADS is defined.
 *      NOTA BENE: This is very experimental, and has several problems.
 *
 * X-1.24       Fang Zhe                                        05-JAN-2008
 *      Do 64-bit file I/O properly for FreeBSD and OS X.
 *
 * X-1.23       Camiel Vanderhoeven                             04-JAN-2008
 *      Put in definitions to handle 64-bit file I/O OS-independently.
 *
 * X-1.22       Fang Zhe                                        03-JAN-2008
 *      Put es40_endian.h after es40_debug.h as it uses the FAILURE macro.
 *
 * X-1.20       Fang Zhe                                        03-JAN-2008
 *      Help compilation on Mac OS X and FreeBSD.
 *
 * X-1.19       Camiel Vanderhoeven                             02-JAN-2008
 *      Cleanup. 
 *
 * X-1.18       Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.17       Camiel Vanderhoeven                             1-DEC-2007
 *      Include stdlib.h by default. We don't do MFC, so replace afx.h 
 *      with windows.h 
 *
 * X-1.16       Camiel Vanderhoeven                             16-NOV-2007
 *      Removed winsock.
 *
 * X-1.15       Camiel Vanderhoeven                             14-NOV-2007
 *      Added es40_debug.h
 *
 * X-1.14       Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.13       Brian Wheeler                                   8-MAR-2007
 *      endian.h renamed to es40_endian.h
 *
 * X-1.12       Camiel Vanderhoeven                             1-MAR-2007
 *      Included endian.h to support the Solaris/SPARC port.
 *
 * X-1.11       Camiel Vanderhoeven                             28-FEB-2007
 *      Included datatypes.h
 *
 *
 * X-1.10       Camiel Vanderhoeven                             16-FEB-2007
 *   a) Changed header guards
 *   b) Re-design of #if-#else-#endif constructions to make more sense.
 *   c) Added strcasecmp and strncasecmp definitions for Win32.
 *   d) Added _strdup for non-Win32.
 *
 * X-1.9        Camiel Vanderhoeven                             12-FEB-2007
 *      Added comments.
 *
 * X-1.8        Camiel Vanderhoeven                             9-FEB-2007
 *      Added comments.
 *
 * X-1.7        Camiel Vanderhoeven                             7-FEB-2007
 *      Added isblank for DEC CXX (Linux/Alpha).
 *
 * X-1.6        Camiel Vanderhoeven                             3-FEB-2007
 *      Added sleep_ms.
 *
 * X-1.5        Camiel Vanderhoeven                             3-FEB-2007
 *      Added printable.
 *
 * X-1.4        Camiel Vanderhoeven                             3-FEB-2007
 *      Added is_blank for Win32.
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

#if !defined(INCLUDED_STDAFX_H)
#define INCLUDED_STDAFX_H

#include "datatypes.h"

#if defined(_WIN32)

#ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x400
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_DEPRECATE 1
#if _MSC_VER < 1400
#define WINVER 0x0400
#else
#define WINVER 0x0501
#endif
#include <windows.h>
#define strcasecmp(a,b) _stricmp(a,b)
#define strncasecmp(a,b,c) _strnicmp (a,b,c)

#include <process.h>
#else // not windows
#define _strdup strdup
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#include "signal.h"
#include <sys/wait.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if !defined(__APPLE__)  && !defined(__FreeBSD__)
#include <malloc.h>
#endif
#include <time.h>
#include <ctype.h>

#if (defined(_MSC_VER) && (_MSC_VER < 1400)) || !defined(_WIN32) || defined(__GNUWIN32__)
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
  if (isprint((unsigned char)c))
	  return c;
  return '.';
}

#include "es40_debug.h"
#include "es40_endian.h"

// Different OS'es define different functions to access 64-bit files
#if defined(_WIN32)
// Windows obviously does things differently...
#define fopen_large fopen
#define fseek_large _fseeki64
#define ftell_large _ftelli64
#define off_t_large __int64
#elif defined(__APPLE__) || defined(__FreeBSD__)
// OS X and FreeBSD do 64-bit access by default, and don't have the 64-bit versions
#define fopen_large fopen
#define fseek_large fseeko
#define ftell_large ftello
#define off_t_large off_t
#else
#define fopen_large fopen64
#define fseek_large fseeko64
#define ftell_large ftello64
#define off_t_large off64_t
#endif

#define POCO_NO_UNWINDOWS
#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <Poco/Semaphore.h>
#include <Poco/Mutex.h>
#include <Poco/Timestamp.h>

#include "Lock.h"

#endif // !defined(INCLUDED_STDAFX_H)
