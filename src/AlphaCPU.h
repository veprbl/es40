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
 * \file
 * Contains the definitions for the emulated DecChip 21264CB EV68 Alpha processor.
 *
 * X-1.21       Camiel Vanderhoeven                             11-APR-2007
 *      Moved all data that should be saved to a state file to a structure
 *      "state".
 *
 * X-1.20	Camiel Vanderhoeven				7-APR-2007
 *	Added get_hwpcb;
 *
 * X-1.19	Camiel Vanderhoeven				5-APR-2007
 *	Fixed X-1.14. The virtual address was returned instead of the 
 *	physical one!
 *
 * X-1.18       Camiel Vanderhoeven                             31-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.17	Camiel Vanderhoeven				18-MAR-2007
 *   	Removed pointles comparison (v_prbr > 0).
 *
 * X-1.16	Camiel Vanderhoeven				14-MAR-2007
 *	bListing removed.
 *
 * X-1.15	Camiel Vanderhoeven				12-MAR-2007
 *   a)	Added possibility to retrieve physical address of current instruction.
 *   b) Added member function get_pal_base.
 *
 * X-1.14	Camiel Vanderhoeven				9-MAR-2007
 *	Try to translate a virtual PRBR value to a physical one in get_prbr.
 *
 * X-1.13	Camiel Vanderhoeven				8-MAR-2007
 *	va_form now takes a boolean argument bIBOX to determine which ASN
 *	and VPTB to use.
 *
 * X-1.12	Camiel Vanderhoeven				7-MAR-2007				
 *	Added get_tb, get_asn and get_spe functions.
 *
 * X-1.11	Camiel Vanderhoeven				22-FEB-2007
 *	Add ASM bit to the instruction cache & corresponding functions.
 *
 * X-1.10	Camiel Vanderhoeven				18-FEB-2007
 *	Add get_f function.
 *
 * X-1.9        Camiel Vanderhoeven                             16-FEB-2007
 *   a) Added CAlphaCPU::listing.
 *   b) CAlphaCPU::DoClock now returns a value.
 *
 * X-1.8        Camiel Vanderhoeven                             12-FEB-2007
 *	Added get_r and get_prbr functions as inlines.
 *
 * X-1.7        Camiel Vanderhoeven                             12-FEB-2007
 *	Added inline functions to get and update the program counter (pc).
 *
 * X-1.6	Camiel Vanderhoeven				12-FEB-2007
 *	Added comments.
 *
 * X-1.5        Camiel Vanderhoeven                             9-FEB-2007
 *	Added comments.
 *
 * X-1.4        Camiel Vanderhoeven                             9-FEB-2007
 *      Moved debugging flags (booleans) to TraceEngine.
 *
 * X-1.3	Camiel Vanderhoeven				7-FEB-2007
 *	Added comments.
 *
 * X-1.2        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.1        Camiel Vanderhoeven                             19-JAN-2007
 *      Initial version in CVS.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#if !defined(INCLUDED_ALPHACPU_H)
#define INCLUDED_ALPHACPU_H

#include "SystemComponent.h"
#include "System.h"
#include "TranslationBuffer.h"

/**
 * Instruction cache entry.
 * An instruction cache entry contains the address and address space number
 * (ASN) + 16 32-bit instructions. [HRM 2-11]
 **/
 
struct SICache {
  int asn;		/**< Address Space Number */
  u32 data[16];		/**< Actual cached instructions  */
  u64 address;		/**< Address of first instruction */
  u64 p_address;	/**< Physical address of first instruction */
  bool asm_bit;		/**< Address Space Match bit */
  bool valid;		/**< Valid cache entry */
};

/**
 * Emulated CPU.
 * The CPU emulated is the DECchip 21264CB Alpha Processor (EV68).
 * 
 * Documents referred to:
 *	- DS-0026A-TE: Alpha 21264B Microprocessor Hardware Reference Manual [HRM].
 *	  (http://ftp.digital.com/pub/Digital/info/semiconductor/literature/21264hrm.pdf)
 *	- Alpha Architecture Reference Manual, fourth edition [ARM].
 *	.
 **/

class CAlphaCPU : public CSystemComponent  
{
 public:
	 void flush_icache_asm();
  virtual void SaveState(FILE * f);
  virtual void RestoreState(FILE * f);
  void irq_h(int number, bool assert);
  int get_cpuid();
  void flush_icache();

  void set_PAL_BASE(u64 pb);
  virtual int DoClock();
  CAlphaCPU(CSystem * system);
  virtual ~CAlphaCPU();
  u64 get_r(int i, bool translate);
  u64 get_f(int i);
  u64 get_prbr(void);
  u64 get_hwpcb(void);
  u64 get_pc();
  u64 get_pal_base();

#ifdef IDB
  u64 get_current_pc_physical();
#endif
  
  u64 get_clean_pc();
  void next_pc();
  void set_pc(u64 p_pc);

  CTranslationBuffer * get_tb(bool bIBOX);
  int get_asn(bool bIBOX);
  int get_spe(bool bIBOX);
  u64 va_form(u64 address, bool bIBOX);

#if defined(IDB)
  void listing(u64 from, u64 to);
#endif

 private:
  int get_icache(u64 address, u32 * data);
  void go_pal(u32 pal_offset);

         
  struct SAlphaCPUState {
    u64 pal_base;			/**< IPR PAL_BASE [HRM: p 5-15] */
    u64 pc;			/**< Program counter */
    u32 cc;			/**< IPR CC: Cycle counter [HRM p 5-3] */
    u64 r[64];			/**< Integer registers (0-31 normal, 32-63 shadow) */
    u64 dc_stat;			/**< IPR DC_STAT: Dcache status [HRM p 5-31..32] */
    bool ppcen;			/**< IPR PCTX: ppce (proc perf counting enable) [HRM p 5-21..23] */
    u64 i_stat;			/**< IPR I_STAT: Ibox status [HRM p 5-18..20] */
    u64 pctr_ctl;			/**< IPR PCTR_CTL [HRM p 5-23..25] */
    bool cc_ena;			/**< IPR CC_CTL: Cycle counter enabled [HRM p 5-3] */
    u32 cc_offset;		/**< IPR CC: Cycle counter offset [HRM p 5-3] */
    u64 dc_ctl;			/**< IPR DC_CTL: Dcache control [HRM p 5-30..31] */
    int alt_cm;			/**< IPR DTB_ALTMODE: alternative cm for HW_LD/HW_ST [HRM p 5-26..27] */
    int smc;			/**< IPR M_CTL: smc (speculative miss control) [HRM p 5-29..30] */
    bool fpen;			/**< IPR PCTX: fpe (floating point enable) [HRM p 5-21..23] */
    bool sde;			/**< IPR I_CTL: sde[1] (PALshadow enable) [HRM p 5-15..18] */
    u64 fault_va;			/**< IPR VA: virtual address of last Dstream miss or fault [HRM p 5-4] */
    u64 exc_sum;			/**< IPR EXC_SUM: exception summary [HRM p 5-13..15] */
    int  i_ctl_va_mode;		/**< IPR I_CTL: (va_form_32 + va_48) [HRM p 5-15..17] */
    int  va_ctl_va_mode;		/**< IPR VA_CTL: (va_form_32 + va_48) [HRM p 5-4] */
    u64  i_ctl_vptb;		/**< IPR I_CTL: vptb (virtual page table base) [HRM p 5-15..16] */
    u64  va_ctl_vptb;		/**< IPR VA_CTL: vptb (virtual page table base) [HRM p 5-4] */
    int  cm;			/**< IPR IER_CM: cm (current mode) [HRM p 5-9..10] */
    int asn;			/**< IPR PCTX: asn (address space number) [HRM p 5-21..22] */
    int asn0;			/**< IPR DTB_ASN0: asn (address space number) [HRM p 5-28] */
    int asn1;			/**< IPR DTB_ASN1: asn (address space number) [HRM p 5-28] */
    int eien;			/**< IPR IER_CM: eien (external interrupt enable) [HRM p 5-9..10] */
    int slen;			/**< IPR IER_CM: slen (serial line interrupt enable) [HRM p 5-9..10] */
    int cren;			/**< IPR IER_CM: cren (corrected read error int enable) [HRM p 5-9..10] */
    int pcen;			/**< IPR IER_CM: pcen (perf counter interrupt enable) [HRM p 5-9..10] */
    int sien;			/**< IPR IER_CM: sien (software interrupt enable) [HRM p 5-9..10] */
    int asten;			/**< IPR IER_CM: asten (AST interrupt enable) [HRM p 5-9..10] */
    int sir;			/**< IPR SIRR: sir (software interrupt request) [HRM p 5-10..11] */
    int eir;			/**< external interrupt request */
    int slr;			/**< serial line interrupt request */
    int crr;			/**< corrected read error interrupt */
    int pcr;			/**< perf counter interrupt */
    int astrr;			/**< IPR PCTX: astrr (AST request) [HRM p 5-21..22] */
    int aster;			/**< IPR PCTX: aster (AST enable) [HRM p 5-21..22] */
    u64 i_ctl_other;		/**< various bits in IPR I_CTL that have no meaning to the emulator */
    u64 mm_stat;			/**< IPR MM_STAT: memory management status [HRM p 5-28..29] */
    bool hwe;			/**< IPR I_CLT: hwe (allow palmode ins in kernel mode) [HRM p 5-15..17] */
    int m_ctl_spe;		/**< IPR M_CTL: spe (Super Page mode enabled) [HRM p 5-29..30] */
    int i_ctl_spe;		/**< IPR I_CTL: spe (Super Page mode enabled) [HRM p 5-15..18] */
    u64 exc_addr;				/**< IPR EXC_ADDR: address of last exception [HRM p 5-8] */
    u64 pmpc;
    u64 fpcr;				/**< Floating-Point Control Register [HRM p 2-36] */
    bool bIntrFlag;			
    u64 current_pc;			/**< Virtual address of current instruction */
    struct SICache icache[1024];		/**< Instruction cache entries [HRM p 2-11] */
    int next_icache;			/**< Number of next cache entry to use */
    bool lock_flag;
    u64 f[64];			/**< Floating point registers (0-31 normal, 32-63 shadow) */
    int iProcNum;			/**< number of the current processor (0 in a 1-processor system) */
  } state;

#ifdef IDB
  u64 current_pc_physical;		/**< Physical address of current instruction */
#endif

  CTranslationBuffer * itb;	/**< Instruction-Stream Translation Buffer [HRM p 2-5] */
  CTranslationBuffer * dtb;	/**< Data-Stream Translation Buffer [HRM p 2-13] */
};

#define RREG(a) (((a) & 0x1f) + (((state.pc&1) && (((a)&0xc)==0x4) && state.sde)?32:0))

/**
 * Empty the instruction cache.
 **/

inline void CAlphaCPU::flush_icache()
{
  int i;
  for(i=0;i<1024;i++) 
    state.icache[i].valid = false;
  state.next_icache = 0;
}

/**
 * Empty the instruction cache of lines with the ASM bit clear.
 **/

inline void CAlphaCPU::flush_icache_asm()
{
  int i;
  for(i=0;i<1024;i++) 
    if (!state.icache[i].asm_bit)
      state.icache[i].valid = false;
}
/**
 * Return the current address space number.
 **/

inline void CAlphaCPU::set_PAL_BASE(u64 pb)
{
  state.pal_base = pb;
}

/**
 * Get an instruction from the instruction cache.
 * If necessary, fill a new cache block from memory.
 **/

inline int CAlphaCPU::get_icache(u64 address, u32 * data)
{
  int i;
  u64 v_a;
  u64 p_a;
  int result;
  bool asm_bit;

  for (i=0;i<1024;i++)
    {
      if (	state.icache[i].valid
		&& (state.icache[i].asn == state.asn || state.icache[i].asm_bit)
		&& state.icache[i].address == (address & X64(ffffffffffffffc1)))
	{
	  *data = state.icache[i].data[(address>>2)&X64(0f)];

#ifdef IDB
	  current_pc_physical = state.icache[i].p_address + (address & X64(3c));
#endif

	  return 0;
	}
    }

  for(i=0;i<16;i++)
    {
      v_a = (address & X64(ffffffffffffffc0)) | (u64)i << 2;
      if (address & 1)
	p_a = v_a & X64(00000fffffffffff);
      else
      {     
	  result = itb->convert_address(v_a, &p_a, ACCESS_READ, true, state.cm, &asm_bit, false, true);
	  if (result)
	    return result;
	}
      state.icache[state.next_icache].data[i] = (u32)(cSystem->ReadMem(p_a,32));
    }

  state.icache[state.next_icache].valid = true;
  state.icache[state.next_icache].asn = state.asn;
  state.icache[state.next_icache].asm_bit = asm_bit;
  state.icache[state.next_icache].address = address & X64(ffffffffffffffc1);
  state.icache[state.next_icache].p_address = p_a & X64(ffffffffffffffc0);
	
  *data = state.icache[state.next_icache].data[(address>>2)&X64(0f)];

#ifdef IDB
  current_pc_physical = state.icache[state.next_icache].p_address + (address & X64(3c));
#endif

  state.next_icache++;
  if (state.next_icache==1024)
    state.next_icache = 0;
  return 0;
}

/**
 * Convert a virtual address to va_form format.
 * Used for IPR VA_FORM [HRM 5-5..6] and IPR IVA_FORM [HRM 5-9].
 **/

inline u64 CAlphaCPU::va_form(u64 address, bool bIBOX)
{
  switch( bIBOX?state.i_ctl_va_mode:state.va_ctl_va_mode)
    {
    case 0:
      return  ((bIBOX?state.i_ctl_vptb:state.va_ctl_vptb) & X64(fffffffe00000000))
	| ((address>>10)   & X64(00000001fffffff8));
    case 1:
      return   ((bIBOX?state.i_ctl_vptb:state.va_ctl_vptb) & X64(fffff80000000000))
	| ((address>>10)  & X64(0000003ffffffff8))
	| (((address>>10) & X64(0000002000000000)) * X64(3e));
    case 2:
      return  ((bIBOX?state.i_ctl_vptb:state.va_ctl_vptb) & X64(ffffffffc0000000))
	|((address>>10)   & X64(00000000003ffff8));
    }
  return 0;
}

/**
 * Return processor number.
 **/

inline int CAlphaCPU::get_cpuid()
{
  return state.iProcNum;
}

/**
 * Assert or release an external interrupt line to the cpu.
 **/

inline void CAlphaCPU::irq_h(int number, bool assert)
{
  if (assert)
    {
      //        if (number<2 && !(eir & (X64(1)<<number)))
      //            printf("Interrupt %d asserted on CPU %d\n",number,iProcNum);
      state.eir |= (X64(1)<<number);
    }
  else
    {
      //        if (number<2 && (eir & (X64(1)<<number)))
      //            printf("Interrupt %d de-asserted on CPU %d\n",number,iProcNum);
      state.eir &= ~(X64(1)<<number);
    }
}

/**
 * Return program counter value.
 **/

inline u64 CAlphaCPU::get_pc()
{
	return state.pc;
}

#ifdef IDB
inline u64 CAlphaCPU::get_current_pc_physical()
{
	return current_pc_physical;
}
#endif

/**
 * Return program counter value without PALmode bit.
 **/

inline u64 CAlphaCPU::get_clean_pc()
{
	return state.pc & ~X64(3);
}

/**
 * Jump to next instruction
 **/

inline void CAlphaCPU::next_pc()
{
	state.pc += 4;
}

/**
 * Set program counter to a certain value.
 **/

inline void CAlphaCPU::set_pc(u64 p_pc)
{
	state.pc = p_pc;
}

/**
 * Get a register value.
 * If \a translate is true, use shadow registers if currently enabled.
 **/

inline u64 CAlphaCPU::get_r(int i, bool translate)
{
  if (translate)
    return state.r[RREG(i)];
  else
    return state.r[i];
}

/**
 * Get a fp register value.
 **/

inline u64 CAlphaCPU::get_f(int i)
{
  return state.f[i];
}

inline u64 CAlphaCPU::get_pal_base()
{
  return state.pal_base;
}
/**
 * Get the processor base register.
 * A bit fuzzy...
 **/

inline u64 CAlphaCPU::get_prbr(void)
{
  u64 v_prbr;	// virtual
  u64 p_prbr;	// physical
  bool b;

  if (state.r[21+32] && (   (state.r[21+32]+0xaf) < (X64(1)<<cSystem->get_memory_bits())))
    v_prbr = cSystem->ReadMem(state.r[21+32] + 0xa8,64);
  else
    v_prbr = cSystem->ReadMem(0x70a8 + (0x200 * get_cpuid()),64);

  if (dtb->convert_address(v_prbr, &p_prbr, 0, false, 0, &b, false, false))
    p_prbr = v_prbr;

  if (p_prbr > (X64(1)<<cSystem->get_memory_bits()))
    p_prbr = 0;

  return p_prbr;
}

inline u64 CAlphaCPU::get_hwpcb(void)
{
  u64 v_pcb;	// virtual
  u64 p_pcb;	// physical
  bool b;

  if (state.r[21+32] && (   (state.r[21+32]+0x17) < (X64(1)<<cSystem->get_memory_bits())))
    v_pcb = cSystem->ReadMem(state.r[21+32] + 0x10,64);
  else
    v_pcb = cSystem->ReadMem(0x7010 + (0x200 * get_cpuid()),64);

  if (dtb->convert_address(v_pcb, &p_pcb, 0, false, 0, &b, false, false))
    p_pcb = v_pcb;

  if (p_pcb > (X64(1)<<cSystem->get_memory_bits()))
    p_pcb = 0;

  return p_pcb;
}

inline CTranslationBuffer * CAlphaCPU::get_tb(bool bIBOX)
{
  if (bIBOX)
    return itb;
  else
    return dtb;
};

inline int CAlphaCPU::get_asn(bool bIBOX)
{
  if (bIBOX)
    return state.asn;
  else
  {
    if (state.asn0 != state.asn1)
      printf("Warning: MBOX-ASN needed, and asn0 != asn1!\n");
    return state.asn0;
  }
}

inline int CAlphaCPU::get_spe(bool bIBOX)
{
  if (bIBOX)
    return state.i_ctl_spe;
  else
    return state.m_ctl_spe;
}
#endif // !defined(INCLUDED_ALPHACPU_H)
