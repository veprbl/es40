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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
 * USA.
 * 
 * Although this is not required, the author would appreciate being notified 
 * of, and receiving any modifications you may make to the source code that 
 * might serve the general public.
 */
 
/**
 * \file AlphaCPU.H 
 * \brief AlphaCPU.H contains the code for the emulated DecChip 21264CB EV68 Alpha processor.
 * \author Camiel Vanderhoeven
 **/

#if !defined(__ALPHACPU_H__)
#define __ALPHACPU_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SystemComponent.h"
#include "System.h"
#include "TranslationBuffer.h"

/**
 * Instruction cache entry.
 * An instruction cache entry contains the address and address space number
 * (ASN) + 16 32-bit instructions.
 **/
 
struct SICache {
  int asn;		/**< Address Space Number*/
  u32 data[16];		/**< Actual cached instructions*/
  u64 address;		/**< Address of first instruction*/
  bool valid;		/**< Valid cache entry*/
};

/**
 * Emulated CPU.
 * The CPU emulated is the DECchip 21264CB Alpha Processor (EV68).
 **/

class CAlphaCPU : public CSystemComponent  
{
 public:
  virtual void SaveState(FILE * f);
  virtual void RestoreState(FILE * f);
  void irq_h(int number, bool assert);
  int get_cpuid();
  u64 pal_base;
  int get_i_spe();
  int get_d_spe();
  void flush_icache();

  void set_PAL_BASE(u64 pb);
  int get_altcm();
  int get_cm();
  int get_asn();
  virtual void DoClock();
  CAlphaCPU(CSystem * system);
  virtual ~CAlphaCPU();
  u64 pc;			/**< Program counter */
  u64 get_r(int i, bool translate);
  u64 get_prbr(void);

  /*	int trclvl;
    u64 trcadd[700];
    int trcfncs;
    int trchide;
    u64 trc_waitfor;
    struct STraceFunction trcfnc[5000]; 
  */	u32 cc;
  u64 r[64];			/**< Integer registers (0-31 normal, 32-63 shadow) */

 private:
  u64 dc_stat;
  bool ppcen;
  u64 i_stat;
  u64 pctr_ctl;
  bool cc_ena;
  u32 cc_offset;
  u64 dc_ctl;
  int alt_cm;
  int smc;
  bool fpen;
  bool sde;
  u64 fault_va;
  u64 exc_sum;
  int ier;
  int  i_ctl_va_mode;
  int  va_ctl_va_mode;
  u64  i_ctl_vptb;
  u64  va_ctl_vptb;
  //    bool palmode;
  int  cm;
  int asn;
  int asn0;
  int asn1;
  int eien;
  int slen;
  int cren;
  int pcen;
  int sien;
  int asten;
  int sir;
  int eir;
  int slr;
  int crr;
  int pcr;
  int astrr;
  int aster;
  u64 i_ctl_other;
  u64 mm_stat;
  bool hwe;
  int m_ctl_spe;
  int i_ctl_spe;

  u64 va_form(u64 address, int va_ctl, u64 ptbr);
  u64 exc_addr;
  u64 pmpc;
  u64 fpcr;
  bool bIntrFlag;
  u64 current_pc;
  struct SICache icache[1024];		/**< Instruction cache entries*/
  int next_icache;			/**< Number of next cache entry to use*/
  int get_icache(u64 address, u32 * data);
  void go_pal(u32 pal_offset);
	
  bool lock_flag;

  u64 f[64];
  int iProcNum;

  CTranslationBuffer * itb;
  CTranslationBuffer * dtb;
};


inline void CAlphaCPU::flush_icache()
{
  int i;
  for(i=0;i<1024;i++) 
    icache[i].valid = false;
  next_icache = 0;
}

inline int CAlphaCPU::get_d_spe()
{
  return (int) m_ctl_spe;
}

inline int CAlphaCPU::get_i_spe()
{
  return (int) i_ctl_spe;
}

inline int CAlphaCPU::get_asn()
{
  return asn;
}

inline int CAlphaCPU::get_cm()
{
  return cm;
}

inline int CAlphaCPU::get_altcm()
{
  return alt_cm;
}

inline void CAlphaCPU::set_PAL_BASE(u64 pb)
{
  pal_base = pb;
}

inline int CAlphaCPU::get_icache(u64 address, u32 * data)
{
  int i;
  u64 v_a;
  u64 p_a;
  int result;

  for (i=0;i<1024;i++)
    {
      if (	icache[i].valid
		&& icache[i].asn == asn
		&& icache[i].address == (address & X64(ffffffffffffffc1)))
	{
	  *data = icache[i].data[(address>>2)&X64(0f)];
	  return 0;
	}
    }

  for(i=0;i<16;i++)
    {
      v_a = (address & X64(ffffffffffffffc0)) | (u64)i << 2;
      if (address & 1)
	p_a = v_a & X64(00000fffffffffff);
      else
	{                                       //              vv== SHOULD BE TRUE
	  result = itb->convert_address(v_a,&p_a,ACCESS_READ,false,get_cm(),get_asn(),get_i_spe());
	  if (result)
	    return result;
	}
      icache[next_icache].data[i] = (u32)(cSystem->ReadMem(p_a,32));
    }

  icache[next_icache].valid = true;
  icache[next_icache].asn = asn;
  icache[next_icache].address = address & X64(ffffffffffffffc1);
	
  *data = icache[next_icache].data[(address>>2)&X64(0f)];

  next_icache++;
  if (next_icache==1024)
    next_icache = 0;
  return 0;
}

inline u64 CAlphaCPU::va_form(u64 address, int va_ctl, u64 ptbr)
{
  switch( va_ctl)
    {
    case 0:
      return  (ptbr            & X64(fffffffe00000000))
	| ((address>>10)   & X64(00000001fffffff8));
    case 1:
      return   (ptbr	         & X64(fffff80000000000))
	| ((address>>10)  & X64(0000003ffffffff8))
	| (((address>>10) & X64(0000002000000000)) * X64(3e));
    case 2:
      return  (ptbr			 & X64(ffffffffc0000000))
	|((address>>10)   & X64(00000000003ffff8));
    }
  return 0;
}

inline int CAlphaCPU::get_cpuid()
{
  return iProcNum;
}

inline void CAlphaCPU::irq_h(int number, bool assert)
{
  if (assert)
    {
      //        if (number<2 && !(eir & (X64(1)<<number)))
      //            printf("Interrupt %d asserted on CPU %d\n",number,iProcNum);
      eir |= (X64(1)<<number);
    }
  else
    {
      //        if (number<2 && (eir & (X64(1)<<number)))
      //            printf("Interrupt %d de-asserted on CPU %d\n",number,iProcNum);
      eir &= ~(X64(1)<<number);
    }
}



#endif // !defined(__ALPHACPU_H__)
