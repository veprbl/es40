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
 * $Id: Disk.cpp,v 1.9 2008/01/09 10:13:58 iamcamiel Exp $
 *
 * X-1.9        Camiel Vanderhoeven                             09-JAN-2008
 *      Save disk state to state file.
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

CDisk::CDisk(CConfigurator * cfg, CSystem * sys, CDiskController * ctrl, int idebus, int idedev) : CSystemComponent(cfg, sys)
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

  free(devid_string); // we override the default to include the controller.
  CHECK_ALLOCATION(devid_string = (char*) malloc(strlen(a)+strlen(b)+strlen(c)+strlen(d)+6));
  sprintf(devid_string,"%s(%s).%s(%s)",c,d,a,b);

  serial_number = myCfg->get_text_value("serial_num", "ES40EM00000");
  revision_number = myCfg->get_text_value("rev_num", "0.0");
  read_only = myCfg->get_bool_value("read_only");
  is_cdrom = myCfg->get_bool_value("cdrom");

  state.block_size = is_cdrom?2048:512;

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
  cylinders = byte_size/state.block_size/sectors/heads;

  off_t_large chs_size = sectors*cylinders*heads*state.block_size;
  if (chs_size<byte_size)
    cylinders++;
}


static u32 disk_magic1 = 0xD15D15D1;
static u32 disk_magic2 = 0x15D15D5;

/**
 * Save state to a Virtual Machine State file.
 **/

int CDisk::SaveState(FILE *f)
{
  long ss = sizeof(state);

  fwrite(&disk_magic1,sizeof(u32),1,f);
  fwrite(&ss,sizeof(long),1,f);
  fwrite(&state,sizeof(state),1,f);
  fwrite(&disk_magic2,sizeof(u32),1,f);
  printf("%s: %d bytes saved.\n",devid_string,ss);
  return 0;
}

/**
 * Restore state from a Virtual Machine State file.
 **/

int CDisk::RestoreState(FILE *f)
{
  long ss;
  u32 m1;
  u32 m2;
  size_t r;

  r = fread(&m1,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (m1 != disk_magic1)
  {
    printf("%s: MAGIC 1 does not match!\n",devid_string);
    return -1;
  }

  fread(&ss,sizeof(long),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (ss != sizeof(state))
  {
    printf("%s: STRUCT SIZE does not match!\n",devid_string);
    return -1;
  }

  fread(&state,sizeof(state),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }

  r = fread(&m2,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (m2 != disk_magic2)
  {
    printf("%s: MAGIC 1 does not match!\n",devid_string);
    return -1;
  }

  calc_cylinders(); // state.block_size may have changed.

  printf("%s: %d bytes restored.\n",devid_string,ss);
  return 0;
}
