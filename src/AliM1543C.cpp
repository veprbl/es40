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
  kb_intState = 0;
  kb_Status = 0;
  kb_Output = 0;
  kb_Command = 0;
  kb_Input = 0;
  reg_61 = 0;

  c->RegisterMemory(this, 2, X64(00000801fc000070), 4);
  for (i=0;i<4;i++)
    toy_access_ports[i] = 0;
  for (i=0;i<256;i++)
    toy_stored_data[i] = 0;
  //    toy_stored_data[0x17] = 1;

  c->RegisterMemory(this, 3, X64(00000801fe003800),0x100);

  for (i=0;i<256;i++) 
    {
      isa_config_data[i] = 0;
      isa_config_mask[i] = 0;
    }
  isa_config_data[0x00] = 0xb9;
  isa_config_data[0x01] = 0x10;
  isa_config_data[0x02] = 0x33;
  isa_config_data[0x03] = 0x15;
  isa_config_data[0x04] = 0x0f;
  isa_config_data[0x07] = 0x02;
  isa_config_data[0x08] = 0xc3;
  isa_config_data[0x0a] = 0x01; 
  isa_config_data[0x0b] = 0x06;
  isa_config_data[0x55] = 0x02;
  isa_config_mask[0x40] = 0x7f;
  isa_config_mask[0x41] = 0xff;
  isa_config_mask[0x42] = 0xcf;
  isa_config_mask[0x43] = 0xff;
  isa_config_mask[0x44] = 0xdf;
  isa_config_mask[0x45] = 0xcb;
  isa_config_mask[0x47] = 0xff;
  isa_config_mask[0x48] = 0xff;
  isa_config_mask[0x49] = 0xff;
  isa_config_mask[0x4a] = 0xff;
  isa_config_mask[0x4b] = 0xff;
  isa_config_mask[0x4c] = 0xff;
  isa_config_mask[0x50] = 0xff;
  isa_config_mask[0x51] = 0x8f;
  isa_config_mask[0x52] = 0xff;
  isa_config_mask[0x53] = 0xff;
  isa_config_mask[0x55] = 0xff;
  isa_config_mask[0x56] = 0xff;
  isa_config_mask[0x57] = 0xf0;
  isa_config_mask[0x58] = 0x7f;
  isa_config_mask[0x59] = 0x0d;
  isa_config_mask[0x5a] = 0x0f;
  isa_config_mask[0x5b] = 0x03;
  // ...
									
  c->RegisterMemory(this, 6, X64(00000801fc000040), 4);
  c->RegisterClock(this, true);
  pit_enable = false;

  c->RegisterMemory(this, 7, X64(00000801fc000020), 2);
  c->RegisterMemory(this, 8, X64(00000801fc0000a0), 2);
  c->RegisterMemory(this, 20, X64(00000801f8000000), 1);
  for(i=0;i<2;i++)
    {
      pic_mode[i] = 0;
      pic_intvec[i] = 0;
      pic_mask[i] = 0;
      pic_asserted[i] = 0;
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
      ide_config_data[i] = 0;
      ide_config_mask[i] = 0;
    }
  ide_config_data[0x00] = 0xb9;	// vendor
  ide_config_data[0x01] = 0x10;
  ide_config_data[0x02] = 0x29;	// device
  ide_config_data[0x03] = 0x52;
  ide_config_data[0x06] = 0x80;	// status
  ide_config_data[0x07] = 0x02;	
  ide_config_data[0x08] = 0x1c;	// revision
  ide_config_data[0x09] = 0xFA;	// class code	
  ide_config_data[0x0a] = 0x01;	
  ide_config_data[0x0b] = 0x01;

  ide_config_data[0x10] = 0xF1;	// address I	
  ide_config_data[0x11] = 0x01;
  ide_config_data[0x14] = 0xF5;	// address II	
  ide_config_data[0x15] = 0x03;
  ide_config_data[0x18] = 0x71;	// address III	
  ide_config_data[0x19] = 0x01;
  ide_config_data[0x1c] = 0x75;	// address IV	
  ide_config_data[0x1d] = 0x03;
  ide_config_data[0x20] = 0x01;	// address V	
  ide_config_data[0x21] = 0xF0;

  ide_config_data[0x3d] = 0x01;	// interrupt pin	
  ide_config_data[0x3e] = 0x02;	// min_gnt
  ide_config_data[0x3f] = 0x04;	// max_lat	
  ide_config_data[0x4b] = 0x4a;	// udma test
  ide_config_data[0x4e] = 0xba;	// reserved	
  ide_config_data[0x4f] = 0x1a;
  ide_config_data[0x54] = 0x55;	// fifo treshold ch 1	
  ide_config_data[0x55] = 0x55;	// fifo treshold ch 2
  ide_config_data[0x56] = 0x44;	// udma setting ch 1	
  ide_config_data[0x57] = 0x44;	// udma setting ch 2
  ide_config_data[0x78] = 0x21;	// ide clock	

  //

  ide_config_mask[0x04] = 0x45;	// command

  ide_config_mask[0x0d] = 0xff;	// latency timer

  ide_config_mask[0x10] = 0xfc;	// address I
  ide_config_mask[0x11] = 0xff;	
  ide_config_mask[0x12] = 0xff;	
  ide_config_mask[0x13] = 0xff;	
  ide_config_mask[0x14] = 0xfc;	// address II
  ide_config_mask[0x15] = 0xff;	
  ide_config_mask[0x16] = 0xff;	
  ide_config_mask[0x17] = 0xff;	
  ide_config_mask[0x18] = 0xfc;	// address III
  ide_config_mask[0x19] = 0xff;	
  ide_config_mask[0x1a] = 0xff;	
  ide_config_mask[0x1b] = 0xff;	
  ide_config_mask[0x1c] = 0xfc;	// address IV
  ide_config_mask[0x1d] = 0xff;	
  ide_config_mask[0x1e] = 0xff;	
  ide_config_mask[0x1f] = 0xff;	
  ide_config_mask[0x20] = 0xfc;	// address V
  ide_config_mask[0x21] = 0xff;	
  ide_config_mask[0x22] = 0xff;	
  ide_config_mask[0x23] = 0xff;	

  ide_config_mask[0x3c] = 0xff;	// interrupt
  ide_config_mask[0x11] = 0xff;	
  ide_config_mask[0x12] = 0xff;	
  ide_config_mask[0x13] = 0xff;	

  ide_status[0] = 0;
  ide_reading[0] = false;
  ide_writing[0] = false;
  ide_sectors[0] = 0;
  ide_selected[0] = 0;
  ide_error[0] = 0;

  ide_status[1] = 0;
  ide_reading[1] = false;
  ide_writing[1] = false;
  ide_sectors[1] = 0;
  ide_selected[1] = 0;
  ide_error[1] = 0;

  c->RegisterMemory(this, 11, X64(00000801fe009800), 0x100);
  for (i=0;i<256;i++)
    {
      usb_config_data[i] = 0;
      usb_config_mask[i] = 0;
    }
  usb_config_data[0x00] = 0xb9;
  usb_config_data[0x01] = 0x10;
  usb_config_data[0x02] = 0x37;
  usb_config_data[0x03] = 0x52;
  usb_config_mask[0x04] = 0x13;
  usb_config_data[0x05] = 0x02;	usb_config_mask[0x05] = 0x01;
  usb_config_data[0x06] = 0x80;
  usb_config_data[0x07] = 0x02;
  usb_config_data[0x09] = 0x10;
  usb_config_data[0x0a] = 0x03;
  usb_config_data[0x0b] = 0x0c;
  usb_config_mask[0x0c] = 0x08;
  usb_config_mask[0x0d] = 0xff;
  usb_config_mask[0x11] = 0xf0;
  usb_config_mask[0x12] = 0xff;
  usb_config_mask[0x13] = 0xff;
  usb_config_mask[0x3c] = 0xff;
  usb_config_data[0x3d] = 0x01;	usb_config_mask[0x3d] = 0xff;
  usb_config_mask[0x3e] = 0xff;
  usb_config_mask[0x3f] = 0xff;


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
      kb_Status &= ~1;
      data = kb_Output;
      break;
    case 4:
      data = kb_Status;
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
      //        kb_Status &= ~8;
      //        kb_Input = (u8) data;
      //        return;
    case 4:
      kb_Status |= 8;
      kb_Command = (u8) data;
      switch (kb_Command)
        {
        case 0xAA:
	  kb_Output = 0x55;
	  kb_Status  |= 0x05; // data ready; initialized
	  return;
        case 0xAB:
	  kb_Output = 0x01;
	  kb_Status |= 0x01;
	  return;
        default:
	      TRC_DEV2("%%ALI-W-UNKCMD: Unknown keyboard command: %02x\n", kb_Command);
	  return;
        }
    }

}

u8 CAliM1543C::reg_61_read()
{
  return reg_61;
}

void CAliM1543C::reg_61_write(u8 data)
{
  reg_61 = (reg_61 & 0xf0) | (((u8)data) & 0x0f);
}

u8 CAliM1543C::toy_read(u64 address)
{
  TRC_DEV3("%%ALI-I-READTOY: read port %02x: 0x%02x\n", (u32)(0x70 + address), toy_access_ports[address]);

  return (u8)toy_access_ports[address];
}

void CAliM1543C::toy_write(u64 address, u8 data)
{
  time_t ltime;
  struct tm stime;

  TRC_DEV3("%%ALI-I-WRITETOY: write port %02x: 0x%02x\n", (u32)(0x70 + address), data);

  toy_access_ports[address] = (u8)data;

  switch (address)
    {
    case 0:
      if ((data&0x7f)<14)
        {
	  toy_stored_data[0x0d] = 0x80; // data is geldig!
	  // update clock.......
	  time (&ltime);
	  gmtime_s(&stime,&ltime);
	  if (toy_stored_data[0x0b] & 4)
            {
	      toy_stored_data[0] = (u8)(stime.tm_sec);
	      toy_stored_data[2] = (u8)(stime.tm_min);
	      if (toy_stored_data[0x0b] & 2)
                {
		  // 24-hour
		  toy_stored_data[4] = (u8)(stime.tm_hour);
                }
	      else
                {
		  // 12-hour
		  toy_stored_data[4] = (u8)(((stime.tm_hour/12)?0x80:0) | (stime.tm_hour%12));
                }
	      toy_stored_data[6] = (u8)(stime.tm_wday + 1);
	      toy_stored_data[7] = (u8)(stime.tm_mday);
	      toy_stored_data[8] = (u8)(stime.tm_mon + 1);
	      toy_stored_data[9] = (u8)(stime.tm_year % 100);

            }
	  else
            {
	      // BCD
	      toy_stored_data[0] = (u8)(((stime.tm_sec/10)<<4) | (stime.tm_sec%10));
	      toy_stored_data[2] = (u8)(((stime.tm_min/10)<<4) | (stime.tm_min%10));
	      if (toy_stored_data[0x0b] & 2)
                {
		  // 24-hour
		  toy_stored_data[4] = (u8)(((stime.tm_hour/10)<<4) | (stime.tm_hour%10));
                }
	      else
                {
		  // 12-hour
		  toy_stored_data[4] = (u8)(((stime.tm_hour/12)?0x80:0) | (((stime.tm_hour%12)/10)<<4) | ((stime.tm_hour%12)%10));
                }
	      toy_stored_data[6] = (u8)(stime.tm_wday + 1);
	      toy_stored_data[7] = (u8)(((stime.tm_mday/10)<<4) | (stime.tm_mday%10));
	      toy_stored_data[8] = (u8)((((stime.tm_mon+1)/10)<<4) | ((stime.tm_mon+1)%10));
	      toy_stored_data[9] = (u8)((((stime.tm_year%100)/10)<<4) | ((stime.tm_year%100)%10));
            }
	}
      /* bdw:  I'm getting a 0x17 as data, which should copy some data 
	 to port 0x71.  However, there's nothing there.  Problem? */
      toy_access_ports[1] = toy_stored_data[data & 0x7f];
      break;
    case 1:
      toy_stored_data[toy_access_ports[0] & 0x7f] = (u8)data;
      break;
    case 2:
      toy_access_ports[3] = toy_stored_data[0x80 + (data & 0x7f)];
      break;
    case 3:
      toy_stored_data[0x80 + (toy_access_ports[2] & 0x7f)] = (u8)data;
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

  x = &(isa_config_data[address]);

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

  x = &(isa_config_data[address]);
  y = &(isa_config_mask[address]);

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

  pit_enable = true;
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
    data = pic_mask[index];

#ifdef DEBUG_PIC
  if (pic_messages) printf("%%PIC-I-READ: read %02x from port %" LL "d on PIC %d\n",data,address,index);
#endif

  return data;
}

u8 CAliM1543C::pic_read_vector()
{
  if (pic_asserted[0] & 1)
    return pic_intvec[0];
  if (pic_asserted[0] & 2)
    return pic_intvec[0]+1;
  if (pic_asserted[0] & 4)
  {
    if (pic_asserted[1] & 1)
      return pic_intvec[1];
    if (pic_asserted[1] & 2)
      return pic_intvec[1]+1;
    if (pic_asserted[1] & 4)
      return pic_intvec[1]+2;
    if (pic_asserted[1] & 8)
      return pic_intvec[1]+3;
    if (pic_asserted[1] & 16)
      return pic_intvec[1]+4;
    if (pic_asserted[1] & 32)
      return pic_intvec[1]+5;
    if (pic_asserted[1] & 64)
      return pic_intvec[1]+6;
    if (pic_asserted[1] & 128)
      return pic_intvec[1]+7;
  }
  if (pic_asserted[0] & 8)
    return pic_intvec[0]+3;
  if (pic_asserted[0] & 16)
    return pic_intvec[0]+4;
  if (pic_asserted[0] & 32)
    return pic_intvec[0]+5;
  if (pic_asserted[0] & 64)
    return pic_intvec[0]+6;
  if (pic_asserted[0] & 128)
    return pic_intvec[0]+7;
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
	pic_mode[index] = PIC_INIT_0;
      else
	pic_mode[index] = PIC_STD;
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
	      pic_asserted[index] = 0;
	      //
	      if (index==1)
	        pic_asserted[0] &= ~(1<<2);
	      //
	      if (!pic_asserted[0])
		cSystem->interrupt(55,false);
#ifdef DEBUG_PIC
	      pic_messages = false;
#endif
	      break;
	    case 3:
	      // specific EOI
	      pic_asserted[index] &= ~(1<<level);
	      //
	      if ((index==1) && (!pic_asserted[1]))
	        pic_asserted[0] &= ~(1<<2);
	      //
	      if (!pic_asserted[0])
		cSystem->interrupt(55,false);
#ifdef DEBUG_PIC
	      pic_messages = false;
#endif
	      break;				
	    }
	}
      return;
    case 1:
      switch(pic_mode[index])
	{
	case PIC_INIT_0:
	  pic_intvec[index] = (u8)data & 0xf8;
	  pic_mode[index] = PIC_INIT_1;
	  return;
	case PIC_INIT_1:
	  pic_mode[index] = PIC_INIT_2;
	  return;
	case PIC_INIT_2:
	  pic_mode[index] = PIC_STD;
	  return;
	case PIC_STD:
	  pic_mask[index] = data;
	  pic_asserted[index] &= ~data;
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
  if (pic_mask[index] & (1<<intno))
  {
#ifdef DEBUG_PIC
  if (index!=0 || intno <3 || intno >4)     printf(" (masked)\n");
  pic_messages = false;
#endif
    return;
  }

  if (pic_asserted[index] & (1<<intno))
  {
#ifdef DEBUG_PIC
  if (index!=0 || intno <3 || intno >4)     printf(" (already asserted)\n");
#endif
    return;
  }

#ifdef DEBUG_PIC
  if (index!=0 || intno <3 || intno >4)   printf("\n");
#endif

  pic_asserted[index] |= (1<<intno);
	
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

  x = &(ide_config_data[address]);

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

  x = &(ide_config_data[address]);
  y = &(ide_config_mask[address]);

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

  data = ide_command[index][address];

  if (!(ide_info[index][ide_selected[index]].handle))
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
      if (ide_reading[index])
        data = endian_16(ide_data[index][ide_data_ptr[index]]);
      else
        data = ide_data[index][ide_data_ptr[index]];
//      printf("%c%c",printable((char)(data&0xff)),printable((char)((data>>8)&0xff)));
      ide_data_ptr[index]++;
      if (ide_data_ptr[index]==256)
	{
//	  printf("\n");
	  if (ide_reading[index] && ide_sectors[index])
	    {
	      fread(&(ide_data[index][0]),1,512,ide_info[index][ide_selected[index]].handle);
	      ide_sectors[index]--;
	      if (!(ide_command[index][6]&2))
		pic_interrupt(1,6);
	    }
	  else
	    {
	      ide_status[index] &= ~0x08;	// (no DRQ)
	      ide_reading[index] = false;
	    }
	  ide_data_ptr[index] = 0;
	}
      break;
    case 1:
      data = ide_error[index]; // no error
      break;
    case 7:
      //
      // HACK FOR STRANGE ERROR WHEN SAVING/LOADING STATE
      //
      if (ide_status[index]==0xb9)
        ide_status[index] = 0x40;
      //
      //
      data = ide_status[index];
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
//  if (address)
//    printf("%%ALI-I-WRITEIDECMD: write port %d on IDE command %d: 0x%02x\n",  (u32)(address),index, data);
#endif

  ide_command[index][address]=(u8)data;
	
  ide_selected[index] = (ide_command[index][6]>>4)&1;

  if (ide_info[index][ide_selected[index]].handle)
    {
      // drive is present
      ide_status[index] = 0x40;
      ide_error[index] = 0;

      if (address==0 && ide_writing[index])
      {
        ide_data[index][ide_data_ptr[index]] = endian_16((u16)data);
        ide_data_ptr[index]++;
        if (ide_data_ptr[index]==256)
	{
	  fwrite(&(ide_data[index][0]),1,512,ide_info[index][ide_selected[index]].handle);
	  ide_sectors[index]--;
	  ide_data_ptr[index] = 0;
	  if (!(ide_command[index][6]&2))
            pic_interrupt(1,6);
	}
	if (ide_sectors[index])
	  ide_status[index] = 0x48;
	else	      
	  ide_writing[index] = false;
      }
      else if (address==7)	// command
	{
	  switch (data)
	    {
	    case 0xec:	// identify drive
#ifdef DEBUG_IDE
	      printf("%%IDE-I-IDENTIFY: Identify IDE disk\n");
#endif
	      ide_data_ptr[index] = 0;
	      ide_data[index][0] = 0x0140;	// flags
	      ide_data[index][1] = 3000;	// cylinders
	      ide_data[index][2] = 0xc837;	// specific configuration (ATA-4 specs)
	      ide_data[index][3] = 14;		// heads
	      ide_data[index][4] = 25600;	// bytes per track
	      ide_data[index][5] = 512;		// bytes per sector
	      ide_data[index][6] = 50;		// sectors per track
	      ide_data[index][7] = 0;		// spec. bytes
	      ide_data[index][8] = 0;		// spec. bytes
	      ide_data[index][9] = 0;		// unique vendor status words
	      ide_data[index][10] = 0x2020;	// serial number
	      ide_data[index][11] = 0x2020;
	      ide_data[index][12] = 0x2020;
	      ide_data[index][13] = 0x2020;
	      ide_data[index][14] = 0x2020;
	      ide_data[index][15] = 0x2020;
	      ide_data[index][16] = 0x2020;
	      ide_data[index][17] = 0x2020;
	      ide_data[index][18] = 0x2020;
	      ide_data[index][19] = 0x2020;
	      ide_data[index][20] = 1;		// single ported, single buffer
	      ide_data[index][21] = 51200;	// buffer size
	      ide_data[index][22] = 0;		// ecc bytes
	      ide_data[index][23] = 0x2020;	// firmware revision
	      ide_data[index][24] = 0x2020;
	      ide_data[index][25] = 0x2020;
	      ide_data[index][26] = 0x2020;

	      // clear the name
	      for(x=27;x<47;x++) {
		ide_data[index][x]=0x2020;
	      }
	      l = strlen(ide_info[index][ide_selected[index]].filename);
	      l = (l > 40)? 40 : l;
	      memcpy((char *)&ide_data[index][27],ide_info[index][ide_selected[index]].filename,l);
     #if defined(ES40_LITTLE_ENDIAN)         
	      for(x=27;x<47;x++) {
		ide_data[index][x]=((ide_data[index][x]>>8) & 0xff) | (ide_data[index][x]<<8);
	      }
     #endif
     
              ide_data[index][47] = 1;		// read/write multiples
	      ide_data[index][48] = 0;		// double-word IO transfers supported
	      ide_data[index][49] = 0x0202;		// capability LBA
	      ide_data[index][50] = 0;
	      ide_data[index][51] = 0x101;		// cycle time
	      ide_data[index][52] = 0x101;		// cycle time
	      ide_data[index][53] = 1;			// field_valid
	      ide_data[index][54] = 3000;		// cylinders
	      ide_data[index][55] = 14;		// heads
	      ide_data[index][56] = 50;		// sectors
	      ide_data[index][57] = ide_info[index][ide_selected[index]].size & 0xFFFF;	// total_sectors
	      ide_data[index][58] = ide_info[index][ide_selected[index]].size >> 16;	// ""
	      ide_data[index][59] = 0;							// multiple sector count
	      ide_data[index][60] = ide_info[index][ide_selected[index]].size & 0xFFFF;	// LBA capacity
	      ide_data[index][61] = ide_info[index][ide_selected[index]].size >> 16;	// ""
				
	      //	unsigned int	lba_capacity;	/* total number of sectors */
	      //	unsigned short	dma_1word;	/* single-word dma info */
	      //	unsigned short	dma_mword;	/* multiple-word dma info */
	      //	unsigned short  eide_pio_modes; /* bits 0:mode3 1:mode4 */
	      //	unsigned short  eide_dma_min;	/* min mword dma cycle time (ns) */
	      //	unsigned short  eide_dma_time;	/* recommended mword dma cycle time (ns) */
	      //	unsigned short  eide_pio;       /* min cycle time (ns), no IORDY  */
	      //	unsigned short  eide_pio_iordy; /* min cycle time (ns), with IORDY */



	      ide_status[index] = 0x48;	// RDY+DRQ

	      if (!(ide_command[index][6]&2))
		pic_interrupt(1,6);

	      break;
	    case 0x20: // read sector
	      lba =      *((int*)(&(ide_command[index][3]))) & 0x0fffffff;
	      TRC_DEV5("%%IDE-I-READSECT: Read  %3d sectors @ IDE %d.%d LBA %8d\n",ide_command[index][2]?ide_command[index][2]:256,index,ide_selected[index],lba);
#ifdef DEBUG_IDE
//	      printf("%%IDE-I-READSECT: Read  %3d sectors @ IDE %d.%d LBA %8d\n",ide_command[index][2]?ide_command[index][2]:256,index,ide_selected[index],lba);
#endif
	      fseek(ide_info[index][ide_selected[index]].handle,lba*512,0);
	      fread(&(ide_data[index][0]),1,512,ide_info[index][ide_selected[index]].handle);
	      ide_data_ptr[index] = 0;
	      ide_status[index] = 0x48;
	      ide_sectors[index] = ide_command[index][2]-1;
	      if (ide_sectors[index]) ide_reading[index] = true;
	      if (!(ide_command[index][6]&2))
		pic_interrupt(1,6);
	      break;
	    case 0x30:
	      if (!ide_info[index][ide_selected[index]].mode)
	      {
	        printf("%%IDE-W-WRITPROT: Attempt to write to write-protected disk.\n");
		ide_status[index] = 0x41;
		ide_error[index] = 0x04;
	      }
	      else
	      {
	        lba =      *((int*)(&(ide_command[index][3]))) & 0x0fffffff;
	        TRC_DEV5("%%IDE-I-WRITSECT: Write %3d sectors @ IDE %d.%d @ LBA %8d\n",ide_command[index][2]?ide_command[index][2]:256,index,ide_selected[index],lba);
#ifdef DEBUG_IDE
//	        printf("%%IDE-I-WRITSECT: Write %3d sectors @ IDE %d.%d @ LBA %8d\n",ide_command[index][2]?ide_command[index][2]:256,index,ide_selected[index],lba);
#endif
	        fseek(ide_info[index][ide_selected[index]].handle,lba*512,0);
	        ide_data_ptr[index] = 0;
	        ide_status[index] = 0x48;
	        ide_sectors[index] = ide_command[index][2];
	        ide_writing[index] = true;
	      }
	      break;
	case 0x91:			// SET TRANSLATION
#ifdef DEBUG_IDE
	      printf("%%IDE-I-SETTRANS: Set IDE translation\n");
#endif
	      ide_status[index] = 0x40;
	      if (!(ide_command[index][6]&2))
		pic_interrupt(1,6);
	      break;
        default:
	      ide_status[index] = 0x41;	// ERROR
	      ide_error[index] = 0x20;	// ABORTED

#ifdef DEBUG_IDE
	      printf("%%IDE-I-UNKCMND : Unknown IDE Command: ");
	      for (x=0;x<8;x++) printf("%02x ",ide_command[index][x]);
	      printf("\n");
#endif
	      if (!(ide_command[index][6]&2))
		pic_interrupt(1,6);
	  }
	}
  }
  else
  {
#ifdef DEBUG_IDE
    if (address==7)
    {
      printf("%%IDE-I-NODRIVE : IDE Command for non-existing drive %d.%d: ",index,ide_selected[index]);
      for (x=0;x<8;x++) printf("%02x ",ide_command[index][x]);
        printf("\n");
    }
#endif
    ide_status[index] = 0;

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
      if (ide_status[index]==0xb9)
        ide_status[index] = 0x40;
      //
      //
    data = ide_status[index];
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

  x = &(usb_config_data[address]);

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

  x = &(usb_config_data[address]);
  y = &(usb_config_mask[address]);

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
  // REGISTERS 60 & 64: KEYBOARD
  fwrite(&kb_Input,1,1,f);
  fwrite(&kb_Output,1,1,f);
  fwrite(&kb_Command,1,1,f);
  fwrite(&kb_Status,1,1,f);
  fwrite(&kb_intState,1,1,f);

  // REGISTER 61 (NMI)
  fwrite(&reg_61,1,1,f);
    
  // REGISTERS 70 - 73: TOY
  fwrite(toy_stored_data,1,256,f);
  fwrite(toy_access_ports,1,4,f);

  // ISA bridge
  fwrite(isa_config_data,1,256,f);

  // Timer/Counter
  fwrite(&pit_enable,1,sizeof(bool),f);

  // interrupc controller
  fwrite(pic_mode,1,2*sizeof(int),f);
  fwrite(pic_intvec,1,2,f);
  fwrite(pic_mask,1,2,f);

  // IDE controller
  fwrite(ide_config_data,1,256,f);
  fwrite(ide_command,1,16,f);
  fwrite(ide_status,1,2,f);

  // USB host controller
  fwrite(usb_config_data,1,256,f);

  // DMA controller
}

/**
 * Restore state from a Virtual Machine State file.
 **/

void CAliM1543C::RestoreState(FILE *f)
{
  // REGISTERS 60 & 64: KEYBOARD
  fread(&kb_Input,1,1,f);
  fread(&kb_Output,1,1,f);
  fread(&kb_Command,1,1,f);
  fread(&kb_Status,1,1,f);
  fread(&kb_intState,1,1,f);

  // REGISTER 61 (NMI)
  fread(&reg_61,1,1,f);
    
  // REGISTERS 70 - 73: TOY
  fread(toy_stored_data,256,1,f);
  fread(toy_access_ports,4,1,f);

  // ISA bridge
  fread(isa_config_data,256,1,f);

  // Timer/Counter
  fread(&pit_enable,1,sizeof(bool),f);

  // interrupc controller
  fread(pic_mode,1,2*sizeof(int),f);
  fread(pic_intvec,1,2,f);
  fread(pic_mask,1,2,f);
  fread(pic_asserted,1,2,f);

  // IDE controller
  fread(ide_config_data,256,1,f);
  fread(ide_command,16,1,f);
  fread(ide_status,2,1,f);
  // allocations 
  ide_config_write(0x10,32,(*((u32*)(&ide_config_data[0x10])))&~1);
  ide_config_write(0x14,32,(*((u32*)(&ide_config_data[0x14])))&~1);
  ide_config_write(0x18,32,(*((u32*)(&ide_config_data[0x18])))&~1);
  ide_config_write(0x1c,32,(*((u32*)(&ide_config_data[0x1c])))&~1);
  ide_config_write(0x20,32,(*((u32*)(&ide_config_data[0x20])))&~1);

  // USB host controller
  fread(usb_config_data,256,1,f);

  // DMA controller
}
