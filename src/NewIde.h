/* ES40 emulator.
 * Copyright (C) 2007-2008 by the ES40 Emulator Project
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
 * X-1.1         Brian wheeler                                   08-JAN-2008
 *      Complete rewrite of IDE controller.
 *
 * \todo This code is currently used in stead of the "normal" IDE controller
 *       if HAVE_NEW_IDE is defined. When this implementation is finished,
 *       it should take over the function of the old controller; this file
 *       will disappear, and the code in this file should be moved into
 *       AliM1543C_ide.h as class CAliM1543C_ide.
 **/

#if !defined(INCLUDED_NEWIDE_H_)
#define INCLUDED_NEWIDE_H_

#include "DiskController.h"
#include "Configurator.h"

#define MAX_MULTIPLE_SECTORS 16


class CNewIde : public CDiskController  
{
 public:
  CNewIde(CConfigurator * cfg, class CSystem * c, int pcibus, int pcidev);
  virtual ~CNewIde();

  virtual void WriteMem_Legacy(int index, u32 address, int dsize, u32 data);
  virtual u32 ReadMem_Legacy(int index, u32 address, int dsize);

  virtual void WriteMem_Bar(int func,int bar, u32 address, int dsize, u32 data);
  virtual u32 ReadMem_Bar(int func,int bar, u32 address, int dsize);

  virtual int SaveState(FILE * f);
  virtual int RestoreState(FILE * f);

  virtual int  DoClock();
  virtual void ResetPCI();

 private:

  // IDE controller
  u32 ide_command_read(int channel, u32 address, int dsize);
  void ide_command_write(int channel, u32 address, int dsize, u32 data);
  u32 ide_control_read(int channel, u32 address);
  void ide_control_write(int channel, u32 address, u32 data);
  u32 ide_busmaster_read(int channel, u32 address, int dsize);
  void ide_busmaster_write(int channel, u32 address, u32 data, int dsize);
  int do_dma_transfer(int index, u8 *buffer, u32 size, bool direction);

  void raise_interrupt(int channel);
  void set_signature(int channel, int id);
  u8 get_status(int index);
  void command_aborted(int index, u8 command);
  void identify_drive(int index,bool packet);
  void ide_status(int index);

// The state structure contains all elements that need to be saved to the statefile.
  struct SNewIdeState {
    struct SDriveState {
      struct {
	bool busy;
	bool drive_ready;
	bool fault;
	bool seek_complete;
	bool drq;
	bool bit_2;
	bool index_pulse;
	bool err;
	int index_pulse_count;
	
	// debugging
	u8 debug_last_status;
	bool debug_status_update;
      } status;
      
      struct {
	bool lba_mode;
	int features;
	int error;
	int sector_count;
	int sector_no;
	int cylinder_no;
	int head_no;
	int command;
      } registers;
      
      struct {
	bool command_in_progress;
	int current_command;
	int command_cycle;
	bool packet_dma;
	int packet_phase;
	u8 packet_command[12];
	int packet_buffersize;
      } command;
    };
    
    
    struct SControllerState {
      // the attached devices
      struct SDriveState drive[2];
      
      // control data.
      bool disable_irq;
      bool reset;
      
      // internal state
      bool reset_in_progress;
      bool interrupt_pending;
      int selected;

      // dma stuff
      u8 busmaster[8];
      u8 dma_mode;
      u8 bm_status;
      
      // pio stuff
#define IDE_BUFFER_SIZE 65536 // 64K words = 128K = 256 sectors @ 512 bytes
      u16 data[IDE_BUFFER_SIZE];
      int data_ptr;
      int data_size;
    } controller[2];

    
#define SEL_STATUS(a) state.controller[a].drive[state.controller[a].selected].status
#define SEL_COMMAND(a) state.controller[a].drive[state.controller[a].selected].command
#define SEL_REGISTERS(a) state.controller[a].drive[state.controller[a].selected].registers

#define SEL_DISK(a) get_disk(a,state.controller[a].selected)
#define SEL_PER_DRIVE(a) state.controller[a].drive[state.controller[a].selected]

#define STATUS(a,b) state.controller[a].drive[b].status
#define COMMAND(a,b) state.controller[a].drive[b].command
#define REGISTERS(a,b) state.controller[a].drive[b].registers
#define PER_DRIVE(a,b) state.controller[a].drive[b]
#define CONTROLLER(a) state.controller[a]



  } state;
};

extern CNewIde * theNewIde;


/* memory region ids */
#define PRI_COMMAND 1
#define PRI_CONTROL 2
#define SEC_COMMAND 3
#define SEC_CONTROL 4
#define PRI_BUSMASTER 5
#define SEC_BUSMASTER 6

/* bar IDs */
#define BAR_PRI_COMMAND 0
#define BAR_PRI_CONTROL 1
#define BAR_SEC_COMMAND 2
#define BAR_SEC_CONTROL 3
#define BAR_BUSMASTER 4

/* device registers */
#define REG_COMMAND_DATA 0
#define REG_COMMAND_ERROR 1
#define REG_COMMAND_FEATURES 1
#define REG_COMMAND_SECTOR_COUNT 2
#define REG_COMMAND_SECTOR_NO 3
#define REG_COMMAND_CYL_LOW 4
#define REG_COMMAND_CYL_HI 5
#define REG_COMMAND_DRIVE 6
#define REG_COMMAND_STATUS 7
#define REG_COMMAND_COMMAND 7

static char *register_names[] = {
  "DATA",
  "ERROR/FEATURES",
  "SECTOR_COUNT/PKT REASON",
  "SECTOR_NO",
  "CYL_LOW/PKT BYTE LOW",
  "CYL_HI/PKT BYTE HI",
  "DRIVE",
  "STATUS/COMMAND",
};




/* misc constants */



/* Packet Protocol Aliases */
#define DMRD fault
#define SERV seek_complete
#define CHK  err
#define BYTE_COUNT cylinder_no
#define REASON sector_count
#define IR_CD 0x01
#define IR_IO 0x02
#define IR_REL 0x04

/* Packet protocol states */
static char *packet_states[] = {
  "DP0: Prepare A",
  "DP1: Receive Packet",
  "DP2: Prepare B",
  "DP3/4: Ready INITRQ/Transfer Data",
  "DIx: Device Interrupt ",
};
#define PACKET_NONE 0
#define PACKET_DP0 0
#define PACKET_DP1 1
#define PACKET_DP2 2
#define PACKET_DP34 3
#define PACKET_DI  4




#endif 
