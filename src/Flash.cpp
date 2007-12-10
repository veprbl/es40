/* ES40 emulator.
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
 */

/** 
 * \file
 * Contains the code for the emulated Flash ROM devices.
 *
 * X-1.11       Camiel Vanderhoeven                             10-DEC-2007
 *      Use configurator.
 *
 * X-1.10       Camiel Vanderhoeven                             31-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.9        Camiel Vanderhoeven                             16-FEB-2007
 *      Added functions SaveStateF and RestoreStateF.
 *
 * X-1.8	Brian Wheeler					13-FEB-2007
 *	Formatting.
 *
 * X-1.7 	Camiel Vanderhoeven				12-FEB-2007
 *	Added comments.
 *
 * X-1.6        Camiel Vanderhoeven                             9-FEB-2007
 *      Added comments.
 *
 * X-1.5	Camiel Vanderhoeven				7-FEB-2007
 *	Calls to trace_dev now use the TRC_DEVx macro's.
 *
 * X-1.4        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.3        Brian Wheeler                                   3-FEB-2007
 *      64-bit literals made compatible with Linux/GCC/glibc.
 *      
 * X-1.2        Brian Wheeler                                   3-FEB-2007
 *      Includes are now case-correct (necessary on Linux)
 *
 * X-1.1        Camiel Vanderhoeven                             19-JAN-2007
 *      Initial version in CVS.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
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

/**
 * Constructor.
 **/

CFlash::CFlash(CConfigurator * cfg, CSystem * c) : CSystemComponent(cfg,c)
{
  if (theSROM)
    FAILURE("More than one SROM!!\n");
  theSROM = this;
  c->RegisterMemory(this, 0, X64(0000080100000000),0x8000000); // 2MB
  printf("%%FLS-I-INIT: Flash ROM emulator initialized.\n");
  memset(Flash,0xff,2*1024*1024);
  mode = MODE_READ;
}

/**
 * Destructor.
 **/

CFlash::~CFlash()
{

}

/**
 * Read a byte from flashmemory.
 * Normally, this returns one byte from flash, however, after some commands
 * sent to the flash-rom, this returns identification of status information.
 **/

u64 CFlash::ReadMem(int index, u64 address, int dsize)
{
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

  return data;
}

/**
 * Write command or programming data to flash-rom.
 **/

void CFlash::WriteMem(int index, u64 address, int dsize, u64 data)
{
  int a = (int)(address>>6);


  switch(mode)
    {
    case MODE_READ:
    case MODE_AUTOSEL:
      if ((a == 0x5555) && (data==0xaa))
	{
	  mode = MODE_STEP1;
	  return;
	}
      mode = MODE_READ;
      return;
    case MODE_STEP1:
      if ((a == 0x2aaa) && (data==0x55))
	{
	  mode = MODE_STEP2;
	  return;
	}
      mode = MODE_READ;
      return;
    case MODE_STEP2:
      if (a != 0x5555)
	{
	  mode = MODE_READ;
	  return;
	}
      switch (data)
	{
	case 0x90:
	  mode = MODE_AUTOSEL;
	  return;
	case 0xa0:
	  mode = MODE_PROGRAM;
	  return;
	case 0x80:
	  mode = MODE_ERASE_STEP3;
	  return;
	}
      mode = MODE_READ;
      return;
    case MODE_ERASE_STEP3:
      if ((a == 0x5555) && (data==0xaa))
	{
	  mode = MODE_ERASE_STEP4;
	  return;
	}
      mode = MODE_READ;
      return;
    case MODE_ERASE_STEP4:
      if ((a == 0x2aaa) && (data==0x55))
	{
	  mode = MODE_ERASE_STEP5;
	  return;
	}
      mode = MODE_READ;
      return;
    case MODE_ERASE_STEP5:
      if ((a == 0x5555) && (data==0x10))
	{
	  memset(Flash, 0xff, 1<<21);
	  mode = MODE_CONFIRM_1;
	  return;
	}
      if (data==0x30)
	{
	  memset(&Flash[(a>>16)<<16], 0xff, 1<<16);
	  mode = MODE_CONFIRM_1;
	  return;
	}
      mode = MODE_READ;
      return;
    }
	

  // we must now be in mode program...
  Flash[a] = (u8)data;
  mode = MODE_READ;
}


/**
 * Save state to a Virtual Machine State file.
 **/

void CFlash::SaveState(FILE * f)
{
  fwrite(Flash,2*1024*1024,1,f);
  fwrite(&mode,sizeof(int),1,f);
}

/**
 * Save state to a flash rom file.
 **/

void CFlash::SaveStateF()
{
  FILE * ff;
  ff = fopen(myCfg->get_text_value("rom.flash","flash.rom"),"wb");
  if (ff)
    {
      SaveState(ff);
      fclose(ff);
      printf("%%FLS-I-SAVEST: Flash state saved to %s\n",myCfg->get_text_value("rom.flash","flash.rom"));
    }
  else
  {
    printf("%%FLS-F-NOSAVE: Flash could not be saved to %s\n",myCfg->get_text_value("rom.flash","flash.rom"));
  }
}

/**
 * Restore state from a Virtual Machine state file.
 **/

void CFlash::RestoreState(FILE * f)
{
  fread(Flash,2*1024*1024,1,f);
  fread(&mode,sizeof(int),1,f);
}

/**
 * Restore state from a flash rom file.
 **/

void CFlash::RestoreStateF()
{
  FILE * ff;
  ff = fopen(myCfg->get_text_value("rom.flash","flash.rom"),"rb");
  if (ff)
    {
      RestoreState(ff);
      fclose(ff);
      printf("%%FLS-I-RESTST: Flash state restored from %s\n",myCfg->get_text_value("rom.flash","flash.rom"));
    }
  else
  {
    printf("%%FLS-F-NOREST: Flash could not be restored from %s\n",myCfg->get_text_value("rom.flash","flash.rom"));
  }
}

CFlash * theSROM = 0;
