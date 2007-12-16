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
 * Contains the definitions for the emulated Symbios SCSI controller.
 *
 * X-1.2        Camiel Vanderhoeven                             16-DEC-2007
 *      Changed register structure.
 *
 * X-1.1        Camiel Vanderhoeven                             14-DEC-2007
 *      Initial version in CVS
 **/

#if !defined(INCLUDED_SYM53C895_H_)
#define INCLUDED_SYM53C895_H_

#include "DiskController.h"
#include "Configurator.h"

class CSym53C895 : public CDiskController  
{
 public:
  virtual void SaveState(FILE * f);
  virtual void RestoreState(FILE * f);
  virtual int DoClock();

  virtual void WriteMem_Bar(int func,int bar, u32 address, int dsize, u32 data);
  virtual u32 ReadMem_Bar(int func,int bar, u32 address, int dsize);

  virtual u32 config_read_custom(int func, u32 address, int dsize, u32 data);
  virtual void config_write_custom(int func, u32 address, int dsize, u32 old_data, u32 new_data, u32 data);

  CSym53C895(CConfigurator * cfg, class CSystem * c, int pcibus, int pcidev);
  virtual ~CSym53C895();

 private:

  void write_b_scntl0(u8 value);
  void write_b_scntl1(u8 value);
  void write_b_scntl3(u8 value);
  void write_b_istat(u8 value);
  u8 read_b_ctest2();
  void write_b_ctest3(u8 value);
  void write_b_ctest4(u8 value);
  void write_b_ctest5(u8 value);
  void write_b_stest2(u8 value);
  void write_b_stest3(u8 value);
  u8 read_b_dstat();
  u8 read_b_sist(int id);
  void write_b_dcntl(u8 value);
  u8 read_b_scratcha(int reg);
  u8 read_b_scratchb(int reg);

  void post_dsp_write();

  void select_target(int target);
  void byte_to_target(u8 value);
  u8 byte_from_target();
  void end_xfer();
  int do_command();
  void eval_interrupts();
  void set_interrupt(int reg, u8 interrupt);

// The state structure contains all elements that need to be saved to the statefile.
  struct SSym53C895State {

    bool irq_asserted;

    union {
      u8 reg8[128];
      u16 reg16[64];
      u32 reg32[64];
    } regs;

    struct {
      bool carry;
    } alu;

    u8 ram[4096];

    bool executing;

    u8 dstat_stack;
    u8 sist0_stack;
    u8 sist1_stack;

    int phase;

    struct {
      // msgi: Message In Phase (disk -> controller)
      u8 msgi[10];
      int msgi_len;
      int msgi_ptr;

      // msgo: Message Out Phase (controller -> disk)
      u8 msgo[10];
      int msgo_len;

      bool msg_err;

      // cmd: Command phase 
      u8 cmd[20];
      int cmd_len;

      u8 dati[512];
      int dati_ptr;
      int dati_len;
      bool dati_off_disk;

      u8 dato[512];
      int dato_ptr;
      int dato_len;
      bool dato_to_disk;

      int stat[10];
      int stat_ptr;
      int stat_len;

    } per_target[16];

  } state;

};

#endif // !defined(INCLUDED_SYM_H)
