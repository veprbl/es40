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
 * Contains the code for the emulated on-cpu translation buffers.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
 **/

#include "StdAfx.h"
#include "TranslationBuffer.h"
#include "AlphaCPU.h"

extern CSystem * systm;

/**
 * Constructor.
 * Creates a translation buffer and invalidates all entries.
 **/

CTranslationBuffer::CTranslationBuffer(class CAlphaCPU * c, bool ibox)
{
  cCPU = c;
  bIBOX = ibox;
  p_mask = 0;
  v_mask = 0;
  temp_tag[0] = 0;
  temp_tag[1] = 0;

  InvalidateAll();
}

/**
 * Destructor.
 **/

CTranslationBuffer::~CTranslationBuffer()
{

}

/**
 * Save state to a Virtual Machine State file.
 **/

void CTranslationBuffer::SaveState(FILE *f)
{
  fwrite(&bIBOX,1,sizeof(bool),f);
  fwrite(entry,1,TB_ENTRIES * sizeof(struct STBEntry),f);
  fwrite(&next_entry,1,sizeof(int),f);
  fwrite(temp_tag,1,2*8,f);
}

/**
 * Restore state from a Virtual Machine State file.
 **/

void CTranslationBuffer::RestoreState(FILE *f)
{
  fread(&bIBOX,1,sizeof(bool),f);
  fread(entry,1,TB_ENTRIES * sizeof(struct STBEntry),f);
  fread(&next_entry,1,sizeof(int),f);
  fread(temp_tag,1,2*8,f);
}


void CTranslationBuffer::write_tag(int number, u64 value)
{
  temp_tag[number] = value;
}

void CTranslationBuffer::write_pte(int number, u64 value)
{
  int i,j;

  if (FindEntry(temp_tag[number]) == -1)
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
	entry[next_entry].asn = cCPU->get_asn(bIBOX);
      entry[next_entry].valid = true;
	
      next_entry++;
      if (next_entry==TB_ENTRIES)
	next_entry = 0;
    }
}

int CTranslationBuffer::FindEntry(u64 virt)
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
			 || (entry[i].asn == cCPU->get_asn(bIBOX))
			 )
	       )
	return i;
    }
  return -1;
}

int CTranslationBuffer::convert_address(u64 virt, u64 *phys, u8 access, bool check, int cm, bool * asm_bit, bool cascade, bool forreal)
{
  int i;
  u64 phys_pte;
  u64 pte;
  u64 va_form;
  bool b;

#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
  if (bTB_Debug)
#endif
    printf("%s %" LL "x: ",bIBOX?"I":"D", virt);
#endif

  if (cCPU->get_spe(bIBOX) && !cm)
    {
      if (   (((virt>>46)&3) == 2)
	     && (cCPU->get_spe(bIBOX)&4))
	{
	  *phys = virt & X64(00000fffffffffff);
	  *asm_bit = false;
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
          if (bTB_Debug)
#endif
	    printf("SPE\n");
#endif
	  return 0;
	}
      else if (   (((virt>>41)&0x7f) == 0x7e)
		  && (cCPU->get_spe(bIBOX) & 2))
	{
	  *phys =   (virt & X64(000001ffffffffff)) 
	    | ((virt & X64(0000010000000000)) * 6);
	  *asm_bit = false;
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
          if (bTB_Debug)
#endif
	    printf("SPE\n");
#endif
	  return 0;
	}
      else if (   (((virt>>30)&0x3ffff) == 0x3fffe)
		  && (cCPU->get_spe(bIBOX) & 1))
	{
	  *phys = virt & X64(000000003fffffff);
	  *asm_bit = false;
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
          if (bTB_Debug)
#endif
	    printf("SPE\n");
#endif
	  return 0;
	}
    }

  i = FindEntry(virt);
  if (i<0)
  {
    if (cascade)
    {
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
      if (bTB_Debug)
#endif
        printf("cascade not found\n");
#endif
      return E_NOT_FOUND; 
    }
    if ((cCPU->get_pal_base()!= 0x8000))
    {
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
      if (bTB_Debug)
#endif
        printf("other PAL base\n");
#endif
      return E_NOT_FOUND;
    }
    va_form = cCPU->va_form(virt,bIBOX);
    if (bIBOX) {
      if (cCPU->get_r(22+32,false) & X64(8000000000000000)) {
        pte = (cCPU->get_pal_base() & X64(ffffffffffe00000)) | (virt & X64(ffffffffffffe000)) | 0xf01;
      } else {
	if (cCPU->get_tb(false)->convert_address(va_form, &phys_pte, 0, false, 0, &b, true, forreal))
	{
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
          if (bTB_Debug)
#endif
	    printf("can't convert %" LL "x\n",va_form);
#endif
	  return E_NOT_FOUND;
        }
	pte = systm->ReadMem(phys_pte,64);
	if (pte&8)
	{
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
          if (bTB_Debug)
#endif
	    printf("PTE & 8\n");
#endif
	  return E_NOT_FOUND;
	}
	pte = ((pte >> 0x20) << 0x0d) | (pte & 0xfff);
      }
    } else {	// bIBOX
      if (cCPU->get_r(22+32,false)&X64(8000000000000000)) {
	if (virt&X64(80000000000)) {
	  u64 t1;
	  u64 t2 = X64(10f44);

	  t1 = (virt >> 0xd) & ~X64(ffffffffbfc00000);
	  for(;;)
	  {
	    if (t1 < systm->ReadMem(t2,32)) {
	      pte = (t1 << 0x20) | 0xff01;
	      break;
	    }
	    if (t1 <= systm->ReadMem(t2+4,32))
	    {
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
              if (bTB_Debug)
#endif
	        printf("complex-2\n");
#endif
	      return E_NOT_FOUND;
	    }
	    t2 += 8;
	  }
	} else {
          pte = (((cCPU->get_pal_base() & X64(ffffffffffe00000)) | (virt & X64(ffffffffffffe000)))<<0x13)| 0xff01;
	}
      } else {
	if (convert_address(va_form, &phys_pte, 0, false, 0, &b, true, forreal))
	{
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
          if (bTB_Debug)
#endif
	    printf("can't convert %" LL "x\n",va_form);
#endif
	  return E_NOT_FOUND;
	}
        pte = systm->ReadMem(phys_pte,64);
      }
    }

#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
    if (bTB_Debug)
#endif
      printf("PTE: %" LL "x - ", pte);
#endif

    if (!(pte&1))
    {
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
      if (bTB_Debug)
#endif
        printf("PTE not valid\n");
#endif
      return E_NOT_FOUND;
    }

    write_tag(0,virt);
    write_pte(0,pte);
    i = FindEntry(virt);
    if (i<0)
    {
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
      if (bTB_Debug)
#endif
        printf("still not found\n");
#endif
      return E_NOT_FOUND;
    }
  }
#if defined(DEBUG_TB)
  else
  {
  if (forreal)
#if defined(IDB)
    if (bTB_Debug)
#endif
      printf("entry %d - ", i);
  }
#endif

  // check access...
  if (check)
  {
    if (!entry[i].access[access][cm])
    {
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
      if (bTB_Debug)
#endif
        printf("acv\n");
#endif
      return E_ACCESS;
    }
    if (entry[i].fault[access])
    {
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
      if (bTB_Debug)
#endif
        printf("fault\n");
#endif
        return E_FAULT;
    }
  }
  // all is ok...

  *phys = (entry[i].phys & v_mask) | (virt & p_mask);
  *asm_bit = entry[i].asm_bit;

#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
  if (bTB_Debug)
#endif
    printf("phys: %" LL "x - OK\n", *phys);
#endif
  
  return 0;
}

void CTranslationBuffer::InvalidateAll()
{
  int i;
  for (i=0;i<TB_ENTRIES;i++)
    entry[i].valid = false;

  next_entry = 0;
}

void CTranslationBuffer::InvalidateAllProcess()
{
  int i;
  for (i=0;i<TB_ENTRIES;i++)
    {
      if (!entry[i].asm_bit)
	entry[i].valid = false;
    }

  next_entry = 0;
}

void CTranslationBuffer::InvalidateSingle(u64 address)
{
  int i;
  i = FindEntry(address);
  if (i<0)
    return;
  entry[i].valid = false;
}
