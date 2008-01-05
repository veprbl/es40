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
 * Contains code to use a raw device as a disk image.
 *
 * $Id: DiskDevice.cpp,v 1.1 2008/01/05 21:18:17 iamcamiel Exp $
 *
 * X-1.1        Camiel Vanderhoeven                             05-JAN-2008
 *      Initial version in CVS.
 **/

#include "StdAfx.h" 
#include "DiskDevice.h"

#if defined(_WIN32)
#include <WinIoCtl.h>
#endif

CDiskDevice::CDiskDevice(CConfigurator * cfg, CDiskController * c, int idebus, int idedev) : CDisk(cfg,c,idebus,idedev)
{
  filename = myCfg->get_text_value("device");
  if (!filename)
  {
    printf("%s: Disk has no device attached!\n",devid_string);
    throw((int)1);
  }
  
  if (read_only)
  {
#if defined(_WIN32)
    buffer = (char*)malloc(2048);
    buffer_size = 2048;
    handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
#else
    handle = fopen(filename,"rb");
#endif
  }
  else
  {
#if defined(_WIN32)
    handle = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
#else
    handle = fopen(filename,"rb+");
#endif
  }
#if defined(_WIN32)
  if (handle==INVALID_HANDLE_VALUE)
  {
    printf("%s: Could not open device %s!\n",devid_string,filename);
    printf("%s: Error %ld.\n",devid_string,GetLastError());
    throw((int)1);
  }
#else
  if (!handle)
  {
    printf("%s: Could not open device %s!\n",devid_string,filename);
    throw((int)1);
  }
#endif

  // determine size...
#if defined(_WIN32)
  DISK_GEOMETRY_EX x;
  DWORD bytesret;

  if (!DeviceIoControl(handle, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &x, sizeof(x), &bytesret, NULL))
  {
    printf("%s: Could not get drive geometry for %s!\n",devid_string,filename);
    printf("%s: Error %ld.\n",devid_string,GetLastError());
    throw((int)1);
  }

  sectors = x.Geometry.SectorsPerTrack;
  cylinders = x.Geometry.Cylinders.QuadPart * x.Geometry.BytesPerSector / 512;
  heads = x.Geometry.TracksPerCylinder;
  lba_size = x.DiskSize.QuadPart/512;
  byte_size = lba_size * 512;
  block_size = x.Geometry.BytesPerSector;

  LARGE_INTEGER a;
  a.QuadPart = 0;
  SetFilePointerEx(handle, a, (PLARGE_INTEGER) &byte_pos, FILE_BEGIN); 
#else
  fseek_large(handle,0,SEEK_END);
  lba_size=ftell_large(handle)/512;
  byte_size = lba_size * 512;
  fseek_large(handle,0,SEEK_SET);
  byte_pos = ftell_large(handle);

  sectors = 32;
  heads = 8;
  cylinders = lba_size/sectors/heads;

  off_t_large chs_size = sectors*cylinders*heads;
  if (chs_size<lba_size)
    cylinders++;
#endif

  model_number=myCfg->get_text_value("model_number",filename);

  printf("%s: Mounted device %s, %" LL "d blocks, %" LL "d/%d/%d.\n",devid_string,filename,lba_size,cylinders,heads,sectors);
}

CDiskDevice::~CDiskDevice(void)
{
  printf("%s: Closing file.\n",devid_string);
#if defined(_WIN32)
  if (handle != INVALID_HANDLE_VALUE)
    CloseHandle(handle);
#else
  if (handle)
    fclose(handle);
#endif
}

bool CDiskDevice::seek_block(off_t_large lba)
{
  if (lba >=lba_size)
  {
    printf("%s: Seek beyond end of file!\n",devid_string);
    throw((int)1);
  }

#if defined(_WIN32)
  byte_pos = lba * 512;
#else
  fseek_large(handle,lba*512,SEEK_SET);
  byte_pos = ftell_large(handle);
#endif
  return true;
}

size_t CDiskDevice::read_blocks(void *dest, size_t blocks)
{
#if defined(_WIN32)
  return read_bytes(dest,blocks*512)/512;
#else
  size_t r;
  r = fread(dest,512,blocks,handle);
  byte_pos = ftell_large(handle);
  return r;
#endif
}

size_t CDiskDevice::write_blocks(void * src, size_t blocks)
{
  if (read_only)
    return 0;

#if defined(_WIN32)
  return write_bytes(src,blocks*512)/512;
#else
  size_t r;
  r = fwrite(src,512,blocks,handle);
  byte_pos = ftell_large(handle);
  return r;
#endif
}

bool CDiskDevice::seek_byte(off_t_large byte)
{
  if (byte >=byte_size)
  {
    printf("%s: Seek beyond end of file!\n",devid_string);
    throw((int)1);
  }

#if defined(_WIN32)
  byte_pos = byte;
#else
  fseek_large(handle,byte,SEEK_SET);
  byte_pos = ftell_large(handle);
#endif

  return true;
}

size_t CDiskDevice::read_bytes(void *dest, size_t bytes)
{
#if defined(_WIN32)
  size_t byte_from = (byte_pos/block_size)*block_size;  
  size_t byte_to   = (((byte_pos+bytes-1)/block_size)+1)*block_size;
  LARGE_INTEGER a;
  DWORD r;

  if (byte_to-byte_from > buffer_size)
  {
    CHECK_REALLOCATION(buffer,byte_to-byte_from,char);
    buffer_size = byte_to-byte_from;
  }

  a.QuadPart = byte_from;
  SetFilePointerEx(handle, a, NULL, FILE_BEGIN);

  ReadFile(handle,buffer,byte_to-byte_from,&r,NULL);

  if (r != (byte_to-byte_from))
    printf("%s: Tried to read %d bytes from pos %ld, but could only read %d bytes!\n",devid_string,bytes,byte_from,r);

  memcpy(dest,buffer+byte_pos-byte_from,bytes);
  byte_pos += bytes;

  r = bytes;
#else
  size_t r;
  r = fread(dest,1,bytes,handle);
  byte_pos = ftell_large(handle);
#endif
  return r;
}

size_t CDiskDevice::write_bytes(void * src, size_t bytes)
{
  if (read_only)
    return 0;

#if defined(_WIN32)
  size_t byte_from = (byte_pos/block_size)*block_size;  
  size_t byte_to   = (((byte_pos+bytes-1)/block_size)+1)*block_size;
  LARGE_INTEGER a;
  DWORD r;

  if (byte_to-byte_from > buffer_size)
  {
    CHECK_REALLOCATION(buffer,byte_to-byte_from,char);
    buffer_size = byte_to-byte_from;
  }

  if (byte_from != byte_pos)
  {
    // we don't write the entire first block, so we read it 
    // from disk first so we don't corrupt the disk
    a.QuadPart = byte_from;
    SetFilePointerEx(handle, a, NULL, FILE_BEGIN);
    ReadFile(handle,buffer,block_size,&r,NULL);
    if (r != (block_size))
    {
      printf("%s: Tried to read %d bytes from pos %ld, but could only read %d bytes!\n",devid_string,block_size,byte_from,r);
      FAILURE("Error during device write operation. Terminating to avoid disk corruption.");
    }
  }

  if ((byte_to != byte_pos+bytes) && (byte_to-byte_from>block_size))
  {
    // we don't write the entire last block, so we read it 
    // from disk first so we don't corrupt the disk
    a.QuadPart = byte_to-block_size;
    SetFilePointerEx(handle, a, NULL, FILE_BEGIN);
    ReadFile(handle,buffer+byte_to-byte_from-block_size,block_size,&r,NULL);
    if (r != (block_size))
    {
      printf("%s: Tried to read %d bytes from pos %ld, but could only read %d bytes!\n",devid_string,block_size,byte_to-block_size,r);
      FAILURE("Error during device write operation. Terminating to avoid disk corruption.");
    }
  }

  // add the data we're writing to the buffer
  memcpy(buffer+byte_pos-byte_from,src,bytes);
  
  a.QuadPart = byte_from;
  SetFilePointerEx(handle, a, NULL, FILE_BEGIN);

  // and write the buffer to disk
  WriteFile(handle,buffer,byte_to-byte_from,&r,NULL);

  if (r != (byte_to-byte_from))
  {
    printf("%s: Tried to write %d bytes to pos %ld, but could only write %d bytes!\n",devid_string,bytes,byte_from,r);
    FAILURE("Error during device write operation. Terminating to avoid disk corruption.");
  }

  byte_pos += bytes;

  r = bytes;
#else
  size_t r;
  r = fwrite(src,1,bytes,handle);
  byte_pos = ftell_large(handle);
#endif
  return r;
}