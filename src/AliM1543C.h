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
 * Contains the definitions for the emulated Ali M1543C chipset devices.
 *
 * X-1.23       Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.22       Brian wheeler                                   17-DEC-2007
 *      Better DMA support.      
 *
 * X-1.21       Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.20       Brian Wheeler                                   11-DEC-2007
 *      Improved timer logic (again).
 *
 * X-1.19       Camiel Vanderhoeven                             10-DEC-2007
 *      Use configurator; move IDE and USB to their own classes.
 *
 * X-1.18       Camiel Vanderhoeven                             7-DEC-2007
 *      Add busmaster_status; add pic_edge_level.
 *
 * X-1.17       Camiel Vanderhoeven                             7-DEC-2007
 *      Generate keyboard interrupts when needed.
 *
 * X-1.16       Camiel Vanderhoeven                             6-DEC-2007
 *      Changed keyboard implementation (with thanks to the Bochs project!!)
 *
 * X-1.15       Brian Wheeler                                   1-DEC-2007
 *      Added console support (using SDL library), corrected timer
 *      behavior for Linux/BSD as a guest OS.
 *
 * X-1.14       Camiel Vanderhoeven                             16-APR-2007
 *      Added ResetPCI()
 *
 * X-1.13       Camiel Vanderhoeven                             11-APR-2007
 *      Moved all data that should be saved to a state file to a structure
 *      "state".
 *
 * X-1.12       Camiel Vanderhoeven                             31-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.11	Camiel Vanderhoeven				3-MAR-2007
 *	Added inline function get_ide_disk, which returns a file handle.
 *
 * X-1.10	Camiel Vanderhoeven				20-FEB-2007
 *	Added member variable to keep error status.
 *
 * X-1.9	Brian Wheeler					20-FEB-2007
 *	Information about IDE disks is now kept in the ide_info structure.
 *
 * X-1.8	Camiel Vanderhoeven				16-FEB-2007
 *	DoClock now returns int.
 *
 * X-1.7	Camiel Vanderhoeven				12-FEB-2007
 *	Formatting.
 *
 * X-1.6	Camiel Vanderhoeven				12-FEB-2007
 *	Added comments.
 *
 * X-1.5        Camiel Vanderhoeven                             9-FEB-2007
 *      Replaced f_ variables with ide_ members.
 *
 * X-1.4        Camiel Vanderhoeven                             9-FEB-2007
 *      Added comments.
 *
 * X-1.3        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.2        Brian Wheeler                                   3-FEB-2007
 *      Includes are now case-correct (necessary on Linux)
 *
 * X-1.1        Camiel Vanderhoeven                             19-JAN-2007
 *      Initial version in CVS.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#if !defined(INCLUDED_ALIM1543C_H_)
#define INCLUDED_ALIM1543C_H

#include "PCIDevice.h"
#include "gui/gui.h"
#include "Configurator.h"

#define BX_KBD_ELEMENTS 16
#define BX_MOUSE_BUFF_SIZE 48

#define MOUSE_MODE_RESET  10
#define MOUSE_MODE_STREAM 11
#define MOUSE_MODE_REMOTE 12
#define MOUSE_MODE_WRAP   13

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

class CAliM1543C : public CPCIDevice
{
 public:
  virtual int SaveState(FILE * f);
  virtual int RestoreState(FILE * f);
  void instant_tick();
  //	void interrupt(int number);
  virtual int DoClock();
  virtual void WriteMem_Legacy(int index, u32 address, int dsize, u32 data);
  virtual u32 ReadMem_Legacy(int index, u32 address, int dsize);

  CAliM1543C(CConfigurator * cfg, class CSystem * c, int pcibus, int pcidev);
  virtual ~CAliM1543C();
  void pic_interrupt(int index, int intno);
  void pic_deassert(int index, int intno);

  void kbd_gen_scancode(u32 key);

 private:

  // REGISTERS 60 & 64: KEYBOARD
  u8 kbd_60_read();
  void kbd_60_write(u8 data);
  u8 kbd_64_read();
  void kbd_64_write(u8 data);
  void kbd_resetinternals(bool powerup);
  void kbd_enQ(u8 scancode);
  void kbd_controller_enQ(u8 data, unsigned source);
  void set_kbd_clock_enable(u8 value);
  void set_aux_clock_enable(u8 value);
  void kbd_ctrl_to_kbd(u8 value);
  void kbd_enQ_imm(u8 val);
  void kbd_ctrl_to_mouse(u8 value);
  bool mouse_enQ_packet(u8 b1, u8 b2, u8 b3, u8 b4);
  void mouse_enQ(u8 mouse_data);
  unsigned kbd_periodic();
  void kbd_clock();
  void create_mouse_packet(bool force_enq);

  // REGISTER 61 (NMI)
  u8 reg_61_read();
  void reg_61_write(u8 data);

  // REGISTERS 70 - 73: TOY
  u8 toy_read(u32 address);
  void toy_write(u32 address, u8 data);

  // Timer/Counter
  u8 pit_read(u32 address);
  void pit_write(u32 address, u8 data);
  void pit_clock();

  // interrupt controller
  u8 pic_read(int index, u32 address);
  void pic_write(int index, u32 address, u8 data);
  u8 pic_read_vector();
  u8 pic_read_edge_level(int index);
  void pic_write_edge_level(int index, u8 data);

  // DMA controller
  u8 dma_read(int channel, u32 address);
  void dma_write(int channel, u32 address, u8 data);

  // LPT controller
  u8 lpt_read(u32 address);
  void lpt_write(u32 address, u8 data);

#ifdef USE_CONSOLE
  u8 key2scan(int key);
#endif

  // The state structure contains all elements that need to be saved to the statefile.
  struct SAliM1543CState {
    // REGISTERS 60 & 64: KEYBOARD / MOUSE
    struct {
      /* status bits matching the status port*/
      bool pare; // Bit7, 1= parity error from keyboard/mouse - ignored.
      bool tim;  // Bit6, 1= timeout from keyboard - ignored.
      bool auxb; // Bit5, 1= mouse data waiting for CPU to read.
      bool keyl; // Bit4, 1= keyswitch in lock position - ignored.
      bool c_d; /*  Bit3, 1=command to port 64h, 0=data to port 60h */
      bool sysf; // Bit2,
      bool inpb; // Bit1,
      bool outb; // Bit0, 1= keyboard data or mouse data ready for CPU
                    //       check aux to see which. Or just keyboard
                    //       data before AT style machines

      /* internal to our version of the keyboard controller */
      bool kbd_clock_enabled;
      bool aux_clock_enabled;
      bool allow_irq1;
      bool allow_irq12;
      u8   kbd_output_buffer;
      u8   aux_output_buffer;
      u8   last_comm;
      u8   expecting_port60h;
      u8   expecting_mouse_parameter;
      u8   last_mouse_command;
      u32   timer_pending;
      bool irq1_requested;
      bool irq12_requested;
      bool scancodes_translate;
      bool expecting_scancodes_set;
      u8   current_scancodes_set;
      bool bat_in_progress;
    } kbd_controller;

    struct mouseStruct {
      bool captured; // host mouse capture enabled
//      u8   type;
      u8   sample_rate;
      u8   resolution_cpmm; // resolution in counts per mm
      u8   scaling;
      u8   mode;
      u8   saved_mode;  // the mode prior to entering wrap mode
      bool enable;

      u8 get_status_byte ()
	{
	  // top bit is 0 , bit 6 is 1 if remote mode.
	  u8 ret = (u8) ((mode == MOUSE_MODE_REMOTE) ? 0x40 : 0);
	  ret |= (enable << 5);
	  ret |= (scaling == 1) ? 0 : (1 << 4);
	  ret |= ((button_status & 0x1) << 2);
	  ret |= ((button_status & 0x2) << 0);
	  return ret;
	}

      u8 get_resolution_byte ()
	{
	  u8 ret = 0;

	  switch (resolution_cpmm) {
	  case 1:
	    ret = 0;
	    break;

	  case 2:
	    ret = 1;
	    break;

	  case 4:
	    ret = 2;
	    break;

	  case 8:
	    ret = 3;
	    break;

	  default:
	    FAILURE("mouse: invalid resolution_cpmm");
	  };
	  return ret;
	}

      u8 button_status;
      s16 delayed_dx;
      s16 delayed_dy;
      s16 delayed_dz;
      u8 im_request;
      bool im_mode;
    } mouse;

    struct {
      int     num_elements;
      u8   buffer[BX_KBD_ELEMENTS];
      int     head;
      bool expecting_typematic;
      bool expecting_led_write;
      bool expecting_make_break;
      u8   delay;
      u8   repeat_rate;
      u8   led_status;
      bool scanning_enabled;
    } kbd_internal_buffer;

    struct {
      int     num_elements;
      u8   buffer[BX_MOUSE_BUFF_SIZE];
      int     head;
    } mouse_internal_buffer;
#define BX_KBD_CONTROLLER_QSIZE 5
    u8    kbd_controller_Q[BX_KBD_CONTROLLER_QSIZE];
    unsigned kbd_controller_Qsize;
    unsigned kbd_controller_Qsource; // 0=keyboard, 1=mouse

    // REGISTER 61 (NMI)
    u8 reg_61;
    
    // REGISTERS 70 - 73: TOY
    u8 toy_stored_data[256];
    u8 toy_access_ports[4];

    // Timer/Counter
    u32 pit_counter[9];
#define PIT_OFFSET_LATCH 3
#define PIT_OFFSET_MAX 6
    u8 pit_status[4];
    u8 pit_mode[4];

    // interrupt controller
    int pic_mode[2];
    u8 pic_intvec[2];
    u8 pic_mask[2];
    u8 pic_asserted[2];
    u8 pic_edge_level[2];

    // DMA Controller
    struct dmaStruct {
      bool lobyte;
      u16 current;
      u16 base;
      u16 pagebase;
      u16 count;
    } dma_channel[8];

    struct dmacontrollerStruct {
      u8 status;
      u8 command;
      u8 writereq;
      u8 mask;
      u8 mode;

    } dma_controller[2];



    u8 lpt_data;
    u8 lpt_control;
  } state;

  FILE *lpt;
};

extern CAliM1543C * theAli;
#endif // !defined(INCLUDED_ALIM1543C_H)
