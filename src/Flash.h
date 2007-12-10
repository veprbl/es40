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
 * X-1.8        Camiel Vanderhoeven                             10-DEC-2007
 *      Use configurator.
 *
 * X-1.7        Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.6        Camiel Vanderhoeven                             16-FEB-2007
 *      Added SaveStateF and RestoreStateF functions.
 *
 * X-1.5        Camiel Vanderhoeven                             12-FEB-2007
 *      Formatting.
 *
 * X-1.4        Camiel Vanderhoeven                             12-FEB-2007
 *      Added comments.
 *
 * X-1.3        Camiel Vanderhoeven                             7-FEB-2007
 *      Added comments.
 *
 * X-1.2        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.1        Camiel Vanderhoeven                             19-JAN-2007
 *      Initial version in CVS.
 *
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#if !defined(INCLUDED_FLASH_H)
#define INCLUDED_FLASH_H

#include "SystemComponent.h"
#include "Configurator.h"

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
  CFlash(CConfigurator * cfg, class CSystem * c);
  virtual ~CFlash();
  virtual void SaveState(FILE * f);
  virtual void RestoreState(FILE * f);
  virtual void SaveStateF();
  virtual void RestoreStateF();

 protected:
  u8 Flash[2*1024*1024];
  int mode;
};

extern CFlash * theSROM;

#endif // !defined(INCLUDED_FLASH_H)
