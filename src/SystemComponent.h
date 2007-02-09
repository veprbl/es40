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
 * 
 * SYSTEMCOMPONENT.H contains the definitions for the base class for devices that connect 
 * to the emulated Typhoon Chipset devices.
 */


#include "datatypes.h"
 

#if !defined(AFX_SYSTEMCOMPONENT_H__54D2AACC_ED4A_47C2_9E9F_1795A246E458__INCLUDED_)
#define AFX_SYSTEMCOMPONENT_H__54D2AACC_ED4A_47C2_9E9F_1795A246E458__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSystemComponent  
{
 public:
  virtual void RestoreState(FILE * f);
  virtual void SaveState(FILE * f);
  class CSystem * cSystem;

  CSystemComponent(class CSystem * system);
  virtual ~CSystemComponent();

  //=== abstract ===
  virtual u64 ReadMem(int index, u64 address, int dsize) {index; address; dsize; return 0;};
  virtual void WriteMem(int index, u64 address, int dsize, u64 data) {index; address; dsize; data;};
  virtual u8 ReadTIG(int index, int address) {index; address; return 0;};
  virtual void WriteTIG(int index, int address, u8 data) {index; address; data;};
  virtual void DoClock() {};
};

#endif // !defined(AFX_SYSTEMCOMPONENT_H__54D2AACC_ED4A_47C2_9E9F_1795A246E458__INCLUDED_)
