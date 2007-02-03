/** ES40 emulator.
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
 * 
 * stdafx.h : include file for standard system include files,
 * or project specific include files that are used frequently, but
 * are changed infrequently
 *
 **/

#if !defined(AFX_STDAFX_H__E45CEFB7_CE00_44EB_ADFF_B17622633B52__INCLUDED_)
#define AFX_STDAFX_H__E45CEFB7_CE00_44EB_ADFF_B17622633B52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define _CRT_SECURE_NO_DEPRECATE 1

#if _MSC_VER < 1400
#define WINVER 0x0400
#else
#define WINVER 0x0501
#endif

#include <stdio.h>
#ifdef _WIN32
#include <afx.h>
#include <winsock.h>
#endif
#include <string.h>
#include <malloc.h>
#include <time.h>

#if _MSC_VER < 1400
inline void gmtime_s(struct tm * t1, time_t * t2)
{
  struct tm * t3;
  t3 = gmtime(t2);
  memcpy(t1,t3,sizeof(struct tm));
}
#endif //_MSC_VER

#define ES40 1


// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__E45CEFB7_CE00_44EB_ADFF_B17622633B52__INCLUDED_)
