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
 * Contains the code for the emulated DMA controller.
 *
 * $Id: DMA.cpp,v 1.1 2008/02/26 11:22:15 iamcamiel Exp $
 *
 * X-1.1        Camiel Vanderhoeven                             26-FEB-2008
 *      Created. Contains code previously found in AliM1543C.cpp
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#include "StdAfx.h"
#include "System.h"
#include "DMA.h"
#include "AliM1543C.h"

/**
 * Constructor.
 **/

CDMA::CDMA(CConfigurator * cfg, CSystem * c) : CSystemComponent(cfg,c)
{
  int i;

  // DMA Setup
  c->RegisterMemory(this, 0, X64(00000801fc000000), 16); // dma 0-3
  c->RegisterMemory(this, 1, X64(00000801fc0000c0),32); // dma 4-7
  c->RegisterMemory(this, 2, X64(00000801fc000080),16); // dma 0-7 (memory base low page register)
  c->RegisterMemory(this, 3, X64(00000801fc000480),16); // dma 0-7 (memory base high page register)
  for(i=0;i<8;i++) {
    state.channel[i].c_lobyte=true;
    state.channel[i].a_lobyte=true;
  }

  c->RegisterClock(this, true);

  printf("dma: $Id: DMA.cpp,v 1.1 2008/02/26 11:22:15 iamcamiel Exp $\n");
}

/**
 * Destructor.
 **/

CDMA::~CDMA()
{
}

int CDMA::DoClock()
{
  return 0;
}

u64 CDMA::ReadMem(int index, u64 address, int dsize)
{
  u64 ret;
  u8 data;
  int num;
  switch (dsize)
  {
  case 32:
    ret = ReadMem(index,address,8);
    ret |= ReadMem(index,address+1,8) << 8;
    ret |= ReadMem(index,address+2,8) << 16;
    ret |= ReadMem(index,address+3,8) << 32;
    return ret;
  case 16:
    ret = ReadMem(index,address,8);
    ret |= ReadMem(index,address+1,8) << 8;
    return ret;
  case 8:
    if (index==1)
      address >>= 1;
    switch (index)
    {
    case 0:
    case 1:
      switch(address) {
      case 0x00: // base address
      case 0x02: 
      case 0x04:
      case 0x06:
        num = ((int)address/2)+(index*4);
        if(state.channel[num].a_lobyte) {
          data = state.channel[num].current & 0xff;
          state.channel[num].a_lobyte=false;
        } else {
          data = (state.channel[num].current>>8) & 0xff;
          state.channel[num].a_lobyte=true;
        }
        break;
      
      case 0x01: // word count
      case 0x03:
      case 0x05:
      case 0x07:
        num = (((int)address-1)/2)+(index*4);
        if(state.channel[num].c_lobyte) {
          data = state.channel[num].count & 0xff;
          state.channel[num].c_lobyte=false;
        } else {
          data = (state.channel[num].count>>8) & 0xff;
          state.channel[num].c_lobyte=true;
        }
        break;

      default:
        data = 0;
//        FAILURE("dma: don't know what to do.\n");
      }
      break;
    case 2:
    case 3:
      data = 0;
      break;
    default:
      FAILURE("dma: ReadMem index out of range");
    }
    printf("dma: read %d,%02x: %02x.   \n",index,address,data);
    return data;
  }
}

void CDMA::WriteMem(int index, u64 address, int dsize, u64 data)
{
  int num;
  switch (dsize)
  {
  case 32:
    WriteMem(index,address+0, 8, (data>>0 )&0xff);
    WriteMem(index,address+1, 8, (data>>8 )&0xff);
    WriteMem(index,address+2, 8, (data>>16)&0xff);
    WriteMem(index,address+3, 8, (data>>24)&0xff);
    return;
  case 16:
    WriteMem(index,address+0, 8, (data>>0 )&0xff);
    WriteMem(index,address+1, 8, (data>>8 )&0xff);
    return;
  case 8:
    if (index==1)
      address >>= 1;
    printf("dma: write %d,%02x: %02x.   \n",index,address,data);
    switch (index) {
    case 0:
    case 1:
      switch(address) {
      case 0x00: // base address
      case 0x02: 
      case 0x04:
      case 0x06:
        num = ((int)address/2)+(index*4);
        if(state.channel[num].a_lobyte) {
          state.channel[num].base = (u8)data;
          state.channel[num].a_lobyte=false;
        } else {
          state.channel[num].base |= ((u8)data << 8);
          state.channel[num].current=state.channel[num].base;
          state.channel[num].a_lobyte=true;
        }
        break;
      
      case 0x01: // word count
      case 0x03:
      case 0x05:
      case 0x07:
        num = (((int)address-1)/2)+(index*4);
        if(state.channel[num].c_lobyte) {
          state.channel[num].count = (u8)data;
          state.channel[num].c_lobyte=false;
        } else {
          state.channel[num].count |= ((u8)data << 8);
          state.channel[num].c_lobyte=true;
        }
        break;

      //case 0x08: // command register (2)
      //  /*
      //  Bit(s)  Description     (Table P0002)
      //  7      DACK sense active high
      //  6      DREQ sense active high
      //  5      =1 extended write selection
      //    =0 late write selection
      //  4      rotating priority instead of fixed priority
      //  3      compressed timing (two clocks instead of four per transfer)
      //    =1 normal timing (default)
      //    =0 compressed timing
      //  2      =1 enable controller
      //    =0 enable memory-to-memory
      //  1-0    channel number
      //   */
      //  /* we'll actually do the DMA here. */

      //  break;

      case 0x09: // write request register (3)
        /*
        Bit(s)  Description     (Table P0003)
        7-3    reserved (0)
        2      =0 clear request bit
          =1 set request bit
        1-0    channel number
          00 channel 0 select
          01 channel 1 select
          10 channel 2 select
          11 channel 3 select
         */
        state.controller[index].writereq=data;
        break;

      case 0x0a: // mask register (4)
        /*
        Bit(s)  Description     (Table P0004)
        7-3    reserved (0)
        2      =0 clear mask bit
          =1 set mask bit
        1-0    channel number
          00 channel 0 select
          01 channel 1 select
          10 channel 2 select
          11 channel 3 select
         */
        state.controller[index].mask=data;
        break;

      case 0x0b: // mode register (5)
        /*
  Bit(s)  Description     (Table P0005)
   7-6    transfer mode
          00 demand mode
          01 single mode
          10 block mode
          11 cascade mode
   5      direction
          =0 increment address after each transfer
          =1 decrement address
   3-2    operation
          00 verify operation
          01 write to memory
          10 read from memory
          11 reserved
   1-0    channel number
          00 channel 0 select
          01 channel 1 select
          10 channel 2 select
          11 channel 3 select
         */
        state.controller[index].mode=data;
        break;

      case 0x0c: // clear flip/flop:
        for (num=index*4; num<(index*4)+3; num++)
        {
          state.channel[num].a_lobyte = true;
          state.channel[num].c_lobyte = true;
        }
        break;

      //case 0x0d: // dma channel master clear register
      //  printf("DMA-I-RESET: DMA %d reset.",index);
      //  break;

  //    case 0x0e: // clear mask register
  //      break;

  //    case 0x0f: // write mask register (6)
  //      /*      
  //Bit(s)  Description     (Table P0006)
  // 7-4    reserved
  // 3      channel 3 mask bit
  // 2      channel 2 mask bit
  // 1      channel 1 mask bit
  // 0      channel 0 mask bit
  //Note:   each mask bit is automatically set when the corresponding channel
  //          reaches terminal count or an extenal EOP sigmal is received
  //      */
  //      break;

      default:
        FAILURE("dma: don't know what to do.\n");
      }
      break;

    case 2:
    case 3:
      switch(address) {
      case 1:
        num=2;
        break;
      case 2:
        num=3;
        break;
      case 3:
        num=1;
        break;
      case 7:
        num=0;
        break;
      case 9:
        num=6;
        break;
      case 0xa:
        num=7;
        break;
      case 0xb:
        num=5;
        break;
      default:
        printf("dma: Unknown page register: %x\n",address);
        return;
      }
      if (index==2)
        state.channel[num].pagebase = (state.channel[num].pagebase & 0xff00) | (u8)data;
      else
        state.channel[num].pagebase = (state.channel[num].pagebase & 0xff) | (data<<8);

//      FAILURE("dma: don't know what to do.\n");
      break;
    default:
      FAILURE("dma: ReadMem index out of range");
    }
    return;
  }
}

static u32 dma_magic1 = 0x65324387;
static u32 dma_magic2 = 0x24092875;

/**
 * Save state to a Virtual Machine State file.
 **/

int CDMA::SaveState(FILE *f)
{
  long ss = sizeof(state);

  fwrite(&dma_magic1,sizeof(u32),1,f);
  fwrite(&ss,sizeof(long),1,f);
  fwrite(&state,sizeof(state),1,f);
  fwrite(&dma_magic2,sizeof(u32),1,f);
  printf("dma: %d bytes saved.\n",ss);
  return 0;
}

/**
 * Restore state from a Virtual Machine State file.
 **/

int CDMA::RestoreState(FILE *f)
{
  long ss;
  u32 m1;
  u32 m2;
  size_t r;

  r = fread(&m1,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("dma: unexpected end of file!\n");
    return -1;
  }
  if (m1 != dma_magic1)
  {
    printf("dma: MAGIC 1 does not match!\n");
    return -1;
  }

  fread(&ss,sizeof(long),1,f);
  if (r!=1)
  {
    printf("dma: unexpected end of file!\n");
    return -1;
  }
  if (ss != sizeof(state))
  {
    printf("dma: STRUCT SIZE does not match!\n");
    return -1;
  }

  fread(&state,sizeof(state),1,f);
  if (r!=1)
  {
    printf("dma: unexpected end of file!\n");
    return -1;
  }

  r = fread(&m2,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("dma: unexpected end of file!\n");
    return -1;
  }
  if (m2 != dma_magic2)
  {
    printf("dma: MAGIC 1 does not match!\n");
    return -1;
  }

  printf("dma: %d bytes restored.\n",ss);
  return 0;
}
