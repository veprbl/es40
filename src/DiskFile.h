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
 * Contains definitions to use a file as a disk image.
 *
 * X-1.3        Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.2        Camiel Vanderhoeven                             20-DEC-2007
 *      Close files and free memory when the emulator shuts down.
 *
 * X-1.1        Camiel Vanderhoeven                             12-DEC-2007
 *      Initial version in CVS.
 **/

#if !defined(__DISKFILE_H__)
#define __DISKFILE_H__

#include "Disk.h"

class CDiskFile : public CDisk
{
public:
  CDiskFile(CConfigurator * cfg, CDiskController * c, int idebus, int idedev);
  virtual ~CDiskFile(void);

  virtual bool seek_block(unsigned long block);
  virtual size_t read_blocks(void * dest, size_t blocks);
  virtual size_t write_blocks(void * src, size_t blocks);

  virtual bool seek_byte(unsigned long byte);
  virtual size_t read_bytes(void * dest, size_t bytes);
  virtual size_t write_bytes(void * src, size_t bytes);

  FILE * get_handle() { return handle; };
  
protected:
  FILE * handle;
  char * filename;
};

#endif //!defined(__DISKFILE_H__)
