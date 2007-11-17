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
 * Contains the code for the emulated DecChip 21264CB EV68 Alpha processor.
 *
 * \bug Rounding and trap modes are not used for floating point ops.
 * \bug /V is ignored for integer ops.
 *
 * X-1.44       Camiel Vanderhoeven                             16-NOV-2007
 *      Avoid more compiler warnings.
 *
 * X-1.43       Camiel Vanderhoeven                             16-NOV-2007
 *      Avoid compiler warning about default without any cases.
 *
 * X-1.42       Camiel Vanderhoeven                             08-NOV-2007
 *      Instruction set complete now.
 *
 * X-1.41       Camiel Vanderhoeven                             06-NOV-2007
 *      Performance improvements to ICACHE: last result is kept; cache
 *      lines are larger (512 DWORDS in stead of 16 DWORDS), cache size is
 *      configurable (both number of cache lines and size of each cache 
 *      line), memcpy is used to move memory into the ICACHE.
 *      CAVEAT: ICACHE can only be filled from memory (not from I/O).
 *
 * X-1.40       Camiel Vanderhoeven                             02-NOV-2007
 *      Added integer /V instructions.
 *
 * X-1.39       Camiel Vanderhoeven                             02-NOV-2007
 *      Added missing floating point instructions.
 *
 * X-1.38       Eduardo Marcelo Ferrat                          31-OCT-2007
 *      EXC_SUM contained the wrong register (3 in stead of 1) on a DTBM
 *      exception. Added instructions for CVTDG, CVTGD, MULG, CVTGF.
 *
 * X-1.37       Camiel Vanderhoeven                             18-APR-2007
 *      Faster lockstep mechanism (send info 50 cpu cycles at a time)
 *
 * X-1.36       Camiel Vanderhoeven                             11-APR-2007
 *      Moved all data that should be saved to a state file to a structure
 *      "state".
 *
 * X-1.35	Camiel Vanderhoeven				10-APR-2007
 *	New mechanism for SRM replacements. Where these need to be executed,
 *	CSystem::LoadROM() puts a special opcode (a CALL_PAL instruction
 *	with an otherwise illegal operand of 0x01234xx) in memory. 
 *	CAlphaCPU::DoClock() recognizes these opcodes and performs the SRM
 *	action.
 *
 * X-1.34	Camiel Vanderhoeven				10-APR-2007
 *	Unintentional version number increase.
 *
 * X-1.33       Camiel Vanderhoeven                             30-MAR-2007
 *      Formatting.
 *
 * X-1.32	Camiel Vanderhoeven				29-MAR-2007
 *	Added AST to the list of conditions that cause the processor to go to 
 *	the interrupt PAL vector (680).
 *
 * X-1.31	Brian Wheeler					28-MAR-2007
 *	Fixed missing ) after #if !defined(SRM_NO_SPEEDUPS
 *
 * X-1.30	Camiel Vanderhoeven				26-MAR-2007
 *   a)	Possibility to disable SRM-code replacements with the defines
 *	SRM_NO_IDE, SRM_NO_SRL, and SRM_NO_SPEEDUPS
 *   b) Possibility to send SRM-code replacement debugging messages to the
 *	console, with the defines DEBUG_SRM_IDE and DEBUG_SRM_SRL
 *   c)	Added software-interrupts to the list of conditions that can cause
 *	the processot to go to the interrupt PAL vector (680)
 *
 * X-1.29	Camiel Vanderhoeven				14-MAR-2007
 *	Formatting.
 *
 * X-1.28	Camiel Vanderhoeven				14-MAR-2007
 *	Fixed typo in "case 0x22: OP(CPYSE,F12_f3);"
 *
 * X-1.27	Camiel Vanderhoeven				13-MAR-2007
 *	Added some floating-point opcodes, added es_float.h inclusion
 *
 * X-1.26	Camiel Vanderhoeven				12-MAR-2007
 *   a)	Changed call to CTranslationBuffer::convert_address (arguments list
 *	changed)
 *   b) Set values for EXC_SUM and MM_STAT on various exceptions
 *
 * X-1.25	Camiel Vanderhoeven				9-MAR-2007
 *	In the listing-process, addresses were executed twice
 *
 * X-1.24	Camiel Vanderhoeven				8-MAR-2007
 *   a)	Changed call to CTranslationBuffer::write_pte (arguments list
 *	changed)
 *   b)	Backed-out X-1.23 as real problem was solved. (X-1.3 in cpu_bwx.h)
 *
 * X-1.23	Camiel Vanderhoeven				7-MAR-2007				
 *	HACK to stop APB.EXE from crashing when passing bootflags
 *
 * X-1.22	Camiel Vanderhoeven				3-MAR-2007
 *	Wrote code to be executed in stead of SRM console code for writing
 *	to the serial port, and reading from IDE disks. Mechanism is based 
 *	on recognition of the PC value. Should be replaced with a better 
 *	mechanism in the future.
 *
 * X-1.21	Camiel Vanderhoeven				2-MAR-2007
 *	Initialize debug_string to "".
 *
 * X-1.20	Camiel Vanderhoeven				2-MAR-2007
 *	Fixed problem in Save and RestoreState; argument f conflicted with
 *	class member f.
 *
 * X-1.19	Camiel Vanderhoeven				28-FEB-2007
 *	Added support for the lockstep-mechanism.
 *
 * X-1.18	Camiel Vanderhoeven				27-FEB-2007
 *	Removed an unreachable "return 0;" line from DoClock
 *
 * X-1.17	Camiel Vanderhoeven				22-FEB-2007
 *	E_FAULT returned from translation buffer now causes DFAULT exception
 *
 * X-1.16	Camiel Vanderhoven				22-FEB-2007
 *   a)	Changed call to CTranslationBuffer::convert_address (arguments list
 *	changed)
 *   b)	Fixed HW_MTPR and HW_MFPR opcodes
 *
 * X-1.15	Camiel Vanderhoeven				19-FEB-2007
 *	Fixed preprocessor macro concatenation bug (used ## both before and
 *	after the literal; changed this to only before).
 *
 * X-1.14	Camiel Vanderhoeven				18-FEB-2007
 *	Put all actual code behind the processor opcodes in cpu_xxx.h include
 *	files, and replaced them with OP(...,...) macro's in this file.
 *
 * X-1.13       Camiel Vanderhoeven                             16-FEB-2007
 *   a) Added CAlphaCPU::listing.
 *   b) Clocking changes (due to changes in CSystem): CAlphaCPU::DoClock now 
 *      returns a value, and the CPU is registered as a fast clocked device.
 *      
 * X-1.12       Brian Wheeler                                   13-FEB-2007
 *      Different algorithm used for UMULH (previous algorithm suffered from
 *      portability issues).
 *
 * X-1.11       Camiel Vanderhoeven                             13-FEB-2007
 *   a) Bugfix in the UMULH instruction.
 *   b) Bugfix in the HW_MTPR VA_CTL instruction. Now updates va_ctl_va_mode
 *      instead of i_ctl_va_mode.
 *
 * X-1.10       Camiel Vanderhoeven                             12-FEB-2007
 *   a) Moved debugging macro's to cpu_debug.h
 *   b) Cleaned up SEXT and REG macro's (a lot neater now)
 *   c) Moved CAlphaCPU::get_r and CAlphaCPU::get_prbr to AlphaCPU.h as
 *      inline functions
 *   d) Use SEXT macro in a some places where exotic constructions were used 
 *      previously
 *
 * X-1.9        Camiel Vanderhoeven                             12-FEB-2007
 *   a) Added X64_BYTE, X64_WORD, X64_LONG and X64_QUAD, and used these 
 *      instead of the corresponding values.
 *   b) Added ier to the variables that are saved to the state file.
 *
 * X-1.8        Camiel Vanderhoeven                             9-FEB-2007
 *   a) Moved debugging flags (booleans) to CSystem.cpp.
 *   b) Removed loggin of last_write_loc and last_write_val
 *
 * X-1.7        Camiel Vanderhoeven                             7-FEB-2007
 *      Made various dubugging-related statements dependent on the 
 *      definition of IDB (interactive debugger)
 *
 * X-1.6        Camiel Vanderhoeven                             3-FEB-2007
 *      Inline function printable moved to StdAfx.h
 *
 * X-1.5        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.4        Brian Wheeler                                   3-FEB-2007
 *      More scanf and printf statements made compatible with Linux/GCC/glibc.
 *
 * X-1.3        Brian Wheeler                                   3-FEB-2007
 *      Scanf and printf statements made compatible with Linux/GCC/glibc.
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
#include "AlphaCPU.h"
#include "TraceEngine.h"
#include "lockstep.h"
#include "es40_float.h"
#include "cpu_memory.h"
#include "cpu_control.h"
#include "cpu_arith.h"
#include "cpu_logical.h"
#include "cpu_bwx.h"
#include "cpu_fp_memory.h"
#include "cpu_fp_branch.h"
#include "cpu_fp_operate.h"
#include "cpu_misc.h"
#include "cpu_vax.h"
#include "cpu_mvi.h"
#include "cpu_pal.h"
#include "cpu_srm.h"
#include "cpu_debug.h"

#include "Serial.h"
#include "AliM1543C.h"

extern CSerial * srl[2];
extern CAliM1543C * ali;

#if defined(IDB)

#define OP(mnemonic, format)							\
        PRE_##format(mnemonic);							\
	if (!bListing) {							\
	  DO_##mnemonic;							\
	}									\
	POST_##format;								\
        handle_debug_string(dbg_string);					\
	return 0;

#else //defined(IDB)

#define OP(mnemonic, format)							\
	DO_##mnemonic;								\
	return 0;

#endif //defined(IDB)

// INTERRUPT VECTORS
#define DTBM_DOUBLE_3 X64(100)
#define DTBM_DOUBLE_4 X64(180)
#define FEN           X64(200)
#define UNALIGN       X64(280)
#define DTBM_SINGLE   X64(300)
#define DFAULT        X64(380)
#define OPCDEC        X64(400)
#define IACV          X64(480)
#define MCHK          X64(500)
#define ITB_MISS      X64(580)
#define ARITH         X64(600)
#define INTERRUPT     X64(680)
#define MT_FPCR       X64(700)
#define RESET         X64(780)

/** Chip ID (EV68CB pass 4) [HRM p 5-16]; actual value derived from SRM-code */
#define CPU_CHIP_ID	0x21
/** Major CPU type (EV68CB) [ARM pp D-1..3] */
#define CPU_TYPE_MAJOR	12
/** Minor CPU type (pass 4) [ARM pp D-1..3] */
#define CPU_TYPE_MINOR	6
/** Implementation version [HRM p 2-38; ARM p D-5] */
#define CPU_IMPLVER	2
/** Architecture mask [HRM p 2-38; ARM p D-4]; FIX not implemented */
#define CPU_AMASK	X64(1305)

#define X64_BYTE	X64(ff)
#define X64_WORD	X64(ffff)
#define X64_LONG	X64(ffffffff)
#define X64_QUAD	X64(ffffffffffffffff)

/**
 * Constructor.
 **/

CAlphaCPU::CAlphaCPU(CSystem * system) : CSystemComponent (system)
{
  state.iProcNum = cSystem->RegisterCPU(this);
  cSystem = system;
  int i;

  cSystem->RegisterClock(this, false);

  state.pc = 0;
  state.last_found_icache = 0;
  state.bIntrFlag = false;

  for(i=0;i<64;i++) 
    {
      state.r[i] = 0;
      state.f[i] = 0;
    }

  flush_icache();

  state.alt_cm = 0;
  state.asn = 0;
  state.asn0 = 0;
  state.asn1 = 0;
  state.asten = 0;
  state.aster = 0;
  state.astrr = 0;
  state.cc = 0;
  state.cc_ena = false;
  state.cc_offset = 0;
  state.cm = 0;
  state.cren = 0;
  state.crr = 0;
  state.current_pc = 0;
  state.dc_ctl = 3;
  state.dc_ctl = 0;
  state.dc_stat = 0;
  state.eien = 0;
  state.eir = 0;
  state.exc_addr = 0;
  state.exc_sum = 0;
  state.fault_va = 0;
  state.fpcr = X64(8ff0000000000000);
  state.fpen = true;
  state.hwe = false;
  state.i_ctl_other = X64(502086);
  state.i_ctl_va_mode = 0;
  state.i_ctl_vptb = 0;
  state.i_stat = 0;
  state.mm_stat = 0;
  state.pal_base = 0;
  state.pc = 0;
  state.pcen = 0;
  state.pcr = 0;
  state.pctr_ctl = 0;
  state.pmpc = 0;
  state.ppcen = false;
  state.sde = false;
  state.sien = 0;
  state.sir = 0;
  state.slen = 0;
  state.slr = 0;
  state.smc = 1;
  state.m_ctl_spe = 0;
  state.i_ctl_spe = 0;
  state.va_ctl_va_mode = 0;
  state.va_ctl_vptb = 0;

  itb = new CTranslationBuffer (this, true);
  dtb = new CTranslationBuffer (this, false);

  // SROM imitation...

  dtb->write_tag(0,0);
  dtb->write_pte(0,X64(ff61));

#if defined(IDB)
  bListing = false;
#endif
  
  printf("%%CPU-I-INIT: Alpha AXP 21264 EV68 processor %d initialized.\n", state.iProcNum);
}

/**
 * Destructor.
 **/

CAlphaCPU::~CAlphaCPU()
{

}

/**
 * Sign-extend \a bits - bit value \a x to a 64-bit signed value.
 **/

#define SEXT(x,bits) (((x)&((X64(1)<<(bits))-1)) | \
	( (((x)>>((bits)-1))&1) ? (X64_QUAD-((X64(1)<<(bits))-1)) : 0 ) )

#define DISP_12 (SEXT(ins,12))
#define DISP_13 (SEXT(ins,13))
#define DISP_16 (SEXT(ins,16))
#define DISP_21 (SEXT(ins,21))

#define DATA_PHYS(addr,access,check,alt,vpte) {				\
    int dp_result;							\
    dp_result = dtb->convert_address(addr, &phys_address, access, check, alt?state.alt_cm:state.cm, &temp_bool, false, true); \
    if (dp_result) {							\
      state.fault_va = addr;							\
      switch (dp_result) {						\
      case E_NOT_FOUND:							\
        if (vpte) {							\
	      state.exc_sum = REG_1<<8;						\
	      GO_PAL(DTBM_DOUBLE_3);					\
	    } else {							\
          state.mm_stat = (((opcode==0x1b || opcode==0x1f)?opcode-0x18:opcode)<<4) |	\
		    (access);						\
		  state.exc_sum = REG_1<<8;					\
	      GO_PAL(DTBM_SINGLE);						\
	    }								\
	break;								\
      case E_ACCESS:							\
        if (!vpte)							\
	  state.mm_stat = (((opcode==0x1b || opcode==0x1f)?opcode-0x18:opcode)<<4) |	\
		    (access) | 2;					\
	state.exc_sum = REG_3<<8;						\
	GO_PAL(DFAULT);							\
	break;								\
      case E_FAULT:							\
       if (!vpte)							\
	  state.mm_stat = (((opcode==0x1b || opcode==0x1f)?opcode-0x18:opcode)<<4) |	\
	  (access?9:4);							\
	state.exc_sum = REG_3<<8;						\
	GO_PAL(DFAULT);							\
	break; }							\
      return 0;	} }

#define ALIGN_PHYS(a) (phys_address & ~((u64)((a)-1)))

/**
 * Normal variant of read action
 * In reality, these would generate an alignment trap, and the exception
 * handler would put things straight. Instead, to speed things up, we'll
 * just perform the read as requested using the unaligned address.
 **/

#define READ_PHYS(size)				\
  cSystem->ReadMem(phys_address, size)

/**
 * Normal variant of write action
 * In reality, these would generate an alignment trap, and the exception
 * handler would put things straight. Instead, to speed things up, we'll
 * just perform the write as requested using the unaligned address.
 **/

#define WRITE_PHYS(data,size)			\
  cSystem->WriteMem(phys_address, size, data)

/**
 * NO-TRAP (NT) variants of read action.
 * This is used for HW_LD, where alignment traps are 
 * inhibited. We'll align the adress and read using the aligned
 * address.
 **/

#define READ_PHYS_NT(size)			\
  cSystem->ReadMem(ALIGN_PHYS((size)/8), size)

/**
 * NO-TRAP (NT) variants of write action.
 * This is used for HW_ST, where alignment traps are 
 * inhibited. We'll align the adress and write using the aligned
 * address.
 **/

#define WRITE_PHYS_NT(data,size) 				\
    cSystem->WriteMem(ALIGN_PHYS((size)/8), size, data)

#define REG_1 RREG(ins>>21)
#define REG_2 RREG(ins>>16)
#define REG_3 RREG(ins)

#define FREG_1 ((ins>>21) & 0x1f)
#define FREG_2 ((ins>>16) & 0x1f)
#define FREG_3 ( ins      & 0x1f)

#define V_2 ( (ins&0x1000)?((ins>>13)&0xff):state.r[REG_2] )

#if defined(IDB)

void handle_debug_string(char * s)
{
#if defined(LS_SLAVE) || defined(LS_MASTER)
//    lockstep_compare(s);
    *dbg_strptr++ = '\n';
    *dbg_strptr='\0';
#else
  if (*s)
    printf("%s\n",s);
#endif
}

#endif

/**
 * Called each clock-cycle.
 * This is where the actual CPU emulation takes place. Each clocktick, one instruction
 * is processed by the processor. The instruction pipeline is not emulated, things are
 * complicated enough as it is. The one exception is the instruction cache, which is
 * implemented, to accomodate self-modifying code.
 **/

int CAlphaCPU::DoClock()
{
  u32 ins;
  int i;
  int result;
  u64 phys_address;
  u64 temp_64;
  u64 temp_64_1;
  u64 temp_64_2;
  u64 temp_64_x;
  u64 temp_64_y;
  u64 temp_64_a;
  u64 temp_64_b;
  u64 temp_64_c;
  u64 temp_64_d;
  u64 temp_64_hi;
  u64 temp_64_lo;
  u32 temp_32;
  u32 temp_32_1;
  char temp_char2[2];
  bool temp_bool;

  int opcode;
  int function;


#if defined(IDB)
  char * funcname = 0;
  char dbg_string[1000] = "";
#if !defined(LS_MASTER) && !defined(LS_SLAVE)
  char * dbg_strptr = dbg_string;
#endif
#endif

   state.current_pc = state.pc;

   if (DO_ACTION)
    {
      // check for interrupts
      if ((!(state.pc&X64(1))) && ((state.eien & state.eir) || 
                                   (state.sien & state.sir) || 
                                   (state.asten && (state.aster & state.astrr & ((1<<(state.cm+1))-1) ))))
	{
	  GO_PAL(INTERRUPT);
	  return 0;
	}

      // get next instruction
      result = get_icache(state.pc,&ins);
      if (result)
	{
	  switch (result)
	    {
	    case E_NOT_FOUND:
	      GO_PAL(ITB_MISS);
	      break;
	    case E_ACCESS:
	      GO_PAL(IACV);
	      break;
	    }
	  return 0;
	}
    }
  else
    {
      ins = (u32)(cSystem->ReadMem(state.pc,32));
    }
    state.pc += 4;

  state.r[31] = 0;
  state.f[31] = 0;

  if (state.cc_ena)
  {
    if (DO_ACTION)
    {
      if (state.pc>X64(600000))
	state.cc+=X64(1654321);
      else
	state.cc+=83;
    }
  }

  opcode = ins >>26;

  switch (opcode)
    {
    case 0x00: // CALL_PAL
      function = ins&0x1fffffff;
      switch (function)
      {
      case 0x01234ff: 
        temp_32 = 0;
        temp_32_1 = 0;
        temp_char2[0] = 0;
        UNKNOWN2;

#if !defined(SRM_NO_SRL)
      case 0x0123400: OP(SRM_WRITE_SERIAL, NOP);
#endif

#if !defined(SRM_NO_IDE)
      case 0x0123401: OP(SRM_READ_IDE_DISK, NOP);
#endif

        default: OP(CALL_PAL,PAL);
      }

    case 0x08: OP(LDA,MEM);
    case 0x09: OP(LDAH,MEM);
    case 0x0a: OP(LDBU,MEM);
    case 0x0b: OP(LDQ_U,MEM);
    case 0x0c: OP(LDWU,MEM);
    case 0x0d: OP(STW,MEM);
    case 0x0e: OP(STB,MEM);
    case 0x0f: OP(STQ_U,MEM);

    case 0x10: // op
      function = (ins>>5) & 0x7f;
      switch (function) // ignore /V for now
      {
        case 0x00:
        case 0x40: OP(ADDL,R12_R3);
        case 0x02: OP(S4ADDL,R12_R3);
        case 0x09:
        case 0x49: OP(SUBL,R12_R3);
        case 0x0b: OP(S4SUBL,R12_R3);
        case 0x0f: OP(CMPBGE,R12_R3);
        case 0x12: OP(S8ADDL,R12_R3);
        case 0x1b: OP(S8SUBL,R12_R3);
        case 0x1d: OP(CMPULT,R12_R3);
        case 0x20: 
        case 0x60: OP(ADDQ,R12_R3);
        case 0x22: OP(S4ADDQ,R12_R3);
        case 0x29:
        case 0x69: OP(SUBQ,R12_R3);
        case 0x2b: OP(S4SUBQ,R12_R3);
        case 0x2d: OP(CMPEQ,R12_R3);
        case 0x32: OP(S8ADDQ,R12_R3);
        case 0x3b: OP(S8SUBQ,R12_R3);
        case 0x3d: OP(CMPULE,R12_R3);
        case 0x4d: OP(CMPLT,R12_R3);
        case 0x6d: OP(CMPLE,R12_R3);
        default:   UNKNOWN2;
      }

    case 0x11: // op
      function = (ins>>5) & 0x7f;
      switch (function)
      {
        case 0x00: OP(AND,R12_R3);
        case 0x08: OP(BIC,R12_R3);
        case 0x14: OP(CMOVLBS,R12_R3);
        case 0x16: OP(CMOVLBC,R12_R3);
        case 0x20: OP(BIS,R12_R3);
        case 0x24: OP(CMOVEQ,R12_R3);
        case 0x26: OP(CMOVNE,R12_R3);
        case 0x28: OP(ORNOT,R12_R3);
        case 0x40: OP(XOR,R12_R3);
        case 0x44: OP(CMOVLT,R12_R3);
        case 0x46: OP(CMOVGE,R12_R3);
        case 0x48: OP(EQV,R12_R3);
        case 0x61: OP(AMASK,R2_R3);
        case 0x64: OP(CMOVLE,R12_R3);
        case 0x66: OP(CMOVGT,R12_R3);
        case 0x6c: OP(IMPLVER,X_R3);
        default:   UNKNOWN2;
      }

    case 0x12:
      function = (ins>>5) & 0x7f;
      switch (function)
      {
        case 0x02: OP(MSKBL,R12_R3);
        case 0x06: OP(EXTBL,R12_R3);
        case 0x0b: OP(INSBL,R12_R3);
        case 0x12: OP(MSKWL,R12_R3);
        case 0x16: OP(EXTWL,R12_R3);
        case 0x1b: OP(INSWL,R12_R3);
        case 0x22: OP(MSKLL,R12_R3);
        case 0x26: OP(EXTLL,R12_R3);
        case 0x2b: OP(INSLL,R12_R3);
        case 0x30: OP(ZAP,R12_R3);
        case 0x31: OP(ZAPNOT,R12_R3);
        case 0x32: OP(MSKQL,R12_R3);
        case 0x34: OP(SRL,R12_R3);
        case 0x36: OP(EXTQL,R12_R3);
        case 0x39: OP(SLL,R12_R3);
        case 0x3b: OP(INSQL,R12_R3);
        case 0x3c: OP(SRA,R12_R3);
        case 0x52: OP(MSKWH,R12_R3);
        case 0x57: OP(INSWH,R12_R3);
        case 0x5a: OP(EXTWH,R12_R3);
        case 0x62: OP(MSKLH,R12_R3);
        case 0x67: OP(INSLH,R12_R3);
        case 0x6a: OP(EXTLH,R12_R3);
        case 0x72: OP(MSKQH,R12_R3);
        case 0x77: OP(INSQH,R12_R3);
        case 0x7a: OP(EXTQH,R12_R3);
        default:   UNKNOWN2;
      }

    case 0x13:
      function = (ins>>5) & 0x7f;
      switch (function) // ignore /V for now
      {
	case 0x00: 
        case 0x40: OP(MULL,R12_R3);
	case 0x20: 
        case 0x60: OP(MULQ,R12_R3);
	case 0x30: OP(UMULH,R12_R3);
	default:   UNKNOWN2;
      }

    case 0x14:
      function = (ins>>5) & 0x7ff;
      switch(function)
      {
      case 0x004: OP(ITOFS,R1_F3);
      case 0x00a: 
      case 0x08a: 
      case 0x10a: 
      case 0x18a: 
      case 0x40a: 
      case 0x48a: 
      case 0x50a: 
      case 0x58a: OP(SQRTF,F2_F3);
      case 0x00b:
      case 0x04b: 
      case 0x08b: 
      case 0x0cb: 
      case 0x10b: 
      case 0x14b: 
      case 0x18b: 
      case 0x1cb: 
      case 0x50b: 
      case 0x54b: 
      case 0x58b: 
      case 0x5cb: 
      case 0x70b: 
      case 0x74b: 
      case 0x78b: 
      case 0x7cb: OP(SQRTS,F2_F3);
      case 0x014: OP(ITOFF,R1_F3);
      case 0x024: OP(ITOFT,R1_F3);
      case 0x02a: 
      case 0x0aa: 
      case 0x12a: 
      case 0x1aa: 
      case 0x42a: 
      case 0x4aa: 
      case 0x52a: 
      case 0x5aa: OP(SQRTG,F2_F3);
      case 0x02b: 
      case 0x06b: 
      case 0x0ab: 
      case 0x0eb: 
      case 0x12b: 
      case 0x16b: 
      case 0x1ab: 
      case 0x1eb: 
      case 0x52b: 
      case 0x56b: 
      case 0x5ab: 
      case 0x5eb: 
      case 0x72b: 
      case 0x76b: 
      case 0x7ab: 
      case 0x7eb: OP(SQRTT,F2_F3);
      default:    UNKNOWN2;
      }

    case 0x15:
      function = (ins>>5) & 0x7ff;
      switch(function)
      {
        case 0x0a5:
        case 0x4a5: OP(CMPGEQ,F12_F3);
        case 0x0a6:
        case 0x4a6: OP(CMPGLT,F12_F3);
        case 0x0a7:
        case 0x4a7: OP(CMPGLE,F12_F3);
        case 0x03c:
        case 0x0bc: OP(CVTQF,F2_F3);
        case 0x03e: 
        case 0x0be: OP(CVTQG,F2_F3);
        default: if (function & 0x200) {
                   UNKNOWN2;
                 }
                 switch(function & 0x7f)
                 {
                 case 0x000: OP(ADDF,F12_F3);
                 case 0x001: OP(SUBF,F12_F3);
                 case 0x002: OP(MULF,F12_F3);
                 case 0x003: OP(DIVF,F12_F3);
                 case 0x01e: OP(CVTDG,F2_F3);
                 case 0x020: OP(ADDG,F12_F3);
                 case 0x021: OP(SUBG,F12_F3);
	         case 0x022: OP(MULG,F12_F3);
                 case 0x023: OP(DIVG,F12_F3);
	         case 0x02c: OP(CVTGF,F12_F3);
	         case 0x02d: OP(CVTGD,F2_F3);
                 case 0x02f: OP(CVTGQ,F2_F3);
                 default:   UNKNOWN2;
                 }
      }
    case 0x16:
      function = (ins>>5) & 0x7ff;
      switch(function)
      {
      case 0x0a4:
      case 0x5a4: OP(CMPTUN,F12_F3);
      case 0x0a5: 
      case 0x5a5: OP(CMPTEQ,F12_F3);
      case 0x0a6: 
      case 0x5a6: OP(CMPTLT,F12_F3);
      case 0x0a7: 
      case 0x5a7: OP(CMPTLE,F12_F3);
      case 0x2ac:
      case 0x6ac: OP(CVTST,F2_F3);
      default: if (((function & 0x600) == 0x200) || ((function & 0x500) == 0x400)) {
                 UNKNOWN2;
               }
               switch (function & 0x3f)
               {
               case 0x00: OP(ADDS,F12_F3);
               case 0x01: OP(SUBS,F12_F3);
               case 0x02: OP(MULS,F12_F3);
               case 0x03: OP(DIVS,F12_F3);
               case 0x20: OP(ADDT,F12_F3);
               case 0x21: OP(SUBT,F12_F3);
               case 0x22: OP(MULT,F12_F3);
               case 0x23: OP(DIVT,F12_F3);
               case 0x2c: OP(CVTTS,F2_F3);
               case 0x2f: OP(CVTTQ,F2_F3);
               case 0x3c: if ((function &0x300) == 0x100) {
                            UNKNOWN2;
                          }
                          OP(CVTQS,F2_F3);
               case 0x3e: if ((function &0x300) == 0x100) {
                            UNKNOWN2;
                          }
                          OP(CVTQT,F2_F3);
               default:   UNKNOWN2;
               }
      }
    case 0x17:
      function = (ins>>5) & 0x7ff;
      switch (function)
      {
      case 0x010: OP(CVTLQ,F2_F3);
      case 0x020: OP(CPYS,F12_F3);
      case 0x021: OP(CPYSN,F12_F3);
      case 0x022: OP(CPYSE,F12_F3);
      case 0x024: OP(MT_FPCR,X_F1);
      case 0x025: OP(MF_FPCR,X_F1);
      case 0x02a: OP(FCMOVEQ,F12_F3);
      case 0x02b: OP(FCMOVNE,F12_F3);
      case 0x02c: OP(FCMOVLT,F12_F3);
      case 0x02d: OP(FCMOVGE,F12_F3);
      case 0x02e: OP(FCMOVLE,F12_F3);
      case 0x02f: OP(FCMOVGT,F12_F3);
      case 0x030:
      case 0x130:
      case 0x530: OP(CVTQL,F12_F3);
      default:   UNKNOWN2;
      }

    case 0x18:
      function = (ins & 0xffff);
      switch (function)
	{
	case 0x0000: OP(TRAPB,NOP);
	case 0x0400: OP(EXCB,NOP);
	case 0x4000: OP(MB,NOP);
	case 0x4400: OP(WMB,NOP);
	case 0x8000: OP(FETCH,NOP);
	case 0xA000: OP(FETCH_M,NOP);
	case 0xC000: OP(RPCC,X_R1);
	case 0xE000: OP(RC,X_R1);
	case 0xE800: OP(ECB,NOP);
	case 0xF000: OP(RS,X_R1);
	case 0xF800: OP(WH64,NOP);
	case 0xFC00: OP(WH64EN,NOP);
	default:     UNKNOWN2;
	}

    case 0x19: // HW_MFPR
      function = (ins>>8) & 0xff;
      OP(HW_MFPR,MFPR);

    case 0x1a: OP(JMP,JMP);

    case 0x1b: // HW_LD
      function = (ins>>12) & 0xf;
      if (function&1) {
	OP(HW_LDQ,HW_LD);
      } else {
	OP(HW_LDL,HW_LD);
      }

    case 0x1c: // op
      function = (ins>>5) & 0x7f;
      switch (function)
        {
        case 0x00: OP(SEXTB,R2_R3);
        case 0x01: OP(SEXTW,R2_R3);
        case 0x30: OP(CTPOP,R2_R3);
        case 0x31: OP(PERR,R2_R3);
        case 0x32: OP(CTLZ,R2_R3);
        case 0x33: OP(CTTZ,R2_R3);
        case 0x34: OP(UNPKBW,R2_R3);
        case 0x35: OP(UNPKBL,R2_R3);
        case 0x36: OP(PKWB,R2_R3);
        case 0x37: OP(PKLB,R2_R3);
        case 0x38: OP(MINSB8,R12_R3);
        case 0x39: OP(MINSW4,R12_R3);
        case 0x3a: OP(MINUB8,R12_R3);
        case 0x3b: OP(MINUW4,R12_R3);
        case 0x3c: OP(MAXUB8,R12_R3);
        case 0x3d: OP(MAXUW4,R12_R3);
        case 0x3e: OP(MAXSB8,R12_R3);
        case 0x3f: OP(MAXSW4,R12_R3);
        case 0x70: OP(FTOIT,F1_R3);
        case 0x78: OP(FTOIS,F1_R3);
        default:   UNKNOWN2;
        }

    case 0x1d: // HW_MTPR
      function = (ins>>8) & 0xff;
      OP(HW_MTPR,MTPR);

    case 0x1e: OP(HW_RET,RET);

    case 0x1f: // HW_ST
      function = (ins>>12) & 0xf;
      if (function&1) {
	OP(HW_STQ,HW_ST);
      } else {
	OP(HW_STL,HW_ST);
      }

    case 0x20: OP(LDF, FMEM);
    case 0x21: OP(LDG, FMEM);
    case 0x22: OP(LDS, FMEM);
    case 0x23: OP(LDT,FMEM);
    case 0x24: OP(STF,FMEM);
    case 0x25: OP(STG,FMEM);
    case 0x26: OP(STS,FMEM);
    case 0x27: OP(STT,FMEM);
    case 0x28: OP(LDL,MEM);
    case 0x29: OP(LDQ,MEM);
    case 0x2a: OP(LDL_L,MEM);
    case 0x2b: OP(LDQ_L,MEM);
    case 0x2c: OP(STL,MEM);
    case 0x2d: OP(STQ,MEM);
    case 0x2e: OP(STL_C,MEM);
    case 0x2f: OP(STQ_C,MEM);
    case 0x30: OP(BR,BR);
    case 0x31: OP(FBEQ,FCOND);
    case 0x32: OP(FBLT,FCOND);
    case 0x33: OP(FBLE,FCOND);
    case 0x34: OP(BSR,BSR);
    case 0x35: OP(FBNE,FCOND);
    case 0x36: OP(FBGE,FCOND);
    case 0x37: OP(FBGT,FCOND);
    case 0x38: OP(BLBC,COND);
    case 0x39: OP(BEQ,COND);
    case 0x3a: OP(BLT,COND);
    case 0x3b: OP(BLE,COND);
    case 0x3c: OP(BLBS,COND);
    case 0x3d: OP(BNE,COND);
    case 0x3e: OP(BGE,COND);
    case 0x3f: OP(BGT,COND);

    default:
      UNKNOWN1;
    }
}

#if defined(IDB)
void CAlphaCPU::listing(u64 from, u64 to)
{
  printf("%%CPU-I-LISTNG: Listing from %016" LL "x to %016" LL "x\n",from,to);
  u64 iSavedPC;
  bool bSavedDebug;
  iSavedPC = state.pc;
  bSavedDebug = bDisassemble;
  bDisassemble = true;
  bListing = true;
  for(state.pc=from;state.pc<=to;DoClock());
  bListing = false;
  state.pc = iSavedPC;
  bDisassemble = bSavedDebug;
}
#endif

/**
 * Save state to a Virtual Machine State file.
 **/

void CAlphaCPU::SaveState(FILE *f)
{
  fwrite(&state,sizeof(state),1,f);
  
  itb->SaveState(f);
  dtb->SaveState(f);
}

/**
 * Restore state from a Virtual Machine State file.
 **/

void CAlphaCPU::RestoreState(FILE *f)
{
  fread(&state,sizeof(state),1,f);

  itb->RestoreState(f);
  dtb->RestoreState(f);
}

