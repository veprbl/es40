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
 * Contains the definitions for the emulated Ali M1543C IDE chipset part.
 *
 * X-1.8         Brian wheeler                                   19-DEC-2007
 *      Added basic ATAPI support.
 *
 * X-1.7         Brian wheeler                                   17-DEC-2007
 *      Delayed IDE interrupts. (NetBSD requirement)
 *
 * X-1.6        Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.5        Camiel Vanderhoeven                             12-DEC-2007
 *      Use disk controller base class.
 *
 * X-1.4        Camiel Vanderhoeven                             11-DEC-2007
 *      Cleanup.
 *
 * X-1.3        Camiel Vanderhoeven                             11-DEC-2007
 *      More complete IDE implementation allows NetBSD to recognize disks.
 *
 * X-1.2        Brian Wheeler                                   10-DEC-2007
 *      Fixed include case.
 *
 * X-1.1        Camiel Vanderhoeven                             10-DEC-2007
 *      Initial version in CVS; this part was split off from the CAliM1543C
 *      class.
 **/

#if !defined(INCLUDED_ALIM1543C_IDE_H_)
#define INCLUDED_ALIM1543C_IDE_H_

#include "DiskController.h"
//#include "gui/gui.h"
#include "Configurator.h"

#define MAX_MULTIPLE_SECTORS 16

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

class CAliM1543C_ide : public CDiskController  
{
 public:
  virtual int SaveState(FILE * f);
  virtual int RestoreState(FILE * f);

  virtual void WriteMem_Legacy(int index, u32 address, int dsize, u32 data);
  virtual u32 ReadMem_Legacy(int index, u32 address, int dsize);

  virtual void WriteMem_Bar(int func,int bar, u32 address, int dsize, u32 data);
  virtual u32 ReadMem_Bar(int func,int bar, u32 address, int dsize);

  virtual int  DoClock();

  CAliM1543C_ide(CConfigurator * cfg, class CSystem * c, int pcibus, int pcidev);
  virtual ~CAliM1543C_ide();
//  FILE * get_ide_disk(int controller, int drive);
  virtual void ResetPCI();

 private:

  // IDE controller
  u32 ide_command_read(int channel, u32 address, int dsize);
  void ide_command_write(int channel, u32 address, int dsize, u32 data);
  u32 ide_control_read(int channel, u32 address);
  void ide_control_write(int channel, u32 address, u32 data);
  u32 ide_busmaster_read(int channel, u32 address, int dsize);
  void ide_busmaster_write(int channel, u32 address, u32 data, int dsize);

  void raise_interrupt(int channel);
  void set_signature(int channel, int id);
  u8 get_status(int index);
  void command_aborted(int index, u8 command);
  void identify_drive(int index);

// The state structure contains all elements that need to be saved to the statefile.
  struct SAliM1543C_ideState {

    struct {
      bool disable_irq;
      bool reset;
      bool irq_ready;
    } ide_control[2];

    struct {
      bool busy;
      bool drive_ready;
      bool seek_complete;
      bool drq;
      bool err;
      bool index_pulse;
      int index_pulse_count;
      u8 current_command;
    } ide_status[2][2];

    struct {
      int head_no;
      int sector_count;
      int sector_no;
      int cylinder_no;
      int features;
      bool lba_mode;
    } ide_per_drive[2][2];
    
    bool ide_reset_in_progress[2];
    u8 ide_error[2];
    u16 ide_data[2][256];
    int ide_data_ptr[2];
    int ide_atapi_size[2];
    int ide_sectors[2];
    int ide_selected[2];
    u8 ide_bm_status[2];

    // Bus Mastering
    u8 busmaster[2][8];

  } state;

//  struct disk_info ide_info[2][2];
};

extern CAliM1543C_ide * theAliIDE;
#endif // !defined(INCLUDED_ALIM1543C_IDE_H)
