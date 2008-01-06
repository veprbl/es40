/* ES40 emulator.
 * Copyright (C) 2007-2008 by the ES40 Emulator Project
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
 * Contains code for the disk base class.
 *
 * $Id: Disk.cpp,v 1.8 2008/01/06 13:00:31 iamcamiel Exp $
 *
 * X-1.8        Camiel Vanderhoeven                             06-JAN-2008
 *      Set default blocksize to 2048 for cd-rom devices.
 *
 * X-1.7        Camiel Vanderhoeven                             06-JAN-2008
 *      Support changing the block size (required for SCSI, ATAPI).
 *
 * X-1.6        Camiel Vanderhoeven                             02-JAN-2008
 *      Cleanup.
 *
 * X-1.5        Camiel Vanderhoeven                             29-DEC-2007
 *      Fix memory-leak.
 *
 * X-1.4        Camiel Vanderhoeven                             28-DEC-2007
 *      Throw exceptions rather than just exiting when errors occur.
 *
 * X-1.3        Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.2        Brian Wheeler                                   16-DEC-2007
 *      Fixed case of StdAfx.h.
 *
 * X-1.1        Camiel Vanderhoeven                             12-DEC-2007
 *      Initial version in CVS.
 **/

#include "StdAfx.h" 
#include "Disk.h"

CDisk::CDisk(CConfigurator * cfg, CDiskController * ctrl, int idebus, int idedev)
{
  char * a;
  char * b;
  char * c;
  char * d;

  myCfg = cfg;
  myCtrl = ctrl;
  myBus = idebus;
  myDev = idedev;

  a = myCfg->get_myName();
  b = myCfg->get_myValue();
  c = myCfg->get_myParent()->get_myName();
  d = myCfg->get_myParent()->get_myValue();

  CHECK_ALLOCATION(devid_string = (char*) malloc(strlen(a)+strlen(b)+strlen(c)+strlen(d)+6));
  sprintf(devid_string,"%s(%s).%s(%s)",c,d,a,b);

  serial_number = myCfg->get_text_value("serial_num", "ES40EM00000");
  revision_number = myCfg->get_text_value("rev_num", "0.0");
  read_only = myCfg->get_bool_value("read_only");
  is_cdrom = myCfg->get_bool_value("cdrom");

  block_size = is_cdrom?2048:512;

  if (!myCtrl->register_disk(this,myBus,myDev))
  {
    printf("%s: Could not register disk!\n",devid_string);
    throw((int)1);
  }
}

CDisk::~CDisk(void)
{
  free(devid_string);
}

void CDisk::calc_cylinders()
{
  cylinders = byte_size/block_size/sectors/heads;

  off_t_large chs_size = sectors*cylinders*heads*block_size;
  if (chs_size<byte_size)
    cylinders++;
}