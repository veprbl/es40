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

#include "SystemComponent.h"
#include "gui/gui.h"

#define BX_KBD_ELEMENTS 16
#define BX_MOUSE_BUFF_SIZE 48

#define MOUSE_MODE_RESET  10
#define MOUSE_MODE_STREAM 11
#define MOUSE_MODE_REMOTE 12
#define MOUSE_MODE_WRAP   13

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
  FILE * get_ide_disk(int controller, int drive);
  virtual void ResetPCI();

  void kbd_gen_scancode(u32 key);

 private:

  // REGISTERS 60 & 64: KEYBOARD
  u8 kbd_60_read();
  void kbd_60_write(u8 data);
  u8 kbd_64_read();
  void kbd_64_write(u8 data);
  void kbd_resetinternals(bool powerup);
  void kbd_enQ(u8 scancode);
  void kbd_controller_enQ(Bit8u data, unsigned source);
  void set_kbd_clock_enable(u8 value);
  void set_aux_clock_enable(u8 value);
  void kbd_ctrl_to_kbd(u8 value);
  void kbd_enQ_imm(u8 val);
  void kbd_ctrl_to_mouse(u8 value);
  bool mouse_enQ_packet(u8 b1, u8 b2, u8 b3, u8 b4);
  void mouse_enQ(u8 mouse_data);
  unsigned kbd_periodic();
  void create_mouse_packet(bool force_enq);

  // REGISTER 61 (NMI)
  u8 reg_61_read();
  void reg_61_write(u8 data);

  // REGISTERS 70 - 73: TOY
  u8 toy_read(u64 address);
  void toy_write(u64 address, u8 data);

  // ISA bridge
  u64 isa_config_read(u64 address, int dsize);
  void isa_config_write(u64 address, int dsize, u64 data);

  // Timer/Counter
  u8 pit_read(u64 address);
  void pit_write(u64 address, u8 data);

  // interrupt controller
  u8 pic_read(int index, u64 address);
  void pic_write(int index, u64 address, u8 data);
  u8 pic_read_vector();

  // IDE controller
  u64 ide_config_read(u64 address, int dsize);
  void ide_config_write(u64 address, int dsize, u64 data);
  u64 ide_command_read(int channel, u64 address);
  void ide_command_write(int channel, u64 address, u64 data);
  u64 ide_control_read(int channel, u64 address);
  void ide_control_write(int channel, u64 address, u64 data);
  u64 ide_busmaster_read(int channel, u64 address);
  void ide_busmaster_write(int channel, u64 address, u64 data);

  // USB host controller
  u64 usb_config_read(u64 address, int dsize);
  void usb_config_write(u64 address, int dsize, u64 data);

  // DMA controller
  u8 dma_read(int channel, u64 address);
  void dma_write(int channel, u64 address, u8 data);

  // LPT controller
  u8 lpt_read(u64 address);
  void lpt_write(u64 address, u8 data);

#ifdef USE_CONSOLE
  u8 key2scan(int key);
#endif

  // The state structure contains all elements that need to be saved to the statefile.
  struct SAliM1543CState {
    // REGISTERS 60 & 64: KEYBOARD / MOUSE
    struct {
      /* status bits matching the status port*/
      bx_bool pare; // Bit7, 1= parity error from keyboard/mouse - ignored.
      bx_bool tim;  // Bit6, 1= timeout from keyboard - ignored.
      bx_bool auxb; // Bit5, 1= mouse data waiting for CPU to read.
      bx_bool keyl; // Bit4, 1= keyswitch in lock position - ignored.
      bx_bool c_d; /*  Bit3, 1=command to port 64h, 0=data to port 60h */
      bx_bool sysf; // Bit2,
      bx_bool inpb; // Bit1,
      bx_bool outb; // Bit0, 1= keyboard data or mouse data ready for CPU
                    //       check aux to see which. Or just keyboard
                    //       data before AT style machines

      /* internal to our version of the keyboard controller */
      bx_bool kbd_clock_enabled;
      bx_bool aux_clock_enabled;
      bx_bool allow_irq1;
      bx_bool allow_irq12;
      Bit8u   kbd_output_buffer;
      Bit8u   aux_output_buffer;
      Bit8u   last_comm;
      Bit8u   expecting_port60h;
      Bit8u   expecting_mouse_parameter;
      Bit8u   last_mouse_command;
      Bit32u   timer_pending;
      bx_bool irq1_requested;
      bx_bool irq12_requested;
      bx_bool scancodes_translate;
      bx_bool expecting_scancodes_set;
      Bit8u   current_scancodes_set;
      bx_bool bat_in_progress;
    } kbd_controller;

    struct mouseStruct {
      bx_bool captured; // host mouse capture enabled
//      Bit8u   type;
      Bit8u   sample_rate;
      Bit8u   resolution_cpmm; // resolution in counts per mm
      Bit8u   scaling;
      Bit8u   mode;
      Bit8u   saved_mode;  // the mode prior to entering wrap mode
      bx_bool enable;

      Bit8u get_status_byte ()
	{
	  // top bit is 0 , bit 6 is 1 if remote mode.
	  Bit8u ret = (Bit8u) ((mode == MOUSE_MODE_REMOTE) ? 0x40 : 0);
	  ret |= (enable << 5);
	  ret |= (scaling == 1) ? 0 : (1 << 4);
	  ret |= ((button_status & 0x1) << 2);
	  ret |= ((button_status & 0x2) << 0);
	  return ret;
	}

      Bit8u get_resolution_byte ()
	{
	  Bit8u ret = 0;

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

      Bit8u button_status;
      Bit16s delayed_dx;
      Bit16s delayed_dy;
      Bit16s delayed_dz;
      Bit8u im_request;
      bx_bool im_mode;
    } mouse;

    struct {
      int     num_elements;
      Bit8u   buffer[BX_KBD_ELEMENTS];
      int     head;
      bx_bool expecting_typematic;
      bx_bool expecting_led_write;
      bool expecting_make_break;
      Bit8u   delay;
      Bit8u   repeat_rate;
      Bit8u   led_status;
      bx_bool scanning_enabled;
    } kbd_internal_buffer;

    struct {
      int     num_elements;
      Bit8u   buffer[BX_MOUSE_BUFF_SIZE];
      int     head;
    } mouse_internal_buffer;
#define BX_KBD_CONTROLLER_QSIZE 5
    Bit8u    kbd_controller_Q[BX_KBD_CONTROLLER_QSIZE];
    unsigned kbd_controller_Qsize;
    unsigned kbd_controller_Qsource; // 0=keyboard, 1=mouse

    // REGISTER 61 (NMI)
    u8 reg_61;
    
    // REGISTERS 70 - 73: TOY
    u8 toy_stored_data[256];
    u8 toy_access_ports[4];

    // ISA bridge
    u8 isa_config_data[256];
    u8 isa_config_mask[256];

    // Timer/Counter
    bool pit_enable;

    // interrupt controller
    int pic_mode[2];
    u8 pic_intvec[2];
    u8 pic_mask[2];
    u8 pic_asserted[2];

    // IDE controller
    u8 ide_config_data[256];
    u8 ide_config_mask[256];
    u8 ide_command[2][8];
    u8 ide_control[2];
    u8 ide_status[2];
    u8 ide_error[2];
    u16 ide_data[2][256];
    int ide_data_ptr[2];
    bool ide_writing[2];
    bool ide_reading[2];
    int ide_sectors[2];
    int ide_selected[2];

    // USB host controller
    u8 usb_config_data[256];
    u8 usb_config_mask[256];

    u8 lpt_data;
    u8 lpt_control;
  } state;

  struct disk_info ide_info[2][2];
  FILE *lpt;
};

inline FILE * CAliM1543C::get_ide_disk(int controller, int drive)
{
  return ide_info[controller][drive].handle;
}

#endif // !defined(INCLUDED_ALIM1543C_H)
