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
 * Contains the definitions for the emulated Port 80 device.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
 **/

#if !defined(AFX_PORT80_H__C93A4910_DB8A_4AF8_A890_5724EFB9C654__INCLUDED_)
#define AFX_PORT80_H__C93A4910_DB8A_4AF8_A890_5724EFB9C654__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SystemComponent.h"

/**
 * Emulated port 80.
 * Port 80 is a port without a real function, that is used to slow things down.
 * Since our emulator is slow enough already ;-) this port has no function at
 * all, but it needs to be there to avoid error messages about non-existing
 * hardware.
 **/

class CPort80 : public CSystemComponent  
{
 public:

  CPort80(class CSystem * c);
  virtual ~CPort80();
  virtual u64 ReadMem(int index, u64 address, int dsize);
  virtual void WriteMem(int index, u64 address, int dsize, u64 data);

 protected:
  u8 p80;	/**< Last value written.*/
};

#endif // !defined(AFX_PORT80_H__C93A4910_DB8A_4AF8_A890_5724EFB9C654__INCLUDED_)
