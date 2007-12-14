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
 * X-1.6        Camiel Vanderhoeven                             14-DEC-2007
 *      Commented out printing each IDE command.
 *
 * X-1.5        Camiel Vanderhoeven                             12-DEC-2007
 *      Use disk controller base class.
 *
 * X-1.4        Camiel Vanderhoeven                             11-DEC-2007
 *      Removed last references to ide_command[][].
 *
 * X-1.3        Camiel Vanderhoeven                             11-DEC-2007
 *      Cleanup.
 *
 * X-1.2        Camiel Vanderhoeven                             11-DEC-2007
 *      More complete IDE implementation allows NetBSD to recognize disks.
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
#include "Disk.h"

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
/*04*/  0x00000105, // CFCS: command + status
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

#define SEL_STATUS(a) state.ide_status[a][state.ide_selected[a]]
#define SEL_DISK(a) get_disk(a,state.ide_selected[a])
#define SEL_PER_DRIVE(a) state.ide_per_drive[a][state.ide_selected[a]]

/**
 * Constructor.
 **/

CAliM1543C_ide::CAliM1543C_ide(CConfigurator * cfg, CSystem * c, int pcibus, int pcidev) 
  : CDiskController(cfg,c,pcibus,pcidev,2,2)
{
  if (theAliIDE != 0)
    FAILURE("More than one AliIDE!!\n");
  theAliIDE = this;

  add_function(0,ide_cfg_data, ide_cfg_mask);

  add_legacy_io(14, 0x1f0, 8);
  add_legacy_io(16, 0x3f6, 1);
  add_legacy_io(15, 0x170, 8);
  add_legacy_io(17, 0x376, 1);
  add_legacy_io(18, 0xf000, 8);
  add_legacy_io(19, 0xf008, 8);
  
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
      return ide_command_read(channel,address,dsize);
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
      ide_command_write(channel,address, dsize,data);
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
      return ide_command_read(channel,address,dsize);
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
      ide_command_write(channel,address, dsize,data);
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

u32 CAliM1543C_ide::ide_command_read(int index, u32 address, int dsize)
{
  u32 data;

  switch (address)
  {
  case 0:
    if (!SEL_STATUS(index).drq) 
    {
      printf("IDE%d port 0 read with drq == 0: last command was %02xh", index, SEL_STATUS(index).current_command);
      return 0;
    }
    switch(SEL_STATUS(index).current_command)
    {
    case 0x20:
    case 0x21:
    case 0xec:
      data = 0;
      switch(dsize)
      {
      case 32:
        data = state.ide_data[index][state.ide_data_ptr[index]++];
        data |= state.ide_data[index][state.ide_data_ptr[index]++] << 16;
        break;
      case 16:
        data = state.ide_data[index][state.ide_data_ptr[index]++];
      }
      if (state.ide_data_ptr[index]>=256)
      {
        SEL_STATUS(index).busy = false;
        SEL_STATUS(index).drive_ready = true;
        SEL_STATUS(index).seek_complete = true;
        SEL_STATUS(index).err = false;
        state.ide_data_ptr[index] = 0;

        state.ide_sectors[index]--;

        if (!state.ide_sectors[index] || (SEL_STATUS(index).current_command == 0xec))
          SEL_STATUS(index).drq = false;
        else
        {
          SEL_STATUS(index).drq = true;
          SEL_DISK(index)->read_blocks(&(state.ide_data[index][0]),1);
          raise_interrupt(index);
        }
      }
      break;
    default:
      printf("IDE read with unsupported command: %02x\n",SEL_STATUS(index).current_command);
      exit(1);
    }
    break;
  case 1:
    data = state.ide_error[index]; // no error
    break;
  case 2:
    data = SEL_PER_DRIVE(index).sector_count;
    break;
  case 3:
    data = SEL_PER_DRIVE(index).sector_no;
    break;
  case 4:
    data = SEL_PER_DRIVE(index).cylinder_no & 0xff;
    break;
  case 5:
    data = (SEL_PER_DRIVE(index).cylinder_no>>8) & 0xff;
    break;
  case 6:
    data =                                  0x80 
         | (SEL_PER_DRIVE(index).lba_mode ? 0x40 : 0x00)
         |                                  0x20 //512 byte sector size
         | (state.ide_selected[index]     ? 0x10 : 0x00)
         | (SEL_PER_DRIVE(index).head_no  & 0x0f       );
    break;
  case 7:
      data = get_status(index);
      break;
    }
  TRC_DEV4("%%ALI-I-READIDECMD: read port %d on IDE command %d: 0x%02x\n", (u32)(address), index, data);
#ifdef DEBUG_IDE
  if (address)
    printf("%%ALI-I-READIDECMD: read port %d on IDE command %d: 0x%02x\n", (u32)(address), index, data);
#endif
  return data;
}

void CAliM1543C_ide::ide_command_write(int index, u32 address, int dsize, u32 data)
{
  int lba;

  TRC_DEV4("%%ALI-I-WRITEIDECMD: write port %d on IDE command %d: 0x%02x\n",  (u32)(address),index, data);

#ifdef DEBUG_IDE
  if (address)
    printf("%%ALI-I-WRITEIDECMD: write port %d on IDE command %d: 0x%02x\n",  (u32)(address),index, data);
#endif

  switch(address)
  {
  case 0:
    switch(SEL_STATUS(index).current_command)
    {
    case 0x30:
      switch(dsize)
      {
      case 32:
        state.ide_data[index][state.ide_data_ptr[index]++] = data & 0xffff;
        state.ide_data[index][state.ide_data_ptr[index]++] = (data>>16) & 0xffff;
        break;
      case 16:
        state.ide_data[index][state.ide_data_ptr[index]++] = data & 0xffff;
      }
      if (state.ide_data_ptr[index]>=256)
      {
        SEL_STATUS(index).busy = false;
        SEL_STATUS(index).drive_ready = true;
        SEL_STATUS(index).seek_complete = true;
        SEL_STATUS(index).err = false;
        state.ide_data_ptr[index] = 0;
        SEL_DISK(index)->write_blocks(&(state.ide_data[index][0]),1);
	    state.ide_sectors[index]--;
	    state.ide_data_ptr[index] = 0;

        if (!state.ide_sectors[index])
          SEL_STATUS(index).drq = false;
        else
          SEL_STATUS(index).drq = true;
        raise_interrupt(index);
      }
      break;
    default:
      printf("IDE write with unsupported command: %02x\n",SEL_STATUS(index).current_command);
    }
    break;
  case 1:
    state.ide_per_drive[index][0].features = data;
    state.ide_per_drive[index][1].features = data;
    // ignore precompensation
    break;
  case 2:
    state.ide_per_drive[index][0].sector_count = data & 0xff;
    state.ide_per_drive[index][1].sector_count = data & 0xff;
    break;
  case 3:
    state.ide_per_drive[index][0].sector_no = data & 0xff;
    state.ide_per_drive[index][1].sector_no = data & 0xff;
    break;
  case 4:
    state.ide_per_drive[index][0].cylinder_no = (state.ide_per_drive[index][0].cylinder_no & 0xff00) | (data & 0xff);
    state.ide_per_drive[index][1].cylinder_no = (state.ide_per_drive[index][1].cylinder_no & 0xff00) | (data & 0xff);
    break;
  case 5:
    state.ide_per_drive[index][0].cylinder_no = (state.ide_per_drive[index][0].cylinder_no & 0xff) | ((data<<8) & 0xff00);
    state.ide_per_drive[index][1].cylinder_no = (state.ide_per_drive[index][1].cylinder_no & 0xff) | ((data<<8) & 0xff00);
    break;
  case 6:
    state.ide_selected[index] = (data >> 4) & 1;
    state.ide_per_drive[index][0].head_no = data & 0x0f;
    state.ide_per_drive[index][1].head_no = data & 0x0f;
    state.ide_per_drive[index][0].lba_mode = (data >> 6) & 1;
    state.ide_per_drive[index][1].lba_mode = (data >> 6) & 1;
    break;
  case 7:
    if (state.ide_selected[index] && !SEL_DISK(index))
      // ignore command for non-existing slave.
      break;

    if(SEL_STATUS(index).busy)
    {
      printf("Command sent to busy IDE device!\n");
      break;
    }

    SEL_STATUS(index).err = false;

    if ( (data & 0xf0) == 0x10 )
      data = 0x10;

//    printf("IDE Command %02x\n",data);

    switch (data)
    {
    case 0x00: // nop
        SEL_STATUS(index).drive_ready = true;
        SEL_STATUS(index).drq = false;
        raise_interrupt(index);
      break;

    case 0x08: // reset drive (DRST)
//#ifdef DEBUG_IDE
      printf("%%IDE-I-RESET: IDE Reset\n");
//#endif
      command_aborted(index,0x08);
      break;

    case 0x10: // CALIBRATE DRIVE

      if (!SEL_DISK(index)) {
        state.ide_error[index] = 0x02; // Track 0 not found
        SEL_STATUS(index).busy = false;
        SEL_STATUS(index).drive_ready = true;
        SEL_STATUS(index).seek_complete = false;
        SEL_STATUS(index).drq = false;
        SEL_STATUS(index).err = true;
        raise_interrupt(index);
        printf("calibrate drive: disk ata%d-%d not present\n", index,state.ide_selected[index]);
        break;
      }

      /* move head to cylinder 0, issue IRQ */
      state.ide_error[index] = 0;
      SEL_STATUS(index).busy = false;
      SEL_STATUS(index).drive_ready = true;
      SEL_STATUS(index).seek_complete = true;
      SEL_STATUS(index).drq = false;
      SEL_STATUS(index).err = false;
      SEL_PER_DRIVE(index).cylinder_no = 0;
      raise_interrupt(index);
      break;

    case 0x20: // read sector, with retries
    case 0x21: // read sector, without retries
      if (!SEL_DISK(index))
      {
        printf("Read from non-existing disk!\n");
        exit(1);
      }
      if (!SEL_PER_DRIVE(index).lba_mode)
      {
        printf("Non-LBA mode!!\n");
        exit(1);
      }
      else
      {
        lba = (SEL_PER_DRIVE(index).head_no << 24)
            | (SEL_PER_DRIVE(index).cylinder_no << 8)
            | SEL_PER_DRIVE(index).sector_no;
      }
      if (!SEL_PER_DRIVE(index).sector_count)
        state.ide_sectors[index] = 256;
      else
        state.ide_sectors[index] = SEL_PER_DRIVE(index).sector_count;
      TRC_DEV5("%%IDE-I-READSECT: Read  %3d sectors @ IDE %d.%d LBA %8d\n",SEL_PER_DRIVE(index).sector_count,index,state.ide_selected[index],lba);
#ifdef DEBUG_IDE
      printf("%%IDE-I-READSECT: Read  %3d sectors @ IDE %d.%d LBA %8d\n",SEL_PER_DRIVE(index).sector_count,index,state.ide_selected[index],lba);
#endif
      SEL_STATUS(index).current_command = data;

      SEL_DISK(index)->seek_block(lba);
      SEL_DISK(index)->read_blocks(&(state.ide_data[index][0]),1);
	  state.ide_data_ptr[index] = 0;
      state.ide_error[index] = 0;
      SEL_STATUS(index).busy = false;
      SEL_STATUS(index).drive_ready = true;
      SEL_STATUS(index).seek_complete = true;
      SEL_STATUS(index).drq = true;
      raise_interrupt(index);
      break;

    case 0x30: // write sectors, with retries
      if (!SEL_DISK(index))
      {
        printf("Write to non-existing disk!\n");
        exit(1);
      }
      if (!SEL_PER_DRIVE(index).lba_mode)
      {
        printf("Non-LBA mode!!\n");
        exit(1);
      }
      else
      {
        lba = (SEL_PER_DRIVE(index).head_no << 24)
            | (SEL_PER_DRIVE(index).cylinder_no << 8)
            | SEL_PER_DRIVE(index).sector_no;
      }
      if (!SEL_PER_DRIVE(index).sector_count)
        state.ide_sectors[index] = 256;
      else
        state.ide_sectors[index] = SEL_PER_DRIVE(index).sector_count;
      TRC_DEV5("%%IDE-I-READSECT: Read  %3d sectors @ IDE %d.%d LBA %8d\n",SEL_PER_DRIVE(index).sector_count,index,state.ide_selected[index],lba);
#ifdef DEBUG_IDE
      printf("%%IDE-I-WRITSECT: Write  %3d sectors @ IDE %d.%d LBA %8d\n",SEL_PER_DRIVE(index).sector_count,index,state.ide_selected[index],lba);
#endif
      SEL_STATUS(index).current_command = data;

      SEL_DISK(index)->seek_block(lba);
      state.ide_data_ptr[index] = 0;
      state.ide_error[index] = 0;
      SEL_STATUS(index).busy = false;
      SEL_STATUS(index).drive_ready = true;
      SEL_STATUS(index).seek_complete = true;
      SEL_STATUS(index).drq = true;
      break;

	case 0x91:			// SET TRANSLATION
//#ifdef DEBUG_IDE
      printf("%%IDE-I-SETTRANS: Set IDE translation\n");
//#endif
      if (!SEL_DISK(index))
      {
        printf("init drive params: ide%d.%d not present\n",index,state.ide_selected[index]);
        SEL_STATUS(index).busy = false;
        SEL_STATUS(index).drive_ready = true;
        SEL_STATUS(index).drq = false;
        raise_interrupt(index);
        break;
      }
      SEL_STATUS(index).busy = false;
      SEL_STATUS(index).drive_ready = true;
      SEL_STATUS(index).drq = false;
      break;

    case 0xc6: // SET MULTIPLE MODE
      if (SEL_DISK(index)->cdrom())
      {
        printf("set multiple mode issued to non-disk\n");
        command_aborted(index, data);
      } 
      else if ((SEL_PER_DRIVE(index).sector_count > MAX_MULTIPLE_SECTORS) ||
          ((SEL_PER_DRIVE(index).sector_count & (SEL_PER_DRIVE(index).sector_count - 1)) != 0) ||
          (SEL_PER_DRIVE(index).sector_count == 0)) 
      {
        command_aborted(index, data);
      } else {
        printf("set multiple mode: sectors=%d", SEL_PER_DRIVE(index).sector_count);
        //SEL_STATUS(index).multiple_sectors = SEL_PER_DRIVE(index).sector_count;
        SEL_STATUS(index).busy = false;
        SEL_STATUS(index).drive_ready = true;
        SEL_STATUS(index).drq = false;
        raise_interrupt(index);
      }
      break;

    // power management & flush cache stubs
    case 0xE0: // STANDBY NOW
    case 0xE1: // IDLE IMMEDIATE
    case 0xE7: // FLUSH CACHE
    case 0xEA: // FLUSH CACHE EXT
      SEL_STATUS(index).busy = false;
      SEL_STATUS(index).drive_ready = true;
      SEL_STATUS(index).drq = false;
      raise_interrupt(index);
      break;

    case 0xe5: // CHECK POWER MODE
      SEL_STATUS(index).busy = false;
      SEL_STATUS(index).drive_ready = true;
      SEL_STATUS(index).drq = false;
      SEL_PER_DRIVE(index).sector_count = 0xff; // Active or Idle mode
      raise_interrupt(index);
      break;

    case 0xec:
//#ifdef DEBUG_IDE
	  printf("%%IDE-I-IDENTIFY: Identify IDE disk %d.%d   \n",index,state.ide_selected[index]);
//#endif
      if (!SEL_DISK(index))
      {
        printf("Disk ^%d.%d not present, aborting.\n",index,state.ide_selected[index]);
        command_aborted(index,data);
        break;
      }

      SEL_STATUS(index).current_command = data;
      state.ide_error[index] = 0;

      SEL_STATUS(index).busy = false;
      SEL_STATUS(index).drive_ready = true;
      SEL_STATUS(index).drq = true;
      SEL_STATUS(index).seek_complete = true;

      identify_drive(index);

      raise_interrupt(index);
      break;

    case 0xef: // SET FEATURES
      switch(SEL_PER_DRIVE(index).features) 
      {
      case 0x03: // Set Transfer Mode
        {
          u8 type = SEL_PER_DRIVE(index).sector_count >> 3;
          u8 mode = SEL_PER_DRIVE(index).sector_count & 0x07;
          switch (type) 
          {
            case 0x00: // PIO default
            case 0x01: // PIO mode
              printf("ide%d.%d: set transfer mode to PIO", index, state.ide_selected[index]);
              //SEL_STATUS(index).mdma_mode = 0x00;
              //SEL_STATUS(index).udma_mode = 0x00;
              SEL_STATUS(index).drive_ready = true;
              SEL_STATUS(index).seek_complete = true;
              raise_interrupt(index);
              break;
            case 0x04: // MDMA mode
              printf("ide%d.%d: set transfer mode to MDMA%d", index, state.ide_selected[index],mode);
              //SEL_STATUS(index).mdma_mode = 1 << mode;
              //SEL_STATUS(index).udma_mode = 0x00;
              SEL_STATUS(index).drive_ready = true;
              SEL_STATUS(index).seek_complete = true;
              raise_interrupt(index);
              break;
            case 0x08: // UDMA mode
              printf("ide%d.%d: set transfer mode to UDMA%d", index, state.ide_selected[index],mode);
              //SEL_STATUS(index).mdma_mode = 0x00;
              //SEL_STATUS(index).udma_mode = 1 << mode;
              SEL_STATUS(index).drive_ready = true;
              SEL_STATUS(index).seek_complete = true;
              raise_interrupt(index);
              break;
            default:
              printf("ide%d.%d: set transfer mode to UNKNOWN %02x-%02x", index, state.ide_selected[index],type,mode);
              raise_interrupt(index);
              command_aborted(index, data);
            }
            break;
          }
        case 0x02: // Enable and
        case 0x82: //  Disable write cache.
        case 0xAA: // Enable and
        case 0x55: //  Disable look-ahead cache.
        case 0xCC: // Enable and
        case 0x66: //  Disable reverting to power-on default
          printf("ide%d.%d: SET FEATURES subcommand 0x%02x not supported, but returning success",
          index,state.ide_selected[index],SEL_PER_DRIVE(index).features);
          SEL_STATUS(index).drive_ready = true;
          SEL_STATUS(index).seek_complete = true;
          raise_interrupt(index);
          break;

        default:
          printf("ide%d.%d: SET FEATURES with unknown subcommand: 0x%02x",
            index,state.ide_selected[index],SEL_PER_DRIVE(index).features);
          command_aborted(index, data);
      }
      break;

    default:
      printf("Unknown command: %02x!\n",data);
      exit(1);
    }
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

  data = get_status(index);

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
  bool prev_reset;

  TRC_DEV4("%%IDE-I-WRITCRTL: write port %d on IDE control %d: 0x%02x\n",  (u32)(address),index, data);
//#ifdef DEBUG_IDE
  printf("%%IDE-I-WRITCTRL: write port %d on IDE control %d: 0x%02x\n",  (u32)(address),index, data);
//#endif

  prev_reset = state.ide_control[index].reset;
  state.ide_control[index].reset       = (data>>2) & 1;
  state.ide_control[index].disable_irq = (data>>1) & 1;

  if (!prev_reset && state.ide_control[index].reset)
  {
    printf("IDE reset on index %d started.\n",index);
    
    state.ide_status[index][0].busy = true;
    state.ide_status[index][0].drive_ready = false;
    state.ide_status[index][0].seek_complete = true;
    state.ide_status[index][0].drq = false;
    state.ide_status[index][0].err = false;
    state.ide_status[index][0].current_command = 0;
    state.ide_status[index][1].busy = true;
    state.ide_status[index][1].drive_ready = false;
    state.ide_status[index][1].seek_complete = true;
    state.ide_status[index][1].drq = false;
    state.ide_status[index][1].err = false;
    state.ide_status[index][1].current_command = 0;

    state.ide_reset_in_progress[index] = true;
    state.ide_error[index] = 0x01; // no error
    state.ide_status[index][0].current_command = 0;
    state.ide_sectors[index] = 0;
    state.ide_control[index].disable_irq = false;
  }
  else if (prev_reset && !state.ide_control[index].reset)
  {
    printf("IDE reset on index %d ended.\n",index);
    state.ide_status[index][0].busy = false;
    state.ide_status[index][0].drive_ready = true;
    state.ide_status[index][1].busy = false;
    state.ide_status[index][1].drive_ready = true;
    state.ide_reset_in_progress[index] = false;

    set_signature(index,0);
    set_signature(index,1);
  }
}

void CAliM1543C_ide::set_signature(int index, int id)
{
  // Device signature
  state.ide_per_drive[index][id].head_no       = 0;
  state.ide_per_drive[index][id].sector_count  = 1;
  state.ide_per_drive[index][id].sector_no     = 1;
  if (get_disk(index,id))
  {
    if (!get_disk(index,id)->cdrom())
    {
      state.ide_per_drive[index][id].cylinder_no = 0;
      state.ide_selected[index] = 0;
    } else {
      state.ide_per_drive[index][id].cylinder_no = 0xeb14;
    } 
  } else {
    state.ide_per_drive[index][id].cylinder_no = 0xffff;
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

void CAliM1543C_ide::raise_interrupt(int index)
{
  if (!state.ide_control[index].disable_irq) 
  {
    state.ide_bm_status[index] |= 0x04;
    theAli->pic_interrupt(1, 6+index);
  }
}


/**
 * Make sure a clock interrupt is generated on the next clock.
 * used for debugging, or to speed tings up when software is waiting for a clock tick.
 **/

void CAliM1543C_ide::ResetPCI()
{
  int i,j;

  CPCIDevice::ResetPCI();

  for (i=0;i<2;i++)
  {
    state.ide_error[i] = 0;
    state.ide_bm_status[i] = 0;
    state.ide_sectors[i] = 0;
    state.ide_selected[i] = 0;

    for (j=0;j<2;j++)
    {
      state.ide_status[i][j].busy = false;
      state.ide_status[i][j].drive_ready = false;
      state.ide_status[i][j].drq = false;
      state.ide_status[i][j].err = false;
      state.ide_status[i][j].index_pulse = false;
      state.ide_status[i][j].index_pulse_count = 0;
      state.ide_status[i][j].seek_complete = false;
    }
  }
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


u8 CAliM1543C_ide::get_status(int index)
{
  u8 data;

  if (!SEL_DISK(index))
    return 0;

  data = (SEL_STATUS(index).busy           ? 0x80 : 0x00)
       | (SEL_STATUS(index).drive_ready    ? 0x40 : 0x00)
       | (SEL_STATUS(index).seek_complete  ? 0x10 : 0x00)
       | (SEL_STATUS(index).drq            ? 0x08 : 0x00)
       | (SEL_STATUS(index).index_pulse    ? 0x02 : 0x00)
       | (SEL_STATUS(index).err            ? 0x01 : 0x00);
  SEL_STATUS(index).index_pulse_count++;
  SEL_STATUS(index).index_pulse = false;
  if (SEL_STATUS(index).index_pulse_count >= 10)
  {
    SEL_STATUS(index).index_pulse_count = 0;
    SEL_STATUS(index).index_pulse = true;
  }

  return data;
}

void CAliM1543C_ide::identify_drive(int index)
{
  char serial_number[21];
  char model_number[41];
  char rev_number[9];
  int i,l;

  state.ide_data_ptr[index] = 0;

  state.ide_data[index][0] = 0x0040;	// flags
  
  if (SEL_DISK(index)->get_cylinders() > 16383)
    state.ide_data[index][1] = 16383;	// cylinders
  else
    state.ide_data[index][1] = SEL_DISK(index)->get_cylinders();	// cylinders

  state.ide_data[index][2] = 0xc837;	// specific configuration (ATA-4 specs)

  state.ide_data[index][3] = SEL_DISK(index)->get_heads();		// heads
  state.ide_data[index][4] = 512 * SEL_DISK(index)->get_sectors();	// bytes per track
  state.ide_data[index][5] = 512;		// bytes per sector
  state.ide_data[index][6] = SEL_DISK(index)->get_sectors();		// sectors per track
  state.ide_data[index][7] = 0;		// spec. bytes
  state.ide_data[index][8] = 0;		// spec. bytes
  state.ide_data[index][9] = 0;		// unique vendor status words

  strcpy(serial_number,"                    ");
  i = strlen(SEL_DISK(index)->get_serial());
  i = (i > 20)? 20 : i;
  memcpy(model_number,SEL_DISK(index)->get_serial(),i);
  for (i=0;i<10;i++)
    state.ide_data[index][10+i] = (serial_number[i*2] << 8) |
      serial_number[i*2 + 1];

  state.ide_data[index][20] = 1;		// single ported, single buffer
  state.ide_data[index][21] = 512;  	// buffer size
  state.ide_data[index][22] = 4;		// ecc bytes

  strcpy(rev_number,"        ");
  i = strlen(SEL_DISK(index)->get_rev());
  i = (i > 8)? 8 : i;
  memcpy(model_number,SEL_DISK(index)->get_rev(),i);
  for (i=0;i<4;i++)
    state.ide_data[index][23+i] = (rev_number[i*2] << 8) |
      rev_number[i*2 + 1];

  strcpy(model_number,"                                        ");
  // clear the name
  i = strlen(SEL_DISK(index)->get_model());
  i = (i > 40)? 40 : i;
  memcpy(model_number,SEL_DISK(index)->get_model(),i);
  for(i=0;i<20;i++) 
    state.ide_data[index][i+27]= (model_number[i*2] << 8) | model_number[i*2+1];

  state.ide_data[index][47] = 0;		// read/write multiples
  state.ide_data[index][48] = 1;		// double-word IO transfers supported
  state.ide_data[index][49] = 0x0200;		// capability LBA
  state.ide_data[index][50] = 0;
  state.ide_data[index][51] = 0x200;		// cycle time
  state.ide_data[index][52] = 0x200;		// cycle time
  state.ide_data[index][53] = 7;			// field_valid

  state.ide_data[index][54] = SEL_DISK(index)->get_cylinders();		// cylinders
  state.ide_data[index][55] = SEL_DISK(index)->get_heads();		// heads
  state.ide_data[index][56] = SEL_DISK(index)->get_sectors();		// sectors

  state.ide_data[index][57] = (SEL_DISK(index)->get_chs_size() >> 0)  & 0xFFFF;	// total_sectors
  state.ide_data[index][58] = (SEL_DISK(index)->get_chs_size() >> 16) & 0xFFFF;	// ""
  state.ide_data[index][59] = 0;							// multiple sector count
  state.ide_data[index][60] = (SEL_DISK(index)->get_lba_size() >> 0)  & 0xFFFF;	// LBA capacity
  state.ide_data[index][61] = (SEL_DISK(index)->get_lba_size() >> 16) & 0xFFFF;	// ""
  
  state.ide_data[index][62] = 0;
  state.ide_data[index][63] = 0;
  state.ide_data[index][64] = 0;
  state.ide_data[index][65] = 120;
  state.ide_data[index][66] = 120;
  state.ide_data[index][67] = 120;
  state.ide_data[index][68] = 120;
  state.ide_data[index][80] = 0x0e;
  state.ide_data[index][81] = 0;
  state.ide_data[index][82] = 0x4000;
  state.ide_data[index][83] = 0x5000;
  state.ide_data[index][84] = 0x4000;
  state.ide_data[index][85] = 0x4000;
  state.ide_data[index][86] = 0x5000;
  state.ide_data[index][87] = 0x4000;
  state.ide_data[index][88] = 0;
  state.ide_data[index][93] = 0x6001;
}

void CAliM1543C_ide::command_aborted(int index, u8 command)
{
  printf("ide%d.%d aborting on command 0x%02x \n", index, state.ide_selected[index], command);
  SEL_STATUS(index).current_command = 0;
  SEL_STATUS(index).busy = false;
  SEL_STATUS(index).drive_ready = true;
  SEL_STATUS(index).err = true;
  SEL_STATUS(index).drq = false;

  state.ide_error[index] = 0x04; // command ABORTED
  state.ide_data_ptr[index] = 0;
  raise_interrupt(index);
}

CAliM1543C_ide * theAliIDE = 0;