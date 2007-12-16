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
 * Contains code to use a file as a disk image.
 *
 * X-1.2        Brian Wheeler                                   16-DEC-2007
 *      Fixed case of StdAfx.h.
 *
 * X-1.1        Camiel Vanderhoeven                             12-DEC-2007
 *      Initial version in CVS.
 **/

#include "StdAfx.h" 
#include "DiskFile.h"
#include "Configurator.h"

CDiskFile::CDiskFile(CConfigurator * cfg, CDiskController * c, int idebus, int idedev) : CDisk(cfg,c,idebus,idedev)
{

  filename = myCfg->get_text_value("file");
  if (!filename)
  {
    printf("%s: Disk has no file attached!\n",devid_string);
    exit(1);
  }
  
  if (read_only)
    handle = fopen(filename,"rb");
  else
    handle = fopen(filename,"rb+");
  if (!handle)
  {
    printf("%s: Could not open file %s!\n",devid_string,filename);
    exit(1);
  }

  // determine size...
  fseek(handle,0,SEEK_END);
  lba_size=ftell(handle)/512;
  byte_size = lba_size * 512;
  fseek(handle,0,0);
  lba_pos = 0;

  sectors = 32;
  heads = 8;
  cylinders = (lba_size/sectors/heads);

  long chs_size = sectors*cylinders*heads;
  if (chs_size>lba_size)
    cylinders--;

  model_number=myCfg->get_text_value("model_number",filename);

  printf("%s: Mounted file %s, %d blocks, %d/%d/%d.\n",devid_string,filename,lba_size,cylinders,heads,sectors);
}

CDiskFile::~CDiskFile(void)
{
}

bool CDiskFile::seek_block(long lba)
{
  if (lba >=lba_size)
  {
    printf("%s: Seek beyond end of file!\n",devid_string);
    exit(1);
  }

  fseek(handle,lba*512,0);

  lba_pos = lba;
  return true;
}

size_t CDiskFile::read_blocks(void *dest, size_t blocks)
{
  size_t r;
  r = fread(dest,512,blocks,handle);
  lba_pos += r;
  return r;
}

size_t CDiskFile::write_blocks(void * src, size_t blocks)
{
  if (read_only)
    return 0;

  size_t r;
  r = fwrite(src,512,blocks,handle);
  lba_pos += r;
  return r;
}

bool CDiskFile::seek_byte(long byte)
{
  if (byte >=byte_size)
  {
    printf("%s: Seek beyond end of file!\n",devid_string);
    exit(1);
  }

  fseek(handle,byte,0);

  byte_pos = byte;
  return true;
}

size_t CDiskFile::read_bytes(void *dest, size_t bytes)
{
  size_t r;
  r = fread(dest,1,bytes,handle);
  byte_pos += r;
  return r;
}

size_t CDiskFile::write_bytes(void * src, size_t bytes)
{
  if (read_only)
    return 0;

  size_t r;
  r = fwrite(src,1,bytes,handle);
  byte_pos += r;
  return r;
}