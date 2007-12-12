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
 * Contains code to use a RAM disk.
 *
 * X-1.1        Camiel Vanderhoeven                             12-DEC-2007
 *      Initial version in CVS.
 **/

#include "stdAfx.h" 
#include "DiskRam.h"
#include "Configurator.h"

CDiskRam::CDiskRam(CConfigurator * cfg, CDiskController * c, int idebus, int idedev) : CDisk(cfg,c,idebus,idedev)
{
  lba_size = myCfg->get_int_value("size",512)*2*1024;
  byte_size = lba_size * 512;

  ramdisk = malloc(lba_size*512);

  if (!ramdisk)
  {
    printf("%s: Could not allocate %d MB!\n",devid_string,lba_size/2);
    exit(1);
  }

  lba_pos = 0;
  byte_pos = 0;

  sectors = 32;
  heads = 8;
  cylinders = (lba_size/sectors/heads);

  long chs_size = sectors*cylinders*heads;
  if (chs_size>lba_size)
    cylinders--;

  model_number=myCfg->get_text_value("model_number","ES40RAMDISK");

  printf("%s: Mounted RAMDISK, %d blocks, %d/%d/%d.\n",devid_string,lba_size,cylinders,heads,sectors);
}

CDiskRam::~CDiskRam(void)
{
}

bool CDiskRam::seek_block(long lba)
{
  if (lba >=lba_size)
  {
    printf("%s: Seek beyond end of file!\n",devid_string);
    exit(1);
  }

  lba_pos = lba;
  return true;
}

size_t CDiskRam::read_blocks(void *dest, size_t blocks)
{
  if (lba_pos >=lba_size)
    return 0;

  while (lba_pos + blocks >= lba_size)
    blocks--;

  memcpy(dest,&(((char*)ramdisk)[lba_pos*512]),blocks);
  lba_pos += blocks;
  return blocks;
}

size_t CDiskRam::write_blocks(void * src, size_t blocks)
{
  if (read_only)
    return 0;

  if (lba_pos >=lba_size)
    return 0;

  while (lba_pos + blocks >= lba_size)
    blocks--;

  memcpy(&(((char*)ramdisk)[lba_pos*512]),src,blocks);
  lba_pos += blocks;
  return blocks;

}

bool CDiskRam::seek_byte(long byte)
{
  if (byte >=byte_size)
  {
    printf("%s: Seek beyond end of file!\n",devid_string);
    exit(1);
  }

  byte_pos = byte;
  return true;
}

size_t CDiskRam::read_bytes(void *dest, size_t bytes)
{
  if (byte_pos >=byte_size)
    return 0;

  while (byte_pos + bytes >= byte_size)
    bytes--;

  memcpy(dest,&(((char*)ramdisk)[byte_pos]),bytes);
  byte_pos += bytes;
  return bytes;
}

size_t CDiskRam::write_bytes(void *src, size_t bytes)
{
  if (byte_pos >=byte_size)
    return 0;

  while (byte_pos + bytes >= byte_size)
    bytes--;

  memcpy(&(((char*)ramdisk)[byte_pos]),src,bytes);
  byte_pos += bytes;
  return bytes;
}
