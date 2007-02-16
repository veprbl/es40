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
 * Contains the definitions for the emulated Flash ROM devices.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
 **/

#if !defined(INCLUDED_FLASH_H)
#define INCLUDED_FLASH_H

#include "SystemComponent.h"

/**
 * Emulated flash memory.
 * Flash memory is only used for storing configuration data (such as SRM console variables),
 * it is not used for firmware.
 **/

class CFlash : public CSystemComponent  
{
 public:
  virtual void WriteMem(int index, u64 address, int dsize, u64 data);
  virtual u64 ReadMem(int index, u64 address, int dsize);
  CFlash(class CSystem * c);
  virtual ~CFlash();
  virtual void SaveState(FILE * f);
  virtual void RestoreState(FILE * f);
  virtual void SaveStateF(char * fn);
  virtual void RestoreStateF(char * fn);

 protected:
  u8 Flash[2*1024*1024];
  int mode;
};

#endif // !defined(INCLUDED_FLASH_H)
