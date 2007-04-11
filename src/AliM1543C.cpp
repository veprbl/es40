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
 * Contains the code for the emulated Ali M1543C chipset devices.
 *
 * \bug When restoring state, the ide_status may be 0xb9...
 *
 * X-1.24       Camiel Vanderhoeven                             11-APR-2007
 *      Moved all data that should be saved to a state file to a structure
 *      "state".
 *
 * X-1.23	Camiel Vanderhoeven				3-APR-2007
 *	Fixed wrong IDE configuration mask (address ranges masked were too 
 *	short, leading to overlapping memory regions.)	
 *
 * X-1.22	Camiel Vanderhoeven				1-APR-2007
 *	Uncommented the IDE debugging statements.
 *
 * X-1.21       Camiel Vanderhoeven                             31-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.20	Camiel Vanderhoeven				30-MAR-2007
 *	Unintentional CVS commit / version number increase.
 *
 * X-1.19	Camiel Vanderhoeven				27-MAR-2007
 *   a) When DEBUG_PIC is defined, generate more debugging messages.
 *   b)	When an interrupt originates from the cascaded interrupt controller,
 *	the interrupt vector from the cascaded controller is returned.
 *   c)	When interrupts are ended on the cascaded controller, and no 
 *	interrupts are left on that controller, the cascade interrupt (2)
 *	on the primary controller is ended as well. I'M NOT COMPLETELY SURE
 *	IF THIS IS CORRECT, but what goes on in OpenVMS seems to imply this.
 *   d) When the system state is saved to a vms file, and then restored, the
 *	ide_status may be 0xb9, this bug has not been found yet, but as a 
 *	workaround, we detect the value 0xb9, and replace it with 0x40.
 *   e) Changed the values for cylinders/heads/sectors on the IDE identify
 *	command, because it looks like OpenVMS' DQDRIVER doesn't like it if
 *	the number of sectors is greater than 63.
 *   f) All IDE commands generate an interrupt upon completion.
 *   g) IDE command SET TRANSLATION (0x91) is recognized, but has no effect.
 *	This is allright, as long as OpenVMS NEVER DOES CHS-mode access to 
 *	the disk.
 *
 * X-1.18	Camiel Vanderhoeven				26-MAR-2007
 *   a) Specific-EOI's (end-of-interrupt) now only end the interrupt they 
 *	are meant for.
 *   b) When DEBUG_PIC is defined, debugging messages for the interrupt 
 *	controllers are output to the console, same with DEBUG_IDE and the
 *	IDE controller.
 *   c) If IDE registers for a non-existing drive are read, 0xff is returned.
 *   d) Generate an interrupt when a sector is read or written from a disk.
 *
 * X-1.17	Camiel Vanderhoeven				1-MAR-2007
 *   a) Accesses to IDE-configuration space are byte-swapped on a big-endian 
 *	architecture. This is done through the endian_bits macro.
 *   b) Access to the IDE databuffers (16-bit transfers) are byte-swapped on 
 *	a big-endian architecture. This is done through the endian_16 macro.
 *
 * X-1.16	Camiel Vanderhoeven				20-FEB-2007
 *	Write sectors to disk when the IDE WRITE command (0x30) is executed.
 *
 * X-1.15	Brian Wheeler					20-FEB-2007
 *	Information about IDE disks is now kept in the ide_info structure.
 *
 * X-1.14	Camiel Vanderhoeven				16-FEB-2007
 *   a) This is now a slow-clocked device.
 *   b) Removed #ifdef _WIN32 from printf statements.
 *
 * X-1.13	Brian Wheeler					13-FEB-2007
 *      Corrected some typecasts in printf statements.
 *
 * X-1.12	Camiel Vanderhoeven				12-FEB-2007
 *	Added comments.
 *
 * X-1.11       Camiel Vanderhoeven                             9-FEB-2007
 *      Replaced f_ variables with ide_ members.
 *
 * X-1.10       Camiel Vanderhoeven                             9-FEB-2007
 *      Only open an IDE disk image, if there is a filename.
 *
 * X-1.9 	Brian Wheeler					7-FEB-2007
 *	Load disk images according to the configuration file.
 *
 * X-1.8	Camiel Vanderhoeven				7-FEB-2007
 *   a)	Removed a lot of pointless messages.
 *   b)	Calls to trace_dev now use the TRC_DEVx macro's.
 *
 * X-1.7	Camiel Vanderhoeven				3-FEB-2007
 *      Removed last conditional for supporting another system than an ES40
 *      (#ifdef DS15)
 *
 * X-1.6        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.5	Brian Wheeler					3-FEB-2007
 *	Fixed some problems with sprintf statements.
 *
 * X-1.4	Brian Wheeler					3-FEB-2007
 *	Space for 4 disks in f_img.
 *
 * X-1.3        Brian Wheeler                                   3-FEB-2007
 *      Scanf, printf and 64-bit literals made compatible with 
 *	Linux/GCC/glibc.
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
#include "AliM1543C.h"
#include "System.h"

#ifdef DEBUG_PIC
bool pic_messages = false;
#endif

/**
 * Constructor.
 **/

CAliM1543C::CAliM1543C(CSystem * c): CSystemComponent(c)
{
  int i;
  char buffer[64];
  char * filename;

  for(i=0;i<4;i++) {
    int C = i/2;
    int D = i%2;

    ide_info[C][D].handle = NULL;
    ide_info[C][D].filename = NULL;

    sprintf(buffer,"disk.%d",i);
    filename=c->GetConfig(buffer,NULL);
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
        char *p=(char *)malloc(strlen(filename)+1);
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
  
  c->RegisterMemory(this, 1, X64(00000801fc000060),8);
  state.kb_intState = 0;
  state.kb_Status = 0;
  state.kb_Output = 0;
  state.kb_Command = 0;
  state.kb_Input = 0;
  state.reg_61 = 0;

  c->RegisterMemory(this, 2, X64(00000801fc000070), 4);
  for (i=0;i<4;i++)
    state.toy_access_ports[i] = 0;
  for (i=0;i<256;i++)
    state.toy_stored_data[i] = 0;
  //    state.toy_stored_data[0x17] = 1;

  c->RegisterMemory(this, 3, X64(00000801fe003800),0x100);

  for (i=0;i<256;i++) 
    {
      state.isa_config_data[i] = 0;
      state.isa_config_mask[i] = 0;
    }
  state.isa_config_data[0x00] = 0xb9;
  state.isa_config_data[0x01] = 0x10;
  state.isa_config_data[0x02] = 0x33;
  state.isa_config_data[0x03] = 0x15;
  state.isa_config_data[0x04] = 0x0f;
  state.isa_config_data[0x07] = 0x02;
  state.isa_config_data[0x08] = 0xc3;
  state.isa_config_data[0x0a] = 0x01; 
  state.isa_config_data[0x0b] = 0x06;
  state.isa_config_data[0x55] = 0x02;
  state.isa_config_mask[0x40] = 0x7f;
  state.isa_config_mask[0x41] = 0xff;
  state.isa_config_mask[0x42] = 0xcf;
  state.isa_config_mask[0x43] = 0xff;
  state.isa_config_mask[0x44] = 0xdf;
  state.isa_config_mask[0x45] = 0xcb;
  state.isa_config_mask[0x47] = 0xff;
  state.isa_config_mask[0x48] = 0xff;
  state.isa_config_mask[0x49] = 0xff;
  state.isa_config_mask[0x4a] = 0xff;
  state.isa_config_mask[0x4b] = 0xff;
  state.isa_config_mask[0x4c] = 0xff;
  state.isa_config_mask[0x50] = 0xff;
  state.isa_config_mask[0x51] = 0x8f;
  state.isa_config_mask[0x52] = 0xff;
  state.isa_config_mask[0x53] = 0xff;
  state.isa_config_mask[0x55] = 0xff;
  state.isa_config_mask[0x56] = 0xff;
  state.isa_config_mask[0x57] = 0xf0;
  state.isa_config_mask[0x58] = 0x7f;
  state.isa_config_mask[0x59] = 0x0d;
  state.isa_config_mask[0x5a] = 0x0f;
  state.isa_config_mask[0x5b] = 0x03;
  // ...
									
  c->RegisterMemory(this, 6, X64(00000801fc000040), 4);
  c->RegisterClock(this, true);
  state.pit_enable = false;

  c->RegisterMemory(this, 7, X64(00000801fc000020), 2);
  c->RegisterMemory(this, 8, X64(00000801fc0000a0), 2);
  c->RegisterMemory(this, 20, X64(00000801f8000000), 1);
  for(i=0;i<2;i++)
    {
      state.pic_mode[i] = 0;
      state.pic_intvec[i] = 0;
      state.pic_mask[i] = 0;
      state.pic_asserted[i] = 0;
    }

  c->RegisterMemory(this, 9, X64(00000801fe007800), 0x100);
  cSystem->RegisterMemory(this,14, X64(00000801fc0001f0), 8);
  cSystem->RegisterMemory(this,16, X64(00000801fc0003f4), 4);
  cSystem->RegisterMemory(this,15, X64(00000801fc000170), 8);
  cSystem->RegisterMemory(this,17, X64(00000801fc000374), 4);
  cSystem->RegisterMemory(this,18, X64(00000801fc00f000), 8);
  cSystem->RegisterMemory(this,19, X64(00000801fc00f008), 8);

  for (i=0;i<256;i++)
    {
      state.ide_config_data[i] = 0;
      state.ide_config_mask[i] = 0;
    }
  state.ide_config_data[0x00] = 0xb9;	// vendor
  state.ide_config_data[0x01] = 0x10;
  state.ide_config_data[0x02] = 0x29;	// device
  state.ide_config_data[0x03] = 0x52;
  state.ide_config_data[0x06] = 0x80;	// status
  state.ide_config_data[0x07] = 0x02;	
  state.ide_config_data[0x08] = 0x1c;	// revision
  state.ide_config_data[0x09] = 0xFA;	// class code	
  state.ide_config_data[0x0a] = 0x01;	
  state.ide_config_data[0x0b] = 0x01;

  state.ide_config_data[0x10] = 0xF1;	// address I	
  state.ide_config_data[0x11] = 0x01;
  state.ide_config_data[0x14] = 0xF5;	// address II	
  state.ide_config_data[0x15] = 0x03;
  state.ide_config_data[0x18] = 0x71;	// address III	
  state.ide_config_data[0x19] = 0x01;
  state.ide_config_data[0x1c] = 0x75;	// address IV	
  state.ide_config_data[0x1d] = 0x03;
  state.ide_config_data[0x20] = 0x01;	// address V	
  state.ide_config_data[0x21] = 0xF0;

  state.ide_config_data[0x3d] = 0x01;	// interrupt pin	
  state.ide_config_data[0x3e] = 0x02;	// min_gnt
  state.ide_config_data[0x3f] = 0x04;	// max_lat	
  state.ide_config_data[0x4b] = 0x4a;	// udma test
  state.ide_config_data[0x4e] = 0xba;	// reserved	
  state.ide_config_data[0x4f] = 0x1a;
  state.ide_config_data[0x54] = 0x55;	// fifo treshold ch 1	
  state.ide_config_data[0x55] = 0x55;	// fifo treshold ch 2
  state.ide_config_data[0x56] = 0x44;	// udma setting ch 1	
  state.ide_config_data[0x57] = 0x44;	// udma setting ch 2
  state.ide_config_data[0x78] = 0x21;	// ide clock	

  //

  state.ide_config_mask[0x04] = 0x45;	// command

  state.ide_config_mask[0x0d] = 0xff;	// latency timer

  state.ide_config_mask[0x10] = 0xf8;	// address I
  state.ide_config_mask[0x11] = 0xff;	
  state.ide_config_mask[0x12] = 0xff;	
  state.ide_config_mask[0x13] = 0xff;	
  state.ide_config_mask[0x14] = 0xfc;	// address II
  state.ide_config_mask[0x15] = 0xff;	
  state.ide_config_mask[0x16] = 0xff;	
  state.ide_config_mask[0x17] = 0xff;	
  state.ide_config_mask[0x18] = 0xf8;	// address III
  state.ide_config_mask[0x19] = 0xff;	
  state.ide_config_mask[0x1a] = 0xff;	
  state.ide_config_mask[0x1b] = 0xff;	
  state.ide_config_mask[0x1c] = 0xfc;	// address IV
  state.ide_config_mask[0x1d] = 0xff;	
  state.ide_config_mask[0x1e] = 0xff;	
  state.ide_config_mask[0x1f] = 0xff;	
  state.ide_config_mask[0x20] = 0xf0;	// address V
  state.ide_config_mask[0x21] = 0xff;	
  state.ide_config_mask[0x22] = 0xff;	
  state.ide_config_mask[0x23] = 0xff;	

  state.ide_config_mask[0x3c] = 0xff;	// interrupt
  state.ide_config_mask[0x11] = 0xff;	
  state.ide_config_mask[0x12] = 0xff;	
  state.ide_config_mask[0x13] = 0xff;	

  state.ide_status[0] = 0;
  state.ide_reading[0] = false;
  state.ide_writing[0] = false;
  state.ide_sectors[0] = 0;
  state.ide_selected[0] = 0;
  state.ide_error[0] = 0;

  state.ide_status[1] = 0;
  state.ide_reading[1] = false;
  state.ide_writing[1] = false;
  state.ide_sectors[1] = 0;
  state.ide_selected[1] = 0;
  state.ide_error[1] = 0;

  c->RegisterMemory(this, 11, X64(00000801fe009800), 0x100);
  for (i=0;i<256;i++)
    {
      state.usb_config_data[i] = 0;
      state.usb_config_mask[i] = 0;
    }
  state.usb_config_data[0x00] = 0xb9;
  state.usb_config_data[0x01] = 0x10;
  state.usb_config_data[0x02] = 0x37;
  state.usb_config_data[0x03] = 0x52;
  state.usb_config_mask[0x04] = 0x13;
  state.usb_config_data[0x05] = 0x02;	state.usb_config_mask[0x05] = 0x01;
  state.usb_config_data[0x06] = 0x80;
  state.usb_config_data[0x07] = 0x02;
  state.usb_config_data[0x09] = 0x10;
  state.usb_config_data[0x0a] = 0x03;
  state.usb_config_data[0x0b] = 0x0c;
  state.usb_config_mask[0x0c] = 0x08;
  state.usb_config_mask[0x0d] = 0xff;
  state.usb_config_mask[0x11] = 0xf0;
  state.usb_config_mask[0x12] = 0xff;
  state.usb_config_mask[0x13] = 0xff;
  state.usb_config_mask[0x3c] = 0xff;
  state.usb_config_data[0x3d] = 0x01;	state.usb_config_mask[0x3d] = 0xff;
  state.usb_config_mask[0x3e] = 0xff;
  state.usb_config_mask[0x3f] = 0xff;


  c->RegisterMemory(this, 12, X64(00000801fc000000), 16);
  c->RegisterMemory(this, 13, X64(00000801fc0000c0), 32);

  printf("%%ALI-I-INIT: ALi M1543C chipset emulator initialized.\n");
  printf("%%ALI-I-IMPL: Implemented: keyboard, port 61, toy clock, isa bridge, flash ROM.\n");
}

CAliM1543C::~CAliM1543C()
{
}

u64 CAliM1543C::ReadMem(int index, u64 address, int dsize)
{
  int channel = 0;
  switch(index)
    {
    case 1:
      if ((address==0) || (address==4))
	return kb_read(address);
      else if (address==1)
	return reg_61_read();
      else
	printf("%%ALI-W-INVPORT: Read from unknown port %02x\n",(int)(0x60+address));
    case 2:
      return toy_read(address);
    case 3:
      return endian_bits(isa_config_read(address, dsize), dsize);
    case 6:
      return pit_read(address);
    case 8:
      channel = 1;
    case 7:
      return pic_read(channel, address);
    case 20:
      return pic_read_vector();
    case 9:
      return endian_bits(ide_config_read(address, dsize), dsize);
    case 11:
      return endian_bits(usb_config_read(address, dsize), dsize);
    case 13:
      channel = 1;
      address >>= 1;
    case 12:
      return dma_read(channel, address);
    case 15:
      channel = 1;
    case 14:
      return ide_command_read(channel,address);
    case 17:
      channel = 1;
    case 16:
      return ide_control_read(channel,address);
    case 19:
      channel = 1;
    case 18:
      return ide_busmaster_read(channel,address);
    }

  return 0;
}

void CAliM1543C::WriteMem(int index, u64 address, int dsize, u64 data)
{
  int channel = 0;
  switch(index)
    {
    case 1:
      if ((address==0) || (address==4))
	kb_write(address, (u8) data);
      else if (address==1)
	reg_61_write((u8)data);
      else
	printf("%%ALI-W-INVPORT: Write to unknown port %02x\n",(int)(0x60+address));
      return;
    case 2:
      toy_write(address, (u8)data);
      return;
    case 3:
      isa_config_write(address, dsize, endian_bits(data, dsize));
      return;
    case 6:
      pit_write(address, (u8) data);
      return;
    case 8:
      channel = 1;
    case 7:
      pic_write(channel, address, (u8) data);
      return;
    case 9:
      ide_config_write(address, dsize, endian_bits(data, dsize));
      return;
    case 11:
      usb_config_write(address, dsize, endian_bits(data, dsize));
      return;
    case 13:
      channel = 1;
      address >>= 1;
    case 12:
      dma_write(channel, address, (u8) data);
      return;
    case 15:
      channel = 1;
    case 14:
      ide_command_write(channel,address, data);
      return;
    case 17:
      channel = 1;
    case 16:
      ide_control_write(channel,address, data);
      return;
    case 19:
      channel = 1;
    case 18:
      ide_busmaster_write(channel,address, data);
      return;
    }
}

u8 CAliM1543C::kb_read(u64 address)
{
  u8 data;

  switch (address)
    {
    case 0:
      state.kb_Status &= ~1;
      data = state.kb_Output;
      break;
    case 4:
      data = state.kb_Status;
      break;
    default:
      data = 0;
      break;
    }
  
  TRC_DEV3("%%ALI-I-KBDREAD: %02" LL"x read from Keyboard port %02x\n",data,address+0x60);
  return data;
}

void CAliM1543C::kb_write(u64 address, u8 data)
{
  TRC_DEV3("%%ALI-I-KBDWRITE: %02" LL "x written to Keyboard port %02x\n",data,address+0x60);
  switch (address)
    {
      //    case 0:
      //        state.kb_Status &= ~8;
      //        state.kb_Input = (u8) data;
      //        return;
    case 4:
      state.kb_Status |= 8;
      state.kb_Command = (u8) data;
      switch (state.kb_Command)
        {
        case 0xAA:
	  state.kb_Output = 0x55;
	  state.kb_Status  |= 0x05; // data ready; initialized
	  return;
        case 0xAB:
	  state.kb_Output = 0x01;
	  state.kb_Status |= 0x01;
	  return;
        default:
	      TRC_DEV2("%%ALI-W-UNKCMD: Unknown keyboard command: %02x\n", state.kb_Command);
	  return;
        }
    }

}

u8 CAliM1543C::reg_61_read()
{
  return state.reg_61;
}

void CAliM1543C::reg_61_write(u8 data)
{
  state.reg_61 = (state.reg_61 & 0xf0) | (((u8)data) & 0x0f);
}

u8 CAliM1543C::toy_read(u64 address)
{
  TRC_DEV3("%%ALI-I-READTOY: read port %02x: 0x%02x\n", (u32)(0x70 + address), state.toy_access_ports[address]);

  return (u8)state.toy_access_ports[address];
}

void CAliM1543C::toy_write(u64 address, u8 data)
{
  time_t ltime;
  struct tm stime;

  TRC_DEV3("%%ALI-I-WRITETOY: write port %02x: 0x%02x\n", (u32)(0x70 + address), data);

  state.toy_access_ports[address] = (u8)data;

  switch (address)
    {
    case 0:
      if ((data&0x7f)<14)
        {
	  state.toy_stored_data[0x0d] = 0x80; // data is geldig!
	  // update clock.......
	  time (&ltime);
	  gmtime_s(&stime,&ltime);
	  if (state.toy_stored_data[0x0b] & 4)
            {
	      state.toy_stored_data[0] = (u8)(stime.tm_sec);
	      state.toy_stored_data[2] = (u8)(stime.tm_min);
	      if (state.toy_stored_data[0x0b] & 2)
                {
		  // 24-hour
		  state.toy_stored_data[4] = (u8)(stime.tm_hour);
                }
	      else
                {
		  // 12-hour
		  state.toy_stored_data[4] = (u8)(((stime.tm_hour/12)?0x80:0) | (stime.tm_hour%12));
                }
	      state.toy_stored_data[6] = (u8)(stime.tm_wday + 1);
	      state.toy_stored_data[7] = (u8)(stime.tm_mday);
	      state.toy_stored_data[8] = (u8)(stime.tm_mon + 1);
	      state.toy_stored_data[9] = (u8)(stime.tm_year % 100);

            }
	  else
            {
	      // BCD
	      state.toy_stored_data[0] = (u8)(((stime.tm_sec/10)<<4) | (stime.tm_sec%10));
	      state.toy_stored_data[2] = (u8)(((stime.tm_min/10)<<4) | (stime.tm_min%10));
	      if (state.toy_stored_data[0x0b] & 2)
                {
		  // 24-hour
		  state.toy_stored_data[4] = (u8)(((stime.tm_hour/10)<<4) | (stime.tm_hour%10));
                }
	      else
                {
		  // 12-hour
		  state.toy_stored_data[4] = (u8)(((stime.tm_hour/12)?0x80:0) | (((stime.tm_hour%12)/10)<<4) | ((stime.tm_hour%12)%10));
                }
	      state.toy_stored_data[6] = (u8)(stime.tm_wday + 1);
	      state.toy_stored_data[7] = (u8)(((stime.tm_mday/10)<<4) | (stime.tm_mday%10));
	      state.toy_stored_data[8] = (u8)((((stime.tm_mon+1)/10)<<4) | ((stime.tm_mon+1)%10));
	      state.toy_stored_data[9] = (u8)((((stime.tm_year%100)/10)<<4) | ((stime.tm_year%100)%10));
            }
	}
      /* bdw:  I'm getting a 0x17 as data, which should copy some data 
	 to port 0x71.  However, there's nothing there.  Problem? */
      state.toy_access_ports[1] = state.toy_stored_data[data & 0x7f];
      break;
    case 1:
      state.toy_stored_data[state.toy_access_ports[0] & 0x7f] = (u8)data;
      break;
    case 2:
      state.toy_access_ports[3] = state.toy_stored_data[0x80 + (data & 0x7f)];
      break;
    case 3:
      state.toy_stored_data[0x80 + (state.toy_access_ports[2] & 0x7f)] = (u8)data;
      break;
    }
}

/**
 * Read from the ISA interfaces PCI configuration space.
 **/

u64 CAliM1543C::isa_config_read(u64 address, int dsize)
{
    
  u64 data;
  void * x;

  x = &(state.isa_config_data[address]);

  switch (dsize)
    {
    case 8:
      data = (u64)(*((u8*)x))&0xff;
      break;
    case 16:
      data = (u64)(*((u16*)x))&0xffff;
      break;
    case 32:
      data = (u64)(*((u32*)x))&0xffffffff;
      break;
    default:
      data = (u64)(*((u64*)x));
      break;
    }
  return data;
}

/**
 * Write to the ISA interfaces PCI configuration space.
 **/

void CAliM1543C::isa_config_write(u64 address, int dsize, u64 data)
{
  void * x;
  void * y;

  x = &(state.isa_config_data[address]);
  y = &(state.isa_config_mask[address]);

  switch (dsize)
    {
    case 8:
      *((u8*)x) = (*((u8*)x) & ~*((u8*)y)) | (((u8)data) & *((u8*)y));
      break;
    case 16:
      *((u16*)x) = (*((u16*)x) & ~*((u16*)y)) | (((u16)data) & *((u16*)y));
      break;
    case 32:
      *((u32*)x) = (*((u32*)x) & ~*((u32*)y)) | (((u32)data) & *((u32*)y));
      break;
    case 64:
      *((u64*)x) = (*((u64*)x) & ~*((u64*)y)) | (((u64)data) & *((u64*)y));
      break;
    }
}

u8 CAliM1543C::pit_read(u64 address)
{
  u8 data;
  data = 0;
  return data;
}

void CAliM1543C::pit_write(u64 address, u8 data)
{

  state.pit_enable = true;
}

int CAliM1543C::DoClock()
{
  cSystem->interrupt(-1, true);
  return 0;
}

#define PIC_STD 0
#define PIC_INIT_0 1
#define PIC_INIT_1 2
#define PIC_INIT_2 3

u8 CAliM1543C::pic_read(int index, u64 address)
{
  u8 data;

  data = 0;

  if (address == 1) 
    data = state.pic_mask[index];

#ifdef DEBUG_PIC
  if (pic_messages) printf("%%PIC-I-READ: read %02x from port %" LL "d on PIC %d\n",data,address,index);
#endif

  return data;
}

u8 CAliM1543C::pic_read_vector()
{
  if (state.pic_asserted[0] & 1)
    return state.pic_intvec[0];
  if (state.pic_asserted[0] & 2)
    return state.pic_intvec[0]+1;
  if (state.pic_asserted[0] & 4)
  {
    if (state.pic_asserted[1] & 1)
      return state.pic_intvec[1];
    if (state.pic_asserted[1] & 2)
      return state.pic_intvec[1]+1;
    if (state.pic_asserted[1] & 4)
      return state.pic_intvec[1]+2;
    if (state.pic_asserted[1] & 8)
      return state.pic_intvec[1]+3;
    if (state.pic_asserted[1] & 16)
      return state.pic_intvec[1]+4;
    if (state.pic_asserted[1] & 32)
      return state.pic_intvec[1]+5;
    if (state.pic_asserted[1] & 64)
      return state.pic_intvec[1]+6;
    if (state.pic_asserted[1] & 128)
      return state.pic_intvec[1]+7;
  }
  if (state.pic_asserted[0] & 8)
    return state.pic_intvec[0]+3;
  if (state.pic_asserted[0] & 16)
    return state.pic_intvec[0]+4;
  if (state.pic_asserted[0] & 32)
    return state.pic_intvec[0]+5;
  if (state.pic_asserted[0] & 64)
    return state.pic_intvec[0]+6;
  if (state.pic_asserted[0] & 128)
    return state.pic_intvec[0]+7;
  return 0;
}

void CAliM1543C::pic_write(int index, u64 address, u8 data)
{
  int level;
  int op;
#ifdef DEBUG_PIC
  if (pic_messages) printf("%%PIC-I-WRITE: write %02x to port %" LL "d on PIC %d\n",data,address,index);
#endif

  switch(address)
    {
    case 0:
      if (data & 0x10)
	state.pic_mode[index] = PIC_INIT_0;
      else
	state.pic_mode[index] = PIC_STD;
      if (data & 0x08)
	{
	  // OCW3
	}
      else
	{
	  // OCW2
	  op = (data>>5) & 7;
	  level = data & 7;
	  switch(op)
	    {
	    case 1:
	      //non-specific EOI
	      state.pic_asserted[index] = 0;
	      //
	      if (index==1)
	        state.pic_asserted[0] &= ~(1<<2);
	      //
	      if (!state.pic_asserted[0])
		cSystem->interrupt(55,false);
#ifdef DEBUG_PIC
	      pic_messages = false;
#endif
	      break;
	    case 3:
	      // specific EOI
	      state.pic_asserted[index] &= ~(1<<level);
	      //
	      if ((index==1) && (!state.pic_asserted[1]))
	        state.pic_asserted[0] &= ~(1<<2);
	      //
	      if (!state.pic_asserted[0])
		cSystem->interrupt(55,false);
#ifdef DEBUG_PIC
	      pic_messages = false;
#endif
	      break;				
	    }
	}
      return;
    case 1:
      switch(state.pic_mode[index])
	{
	case PIC_INIT_0:
	  state.pic_intvec[index] = (u8)data & 0xf8;
	  state.pic_mode[index] = PIC_INIT_1;
	  return;
	case PIC_INIT_1:
	  state.pic_mode[index] = PIC_INIT_2;
	  return;
	case PIC_INIT_2:
	  state.pic_mode[index] = PIC_STD;
	  return;
	case PIC_STD:
	  state.pic_mask[index] = data;
	  state.pic_asserted[index] &= ~data;
	  return;
	}
    }
}

void CAliM1543C::pic_interrupt(int index, int intno)
{
#ifdef DEBUG_PIC
  if (index!=0 || intno <3 || intno >4) 
  {
    printf("%%PIC-I-INCOMING: Interrupt %d incomming on PIC %d",intno,index);
    pic_messages = true;
  }
#endif

  // do we have this interrupt enabled?
  if (state.pic_mask[index] & (1<<intno))
  {
#ifdef DEBUG_PIC
  if (index!=0 || intno <3 || intno >4)     printf(" (masked)\n");
  pic_messages = false;
#endif
    return;
  }

  if (state.pic_asserted[index] & (1<<intno))
  {
#ifdef DEBUG_PIC
  if (index!=0 || intno <3 || intno >4)     printf(" (already asserted)\n");
#endif
    return;
  }

#ifdef DEBUG_PIC
  if (index!=0 || intno <3 || intno >4)   printf("\n");
#endif

  state.pic_asserted[index] |= (1<<intno);
	
  if (index==1)
    pic_interrupt(0,2);	// cascade

  if (index==0)
    cSystem->interrupt(55,true);
}

/**
 * Read from the IDE controllers PCI configuration space.
 **/

u64 CAliM1543C::ide_config_read(u64 address, int dsize)
{
    
  u64 data;
  void * x;

  x = &(state.ide_config_data[address]);

  switch (dsize)
    {
    case 8:
      data = (u64)(*((u8*)x))&0xff;
      break;
    case 16:
      data = (u64)(*((u16*)x))&0xffff;
      break;
    case 32:
      data = (u64)(*((u32*)x))&0xffffffff;
      break;
    default:
      data = (u64)(*((u64*)x));
      break;
    }
  return data;

}

/**
 * Write to the IDE controllers PCI configuration space.
 **/

void CAliM1543C::ide_config_write(u64 address, int dsize, u64 data)
{

  void * x;
  void * y;

  x = &(state.ide_config_data[address]);
  y = &(state.ide_config_mask[address]);

  switch (dsize)
    {
    case 8:
      *((u8*)x) = (*((u8*)x)  & ~*((u8*)y) ) 
	| (  ((u8)data) &  *((u8*)y) );
      break;
    case 16:
      *((u16*)x) = (*((u16*)x) & ~*((u16*)y)) | (((u16)data) & *((u16*)y));
      break;
    case 32:
      *((u32*)x) = (*((u32*)x) & ~*((u32*)y)) | ((u32)data & *((u32*)y));
      break;
    case 64:
      *((u64*)x) = (*((u64*)x) & ~*((u64*)y)) | ((u64)data & *((u64*)y));
      break;
    }
  if (   ((data&0xffffffff)!=0xffffffff) 
	 && ((data&0xffffffff)!=0x00000000) 
	 && ( dsize           ==32        ))
    switch(address)
      {
      case 0x10:
	// command
	cSystem->RegisterMemory(this,14, X64(00000801fc000000) + (endian_32(data)&0x00fffffe), 8);
	return;
      case 0x14:
	// control
	cSystem->RegisterMemory(this,16, X64(00000801fc000000) + (endian_32(data)&0x00fffffe), 4);
	return;
      case 0x18:
	// command
	cSystem->RegisterMemory(this,15, X64(00000801fc000000) + (endian_32(data)&0x00fffffe), 8);
	return;
      case 0x1c:
	// control
	cSystem->RegisterMemory(this,17, X64(00000801fc000000) + (endian_32(data)&0x00fffffe), 4);
	return;
      case 0x20:
	// bus master control
	cSystem->RegisterMemory(this,18, X64(00000801fc000000) + (endian_32(data)&0x00fffffe), 8);
	// bus master control
	cSystem->RegisterMemory(this,19, X64(00000801fc000000) + (endian_32(data)&0x00fffffe) + 8, 8);
	return;
      }
}

u64 CAliM1543C::ide_command_read(int index, u64 address)
{
  u64 data;

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
	      if (!(state.ide_command[index][6]&2))
		pic_interrupt(1,6);
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

void CAliM1543C::ide_command_write(int index, u64 address, u64 data)
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
	  if (!(state.ide_command[index][6]&2))
            pic_interrupt(1,6);
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
#ifdef DEBUG_IDE
	      printf("%%IDE-I-IDENTIFY: Identify IDE disk\n");
#endif
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

	      if (!(state.ide_command[index][6]&2))
		pic_interrupt(1,6);

	      break;
	    case 0x20: // read sector
	      lba =      *((int*)(&(state.ide_command[index][3]))) & 0x0fffffff;
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
	      if (!(state.ide_command[index][6]&2))
		pic_interrupt(1,6);
	      break;
	    case 0x30:
	      if (!ide_info[index][state.ide_selected[index]].mode)
	      {
	        printf("%%IDE-W-WRITPROT: Attempt to write to write-protected disk.\n");
		state.ide_status[index] = 0x41;
		state.ide_error[index] = 0x04;
	      }
	      else
	      {
	        lba =      *((int*)(&(state.ide_command[index][3]))) & 0x0fffffff;
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
#ifdef DEBUG_IDE
	      printf("%%IDE-I-SETTRANS: Set IDE translation\n");
#endif
	      state.ide_status[index] = 0x40;
	      if (!(state.ide_command[index][6]&2))
		pic_interrupt(1,6);
	      break;
        default:
	      state.ide_status[index] = 0x41;	// ERROR
	      state.ide_error[index] = 0x20;	// ABORTED

#ifdef DEBUG_IDE
	      printf("%%IDE-I-UNKCMND : Unknown IDE Command: ");
	      for (x=0;x<8;x++) printf("%02x ",state.ide_command[index][x]);
	      printf("\n");
#endif
	      if (!(state.ide_command[index][6]&2))
		pic_interrupt(1,6);
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
 **/

u64 CAliM1543C::ide_control_read(int index, u64 address)
{
  u64 data;

  data = 0;
  if (address==2) 
  {
      //
      // HACK FOR STRANGE ERROR WHEN SAVING/LOADING STATE
      //
      if (state.ide_status[index]==0xb9)
        state.ide_status[index] = 0x40;
      //
      //
    data = state.ide_status[index];
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

void CAliM1543C::ide_control_write(int index, u64 address, u64 data)
{
  TRC_DEV4("%%IDE-I-WRITCRTL: write port %d on IDE control %d: 0x%02x\n",  (u32)(address),index, data);
#ifdef DEBUG_IDE
  printf("%%IDE-I-WRITCTRL: write port %d on IDE control %d: 0x%02x\n",  (u32)(address),index, data);
#endif
}

/**
 * Read from the IDE controller busmaster interface.
 * Always returns 0.
 **/

u64 CAliM1543C::ide_busmaster_read(int index, u64 address)
{
  u64 data;

  data = 0;

  TRC_DEV4("%%IDE-I-READBUSM: read port %d on IDE bus master %d: 0x%02x\n", (u32)(address), index, data);
#ifdef DEBUG_IDE
  printf("%%IDE-I-READBUSM: read port %d on IDE bus master %d: 0x%02x\n", (u32)(address), index, data);
#endif
  return data;
}

/**
 * Write to the IDE controller busmaster interface.
 * Not functional.
 **/

void CAliM1543C::ide_busmaster_write(int index, u64 address, u64 data)
{
  TRC_DEV4("%%IDE-I-WRITBUSM: write port %d on IDE bus master %d: 0x%02x\n",  (u32)(address),index, data);
#ifdef DEBUG_IDE
  printf("%%IDE-I-WRITBUSM: write port %d on IDE bus master %d: 0x%02x\n",  (u32)(address),index, data);
#endif

}

/**
 * Read from the USB controllers PCI configuration space.
 **/

u64 CAliM1543C::usb_config_read(u64 address, int dsize)
{
    
  u64 data;
  void * x;

  x = &(state.usb_config_data[address]);

  switch (dsize)
    {
    case 8:
      data = (u64)(*((u8*)x))&0xff;
      break;
    case 16:
      data = (u64)(*((u16*)x))&0xffff;
      break;
    case 32:
      data = (u64)(*((u32*)x))&0xffffffff;
      break;
    default:
      data = (u64)(*((u64*)x));
      break;
    }
  return data;

}

/**
 * Write to the USB controllers PCI configuration space.
 **/

void CAliM1543C::usb_config_write(u64 address, int dsize, u64 data)
{
  void * x;
  void * y;

  x = &(state.usb_config_data[address]);
  y = &(state.usb_config_mask[address]);

  switch (dsize)
    {
    case 8:
      *((u8*)x) = (*((u8*)x) & ~*((u8*)y)) | ((u8)data & *((u8*)y));
      break;
    case 16:
      *((u16*)x) = (*((u16*)x) & ~*((u16*)y)) | ((u16)data & *((u16*)y));
      break;
    case 32:
      *((u32*)x) = (*((u32*)x) & ~*((u32*)y)) | ((u32)data & *((u32*)y));
      break;
    case 64:
      *((u64*)x) = (*((u64*)x) & ~*((u64*)y)) | ((u64)data & *((u64*)y));
      break;
    }
}

/**
 * Read a byte from the dma controller.
 * Always returns 0.
 **/

u8 CAliM1543C::dma_read(int index, u64 address)
{
  u8 data;

  data = 0;

  return data;
}

/**
 * Write a byte to the dma controller.
 * Not functional.
 **/

void CAliM1543C::dma_write(int index, u64 address, u8 data)
{
}

/**
 * Make sure a clock interrupt is generated on the next clock.
 * used for debugging, or to speed tings up when software is waiting for a clock tick.
 **/

void CAliM1543C::instant_tick()
{
  DoClock();
}

/**
 * Save state to a Virtual Machine State file.
 **/

void CAliM1543C::SaveState(FILE *f)
{
  fwrite(&state,sizeof(state),1,f);
}

/**
 * Restore state from a Virtual Machine State file.
 **/

void CAliM1543C::RestoreState(FILE *f)
{
  fread(&state,sizeof(state),1,f);

  // allocations 
  ide_config_write(0x10,32,(*((u32*)(&state.ide_config_data[0x10])))&~1);
  ide_config_write(0x14,32,(*((u32*)(&state.ide_config_data[0x14])))&~1);
  ide_config_write(0x18,32,(*((u32*)(&state.ide_config_data[0x18])))&~1);
  ide_config_write(0x1c,32,(*((u32*)(&state.ide_config_data[0x1c])))&~1);
  ide_config_write(0x20,32,(*((u32*)(&state.ide_config_data[0x20])))&~1);
}
