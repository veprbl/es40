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
 * X-1.18       Eduardo Marcelo Serrat					        31-OCT-2007
 *      Fixed address translation sign extension.  Disable access checks
 *      for now.
 *
 * X-1.17       Camiel Vanderhoeven                             16-APR-2007
 *      Initialize state.next_entry to 0.
 *
 * X-1.16       Camiel Vanderhoeven                             11-APR-2007
 *      Moved all data that should be saved to a state file to a structure
 *      "state".
 *
 * X-1.15	Camiel Vanderhoeven				3-APR-2007
 *	Backed out X-1.13, because this did not solve the problem, and the
 *	real problem was found and fixed (AliM1543C.cpp/X-1.23).
 *
 * X-1.14	Camiel Vanderhoeven				1-APR-2007
 *	InvalidateSingle invalidates all matching entries, if more than
 *	one match.
 *
 * X-1.13       Camiel Vanderhoeven                             31-MAR-2007
 *      Define NO_INTELLIGENT_TB by default, because some bug causes the 
 *	translation buffer to return bogus translations for IDE addresses
 *	to the DQDRIVER.
 *
 * X-1.12	Camiel Vanderhoeven				27-MAR-2007
 *	Add possibility to disable "smart" Translation Buffers by defining
 *	NO_INTELLIGENT_TB
 *
 * X-1.11	Camiel Vanderhoeven				14-MAR-2007
 *	Do not perform translations when we are listing.
 *
 * X-1.10       Camiel Vanderhoeven                             12-MAR-2007
 *   a) Added support for "smart" Translation Buffers.
 *   b)	Added support for Translation Buffer debugging.
 *
 * X-1.9	Camiel Vanderhoeven				9-MAR-2007
 *	Formatting.
 *
 * X-1.8	Camiel Vanderhoeven				8-MAR-2007
 *	When doing a double-lookup, do it manually.
 *
 * X-1.7	Camiel Vanderhoeven				8-MAR-2007
 *	When no entry is found, try to look it up in the page table ourselves.
 *
 * X-1.6       Camiel Vanderhoeven                             8-MAR-2007
 *	Inline functions from TranslationBuffer.h turned into normal 
 *	functions, and moved here.
 *
 * X-1.5        Camiel Vanderhoeven                             12-FEB-2007
 *      Added comments.
 *
 * X-1.4        Camiel Vanderhoeven                             7-FEB-2007
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

#include "StdAfx.h"
#include "TranslationBuffer.h"
#include "AlphaCPU.h"
#include "TraceEngine.h"

extern CSystem * systm;
/**
 * Constructor.
 * Creates a translation buffer and invalidates all entries.
 **/

CTranslationBuffer::CTranslationBuffer(class CAlphaCPU * c, bool ibox)
{
  cCPU = c;
  state.bIBOX = ibox;
  state.p_mask = 0;
  state.v_mask = 0;
  state.temp_tag[0] = 0;
  state.temp_tag[1] = 0;
  state.next_entry = 0;

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
  fwrite(&state,sizeof(state),1,f);
}

/**
 * Restore state from a Virtual Machine State file.
 **/

void CTranslationBuffer::RestoreState(FILE *f)
{
  fread(&state,sizeof(state),1,f);
}


void CTranslationBuffer::write_tag(int number, u64 value)
{
  state.temp_tag[number] = value;
}

void CTranslationBuffer::write_pte(int number, u64 value)
{
  int i,j;

  if (FindEntry(state.temp_tag[number]) == -1)
    {
      state.entry[state.next_entry].gh      = (u8)((value>>GH_BIT)  & 3);
      switch (state.entry[state.next_entry].gh)
	{
	case 0:
	  state.v_mask = X64(0000ffffffffe000);  
	  break;
	case 1:
	  state.v_mask = X64(0000ffffffff0000);
	  break;
	case 2:
	  state.v_mask = X64(0000fffffff80000);
	  break;
	case 3:
	  state.v_mask = X64(0000ffffffc00000);
	  break;
	}
      state.entry[state.next_entry].virt = (state.temp_tag[number] & state.v_mask);
	
      if (state.bIBOX) 
	{
	  state.entry[state.next_entry].phys = value & state.v_mask;
	  state.entry[state.next_entry].fault[0] = false;
	  state.entry[state.next_entry].fault[1] = false;
	  for (j=0;j<4;j++) {
	    state.entry[state.next_entry].access[ACCESS_READ][j] = (value>>(ACCESS_BIT+j)) & 1;
	    state.entry[state.next_entry].access[ACCESS_WRITE][j] = false;
	  }

	} 
      else 
	{
	  state.entry[state.next_entry].phys = (value >> MBOX_PHYS_S) & state.v_mask;
	  for (i=0;i<2;i++) 
	    {
	      state.entry[state.next_entry].fault[i] = (value>>(FAULT_BIT+i)) & 1;
	      for (j=0;j<4;j++)
		state.entry[state.next_entry].access[i][j] = (value>>(ACCESS_BIT+j+(i*4))) & 1;
	    }
	}

      state.entry[state.next_entry].asm_bit = (value>>ASM_BIT) & 1;
      if (!state.entry[state.next_entry].asm_bit)
	state.entry[state.next_entry].asn = cCPU->get_asn(state.bIBOX);
      state.entry[state.next_entry].valid = true;
	
      state.next_entry++;
      if (state.next_entry==TB_ENTRIES)
	state.next_entry = 0;
    }
}

int CTranslationBuffer::FindEntry(u64 virt)
{
  int i;

  for (i=0;i<TB_ENTRIES;i++)
    {
      switch (state.entry[i].gh)
	{
	case 0:
	  state.v_mask = X64(0000ffffffffe000);
	  state.p_mask = X64(0000000000001fff);
	  break;
	case 1:
	  state.v_mask = X64(0000ffffffff0000);
	  state.p_mask = X64(000000000000ffff);
	  break;
	case 2:
	  state.v_mask = X64(0000fffffff80000);
	  state.p_mask = X64(000000000007ffff);
	  break;
	case 3:
	  state.v_mask = X64(0000ffffffc00000);
	  state.p_mask = X64(00000000003fffff);
	  break;
	}

      if (     state.entry[i].valid
	       && !((state.entry[i].virt ^ virt) & state.v_mask)
	       &&  (     state.entry[i].asm_bit
			 || (state.entry[i].asn == cCPU->get_asn(state.bIBOX))
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

#if defined IDB
  if (bListing)
  {
    *phys = virt;
    return 0;
  }
#endif

#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
  if (bTB_Debug)
#endif
    printf("%s %" LL "x: ",state.bIBOX?"I":"D", virt);
#endif

  if (cCPU->get_spe(state.bIBOX) && !cm)
    {
      if (   (((virt>>46)&3) == 2)
	     && (cCPU->get_spe(state.bIBOX)&4))
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
		  && (cCPU->get_spe(state.bIBOX) & 2))
	{
	  *phys =   (virt & X64(000001ffffffffff)) 
	    | ((virt & X64(0000010000000000)) * 14);
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
		  && (cCPU->get_spe(state.bIBOX) & 1))
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
#if defined(NO_INTELLIGENT_TB)
  {
#if defined(DEBUG_TB)
    if (forreal)
#if defined(IDB)
      if (bTB_Debug)
#endif
        printf("not found\n");
#endif
      return E_NOT_FOUND;
  }
#else
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
    va_form = cCPU->va_form(virt,state.bIBOX);
    if (state.bIBOX) {
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
    } else {	// state.bIBOX
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
#endif // NO_INTELLIGENT_TB

  // check access...
  //if (check)
  //{
  //  if (!state.entry[i].access[access][cm])
  //  {
#if defined(DEBUG_TB)
  //if (forreal)
#if defined(IDB)
  //    if (bTB_Debug)
#endif
  //      printf("acv\n");
#endif
  //    return E_ACCESS;
  //  }
  //  if (state.entry[i].fault[access])
  //  {
#if defined(DEBUG_TB)
  //if (forreal)
#if defined(IDB)
  //    if (bTB_Debug)
#endif
  //      printf("fault\n");
#endif
  //      return E_FAULT;
  //  }
  //}
  // all is ok...

  *phys = (state.entry[i].phys & state.v_mask) | (virt & state.p_mask);
  *asm_bit = state.entry[i].asm_bit;

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
    state.entry[i].valid = false;
}

void CTranslationBuffer::InvalidateAllProcess()
{
  int i;
  for (i=0;i<TB_ENTRIES;i++)
    {
      if (!state.entry[i].asm_bit)
	state.entry[i].valid = false;
    }
}

void CTranslationBuffer::InvalidateSingle(u64 address)
{
  int i;
  while ((i = FindEntry(address)) >= 0)
    state.entry[i].valid = false;
}
