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
 * Contains the definitions for the emulated Ali M1543C chipset devices.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
 **/

#if !defined(INCLUDED_ALIM1543C_H_)
#define INCLUDED_ALIM1543C_H

#include "SystemComponent.h"

/**
 * Disk information structure.
 **/

struct disk_info {
  FILE *handle;         /**< disk image handle. */
  char *filename;       /**< disk image filename. */
  int size;           /**< disk image size in 512-byte blocks */  
  int mode;           /**< disk image mode. */
};

/**
 * Emulated ALi M1543C multi-function device.
 * The ALi M1543C device provides i/o and glue logic support to the system: 
 * ISA, USB, IDE, DMA, Interrupt, Timer, TOY Clock. 
 *
 * Known shortcomings:
 *   - IDE
 *     - disk images are not checked for size, so size is not always correctly 
 *       reported.
 *     - IDE disks can be read but not written.
 *     .
 *   .
 **/

class CAliM1543C : public CSystemComponent  
{
 public:
  virtual void SaveState(FILE * f);
  virtual void RestoreState(FILE * f);
  void instant_tick();
  //	void interrupt(int number);
  virtual int DoClock();
  virtual void WriteMem(int index, u64 address, int dsize, u64 data);

  virtual u64 ReadMem(int index, u64 address, int dsize);
  CAliM1543C(class CSystem * c);
  virtual ~CAliM1543C();
  void pic_interrupt(int index, int intno);
 private:

  // REGISTERS 60 & 64: KEYBOARD
  u8 kb_read(u64 address);
  void kb_write(u64 address, u8 data);
  u8 kb_Input;
  u8 kb_Output;   	
  u8 kb_Status;   	
  u8 kb_Command;
  u8 kb_intState;

  // REGISTER 61 (NMI)
  u8 reg_61_read();
  void reg_61_write(u8 data);
  u8 reg_61;
    
  // REGISTERS 70 - 73: TOY
  u8 toy_read(u64 address);
  void toy_write(u64 address, u8 data);
  u8 toy_stored_data[256];
  u8 toy_access_ports[4];

  // ISA bridge
  u64 isa_config_read(u64 address, int dsize);
  void isa_config_write(u64 address, int dsize, u64 data);
  u8 isa_config_data[256];
  u8 isa_config_mask[256];

  // Timer/Counter
  u8 pit_read(u64 address);
  void pit_write(u64 address, u8 data);
  bool pit_enable;

  // interrupt controller
  u8 pic_read(int index, u64 address);
  void pic_write(int index, u64 address, u8 data);
  u8 pic_read_vector();
  int pic_mode[2];
  u8 pic_intvec[2];
  u8 pic_mask[2];
  u8 pic_asserted[2];

  // IDE controller
  u64 ide_config_read(u64 address, int dsize);
  void ide_config_write(u64 address, int dsize, u64 data);
  u64 ide_command_read(int channel, u64 address);
  void ide_command_write(int channel, u64 address, u64 data);
  u64 ide_control_read(int channel, u64 address);
  void ide_control_write(int channel, u64 address, u64 data);
  u64 ide_busmaster_read(int channel, u64 address);
  void ide_busmaster_write(int channel, u64 address, u64 data);
  u8 ide_config_data[256];
  u8 ide_config_mask[256];

  // USB host controller
  u64 usb_config_read(u64 address, int dsize);
  void usb_config_write(u64 address, int dsize, u64 data);
  u8 usb_config_data[256];
  u8 usb_config_mask[256];

  // DMA controller
  u8 dma_read(int channel, u64 address);
  void dma_write(int channel, u64 address, u8 data);

  u8 ide_command[2][8];
  u8 ide_status[2];
  u16 ide_data[2][256];
  int ide_data_ptr[2];
  bool ide_writing[2];
  bool ide_reading[2];
  int ide_sectors[2];
  int ide_selected[2];

  struct disk_info ide_info[2][2];

};

#endif // !defined(INCLUDED_ALIM1543C_H)
