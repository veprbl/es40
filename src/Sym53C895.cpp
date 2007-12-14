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
 * X-1.1        Camiel Vanderhoeven                             14-DEC-2007
 *      Initial version in CVS.
 **/

#include "StdAfx.h"
#include "Sym53C895.h"
#include "System.h"
#include "Disk.h"

#define PT state.per_target[state.target]
#define PTD get_disk(0,state.target)

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

  printf("%%SYM-I-INIT: Symbios 53c895 emulator initialized.\n");
}

CSym53C895::~CSym53C895()
{
}
/**
 * Save state to a Virtual Machine State file.
 **/

void CSym53C895::SaveState(FILE *f)
{
  CPCIDevice::SaveState(f);
  fwrite(&state,sizeof(state),1,f);
}

/**
 * Restore state from a Virtual Machine State file.
 **/

void CSym53C895::RestoreState(FILE *f)
{
  CPCIDevice::SaveState(f);
  fread(&state,sizeof(state),1,f);
}

void CSym53C895::WriteMem_Bar(int func,int bar, u32 address, int dsize, u32 data)
{
  void * p;

  switch (bar)
  {
    case 0:
    case 1:
      address &= 0x7f;
      if ((address>=0x34 && address <=0x37) || address>=0x5c)
      {
        // scratch
        if (address<0x5c)
          address -= 0x34;
        else
          address -= 0x58;

        p = (u8*)state.scratch + address;
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
      switch(dsize)
      {
      case 8:
        switch (address)
        {
        case 0x00:
          write_b_scntl0((u8)data);
          break;
        case 0x01:
          write_b_scntl1((u8)data);
          break;
        case 0x02:
          write_b_scntl2((u8)data);
          break;
        case 0x03:
          write_b_scntl3((u8)data);
          break;
        case 0x04:
          write_b_scid((u8)data);
          break;
        case 0x05:
          write_b_sxfer((u8)data);
          break;
        case 0x07:
          write_b_gpreg((u8)data);
          break;
        case 0x10:
          state.dsa = (state.dsa & 0xffffff00) | (data & 0xff);
          break;
        case 0x11:
          state.dsa = (state.dsa & 0xffff00ff) | ((data<<8) & 0xff00);
          break;
        case 0x12:
          state.dsa = (state.dsa & 0xff00ffff) | ((data<<16) & 0xff0000);
          break;
        case 0x13:
          state.dsa = (state.dsa & 0x00ffffff) | ((data<<24) & 0xff000000);
          break;
        case 0x14:
          write_b_istat((u8)data);
          break;
        case 0x1b:
          write_b_ctest3((u8)data);
          break;
        case 0x21:
          write_b_ctest4((u8)data);
          break;
        case 0x22:
          write_b_ctest5((u8)data);
          break;
        case 0x38:
          write_b_dmode((u8)data);
          break;
        case 0x39:
          write_b_dien((u8)data);
          break;
        case 0x3b:
          write_b_dcntl((u8)data);
          break;
        case 0x40:
          write_b_sien0((u8)data);
          break;
        case 0x41:
          write_b_sien1((u8)data);
          break;
        case 0x47:
          write_b_gpcntl((u8)data);
          break;
        case 0x48:
          write_b_stime0((u8)data);
          break;
        case 0x49:
          write_b_stime1((u8)data);
          break;
        case 0x4a:
          write_b_respid0((u8)data);
          break;
        case 0x4b:
          write_b_respid1((u8)data);
          break;
        case 0x4d:
          write_b_stest1((u8)data);
          break;
        case 0x4e:
          write_b_stest2((u8)data);
          break;
        case 0x4f:
          write_b_stest3((u8)data);
          break;
        default:
          printf("SYM: Attempt to write %d bits to memory at %02x with %08x\n", dsize, address,data);
          exit(1);
        }
        break;
      case 16:
        WriteMem_Bar(0,1,address,8,data & 0xff);
        WriteMem_Bar(0,1,address+1,8,(data>>8) & 0xff);
        break;
      case 32:
        switch (address)
        {
        case 0x0c: // dstat, sstat0..sstat2
          printf("SYM: Attempt to write 32-bits at 0x0c: FreeBSD driver quirk.\n");
          break;
        case 0x10:
          write_d_dsa(data);
          break;
        case 0x2c:
          write_d_dsp(data);
          break;
        case 0x30:
          write_d_dsps(data);
          break;
        default:
          printf("SYM: Attempt to write %d bits to memory at %02x with %08x\n", dsize, address,data);
          exit(1);
        }
        break;
      }
      break;
    case 2:
      printf("SYM: Attempt to write %d bits to ram at %04x with %08x\n", dsize, address,data);
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
      if ((address>=0x34 && address <=0x37) || address>=0x5c)
      {
        // scratch
        if (address<0x5c)
          address -= 0x34;
        else
          address -= 0x58;

        if (address>=0 && address<=7 && state.ctest.srtch)
          p = (u8*)&(pci_state.config_data[0][5]) + address; // memory registers BAR / RAM BAR
        else
          p = (u8*)state.scratch + address;
        switch(dsize)
        {
        case 8:
          data = *((u8 *) p);
          break;
        case 16:
          data = *((u16 *) p);
          break;
        case 32:
          data = *((u32 *) p);
          break;
        }
        break;
      }
      switch(dsize)
      {
      case 8:
        switch(address)
        {
        case 0x00:
          data = read_b_scntl0();
          break;
        case 0x01:
          data = read_b_scntl1();
          break;
        case 0x02:
          data = read_b_scntl2();
          break;
        case 0x03:
          data = read_b_scntl3();
          break;
        case 0x04:
          data = read_b_scid();
          break;
        case 0x05:
          data = read_b_sxfer();
          break;
        case 0x07:
          data = read_b_gpreg();
          break;
        case 0x0c:
          data = read_b_dstat();
          break;
        case 0x14:
          data = read_b_istat();
          break;
        case 0x1b:
          data = read_b_ctest3();
          break;
        case 0x21:
          data = read_b_ctest4();
          break;
        case 0x22:
          data = read_b_ctest5();
          break;
        case 0x38:
          data = read_b_dmode();
          break;
        case 0x39:
          data = read_b_dien();
          break;
        case 0x3b:
          data = read_b_dcntl();
          break;
        case 0x40:
          data = read_b_sien0();
          break;
        case 0x41:
          data = read_b_sien1();
          break;
        case 0x42:
          data = read_b_sist0();
          break;
        case 0x43:
          data = read_b_sist1();
          break;
        case 0x47:
          data = read_b_gpcntl();
          break;
        case 0x48:
          data = read_b_stime0();
          break;
        case 0x49:
          data = read_b_stime1();
          break;
        case 0x4a:
          data = read_b_respid0();
          break;
        case 0x4b:
          data = read_b_respid1();
          break;
        case 0x4d:
          data = read_b_stest1();
          break;
        case 0x4e:
          data = read_b_stest2();
          break;
        case 0x4f:
          data = read_b_stest3();
          break;
        case 0x52:
          data = read_b_stest4();
          break;
        default:
          printf("SYM: Attempt to read %d bits from memory at %02x\n", dsize, address);
          exit(1);
        }
        break;
      case 16:
        data = (ReadMem_Bar(0,1,address,8) & 0xff) | ((ReadMem_Bar(0,1,address+1,8)<<8) & 0xff00);
        break;
      case 32:
        switch (address)
        {
        case 0x0c: // dstat, sstat0..sstat2
          printf("SYM: Attempt to read 32-bits at 0x0c: FreeBSD driver quirk.\n");
          data = ((ReadMem_Bar(0,1,address,  8)<< 0) & 0x000000ff) 
               | ((ReadMem_Bar(0,1,address+1,8)<< 8) & 0x0000ff00)
               | ((ReadMem_Bar(0,1,address+2,8)<<16) & 0x00ff0000)
               | ((ReadMem_Bar(0,1,address+3,8)<<24) & 0xff000000);
          break;
        case 0x10:
          return read_d_dsa();
        case 0x2c:
          return read_d_dsp();
        case 0x30:
          return read_d_dsps();
        default:
          printf("SYM: Attempt to read %d bits from memory at %02x\n", dsize, address);
          exit(1);
        }
      break;
      }
      break;
    case 2:
      printf("SYM: Attempt to read %d bits from ram at %04x\n", dsize, address);
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


u8 CSym53C895::read_b_scntl0()
{
  return (state.scntl.arb << 6)
       | (state.scntl.start ? 0x20 : 0x00)
       | (state.scntl.watn  ? 0x10 : 0x00)
       | (state.scntl.epc   ? 0x08 : 0x00)
       | (state.scntl.aap   ? 0x02 : 0x00)
       | (state.scntl.trg   ? 0x01 : 0x00);
}

void CSym53C895::write_b_scntl0(u8 value)
{
  bool old_start = state.scntl.start;

  state.scntl.arb = (value>>6) & 3;
  state.scntl.start = (value>>5) & 1;
  state.scntl.watn = (value>>4) & 1;
  state.scntl.epc = (value>>3) & 1;
  state.scntl.aap = (value>>1) & 1;
  state.scntl.trg = (value>>0) & 1;

  if (state.scntl.start && !old_start)
    printf("SYM: Don't know how to start arbitration sequence.\n");
}

u8 CSym53C895::read_b_scntl1()
{
  return (state.scntl.exc ? 0x80 : 0x00)
       | (state.scntl.adb ? 0x40 : 0x00)
       | (state.scntl.dhp ? 0x20 : 0x00)
       | (state.scntl.con ? 0x10 : 0x00)
       | (state.scntl.rst ? 0x08 : 0x00)
       | (state.scntl.aesp ? 0x04 : 0x00)
       | (state.scntl.iarb? 0x02 : 0x00)
       | (state.scntl.sst ? 0x01 : 0x00);
}

void CSym53C895::write_b_scntl1(u8 value)
{
  bool old_iarb = state.scntl.iarb;
  bool old_con = state.scntl.con;

  state.scntl.exc = (value>>7) & 1;
  state.scntl.adb = (value>>6) & 1;
  state.scntl.dhp = (value>>5) & 1;
  state.scntl.con = (value>>4) & 1;
  state.scntl.rst = (value>>3) & 1;
  state.scntl.aesp = (value>>2) & 1;
  state.scntl.iarb = (value>>1) & 1;
  state.scntl.sst = (value>>0) & 1;

  if (state.scntl.con != old_con)
    printf("SYM: Don't know how to forcibly connect or disconnect\n");

  if (state.scntl.iarb && !old_iarb)
    printf("SYM: Don't know how to start immediate arbitration sequence.\n");

}

u8 CSym53C895::read_b_scntl2()
{
  return (state.scntl.sdu ? 0x80 : 0x00)
       | (state.scntl.chm ? 0x40 : 0x00)
       | (state.scntl.slpmd ? 0x20 : 0x00)
       | (state.scntl.slphben ? 0x10 : 0x00)
       | (state.scntl.wss ? 0x08 : 0x00)
       | (state.scntl.vue0 ? 0x04 : 0x00)
       | (state.scntl.vue1 ? 0x02 : 0x00)
       | (state.scntl.wsr ? 0x01 : 0x00);
}

void CSym53C895::write_b_scntl2(u8 value)
{
  state.scntl.slpmd = (value>>5) & 1;
  state.scntl.slphben = (value>>4) & 1;
  if ((value>>3)&1) 
    state.scntl.wss = false;
  state.scntl.vue1 = (value>>1) & 1;
  if ((value>>0)&1) 
    state.scntl.wsr = false;
}

u8 CSym53C895::read_b_scntl3()
{
  return (state.scntl.ultra ? 0x80 : 0x00)
       | (state.scntl.scf << 4)
       | (state.scntl.ews ? 0x08: 0x00)
       | state.scntl.ccf;
}

void CSym53C895::write_b_scntl3(u8 value)
{
  state.scntl.ultra = (value>>7) & 1;
  state.scntl.scf = (value>>4) & 7;
  state.scntl.ews = (value>>3) & 1;
  state.scntl.ccf = (value>>0) & 7;
}

u8 CSym53C895::read_b_scid()
{
  return (state.scntl.rre ? 0x40 : 0x00)
       | (state.scntl.sre ? 0x20 : 0x00)
       | state.scntl.my_scsi_id;
}

void CSym53C895::write_b_scid(u8 value)
{
  int old_scsi_id = state.scntl.my_scsi_id;
  state.scntl.rre = (value>>6) & 1;
  state.scntl.sre = (value>>5) & 1;
  state.scntl.my_scsi_id = value & 0x0f;
  if (state.scntl.my_scsi_id != old_scsi_id)
    printf("SYM: SCSI id set to %d.\n",state.scntl.my_scsi_id);
}

u8 CSym53C895::read_b_sxfer()
{
  return (state.sxfer.tp << 6)
    | state.sxfer.mo;
}

void CSym53C895::write_b_sxfer(u8 value)
{
  state.sxfer.tp = (value>>6) & 3;
  state.sxfer.mo = (value>>0) & 0x3f;
}


u8 CSym53C895::read_b_istat()
{
  return (state.istat.abrt ? 0x80 : 0x00)
       | (state.istat.srst ? 0x40 : 0x00)
       | (state.istat.sigp ? 0x20 : 0x00)
       | (state.istat.sem  ? 0x10 : 0x00)
       | (state.istat.con  ? 0x08 : 0x00)
       | (state.istat.intf ? 0x04 : 0x00)
       | (state.istat.sip  ? 0x02 : 0x00)
       | (state.istat.dip  ? 0x01 : 0x00);
}

void CSym53C895::write_b_istat(u8 value)
{
  bool old_abrt = state.istat.abrt;
  bool old_srst = state.istat.srst;

  state.istat.abrt = (value>>7) & 1;
  state.istat.srst = (value>>6) & 1;
  state.istat.sigp = (value>>5) & 1;
  state.istat.sem  = (value>>4) & 1;

  if ((value>>2)&1) state.istat.intf = false;

  if (state.istat.abrt && !old_abrt)
    printf("SYM: Don't know how to initiate an abort yet!\n");

  if (state.istat.srst && !old_srst)
    printf("SYM: Don't know how to initiate a reset yet!\n");
}

u8 CSym53C895::read_b_ctest3()
{
  return ((pci_state.config_data[0][2] << 4) & 0xf0) //chip rev level
    | (state.ctest.flf ? 0x08: 0x00)
    | (state.ctest.fm ? 0x02: 0x00)
    | (state.ctest.wrie ? 0x01: 0x00);
}

void CSym53C895::write_b_ctest3(u8 value)
{
  state.ctest.flf = (value>>3) & 1;
  state.ctest.fm = (value>>1) & 1;
  state.ctest.wrie =  (value>>0) & 1;

  if (state.ctest.flf)
    printf("SYM: Don't know how to flush DMA FIFO\n");

  if ((value>>2) & 1)
    printf("SYM: Don't know how to clear DMA FIFO\n");
}

u8 CSym53C895::read_b_ctest4()
{
  return (state.ctest.bdis ? 0x80 : 0x00)
    | (state.ctest.zmod? 0x40: 0x00)
    | (state.ctest.zsd? 0x20: 0x00)
    | (state.ctest.srtm? 0x10: 0x00)
    | (state.ctest.mpee? 0x08: 0x00)
    | state.ctest.fbl;
}

void CSym53C895::write_b_ctest4(u8 value)
{
  state.ctest.bdis = (value>>7) & 1;
  state.ctest.zmod = (value>>6) & 1;
  state.ctest.zsd =  (value>>5) & 1;
  state.ctest.srtm = (value>>4) & 1;
  state.ctest.mpee = (value>>3) & 1;
  state.ctest.fbl =  (value>>0) & 7;

  if (state.ctest.srtm)
    printf("SYM: Shadow Register Test Mode not supported\n");
}

u8 CSym53C895::read_b_ctest5()
{
  return (state.dma.dfs ? 0x20 : 0x00)
    | (state.ctest.masr ? 0x10 : 0x00)
    | (state.dma.ddir ? 0x08 : 0x00)
    | (state.dma.bl & 0x04)
    | ((state.dma.bo >> 8) & 0x03);
}

void CSym53C895::write_b_ctest5(u8 value)
{
  if ((value>>7) & 1)
    printf("SYM: Don't know how to do Clock Address increment.\n");
  if ((value>>6) & 1)
    printf("SYM: Don't know how to do Clock Byte Counter decrement.\n");
  state.dma.dfs = (value>>5) & 1;
  if ((value>>3) & 1)
    state.dma.ddir = (value>>4) & 1;

  state.dma.bl = (state.dma.bl & 0x03) | (value & 0x04);
  state.dma.bo = (state.dma.bo & 0xff) | ((value<<8) & 0x300);
}

u8 CSym53C895::read_b_dmode()
{
  return ((state.dma.bl << 6) & 0xc0)
    | (state.dma.siom ? 0x20 : 0x00)
    | (state.dma.diom ? 0x10 : 0x00)
    | (state.dma.erl ? 0x08 : 0x00)
    | (state.dma.ermp ? 0x04 : 0x00)
    | (state.dma.bof ? 0x02 : 0x00)
    | (state.dma.man ? 0x01 : 0x00);
}

void CSym53C895::write_b_dmode(u8 value)
{
  state.dma.bl = (state.dma.bl & 0x04) | ((value>>6) & 0x03);
  state.dma.siom = (value>>5) & 1;
  state.dma.diom = (value>>4) & 1;
  state.dma.erl = (value>>3) & 1;
  state.dma.ermp = (value>>2) & 1;
  state.dma.bof = (value>>1) & 1;
  state.dma.man = (value>>0) & 1;
}

u8 CSym53C895::read_b_dien()
{
  return (state.dien.mdpe ? 0x40 : 0x00)
    | (state.dien.bf ? 0x20 : 0x00)
    | (state.dien.abrt ? 0x10 : 0x00)
    | (state.dien.ssi ? 0x08 : 0x00)
    | (state.dien.sir ? 0x04 : 0x00)
    | (state.dien.iid ? 0x01 : 0x00);
}

void CSym53C895::write_b_dien(u8 value)
{
  state.dien.mdpe = (value>>6) & 1;
  state.dien.bf = (value>>5) & 1;
  state.dien.abrt = (value>>4) & 1;
  state.dien.ssi = (value>>3) & 1;
  state.dien.sir = (value>>2) & 1;
  state.dien.iid = (value>>0) & 1;
}

u8 CSym53C895::read_b_dstat()
{
  u8 retval = (state.dstat.dfe ? 0x80 : 0x00)
    |  (state.dstat.mdpe ? 0x40 : 0x00)
    | (state.dstat.bf ? 0x20 : 0x00)
    | (state.dstat.abrt ? 0x10 : 0x00)
    | (state.dstat.ssi ? 0x08 : 0x00)
    | (state.dstat.sir ? 0x04 : 0x00)
    | (state.dstat.iid ? 0x01 : 0x00);

  state.dstat.mdpe = false;
  state.dstat.bf = false;
  state.dstat.abrt = false;
  state.dstat.ssi = false;
  state.dstat.sir = false;
  state.dstat.iid = false;

  state.istat.dip = false;

  // end any interrupts pending...
  do_pci_interrupt(0, false);

  return retval;
}

u8 CSym53C895::read_b_dcntl()
{
  return (state.dma.clse ? 0x80 : 0x00)
    | (state.dma.pff ? 0x40 : 0x00)
    | (state.dma.pfen ? 0x20 : 0x00)
    | (state.dma.ssm ? 0x10 : 0x00)
    | (state.dma.irqm ? 0x08 : 0x00)
    | (state.dma.irqd ? 0x02 : 0x00)
    | (state.dma.com ? 0x01 : 0x00);
}

void CSym53C895::write_b_dcntl(u8 value)
{
  state.dma.clse = (value>>7) & 1;
  state.dma.pff = (value>>6) & 1;
  state.dma.pfen = (value>>5) & 1;
  state.dma.ssm = (value>>4) & 1;
  state.dma.irqm = (value>>3) & 1;

  // start operation
  if ((value>>2) & 1)
    state.executing = true;

  state.dma.irqd = (value>>1) & 1;
  state.dma.com = (value>>0) & 1;
}

u8 CSym53C895::read_b_sien0()
{
  u8 retval;
  retval = (state.sien.ma ? 0x80 : 0x00)
    | (state.sien.cmp ? 0x40 : 0x00)
    | (state.sien.sel ? 0x20 : 0x00)
    | (state.sien.rsl ? 0x10 : 0x00)
    | (state.sien.sge ? 0x08 : 0x00)
    | (state.sien.udc ? 0x04 : 0x00)
    | (state.sien.rst ? 0x02 : 0x00)
    | (state.sien.par ? 0x01 : 0x00);
  return retval;
}

void CSym53C895::write_b_sien0(u8 value)
{
  state.sien.ma = (value>>7) & 1;
  state.sien.cmp = (value>>6) & 1;
  state.sien.sel = (value>>5) & 1;
  state.sien.rsl = (value>>4) & 1;
  state.sien.sge = (value>>3) & 1;
  state.sien.udc = (value>>2) & 1;
  state.sien.rst = (value>>1) & 1;
  state.sien.par = (value>>0) & 1;
}

u8 CSym53C895::read_b_sien1()
{
  u8 retval;
  retval = (state.sien.sbmc ? 0x10 : 0x00)
    | (state.sien.sto ? 0x04 : 0x00)
    | (state.sien.gen ? 0x02 : 0x00)
    | (state.sien.hth ? 0x01 : 0x00);
  return retval;
}

void CSym53C895::write_b_sien1(u8 value)
{
  state.sien.sbmc = (value>>4) & 1;
  state.sien.sto = (value>>2) & 1;
  state.sien.gen = (value>>1) & 1;
  state.sien.hth = (value>>0) & 1;
}

u8 CSym53C895::read_b_sist0()
{
  u8 retval;
  retval = (state.sist.ma ? 0x80 : 0x00)
    | (state.sist.cmp ? 0x40 : 0x00)
    | (state.sist.sel ? 0x20 : 0x00)
    | (state.sist.rsl ? 0x10 : 0x00)
    | (state.sist.sge ? 0x08 : 0x00)
    | (state.sist.udc ? 0x04 : 0x00)
    | (state.sist.rst ? 0x02 : 0x00)
    | (state.sist.par ? 0x01 : 0x00);

  state.sist.ma = false;
  state.sist.cmp = false;
  state.sist.sel = false;
  state.sist.rsl = false;
  state.sist.sge = false;
  state.sist.udc = false;
  state.sist.rst = false;
  state.sist.par = false;

  return retval;
}

u8 CSym53C895::read_b_sist1()
{
  u8 retval;
  retval = (state.sist.sbmc ? 0x10 : 0x00)
    | (state.sist.sto ? 0x04 : 0x00)
    | (state.sist.gen ? 0x02 : 0x00)
    | (state.sist.hth ? 0x01 : 0x00);

  state.sist.sbmc = false;
  state.sist.sto = false;
  state.sist.gen = false;
  state.sist.hth = false;

  return retval;
}

u8 CSym53C895::read_b_stest1()
{
  return (state.stest.sclk ? 0x80 : 0x00)
    | (state.stest.siso ? 0x40 : 0x00)
    | (state.stest.qen ? 0x08 : 0x00)
    | (state.stest.qsel ? 0x04 : 0x00);
}

void CSym53C895::write_b_stest1(u8 value)
{
  state.stest.sclk = (value>>7) & 1;
  state.stest.siso = (value>>6) & 1;
  state.stest.qen = (value>>3) & 1;
  state.stest.qsel = (value>>2) & 1;
}

u8 CSym53C895::read_b_stest2()
{
  return (state.stest.sce ? 0x80 : 0x00)
    | (state.stest.dif ? 0x20 : 0x00)
    | (state.stest.slb ? 0x10 : 0x00)
    | (state.stest.szm ? 0x08 : 0x00)
    | (state.stest.aws ? 0x04 : 0x00)
    | (state.stest.ext ? 0x02 : 0x00)
    | (state.stest.low ? 0x01 : 0x00);
}

void CSym53C895::write_b_stest2(u8 value)
{
  state.stest.sce = (value>>7) & 1;
  state.stest.dif = (value>>5) & 1;
  state.stest.slb = (value>>4) & 1;
  state.stest.szm = (value>>3) & 1;
  state.stest.aws = (value>>2) & 1;
  state.stest.ext = (value>>1) & 1;
  state.stest.low = (value>>0) & 1;

  if ((value>>6) & 1)
    printf("SYM: Don't know how to reset SCSI offset!\n");

  if (state.stest.low)
  {
    printf("SYM: I don't like LOW level mode!\n");
    exit(1);
  }
}

u8 CSym53C895::read_b_stest3()
{
  return (state.stest.te ? 0x80 : 0x00)
    | (state.stest.str ? 0x40 : 0x00)
    | (state.stest.hsc ? 0x20 : 0x00)
    | (state.stest.dsi ? 0x10 : 0x00)
    | (state.stest.s16 ? 0x08 : 0x00)
    | (state.stest.ttm ? 0x04 : 0x00)
    | (state.stest.stw ? 0x01 : 0x00);
}

void CSym53C895::write_b_stest3(u8 value)
{
  state.stest.te = (value>>7) & 1;
  state.stest.str = (value>>6) & 1;
  state.stest.hsc = (value>>5) & 1;
  state.stest.dsi = (value>>4) & 1;
  state.stest.s16 = (value>>3) & 1;
  state.stest.ttm = (value>>2) & 1;
  if ((value>>1) & 1)
    printf("SYM: Don't know how to clear the SCSI fifo.\n");
  state.stest.stw = (value>>0) & 1;
}

u8 CSym53C895::read_b_stest4()
{
  return 0xe0; //LVD SCSI, Freq. Lock
}

u8 CSym53C895::read_b_respid0()
{
  return state.scntl.response_id & 0xff;
}

void CSym53C895::write_b_respid0(u8 value)
{
  state.scntl.response_id = (state.scntl.response_id & 0xff00) | (value & 0xff);
}

u8 CSym53C895::read_b_respid1()
{
  return (state.scntl.response_id>>8) & 0xff;
}

void CSym53C895::write_b_respid1(u8 value)
{
  state.scntl.response_id = (state.scntl.response_id & 0xff) | ((value<<8) & 0xff00);
}

u8 CSym53C895::read_b_stime0()
{
  return (state.stime.hth << 4)
    | state.stime.sel;
}

void CSym53C895::write_b_stime0(u8 value)
{
  state.stime.hth = (value>>4) & 0x0f;
  state.stime.sel = (value>>0) & 0x0f;
}

u8 CSym53C895::read_b_stime1()
{
  return (state.stime.hthba ? 0x40 : 0x00)
    | (state.stime.gensf ? 0x20 : 0x00)
    | (state.stime.hthsf ? 0x10 : 0x00)
    | state.stime.gen;
}


void CSym53C895::write_b_stime1(u8 value)
{
  state.stime.hthba = (value>>6) & 1;
  state.stime.gensf = (value>>5) & 1;
  state.stime.hthsf = (value>>4) & 1;
  state.stime.gen = (value>>0) & 0x0f;
}

u8 CSym53C895::read_b_gpreg()
{
  return state.gp.gpio & 0x1f;
}

void CSym53C895::write_b_gpreg(u8 value)
{
  state.gp.gpio = value & 0x1f;
}

u8 CSym53C895::read_b_gpcntl()
{
  return (state.gp.me ? 0x80 : 0x00)
    | (state.gp.fe ? 0x40 : 0x00)
    | (state.gp.gpio_en & 0x1f);

}

void CSym53C895::write_b_gpcntl(u8 value)
{
  state.gp.me = (value>>7) & 1;
  state.gp.fe = (value>>6) & 1;
  state.gp.gpio_en = value & 0x1f;
}

u32 CSym53C895::read_d_dsa()
{
  return state.dsa;
}

void CSym53C895::write_d_dsa(u32 value)
{
  state.dsa = value;
}

u32 CSym53C895::read_d_dsps()
{
  return state.dsps;
}

void CSym53C895::write_d_dsps(u32 value)
{
  state.dsps = value;
}

u32 CSym53C895::read_d_dsp()
{
  return state.dsp;
}

void CSym53C895::write_d_dsp(u32 value)
{
  state.dsp = value;
  if (!state.dma.man)
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
    if (state.dma.ssm)
    {
      printf("SYM: Single step...\n");
      state.istat.dip = true;
      state.dstat.ssi = true;
      do_pci_interrupt(0,true);
    }

    printf("SYM: EXECUTING SCRIPT\n");
    printf("SYM: INS @ %x, %x   \n",state.dsp, state.dsp+4);

    cmda0 = cSystem->PCI_Phys(myPCIBus, state.dsp);
    cmda1 = cSystem->PCI_Phys(myPCIBus, state.dsp + 4);

    state.dsp += 8;

    printf("SYM: INS @ %" LL "x, %" LL "x   \n",cmda0, cmda1);

    state.dbc = cSystem->ReadMem(cmda0,32);
    state.dcmd = (state.dbc>>24) & 0xff;
    state.dbc &= 0xffffff;
    state.dsps = cSystem->ReadMem(cmda1,32);

    printf("SYM: INS = %x, %x, %x   \n",state.dcmd, state.dbc, state.dsps);

    optype = (state.dcmd>>6) & 3;
    switch(optype)
    {
    case 0:
      {
        bool indirect = (state.dcmd>>5) & 1;
        bool table_indirect = (state.dcmd>>4) & 1;
        int opcode = (state.dcmd>>3) & 1;
        int scsi_phase = (state.dcmd>>0) & 7;
        printf("SYM: INS = Block Move (i %d, t %d, opc %d, phase %d\n",indirect,table_indirect,opcode,scsi_phase);

        state.dnad = state.dsps;
        if (state.phase == scsi_phase)
        {
          printf("SYM: Ready for transfer.\n");

          u32 start;
          u32 count;

          int i;
          if (table_indirect)
          {
            u32 add = state.dsa + sext_32(state.dsps,24);

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
            start = state.dsps;
            count = state.dbc;
          }
          state.dnad = start;
          if (state.phase == 0 && PT.dato_to_disk)
          {
            cmda0 = cSystem->PCI_Phys(myPCIBus,state.dnad);
            void * dptr = cSystem->PtrToMem(cmda0);
            PTD->write_blocks(dptr,PT.dato_len);
            PT.dato_len = 0;
            state.dnad +=(PT.dato_len*512);
          }
          else if (state.phase == 1 && PT.dati_off_disk)
          {
            cmda0 = cSystem->PCI_Phys(myPCIBus,state.dnad);
            void * dptr = cSystem->PtrToMem(cmda0);
            PTD->read_blocks(dptr,PT.dati_len);
            PT.dati_len = 0;
            state.dnad +=(PT.dati_len*512);
          }
          else
          {
            for (i=0;i<count;i++)
            {
              u8 dat;
              cmda0 = cSystem->PCI_Phys(myPCIBus,state.dnad++);
              if (state.phase>=0)
              {
                if (state.phase & 1)
                {
                  dat = byte_from_target();
                  printf("SYM: Reading byte %02x\n",dat);
                  printf("SYM: Writing byte to system @ %" LL "x\n",cmda0);
                  cSystem->WriteMem(cmda0,8,dat);
                }
                else
                {
                  printf("SYM: Reading a byte from system at %x\n",cmda0);
                  dat = cSystem->ReadMem(cmda0,8);
                  printf("SYM: Writing byte %02x\n",dat);
                  byte_to_target(dat);
                }
              }
            }
          }
          end_xfer();
          return 0;
        }
      }
      break;
    case 1:
      {
        int opcode = (state.dcmd>>3) & 7;

        if (opcode < 5)
        {
          bool relative = (state.dcmd>>2) & 1;
          bool table_indirect = (state.dcmd>>1) & 1;
          bool atn = (state.dcmd>>0) & 1;
          int destination = (state.dbc>>16) & 0x0f;
          bool sc_carry = (state.dbc>>10) & 1;
          bool sc_target = (state.dbc>>9) & 1;
          bool sc_ack = (state.dbc>>6) & 1;
          bool sc_atn = (state.dbc>>3) & 1;

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
          case 3:
            printf("SYM: OPC = Set %s%s%s%s\n",sc_carry?"carry ":"",sc_target?"target ":"",sc_ack?"ack ":"",sc_atn?"atn ":"");
            if (sc_ack) state.socl.ack = true;
            if (sc_atn) state.socl.atn = true;
            if (sc_target) state.scntl.trg = true;
            if (sc_carry) state.alu.carry = true;
            return 0;
          case 4:
            printf("SYM: OPC = Clear %s%s%s%s\n",sc_carry?"carry ":"",sc_target?"target ":"",sc_ack?"ack ":"",sc_atn?"atn ":"");
            if (sc_ack) state.socl.ack = false;
            if (sc_atn) state.socl.atn = false;
            if (sc_target) state.scntl.trg = false;
            if (sc_carry) state.alu.carry = false;
            return 0;

            break;
          }
        }
        else
        {
          int oper = (state.dcmd>>0) & 7;
          bool use_data8_sfbr = (state.dbc>>23) & 1;
          int reg_address = ((state.dbc>>16) & 0x7f); //| (state.dbc & 0x80); // manual is unclear about bit 7.
          u8 imm_data = (state.dbc>>8) & 0xff;
          u8 op_data;

          printf("SYM: INS = R/W (opc %d, oper %d, use %d, add %d, imm %02x\n"
            ,opcode,oper,use_data8_sfbr,reg_address,imm_data);

          if (use_data8_sfbr)
            imm_data = state.sfbr;

          if (oper!=0)
          {
            if (opcode==5)
            {
              op_data = state.sfbr;
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
            state.sfbr = op_data = state.sfbr;
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
        int opcode = (state.dcmd>>3) & 7;
        int scsi_phase = (state.dcmd>>0) & 7;
        bool relative = (state.dbc>>23) & 1;
        bool carry_test = (state.dbc>>21) & 1;
        bool interrupt_fly = (state.dbc>>20) & 1;
        bool jump_if = (state.dbc>>19) & 1;
        bool cmp_data = (state.dbc>>18) & 1;
        bool cmp_phase = (state.dbc>>17) & 1;
        // wait_valid can be safely ignored, phases are always valid in this ideal world...
        // bool wait_valid = (state.dbc>>16) & 1;
        int cmp_mask = (state.dbc>>8) & 0xff;
        int cmp_dat = (state.dbc>>0) & 0xff;

        bool do_it;

        if (relative)
        {
          printf("SYM: Don't understand relative transfer control yet!\n");
          exit(1);
        }

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
            if (((state.sfbr & ~cmp_mask)==(cmp_dat & ~cmp_mask)) != jump_if)
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
          printf("jump %x\n",state.dsps);
          if (do_it)
          {
            printf("SYM: Jumping...\n");
            state.dsp = state.dsps;
          }
          return 0;
          break;
        case 1:
          printf("call %d\n",state.dsps);
          if (do_it)
          {
            printf("SYM: Calling...\n");
            state.temp = state.dsp;
            state.dsp = state.dsps;
          }
          return 0;
          break;
        case 2:
          printf("return %d\n",state.dsps);
          if (do_it)
          {
            printf("SYM: Returning...\n");
            state.dsp = state.temp;
          }
          return 0;
          break;
        case 3:
          printf("interrupt%s.\n",interrupt_fly?" on the fly":"");
          if (do_it)
          {
            printf("SYM: Interrupting...\n");

            if (!interrupt_fly)
            {
              state.executing = false;
              state.dstat.sir = true;
              state.istat.dip = true;
              do_pci_interrupt(0,true);
            }
            else
            {
              state.istat.intf = true;
              do_pci_interrupt(0,true);
            }
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
      }
    }
    return 1;
  }

  return 0;
}

void CSym53C895::select_target(int target)
{
  state.target = target;
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
  state.sfbr = retval;
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

  printf("SYM.%d: %d-byte command ",state.target,PT.cmd_len);
  for (int x=0;x<PT.cmd_len;x++) printf("%02x ",PT.cmd[x]);
  printf("\n");

  if (PT.cmd_len<1)
    return 0;

  switch(PT.cmd[0])
  {
  case SCSICMD_TEST_UNIT_READY:
    printf("SYM.%d: TEST UNIT READY.\n",state.target);
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
    printf("SYM.%d: INQUIRY.\n",state.target);
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
    printf("SYM.%d: MODE SENSE.\n",state.target);
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

    printf("SYM.%d: MODE SELECT.\n",state.target);

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
    printf("SYM.%d: READ CAPACITY.\n",state.target);
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
    printf("SYM.%d: READ.\n",state.target);
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

	printf("SYM.%d READ  ofs=%d size=%d\n", state.target, ofs, retlen);

	break;

  default:
    printf("SYM: Unknown SCSI command 0x%02x.\n",PT.cmd[0]);
    exit(1);


  }

  return 0;
}