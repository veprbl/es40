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
 * 
 * TRANSLATIONBUFFER.H contains the definitions for the emulated on-cpu instruction and
 * data translation buffers.
 */

#if !defined(__TRANSLATIONBUFFER_H__)
#define __TRANSLATIONBUFFER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "datatypes.h"
#include "System.h"

extern CSystem * systm;

#define TB_ENTRIES 128

#define CM_KERNEL		0
#define CM_EXECUTIVE	1
#define CM_SUPERVISOR	2
#define CM_USER			3

#define ACCESS_READ		0
#define ACCESS_WRITE	1

#define E_ACCESS		1
#define E_NOT_FOUND		2

#define MBOX_PHYS_S 19
#define ASM_BIT     4
#define GH_BIT      5
#define ACCESS_BIT	8
#define FAULT_BIT   1

/**
 * Translation Buffer Entry.
 * A translation buffer entry provides the mapping from a page of virtual memory to a page of physical memory.
 **/

struct STBEntry {
  u64 virt;		/**< Virtual address of page*/
  u64 phys;		/**< Physical address of page*/
  int asn;		/**< Address Space Number*/
  u8 gh;		/**< Granularity Hint*/
  bool asm_bit;		/**< Address Space Match bit*/
  bool access[2][4];	/**< Access permitted [read/write][current mode]*/
  bool fault[2];        /**< Fault on access [read/write]*/
  bool valid;		/**< Valid entry*/
};

/**
 * Translation Buffer.
 * The translation buffers are two on-chip translation units for translating virtual addresses to physical
 * memory. Each Alpha CPU has one translation buffers for instructions, and one for data.
 **/

class CTranslationBuffer  
{
 public:
  virtual void RestoreState(FILE * f);
  void SaveState(FILE * f);
  u64 v_mask;
  u64 p_mask;
  int FindEntry(u64 virt, int asn);
  void InvalidateAll();
  void InvalidateAllProcess();
  void InvalidateSingle(u64 address, int asn);
  int convert_address(u64 virt, u64 * phys, u8 access, bool check, int cm, int asn, int spe);
  void write_pte(int number, u64 value, int asn);
  void write_tag(int number, u64 value);
  CTranslationBuffer(class CAlphaCPU * c, bool ibox);
  virtual ~CTranslationBuffer();

 private:
  bool bIBOX;
  class CAlphaCPU * cCpu;
  struct STBEntry entry[TB_ENTRIES];
  int    next_entry;
  u64	   temp_tag[2];
};


inline void CTranslationBuffer::write_tag(int number, u64 value)
{
  temp_tag[number] = value;
}

inline void CTranslationBuffer::write_pte(int number, u64 value, int asn)
{
  int i,j;

  if (FindEntry(temp_tag[number],asn) == -1)
    {
      entry[next_entry].gh      = (u8)((value>>GH_BIT)  & 3);
      switch (entry[next_entry].gh)
	{
	case 0:
	  v_mask = X64(0000ffffffffe000);  
	  break;
	case 1:
	  v_mask = X64(0000ffffffff0000);
	  break;
	case 2:
	  v_mask = X64(0000fffffff80000);
	  break;
	case 3:
	  v_mask = X64(0000ffffffc00000);
	  break;
	}
      entry[next_entry].virt = (temp_tag[number] & v_mask);
	
      if (bIBOX) 
	{
	  entry[next_entry].phys = value & v_mask;
	  entry[next_entry].fault[0] = false;
	  entry[next_entry].fault[1] = false;
	  for (j=0;j<4;j++) {
	    entry[next_entry].access[ACCESS_READ][j] = (value>>(ACCESS_BIT+j)) & 1;
	    entry[next_entry].access[ACCESS_WRITE][j] = false;
	  }

	} 
      else 
	{
	  entry[next_entry].phys = (value >> MBOX_PHYS_S) & v_mask;
	  for (i=0;i<2;i++) 
	    {
	      entry[next_entry].fault[i] = (value>>(FAULT_BIT+i)) & 1;
	      for (j=0;j<4;j++)
		entry[next_entry].access[i][j] = (value>>(ACCESS_BIT+j+(i*4))) & 1;
	    }
	}

      entry[next_entry].asm_bit = (value>>ASM_BIT) & 1;
      if (!entry[next_entry].asm_bit)
	entry[next_entry].asn = asn;
      entry[next_entry].valid = true;

      //	printf("\n================================\n");
      //	printf("ADD PTE: %d\n",next_entry);
      //	printf("    virt: %08x%08x\n",(u32)(entry[next_entry].virt>>32),(u32)(entry[next_entry].virt));
      //	printf("    phys: %08x%08x\n",(u32)(entry[next_entry].phys>>32),(u32)(entry[next_entry].phys));
      //	printf("    asm : %s\n",entry[next_entry].asm_bit?"true":"false");
      //	printf("    asn : %02x\n",entry[next_entry].asn);
      //	printf("================================\n");
	
      next_entry++;
      if (next_entry==TB_ENTRIES)
	next_entry = 0;
    }
}

inline int CTranslationBuffer::FindEntry(u64 virt, int asn)
{
  int i;

  for (i=0;i<TB_ENTRIES;i++)
    {
      switch (entry[i].gh)
	{
	case 0:
	  v_mask = X64(0000ffffffffe000);
	  p_mask = X64(0000000000001fff);
	  break;
	case 1:
	  v_mask = X64(0000ffffffff0000);
	  p_mask = X64(000000000000ffff);
	  break;
	case 2:
	  v_mask = X64(0000fffffff80000);
	  p_mask = X64(000000000007ffff);
	  break;
	case 3:
	  v_mask = X64(0000ffffffc00000);
	  p_mask = X64(00000000003fffff);
	  break;
	}

      if (     entry[i].valid
	       && !((entry[i].virt ^ virt) & v_mask)
	       &&  (     entry[i].asm_bit
			 || (entry[i].asn == asn)
			 )
	       )
	return i;
    }
  return -1;
}

inline int CTranslationBuffer::convert_address(u64 virt, u64 *phys, u8 access, bool check, int cm, int asn, int spe)
{
  int i;

  //	int spe;

  //	spe = bIBOX?cCpu->get_i_spe():cCpu->get_d_spe();
  if (spe && !cm)
    {
      if (   (((virt>>46)&3) == 2)
	     && (spe&4))
	{
	  *phys = virt & X64(00000fffffffffff);
	  return 0;
	}
      else if (   (((virt>>41)&0x7f) == 0x7e)
		  && (spe & 2))
	{
	  *phys =   (virt & X64(000001ffffffffff)) 
	    | ((virt & X64(0000010000000000)) * 6);
	  return 0;
	}
      else if (   (((virt>>30)&0x3ffff) == 0x3fffe)
		  && (spe & 1))
	{
	  *phys = virt & X64(000000003fffffff);
	  return 0;
	}
    }

  i = FindEntry(virt, asn);
  if (i<0)
    return E_NOT_FOUND;

  // check access...
  if (   check && !entry[i].access[access][cm])
    return E_ACCESS;
  // all is ok...

  *phys = (entry[i].phys & v_mask) | (virt & p_mask);

  //	if (((*phys) & X64(00000fffffffffff)) != (virt & X64(00000fffffffffff)))
  //	{
#ifdef _WIN32
  //		printf("*** %s VIRT: %016I64x translates to PHYS %016I64x\n",bIBOX?"IBOX":"MBOX",virt,*phys);
#else
  //		printf("*** %s VIRT: %016llx translates to PHYS %016llx\n",bIBOX?"IBOX":"MBOX",virt,*phys);
#endif
#ifdef _WIN32
  //		fprintf(systm->trace->trace_file(),"*** %s VIRT: %016I64x translates to PHYS %016I64x\n",bIBOX?"IBOX":"MBOX",virt,*phys);
#else
  //		fprintf(systm->trace->trace_file(),"*** %s VIRT: %016llx translates to PHYS %016llx\n",bIBOX?"IBOX":"MBOX",virt,*phys);
#endif
  //	}

  return 0;
}

inline void CTranslationBuffer::InvalidateAll()
{
  int i;
  for (i=0;i<TB_ENTRIES;i++)
    entry[i].valid = false;

  next_entry = 0;
}

inline void CTranslationBuffer::InvalidateAllProcess()
{
  int i;
  for (i=0;i<TB_ENTRIES;i++)
    {
      if (!entry[i].asm_bit)
	entry[i].valid = false;
    }

  next_entry = 0;
}

inline void CTranslationBuffer::InvalidateSingle(u64 address, int asn)
{
  int i;
  i = FindEntry(address, asn);
  if (i<0)
    return;
  entry[i].valid = false;
}

#endif // !defined(__TRANSLATIONBUFFER_H__)
