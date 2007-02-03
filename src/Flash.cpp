/** ES40 emulator.
 * Copyright (C) 2007 by Camiel Vanderhoeven
 *
 * Website: www.camicom.com
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
 * 
 * FLASH.CPP contains the code for the emulated Flash ROM devices.
 *
 **/

#include "StdAfx.h"
#include "Flash.h"
#include "System.h"
#include "AlphaCPU.h"

#define MODE_READ 0
#define MODE_STEP1 1
#define MODE_STEP2 2
#define MODE_AUTOSEL 3
#define MODE_PROGRAM 4
#define MODE_ERASE_STEP3 5
#define MODE_ERASE_STEP4 6
#define MODE_ERASE_STEP5 7
#define MODE_CONFIRM_0 8
#define MODE_CONFIRM_1 9

extern CAlphaCPU * cpu[4];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFlash::CFlash(CSystem * c) : CSystemComponent(c)
{
  c->RegisterMemory(this, 0, X64(0000080100000000),0x8000000); // 2MB
  printf("%%FLS-I-INIT: Flash ROM emulator initialized.\n");
  memset(Flash,0xff,2*1024*1024);
  mode = MODE_READ;
}

CFlash::~CFlash()
{

}

u64 CFlash::ReadMem(int index, u64 address, int dsize)
{
  dsize;
  index;

  char trcbuffer[1000];
  u64 data = 0;
  int a = (int)(address>>6);

  switch (mode)
    {
    case MODE_AUTOSEL:
      switch (a)
	{
	case 0:
	  data = 1; // manufacturer
	  break;
	case 1:
	  data = 0xad; // device
	  break;
	default:
	  data = 0;
	}
      break;
    case MODE_CONFIRM_0:
      data = 0x80;
      mode = MODE_READ;
      break;
    case MODE_CONFIRM_1:
      data = 0x80;
      mode = MODE_CONFIRM_0;
      break;
    default:
      data = Flash[a];
    }

  sprintf(trcbuffer,"%%FLS-I-READ: Flash ROM read @ 0x%08x: 0x%x\n",a,(u32)(data&0xffffffff));
  cSystem->trace->trace_dev(trcbuffer);
  return data;
}

void CFlash::WriteMem(int index, u64 address, int dsize, u64 data)
{
  dsize;
  index;

  char trcbuffer[1000];
  int a = (int)(address>>6);
  sprintf(trcbuffer,"%%FLS-I-WRITE: Flash ROM write 0x%08x : 0x%x\n",a,(u32)(data&0xffffffff));
  cSystem->trace->trace_dev(trcbuffer);


  switch(mode)
    {
    case MODE_READ:
    case MODE_AUTOSEL:
      if ((a == 0x5555) && (data==0xaa))
	{
	  //			printf("%%FLS-I-MODE: mode_step1.\n");
	  mode = MODE_STEP1;
	  return;
	}
      //		printf("%%FLS-I-MODE: mode_read.\n");
      mode = MODE_READ;
      return;
    case MODE_STEP1:
      if ((a == 0x2aaa) && (data==0x55))
	{
	  //			printf("%%FLS-I-MODE: mode_step2.\n");
	  mode = MODE_STEP2;
	  return;
	}
      //		printf("%%FLS-I-MODE: mode_read.\n");
      mode = MODE_READ;
      return;
    case MODE_STEP2:
      if (a != 0x5555)
	{
	  //			printf("%%FLS-I-MODE: mode_read.\n");
	  mode = MODE_READ;
	  return;
	}
      switch (data)
	{
	case 0x90:
	  //			printf("%%FLS-I-MODE: mode_autosel.\n");
	  mode = MODE_AUTOSEL;
	  return;
	case 0xa0:
	  //			printf("%%FLS-I-MODE: mode_program.\n");
	  mode = MODE_PROGRAM;
	  return;
	case 0x80:
	  //			printf("%%FLS-I-MODE: mode_erase_step3.\n");
	  mode = MODE_ERASE_STEP3;
	  return;
	}
      mode = MODE_READ;
      //		printf("%%FLS-I-MODE: mode_read.\n");
      return;
    case MODE_ERASE_STEP3:
      if ((a == 0x5555) && (data==0xaa))
	{
	  //			printf("%%FLS-I-MODE: mode_erase_step4.\n");
	  mode = MODE_ERASE_STEP4;
	  return;
	}
      //		printf("%%FLS-I-MODE: mode_read.\n");
      mode = MODE_READ;
      return;
    case MODE_ERASE_STEP4:
      if ((a == 0x2aaa) && (data==0x55))
	{
	  //			printf("%%FLS-I-MODE: mode_erase_step5.\n");
	  mode = MODE_ERASE_STEP5;
	  return;
	}
      //		printf("%%FLS-I-MODE: mode_read.\n");
      mode = MODE_READ;
      return;
    case MODE_ERASE_STEP5:
      if ((a == 0x5555) && (data==0x10))
	{
	  printf("%%FLS-I-ERASE: erasing entire flash.\n");
	  memset(Flash, 0xff, 1<<21);
	  mode = MODE_CONFIRM_1;
	  return;
	}
      if (data==0x30)
	{
	  printf("%%FLS-I-ERABLK: erasing flash blok %d.\n", a>>16);
	  memset(&Flash[(a>>16)<<16], 0xff, 1<<16);
	  mode = MODE_CONFIRM_1;
	  return;
	}
      //		printf("%%FLS-I-MODE: mode_read.\n");
      mode = MODE_READ;
      return;
    }
	

  // we must now be in mode program...
  printf("%%FLS-I-PROGRAM: byte %08x set to %02x.\n",a,data);
  Flash[a] = (u8)data;
  mode = MODE_READ;
}


// Save component state to file
void CFlash::SaveState(FILE * f)
{
  fwrite(Flash,2*1024*1024,1,f);
  fwrite(&mode,sizeof(int),1,f);
}

// Restore component state from file
void CFlash::RestoreState(FILE * f)
{
  fread(Flash,2*1024*1024,1,f);
  fread(&mode,sizeof(int),1,f);
}
