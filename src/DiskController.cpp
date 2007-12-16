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
 * Contains definitions for the disk controller base class.
 *
 * X-1.3        Camiel Vanderhoeven                             16-DEC-2007
 *      Include Disk.h, so children's destructors can be called.
 *
 * X-1.2        Camiel Vanderhoeven                             14-DEC-2007
 *      Delete children upon destruction.
 *
 * X-1.1        Camiel Vanderhoeven                             12-DEC-2007
 *      Initial version in CVS.
 **/

#include "StdAfx.h"
#include "DiskController.h"
#include "Disk.h"

CDiskController::CDiskController(CConfigurator * cfg, CSystem * c, int pcibus, int pcidev, int num_busses, int num_devices) : CPCIDevice(cfg,c,pcibus,pcidev)
{
  int i;

  num_bus = num_busses;
  num_dev = num_devices;

  disks = (CDisk**) malloc(num_bus*num_dev*sizeof(void*));


}

CDiskController::~CDiskController(void)
{
  int i;
  for (i=0;i<num_bus*num_dev;i++) 
  {
    if (disks[i])
    {
      delete disks[i];
      disks[i] = 0;
    }
  }
}

bool CDiskController::register_disk(class CDisk * dsk, int bus, int dev)
{
  if (bus>=num_bus) return false;
  if (dev>=num_dev) return false;

  disks[bus*num_bus+dev] = dsk;
  return true;
}

class CDisk * CDiskController::get_disk(int bus, int dev)
{
  if (bus>=num_bus) return 0;
  if (dev>=num_dev) return 0;

  return disks[bus*num_bus+dev];
}