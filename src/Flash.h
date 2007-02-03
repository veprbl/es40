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
 * FLASH.H contains the definitions for the emulated Flash ROM devices.
 *
 **/

#if !defined(AFX_FLASH_H__150F29CB_DA72_455A_82C7_B0DE48DFAE52__INCLUDED_)
#define AFX_FLASH_H__150F29CB_DA72_455A_82C7_B0DE48DFAE52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SystemComponent.h"

class CFlash : public CSystemComponent  
{
 public:
  virtual void WriteMem(int index, u64 address, int dsize, u64 data);
  virtual u64 ReadMem(int index, u64 address, int dsize);
  CFlash(class CSystem * c);
  virtual ~CFlash();

 protected:
  u8 Flash[2*1024*1024];
  int mode;

 public:
  // Save component state to file
  virtual void SaveState(FILE * f);
 public:
  // Restore component state from file
  virtual void RestoreState(FILE * f);
};

#endif // !defined(AFX_FLASH_H__150F29CB_DA72_455A_82C7_B0DE48DFAE52__INCLUDED_)
