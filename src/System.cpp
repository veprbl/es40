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
 * Contains the code for the emulated Typhoon Chipset devices.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#include "StdAfx.h"
#include "System.h"
#include "AlphaCPU.h"
#include "lockstep.h"

#include <ctype.h>
#include <stdlib.h>

/**
 * Constructor.
 **/

CSystem::CSystem(const char *filename)
{
  this->LoadConfig(filename);

  iNumComponents = 0;
  iNumFastClocks = 0;
  iNumSlowClocks = 0;
  iNumMemories = 0;
  iNumCPUs = 0;
  iNumROMs = 0;
  iNumMemoryBits = atoi(this->GetConfig("memory.bits","27"));

#if defined(IDB)
  iSingleStep = 0;
  iSSCycles = 0;
#endif

  c_MISC = X64(0000000800000000);
  c_DIM[0] = 0;
  c_DIM[1] = 0;
  c_DIM[2] = 0;
  c_DIM[3] = 0;
  c_DRIR = 0;
  c_CSC = X64(0042444014157803);
  c_TRR = X64(0000000000003103);
  c_TDR = X64(F37FF37FF37FF37F);

  d_STR = X64(2525252525252525);

  p_PCTL[0] = X64(0000104401440081);
  p_PCTL[1] = X64(0000504401440081);

  p_PERRMASK[0] = 0;
  p_PERRMASK[1] = 0;
  p_PLAT[0] = 0;
  p_PLAT[1] = 0;
  p_TBA [0][0] = 0;
  p_WSBA[0][0] = 0;
  p_WSM [0][0] = 0;
  p_TBA [0][1] = 0;
  p_WSBA[0][1] = 0;
  p_WSM [0][1] = 0;
  p_TBA [0][2] = 0;
  p_WSBA[0][2] = 0;
  p_WSM [0][2] = 0;
  p_TBA [0][3] = 0;
  p_WSBA[0][3] = 0;
  p_WSM [0][3] = 0;
  p_TBA [1][0] = 0;
  p_WSBA[1][0] = 0;
  p_WSM [1][0] = 0;
  p_TBA [1][1] = 0;
  p_WSBA[1][1] = 0;
  p_WSM [1][1] = 0;
  p_TBA [1][2] = 0;
  p_WSBA[1][2] = 0;
  p_WSM [1][2] = 0;
  p_TBA [1][3] = 0;
  p_WSBA[1][3] = 0;
  p_WSM [1][3] = 0;


  tig_FwWrite = 0;
  tig_HaltA   = 0;
  tig_HaltB   = 0;

  memory = calloc(1<<iNumMemoryBits,1);

  printf("%%TYP-I-INIT: 21272 Typhoon chipset emulator initialized.\n");
  printf("%%TYP-I-CONF: 21272 Typhoon config: 1 Cchip, 8 DChip, 2 PChip.\n");
}

/**
 * Destructor.
 **/

CSystem::~CSystem()
{
  int i;
  for (i=0;i<iNumComponents;i++)
    delete acComponents[i];
}

void CSystem::ResetMem(unsigned int membits) {
  free(memory);
  iNumMemoryBits=membits;
  memory = calloc(1<<iNumMemoryBits,1);
}

int CSystem::RegisterComponent(CSystemComponent *component)
{
  acComponents[iNumComponents] = component;
  iNumComponents++;
  return 0;
}

unsigned int CSystem::get_memory_bits()
{
  return iNumMemoryBits;
}

char * CSystem::PtrToMem(u64 address)
{
  return &(((char*)memory)[(int)address]);
}

int CSystem::RegisterCPU(class CAlphaCPU * cpu)
{
  if (iNumCPUs>=4)
    return -1;
  acCPUs[iNumCPUs] = cpu;
  iNumCPUs++;
  return iNumCPUs - 1;
}

int CSystem::RegisterClock(CSystemComponent *component, bool slow)
{
  if (slow)
  {
    acSlowClocks[iNumSlowClocks] = component;
    iNumSlowClocks++;
  }
  else
  {
    acFastClocks[iNumFastClocks] = component;
    iNumFastClocks++;
  }
  return 0;
}

int CSystem::RegisterMemory(CSystemComponent *component, int index, u64 base, u64 length)
{
  struct SMemoryUser * m;
  m = (struct SMemoryUser*) malloc(sizeof(struct SMemoryUser));
  if (m)
    {
      m->component = component;
      m->base = base;
      m->length = length;
      m->index = index;

      asMemories[iNumMemories] = m;
      iNumMemories++;
      return 0;
    }
  else
    return -1;

}

int CSystem::Run()
{
  int i,j,k;
  int result;

  for(k=0;;k++)
  {
    for(j=0;j<10000;j++)
    {
      for(i=0;i<iNumFastClocks;i++)
      {
        result = acFastClocks[i]->DoClock();
	if (result)
	  return result;
      }
    }
	 
    for(i=0;i<iNumSlowClocks;i++)
    {
      result = acSlowClocks[i]->DoClock();
      if (result)
	return result;
    }
    printf("%d0000 | %016" LL "x\r",k,acCPUs[0]->get_pc());
  }
}

#if defined(IDB)
int CSystem::SingleStep()
{
  int i;
  int result;

#if defined(LS_MASTER) || defined(LS_SLAVE)
  lockstep_sync_m2s("sync1");
  lockstep_sync_s2m("sync2");
#endif

  for(i=0;i<iNumFastClocks;i++)
  {
    result = acFastClocks[i]->DoClock();
    if (result)
      return result;
  }

  iSingleStep++;

  if (iSingleStep >= 10000)
  {
     iSingleStep = 0;
     for(i=0;i<iNumSlowClocks;i++)
     {
        result = acSlowClocks[i]->DoClock();
	if (result)
	  return result;
     }
     iSSCycles++;
     if (bHashing)
       printf("%d0000 | %016" LL "x\r",iSSCycles,acCPUs[0]->get_pc());
  }
  return 0;
}
#endif

void CSystem::WriteMem(u64 address, int dsize, u64 data)
{
  u64 a;
  int i;
  u8 * p;
#if defined(ALIGN_MEM_ACCESS)
  u64 t64;
  u32 t32;
  u16 t16;
#endif;

  a = address & X64(00000fffffffffff);

  if (a>>iNumMemoryBits)
    {
      // non-memory...
      for(i=0;i<iNumMemories;i++) {
	if (   (a >= asMemories[i]->base)
	       && (a < asMemories[i]->base + asMemories[i]->length) ) {
	  asMemories[i]->component->WriteMem(asMemories[i]->index, a-asMemories[i]->base, dsize, data);
	  return;
	}
      }

      switch (a)
	{
        case X64(00000801a0000000): // CSC
	  c_CSC        &= ~X64(0777777fff3f0000);
	  c_CSC |= (data & X64(0777777fff3f0000));
	  return;
	case X64(00000801a0000080): // MISC
	  c_MISC |= (data & X64(00000f0000f0f000));	// W1S
	  c_MISC &=~(data & X64(0000000010000ff0));	// W1C
	  if       (data & X64(0000000001000000))
	    c_MISC &=~        X64(0000000000ff0000);	//Arbitration Clear
	  if    (!(c_MISC & X64(00000000000f0000)))
	    c_MISC |= (data & X64(00000000000f0000));	//Arbitration try

	  // stop interval timer interrupt
	  if        (data & X64(00000000000000f0))
	    {
	      for (i=0;i<iNumCPUs;i++)
		{
		  if (data & (0x10<<i))
                    {
		      acCPUs[i]->irq_h(2,false);
		      //                        printf("*** TIMER interrupt cleared for CPU %d\n",i);
                    }
		}
	    }			
	  return;

	case X64(00000801a0000200):
	  c_DIM[0] = data;
	  return;
	case X64(00000801a0000240):
	  c_DIM[1] = data;
	  return;
	case X64(00000801a0000600):
	  c_DIM[2] = data;
	  return;
	case X64(00000801a0000640):
	  c_DIM[3] = data;
	  return;

        case X64(0000080180000000):
	  p_WSBA[0][0] = data & X64(00000000fff00003);
	  return;
        case X64(0000080180000040):
	  p_WSBA[0][1] = data & X64(00000000fff00003);
	  return;
        case X64(0000080180000080):
	  p_WSBA[0][2] = data & X64(00000000fff00003);
	  return;
        case X64(00000801800000c0):
	  p_WSBA[0][3] = data & X64(00000080fff00003);
	  return;
        case X64(0000080380000000):
	  p_WSBA[1][0] = data & X64(00000000fff00003);
	  return;
        case X64(0000080380000040):
	  p_WSBA[1][1] = data & X64(00000000fff00003);
	  return;
        case X64(0000080380000080):
	  p_WSBA[1][2] = data & X64(00000000fff00003);
	  return;
        case X64(00000803800000c0):
	  p_WSBA[1][3] = data & X64(00000080fff00003);
	  return;
        case X64(0000080180000100):
	  p_WSM[0][0] = data & X64(00000000fff00000);
	  return;
        case X64(0000080180000140):
	  p_WSM[0][1] = data & X64(00000000fff00000);
	  return;
        case X64(0000080180000180):
	  p_WSM[0][2] = data & X64(00000000fff00000);
	  return;
        case X64(00000801800001c0):
	  p_WSM[0][3] = data & X64(00000000fff00000);
	  return;
        case X64(0000080380000100):
	  p_WSM[1][0] = data & X64(00000000fff00000);
	  return;
        case X64(0000080380000140):
	  p_WSM[1][1] = data & X64(00000000fff00000);
	  return;
        case X64(0000080380000180):
	  p_WSM[1][2] = data & X64(00000000fff00000);
	  return;
        case X64(00000803800001c0):
	  p_WSM[1][3] = data & X64(00000000fff00000);
	  return;
        case X64(0000080180000200):
	  p_TBA[0][0] = data & X64(00000007fffffc00);
	  return;
        case X64(0000080180000240):
	  p_TBA[0][1] = data & X64(00000007fffffc00);
	  return;
        case X64(0000080180000280):
	  p_TBA[0][2] = data & X64(00000007fffffc00);
	  return;
        case X64(00000801800002c0):
	  p_TBA[0][3] = data & X64(00000007fffffc00);
	  return;
        case X64(0000080380000200):
	  p_TBA[1][0] = data & X64(00000007fffffc00);
	  return;
        case X64(0000080380000240):
	  p_TBA[1][1] = data & X64(00000007fffffc00);
	  return;
        case X64(0000080380000280):
	  p_TBA[1][2] = data & X64(00000007fffffc00);
	  return;
        case X64(00000803800002c0):
	  p_TBA[1][3] = data & X64(00000007fffffc00);
	  return;
	case X64(0000080180000300):
	  p_PCTL[0] &=         X64(ffffe300f0300000);
	  p_PCTL[0] |= (data & X64(00001cff0fcfffff));
	  return;
	case X64(0000080380000300):
	  p_PCTL[1] &=         X64(ffffe300f0300000);
	  p_PCTL[1] |= (data & X64(00001cff0fcfffff));
	  return;
	case X64(0000080180000340):
	  p_PLAT[0] = data;
	  return;
	case X64(0000080380000340):
	  p_PLAT[1] = data;
	  return;		
	case X64(0000080180000400):
	  p_PERRMASK[0] = data;
	  return;
	case X64(0000080380000400):
	  p_PERRMASK[1] = data;
	  return;
	case X64(00000801300003c0):
	  tig_HaltA = (u8)(data&0xff);
	  return;
	case X64(00000801300005c0):
	  tig_HaltB = (u8)(data&0xff);
	  return;
	case X64(0000080130000040):
	  tig_FwWrite = (u8)(data&0xff);
	  return;
	case X64(0000080130000100):
	  // soft reset
	  printf("Soft reset: %02x\n",(int)data);
	  return;
	  //PERROR registers
	case X64(00000801800003c0):
	case X64(00000803800003c0):
	  //TLBIA
	case X64(00000801800004c0):
	case X64(00000803800004c0):
	  // PCI reset
	case X64(0000080180000800):
	case X64(0000080380000800):
	  return;
	default:
#ifdef DEBUG_UNKMEM
	printf("%%MEM-I-WRUNKNWN: Attempt to write %d bytes (%016" LL "x) from unknown addres %011" LL "x\n",dsize/8,data,a);
#endif
	  return;
	}
    }


  p = (u8*)memory + a;


  switch(dsize)
    {
    case 8:
      *((u8 *) p) = (u8) data;
      break;
    case 16:
#if defined(ALIGN_MEM_ACCESS)
      if (address&1)
      {
        t64 = endian_64( (u64) data );
        *p     = (t64 >> 8) & 0xff;
        *(p+1) =  t64       & 0xff;
      }  
      else
#endif
      *((u16 *) p) = endian_16( (u16) data );
      break;
    case 32:
#if defined(ALIGN_MEM_ACCESS)
      if (address&3)
      {
        t32 = endian_32( (u32) data );
        *p     = (t32 >> 24) & 0xff;
        *(p+1) = (t32 >> 16) & 0xff;
        *(p+2) = (t32 >>  8) & 0xff;
        *(p+3) =  t32        & 0xff;
      }  
      else
#endif
        *((u32 *) p) = endian_32( (u32) data );
      break;
    default:
#if defined(ALIGN_MEM_ACCESS)
      if (address&7)
      {
        t64 = endian_64( (u64) data );
        *p     = (t64 >> 56) & 0xff;
        *(p+1) = (t64 >> 48) & 0xff;
        *(p+2) = (t64 >> 40) & 0xff;
        *(p+3) = (t64 >> 32) & 0xff;
        *(p+4) = (t64 >> 24) & 0xff;
        *(p+5) = (t64 >> 16) & 0xff;
        *(p+6) = (t64 >>  8) & 0xff;
        *(p+7) =  t64        & 0xff;
      }  
      else
#endif
        *((u64 *) p) = endian_64( (u64) data );
    }
}

u64 CSystem::ReadMem(u64 address, int dsize)
{
  u64 a;
  int i;
  u8 * p;

  a = address & X64(00000fffffffffff);
  if (a>>iNumMemoryBits)
    {
      // Non memory
      for(i=0;i<iNumMemories;i++) {
	if (   (a >= asMemories[i]->base)
	       && (a < asMemories[i]->base + asMemories[i]->length) ) {
	  return asMemories[i]->component->ReadMem(asMemories[i]->index, a-asMemories[i]->base, dsize);
	}
      }

      switch (a)
	{

	  // PError registers
	case X64(00000801800003c0):
	case X64(00000803800003c0):

        case X64(00000801a0000000):
	  return c_CSC;
	case X64(00000801a0000080):
	  return c_MISC;
	case X64(00000801a0000100):
	  return   ((iNumMemoryBits-23)<<12); //size
	case X64(00000801a0000140):
	case X64(00000801a0000180):
	case X64(00000801a00001c0):
	  return 0;
	  // WE PUT ALL OUR MEMORY IN A SINGLE ARRAY FOR NOW...
	  // memory arrays
	  //			return   0x300				// twice-split
	  //				   | ((iNumMemoryBits-25)<<12) //size
	  //				   | ( (((a>>6)&3)<<(iNumMemoryBits-2)) & 0x7ff000000);	// address

	case X64(00000801a0000200):
	  return c_DIM[0];
	case X64(00000801a0000240):
	  return c_DIM[1];
	case X64(00000801a0000600):
	  return c_DIM[2];
	case X64(00000801a0000640):
	  return c_DIM[3];
        case X64(00000801a0000280):
	  return c_DRIR & c_DIM[0];
        case X64(00000801a00002c0):
	  return c_DRIR & c_DIM[1];
        case X64(00000801a0000680):
	  return c_DRIR & c_DIM[2];
        case X64(00000801a00006c0):
	  return c_DRIR & c_DIM[3];
        case X64(00000801a0000300):
	  return c_DRIR;

	case X64(0000080180000300):
	  return p_PCTL[0];
	case X64(0000080380000300):
	  return p_PCTL[1];
	case X64(0000080180000400):
	  return p_PERRMASK[0];
	case X64(0000080380000400):
	  return p_PERRMASK[1];

        case X64(0000080180000000):
	  return p_WSBA[0][0];
        case X64(0000080180000040):
	  return p_WSBA[0][1];
        case X64(0000080180000080):
	  return p_WSBA[0][2];
        case X64(00000801800000c0):
	  return p_WSBA[0][3];
        case X64(0000080380000000):
	  return p_WSBA[1][0];
        case X64(0000080380000040):
	  return p_WSBA[1][1];
        case X64(0000080380000080):
	  return p_WSBA[1][2];
        case X64(00000803800000c0):
	  return p_WSBA[1][3];
        case X64(0000080180000100):
	  return p_WSM[0][0];
        case X64(0000080180000140):
	  return p_WSM[0][1];
        case X64(0000080180000180):
	  return p_WSM[0][2];
        case X64(00000801800001c0):
	  return p_WSM[0][3];
        case X64(0000080380000100):
	  return p_WSM[1][0];
        case X64(0000080380000140):
	  return p_WSM[1][1];
        case X64(0000080380000180):
	  return p_WSM[1][2];
        case X64(00000803800001c0):
	  return p_WSM[1][3];
        case X64(0000080180000200):
	  return p_TBA[0][0];
        case X64(0000080180000240):
	  return p_TBA[0][1];
        case X64(0000080180000280):
	  return p_TBA[0][2];
        case X64(00000801800002c0):
	  return p_TBA[0][3];
        case X64(0000080380000200):
	  return p_TBA[1][0];
        case X64(0000080380000240):
	  return p_TBA[1][1];
        case X64(0000080380000280):
	  return p_TBA[1][2];
        case X64(00000803800002c0):
	  return p_TBA[1][3];

	case X64(00000801b0000880):
	  // DCHIP revisions
	  return X64(0101010101010101);

	case X64(0000080138000180):
	  // Arbiter revision
	  return 0xfe;
	case X64(0000080130000040):
	  return tig_FwWrite;
	case X64(00000801300003c0):
	  return tig_HaltA;
	case X64(00000801300005c0):
	  return tig_HaltB;

	default:
#ifdef DEBUG_UNKMEM
	printf("%%MEM-I-RDUNKNWN: Attempt to read %d bytes from unknown addres %011" LL "x\n",dsize/8,a);
#endif
	  return 0;
	  //			return 0x77; // 7f
	}
    }

  p = (u8*)memory + a;

  switch(dsize)
    {
    case 8:
      return *((u8 *) p);
    case 16:
#if defined(ALIGN_MEM_ACCESS)
      if (address&1)
        return endian_16( *p | ( *(p+1) << 8 ) );
      else
#endif
        return endian_16( *((u16 *) p) );
    case 32:
#if defined(ALIGN_MEM_ACCESS)
      if (address&3)
        return endian_32( *p | 
                          ( *(p+1) << 8 )  | 
                          ( *(p+2) << 16 ) | 
                          ( *(p+3) << 24 ) );
      else
#endif
        return endian_32( *((u32 *) p) );
    default:
#if defined(ALIGN_MEM_ACCESS)
      if (address&7)
        return endian_64(   (u64)(*p) | 
                          ( (u64)(*(p+1)) <<  8 ) | 
                          ( (u64)(*(p+2)) << 16 ) | 
                          ( (u64)(*(p+3)) << 24 ) |
                          ( (u64)(*(p+4)) << 32 ) | 
                          ( (u64)(*(p+5)) << 40 ) | 
                          ( (u64)(*(p+6)) << 48 ) | 
                          ( (u64)(*(p+7)) << 56 ) );
      else
#endif
        return endian_64( *((u64 *) p) );
    }
}

int CSystem::load_ROM(char *filename)
{
  FILE * f;
  u32 read_u32;
  int state = 0;
  u64 * buffer;
  u64 * bufptr;
  u64 loadptr;

  u32 scratch;

  struct SROM_data * d;

  f = fopen(filename,"rb");
  if (!f)
    {
      printf("%%TYP-F-OPENROM: Couldn't open ROM file: %s\n", filename);
      return -1;
    }

  printf("%%TYP-I-READROM: Reading ROM images from file: %s\n", filename);

  while (!feof(f))
    {
      fread(&read_u32,1,4,f);
      switch (state)
        {
        case 0:
	  if (read_u32==0x5a5ac3c3)
            {
	      state = 1;
	      //                printf("First magic field found...\n");
            }
	  break;
        case 1:
	  if (read_u32==0xa5a53c3c)
            {
	      //                printf("Second magic field found...\n");
	      d = (struct SROM_data *) malloc(sizeof(struct SROM_data));
	      if (d)
		{
		  d->filename = _strdup(filename);
		  strcpy(d->revision,"        ");

		  fread(&(d->header_size),1,4,f);
		  fread(&(d->cksum),1,4,f);
		  fread(&(d->image_size),1,4,f);
		  fread(&(d->compression),1,4,f);
		  fread(&(d->load_at),1,8,f);
		  if (d->header_size >= 0x34)
		    {
		      fread(&scratch,1,1,f);
		      fread(&d->id,1,1,f);
		      fread(&scratch,1,2,f);
		      fread(&d->romsize,1,4,f);
		      switch (d->id)
			{
			case 0:
			  printf("%%TYP-I-ROMTYPE: Image type:     DBM\n");
			  strcpy(d->tp,"DBM");
			  break;
			case 1:
			  printf("%%TYP-I-ROMTYPE: Image type:     WNT\n");
			  strcpy(d->tp,"WNT");
			  break;
			case 2:
			  printf("%%TYP-I-ROMTYPE: Image type:     SRM\n");
			  strcpy(d->tp,"SRM");
			  break;
			case 6:
			  printf("%%TYP-I-ROMTYPE: Image type:     FSB\n");
			  strcpy(d->tp,"FSB");
			  break;
			default:
			  printf("%%TYP-I-UNKROM: Unknown ROM type:   %d\n",d->id);
			  sprintf(d->tp,"%02x ",d->id);
			}
		    }
		  if (d->compression)
		    {
		      printf("%%TYP-F-ROMCPR: Sorry... I can't handle compressed ROM images (yet)\n");
		      free(d->filename);
		      free(d);
		    }
		  else
		    {
		      buffer = (u64*)malloc(d->image_size);
		      if (buffer)
			{
			  if (d->header_size >= 0x34)
			    fread(buffer,1,d->header_size-40,f);

			  // load it
			  fread(buffer,1,d->image_size,f);
                    
			  // and put it in memory at the right place...
			  bufptr = buffer;
			  for (loadptr=d->load_at; loadptr-d->load_at<d->image_size; loadptr+=8)
			    WriteMem(loadptr,64,*bufptr++);
			  free(buffer);
			  asROMs[iNumROMs++] = d;

			  printf("%%TYP-I-ROMLOADED: ROM Image loaded successfully!\n");
			}
		    }
		}
            }
	  else
	    printf("%%TYP-F-INVROM: Sorry... Invalid ROM.\n");
	  state = 0;
	  break;
        }
    }
  return 0;
}

int CSystem::load_ROM2(char *filename, int start_at, u64 load_at, u8 rom_type)
{
  FILE * f;
  char * buffer;
  int i;

  u32 scratch;

  struct SROM_data * d;

  f = fopen(filename,"rb");
  if (!f)
    {
      printf("%%TYP-F-OPENROM: Couldn't open ROM file: %s\n", filename);
      return -1;
    }

  printf("%%TYP-I-READROM: Reading ROM images from file: %s\n", filename);
  for(i=0;i<start_at;i++)
    {
      if (feof(f)) break;
      fread(&scratch,1,1,f);
    }
  if (!feof(f))
    {
      d = (struct SROM_data *) malloc(sizeof(struct SROM_data));
      if (d)
	{
	  d->filename = _strdup(filename);
	  strcpy(d->revision,"        ");
	  d->header_size = 0;
	  d->cksum = 0;
	  d->image_size = 0;
	  d->compression = 0;
	  d->load_at = load_at;
	  d->id = rom_type;
	  switch (d->id)
            {
            case 0:
	      printf("%%TYP-I-ROMTYPE: Image type:     DBM\n");
	      strcpy(d->tp,"DBM");
	      break;
            case 1:
	      printf("%%TYP-I-ROMTYPE: Image type:     WNT\n");
	      strcpy(d->tp,"WNT");
	      break;
	    case 2:
	      printf("%%TYP-I-ROMTYPE: Image type:     SRM\n");
	      strcpy(d->tp,"SRM");
	      break;
            case 6:
	      printf("%%TYP-I-ROMTYPE: Image type:     FSB\n");
	      strcpy(d->tp,"FSB");
	      break;
	    default:
	      printf("%%TYP-I-UNKROM: Unknown ROM type:   %d\n",d->id);
	      sprintf(d->tp,"%02x ",d->id);
	    }
	  buffer = PtrToMem(load_at);
	  while (!feof(f))
	    {
	      fread(buffer++,1,1,f);
	    }
	  asROMs[iNumROMs++] = d;

	  printf("%%TYP-I-ROMLOADED: ROM Image loaded successfully!\n");
        }
    }
  fclose(f);
  return 0;
}

u64 CSystem::Select_ROM()
{
  int i, result;

  if (!iNumROMs)
    {
      printf("%%TYP-F-NOROM: No ROM image loaded. System cannot boot!\n");
      return 0;
    }

  if (iNumROMs == 1)
    {
      printf("%%TYP-I-ROMSELECT: ROM Image %d selected for boot.\n",0);
      return asROMs[0]->load_at;
    }
    
  printf("==== Please select a ROM image to boot from: ====\n");
  for (i=0;i<iNumROMs;i++)
    printf("%d. %s ROM image from %s\n",i,asROMs[i]->tp,asROMs[i]->filename);
  i=-1;
  while (i<0 || i>iNumROMs-1)
    {
      printf(">>");
      result = scanf("%d",&i);
    }

  printf("%%TYP-I-ROMSELECT: ROM Image %d selected for boot.\n",i);
  return asROMs[i]->load_at;

}

void CSystem::interrupt(int number, bool assert)
{
  int i;

  if (number==-1)
    {
      // timer int...
      c_MISC |= 0xf0;
      for(i=0;i<iNumCPUs;i++)
	acCPUs[i]->irq_h(2,true);
    }
  else if (assert)
    {
      //        if (!(c_DRIR & (1i64<<number)))
      //            printf("%%TYP-I-INTERRUPT: Interrupt %d asserted.\n",number);
      c_DRIR |= (X64(1)<<number);
    }
  else
    {
      //        if (c_DRIR & (1i64<<number))
      //            printf("%%TYP-I-INTERRUPT: Interrupt %d deasserted.\n",number);
      c_DRIR &= ~(X64(1)<<number);
    }
  for (i=0;i<iNumCPUs;i++)
    {
      if (c_DRIR & c_DIM[i] & X64(00ffffffffffffff))
	acCPUs[i]->irq_h(1,true);
      else
	acCPUs[i]->irq_h(1,false);

      if (c_DRIR & c_DIM[i] & X64(fc00000000000000))
	acCPUs[i]->irq_h(0,true);
      else
	acCPUs[i]->irq_h(0,false);
    }

}

void CSystem::PCI_WriteMem(int pcibus, u32 address, int dsize, u64 data)
{
  u64 a;
  int j;

  //    printf("-------------- PCI MEMORY ACCESS FOR PCI HOSE %d --------------\n", pcibus);
  //Step through windows
  //	for(j=0;j<4;j++)
  //	{
  //      printf("WSBA%d: %08x%08x WSM%d: %08x%08x TBA%d: %08x%08x\n",
  //        j,(u32)(p_WSBA[pcibus][j]>>32),(u32)(p_WSBA[pcibus][j]),
  //      j,(u32)(p_WSM[pcibus][j]>>32),(u32)(p_WSM[pcibus][j]),
  //    j,(u32)(p_TBA[pcibus][j]>>32),(u32)(p_TBA[pcibus][j]));
  //    }
  //  printf("--------------------------------------------------------------\n");
    
  //Step through windows
  for(j=0;j<4;j++)
    {
      if (      (p_WSBA[pcibus][j] & 1)									// window enabled...
		&& ! ((address ^ p_WSBA[pcibus][j]) & 0xfff00000 & ~p_WSM[pcibus][j]))	// address in range...
	{
	  a = (address & ((p_WSM[pcibus][j] & X64(fff00000)) | 0xfffff)) + (p_TBA[pcibus][j] & X64(3fffc0000));
	  //		  printf("PCI memory address %08x translated to %08x%08x\n",address, (u32)(a>>32),(u32)a);
	  WriteMem(a, dsize, data);
          return;
	}
    }

  WriteMem(X64(80000000000) | (pcibus * X64(200000000)) | (u64)address, dsize, data);
    

}

u64 CSystem::PCI_ReadMem(int pcibus, u32 address, int dsize)
{
  u64 a;
  int j;

  //    printf("-------------- PCI MEMORY ACCESS FOR PCI HOSE %d --------------\n", pcibus);
  //Step through windows
  //	for(j=0;j<4;j++)
  //	{
  //      printf("WSBA%d: %08x%08x WSM%d: %08x%08x TBA%d: %08x%08x\n",
  //        j,(u32)(p_WSBA[pcibus][j]>>32),(u32)(p_WSBA[pcibus][j]),
  //      j,(u32)(p_WSM[pcibus][j]>>32),(u32)(p_WSM[pcibus][j]),
  //    j,(u32)(p_TBA[pcibus][j]>>32),(u32)(p_TBA[pcibus][j]));
  //    }
  //  printf("--------------------------------------------------------------\n");

  //Step through windows
  for(j=0;j<4;j++)
    {
      
      if (      (p_WSBA[pcibus][j] & 1)									// window enabled...
		&& ! ((address ^ p_WSBA[pcibus][j]) & 0xfff00000 & ~p_WSM[pcibus][j]))	// address in range...
	{
	  a = (address & ((p_WSM[pcibus][j] & X64(fff00000)) | 0xfffff)) + (p_TBA[pcibus][j] & X64(3fffc0000));
	  //		  printf("PCI memory address %08x translated to %08x%08x\n",address, (u32)(a>>32),(u32)a);
	  return ReadMem(a, dsize);
	}
    }

  return ReadMem(X64(80000000000) | (pcibus * X64(200000000)) | (u64)address, dsize);

}

void CSystem::SaveState(char *fn)
{
  FILE * f;
  int i;
  unsigned int m;
  unsigned int j;
  int * mem = (int*) memory;
  int int0 = 0;
  unsigned int memints = (1<<iNumMemoryBits)/sizeof(int);

  f = fopen(fn,"wb");
  if (f)
    {
      // memory
      for (m=0 ; m<memints ;m++)
	{
	  if (mem[m])
	    {
	      fwrite(&(mem[m]),1,sizeof(int),f);
	    }
	  else
	    {
	      j = 0;
	      m++;
	      while (!mem[m] && (m<memints))
		{
		  m++;
		  j++;
		  if ((int)j == -1)
		    break;
		}
	      if (mem[m]) 
		m--;
	      fwrite(&int0,1,sizeof(int),f);
	      fwrite(&j,1,sizeof(int),f);
	    }
	}

      // TIG registers
      fwrite(&tig_FwWrite, 1, 1, f);
      fwrite(&tig_HaltA, 1, 1, f);
      fwrite(&tig_HaltB, 1, 1, f);

      // C_ registers
      fwrite(&c_CSC, 1, 8, f);
      fwrite(&c_MISC, 1, 8, f);
      fwrite(&c_TRR, 1, 8, f);
      fwrite(&c_TDR, 1, 8, f);
      fwrite(&c_DRIR, 1, 8, f);
      fwrite(c_DIM, 1, 4*8, f);
	
      // D_ registers
      fwrite(&d_STR, 1, 8, f);

      // P_ registers
      fwrite(p_PLAT, 1, 2*8, f);
      fwrite(p_PERRMASK, 1, 2*8, f);
      fwrite(p_PCTL, 1, 2*8, f);
      fwrite(p_WSBA, 1, 2*4*8, f);
      fwrite(p_WSM, 1, 2*4*8, f);
      fwrite(p_TBA, 1, 2*4*8, f);

      // components
      //
      //  Components should also save any non-initial memory-registrations and re-register upon restore!
      //

      for (i=0;i<iNumComponents;i++)
	acComponents[i]->SaveState(f);
      fclose(f);
    }
}

void CSystem::RestoreState(char *fn)
{
  FILE * f;
  int i;
  unsigned int m;
  unsigned int j;
  int * mem = (int*) memory;
  unsigned int memints = (1<<iNumMemoryBits)/sizeof(int);

  f = fopen(fn,"rb");
  if (!f)
    {
      printf("Can't open restore file %s\n",fn);
      return;
    }

  // memory
  for (m=0;m<memints;m++)
    {
      fread(&(mem[m]),1,sizeof(int),f);
      if (!mem[m])
	{
	  fread(&j,1,sizeof(int),f);
	  while (j--)
	    {
	      mem[++m] = 0;
	    }
	}
    }

  // TIG registers
  fread(&tig_FwWrite, 1, 1, f);
  fread(&tig_HaltA, 1, 1, f);
  fread(&tig_HaltB, 1, 1, f);

  // C_ registers
  fread(&c_CSC, 1, 8, f);
  fread(&c_MISC, 1, 8, f);
  fread(&c_TRR, 1, 8, f);
  fread(&c_TDR, 1, 8, f);
  fread(&c_DRIR, 1, 8, f);
  fread(c_DIM, 1, 4*8, f);

  // D_ registers

  fread(&d_STR, 1, 8, f);

  // P_ registers

  fread(p_PLAT, 1, 2*8, f);
  fread(p_PERRMASK, 1, 2*8, f);
  fread(p_PCTL, 1, 2*8, f);
  fread(p_WSBA, 1, 2*4*8, f);
  fread(p_WSM, 1, 2*4*8, f);
  fread(p_TBA, 1, 2*4*8, f);


  // components
  //
  //  Components should also save any non-initial memory-registrations and re-register upon restore!
  //

  for (i=0;i<iNumComponents;i++)
    acComponents[i]->RestoreState(f);
  fclose(f);
}

void CSystem::DumpMemory(unsigned int filenum)
{

  char file[100];
  int x;
  int * mem = (int*) memory;
  FILE * f;

  sprintf(file,"memory_%012d.dmp",filenum);
  f = fopen(file,"wb");

  x = (1<<iNumMemoryBits)/sizeof(int)/2;

  while (!mem[x-1])
    x--;

  fwrite(mem,1,x*sizeof(int),f);
  fclose(f);
}



void CSystem::LoadConfig(const char *filename) {
  char linebuf[121];
  char *p, *keyp, *valp, *key, *val;
  struct SConfig *conf;

  printf("%%SYS-I-READCFG: Reading configuration file '%s'\n",filename);
  iNumConfig=0;
  FILE *f = fopen(filename,"r");
  if(f==NULL) {
    printf("%%SYS-E-READCFG: Configuration file cannot be read.\n");
    return;
  }
  while(!feof(f)) {
    fgets(linebuf,120,f);
    // terminate the line at the comment char, if any.
    if((p=strchr(linebuf,'#'))) *p=0;
    
    // if the line has an =, it is a config line.
    if((p=strchr(linebuf,'='))) {
      *p=0;
      keyp = linebuf;
      valp = p+1;

      // find start of key
      while(isblank(*keyp) && *keyp!=0) 
        keyp++;
      p=keyp;
      // find end of variable
      while(!isblank(*p) && *p !=0) 
        p++;
      *p=0;

      // find start of value
      while(isblank(*valp) && *valp!=0) 
        valp++;
      p=valp;
      // find end of value
      while(!isblank(*p) && *p != '\n' && *p != '\r' && *p !=0) 
        p++;
      *p=0;

      // keyp and valp now point to valid strings for the variable
      // name and value name, respectively
      conf = (struct SConfig*)malloc(sizeof(struct SConfig));
      
      val = (char *)malloc(strlen(valp)+1);
      key = (char *)malloc(strlen(keyp)+1);
      strcpy(val,valp);
      strcpy(key,keyp);

      conf->value=val;
      conf->key=key;
      
      asConfig[iNumConfig++]=conf;
    } else {
      //printf("Ignored Config Line:  %s\n",linebuf);
    }
  }
  fclose(f);
  printf("%%SYS-I-READCFG: Successful.  %d configuration variables read.\n",iNumConfig);

  //  for(int x = 0; x<iNumConfig; x++) {
  //   printf("'%s'='%s'\n",asConfig[x]->key,asConfig[x]->value);
  // }

}

char *CSystem::GetConfig(const char *key) {
  return GetConfig(key,NULL);
}

char *CSystem::GetConfig(const char *key, char *defval) {
  for(int i=0;i<iNumConfig;i++) {
    if(strcmp(asConfig[i]->key,key)==0) {
      return asConfig[i]->value;
    }
  }
  return defval;
}
