/* ES40 emulator.
 * Copyright (C) 2007 by the ES40 Emulator Project
 *
 * WWW    : http://sourceforge.net/projects/es40
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
 * Contains the definitions for the emulated Ali M1543C USB chipset part.
 *
 * X-1.2        Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.1        Camiel Vanderhoeven                             10-DEC-2007
 *      Initial version in CVS; this part was split off from the CAliM1543C
 *      class.
 **/

#if !defined(INCLUDED_ALIM1543C_USB_H_)
#define INCLUDED_ALIM1543C_USB_H

#include "PCIDevice.h"
#include "gui/gui.h"
#include "Configurator.h"

class CAliM1543C_usb : public CPCIDevice  
{
 public:
  virtual int SaveState(FILE * f);
  virtual int RestoreState(FILE * f);

  CAliM1543C_usb(CConfigurator * cfg, class CSystem * c, int pcibus, int pcidev);
  virtual ~CAliM1543C_usb();

 private:

  u64 usb_config_read(u64 address, int dsize);
  void usb_config_write(u64 address, int dsize, u64 data);

  // The state structure contains all elements that need to be saved to the statefile.
  struct SAliM1543C_usbState {
  } state;

};

#endif // !defined(INCLUDED_ALIM1543C_USB_H)
