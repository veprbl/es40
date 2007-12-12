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
 * Contains definitions for the disk base class.
 *
 * X-1.1        Camiel Vanderhoeven                             12-DEC-2007
 *      Initial version in CVS.
 **/

#if !defined(__DISK_H__)
#define __DISK_H__

#include "DiskController.h"

class CDisk
{
public:
  CDisk(CConfigurator * cfg, CDiskController * c, int idebus, int idedev);
  ~CDisk(void);
 
  virtual bool seek_block(long lba) = 0;
  virtual size_t read_blocks(void * dest, size_t blocks) = 0;
  virtual size_t write_blocks(void * src, size_t blocks) = 0;

  virtual bool seek_byte(long byte) = 0;
  virtual size_t read_bytes(void * dest, size_t bytes) = 0;
  virtual size_t write_bytes(void * src, size_t bytes) = 0;

  long get_lba_size()  { return lba_size; };
  long get_chs_size()  { return cylinders*heads*sectors; };
  long get_cylinders() { return cylinders; };
  long get_heads()     { return heads; };
  long get_sectors()   { return sectors; };

  char * get_serial()  { return serial_number; };
  char * get_model()   { return model_number;  };
  char * get_rev()     { return revision_number; };

  bool ro()            { return read_only; };
  bool rw()            { return !read_only; };
  bool cdrom()         { return is_cdrom; };

protected:
  CConfigurator * myCfg;
  CDiskController * myCtrl;
  int myBus;
  int myDev;

  char * serial_number;
  char * model_number;
  char * revision_number;
  char * devid_string;

  bool read_only;
  bool is_cdrom;

  long lba_size; 
  long byte_size;
  long lba_pos;
  long byte_pos;
  long cylinders;
  long heads;
  long sectors;
};

#endif //!defined(__DISK_H__)
