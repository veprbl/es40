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
 * Contains definitions for the disk base class.
 *
 * $Id: Disk.h,v 1.7 2008/01/04 22:11:22 iamcamiel Exp $
 *
 * X-1.7        Camiel Vanderhoeven                             04-JAN-2008
 *      64-bit file I/O.
 *
 * X-1.6        Camiel Vanderhoeven                             02-JAN-2008
 *      Comments.
 *
 * X-1.5        Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.4        Camiel Vanderhoeven                             20-DEC-2007
 *      Close files and free memory when the emulator shuts down.
 *
 * X-1.3        Camiel Vanderhoeven                             19-DEC-2007
 *      Allow for reporting disk-size in bytes.
 *
 * X-1.2        Camiel Vanderhoeven                             18-DEC-2007
 *      Byte-sized transfers for SCSI controller.
 *
 * X-1.1        Camiel Vanderhoeven                             12-DEC-2007
 *      Initial version in CVS.
 **/

#if !defined(__DISK_H__)
#define __DISK_H__

#include "DiskController.h"

/**
 * \brief Abstract base class for disks (connects to a CDiskController)
 **/

class CDisk
{
public:
  CDisk(CConfigurator * cfg, CDiskController * c, int idebus, int idedev);
  virtual ~CDisk(void);
 
  virtual bool seek_block(off_t_large lba) = 0;
  virtual size_t read_blocks(void * dest, size_t blocks) = 0;
  virtual size_t write_blocks(void * src, size_t blocks) = 0;

  virtual bool seek_byte(off_t_large byte) = 0;
  virtual size_t read_bytes(void * dest, size_t bytes) = 0;
  virtual size_t write_bytes(void * src, size_t bytes) = 0;

  off_t_large get_lba_size()  { return lba_size; };
  off_t_large get_byte_size()  { return byte_size; };
  off_t_large get_chs_size()  { return cylinders*heads*sectors; };
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

  off_t_large lba_size; 
  off_t_large byte_size;
  off_t_large byte_pos;
  long cylinders;
  long heads;
  long sectors;
};

#endif //!defined(__DISK_H__)
