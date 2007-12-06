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
 * X-1.33       Camiel Vanderhoeven                             6-DEC-2007
 *      Changed keyboard implementation (with thanks to the Bochs project!!)
 *
 * X-1.32       Brian Wheeler                                   2-DEC-2007
 *      Timing / floppy tweak for Linux/BSD guests.
 *
 * X-1.31       Brian Wheeler                                   1-DEC-2007
 *      Added console support (using SDL library), corrected timer
 *      behavior for Linux/BSD as a guest OS.
 *
 * X-1.30       Camiel Vanderhoeven                             1-DEC-2007
 *      Use correct interrupt for secondary IDE controller.
 *
 * X-1.29       Camiel Vanderhoeven                             17-NOV-2007
 *      Use CHECK_ALLOCATION.
 *
 * X-1.28       Eduardo Marcelo Serrat                          31-OCT-2007
 *      Corrected IDE interface revision level.
 *
 * X-1.27       Camiel Vanderhoeven                             18-APR-2007
 *      On a big-endian system, the LBA address for a read or write action
 *      was byte-swapped. Fixed this.
 *
 * X-1.26       Camiel Vanderhoeven                             17-APR-2007
 *      Removed debugging messages.
 *
 * X-1.25       Camiel Vanderhoeven                             16-APR-2007
 *      Added ResetPCI()
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

#include <math.h>

#include "gui/scancodes.h"
#include "gui/keymap.h"

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
  char *p;
  
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
  
  c->RegisterMemory(this, 1, X64(00000801fc000061),1);
  c->RegisterMemory(this, 28, X64(00000801fc000060),1);
  c->RegisterMemory(this, 29, X64(00000801fc000064),1);

  kbd_resetinternals(1);

  state.kbd_internal_buffer.led_status = 0;
  state.kbd_internal_buffer.scanning_enabled = 1;

  state.mouse_internal_buffer.num_elements = 0;
  for (i=0; i<BX_MOUSE_BUFF_SIZE; i++)
    state.mouse_internal_buffer.buffer[i] = 0;
  state.mouse_internal_buffer.head = 0;

  state.kbd_controller.pare = 0;
  state.kbd_controller.tim  = 0;
  state.kbd_controller.auxb = 0;
  state.kbd_controller.keyl = 1;
  state.kbd_controller.c_d  = 1;
  state.kbd_controller.sysf = 0;
  state.kbd_controller.inpb = 0;
  state.kbd_controller.outb = 0;

  state.kbd_controller.kbd_clock_enabled = 1;
  state.kbd_controller.aux_clock_enabled = 0;
  state.kbd_controller.allow_irq1 = 1;
  state.kbd_controller.allow_irq12 = 1;
  state.kbd_controller.kbd_output_buffer = 0;
  state.kbd_controller.aux_output_buffer = 0;
  state.kbd_controller.last_comm = 0;
  state.kbd_controller.expecting_port60h = 0;
  state.kbd_controller.irq1_requested = 0;
  state.kbd_controller.irq12_requested = 0;
  state.kbd_controller.expecting_mouse_parameter = 0;
  state.kbd_controller.bat_in_progress = 0;
  state.kbd_controller.scancodes_translate = 1;

  state.kbd_controller.timer_pending = 0;

  // Mouse initialization stuff
  state.mouse.captured        = atoi(cSystem->GetConfig("mouse.enabled","1"));
//  state.mouse.type            = SIM->get_param_enum(BXPN_MOUSE_TYPE)->get();
  state.mouse.sample_rate     = 100; // reports per second
  state.mouse.resolution_cpmm = 4;   // 4 counts per millimeter
  state.mouse.scaling         = 1;   /* 1:1 (default) */
  state.mouse.mode            = MOUSE_MODE_RESET;
  state.mouse.enable          = 0;
  state.mouse.delayed_dx      = 0;
  state.mouse.delayed_dy      = 0;
  state.mouse.delayed_dz      = 0;
  state.mouse.im_request      = 0; // wheel mouse mode request
  state.mouse.im_mode         = 0; // wheel mouse mode

  for (i=0; i<BX_KBD_CONTROLLER_QSIZE; i++)
    state.kbd_controller_Q[i] = 0;
  state.kbd_controller_Qsize = 0;
  state.kbd_controller_Qsource = 0;


  state.reg_61 = 0;

  c->RegisterMemory(this, 2, X64(00000801fc000070), 4);
  for (i=0;i<4;i++)
    state.toy_access_ports[i] = 0;
  for (i=0;i<256;i++)
    state.toy_stored_data[i] = 0;
  
#if defined(USE_CONSOLE)
  state.toy_stored_data[0x17] = 1;
#endif

  ResetPCI();
									
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

  c->RegisterMemory(this, 12, X64(00000801fc000000), 16);
  c->RegisterMemory(this, 13, X64(00000801fc0000c0), 32);


  // Initialize parallel port
  c->RegisterMemory(this,27, X64(00000801fc0003bc), 4);
  filename=c->GetConfig("lpt.outfile",NULL);
  if(filename) {
    lpt=fopen(filename,"wb");
  } else {
    lpt=NULL;
  }


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
	  return reg_61_read();
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
    case 27:
      return lpt_read(address);
    case 28:
      return kbd_60_read();
    case 29:
      return kbd_64_read();
    }

  return 0;
}

void CAliM1543C::WriteMem(int index, u64 address, int dsize, u64 data)
{
  int channel = 0;
  switch(index)
    {
    case 1:
	  reg_61_write((u8)data);
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
    case 27:
      lpt_write(address,(u8)data);
      return;
    case 28:
      kbd_60_write((u8)data);
      return;
    case 29:
      kbd_64_write((u8)data);
      return;
    }
}

void CAliM1543C::kbd_gen_scancode(u32 key)
{
  unsigned char *scancode;
  u8  i;

  printf("gen_scancode(): %s %s  \n", bx_keymap.getBXKeyName(key), (key >> 31)?"released":"pressed");

  if (!state.kbd_controller.scancodes_translate)
    BX_DEBUG(("keyboard: gen_scancode with scancode_translate cleared"));

  // Ignore scancode if keyboard clock is driven low
  if (state.kbd_controller.kbd_clock_enabled==0)
    return;

  // Ignore scancode if scanning is disabled
  if (state.kbd_internal_buffer.scanning_enabled==0)
    return;

  // Switch between make and break code
  if (key & BX_KEY_RELEASED)
    scancode=(unsigned char *)scancodes[(key&0xFF)][state.kbd_controller.current_scancodes_set].brek;
  else
    scancode=(unsigned char *)scancodes[(key&0xFF)][state.kbd_controller.current_scancodes_set].make;

  if (state.kbd_controller.scancodes_translate) {
    // Translate before send
    u8 escaped=0x00;

    for (i=0; i<strlen((const char *)scancode); i++) {
      if (scancode[i] == 0xF0)
        escaped=0x80;
      else {
	    printf("gen_scancode(): writing translated %02x   \n",translation8042[scancode[i] ] | escaped);
        kbd_enQ(translation8042[scancode[i]] | escaped);
        escaped=0x00;
      }
    }
  } 
  else {
    // Send raw data
    for (i=0; i<strlen((const char *)scancode); i++) {
      printf("gen_scancode(): writing raw %02x",scancode[i]);
      kbd_enQ(scancode[i]);
    }
  }
}

/* BDW:
  This may need some expansion to help with timer delays.  It looks like
  the 8254 flips bits on occasion, and the linux kernel (at least) uses
    do {
        count++;
    } while ((inb(0x61) & 0x20) == 0 && count < TIMEOUT_COUNT);
  to calibrate the cpu clock.

  Every 150000 reads the bit gets flipped so maybe the timing will
  seem reasonable to the OS.
 */
u8 CAliM1543C::reg_61_read()
{
  static long read_count = 0;
  if(!(state.reg_61 & 0x20)) {
    if (read_count % 1500 == 0)
      state.reg_61 |= 0x20;
  } else {
    state.reg_61 &= ~0x20;
  }
  read_count++;
  return state.reg_61;
}

void CAliM1543C::reg_61_write(u8 data)
{
  state.reg_61 = (state.reg_61 & 0xf0) | (((u8)data) & 0x0f);
}

u8 CAliM1543C::toy_read(u64 address)
{
  //printf("%%ALI-I-READTOY: read port %02x: 0x%02x\n", (u32)(0x70 + address), state.toy_access_ports[address]);

  return (u8)state.toy_access_ports[address];
}

void CAliM1543C::toy_write(u64 address, u8 data)
{
  time_t ltime;
  struct tm stime;
  static long read_count = 0;

  printf("%%ALI-I-WRITETOY: write port %02x: 0x%02x\n", (u32)(0x70 + address), data);


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

            

	      // Debian Linux wants something out of 0x0a.  It gets initialized
	      // with 0x26, by the SRM (I assume) but I don't know what this
	      // byte is supposed to mean...
	      // Ah, here's something from the linux kernel:
	      
	      //# /********************************************************
	      //# * register details
	      //# ********************************************************/
	      //# #define RTC_FREQ_SELECT RTC_REG_A
	      //#
	      //# /* update-in-progress - set to "1" 244 microsecs before RTC goes off the bus,
	      //# * reset after update (may take 1.984ms @ 32768Hz RefClock) is complete,
	      //# * totalling to a max high interval of 2.228 ms.
	      //# */
	      //# # define RTC_UIP 0x80
	      //# # define RTC_DIV_CTL 0x70
	      //# /* divider control: refclock values 4.194 / 1.049 MHz / 32.768 kHz */
	      //# # define RTC_REF_CLCK_4MHZ 0x00
	      //# # define RTC_REF_CLCK_1MHZ 0x10
	      //# # define RTC_REF_CLCK_32KHZ 0x20
	      //# /* 2 values for divider stage reset, others for "testing purposes only" */
	      //# # define RTC_DIV_RESET1 0x60
	      //# # define RTC_DIV_RESET2 0x70
	      //# /* Periodic intr. / Square wave rate select. 0=none, 1=32.8kHz,... 15=2Hz */
	      //# # define RTC_RATE_SELECT 0x0F
	      //#
	      
	      // Soooooo, the kernel seems to be waiting for the RTC to issue an
	      // update in progress and then clear it.  this clock takes 1500
	      // reads to switch from one state to another.
	      if(state.toy_stored_data[0x0a] & 0x80) {
		state.toy_stored_data[0x0a] &= ~0x80;
	      } else {
		if(read_count % 1500 == 0)
		  state.toy_stored_data[0x0a] |= 0x80;  
	      }
	      read_count++;
	      
	      //# /****************************************************/
	      //# #define RTC_CONTROL RTC_REG_B
	      //# # define RTC_SET 0x80 /* disable updates for clock setting */
	      //# # define RTC_PIE 0x40 /* periodic interrupt enable */
	      //# # define RTC_AIE 0x20 /* alarm interrupt enable */
	      //# # define RTC_UIE 0x10 /* update-finished interrupt enable */
	      //# # define RTC_SQWE 0x08 /* enable square-wave output */
	      //# # define RTC_DM_BINARY 0x04 /* all time/date values are BCD if clear */
	      //# # define RTC_24H 0x02 /* 24 hour mode - else hours bit 7 means pm */
	      //# # define RTC_DST_EN 0x01 /* auto switch DST - works f. USA only */
	      //#
	      // this is set (by the srm?) to 0x0e = SQWE | DM_BINARY | 24H
	      // sets the PIE bit.
    
	      //# /***********************************************************/
	      //# #define RTC_INTR_FLAGS RTC_REG_C
	      //# /* caution - cleared by read */
	      //# # define RTC_IRQF 0x80 /* any of the following 3 is active */
	      //# # define RTC_PF 0x40
	      //# # define RTC_AF 0x20
	      //# # define RTC_UF 0x10
	      //#
  
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

      /* linux reads from 0x1a twice and gets zeros.  Then it halts.  I
	 wonder what it is expecting.... */

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

#ifdef USE_CONSOLE
  bx_gui->handle_events();
#endif

  unsigned retval;

  retval=kbd_periodic();

  if(retval&0x01)
    pic_interrupt(0,1);
  if(retval&0x02)
    pic_interrupt(1,4);

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
	cSystem->RegisterMemory(this,21, X64(00000801fc000000) + (endian_32(data)&0x00fffffe), 8);
	return;
      case 0x14:
	// control
	cSystem->RegisterMemory(this,23, X64(00000801fc000000) + (endian_32(data)&0x00fffffe), 4);
	return;
      case 0x18:
	// command
	cSystem->RegisterMemory(this,22, X64(00000801fc000000) + (endian_32(data)&0x00fffffe), 8);
	return;
      case 0x1c:
	// control
	cSystem->RegisterMemory(this,24, X64(00000801fc000000) + (endian_32(data)&0x00fffffe), 4);
	return;
      case 0x20:
	// bus master control
	cSystem->RegisterMemory(this,25, X64(00000801fc000000) + (endian_32(data)&0x00fffffe), 8);
	// bus master control
	cSystem->RegisterMemory(this,26, X64(00000801fc000000) + (endian_32(data)&0x00fffffe) + 8, 8);
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
	      if (!(state.ide_control[index]&2))
		pic_interrupt(1,6+index);
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
	  if (!(state.ide_control[index]&2))
            pic_interrupt(1,6+index);
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

	      if (!(state.ide_control[index]&2))
		pic_interrupt(1,6+index);
	     

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
		pic_interrupt(1,6+index);
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
#ifdef DEBUG_IDE
	      printf("%%IDE-I-SETTRANS: Set IDE translation\n");
#endif
	      state.ide_status[index] = 0x40;
	      if (!(state.ide_control[index]&2))
		pic_interrupt(1,6+index);
	      break;

	    case 0x08: // reset drive (DRST)
#ifdef DEBUG_IDE

	      printf("%%IDE-I-RESET: IDE Reset\n");
#endif
	      state.ide_status[index]= 0x40;
	      if (!(state.ide_control[index]&2))
		pic_interrupt(1,6+index);
	      break;

	    case 0x00: // nop
	      state.ide_status[index]= 0x40;
	      if (!(state.ide_control[index]&2))
		pic_interrupt(1,6+index);
	      break;
        default:
	      state.ide_status[index] = 0x41;	// ERROR
	      state.ide_error[index] = 0x20;	// ABORTED

#ifdef DEBUG_IDE
	      printf("%%IDE-I-UNKCMND : Unknown IDE Command: ");
	      for (x=0;x<8;x++) printf("%02x ",state.ide_command[index][x]);
	      printf("\n");
#endif
	      if (!(state.ide_control[index]&2))
		pic_interrupt(1,6+index);
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

u64 CAliM1543C::ide_control_read(int index, u64 address)
{
  u64 data;

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

void CAliM1543C::ide_control_write(int index, u64 address, u64 data)
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

void CAliM1543C::ResetPCI()
{
  int i;

  cSystem->RegisterMemory(this, 3, X64(00000801fe003800),0x100);

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
									
  cSystem->RegisterMemory(this, 9, X64(00000801fe007800), 0x100);
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
  state.ide_config_data[0x08] = 0xC1;	// revision
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

//  cSystem->RegisterMemory(this, 11, X64(00000801fe009800), 0x100);
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

u8 CAliM1543C::lpt_read(u64 address) {
  u8 data = 0;
  switch(address) {
  case 0:
    data=state.lpt_data;
    break;
  case 1:
    // 1 0 0 0 1 000
    // ^ ^ ^ ^ ^----- printer has no-error condition
    // | | | +------- printer is not selected.
    // | | +--------- printer has paper
    // | +----------- printer is asserting 'ack'
    // +------------- printer is not busy.
    data = 0x88;
    break;
  case 2:
    data = state.lpt_control;
  }
  return data;
}


void CAliM1543C::lpt_write(u64 address, u8 data) {
  switch(address) {
  case 0:
    state.lpt_data=data;
    if(lpt)
      fputc(data,lpt);
    if(state.lpt_control & 0x10)
      pic_interrupt(0,7);
    break;
  case 1:
    break;
  case 2:
    state.lpt_control=data;
  }
}

void CAliM1543C::kbd_resetinternals(bool powerup)
{
  state.kbd_internal_buffer.num_elements = 0;
  for (int i=0; i<BX_KBD_ELEMENTS; i++)
    state.kbd_internal_buffer.buffer[i] = 0;
  state.kbd_internal_buffer.head = 0;

  state.kbd_internal_buffer.expecting_typematic = 0;
  state.kbd_internal_buffer.expecting_make_break = 0;

  // Default scancode set is mf2 (translation is controlled by the 8042)
  state.kbd_controller.expecting_scancodes_set = 0;
  state.kbd_controller.current_scancodes_set = 1;
  
  if (powerup) {
    state.kbd_internal_buffer.expecting_led_write = 0;
    state.kbd_internal_buffer.delay = 1; // 500 mS
    state.kbd_internal_buffer.repeat_rate = 0x0b; // 10.9 chars/sec
  }
}

void CAliM1543C::kbd_enQ(u8 scancode)
{
  int tail;

  printf("kbd_enQ(0x%02x)", (unsigned) scancode);

  if (state.kbd_internal_buffer.num_elements >= BX_KBD_ELEMENTS) {
    printf("internal keyboard buffer full, ignoring scancode.(%02x)",
      (unsigned) scancode);
    return;
  }

  /* enqueue scancode in multibyte internal keyboard buffer */
  BX_DEBUG(("kbd_enQ: putting scancode 0x%02x in internal buffer",
      (unsigned) scancode));
  tail = (state.kbd_internal_buffer.head + state.kbd_internal_buffer.num_elements) %
   BX_KBD_ELEMENTS;
  state.kbd_internal_buffer.buffer[tail] = scancode;
  state.kbd_internal_buffer.num_elements++;

  if (!state.kbd_controller.outb && state.kbd_controller.kbd_clock_enabled) {
    state.kbd_controller.timer_pending = 1;
    printf("activating timer...");
    return;
  }
}

u8 CAliM1543C::kbd_60_read()
{
  u8 val;
/* output buffer */
    if (state.kbd_controller.auxb) { /* mouse byte available */
      val = state.kbd_controller.aux_output_buffer;
      state.kbd_controller.aux_output_buffer = 0;
      state.kbd_controller.outb = 0;
      state.kbd_controller.auxb = 0;
      state.kbd_controller.irq12_requested = 0;

      if (state.kbd_controller_Qsize) {
        unsigned i;
        state.kbd_controller.aux_output_buffer = state.kbd_controller_Q[0];
        state.kbd_controller.outb = 1;
        state.kbd_controller.auxb = 1;
        if (state.kbd_controller.allow_irq12)
          state.kbd_controller.irq12_requested = 1;
        for (i=0; i<state.kbd_controller_Qsize-1; i++) {
          // move Q elements towards head of queue by one
          state.kbd_controller_Q[i] = state.kbd_controller_Q[i+1];
        }
        state.kbd_controller_Qsize--;
      }

      //DEV_pic_lower_irq(12);
      state.kbd_controller.timer_pending = 1;
      BX_DEBUG(("[mouse] read from 0x60 returns 0x%02x", val));
      return val;
    }
    else if (state.kbd_controller.outb) { /* kbd byte available */
      val = state.kbd_controller.kbd_output_buffer;
      state.kbd_controller.outb = 0;
      state.kbd_controller.auxb = 0;
      state.kbd_controller.irq1_requested = 0;
      state.kbd_controller.bat_in_progress = 0;

      if (state.kbd_controller_Qsize) {
        unsigned i;
        state.kbd_controller.aux_output_buffer = state.kbd_controller_Q[0];
        state.kbd_controller.outb = 1;
        state.kbd_controller.auxb = 1;
        if (state.kbd_controller.allow_irq1)
          state.kbd_controller.irq1_requested = 1;
        for (i=0; i<state.kbd_controller_Qsize-1; i++) {
          // move Q elements towards head of queue by one
          state.kbd_controller_Q[i] = state.kbd_controller_Q[i+1];
        }
	BX_DEBUG(("s.controller_Qsize: %02X",state.kbd_controller_Qsize));
        state.kbd_controller_Qsize--;
      }

//      DEV_pic_lower_irq(1);
      state.kbd_controller.timer_pending = 1;
      BX_DEBUG(("READ(60) = %02x", (unsigned) val));
      return val;
    }
    else {
      BX_DEBUG(("num_elements = %d", state.kbd_internal_buffer.num_elements));
      BX_DEBUG(("read from port 60h with outb empty"));
      return state.kbd_controller.kbd_output_buffer;
    }
}

u8 CAliM1543C::kbd_64_read()
{
  u8 val;
/* status register */
    val = (state.kbd_controller.pare << 7)  |
          (state.kbd_controller.tim  << 6)  |
          (state.kbd_controller.auxb << 5)  |
          (state.kbd_controller.keyl << 4)  |
          (state.kbd_controller.c_d  << 3)  |
          (state.kbd_controller.sysf << 2)  |
          (state.kbd_controller.inpb << 1)  |
          (state.kbd_controller.outb << 0);
    state.kbd_controller.tim = 0;
    return val;
}

void CAliM1543C::kbd_60_write(u8 value)
{
// input buffer
      // if expecting data byte from command last sent to port 64h
      if (state.kbd_controller.expecting_port60h) {
        state.kbd_controller.expecting_port60h = 0;
        // data byte written last to 0x60
        state.kbd_controller.c_d = 0;
        if (state.kbd_controller.inpb) {
          BX_PANIC(("write to port 60h, not ready for write"));
        }
        switch (state.kbd_controller.last_comm) {
          case 0x60: // write command byte
            {
            bx_bool scan_convert, disable_keyboard,
                    disable_aux;

            scan_convert = (value >> 6) & 0x01;
            disable_aux      = (value >> 5) & 0x01;
            disable_keyboard = (value >> 4) & 0x01;
            state.kbd_controller.sysf = (value >> 2) & 0x01;
            state.kbd_controller.allow_irq1  = (value >> 0) & 0x01;
            state.kbd_controller.allow_irq12 = (value >> 1) & 0x01;
            set_kbd_clock_enable(!disable_keyboard);
            set_aux_clock_enable(!disable_aux);
            if (state.kbd_controller.allow_irq12 && state.kbd_controller.auxb)
              state.kbd_controller.irq12_requested = 1;
            else if (state.kbd_controller.allow_irq1  && state.kbd_controller.outb)
              state.kbd_controller.irq1_requested = 1;

            BX_DEBUG(( " allow_irq12 set to %u",
              (unsigned) state.kbd_controller.allow_irq12));
            if (!scan_convert)
              BX_INFO(("keyboard: scan convert turned off"));

	    // (mch) NT needs this
	    state.kbd_controller.scancodes_translate = scan_convert;
            }
            break;
          case 0xd1: // write output port
            BX_DEBUG(("write output port with value %02xh", (unsigned) value));
	    BX_DEBUG(("write output port : %sable A20",(value & 0x02)?"en":"dis"));
//            BX_SET_ENABLE_A20((value & 0x02) != 0);
            if (!(value & 0x01)) {
              BX_INFO(("write output port : processor reset requested!"));
//              bx_pc_system.Reset( BX_RESET_SOFTWARE);
            }
            break;
          case 0xd4: // Write to mouse
            // I don't think this enables the AUX clock
            //set_aux_clock_enable(1); // enable aux clock line
            kbd_ctrl_to_mouse(value);
            // ??? should I reset to previous value of aux enable?
            break;

          case 0xd3: // write mouse output buffer
            // Queue in mouse output buffer
            kbd_controller_enQ(value, 1);
            break;

	  case 0xd2:
	    // Queue in keyboard output buffer
	    kbd_controller_enQ(value, 0);
	    break;

          default:
            BX_PANIC(("=== unsupported write to port 60h(lastcomm=%02x): %02x",
              (unsigned) state.kbd_controller.last_comm, (unsigned) value));
          }
      } else {
        // data byte written last to 0x60
        state.kbd_controller.c_d = 0;
        state.kbd_controller.expecting_port60h = 0;
        /* pass byte to keyboard */
        /* ??? should conditionally pass to mouse device here ??? */
        if (state.kbd_controller.kbd_clock_enabled==0) {
          set_kbd_clock_enable(1);
        }
        kbd_ctrl_to_kbd(value);
      }
}

void CAliM1543C::kbd_64_write(u8 value)
{
  static int kbd_initialized=0;
  u8 command_byte;

// control register
      // command byte written last to 0x64
      state.kbd_controller.c_d = 1;
      state.kbd_controller.last_comm = value;
      // most commands NOT expecting port60 write next
      state.kbd_controller.expecting_port60h = 0;

      switch (value) {
        case 0x20: // get keyboard command byte
          BX_DEBUG(("get keyboard command byte"));
          // controller output buffer must be empty
          if (state.kbd_controller.outb) {
            BX_ERROR(("kbd: OUTB set and command 0x%02x encountered", value));
            break;
          }
          command_byte =
            (state.kbd_controller.scancodes_translate << 6) |
            ((!state.kbd_controller.aux_clock_enabled) << 5) |
            ((!state.kbd_controller.kbd_clock_enabled) << 4) |
            (0 << 3) |
            (state.kbd_controller.sysf << 2) |
            (state.kbd_controller.allow_irq12 << 1) |
            (state.kbd_controller.allow_irq1  << 0);
          kbd_controller_enQ(command_byte, 0);
          break;
        case 0x60: // write command byte
          BX_DEBUG(("write command byte"));
          // following byte written to port 60h is command byte
          state.kbd_controller.expecting_port60h = 1;
          break;

        case 0xa0:
          BX_DEBUG(("keyboard BIOS name not supported"));
          break;

        case 0xa1:
          BX_DEBUG(("keyboard BIOS version not supported"));
          break;

        case 0xa7: // disable the aux device
          set_aux_clock_enable(0);
          BX_DEBUG(("aux device disabled"));
          break;
        case 0xa8: // enable the aux device
          set_aux_clock_enable(1);
          BX_DEBUG(("aux device enabled"));
          break;
        case 0xa9: // Test Mouse Port
          // controller output buffer must be empty
          if (state.kbd_controller.outb) {
            BX_PANIC(("kbd: OUTB set and command 0x%02x encountered", value));
            break;
          }
          kbd_controller_enQ(0x00, 0); // no errors detected
          break;
        case 0xaa: // motherboard controller self test
          BX_DEBUG(("Self Test"));
	  if (kbd_initialized == 0) {
	    state.kbd_controller_Qsize = 0;
	    state.kbd_controller.outb = 0;
	    kbd_initialized++;
	  }
          // controller output buffer must be empty
          if (state.kbd_controller.outb) {
            BX_ERROR(("kbd: OUTB set and command 0x%02x encountered", value));
            break;
          }
	  // (mch) Why is this commented out??? Enabling
          state.kbd_controller.sysf = 1; // self test complete
          kbd_controller_enQ(0x55, 0); // controller OK
          break;
        case 0xab: // Interface Test
          // controller output buffer must be empty
          if (state.kbd_controller.outb) {
            BX_PANIC(("kbd: OUTB set and command 0x%02x encountered", value));
            break;
          }
          kbd_controller_enQ(0x00, 0);
          break;
        case 0xad: // disable keyboard
          set_kbd_clock_enable(0);
          BX_DEBUG(("keyboard disabled"));
          break;
        case 0xae: // enable keyboard
          set_kbd_clock_enable(1);
          BX_DEBUG(("keyboard enabled"));
          break;
        case 0xaf: // get controller version
          BX_INFO(("'get controller version' not supported yet"));
          break;
        case 0xc0: // read input port
          // controller output buffer must be empty
          if (state.kbd_controller.outb) {
            BX_PANIC(("kbd: OUTB set and command 0x%02x encountered", value));
            break;
          }
          // keyboard not inhibited
          kbd_controller_enQ(0x80, 0);
          break;
        case 0xd0: // read output port: next byte read from port 60h
          BX_DEBUG(("io write to port 64h, command d0h (partial)"));
          // controller output buffer must be empty
          if (state.kbd_controller.outb) {
            BX_PANIC(("kbd: OUTB set and command 0x%02x encountered", value));
            break;
          }
          kbd_controller_enQ(
              (state.kbd_controller.irq12_requested << 5) |
              (state.kbd_controller.irq1_requested << 4) |
//              (BX_GET_ENABLE_A20() << 1) |
              0x01, 0);
          break;

        case 0xd1: // write output port: next byte written to port 60h
          BX_DEBUG(("write output port"));
          // following byte to port 60h written to output port
          state.kbd_controller.expecting_port60h = 1;
          break;

        case 0xd3: // write mouse output buffer
	  //FIXME: Why was this a panic?
          BX_DEBUG(("io write 0x64: command = 0xD3(write mouse outb)"));
	  // following byte to port 60h written to output port as mouse write.
          state.kbd_controller.expecting_port60h = 1;
          break;

        case 0xd4: // write to mouse
          BX_DEBUG(("io write 0x64: command = 0xD4 (write to mouse)"));
          // following byte written to port 60h
          state.kbd_controller.expecting_port60h = 1;
          break;

        case 0xd2: // write keyboard output buffer
	  BX_DEBUG(("io write 0x64: write keyboard output buffer"));
	  state.kbd_controller.expecting_port60h = 1;
	  break;
        case 0xdd: // Disable A20 Address Line
//	  BX_SET_ENABLE_A20(0);
	  break;
        case 0xdf: // Enable A20 Address Line
//	  BX_SET_ENABLE_A20(1);
	  break;
        case 0xc1: // Continuous Input Port Poll, Low
        case 0xc2: // Continuous Input Port Poll, High
        case 0xe0: // Read Test Inputs
          BX_PANIC(("io write 0x64: command = %02xh", (unsigned) value));
          break;

        case 0xfe: // System (cpu?) Reset, transition to real mode
          BX_INFO(("io write 0x64: command 0xfe: reset cpu"));
//          bx_pc_system.Reset(BX_RESET_SOFTWARE);
          break;

        default:
          if (value==0xff || (value>=0xf0 && value<=0xfd)) {
            /* useless pulse output bit commands ??? */
            BX_DEBUG(("io write to port 64h, useless command %02x",
                (unsigned) value));
            return;
          }
          BX_ERROR(("unsupported io write to keyboard port 64, value = %x",
            (unsigned) value));
          break;
      }
}

void CAliM1543C::kbd_controller_enQ(Bit8u data, unsigned source)
{
  // source is 0 for keyboard, 1 for mouse

  BX_DEBUG(("kbd_controller_enQ(%02x) source=%02x", (unsigned) data,source));

  // see if we need to Q this byte from the controller
  // remember this includes mouse bytes.
  if (state.kbd_controller.outb) {
    if (state.kbd_controller_Qsize >= BX_KBD_CONTROLLER_QSIZE)
      BX_PANIC(("controller_enq(): controller_Q full!"));
    state.kbd_controller_Q[state.kbd_controller_Qsize++] = data;
    state.kbd_controller_Qsource = source;
    return;
  }

  // the Q is empty
  if (source == 0) { // keyboard
    state.kbd_controller.kbd_output_buffer = data;
    state.kbd_controller.outb = 1;
    state.kbd_controller.auxb = 0;
    state.kbd_controller.inpb = 0;
    if (state.kbd_controller.allow_irq1)
      state.kbd_controller.irq1_requested = 1;
  } else { // mouse
    state.kbd_controller.aux_output_buffer = data;
    state.kbd_controller.outb = 1;
    state.kbd_controller.auxb = 1;
    state.kbd_controller.inpb = 0;
    if (state.kbd_controller.allow_irq12)
      state.kbd_controller.irq12_requested = 1;
  }
}

void CAliM1543C::set_kbd_clock_enable(u8   value)
{
  bool prev_kbd_clock_enabled;

  if (value==0) {
    state.kbd_controller.kbd_clock_enabled = 0;
  } else {
    /* is another byte waiting to be sent from the keyboard ? */
    prev_kbd_clock_enabled = state.kbd_controller.kbd_clock_enabled;
    state.kbd_controller.kbd_clock_enabled = 1;

    if (prev_kbd_clock_enabled==0 && state.kbd_controller.outb==0) {
      state.kbd_controller.timer_pending = 1;
    }
  }
}

void CAliM1543C::set_aux_clock_enable(u8   value)
{
  bool prev_aux_clock_enabled;

  BX_DEBUG(("set_aux_clock_enable(%u)", (unsigned) value));
  if (value==0) {
    state.kbd_controller.aux_clock_enabled = 0;
  } else {
    /* is another byte waiting to be sent from the keyboard ? */
    prev_aux_clock_enabled = state.kbd_controller.aux_clock_enabled;
    state.kbd_controller.aux_clock_enabled = 1;
    if (prev_aux_clock_enabled==0 && state.kbd_controller.outb==0)
      state.kbd_controller.timer_pending = 1;
  }
}

void CAliM1543C::kbd_ctrl_to_kbd(Bit8u value)
{
  BX_DEBUG(("controller passed byte %02xh to keyboard", value));

  if (state.kbd_internal_buffer.expecting_make_break) {
    printf("setting key %x to make/break mode (unused)   \n", value);
    kbd_enQ(0xFA); // send ACK
    return;
  }

  if (state.kbd_internal_buffer.expecting_typematic) {
    state.kbd_internal_buffer.expecting_typematic = 0;
    state.kbd_internal_buffer.delay = (value >> 5) & 0x03;
    switch (state.kbd_internal_buffer.delay) {
      case 0: BX_INFO(("setting delay to 250 mS (unused)")); break;
      case 1: BX_INFO(("setting delay to 500 mS (unused)")); break;
      case 2: BX_INFO(("setting delay to 750 mS (unused)")); break;
      case 3: BX_INFO(("setting delay to 1000 mS (unused)")); break;
    }
    state.kbd_internal_buffer.repeat_rate = value & 0x1f;
    double cps = 1 /((double)(8 + (value & 0x07)) * (double)exp(log((double)2) * (double)((value >> 3) & 0x03)) * 0.00417);
    BX_INFO(("setting repeat rate to %.1f cps (unused)", cps));
    kbd_enQ(0xFA); // send ACK
    return;
  }

  if (state.kbd_internal_buffer.expecting_led_write) {
    state.kbd_internal_buffer.expecting_led_write = 0;
    state.kbd_internal_buffer.led_status = value;
    BX_DEBUG(("LED status set to %02x",
      (unsigned) state.kbd_internal_buffer.led_status));
    kbd_enQ(0xFA); // send ACK %%%
    return;
  }

  if (state.kbd_controller.expecting_scancodes_set) {
    state.kbd_controller.expecting_scancodes_set = 0;
    if( value != 0 ) {
      if( value<4 ) {
        state.kbd_controller.current_scancodes_set = (value-1);
        BX_INFO(("Switched to scancode set %d",
          (unsigned) state.kbd_controller.current_scancodes_set + 1));
        kbd_enQ(0xFA);
      } else {
        BX_ERROR(("Received scancodes set out of range: %d", value ));
        kbd_enQ(0xFF); // send ERROR
      }
    } else {
      // Send ACK (SF patch #1159626)
      kbd_enQ(0xFA);
      // Send current scancodes set to port 0x60
      kbd_enQ(1 + (state.kbd_controller.current_scancodes_set)); 
    }
    return;
  }

  switch (value) {
    case 0x00: // ??? ignore and let OS timeout with no response
      kbd_enQ(0xFA); // send ACK %%%
      break;

    case 0x05: // ???
      // (mch) trying to get this to work...
      state.kbd_controller.sysf = 1;
      kbd_enQ_imm(0xfe);
      break;

    case 0xed: // LED Write
      state.kbd_internal_buffer.expecting_led_write = 1;
      kbd_enQ_imm(0xFA); // send ACK %%%
      break;

    case 0xee: // echo
      kbd_enQ(0xEE); // return same byte (EEh) as echo diagnostic
      break;

    case 0xf0: // Select alternate scan code set
      state.kbd_controller.expecting_scancodes_set = 1;
      BX_DEBUG(("Expecting scancode set info..."));
      kbd_enQ(0xFA); // send ACK
      break;

    case 0xf2:  // identify keyboard
      BX_INFO(("identify keyboard command received"));

      // XT sends nothing, AT sends ACK 
      // MFII with translation sends ACK+ABh+41h
      // MFII without translation sends ACK+ABh+83h
//      if (SIM->get_param_enum(BXPN_KBD_TYPE)->get() != BX_KBD_XT_TYPE) {
        kbd_enQ(0xFA); 
//        if (SIM->get_param_enum(BXPN_KBD_TYPE)->get() == BX_KBD_MF_TYPE) {
          kbd_enQ(0xAB);
          
          if(state.kbd_controller.scancodes_translate)
            kbd_enQ(0x41);
          else
            kbd_enQ(0x83);
  //      }
//      }
      break;

    case 0xf3:  // typematic info
      state.kbd_internal_buffer.expecting_typematic = 1;
      BX_INFO(("setting typematic info"));
      kbd_enQ(0xFA); // send ACK
      break;

    case 0xf4:  // enable keyboard
      state.kbd_internal_buffer.scanning_enabled = 1;
      kbd_enQ(0xFA); // send ACK
      break;

    case 0xf5:  // reset keyboard to power-up settings and disable scanning
      kbd_resetinternals(1);
      kbd_enQ(0xFA); // send ACK
      state.kbd_internal_buffer.scanning_enabled = 0;
      BX_INFO(("reset-disable command received"));
      break;

    case 0xf6:  // reset keyboard to power-up settings and enable scanning
      kbd_resetinternals(1);
      kbd_enQ(0xFA); // send ACK
      state.kbd_internal_buffer.scanning_enabled = 1;
      BX_INFO(("reset-enable command received"));
      break;

    case 0xfc:  // PS/2 Set Key Type to Make/Break
      state.kbd_internal_buffer.expecting_make_break = 1;
      kbd_enQ(0xFA); /* send ACK */
      break;

    case 0xfe:  // resend. aiiee.
      BX_PANIC(("got 0xFE (resend)"));
      break;

    case 0xff:  // reset: internal keyboard reset and afterwards the BAT
      BX_DEBUG(("reset command received"));
      kbd_resetinternals(1);
      kbd_enQ(0xFA); // send ACK
      state.kbd_controller.bat_in_progress = 1;
      kbd_enQ(0xAA); // BAT test passed
      break;

    case 0xd3:
      kbd_enQ(0xfa);
      break;

    case 0xf7:  // PS/2 Set All Keys To Typematic
    case 0xf8:  // PS/2 Set All Keys to Make/Break
    case 0xf9:  // PS/2 PS/2 Set All Keys to Make
    case 0xfa:  // PS/2 Set All Keys to Typematic Make/Break
    case 0xfb:  // PS/2 Set Key Type to Typematic
    case 0xfd:  // PS/2 Set Key Type to Make
    default:
      BX_ERROR(("kbd_ctrl_to_kbd(): got value of 0x%02x", value));
      kbd_enQ(0xFE); /* send NACK */
      break;
  }
}

void CAliM1543C::kbd_enQ_imm(Bit8u val)
{
  int tail;

  if (state.kbd_internal_buffer.num_elements >= BX_KBD_ELEMENTS) {
    BX_PANIC(("internal keyboard buffer full (imm)"));
    return;
  }

  /* enqueue scancode in multibyte internal keyboard buffer */
  tail = (state.kbd_internal_buffer.head + state.kbd_internal_buffer.num_elements) %
    BX_KBD_ELEMENTS;

  state.kbd_controller.kbd_output_buffer = val;
  state.kbd_controller.outb = 1;

  if (state.kbd_controller.allow_irq1)
    state.kbd_controller.irq1_requested = 1;
}

void CAliM1543C::kbd_ctrl_to_mouse(u8 value)
{
  // if we are not using a ps2 mouse, some of the following commands need to return different values
//  bx_bool is_ps2 = 0;
//  if ((state.mouse.type == BX_MOUSE_TYPE_PS2) ||
//      (state.mouse.type == BX_MOUSE_TYPE_IMPS2)) is_ps2 = 1;

  BX_DEBUG(("MOUSE: kbd_ctrl_to_mouse(%02xh)", (unsigned) value));
  BX_DEBUG(("  enable = %u", (unsigned) state.mouse.enable));
  BX_DEBUG(("  allow_irq12 = %u",
    (unsigned) state.kbd_controller.allow_irq12));
  BX_DEBUG(("  aux_clock_enabled = %u",
    (unsigned) state.kbd_controller.aux_clock_enabled));

  // an ACK (0xFA) is always the first response to any valid input
  // received from the system other than Set-Wrap-Mode & Resend-Command

  if (state.kbd_controller.expecting_mouse_parameter) {
    state.kbd_controller.expecting_mouse_parameter = 0;
    switch (state.kbd_controller.last_mouse_command) {
      case 0xf3: // Set Mouse Sample Rate
        state.mouse.sample_rate = value;
        BX_DEBUG(("[mouse] Sampling rate set: %d Hz", value));
        if ((value == 200) && (!state.mouse.im_request)) {
          state.mouse.im_request = 1;
        } else if ((value == 100) && (state.mouse.im_request == 1)) {
          state.mouse.im_request = 2;
        } else if ((value == 80) && (state.mouse.im_request == 2)) {
//          if (state.mouse.type == BX_MOUSE_TYPE_IMPS2) {
            BX_INFO(("wheel mouse mode enabled"));
            state.mouse.im_mode = 1;
//          } else {
//            BX_INFO(("wheel mouse mode request rejected"));
//          }
          state.mouse.im_request = 0;
        } else {
          state.mouse.im_request = 0;
        }
        kbd_controller_enQ(0xFA, 1); // ack
        break;

      case 0xe8: // Set Mouse Resolution
        switch (value) {
          case 0:
            state.mouse.resolution_cpmm = 1;
            break;
          case 1:
            state.mouse.resolution_cpmm = 2;
            break;
          case 2:
            state.mouse.resolution_cpmm = 4;
            break;
          case 3:
            state.mouse.resolution_cpmm = 8;
            break;
          default:
            BX_PANIC(("[mouse] Unknown resolution %d", value));
            break;
        }
        BX_DEBUG(("[mouse] Resolution set to %d counts per mm",
          state.mouse.resolution_cpmm));

        kbd_controller_enQ(0xFA, 1); // ack
        break;

      default:
        BX_PANIC(("MOUSE: unknown last command (%02xh)", (unsigned) state.kbd_controller.last_mouse_command));
    }
  } else {
    state.kbd_controller.expecting_mouse_parameter = 0;
    state.kbd_controller.last_mouse_command = value;

    // test for wrap mode first
    if (state.mouse.mode == MOUSE_MODE_WRAP) {
      // if not a reset command or reset wrap mode
      // then just echo the byte.
      if ((value != 0xff) && (value != 0xec)) {
//        if (bx_dbg.mouse)
          BX_INFO(("[mouse] wrap mode: Ignoring command %0X02.",value));
        kbd_controller_enQ(value,1);
        // bail out
        return;
      }
    }
    switch (value) {
      case 0xe6: // Set Mouse Scaling to 1:1
        kbd_controller_enQ(0xFA, 1); // ACK
        state.mouse.scaling = 2;
        BX_DEBUG(("[mouse] Scaling set to 1:1"));
        break;

      case 0xe7: // Set Mouse Scaling to 2:1
        kbd_controller_enQ(0xFA, 1); // ACK
        state.mouse.scaling         = 2;
        BX_DEBUG(("[mouse] Scaling set to 2:1"));
        break;

      case 0xe8: // Set Mouse Resolution
        kbd_controller_enQ(0xFA, 1); // ACK
        state.kbd_controller.expecting_mouse_parameter = 1;
        break;

      case 0xea: // Set Stream Mode
//        if (bx_dbg.mouse)
          BX_INFO(("[mouse] Mouse stream mode on."));
        state.mouse.mode = MOUSE_MODE_STREAM;
        kbd_controller_enQ(0xFA, 1); // ACK
        break;

      case 0xec: // Reset Wrap Mode
        // unless we are in wrap mode ignore the command
        if (state.mouse.mode == MOUSE_MODE_WRAP) {
//          if (bx_dbg.mouse)
            BX_INFO(("[mouse] Mouse wrap mode off."));
          // restore previous mode except disable stream mode reporting.
          // ### TODO disabling reporting in stream mode
          state.mouse.mode = state.mouse.saved_mode;
          kbd_controller_enQ(0xFA, 1); // ACK
        }
        break;
      case 0xee: // Set Wrap Mode
        // ### TODO flush output queue.
        // ### TODO disable interrupts if in stream mode.
//        if (bx_dbg.mouse)
          BX_INFO(("[mouse] Mouse wrap mode on."));
        state.mouse.saved_mode = state.mouse.mode;
        state.mouse.mode = MOUSE_MODE_WRAP;
        kbd_controller_enQ(0xFA, 1); // ACK
        break;

      case 0xf0: // Set Remote Mode (polling mode, i.e. not stream mode.)
//        if (bx_dbg.mouse)
          BX_INFO(("[mouse] Mouse remote mode on."));
        // ### TODO should we flush/discard/ignore any already queued packets?
        state.mouse.mode = MOUSE_MODE_REMOTE;
        kbd_controller_enQ(0xFA, 1); // ACK
        break;

      case 0xf2: // Read Device Type
        kbd_controller_enQ(0xFA, 1); // ACK
        if (state.mouse.im_mode)
          kbd_controller_enQ(0x03, 1); // Device ID (wheel z-mouse)
        else
          kbd_controller_enQ(0x00, 1); // Device ID (standard)
        BX_DEBUG(("[mouse] Read mouse ID"));
        break;

      case 0xf3: // Set Mouse Sample Rate (sample rate written to port 60h)
        kbd_controller_enQ(0xFA, 1); // ACK
        state.kbd_controller.expecting_mouse_parameter = 1;
        break;

      case 0xf4: // Enable (in stream mode)
        // is a mouse present?
//        if (is_ps2) {
          state.mouse.enable = 1;
          kbd_controller_enQ(0xFA, 1); // ACK
          BX_DEBUG(("[mouse] Mouse enabled (stream mode)"));
//        } else {
//          // a mouse isn't present.  We need to return a 0xFE (resend) instead of a 0xFA (ACK)
//          kbd_controller_enQ(0xFE, 1); // RESEND
//          state.kbd_controller.tim = 1;
//        }
        break;

      case 0xf5: // Disable (in stream mode)
        state.mouse.enable = 0;
        kbd_controller_enQ(0xFA, 1); // ACK
        BX_DEBUG(("[mouse] Mouse disabled (stream mode)"));
        break;

      case 0xf6: // Set Defaults
        state.mouse.sample_rate     = 100; /* reports per second (default) */
        state.mouse.resolution_cpmm = 4; /* 4 counts per millimeter (default) */
        state.mouse.scaling         = 1;   /* 1:1 (default) */
        state.mouse.enable          = 0;
        state.mouse.mode            = MOUSE_MODE_STREAM;
        kbd_controller_enQ(0xFA, 1); // ACK
        BX_DEBUG(("[mouse] Set Defaults"));
        break;

      case 0xff: // Reset
        // is a mouse present?
//        if (is_ps2) {
          state.mouse.sample_rate     = 100; /* reports per second (default) */
          state.mouse.resolution_cpmm = 4; /* 4 counts per millimeter (default) */
          state.mouse.scaling         = 1;   /* 1:1 (default) */
          state.mouse.mode            = MOUSE_MODE_RESET;
          state.mouse.enable          = 0;
          if (state.mouse.im_mode)
            BX_INFO(("wheel mouse mode disabled"));
          state.mouse.im_mode         = 0;
          /* (mch) NT expects an ack here */
          kbd_controller_enQ(0xFA, 1); // ACK
          kbd_controller_enQ(0xAA, 1); // completion code
          kbd_controller_enQ(0x00, 1); // ID code (standard after reset)
          BX_DEBUG(("[mouse] Mouse reset"));
//        } else {
//          // a mouse isn't present.  We need to return a 0xFE (resend) instead of a 0xFA (ACK)
//          kbd_controller_enQ(0xFE, 1); // RESEND
//          state.kbd_controller.tim = 1;
//        }
        break;

      case 0xe9: // Get mouse information
        // should we ack here? (mch): Yes
        kbd_controller_enQ(0xFA, 1); // ACK
        kbd_controller_enQ(state.mouse.get_status_byte(), 1); // status
        kbd_controller_enQ(state.mouse.get_resolution_byte(), 1); // resolution
        kbd_controller_enQ(state.mouse.sample_rate, 1); // sample rate
        BX_DEBUG(("[mouse] Get mouse information"));
        break;

      case 0xeb: // Read Data (send a packet when in Remote Mode)
        kbd_controller_enQ(0xFA, 1); // ACK
        // perhaps we should be adding some movement here.
        mouse_enQ_packet(((state.mouse.button_status & 0x0f) | 0x08),
          0x00, 0x00, 0x00); // bit3 of first byte always set
        //assumed we really aren't in polling mode, a rather odd assumption.
        BX_ERROR(("[mouse] Warning: Read Data command partially supported."));
        break;

      case 0xbb: // OS/2 Warp 3 uses this command
       BX_ERROR(("[mouse] ignoring 0xbb command"));
       break;

      default:
        // If PS/2 mouse present, send NACK for unknown commands, otherwise ignore
//        if (is_ps2) {
          BX_ERROR(("[mouse] kbd_ctrl_to_mouse(): got value of 0x%02x", value));
          kbd_controller_enQ(0xFE, 1); /* send NACK */
//        }
    }
  }
}

bool CAliM1543C::mouse_enQ_packet(u8 b1, u8 b2, u8 b3, u8 b4)
{
  int bytes = 3;
  if (state.mouse.im_mode) bytes = 4;

  if ((state.mouse_internal_buffer.num_elements + bytes) >= BX_MOUSE_BUFF_SIZE) {
    return(0); /* buffer doesn't have the space */
  }

  mouse_enQ(b1);
  mouse_enQ(b2);
  mouse_enQ(b3);
  if (state.mouse.im_mode) mouse_enQ(b4);

  return(1);
}

void CAliM1543C::mouse_enQ(u8 mouse_data)
{
  int tail;

  BX_DEBUG(("mouse_enQ(%02x)", (unsigned) mouse_data));

  if (state.mouse_internal_buffer.num_elements >= BX_MOUSE_BUFF_SIZE) {
    BX_ERROR(("[mouse] internal mouse buffer full, ignoring mouse data.(%02x)",
      (unsigned) mouse_data));
    return;
  }

  /* enqueue mouse data in multibyte internal mouse buffer */
  tail = (state.mouse_internal_buffer.head + state.mouse_internal_buffer.num_elements) %
   BX_MOUSE_BUFF_SIZE;
  state.mouse_internal_buffer.buffer[tail] = mouse_data;
  state.mouse_internal_buffer.num_elements++;

  if (!state.kbd_controller.outb && state.kbd_controller.aux_clock_enabled) {
    state.kbd_controller.timer_pending = 1;
    return;
  }
}

unsigned CAliM1543C::kbd_periodic()
{
  static unsigned count_before_paste=0;
  Bit8u   retval;

  //if (state.kbd_controller.kbd_clock_enabled ) {
  //  if(++count_before_paste>=BX_KEY_THIS pastedelay) {
  //    // after the paste delay, consider adding moving more chars
  //    // from the paste buffer to the keyboard buffer.
  //    stateervice_paste_buf();
  //    count_before_paste=0;
  //  }
  //}

  retval = (state.kbd_controller.irq1_requested << 0)
         | (state.kbd_controller.irq12_requested << 1);
  state.kbd_controller.irq1_requested = 0;
  state.kbd_controller.irq12_requested = 0;

  if (state.kbd_controller.timer_pending == 0) {
    return(retval);
  }

  if (1 >= state.kbd_controller.timer_pending) {
    state.kbd_controller.timer_pending = 0;
  } else {
    state.kbd_controller.timer_pending -= 1;
    return(retval);
  }

  if (state.kbd_controller.outb) {
    return(retval);
  }

  /* nothing in outb, look for possible data xfer from keyboard or mouse */
  if (state.kbd_internal_buffer.num_elements &&
      (state.kbd_controller.kbd_clock_enabled || state.kbd_controller.bat_in_progress)) {
    BX_DEBUG(("service_keyboard: key in internal buffer waiting"));
    state.kbd_controller.kbd_output_buffer =
      state.kbd_internal_buffer.buffer[state.kbd_internal_buffer.head];
    state.kbd_controller.outb = 1;
    // commented out since this would override the current state of the
    // mouse buffer flag - no bug seen - just seems wrong (das)
    //    state.kbd_controller.auxb = 0;
    state.kbd_internal_buffer.head = (state.kbd_internal_buffer.head + 1) %
      BX_KBD_ELEMENTS;
    state.kbd_internal_buffer.num_elements--;
    if (state.kbd_controller.allow_irq1)
      state.kbd_controller.irq1_requested = 1;
  } else { 
    create_mouse_packet(0);
    if (state.kbd_controller.aux_clock_enabled && state.mouse_internal_buffer.num_elements) {
      BX_DEBUG(("service_keyboard: key(from mouse) in internal buffer waiting"));
      state.kbd_controller.aux_output_buffer =
	state.mouse_internal_buffer.buffer[state.mouse_internal_buffer.head];

      state.kbd_controller.outb = 1;
      state.kbd_controller.auxb = 1;
      state.mouse_internal_buffer.head = (state.mouse_internal_buffer.head + 1) %
	BX_MOUSE_BUFF_SIZE;
      state.mouse_internal_buffer.num_elements--;
      if (state.kbd_controller.allow_irq12)
	state.kbd_controller.irq12_requested = 1;
    } else {
      BX_DEBUG(("service_keyboard(): no keys waiting"));
    }
  }
  return(retval);
}

void CAliM1543C::create_mouse_packet(bool force_enq)
{
  Bit8u b1, b2, b3, b4;

  if(state.mouse_internal_buffer.num_elements && !force_enq)
    return;

  Bit16s delta_x = state.mouse.delayed_dx;
  Bit16s delta_y = state.mouse.delayed_dy;
  Bit8u button_state=state.mouse.button_status | 0x08;

  if(!force_enq && !delta_x && !delta_y) {
    return;
  }

  if(delta_x>254) delta_x=254;
  if(delta_x<-254) delta_x=-254;
  if(delta_y>254) delta_y=254;
  if(delta_y<-254) delta_y=-254;

  b1 = (button_state & 0x0f) | 0x08; // bit3 always set

  if ( (delta_x>=0) && (delta_x<=255) ) {
    b2 = (Bit8u) delta_x;
    state.mouse.delayed_dx-=delta_x;
  }
  else if (delta_x > 255) {
    b2 = (Bit8u) 0xff;
    state.mouse.delayed_dx-=255;
  }
  else if (delta_x >= -256) {
    b2 = (Bit8u) delta_x;
    b1 |= 0x10;
    state.mouse.delayed_dx-=delta_x;
  }
  else {
    b2 = (Bit8u) 0x00;
    b1 |= 0x10;
    state.mouse.delayed_dx+=256;
  }

  if ( (delta_y>=0) && (delta_y<=255) ) {
    b3 = (Bit8u) delta_y;
    state.mouse.delayed_dy-=delta_y;
  }
  else if (delta_y > 255) {
    b3 = (Bit8u) 0xff;
    state.mouse.delayed_dy-=255;
  }
  else if (delta_y >= -256) {
    b3 = (Bit8u) delta_y;
    b1 |= 0x20;
    state.mouse.delayed_dy-=delta_y;
  }
  else {
    b3 = (Bit8u) 0x00;
    b1 |= 0x20;
    state.mouse.delayed_dy+=256;
  }

  b4 = (Bit8u) -state.mouse.delayed_dz;

  mouse_enQ_packet(b1, b2, b3, b4);
}
