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
 * Contains the code for the emulated Symbios SCSI controller.
 *
 * X-1.3        Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.2        Camiel Vanderhoeven                             16-DEC-2007
 *      Changed register structure.
 *
 * X-1.1        Camiel Vanderhoeven                             14-DEC-2007
 *      Initial version in CVS.
 **/

#include "StdAfx.h"
#include "Sym53C895.h"
#include "System.h"
#include "Disk.h"

#define R_SCNTL0      0x00
#define R_SCNTL0_TRG        0x01
#define R_SCNTL0_START      0x20
#define   SCNTL0_MASK       0xFB

#define R_SCNTL1      0x01
#define R_SCNTL1_CON        0x10
#define R_SCNTL1_RST        0x08
#define R_SCNTL1_IARB       0x02

#define R_SCNTL2      0x02
#define R_SCNTL2_WSR        0x01
#define   SCNTL2_MASK       0xF2
#define   SCNTL2_W1C        0x09

#define R_SCNTL3      0x03
#define R_SCNTL3_EWS        0x08

#define R_SCID        0x04
#define R_SCID_ID           0x0F
#define   SCID_MASK         0x6F

#define R_SXFER       0x05

#define R_SDID        0x06
#define R_SDID_ID           0x0F
#define   SDID_MASK         0x0F

#define R_GPREG       0x07
#define   GPREG_MASK        0x1F

#define R_SFBR        0x08

#define R_SOCL        0x09
#define R_SOCL_ACK          0x40
#define R_SOCL_ATN          0x20

#define R_SBCL        0x0B
#define R_SBCL_REQ          0x80
#define R_SBCL_ACK          0x40
#define R_SBCL_BSY          0x20
#define R_SBCL_SEL          0x10
#define R_SBCL_ATN          0x08
#define R_SBCL_MSG          0x04
#define R_SBCL_CD           0x02
#define R_SBCL_IO           0x01
#define R_SBCL_PHASE        0x07

#define R_DSTAT       0x0C
#define R_DSTAT_MDPE        0x40
#define R_DSTAT_BF          0x20
#define R_DSTAT_ABRT        0x10
#define R_DSTAT_SSI         0x08
#define R_DSTAT_SIR         0x04
#define R_DSTAT_IID         0x01
#define   DSTAT_RC          0x7D
#define   DSTAT_FATAL       0x7D

#define R_SSTAT0      0x0D
#define R_SSTAT0_RST        0x02
#define R_SSTAT0_SDP0       0x01

#define R_SSTAT1      0x0E
#define R_SSTAT1_SDP1       0x01

#define R_SSTAT2      0x0F

#define R_DSA         0x10

#define R_ISTAT       0x14
#define R_ISTAT_ABRT        0x80
#define R_ISTAT_SRST        0x40
#define R_ISTAT_SIGP        0x20
#define R_ISTAT_SEM         0x10
#define R_ISTAT_CON         0x08
#define R_ISTAT_INTF        0x04
#define R_ISTAT_SIP         0x02
#define R_ISTAT_DIP         0x01
#define   ISTAT_MASK        0xF0
#define   ISTAT_W1C         0x04

#define R_CTEST0      0x18
#define R_CTEST1      0x19

#define R_CTEST2      0x1A
#define R_CTEST2_DDIR       0x80
#define R_CTEST2_SIGP       0x40
#define R_CTEST2_CIO        0x20
#define R_CTEST2_CM         0x10
#define R_CTEST2_SRTCH      0x08
#define   CTEST2_MASK       0x08

#define R_CTEST3      0x1B
#define R_CTEST3_REV        0xf0
#define R_CTEST3_FLF        0x08
#define R_CTEST3_CLF        0x04
#define R_CTEST3_FM         0x02
#define   CTEST3_MASK       0x0B

#define R_TEMP        0x1C
#define R_CTEST4      0x21

#define R_CTEST5      0x22
#define R_CTEST5_ADCK       0x80
#define R_CTEST5_BBCK       0x40
#define   CTEST5_MASK       0x3F

#define R_DBC         0x24
#define R_DCMD        0x27
#define R_DNAD        0x28
#define R_DSP         0x2C
#define R_DSPS        0x30
#define R_SCRATCHA    0x34

#define R_DMODE       0x38
#define R_DMODE_MAN         0x01

#define R_DIEN        0x39
#define   DIEN_MASK         0x7D

#define R_SBR         0x3A

#define R_DCNTL       0x3B
#define R_DCNTL_SSM         0x10
#define R_DCNTL_STD         0x04
#define R_DCNTL_IRQD        0x02
#define R_DCNTL_COM         0x01
#define   DCNTL_MASK        0xFB 

#define R_ADDER       0x3C
#define R_SIEN0       0x40
#define R_SIEN1       0x41

#define   SIEN1_MASK        0x17

#define R_SIST0       0x42
#define   SIST0_RC          0xFF

#define R_SIST1       0x43
#define   SIST1_RC          0x17

#define R_GPCNTL      0x47
#define R_STIME0      0x48
#define R_STIME1      0x49
#define   STIME1_MASK       0x7F

#define R_RESPID      0x4A

#define R_STEST0      0x4C
#define R_STEST1      0x4D
#define   STEST1_MASK       0xCC

#define R_STEST2      0x4E
#define R_STEST2_SCE        0x80
#define R_STEST2_ROF        0x40
#define R_STEST2_DIF        0x20
#define R_STEST2_SLB        0x10
#define R_STEST2_SZM        0x08
#define R_STEST2_AWS        0x04
#define R_STEST2_EXT        0x02
#define R_STEST2_LOW        0x01
#define   STEST2_MASK       0xBF

#define R_STEST3      0x4F
#define R_STEST3_TE         0x80
#define R_STEST3_STR        0x40
#define R_STEST3_HSC        0x20
#define R_STEST3_DSI        0x10
#define R_STEST3_S16        0x08
#define R_STEST3_TTM        0x04
#define R_STEST3_CSF        0x02
#define R_STEST3_STW        0x01
#define   STEST3_MASK       0xFF

#define R_STEST4      0x52

#define R_SBDL        0x58
#define R_SCRATCHB    0x5C
#define R_SCRATCHC    0x60

#define R8(a)  state.regs.reg8[R_##a]
#define R16(a) state.regs.reg16[R_##a/2]
#define R32(a) state.regs.reg32[R_##a/4]

// test bit in register
#define TB_R8(a,b) ((R8(a) & R_##a##_##b) == R_##a##_##b)
//set bit in register
#define SB_R8(a,b,c) R8(a) = (R8(a) & ~R_##a##_##b) | (c ? R_##a##_##b : 0)
// write with mask
#define WRM_R8(a,b)     R8(a) = (R8(a) & ~a##_MASK)            | ((b) & a##_MASK)
// write with mask and write-1-to-clear
#define WRMW1C_R8(a,b)  R8(a) = (R8(a) & ~a##_MASK & ~a##_W1C) | ((b) & a##_MASK) | (R8(a) & ~(b) & a##_W1C)

#define RDCLR_R8(a) R8(a) &= ~a##_RC

#define GET_DEST()  (R8(SDID) & R_SCID_ID)
#define SET_DEST(a) R8(SDID) = (a) & R_SCID_ID

#define GET_DBC() (R32(DBC) & 0x00ffffff)
#define SET_DBC(a) R32(DBC) = (R32(DBC) & 0xff000000) | ((a) & 0x00ffffff)

#define PT state.per_target[GET_DEST()]
#define PTD get_disk(0,GET_DEST())

u32 sym_cfg_data[64] = {
/*00*/  0x000c1000, // CFID: vendor + device
/*04*/  0x02000001, // CFCS: command + status
/*08*/  0x01000000, // CFRV: class + revision
/*0c*/  0x00000000, // CFLT: latency timer + cache line size
/*10*/  0x00000001, // BAR0: IO Space
/*14*/  0x00000000, // BAR1: Memory space
/*18*/  0x00000000, // BAR2: RAM space
/*1c*/  0x00000000, // BAR3: 
/*20*/  0x00000000, // BAR4: 
/*24*/  0x00000000, // BAR5: 
/*28*/  0x00000000, // CCIC: CardBus
/*2c*/  0x00000000, // CSID: subsystem + vendor
/*30*/  0x00000000, // BAR6: expansion rom base
/*34*/  0x00000000, // CCAP: capabilities pointer
/*38*/  0x00000000,
/*3c*/  0x401101ff, // CFIT: interrupt configuration
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

u32 sym_cfg_mask[64] = {
/*00*/  0x00000000, // CFID: vendor + device
/*04*/  0x00000157, // CFCS: command + status
/*08*/  0x00000000, // CFRV: class + revision
/*0c*/  0x0000ffff, // CFLT: latency timer + cache line size
/*10*/  0xffffff00, // BAR0: IO space (256 bytes)
/*14*/  0xffffff00, // BAR1: Memory space (256 bytes)
/*18*/  0xfffff000, // BAR2: RAM space (4KB)
/*1c*/  0x00000000, // BAR3: 
/*20*/  0x00000000, // BAR4: 
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

/**
 * Constructor.
 **/

CSym53C895::CSym53C895(CConfigurator * cfg, CSystem * c, int pcibus, int pcidev) 
  : CDiskController(cfg,c,pcibus,pcidev,1,16)
{
  add_function(0,sym_cfg_data, sym_cfg_mask);

  ResetPCI();

  c->RegisterClock(this, true);

  state.executing = false;
  state.wait_reselect = false;
  state.irq_asserted = false;
  memset(state.regs.reg32,0,sizeof(state.regs.reg32));
  R8(CTEST3) = (u8)(pci_state.config_data[0][2]<<4) & R_CTEST3_REV; // Chip rev.
  R8(STEST4) = 0xC0 | 0x20; // LVD SCSI, Freq. Locked

  printf("%%SYM-I-INIT: Symbios 53c895 emulator initialized. STEST4 = %02x.\n",R8(STEST4));
}

CSym53C895::~CSym53C895()
{
}

static u32 sym_magic1 = 0x53C895CC;
static u32 sym_magic2 = 0xCC53C895;

/**
 * Save state to a Virtual Machine State file.
 **/

int CSym53C895::SaveState(FILE *f)
{
  long ss = sizeof(state);
  int res;

  if (res = CPCIDevice::SaveState(f))
    return res;

  fwrite(&sym_magic1,sizeof(u32),1,f);
  fwrite(&ss,sizeof(long),1,f);
  fwrite(&state,sizeof(state),1,f);
  fwrite(&sym_magic2,sizeof(u32),1,f);
  printf("%s: %d bytes saved.\n",devid_string,ss);
  return 0;
}

/**
 * Restore state from a Virtual Machine State file.
 **/

int CSym53C895::RestoreState(FILE *f)
{
  long ss;
  u32 m1;
  u32 m2;
  int res;
  size_t r;

  if (res = CPCIDevice::RestoreState(f))
    return res;

  r = fread(&m1,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (m1 != sym_magic1)
  {
    printf("%s: MAGIC 1 does not match!\n",devid_string);
    return -1;
  }

  fread(&ss,sizeof(long),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (ss != sizeof(state))
  {
    printf("%s: STRUCT SIZE does not match!\n",devid_string);
    return -1;
  }

  fread(&state,sizeof(state),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }

  r = fread(&m2,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (m2 != sym_magic2)
  {
    printf("%s: MAGIC 1 does not match!\n",devid_string);
    return -1;
  }

  printf("%s: %d bytes restored.\n",devid_string,ss);
  return 0;
}

void CSym53C895::WriteMem_Bar(int func,int bar, u32 address, int dsize, u32 data)
{
  void * p;

  switch (bar)
  {
    case 0:
    case 1:
      address &= 0x7f;
      switch(dsize)
      {
      case 8:
        if (address>=R_SCRATCHC)
        {
          state.regs.reg8[address] = (u8)data;
          break;
        }
        switch (address)
        {
        // SIMPLE CASES: JUST WRITE
        case R_SXFER:   // 05
        case R_DSA:     // 10
        case R_DSA+1:   // 11
        case R_DSA+2:   // 12
        case R_DSA+3:   // 13
        case R_CTEST0:  // 18
        case R_TEMP:    // 1C
        case R_TEMP+1:  // 1D
        case R_TEMP+2:  // 1E
        case R_TEMP+3:  // 1F
        case R_DSP:     // 2C
        case R_DSP+1:   // 2D
        case R_DSP+2:   // 2E
        case R_DSPS:    // 30
        case R_DSPS+1:  // 31
        case R_DSPS+2:  // 32
        case R_DSPS+3:  // 33
        case R_SCRATCHA:// 34
        case R_SCRATCHA+1:// 35
        case R_SCRATCHA+2:// 36
        case R_SCRATCHA+3:// 37
        case R_DMODE:   // 38
        case R_GPCNTL:  // 47
        case R_STIME0:  // 48
        case R_RESPID:  // 4A
        case R_RESPID+1:// 4B
        case R_STEST0:  // 4C
        case R_SCRATCHB:// 5C
        case R_SCRATCHB+1:// 5D
        case R_SCRATCHB+2:// 5E
        case R_SCRATCHB+3:// 5F
          state.regs.reg8[address] = (u8)data;
          break;

        case R_SCNTL0:  // 00
          // side effects: start arbitration bit
          write_b_scntl0((u8)data);
          break;
        case R_SCNTL1:  //01
          // side effects: start immediate arbitration bit
          write_b_scntl1((u8)data);
          break;
        case R_SCNTL2:  //02
          WRMW1C_R8(SCNTL2,(u8)data);
          break;
        case R_SCNTL3:  //03
          // side effects: clearing EWS
          write_b_scntl3((u8)data);
          break;
        case R_SCID:    // 04
          WRM_R8(SCID,(u8)data);
          break;
        case R_SDID:    // 06
          WRM_R8(SDID,(u8)data);
          break;
        case R_GPREG:   // 07
          WRM_R8(GPREG,(u8)data);
          break;
        case R_ISTAT:  // 14
          write_b_istat((u8)data);
          break;
        case R_CTEST2: // 1A
          WRM_R8(CTEST2,(u8)data);
          break;
        case R_CTEST3: // 1B
          write_b_ctest3((u8)data);
          break;
        case R_CTEST4:  // 21
          write_b_ctest4((u8)data);
          break;
        case R_CTEST5:  // 22
          write_b_ctest5((u8)data);
          break;
        case R_DSP+3:   // 2F
          state.regs.reg8[address] = (u8)data;
          post_dsp_write();
          break;
        case R_DIEN:    // 39
          WRM_R8(DIEN,(u8)data);
          eval_interrupts();
          break;
        case R_DCNTL: // 3B
          write_b_dcntl((u8)data);
          break;
        case R_SIEN0:    // 40
          R8(SIEN0) = (u8)data;
          eval_interrupts();
          break;
        case R_SIEN1:    // 41
          WRM_R8(SIEN1,(u8)data);
          eval_interrupts();
          break;
        case R_STIME1:  // 49
          WRM_R8(STIME1,(u8)data);
          break;
        case R_STEST1:  // 4D
          WRM_R8(STEST1,(u8)data);
          break;
        case R_STEST2:  // 4E
          write_b_stest2((u8)data);
          break;
        case R_STEST3:  // 4F
          write_b_stest3((u8)data);
          break;

        case R_DSTAT:   // 0C
        case R_SSTAT0:  // 0D
        case R_SSTAT1:  // 0E
        case R_SSTAT2:  // 0F
          printf("SYM: Write to read-only memory at %02x. FreeBSD driver cache test.\n" ,address,data);
          break;
        default:
          printf("SYM: Write 8 bits to unknown memory at %02x with %08x.\n",address,data);
          exit(1);
        }
        break;
      case 16:
        WriteMem_Bar(0,1,address+0,8,(data>>0) & 0xff);
        WriteMem_Bar(0,1,address+1,8,(data>>8) & 0xff);
        break;
      case 32:
        WriteMem_Bar(0,1,address+0,8,(data>> 0) & 0xff);
        WriteMem_Bar(0,1,address+1,8,(data>> 8) & 0xff);
        WriteMem_Bar(0,1,address+2,8,(data>>16) & 0xff);
        WriteMem_Bar(0,1,address+3,8,(data>>24) & 0xff);
        break;
      }
      break;
    case 2:
      p = (u8*)state.ram + address;
      switch(dsize)
      {
      case 8:
        *((u8 *) p) = (u8) data;
        break;
      case 16:
        *((u16 *) p) = (u16) data;
        break;
      case 32:
        *((u32 *) p) = (u32) data;
        break;
      }
      break;
  }

  if (R8(STEST4)==0)
  {
    printf("STEST4 0 after write 8 bits to unknown memory at %02x with %08x.\n",address,data);
    getchar();
  }

}

u32 CSym53C895::ReadMem_Bar(int func,int bar, u32 address, int dsize)
{
  u32 data = 0;
  void * p;

  switch (bar)
  {
    case 0:
    case 1:
      address &= 0x7f;
      switch(dsize)
      {
      case 8:
        if (address>=R_SCRATCHC)
        {
            data = state.regs.reg8[address];
            break;
        }
        switch(address)
        {
        case R_SCNTL0:  // 00
        case R_SCNTL1:  // 01
        case R_SCNTL2:  // 02
        case R_SCNTL3:  // 03
        case R_SCID:    // 04
        case R_SXFER:   // 05
        case R_SDID:    // 06
        case R_GPREG:   // 07
        case R_SFBR:    // 08
        case R_SBCL:    // 0B
        case R_SSTAT0:  // 0D
        case R_SSTAT1:  // 0E
        case R_SSTAT2:  // 0F
        case R_DSA:     // 10
        case R_DSA+1:   // 11
        case R_DSA+2:   // 12
        case R_DSA+3:   // 13
        case R_ISTAT:   // 14
        case R_CTEST0:  // 18
        case R_CTEST1:  // 19
        case R_CTEST3:  // 1B
        case R_TEMP:    // 1C
        case R_TEMP+1:  // 1D
        case R_TEMP+2:  // 1E
        case R_TEMP+3:  // 1F
        case R_CTEST4:  // 21
        case R_CTEST5:  // 22
        case R_DSP:     // 2C
        case R_DSP+1:   // 2D
        case R_DSP+2:   // 2E
        case R_DSP+3:   // 2F
        case R_DSPS:    // 30
        case R_DSPS+1:  // 31
        case R_DSPS+2:  // 32
        case R_DSPS+3:  // 33
        case R_DMODE:   // 38
        case R_DIEN:    // 39
        case R_DCNTL:   // 3B
        case R_SIEN0:   // 40
        case R_SIEN1:   // 41
        case R_GPCNTL:  // 47
        case R_STIME0:  // 48
        case R_STIME1:  // 49
        case R_RESPID:  // 4A
        case R_RESPID+1:// 4B
        case R_STEST0:  // 4C
        case R_STEST1:  // 4D
        case R_STEST2:  // 4E
        case R_STEST3:  // 4F
        case R_STEST4:  // 52
        case R_SBDL:    // 58
        case R_SBDL+1:  // 59
          data = state.regs.reg8[address];
          break;

        case R_DSTAT:   // 0C
          data = read_b_dstat();
          break;

        case R_CTEST2:  // 1A
          data = read_b_ctest2();
          break;

        case R_SCRATCHA:    // 34
        case R_SCRATCHA+1:  // 35
        case R_SCRATCHA+2:  // 36
        case R_SCRATCHA+3:  // 37
          data = read_b_scratcha(address-R_SCRATCHA);
          break;

        case R_SIST0:   // 42
        case R_SIST1:   // 43
          data = read_b_sist(address-R_SIST0);
          break;

        case R_SCRATCHB:    // 5C
        case R_SCRATCHB+1:  // 5D
        case R_SCRATCHB+2:  // 5E
        case R_SCRATCHB+3:  // 5F
          data = read_b_scratchb(address-R_SCRATCHB);
          break;

        default:
          printf("SYM: Attempt to read %d bits from memory at %02x\n", dsize, address);
          exit(1);
        }
        break;
      case 16:
        data  = (ReadMem_Bar(0,1,address+0,8)<<0) & 0x00ff;
        data |= (ReadMem_Bar(0,1,address+1,8)<<8) & 0xff00;
        break;
      case 32:
        data  = (ReadMem_Bar(0,1,address+0,8)<< 0) & 0x000000ff;
        data |= (ReadMem_Bar(0,1,address+1,8)<< 8) & 0x0000ff00;
        data |= (ReadMem_Bar(0,1,address+2,8)<<16) & 0x00ff0000;
        data |= (ReadMem_Bar(0,1,address+3,8)<<24) & 0xff000000;
        break;
      }
      break;
    case 2:
      p = (u8*)state.ram + address;
      switch(dsize)
      {
      case 8:
        return *((u8 *) p);
      case 16:
        return *((u16 *) p);
      case 32:
        return *((u32 *) p);
      }
      break;
  }

  return data;

}

u32 CSym53C895::config_read_custom(int func, u32 address, int dsize, u32 data)
{
  if (address>=0x80)
    return ReadMem_Bar(func,1,address-0x80,dsize);
  else
    return data;
}

void CSym53C895::config_write_custom(int func, u32 address, int dsize, u32 old_data, u32 new_data, u32 data)
{
  if (address>=0x80)
    WriteMem_Bar(func,1,address-0x80,dsize,data);
}

void CSym53C895::write_b_scntl0(u8 value)
{
  bool old_start = TB_R8(SCNTL0,START);

  WRM_R8(SCNTL0,value);

  if (TB_R8(SCNTL0,START) && !old_start)
  {
    printf("SYM: Don't know how to start arbitration sequence.\n");
    exit(1);
  }

  if (TB_R8(SCNTL0,TRG))
  {
    printf("SYM: Don't know how to operate in target mode.\n");
    exit(1);
  }
}

void CSym53C895::write_b_scntl1(u8 value)
{
  bool old_iarb = TB_R8(SCNTL1,IARB);
  bool old_con = TB_R8(SCNTL1,CON);
  bool old_rst = TB_R8(SCNTL1,RST);

  R8(SCNTL1) = value;

  if (TB_R8(SCNTL1,CON) != old_con)
    printf("SYM: Don't know how to forcibly connect or disconnect\n");

  if (TB_R8(SCNTL1,RST) != old_rst)
  {
    SB_R8(SSTAT0,SDP0,false);
    SB_R8(SSTAT1,SDP1,false);
    R16(SBDL)=0;
    R8(SBCL)=0;

    SB_R8(SSTAT0,RST,!old_rst);

    printf("SYM: %s SCSI bus reset.\n",old_rst?"end":"start");
  }

  if (TB_R8(SCNTL1,IARB) && !old_iarb)
  {
    printf("SYM: Don't know how to start immediate arbitration sequence.\n");
    exit(1);
  }
}

void CSym53C895::write_b_scntl3(u8 value)
{
  R8(SCNTL3) = value;

  if (!TB_R8(SCNTL3,EWS))
    SB_R8(SCNTL2,WSR,false);
}

void CSym53C895::write_b_istat(u8 value)
{
  bool old_srst = TB_R8(ISTAT,SRST);

  WRMW1C_R8(ISTAT, value);

  if (TB_R8(ISTAT,ABRT))
  {
    printf("SYM: Aborting on request.\n");
    set_interrupt(R_DSTAT,R_DSTAT_ABRT);
  }

  if (TB_R8(ISTAT,ABRT) && !old_srst)
    printf("SYM: Don't know how to initiate a reset yet!\n");

  if (TB_R8(ISTAT,SIGP))
  {
    if (state.wait_reselect)
    {
      R32(DSP) = state.wait_jump;
      state.wait_reselect = false;
      state.executing = true;
    }
  }

  eval_interrupts();
}

u8 CSym53C895::read_b_ctest2()
{
  SB_R8(CTEST2, CIO,  pci_state.config_data[0][4]!=0);
  SB_R8(CTEST2, CM,   pci_state.config_data[0][5]!=0);
  SB_R8(CTEST2, SIGP, TB_R8(ISTAT, SIGP));
  SB_R8(ISTAT,  SIGP, false);

  return R8(CTEST2);
}

void CSym53C895::write_b_ctest3(u8 value)
{
  WRM_R8(CTEST3, value);

  if ((value>>3) & 1)
    printf("SYM: Don't know how to flush DMA FIFO\n");

  if ((value>>2) & 1)
    printf("SYM: Don't know how to clear DMA FIFO\n");

  if ((value>>1) & 1)
  {
    printf("SYM: Don't know how to handle FM mode\n");
    exit(1);
  }
}

void CSym53C895::write_b_ctest4(u8 value)
{
  R8(CTEST4) = value;

  if ((value>>4) & 1)
  {
    printf("SYM: Don't know how to handle SRTM mode\n");
    exit(1);
  }
}


void CSym53C895::write_b_ctest5(u8 value)
{
  WRM_R8(CTEST5,value);

  if ((value>>7) & 1)
    printf("SYM: Don't know how to do Clock Address increment.\n");
  if ((value>>6) & 1)
    printf("SYM: Don't know how to do Clock Byte Counter decrement.\n");
}

u8 CSym53C895::read_b_dstat()
{
  u8 retval = R8(DSTAT);

  RDCLR_R8(DSTAT);

  R8(DSTAT) |= state.dstat_stack;

  state.dstat_stack = 0;

  printf("Read DSTAT --> eval int\n");
  eval_interrupts();

  printf("Read DSTAT <-- eval int; retval: %02x; dstat: %02x.\n",retval,R8(DSTAT));

  return retval;
}

u8 CSym53C895::read_b_sist(int id)
{
  u8 retval = state.regs.reg8[R_SIST0+id];

  if (id)
    RDCLR_R8(SIST1);
  else
    RDCLR_R8(SIST0);

  if (!R8(SIST0) && !R8(SIST1))
  {
     R8(SIST0) |= state.sist0_stack;
     R8(SIST1) |= state.sist1_stack;
     state.sist0_stack = 0;
     state.sist1_stack = 0;
  }
  eval_interrupts();

  return retval;
}


void CSym53C895::write_b_dcntl(u8 value)
{
  WRM_R8(DCNTL,value);

  // start operation
  if (value & R_DCNTL_STD)
    state.executing = true;

  //IRQD bit...
  eval_interrupts();
}

u8 CSym53C895::read_b_scratcha(int reg)
{
  if (TB_R8(CTEST2,SRTCH))
  {
    printf("SYM: SCRATCHA from PCI\n");
    return (pci_state.config_data[0][4]>>(reg*8)) & 0xff;
  }
  else
    return state.regs.reg8[R_SCRATCHA+reg];
}

u8 CSym53C895::read_b_scratchb(int reg)
{
  if (TB_R8(CTEST2,SRTCH))
  {
    printf("SYM: SCRATCHB from PCI\n");
    return (pci_state.config_data[0][5]>>(reg*8)) & 0xff;
  }
  else
    return state.regs.reg8[R_SCRATCHB+reg];
}

void CSym53C895::write_b_stest2(u8 value)
{
  WRM_R8(STEST2,value);

  if (value & R_STEST2_ROF)
    printf("SYM: Don't know how to reset SCSI offset!\n");

  if (TB_R8(STEST2,LOW))
  {
    printf("SYM: I don't like LOW level mode!\n");
    exit(1);
  }
}

void CSym53C895::write_b_stest3(u8 value)
{
  WRM_R8(STEST3,value);
  if (value & R_STEST3_CSF)
    printf("SYM: Don't know how to clear the SCSI fifo.\n");
}

void CSym53C895::post_dsp_write()
{
  if (!TB_R8(DMODE,MAN))
    state.executing = true;
}

int CSym53C895::DoClock()
{
  u64 cmda0;
  u64 cmda1;

  int optype;

  if (state.executing)
  {

    // single step mode
    if (TB_R8(DCNTL,SSM))
    {
      printf("SYM: Single step...\n");
      set_interrupt(R_DSTAT,R_DSTAT_SSI);
    }

    //printf("SYM: EXECUTING SCRIPT\n");
    //printf("SYM: INS @ %x, %x   \n",R32(DSP), R32(DSP)+4);

    cmda0 = cSystem->PCI_Phys(myPCIBus, R32(DSP));
    cmda1 = cSystem->PCI_Phys(myPCIBus, R32(DSP) + 4);

    R32(DSP) += 8;

    //printf("SYM: INS @ %" LL "x, %" LL "x   \n",cmda0, cmda1);

    R32(DBC) = cSystem->ReadMem(cmda0,32);  // loads both DBC and DCMD
    R32(DSPS) = cSystem->ReadMem(cmda1,32);

    //printf("SYM: INS = %x, %x, %x   \n",R8(DCMD), GET_DBC(), R32(DSPS));

    optype = (R8(DCMD)>>6) & 3;
    switch(optype)
    {
    case 0:
      {
        bool indirect = (R8(DCMD)>>5) & 1;
        bool table_indirect = (R8(DCMD)>>4) & 1;
        int opcode = (R8(DCMD)>>3) & 1;
        int scsi_phase = (R8(DCMD)>>0) & 7;
        printf("SYM: INS = Block Move (i %d, t %d, opc %d, phase %d\n",indirect,table_indirect,opcode,scsi_phase);

        R32(DNAD) = R32(DSPS);
        if (state.phase == scsi_phase)
        {
          printf("SYM: Ready for transfer.\n");

          u32 start;
          u32 count;

          int i;
          if (table_indirect)
          {
            u32 add = R32(DSA) + sext_32(R32(DSPS),24);

            cmda0 = cSystem->PCI_Phys(myPCIBus,add);
            cmda1 = cSystem->PCI_Phys(myPCIBus,add+4);
            printf("SYM: Reading table from system at %" LL "x, % "LL "x\n",cmda0,cmda1);
            count = cSystem->ReadMem(cmda0,32);
            start = cSystem->ReadMem(cmda1,32);
            printf("SYM: Start/count %x, %x\n",start,count);
          }
          else if (indirect)
          {
            printf("SYM: Unsupported: direct addressing\n");
            exit(1);
          }
          else
          {
            start = R32(DSPS);
            count = GET_DBC();
          }
          R32(DNAD) = start;
          if (state.phase == 0 && PT.dato_to_disk)
          {
            cmda0 = cSystem->PCI_Phys(myPCIBus,R32(DNAD));
            void * dptr = cSystem->PtrToMem(cmda0);
            PTD->write_blocks(dptr,PT.dato_len);
            PT.dato_len = 0;
            R32(DNAD) +=(PT.dato_len*512);
          }
          else if (state.phase == 1 && PT.dati_off_disk)
          {
            cmda0 = cSystem->PCI_Phys(myPCIBus,R32(DNAD));
            void * dptr = cSystem->PtrToMem(cmda0);
            PTD->read_blocks(dptr,PT.dati_len);
            PT.dati_len = 0;
            R32(DNAD) +=(PT.dati_len*512);
          }
          else
          {
            printf("SYM: ");
            for (i=0;i<count;i++)
            {
              u8 dat;
              cmda0 = cSystem->PCI_Phys(myPCIBus,R32(DNAD)++);
              if (state.phase>=0)
              {
                if (state.phase & 1)
                {
                  dat = byte_from_target();
                  printf("%02x ",dat);
//                  printf("SYM: Reading byte %02x\n",dat);
//                  printf("SYM: Writing byte to system @ %" LL "x\n",cmda0);
                  cSystem->WriteMem(cmda0,8,dat);
                }
                else
                {
//                  printf("SYM: Reading a byte from system at %x\n",cmda0);
                  dat = cSystem->ReadMem(cmda0,8);
                  printf("%02x ",dat);
//                  printf("SYM: Writing byte %02x\n",dat);
                  byte_to_target(dat);
                }
              }
            }
            printf("\n");
          }
          end_xfer();
          return 0;
        }
      }
      break;
    case 1:
      {
        int opcode = (R8(DCMD)>>3) & 7;

        if (opcode < 5)
        {
          bool relative = (R8(DCMD)>>2) & 1;
          bool table_indirect = (R8(DCMD)>>1) & 1;
          bool atn = (R8(DCMD)>>0) & 1;
          int destination = (GET_DBC()>>16) & 0x0f;
          bool sc_carry = (GET_DBC()>>10) & 1;
          bool sc_target = (GET_DBC()>>9) & 1;
          bool sc_ack = (GET_DBC()>>6) & 1;
          bool sc_atn = (GET_DBC()>>3) & 1;

          //HACK?? DOCS UNCLEAR: TRY-THIS
          R32(DNAD) = R32(DSPS);

          u32 dest_addr = R32(DNAD);

          if (relative)
            dest_addr = R32(DSP) + sext_32(R32(DNAD),24);

          printf("SYM: INS = I/O (opc %d, r %d, t %d, a %d, dest %d, sc %d%d%d%d\n"
            ,opcode,relative,table_indirect,atn,destination,sc_carry,sc_target,sc_ack,sc_atn);

          switch(opcode)
          {
          case 0:
            printf("SYM: OPC = Select\n");
            printf("SYM: Selecting target %d.\n", destination);
            select_target(destination);
            return 0;
          case 1:
            printf("SYM: OPC = Wait_Disconnect\n");
            // maybe we need to do more??
            state.phase = -1;
            return 0;

          case 2:
            printf("SYM: OPC = Wait_Reselect\n");
            //reselect never happens for now...
            if (TB_R8(ISTAT,SIGP))
              R32(DSP) = dest_addr;
            else
            {
              state.wait_reselect = true;
              state.wait_jump = dest_addr;
              state.executing = false;
            }
            return 0;

          case 3:
            printf("SYM: OPC = Set %s%s%s%s\n",sc_carry?"carry ":"",sc_target?"target ":"",sc_ack?"ack ":"",sc_atn?"atn ":"");
            if (sc_ack) SB_R8(SOCL,ACK,true);
            if (sc_atn) SB_R8(SOCL,ATN,true);
            if (sc_target) SB_R8(SCNTL0,TRG,true);
            if (sc_carry) state.alu.carry = true;
            return 0;
          case 4:
            printf("SYM: OPC = Clear %s%s%s%s\n",sc_carry?"carry ":"",sc_target?"target ":"",sc_ack?"ack ":"",sc_atn?"atn ":"");
            if (sc_ack) SB_R8(SOCL,ACK,false);
            if (sc_atn) SB_R8(SOCL,ATN,false);
            if (sc_target) SB_R8(SCNTL0,TRG,false);
            if (sc_carry) state.alu.carry = false;
            return 0;

            break;
          }
        }
        else
        {
          int oper = (R8(DCMD)>>0) & 7;
          bool use_data8_sfbr = (GET_DBC()>>23) & 1;
          int reg_address = ((GET_DBC()>>16) & 0x7f); //| (GET_DBC() & 0x80); // manual is unclear about bit 7.
          u8 imm_data = (GET_DBC()>>8) & 0xff;
          u8 op_data;

          printf("SYM: INS = R/W (opc %d, oper %d, use %d, add %d, imm %02x\n"
            ,opcode,oper,use_data8_sfbr,reg_address,imm_data);

          if (use_data8_sfbr)
            imm_data = R8(SFBR);

          if (oper!=0)
          {
            if (opcode==5)
            {
              op_data = R8(SFBR);
              printf("SYM: sfbr (%02x) ",op_data);
            }
            else
            {
              op_data = ReadMem_Bar(0,1,reg_address,8);
              printf("SYM: reg%02x (%02x) ",reg_address,op_data);
            }
          }

          switch(oper)
          {
          case 0:
            op_data = imm_data;
            printf("SYM: %02x ",imm_data);
            break;
          case 1:
            op_data <<= 1;
            printf("<< 1 = %02x ",op_data);
            break;
          case 2:
            op_data |= imm_data;
            printf("| %02x = %02x ",imm_data,op_data);
            break;
          case 3:
            op_data ^= imm_data;
            printf("^ %02x = %02x ",imm_data,op_data);
            break;
          case 4:
            op_data &= imm_data;
            printf("& %02x = %02x ",imm_data,op_data);
            break;
          case 5:
            op_data >>= 1;
            printf(">> 1 = %02x ",op_data);
            break;
          case 6:
            op_data += imm_data;
            printf("+ %02x = %02x ",imm_data,op_data);
            break;
          case 7:
            if ((u16)op_data + (u16)imm_data > 0xff)
              state.alu.carry = true;
            else
              state.alu.carry = false;
            op_data += imm_data;
            printf("+ %02x = %02x (carry %d) ",imm_data,op_data,state.alu.carry);
            break;
          }

          if (opcode==6)
          {
            printf("-> sfbr.\n");
            R8(SFBR) = op_data;
          }
          else
          {
            printf("-> reg%02x.\n",reg_address);
            WriteMem_Bar(0,1,reg_address,8,op_data);
          }

          return 0;

        }
      }
      break;
    case 2:
      {
        int opcode = (R8(DCMD)>>3) & 7;
        int scsi_phase = (R8(DCMD)>>0) & 7;
        bool relative = (GET_DBC()>>23) & 1;
        bool carry_test = (GET_DBC()>>21) & 1;
        bool interrupt_fly = (GET_DBC()>>20) & 1;
        bool jump_if = (GET_DBC()>>19) & 1;
        bool cmp_data = (GET_DBC()>>18) & 1;
        bool cmp_phase = (GET_DBC()>>17) & 1;
        // wait_valid can be safely ignored, phases are always valid in this ideal world...
        // bool wait_valid = (GET_DBC()>>16) & 1;
        int cmp_mask = (GET_DBC()>>8) & 0xff;
        int cmp_dat = (GET_DBC()>>0) & 0xff;

        u32 dest_addr = R32(DSPS);

        bool do_it;

        if (relative)
          dest_addr = R32(DSP) + sext_32(R32(DSPS),24);

        printf("SYM: if (");
        if (carry_test)
        {
          printf("(%scarry)",jump_if?"":"!");
          do_it = (state.alu.carry == jump_if);
        }
        else if (cmp_data || cmp_phase)
        {
          do_it = true;
          if (cmp_data)
          {
            printf("((data & 0x%02x) %s 0x%02x)", (~cmp_mask) & 0xff, jump_if?"==":"!=", cmp_dat &~cmp_mask);
            if (((R8(SFBR) & ~cmp_mask)==(cmp_dat & ~cmp_mask)) != jump_if)
              do_it = false;
            if (cmp_phase)
              printf(" && ");
          }
          if (cmp_phase)
          {
            printf("(phase %s %d)",jump_if?"==":"!=", scsi_phase);
            if ((state.phase==scsi_phase) != jump_if)
              do_it = false;
          }
        }
        else
        {
          // no comparison
          do_it = jump_if;
        }

        printf(")\n");
        printf("SYM:   ");
        switch(opcode)
        {
        case 0:
          printf("jump %x\n",R32(DSPS));
          if (do_it)
          {
            printf("SYM: Jumping...\n");
            R32(DSP) = dest_addr;
          }
          return 0;
          break;
        case 1:
          printf("call %d\n",R32(DSPS));
          if (do_it)
          {
            printf("SYM: Calling...\n");
            R32(TEMP) = R32(DSP);
            R32(DSP) = dest_addr;
          }
          return 0;
          break;
        case 2:
          printf("return %d\n",R32(DSPS));
          if (do_it)
          {
            printf("SYM: Returning...\n");
            R32(DSP) = R32(TEMP);
          }
          return 0;
          break;
        case 3:
          printf("interrupt%s.\n",interrupt_fly?" on the fly":"");
          if (do_it)
          {
            printf("SYM: Interrupting...\n");

            if (interrupt_fly)
              set_interrupt(R_ISTAT,R_ISTAT_INTF);
            else
              set_interrupt(R_DSTAT,R_DSTAT_SIR);
          }
          return 0;
          break;
        default:
          printf("SYM: Transfer Control Instruction with opcode %d is RESERVED.\n");
          exit(1);
        }
      }
    case 3:
      {
        bool is_load = (R8(DCMD)>>0) & 1;
        bool no_flush = (R8(DCMD)>>1) & 1;
        bool dsa_relative = (R8(DCMD)>>4) & 1;
        int regaddr = (GET_DBC()>>16) & 0x7f;
        int byte_count = (GET_DBC()>>0) & 7;
        u32 memaddr;

        if (dsa_relative)
          memaddr = R32(DSA) + sext_32(R32(DSPS),24);
        else
          memaddr = R32(DSPS);

        if (is_load)
        {
          printf("SYM: Load reg%02x", regaddr);
          if(byte_count>1)
            printf("..%02x", regaddr+byte_count-1);
          printf("from %x.\n",memaddr);
          for (int i=0; i<byte_count;i++)
          {
            u64 ma = cSystem->PCI_Phys(myPCIBus,memaddr+i);
            u8 dat = cSystem->ReadMem(ma,8);
            printf("SYM: %02x -> reg%02x\n",dat,regaddr+i);
            WriteMem_Bar(0,1,regaddr+i,8,dat);
          }
        }
        else
        {
          printf("SYM: Store reg%02x", regaddr);
          if(byte_count>1)
            printf("..%02x", regaddr+byte_count-1);
          printf("to %x.\n",memaddr);
          for (int i=0; i<byte_count;i++)
          {
            u64 ma = cSystem->PCI_Phys(myPCIBus,memaddr+i);
            u8 dat = ReadMem_Bar(0,1,regaddr+i,8);
            printf("SYM: %02x <- reg%02x\n",dat,regaddr+i);
            cSystem->WriteMem(ma,8,dat);
          }
        }
        return 0;
      }
      break;
    }
    return 1;
  }

  return 0;
}

void CSym53C895::select_target(int target)
{
  SET_DEST(target);
  if (PTD)
  {
    state.phase = 6; // message out
    PT.msgo_len = 0;
    PT.msgi_len = 0;
    PT.cmd_len = 0;
    PT.dati_len = 0;
    PT.dato_len = 0;
    PT.stat_len = 0;
    PT.dati_off_disk = false;
    PT.dato_to_disk = false;
    PT.msg_err = false;
  }
  else
    state.phase = -1; // bus free
}

void CSym53C895::byte_to_target(u8 value)
{
  switch (state.phase)
  {
  case 0:
    PT.dato[PT.dato_len++] = value;
    break;

  case 2:
    PT.cmd[PT.cmd_len++] = value;
    break;

  case 6:
    // message out
    if (PT.msgo_len==0 && value&7)
    {
      // LUN...
      printf("SYM: LUN selected; aborting...\n");
      PT.msg_err = true;
    }
    PT.msgo[PT.msgo_len++] = value;
    break;

  default:
    printf("byte written in phase %d\n",state.phase);
    exit(1);
  }
}

u8 CSym53C895::byte_from_target()
{
  u8 retval;

  switch (state.phase)
  {
  case 1:
    if (!PT.dati_len)
    {
      printf("Try to read DATA IN without data!\n");
      break;
    }
    retval = PT.dati[PT.dati_ptr++];
    if (PT.dati_ptr==PT.dati_len)
    {
      PT.dati_ptr = 0;
      PT.dati_len = 0;
    }
    break;

  case 3:
    if (!PT.stat_len)
    {
      printf("Try to read STATUS without data!\n");
      break;
    }
    retval = PT.stat[PT.stat_ptr++];
    if (PT.stat_ptr==PT.stat_len)
    {
      PT.stat_ptr = 0;
      PT.stat_len = 0;
    }
    break;

  case 7:
    if (!PT.msgi_len)
    {
      printf("Try to read MESSAGE IN without data!\n");
      break;
    }
    retval = PT.msgi[PT.msgi_ptr++];
    if (PT.msgi_ptr==PT.msgi_len)
    {
      PT.msgi_len = 0;
      PT.msgi_ptr = 0;
    }
    break;

  default:
    printf("byte requested in phase %d\n",state.phase);
   // exit(1);
  }
  R8(SFBR) = retval;
  return retval;
}

void CSym53C895::end_xfer()
{
  int res;
  int newphase = state.phase;

  switch (state.phase)
  {
  case 6: // msg out
    PT.cmd_len = 0;
    PT.dato_len = 0;
    if (PT.msg_err)
      newphase = -1;
    else
      newphase = 2; // command
    break;

  case 2: // command;
    res = do_command();
    if (res == 2)
      newphase = 0; // data out
    else if (PT.dati_len)
      newphase = 1; // data in
    else
      newphase = 3; // status
    break;

  case 0: // data out;
    if (!PT.dato_to_disk)
    {
      res = do_command();
      if (res == 2)
      {
        printf("do_command returned 2 after DATA OUT phase!!\n");
        exit(1);
      }
    }
    else
      PT.dato_to_disk = false;

    if (PT.dati_len)
      newphase = 1; // data in
    else
      newphase = 3; // status
    break;

  case 1: // data in
    PT.dati_off_disk = false;
    if (!PT.dati_len)
      newphase = 3; // status
 
  case 3: // status
    if (!PT.stat_len)
      newphase = 7; // message in

  case 7: // message in
    if (!PT.msgi_len)
      newphase = -1;
    break;
  }

  if (newphase != state.phase)
  {
    printf("SYM: Transition from phase %d to phase %d.\n",state.phase,newphase);
    state.phase = newphase;
  }
  //getchar();
}

/*
 *  SCSI commands: 
 */
#define	SCSICMD_TEST_UNIT_READY		0x00	/*  Mandatory  */
#define	SCSICMD_REQUEST_SENSE		0x03	/*  Mandatory  */
#define	SCSICMD_INQUIRY			0x12	/*  Mandatory  */

#define	SCSICMD_READ			0x08
#define	SCSICMD_READ_10			0x28
#define	SCSICMD_WRITE			0x0a
#define	SCSICMD_WRITE_10		0x2a
#define	SCSICMD_MODE_SELECT		0x15
#define	SCSICMD_MODE_SENSE		0x1a
#define	SCSICMD_START_STOP_UNIT		0x1b
#define	SCSICMD_PREVENT_ALLOW_REMOVE	0x1e
#define	SCSICMD_MODE_SENSE10		0x5a

#define	SCSICMD_SYNCHRONIZE_CACHE	0x35

/*  SCSI block device commands:  */
#define	SCSIBLOCKCMD_READ_CAPACITY	0x25

/*  SCSI CD-ROM commands:  */
#define	SCSICDROM_READ_SUBCHANNEL	0x42
#define	SCSICDROM_READ_TOC		0x43
#define	SCSICDROM_READ_DISCINFO		0x51
#define	SCSICDROM_READ_TRACKINFO	0x52

/*  SCSI tape commands:  */
#define	SCSICMD_REWIND			0x01
#define	SCSICMD_READ_BLOCK_LIMITS	0x05
#define	SCSICMD_SPACE			0x11

int CSym53C895::do_command()
{
  int retlen;
  int q;
  int pagecode;
  long ofs;

  printf("SYM.%d: %d-byte command ",GET_DEST(),PT.cmd_len);
  for (int x=0;x<PT.cmd_len;x++) printf("%02x ",PT.cmd[x]);
  printf("\n");

  if (PT.cmd_len<1)
    return 0;

  switch(PT.cmd[0])
  {
  case SCSICMD_TEST_UNIT_READY:
    printf("SYM.%d: TEST UNIT READY.\n",GET_DEST());
    if (PT.cmd_len != 6)
	  printf("Weird cmd_len=%d.\n", PT.cmd_len);
	if (PT.cmd[1] != 0x00) 
    {
      printf("SYM: Don't know how to handle INQUIRY with cmd[1]=0x%02x.\n", PT.cmd[1]);
      break;
	}
    PT.stat_len = 1;
    PT.stat[0] = 0;
    PT.stat_ptr = 0;
    PT.msgi_len = 1;
    PT.msgi[0] = 0;
    PT.msgi_ptr = 0;
    break;

  case SCSICMD_INQUIRY:
    printf("SYM.%d: INQUIRY.\n",GET_DEST());
    if (PT.cmd_len != 6)
	  printf("Weird cmd_len=%d.\n", PT.cmd_len);
	if (PT.cmd[1] != 0x00) 
    {
      printf("SYM: Don't know how to handle INQUIRY with cmd[1]=0x%02x.\n", PT.cmd[1]);
      break;
	}

    /*  Return values:  */
    retlen = PT.cmd[4];
    if (retlen < 36) {
	    printf("SYM: SCSI inquiry len=%i, <36!\n", retlen);
	    retlen = 36;
    }
    PT.dati[0] = 0; // hard disk
    PT.dati[1] = 0; // not removable;
    PT.dati[2] = 0x02; // ANSI scsi 2
    PT.dati[3] = 0x02; // response format
    PT.dati[4] = 32; // additional length
    PT.dati[5] = 0; // reserved
    PT.dati[6] = 0x04; // reserved
    PT.dati[7] = 0x60; // capabilities
//                        vendor  model           rev.
    memcpy(&(PT.dati[8]),"DEC     RZ58     (C) DEC2000",28);
    PT.dati_ptr = 0;
    PT.dati_len = retlen;

    /*  Some data is different for CD-ROM drives:  */
    if (PTD->cdrom()) {
      PT.dati[0] = 0x05;  /*  0x05 = CD-ROM  */
      PT.dati[1] = 0x80;  /*  0x80 = removable  */
//                           vendor  model           rev.
	  memcpy(&(PT.dati[8]), "DEC     RRD42   (C) DEC 4.5d",28);
    }

    PT.stat_len = 1;
    PT.stat[0] = 0;
    PT.stat_ptr = 0;
    PT.msgi_len = 1;
    PT.msgi[0] = 0;
    PT.msgi_ptr = 0;
    break;

  case SCSICMD_MODE_SENSE:
  case SCSICMD_MODE_SENSE10:	
    printf("SYM.%d: MODE SENSE.\n",GET_DEST());
	q = 4; retlen = PT.cmd[4];
	switch (PT.cmd_len) {
	case 6:	break;
	case 10:q = 8;
		retlen = PT.cmd[7] * 256 + PT.cmd[8];
		break;
	default:
	  printf("Weird cmd_len=%d.\n", PT.cmd_len);
      exit(1);
	}

	if ((PT.cmd[2] & 0xc0) != 0)
    {
      printf(" mode sense, cmd[2] = 0x%02x.\n", PT.cmd[2]);
      exit(1);
    }

	/*  Return data:  */

    PT.dati_len = retlen;	/*  Restore size.  */

	pagecode = PT.cmd[2] & 0x3f;

	printf("[ MODE SENSE pagecode=%i ]\n", pagecode);

	/*  4 bytes of header for 6-byte command,
	    8 bytes of header for 10-byte command.  */
    PT.dati[0] = retlen;	/*  0: mode data length  */
    PT.dati[1] = PTD->cdrom() ? 0x05 : 0x00;
			/*  1: medium type  */
	PT.dati[2] = 0x00;	/*  device specific
					    parameter  */
	PT.dati[3] = 8 * 1;	/*  block descriptor
					    length: 1 page (?)  */

	PT.dati[q+0] = 0x00;	/*  density code  */
	PT.dati[q+1] = 0;	/*  nr of blocks, high  */
	PT.dati[q+2] = 0;	/*  nr of blocks, mid  */
	PT.dati[q+3] = 0;	/*  nr of blocks, low */
	PT.dati[q+4] = 0x00;	/*  reserved  */
    PT.dati[q+5] = (512 >> 16) & 255;
	PT.dati[q+6] = (512 >> 8) & 255;
	PT.dati[q+7] = 512 & 255;
	q += 8;

    PT.stat_len = 1;
    PT.stat[0] = 0;
    PT.stat_ptr = 0;
    PT.msgi_len = 1;
    PT.msgi[0] = 0;
    PT.msgi_ptr = 0;

	/*  descriptors, 8 bytes (each)  */

	/*  page, n bytes (each)  */
	switch (pagecode) {
	case 0:
		/*  TODO: Nothing here?  */
		break;
	case 1:		/*  read-write error recovery page  */
		PT.dati[q + 0] = pagecode;
		PT.dati[q + 1] = 10;
		break;
	case 3:		/*  format device page  */
		PT.dati[q + 0] = pagecode;
		PT.dati[q + 1] = 22;

		/*  10,11 = sectors per track  */
		PT.dati[q + 10] = 0;
        PT.dati[q + 11] = PTD->get_sectors();

		/*  12,13 = physical sector size  */
		PT.dati[q + 12] = (512 >> 8) & 255;
		PT.dati[q + 13] = 512 & 255;
		break;
	case 4:		/*  rigid disk geometry page  */
		PT.dati[q + 0] = pagecode;
		PT.dati[q + 1] = 22;
        PT.dati[q + 2] = (PTD->get_cylinders() >> 16) & 255;
		PT.dati[q + 3] = (PTD->get_cylinders() >> 8) & 255;
        PT.dati[q + 4] = PTD->get_cylinders() & 255;
        PT.dati[q + 5] = PTD->get_heads();

        //rpms
		PT.dati[q + 20] = (7200 >> 8) & 255;
		PT.dati[q + 21] = 7200 & 255;
		break;
	case 5:		/*  flexible disk page  */
		PT.dati[q + 0] = pagecode;
		PT.dati[q + 1] = 0x1e;

		/*  2,3 = transfer rate  */
		PT.dati[q + 2] = ((5000) >> 8) & 255;
		PT.dati[q + 3] = (5000) & 255;

		PT.dati[q + 4] = PTD->get_heads();
		PT.dati[q + 5] = PTD->get_sectors();

		/*  6,7 = data bytes per sector  */
		PT.dati[q + 6] = (512 >> 8) & 255;
		PT.dati[q + 7] = 512 & 255;

		PT.dati[q + 8] = (PTD->get_cylinders() >> 8) & 255;
		PT.dati[q + 9] = PTD->get_cylinders() & 255;

        //rpms
		PT.dati[q + 28] = (7200 >> 8) & 255;
		PT.dati[q + 29] = 7200 & 255;
		break;
	default:
		printf("[ MODE_SENSE for page %i is not yet implemented! ]\n", pagecode);
        exit(1);
	}
	break;

  case SCSICMD_MODE_SELECT:
    // get data out first...
    if (PT.dato_len==0)
      return 2;

    printf("SYM.%d: MODE SELECT.\n",GET_DEST());

    printf("SYM: MODE SELECT ignored. Data: ");
    for(int x= 0; x<PT.dato_len; x++) printf("%02x ",PT.dato[x]);
    printf("\n");

    // ignore it...

    PT.stat_len = 1;
    PT.stat[0] = 0;
    PT.stat_ptr = 0;
    PT.msgi_len = 1;
    PT.msgi[0] = 0;
    PT.msgi_ptr = 0;
	break;

  case SCSIBLOCKCMD_READ_CAPACITY:
	debug("READ_CAPACITY");
    printf("SYM.%d: READ CAPACITY.\n",GET_DEST());
    if (PT.cmd_len != 10)
	  printf("Weird cmd_len=%d.\n", PT.cmd_len);
	if (PT.cmd[8] & 1) 
    {
      printf("SYM: Don't know how to handle READ CAPACITY with PMI bit set.\n");
      break;
	}

    PT.dati[0] = (PTD->get_lba_size() >> 24) & 255;
	PT.dati[1] = (PTD->get_lba_size() >> 16) & 255;
	PT.dati[2] = (PTD->get_lba_size() >> 8) & 255;
	PT.dati[3] = PTD->get_lba_size() & 255;

	PT.dati[4] = (512 >> 24) & 255;
	PT.dati[5] = (512 >> 16) & 255;
	PT.dati[6] = (512 >> 8) & 255;
	PT.dati[7] = 512 & 255;

    PT.dati_len = 8;

    PT.stat_len = 1;
    PT.stat[0] = 0;
    PT.stat_ptr = 0;
    PT.msgi_len = 1;
    PT.msgi[0] = 0;
    PT.msgi_ptr = 0;
	break;

  case SCSICMD_READ:
  case SCSICMD_READ_10:
    printf("SYM.%d: READ.\n",GET_DEST());
    if (PT.cmd[0] == SCSICMD_READ)
    {
      if (PT.cmd_len != 6)
	    printf("Weird cmd_len=%d.\n", PT.cmd_len);
	  /*
	   *  bits 4..0 of cmd[1], and cmd[2] and cmd[3]
	   *  hold the logical block address.
	   *
	   *  cmd[4] holds the number of logical blocks
	   *  to transfer. (Special case if the value is
	   *  0, actually means 256.)
	   */
	  ofs = ((PT.cmd[1] & 0x1f) << 16) + (PT.cmd[2] << 8) + PT.cmd[3];
	  retlen = PT.cmd[4];
	  if (retlen == 0)
		retlen = 256;
	} 
    else 
    {
      if (PT.cmd_len != 10)
	    printf("Weird cmd_len=%d.\n", PT.cmd_len);
	  /*
	   *  cmd[2..5] hold the logical block address.
	   *  cmd[7..8] holds the number of logical
	   *  blocks to transfer. (NOTE: If the value is
	   *  0, this means 0, not 65536. :-)
	   */
	  ofs = (PT.cmd[2] << 24) + (PT.cmd[3] << 16) + (PT.cmd[4] << 8) + PT.cmd[5];
      retlen = (PT.cmd[7] << 8) + PT.cmd[8];

	}

    PT.stat_len = 1;
    PT.stat[0] = 0;
    PT.stat_ptr = 0;
    PT.msgi_len = 1;
    PT.msgi[0] = 0;
    PT.msgi_ptr = 0;

    /* Within bounds? */
    if ((ofs+retlen) > PTD->get_lba_size())
    {
      PT.stat[0] = 0x02; // check condition
      break;
    }

	/*  Return data:  */
    PTD->seek_block(ofs);
    PT.dati_len = retlen;
    PT.dati_off_disk = true;

	printf("SYM.%d READ  ofs=%d size=%d\n", GET_DEST(), ofs, retlen);

	break;

  default:
    printf("SYM: Unknown SCSI command 0x%02x.\n",PT.cmd[0]);
    exit(1);

  }
  return 0;
}

void CSym53C895::set_interrupt(int reg, u8 interrupt)
{
  printf("set interrupt %02x, %02x.\n",reg,interrupt);

  switch(reg)
  {
  case R_DSTAT:
    if (TB_R8(ISTAT,DIP) || TB_R8(ISTAT,SIP))
    {
      state.dstat_stack |= interrupt;
      printf("DSTAT stacked.\n");
    }
    else
    {
      R8(DSTAT) |= interrupt;
      printf("DSTAT.\n");
    }
    break;
  case R_ISTAT:
    printf("ISTAT.\n");
    R8(ISTAT) |= interrupt;
    break;
  default:
    printf("set_interrupt reg %02x!!\n",reg);
    exit(1);
  }

  printf("--> eval int\n");
  eval_interrupts();
  printf("<-- eval_int\n");
}

void CSym53C895::eval_interrupts()
{
  bool will_assert = false;
  bool will_halt = false;

  if (R8(DSTAT) & DSTAT_FATAL)
  {
    will_halt = true;
    printf("  will halt(DSTAT).\n");
    SB_R8(ISTAT,DIP,true);
    if (R8(DSTAT) & R8(DIEN) & DSTAT_FATAL)
    {
      will_assert = true;
      printf("  will assert(DSTAT).\n");
    }
  }
  else
    SB_R8(ISTAT,DIP,false);

  // WARNING: SCSI interrupts not checked yet!!

  if (TB_R8(ISTAT,INTF))
  {
    will_assert = true;
    printf("  will assert(INTF).\n");
  }

  if (TB_R8(DCNTL,IRQD))
  {
    will_assert = false;
    printf("  won't assert(IRQD).\n");
  }

  if (will_halt)
    state.executing = false;

  if (will_assert != state.irq_asserted)
  {
    printf("  doing...%d\n",will_assert);
    do_pci_interrupt(0,will_assert);
    state.irq_asserted = will_assert;
  }
}
