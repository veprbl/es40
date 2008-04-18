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
 * $Id: DMA.cpp,v 1.6 2008/04/18 09:56:20 iamcamiel Exp $
 *
 * X-1.6        Brian Wheeler                                   18-APR-2008
 *      Rewrote DMA code to make it ready for floppy support.
 *
 * X-1.5        Camiel Vanderhoeven                             14-MAR-2008
 *      Formatting.
 *
 * X-1.4        Camiel Vanderhoeven                             14-MAR-2008
 *   1. More meaningful exceptions replace throwing (int) 1.
 *   2. U64 macro replaces X64 macro.
 *
 * X-1.3        Camiel Vanderhoeven                             05-MAR-2008
 *      Multi-threading version.
 *
 * X-1.2        Brian Wheeler                                   26-FEB-2008
 *      Debugging statements conditionalized.
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
CDMA::CDMA(CConfigurator* cfg, CSystem* c) : CSystemComponent(cfg, c)
{
  int i;

  // DMA Setup
#define LEGACY_IO(id,port,size) c->RegisterMemory(this, id, U64(0x00000801fc000000 | port), size)
  LEGACY_IO(DMA0_IO_MAIN,0x08,8);
  LEGACY_IO(DMA1_IO_MAIN,0xd0,16); 
  LEGACY_IO(DMA0_IO_CHANNEL,0x00,8);
  LEGACY_IO(DMA1_IO_CHANNEL,0xc0,16);
  LEGACY_IO(DMA_IO_LPAGE,0x80,16);
  LEGACY_IO(DMA_IO_HPAGE,0x480,16); //? where is this documented?

  for(i = 0; i < 8; i++)
  {
    state.channel[i].c_lobyte = true;
    state.channel[i].a_lobyte = true;
  }
  state.controller[0].mask = 0xff;
  state.controller[1].mask = 0xff;
 
  printf("dma: $Id: DMA.cpp,v 1.6 2008/04/18 09:56:20 iamcamiel Exp $\n");
}

/**
 * Destructor.
 **/
CDMA::~CDMA()
{ }
int CDMA::DoClock()
{
  return 0;
}

u64 CDMA::ReadMem(int index, u64 address, int dsize)
{
  u64 ret;
  u8  data;
  int num;
  switch(dsize)
  {
  case 32:
    ret = ReadMem(index, address, 8);
    ret |= ReadMem(index, address + 1, 8) << 8;
    ret |= ReadMem(index, address + 2, 8) << 16;
    ret |= ReadMem(index, address + 3, 8) << 32;
    return ret;

  case 16:
    ret = ReadMem(index, address, 8);
    ret |= ReadMem(index, address + 1, 8) << 8;
    return ret;

  case 8:
    if(index == DMA1_IO_CHANNEL || index == DMA1_IO_MAIN)
      address >>=1;

    switch(index)
    {
    case DMA0_IO_CHANNEL:
    case DMA1_IO_CHANNEL:
      num = ((address & 0x0e)>>1)+(index*4);
      if(address & 1)
      {
	// word count registers
	data = (state.channel[num].count >> (state.channel[num].c_lobyte?8:0)) & 0xff;
	state.channel[num].c_lobyte = ! state.channel[num].c_lobyte;	
      }
      else
      {
	// base address
	data = (state.channel[num].current >> (state.channel[num].a_lobyte?8:0)) & 0xff;
	state.channel[num].a_lobyte = !state.channel[num].a_lobyte;
      }
      break;

    case DMA0_IO_MAIN:
    case DMA1_IO_MAIN:
      for(int i = 0; i< 4; i++) 
	data |= ((state.channel[(num * 4) + i].count == state.channel[(num * 4) + 1].current)? 1 : 0) << i;
      data |= (state.controller[num].request & 0x0f) << 4;
      break;

    default:
      FAILURE(InvalidArgument, "dma: ReadMem index out of range");
    }

#if defined(DEBUG_DMA)
    printf("dma: read %d,%02x: %02x.   \n", index, address, data);
#endif
    return data;
  }
}

void CDMA::WriteMem(int index, u64 address, int dsize, u64 data)
{
  int num = 0;
  int channelmap[] = { 0xff, 2, 3, 1, 0xff, 0xff, 0xff, 0, 
		       0xff, 6, 7, 5, 0xff, 0xff, 0xff, 4 };
  switch(dsize)
  {
  case 32:
    WriteMem(index, address + 0, 8, (data >> 0) & 0xff);
    WriteMem(index, address + 1, 8, (data >> 8) & 0xff);
    WriteMem(index, address + 2, 8, (data >> 16) & 0xff);
    WriteMem(index, address + 3, 8, (data >> 24) & 0xff);
    return;

  case 16:
    WriteMem(index, address + 0, 8, (data >> 0) & 0xff);
    WriteMem(index, address + 1, 8, (data >> 8) & 0xff);
    return;

  case 8:
    data &= 0xff;
    if(index == DMA1_IO_CHANNEL || index == DMA1_IO_MAIN)
      address >>=1;

#if defined(DEBUG_DMA)
    printf("dma: write %d,%02x: %02x.   \n", index, address, data);
#endif
    switch(index)
    {
    case DMA0_IO_CHANNEL:
    case DMA1_IO_CHANNEL:
      num = ((address & 0x0e)>>1)+(index*4);
      if(address & 1)
      {
	if(state.channel[num].c_lobyte) 
	  state.channel[num].count = (state.channel[num].count & 0xff00) | data;
	else
	  state.channel[num].count = (state.channel[num].count & 0xff) | (data << 8);
	state.channel[num].c_lobyte = ! state.channel[num].c_lobyte;
      } else {
	if(state.channel[num].a_lobyte) 
	  state.channel[num].base = (state.channel[num].base & 0xff00) | data;
	else
	  state.channel[num].base = (state.channel[num].base & 0xff) | (data << 8);
	state.channel[num].a_lobyte = ! state.channel[num].a_lobyte;
      }
      break;


    case DMA1_IO_MAIN:
      num = 1;
    case DMA0_IO_MAIN:
      switch(address) {
      case 0: // command
	state.controller[num].command = data;
	break;

      case 1: // request
	set_request(num,data & 0x03,  (data & 0x04) >> 2);
	break;

      case 2: // single mask
	state.controller[num].mask = (state.controller[num].mask & ~(1 << (data & 0x03))) | ((data & 0x04)>>2);
	do_dma();
	break;

      case 3: // mode register
	state.channel[(num * 4) + (data & 0x03)].mode = data;
	break;

      case 4: // clear flipflop(s)
	for(int i = (num * 4); i < ((num+1) * 4) ; i++)
	  state.channel[i].a_lobyte = state.channel[i].c_lobyte = true;
	break;

      case 5: // master reset
#if defined(DEBUG_DMA)
        printf("DMA-I-RESET: DMA %d reset.", index);
#endif
	for(int i = (num * 4); i < ((num+1) * 4) ; i++)
	  state.channel[i].a_lobyte = state.channel[i].c_lobyte = true;
	state.controller[num].mask = 0xff;
	break;

      case 6: // master enable
	state.controller[num].mask = 0x00;
	do_dma();
	break;


      case 7: // master mask
        state.controller[num].mask = data;
	do_dma();
        break;
      }
      break;

    case DMA_IO_LPAGE:
    case DMA_IO_HPAGE:
      if(channelmap[address] == 0xff) {
	printf("dma: unknown page register %x\n", address);
	return;
      }
      if(index==DMA_IO_LPAGE) 
        state.channel[num].pagebase = (state.channel[num].pagebase & 0xff00) | (u8) data;
      else
        state.channel[num].pagebase = (state.channel[num].pagebase & 0xff) | (data << 8);
      break;

    default:
      FAILURE(InvalidArgument, "dma: WriteMem index out of range");
    }

    return;
  }
}

static u32  dma_magic1 = 0x65324387;
static u32  dma_magic2 = 0x24092875;

/**
 * Save state to a Virtual Machine State file.
 **/
int CDMA::SaveState(FILE* f)
{
  long  ss = sizeof(state);

  fwrite(&dma_magic1, sizeof(u32), 1, f);
  fwrite(&ss, sizeof(long), 1, f);
  fwrite(&state, sizeof(state), 1, f);
  fwrite(&dma_magic2, sizeof(u32), 1, f);
  printf("dma: %d bytes saved.\n", ss);
  return 0;
}

/**
 * Restore state from a Virtual Machine State file.
 **/
int CDMA::RestoreState(FILE* f)
{
  long    ss;
  u32     m1;
  u32     m2;
  size_t  r;

  r = fread(&m1, sizeof(u32), 1, f);
  if(r != 1)
  {
    printf("dma: unexpected end of file!\n");
    return -1;
  }

  if(m1 != dma_magic1)
  {
    printf("dma: MAGIC 1 does not match!\n");
    return -1;
  }

  fread(&ss, sizeof(long), 1, f);
  if(r != 1)
  {
    printf("dma: unexpected end of file!\n");
    return -1;
  }

  if(ss != sizeof(state))
  {
    printf("dma: STRUCT SIZE does not match!\n");
    return -1;
  }

  fread(&state, sizeof(state), 1, f);
  if(r != 1)
  {
    printf("dma: unexpected end of file!\n");
    return -1;
  }

  r = fread(&m2, sizeof(u32), 1, f);
  if(r != 1)
  {
    printf("dma: unexpected end of file!\n");
    return -1;
  }

  if(m2 != dma_magic2)
  {
    printf("dma: MAGIC 1 does not match!\n");
    return -1;
  }

  printf("dma: %d bytes restored.\n", ss);
  return 0;
}



void CDMA::set_request(int num, int channel, int data) {
  state.controller[num].request = (state.controller[num].request & ~(1 << (data & 0x03))) | ((data & 0x04)>>2);
  state.channel[(num * 4) + (data & 0x03)].current = 0;
  do_dma();
}


void CDMA::do_dma() {
  for(int ctrlr = 0 ; ctrlr < 2; ctrlr++) {
    if(state.controller[ctrlr].command & 0x04 == 0) { // controller not disabled.
      for(int chnl = 0; chnl < 4; chnl++) {
	if((state.controller[ctrlr].mask & (1 << chnl)) == 0) { // channel not masked
	  if(state.controller[ctrlr].request & (1 << chnl)) { // channel has request
	    
	  }
	}
      }
    }
  }
}
