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
 * Contains the code for the emulated Ali M1543C IDE chipset part.
 *
 * X-1.1        Camiel Vanderhoeven                             10-DEC-2007
 *      Initial version in CVS; this part was split off from the CAliM1543C
 *      class.
 **/

#include "StdAfx.h"
#include "AliM1543C_ide.h"
#include "System.h"

#include <math.h>

#include "gui/scancodes.h"
#include "gui/keymap.h"

#include "AliM1543C.h"

#ifdef DEBUG_PIC
bool pic_messages = false;
#endif

u32 ide_cfg_data[64] = {
/*00*/  0x522910b9, // CFID: vendor + device
/*04*/  0x02800000, // CFCS: command + status
/*08*/  0x0101fac1, // CFRV: class + revision
/*0c*/  0x00000000, // CFLT: latency timer + cache line size
/*10*/  0x000001f1, // BAR0: 
/*14*/  0x000003f5, // BAR1: 
/*18*/  0x00000171, // BAR2: 
/*1c*/  0x00000375, // BAR3: 
/*20*/  0x0000f001, // BAR4: 
/*24*/  0x00000000, // BAR5: 
/*28*/  0x00000000, // CCIC: CardBus
/*2c*/  0x00000000, // CSID: subsystem + vendor
/*30*/  0x00000000, // BAR6: expansion rom base
/*34*/  0x00000000, // CCAP: capabilities pointer
/*38*/  0x00000000,
/*3c*/  0x040201ff, // CFIT: interrupt configuration
        0,0,
/*48*/  0x4a000000, // UDMA test
/*4c*/  0x1aba0000, // reserved
        0,
/*54*/  0x44445555, // udma setting + fifo treshold
        0,0,0,0,0,0,0,0,
/*78*/  0x00000021, // ide clock
        0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

u32 ide_cfg_mask[64] = {
/*00*/  0x00000000, // CFID: vendor + device
/*04*/  0x0000ffff, // CFCS: command + status
/*08*/  0x00000000, // CFRV: class + revision
/*0c*/  0x0000ffff, // CFLT: latency timer + cache line size
/*10*/  0xfffffff8, // BAR0
/*14*/  0xfffffffc, // BAR1: CBMA
/*18*/  0xfffffff8, // BAR2: 
/*1c*/  0xfffffffc, // BAR3: 
/*20*/  0xfffffff0, // BAR4: 
/*24*/  0x00000000, // BAR5: 
/*28*/  0x00000000, // CCIC: CardBus
/*2c*/  0x00000000, // CSID: subsystem + vendor
/*30*/  0x00000000, // BAR6: expansion rom base
/*34*/  0x00000000, // CCAP: capabilities pointer
/*38*/  0x00000000,
/*3c*/  0x000000ff, // CFIT: interrupt configuration
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/**
 * Constructor.
 **/

CAliM1543C_ide::CAliM1543C_ide(CConfigurator * cfg, CSystem * c, int pcibus, int pcidev): CPCIDevice(cfg,c,pcibus,pcidev)
{
  if (theAliIDE != 0)
    FAILURE("More than one AliIDE!!\n");
  theAliIDE = this;

  add_function(0,ide_cfg_data, ide_cfg_mask);

  add_legacy_io(14, 0x1f0, 8);
  add_legacy_io(16, 0x3f4, 4);
  add_legacy_io(15, 0x170, 8);
  add_legacy_io(17, 0x374, 4);
  add_legacy_io(18, 0xf000, 8);
  add_legacy_io(19, 0xf008, 8);

  int i;
  char buffer[64];
  char * filename;
  char *p;
  
  for(i=0;i<4;i++) {
    int C = i/2;
    int D = i%2;

    ide_info[C][D].handle = NULL;
    ide_info[C][D].filename = NULL;

    sprintf(buffer,"disk.%d",i);
    filename=cfg->get_text_value(buffer,NULL);
    if (filename)
    {
      if(*filename == '+') {
	filename++;
	ide_info[C][D].handle=fopen(filename,"rb+");
	ide_info[C][D].mode=1;
      } else {
	ide_info[C][D].handle=fopen(filename,"rb");
	ide_info[C][D].mode=0;
      }

      if(ide_info[C][D].handle != NULL) {
        printf("%%IDE-I-MOUNT: Device '%s' mounted on IDE %d\n",filename,i);
        CHECK_ALLOCATION(p=(char *)malloc(strlen(filename)+1));
        strcpy(p,filename);
        ide_info[C][D].filename=p;

	fseek(ide_info[C][D].handle,0,SEEK_END);
	ide_info[C][D].size=ftell(ide_info[C][D].handle)/512;
	printf("-IDE-I-SIZE: %d blocks available.\n",ide_info[C][D].size);

      } else {
          printf("%%IDE-E-MOUNT: Cannot mount '%s'\n",filename);
      }
    }
  }

  ResetPCI();

  printf("%%ALI-I-INIT: ALi M1543C chipset emulator initialized.\n");
  printf("%%ALI-I-IMPL: Implemented: keyboard, port 61, toy clock, isa bridge, flash ROM.\n");
}

CAliM1543C_ide::~CAliM1543C_ide()
{
}

u32 CAliM1543C_ide::ReadMem_Legacy(int index, u32 address, int dsize)
{
  int channel = 0;
  switch(index)
    {
    case 15:
    case 22:
      channel = 1;
    case 14:
    case 21:
      return ide_command_read(channel,address);
    case 17:
    case 24:
      channel = 1;
    case 16:
    case 23:
      return ide_control_read(channel,address);
    case 19:
    case 26:
      channel = 1;
    case 18:
    case 25:
      return ide_busmaster_read(channel,address);
    }

  return 0;
}

void CAliM1543C_ide::WriteMem_Legacy(int index, u32 address, int dsize, u32 data)
{
  int channel = 0;
  switch(index)
    {
    case 15:
    case 22:
      channel = 1;
    case 14:
    case 21:
      ide_command_write(channel,address, data);
      return;
    case 17:
    case 24:
      channel = 1;
      ide_control_write(channel,address, data);
      return;
    case 16:
    case 23:
      ide_control_write(channel,address, data);
      return;
    case 19:
    case 26:
      channel = 1;
    case 18:
    case 25:
      ide_busmaster_write(channel,address, data);
      return;
    }
}


u32 CAliM1543C_ide::ReadMem_Bar(int func, int bar, u32 address, int dsize)
{
  int channel = 0;
  switch(bar)
    {
    case 2:
      channel = 1;
    case 0:
      return ide_command_read(channel,address);
    case 3:
      channel = 1;
    case 1:
      return ide_control_read(channel,address);
    case 4:
      if (address <8)
        return ide_busmaster_read(0,address);
      else
        return ide_busmaster_read(1,address-8);
    }

  return 0;
}

void CAliM1543C_ide::WriteMem_Bar(int func, int bar, u32 address, int dsize, u32 data)
{
  int channel = 0;
  switch(bar)
    {
    case 2:
      channel = 1;
    case 0:
      ide_command_write(channel,address, data);
      return;
    case 3:
      channel = 1;
    case 1:
      ide_control_write(channel,address, data);
      return;
    case 4:
      if (address <8)
        return ide_busmaster_write(0,address,data);
      else
        return ide_busmaster_write(1,address-8,data);
      return;
    }
}

u32 CAliM1543C_ide::ide_command_read(int index, u32 address)
{
  u32 data;

  data = state.ide_command[index][address];

  if (!(ide_info[index][state.ide_selected[index]].handle))
  {
    // nonexistent drive
    if (address)
      return 0xff;
    else
      return 0xffff;
  }

  switch (address)
    {
    case 0:
      if (state.ide_reading[index])
        data = endian_16(state.ide_data[index][state.ide_data_ptr[index]]);
      else
        data = state.ide_data[index][state.ide_data_ptr[index]];
//      printf("%c%c",printable((char)(data&0xff)),printable((char)((data>>8)&0xff)));
      state.ide_data_ptr[index]++;
      if (state.ide_data_ptr[index]==256)
	{
//	  printf("\n");
	  if (state.ide_reading[index] && state.ide_sectors[index])
	    {
	      fread(&(state.ide_data[index][0]),1,512,ide_info[index][state.ide_selected[index]].handle);
	      state.ide_sectors[index]--;
	      if (!(state.ide_control[index]&2))
          {
  		    theAli->pic_interrupt(1,6+index);
            state.ide_bm_status[index] |= 4;
          }
	    }
	  else
	    {
	      state.ide_status[index] &= ~0x08;	// (no DRQ)
	      state.ide_reading[index] = false;
	    }
	  state.ide_data_ptr[index] = 0;
	}
      break;
    case 1:
      data = state.ide_error[index]; // no error
      break;
    case 7:
      //
      // HACK FOR STRANGE ERROR WHEN SAVING/LOADING STATE
      //
      if (state.ide_status[index]==0xb9)
        state.ide_status[index] = 0x40;
      //
      //
      data = state.ide_status[index];
      break;
    }
  TRC_DEV4("%%ALI-I-READIDECMD: read port %d on IDE command %d: 0x%02x\n", (u32)(address), index, data);
#ifdef DEBUG_IDE
  if (address)
    printf("%%ALI-I-READIDECMD: read port %d on IDE command %d: 0x%02x\n", (u32)(address), index, data);
#endif
  return data;
}

void CAliM1543C_ide::ide_command_write(int index, u32 address, u32 data)
{
  int lba;
  int x;
  int l;

  TRC_DEV4("%%ALI-I-WRITEIDECMD: write port %d on IDE command %d: 0x%02x\n",  (u32)(address),index, data);

#ifdef DEBUG_IDE
  if (address)
    printf("%%ALI-I-WRITEIDECMD: write port %d on IDE command %d: 0x%02x\n",  (u32)(address),index, data);
#endif

  state.ide_command[index][address]=(u8)data;
	
  state.ide_selected[index] = (state.ide_command[index][6]>>4)&1;

  if (ide_info[index][state.ide_selected[index]].handle)
    {
      // drive is present
      state.ide_status[index] = 0x40;
      state.ide_error[index] = 0;

      if (address==0 && state.ide_writing[index])
      {
        state.ide_data[index][state.ide_data_ptr[index]] = endian_16((u16)data);
        state.ide_data_ptr[index]++;
        if (state.ide_data_ptr[index]==256)
	{
	  fwrite(&(state.ide_data[index][0]),1,512,ide_info[index][state.ide_selected[index]].handle);
	  state.ide_sectors[index]--;
	  state.ide_data_ptr[index] = 0;
	  if (!(state.ide_control[index]&2))
      {
	    theAli->pic_interrupt(1,6+index);
        state.ide_bm_status[index] |= 4;
      }
	}
	if (state.ide_sectors[index])
	  state.ide_status[index] = 0x48;
	else	      
	  state.ide_writing[index] = false;
      }
      else if (address==7)	// command
	{
	  switch (data)
	    {
	    case 0xec:	// identify drive
//#ifdef DEBUG_IDE
	      printf("%%IDE-I-IDENTIFY: Identify IDE disk %d.%d   \n",index,state.ide_selected[index]);
//#endif
	      state.ide_data_ptr[index] = 0;
	      state.ide_data[index][0] = 0x0140;	// flags
	      state.ide_data[index][1] = 3000;	// cylinders
	      state.ide_data[index][2] = 0xc837;	// specific configuration (ATA-4 specs)
	      state.ide_data[index][3] = 14;		// heads
	      state.ide_data[index][4] = 25600;	// bytes per track
	      state.ide_data[index][5] = 512;		// bytes per sector
	      state.ide_data[index][6] = 50;		// sectors per track
	      state.ide_data[index][7] = 0;		// spec. bytes
	      state.ide_data[index][8] = 0;		// spec. bytes
	      state.ide_data[index][9] = 0;		// unique vendor status words
	      state.ide_data[index][10] = 0x2020;	// serial number
	      state.ide_data[index][11] = 0x2020;
	      state.ide_data[index][12] = 0x2020;
	      state.ide_data[index][13] = 0x2020;
	      state.ide_data[index][14] = 0x2020;
	      state.ide_data[index][15] = 0x2020;
	      state.ide_data[index][16] = 0x2020;
	      state.ide_data[index][17] = 0x2020;
	      state.ide_data[index][18] = 0x2020;
	      state.ide_data[index][19] = 0x2020;
	      state.ide_data[index][20] = 1;		// single ported, single buffer
	      state.ide_data[index][21] = 51200;	// buffer size
	      state.ide_data[index][22] = 0;		// ecc bytes
	      state.ide_data[index][23] = 0x2020;	// firmware revision
	      state.ide_data[index][24] = 0x2020;
	      state.ide_data[index][25] = 0x2020;
	      state.ide_data[index][26] = 0x2020;

	      // clear the name
	      for(x=27;x<47;x++) {
		state.ide_data[index][x]=0x2020;
	      }
	      l = strlen(ide_info[index][state.ide_selected[index]].filename);
	      l = (l > 40)? 40 : l;
	      memcpy((char *)&state.ide_data[index][27],ide_info[index][state.ide_selected[index]].filename,l);
     #if defined(ES40_LITTLE_ENDIAN)         
	      for(x=27;x<47;x++) {
		state.ide_data[index][x]=((state.ide_data[index][x]>>8) & 0xff) | (state.ide_data[index][x]<<8);
	      }
     #endif
     
              state.ide_data[index][47] = 1;		// read/write multiples
	      state.ide_data[index][48] = 0;		// double-word IO transfers supported
	      state.ide_data[index][49] = 0x0202;		// capability LBA
	      state.ide_data[index][50] = 0;
	      state.ide_data[index][51] = 0x101;		// cycle time
	      state.ide_data[index][52] = 0x101;		// cycle time
	      state.ide_data[index][53] = 1;			// field_valid
	      state.ide_data[index][54] = 3000;		// cylinders
	      state.ide_data[index][55] = 14;		// heads
	      state.ide_data[index][56] = 50;		// sectors
	      state.ide_data[index][57] = ide_info[index][state.ide_selected[index]].size & 0xFFFF;	// total_sectors
	      state.ide_data[index][58] = ide_info[index][state.ide_selected[index]].size >> 16;	// ""
	      state.ide_data[index][59] = 0;							// multiple sector count
	      state.ide_data[index][60] = ide_info[index][state.ide_selected[index]].size & 0xFFFF;	// LBA capacity
	      state.ide_data[index][61] = ide_info[index][state.ide_selected[index]].size >> 16;	// ""
				
	      //	unsigned int	lba_capacity;	/* total number of sectors */
	      //	unsigned short	dma_1word;	/* single-word dma info */
	      //	unsigned short	dma_mword;	/* multiple-word dma info */
	      //	unsigned short  eide_pio_modes; /* bits 0:mode3 1:mode4 */
	      //	unsigned short  eide_dma_min;	/* min mword dma cycle time (ns) */
	      //	unsigned short  eide_dma_time;	/* recommended mword dma cycle time (ns) */
	      //	unsigned short  eide_pio;       /* min cycle time (ns), no IORDY  */
	      //	unsigned short  eide_pio_iordy; /* min cycle time (ns), with IORDY */



	      state.ide_status[index] = 0x48;	// RDY+DRQ

	      if (!(state.ide_control[index]&2))
          {
  		    theAli->pic_interrupt(1,6+index);
            state.ide_bm_status[index] |= 4;
          }
	      break;
	    case 0x20: // read sector
	    case 0x21:
	      lba =      endian_32(*((u32*)(&(state.ide_command[index][3])))) & 0x0fffffff;
	      TRC_DEV5("%%IDE-I-READSECT: Read  %3d sectors @ IDE %d.%d LBA %8d\n",state.ide_command[index][2]?state.ide_command[index][2]:256,index,state.ide_selected[index],lba);
#ifdef DEBUG_IDE
	      printf("%%IDE-I-READSECT: Read  %3d sectors @ IDE %d.%d LBA %8d\n",state.ide_command[index][2]?state.ide_command[index][2]:256,index,state.ide_selected[index],lba);
#endif
	      fseek(ide_info[index][state.ide_selected[index]].handle,lba*512,0);
	      fread(&(state.ide_data[index][0]),1,512,ide_info[index][state.ide_selected[index]].handle);
	      state.ide_data_ptr[index] = 0;
	      state.ide_status[index] = 0x48;
	      state.ide_sectors[index] = state.ide_command[index][2]-1;
	      if (state.ide_sectors[index]) state.ide_reading[index] = true;
	      if (!(state.ide_control[index]&2))
          {
  		    theAli->pic_interrupt(1,6+index);
            state.ide_bm_status[index] |= 4;
          }
	      break;
	    case 0x30:
	    case 0x31:
	      if (!ide_info[index][state.ide_selected[index]].mode)
	      {
	        printf("%%IDE-W-WRITPROT: Attempt to write to write-protected disk.\n");
		state.ide_status[index] = 0x41;
		state.ide_error[index] = 0x04;
	      }
	      else
	      {
	        lba =      endian_32(*((u32*)(&(state.ide_command[index][3])))) & 0x0fffffff;
	        TRC_DEV5("%%IDE-I-WRITSECT: Write %3d sectors @ IDE %d.%d @ LBA %8d\n",state.ide_command[index][2]?state.ide_command[index][2]:256,index,state.ide_selected[index],lba);
#ifdef DEBUG_IDE
	        printf("%%IDE-I-WRITSECT: Write %3d sectors @ IDE %d.%d @ LBA %8d\n",state.ide_command[index][2]?state.ide_command[index][2]:256,index,state.ide_selected[index],lba);
#endif
	        fseek(ide_info[index][state.ide_selected[index]].handle,lba*512,0);
	        state.ide_data_ptr[index] = 0;
	        state.ide_status[index] = 0x48;
	        state.ide_sectors[index] = state.ide_command[index][2];
	        state.ide_writing[index] = true;
	      }
	      break;
	case 0x91:			// SET TRANSLATION
//#ifdef DEBUG_IDE
	      printf("%%IDE-I-SETTRANS: Set IDE translation\n");
//#endif
	      state.ide_status[index] = 0x40;
	      if (!(state.ide_control[index]&2))
          {
  		    theAli->pic_interrupt(1,6+index);
            state.ide_bm_status[index] |= 4;
          }
	      break;

	    case 0x08: // reset drive (DRST)
//#ifdef DEBUG_IDE

	      printf("%%IDE-I-RESET: IDE Reset\n");
//#endif
	      state.ide_status[index]= 0x40;
	      if (!(state.ide_control[index]&2))
          {
  		    theAli->pic_interrupt(1,6+index);
            state.ide_bm_status[index] |= 4;
          }
	      break;

	    case 0x00: // nop
	      state.ide_status[index]= 0x40;
	      if (!(state.ide_control[index]&2))
          {
  		    theAli->pic_interrupt(1,6+index);
            state.ide_bm_status[index] |= 4;
          }
	      break;
        default:
	      state.ide_status[index] = 0x41;	// ERROR
	      state.ide_error[index] = 0x20;	// ABORTED

//#ifdef DEBUG_IDE
	      printf("%%IDE-I-UNKCMND : Unknown IDE Command: ");
	      for (x=0;x<8;x++) printf("%02x ",state.ide_command[index][x]);
	      printf("\n");
//#endif
	      if (!(state.ide_control[index]&2))
          {
  		    theAli->pic_interrupt(1,6+index);
            state.ide_bm_status[index] |= 4;
          }
	  }
	}
  }
  else
  {
#ifdef DEBUG_IDE
    if (address==7)
    {
      printf("%%IDE-I-NODRIVE : IDE Command for non-existing drive %d.%d: ",index,state.ide_selected[index]);
      for (x=0;x<8;x++) printf("%02x ",state.ide_command[index][x]);
        printf("\n");
    }
#endif
    state.ide_status[index] = 0;

  }
}

/**
 * Read from the IDE controller control interface.
 * Return status when \a address is 2, otherwise return 0.
 *
 * TO DO: this address range is a combination of IDE and floppy ports.
 *        Split it up. (3f0 - 3f4 = floppy, 3f6-3f7 = IDE (??)
 **/

u32 CAliM1543C_ide::ide_control_read(int index, u32 address)
{
  u32 data;

  data = 0;
  switch(address) 
  {
  case 0: //3f4 floppy main status register
    data = 0x80; // floppy ready
    break;
  case 1: //3f5 floppy command status register
    break;
  case 2: // 3f6: ide status

    //HACK FOR STRANGE ERROR WHEN SAVING/LOADING STATE
    if (state.ide_status[index]==0xb9)
      state.ide_status[index] = 0x40;
    //END HACK

    data = state.ide_status[index];
    break;
  default:
    data = 0;
  }

  TRC_DEV4("%%IDE-I-READCTRL: read port %d on IDE control %d: 0x%02x\n", (u32)(address), index, data);
#ifdef DEBUG_IDE
//  if (address!=2)
    printf("%%IDE-I-READCTRL: read port %d on IDE control %d: 0x%02x\n", (u32)(address), index, data);
#endif
  return data;
}

/**
 * Write to the IDE controller control interface.
 * Not functional
 **/

void CAliM1543C_ide::ide_control_write(int index, u32 address, u32 data)
{
  TRC_DEV4("%%IDE-I-WRITCRTL: write port %d on IDE control %d: 0x%02x\n",  (u32)(address),index, data);
#ifdef DEBUG_IDE
  printf("%%IDE-I-WRITCTRL: write port %d on IDE control %d: 0x%02x\n",  (u32)(address),index, data);
#endif

  switch(address) {
  case 0: // floppy main status register
    break;  // its read only
  case 1: // floppy command status register.
    break; // Ignored for now.
  case 2: // command register
    state.ide_control[index] = (data & 0xff) | 0x08;
  } 
}

/**
 * Read from the IDE controller busmaster interface.
 * Always returns 0.
 **/

u32 CAliM1543C_ide::ide_busmaster_read(int index, u32 address)
{
  u32 data;

  data = 0;
  if (address == 2)
    data = state.ide_bm_status[index];

  TRC_DEV4("%%IDE-I-READBUSM: read port %d on IDE bus master %d: 0x%02x\n", (u32)(address), index, data);
//#ifdef DEBUG_IDE
  printf("%%IDE-I-READBUSM: read port %d on IDE bus master %d: 0x%02x\n", (u32)(address), index, data);
//#endif
  return data;
}

/**
 * Write to the IDE controller busmaster interface.
 * Not functional.
 **/

void CAliM1543C_ide::ide_busmaster_write(int index, u32 address, u32 data)
{
  TRC_DEV4("%%IDE-I-WRITBUSM: write port %d on IDE bus master %d: 0x%02x\n",  (u32)(address),index, data);
//#ifdef DEBUG_IDE
  printf("%%IDE-I-WRITBUSM: write port %d on IDE bus master %d: 0x%02x\n",  (u32)(address),index, data);
//#endif

  if (address==2)
    state.ide_bm_status[index] &= ~data;
}


/**
 * Make sure a clock interrupt is generated on the next clock.
 * used for debugging, or to speed tings up when software is waiting for a clock tick.
 **/

void CAliM1543C_ide::ResetPCI()
{
  int i;

  CPCIDevice::ResetPCI();

  state.ide_status[0] = 0;
  state.ide_bm_status[0] = 0;
  state.ide_reading[0] = false;
  state.ide_writing[0] = false;
  state.ide_sectors[0] = 0;
  state.ide_selected[0] = 0;
  state.ide_error[0] = 0;

  state.ide_status[1] = 0;
  state.ide_bm_status[1] = 0;
  state.ide_reading[1] = false;
  state.ide_writing[1] = false;
  state.ide_sectors[1] = 0;
  state.ide_selected[1] = 0;
  state.ide_error[1] = 0;
}

/**
 * Save state to a Virtual Machine State file.
 **/

void CAliM1543C_ide::SaveState(FILE *f)
{
  CPCIDevice::SaveState(f);
  fwrite(&state,sizeof(state),1,f);
}

/**
 * Restore state from a Virtual Machine State file.
 **/

void CAliM1543C_ide::RestoreState(FILE *f)
{
  CPCIDevice::SaveState(f);
  fread(&state,sizeof(state),1,f);
}


CAliM1543C_ide * theAliIDE = 0;
