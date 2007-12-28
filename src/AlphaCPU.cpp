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
 * Contains the code for the emulated DecChip 21264CB EV68 Alpha processor.
 *
 * \bug Rounding and trap modes are not used for floating point ops.
 * \bug /V is ignored for integer ops.
 *
 * X-1.49       Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.48       Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.47       Camiel Vanderhoeven                             10-DEC-2007
 *      Use configurator.
 *
 * X-1.46       Camiel Vanderhoeven                             2-DEC-2007
 *      Changed the way translation buffers work, the way interrupts work. 
 *
 * X-1.45       Brian Wheeler                                   1-DEC-2007
 *      Added support for instruction counting, underlined lines in
 *      listings, corrected some unsigned/signed issues.
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
#include "cpu_debug.h"

//#include "Serial.h"
//#include "AliM1543C.h"

#if defined(IDB)

#define OP(mnemonic, format)							\
        PRE_##format(mnemonic);							\
	if (!bListing) {							\
	  DO_##mnemonic;							\
	}									\
	POST_##format;								\
        handle_debug_string(dbg_string);					\
	return 0;

#define OP_FNC(mnemonic, format)							\
        PRE_##format(mnemonic);							\
	if (!bListing) {							\
	  mnemonic();							\
	}									\
	POST_##format;								\
        handle_debug_string(dbg_string);					\
	return 0;

#else //defined(IDB)

#define OP(mnemonic, format)							\
	DO_##mnemonic;								\
	return 0;

#define OP_FNC(mnemonic, format)							\
    mnemonic();								\
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

CAlphaCPU::CAlphaCPU(CConfigurator * cfg, CSystem * system) : CSystemComponent (cfg,system)
{
  state.iProcNum = cSystem->RegisterCPU(this);
  cSystem = system;
  int i;

  cSystem->RegisterClock(this, false);

  state.pc = 0;
  state.bIntrFlag = false;

  for(i=0;i<64;i++) 
    {
      state.r[i] = 0;
      state.f[i] = 0;
    }

  flush_icache();

  tbia(ACCESS_READ);
  tbia(ACCESS_EXEC);

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
  state.pal_vms = false;
  state.check_int = false;

  // SROM imitation...
  add_tb(0, X64(ff61),ACCESS_READ);
//  add_tb(0, X64(ff61),ACCESS_EXEC);
//  state.r[32+22] = X64(1) << 0x3f;

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

#define DISP_12 (sext_64(ins,12))
#define DISP_13 (sext_64(ins,13))
#define DISP_16 (sext_64(ins,16))
#define DISP_21 (sext_64(ins,21))

#define DATA_PHYS(addr,flags) 				\
    if (virt2phys(addr, &phys_address, flags, NULL, ins)) \
      return 0

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

#ifdef IDB
  state.instruction_count++;
#endif

  if (DO_ACTION)
  {
    if (state.check_int && !(state.pc&1))
    {
      if (state.pal_vms)
      {
        if (state.eir & state.eien & 6)
          if (vmspal_ent_ext_int(state.eir&state.eien & 6))
            return 0;

        if (state.sir & state.sien & 0xfffc)
          if (vmspal_ent_sw_int(state.sir&state.sien))
            return 0;

        if (state.asten && (state.aster & state.astrr & ((1<<(state.cm+1))-1) ))
          if (vmspal_ent_ast_int(state.aster & state.astrr & ((1<<(state.cm+1))-1) ))
            return 0;

        if (state.sir & state.sien)
          if (vmspal_ent_sw_int(state.sir&state.sien))
            return 0;
      }
      else
      {
        if ((state.eien & state.eir) || 
            (state.sien & state.sir) || 
            (state.asten && (state.aster & state.astrr & ((1<<(state.cm+1))-1) )))
        {
          GO_PAL(INTERRUPT);
          return 0;
	    }
      }
      state.check_int = false;
    }

    // profile
#if defined(PROFILE)
    PROFILE_DO(state.pc);
#endif

    // get next instruction
    if (get_icache(state.pc,&ins))
  	  return 0;
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
#if 0
      if (state.pc>X64(600000))
	state.cc+=X64(1654321);
      else
#endif
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
        case 0x123401: OP_FNC(vmspal_int_read_ide, NOP);

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
  listing(from,to,0);
}

void CAlphaCPU::listing(u64 from, u64 to, u64 mark)
{
  printf("%%CPU-I-LISTNG: Listing from %016" LL "x to %016" LL "x\n",from,to);
  u64 iSavedPC;
  bool bSavedDebug;
  iSavedPC = state.pc;
  bSavedDebug = bDisassemble;
  bDisassemble = true;
  bListing = true;
  for(state.pc=from;state.pc<=to;) {
    DoClock();
    if(state.pc == mark) 
      printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
  }
  bListing = false;
  state.pc = iSavedPC;
  bDisassemble = bSavedDebug;
}
#endif

static u32 cpu_magic1 = 0x2126468C;
static u32 cpu_magic2 = 0xC8646212;

/**
 * Save state to a Virtual Machine State file.
 **/

int CAlphaCPU::SaveState(FILE *f)
{
  long ss = sizeof(state);

  fwrite(&cpu_magic1,sizeof(u32),1,f);
  fwrite(&ss,sizeof(long),1,f);
  fwrite(&state,sizeof(state),1,f);
  fwrite(&cpu_magic2,sizeof(u32),1,f);
  printf("%s: %d bytes saved.\n",devid_string,ss);
  return 0;
}

/**
 * Restore state from a Virtual Machine State file.
 **/

int CAlphaCPU::RestoreState(FILE *f)
{
  long ss;
  u32 m1;
  u32 m2;
  size_t r;

  r = fread(&m1,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (m1 != cpu_magic1)
  {
    printf("%s: MAGIC 1 does not match!\n",devid_string);
    return -1;
  }

  fread(&ss,sizeof(long),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (ss != sizeof(state))
  {
    printf("%s: STRUCT SIZE does not match!\n",devid_string);
    return -1;
  }

  fread(&state,sizeof(state),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }

  r = fread(&m2,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (m2 != cpu_magic2)
  {
    printf("%s: MAGIC 1 does not match!\n",devid_string);
    return -1;
  }

  printf("%s: %d bytes restored.\n",devid_string,ss);
  return 0;
}

int CAlphaCPU::FindTBEntry(u64 virt, int flags)
{
  int t = (flags&ACCESS_EXEC)?1:0;
  int i = state.last_found_tb[t];
  int asn = (flags & ACCESS_EXEC)?state.asn:state.asn0;

  if (     state.tb[t][i].valid
       && !((state.tb[t][i].virt ^ virt) & state.tb[t][i].match_mask)
       &&  (state.tb[t][i].asm_bit || (state.tb[t][i].asn == asn))
       )
    return i;

  for (i=0;i<TB_ENTRIES;i++)
    {
      if (     state.tb[t][i].valid
	       && !((state.tb[t][i].virt ^ virt) & state.tb[t][i].match_mask)
	       &&  (state.tb[t][i].asm_bit || (state.tb[t][i].asn == asn))
	       )
      {
        state.last_found_tb[t] = i;
	    return i;
      }
    }
  return -1;
}

int CAlphaCPU::virt2phys(u64 virt, u64 * phys, int flags,bool *asm_bit,u32 ins)
{
  int t = (flags&ACCESS_EXEC)?1:0;
  int i;
  int res;

  int spe = (flags & ACCESS_EXEC)?state.i_ctl_spe:state.m_ctl_spe;
  int asn = (flags & ACCESS_EXEC)?state.asn:state.asn0;
  int cm  = (flags & ALT)?state.alt_cm:state.cm;
  bool forreal = !(flags & FAKE);

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
    printf("TB %" LL "x,%x: ", virt,flags);
#endif

if (spe && !cm)
  {
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
          if (bTB_Debug)
#endif
    printf("try spe...");
#endif
    if ((move_bits_64(virt,47,46,0) == X64(2)) && (spe&4))
	{
	  *phys = keep_bits_64(virt,43,0);
      if (asm_bit)
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
    else if ((move_bits_64(virt,47,41,0) == X64(7e)) && (spe&2))
	{
	  *phys = keep_bits_64(virt,40,0) 
	        | extend_bit_64(virt,43,41,40);
      if (asm_bit)
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
      else if ((move_bits_64(virt,47,30,0) == X64(3fffe)) && (spe & 1))
	{
	  *phys = keep_bits_64(virt,29,0);
      if (asm_bit)
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

  i = FindTBEntry(virt,flags);

  if (i<0)
  {
    if (!forreal)
        return -1;
    if (!state.pal_vms)
    {
        state.exc_addr = state.current_pc;
        if (flags & VPTE)
        {
          state.fault_va = virt;
          state.exc_sum = (u64)REG_1<<8;
          state.pc = state.pal_base + 0x101;
        }
        else if (flags & ACCESS_EXEC)
        {
          state.pc = state.pal_base + 0x581;
        }
        else
        {
          state.fault_va = virt;
          state.exc_sum = (u64)REG_1<<8;
          u32 opcode = move_bits_32(ins,31,26,0);
          state.mm_stat =  ((opcode==0x1b||opcode==0x1f)?opcode-0x18:opcode)<<4 | (flags & ACCESS_WRITE);
          state.pc = state.pal_base + 0x301;
        }
        return -1;
    }
    else
    {
        if (flags & RECUR)
            return -1;

        state.exc_addr = state.current_pc;
        if (flags & VPTE)
        {
          if (res = vmspal_ent_dtbm_double_3(flags))
            return res;
          return virt2phys(virt,phys,flags | RECUR, asm_bit, ins);
        }
        else if (flags & ACCESS_EXEC)
        {
          if (res = vmspal_ent_itbm(flags))
            return res;
          return virt2phys(virt,phys,flags | RECUR, asm_bit, ins);
        }
        else
        {
          state.fault_va = virt;
          state.exc_sum = (u64)REG_1<<8;
          u32 opcode = move_bits_32(ins,31,26,0);
          state.mm_stat =  ((opcode==0x1b||opcode==0x1f)?opcode-0x18:opcode)<<4 | (flags & ACCESS_WRITE);
          if (res = vmspal_ent_dtbm_single(flags))
            return res;
          return virt2phys(virt,phys,flags | RECUR, asm_bit, ins);
        }
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
  if (!(flags&NO_CHECK))
  {
    if (!state.tb[t][i].access[flags&ACCESS_WRITE][cm])
    {
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
      if (bTB_Debug)
#endif
        printf("acv\n");
#endif
      if (flags & ACCESS_EXEC)
      {
        state.exc_addr = state.current_pc;
        state.exc_sum = 0;
        if (state.pal_vms)
        {
          if (res = vmspal_ent_iacv(flags))
            return res;
        }
        else
        {
          state.pc = state.pal_base + 0x481;
          return -1;
        }
      }
      else
      {
        state.exc_addr = state.current_pc;
        state.fault_va = virt;
        state.exc_sum = (u64)REG_1<<8;
        u32 opcode = move_bits_32(ins,31,26,0);
        state.mm_stat =  ((opcode==0x1b||opcode==0x1f)?opcode-0x18:opcode)<<4 | (flags & ACCESS_WRITE) | 2;
        if (state.pal_vms)
        {
          if (res = vmspal_ent_dfault(flags))
            return res;
        }
        else
        {
          state.pc = state.pal_base + 0x381;
          return -1;
        }
      }
    }
    if (state.tb[t][i].fault[flags&ACCESS_MODE])
    {
#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
      if (bTB_Debug)
#endif
        printf("fault\n");
#endif
      if (flags & ACCESS_EXEC)
      {
        state.exc_addr = state.current_pc;
        state.exc_sum = 0;
        if (state.pal_vms)
        {
          if (res = vmspal_ent_iacv(flags))
            return res;
        }
        else
        {
          state.pc = state.pal_base + 0x481;
          return -1;
        }
      }
      else
      {
        state.exc_addr = state.current_pc;
        state.fault_va = virt;
        state.exc_sum = (u64)REG_1<<8;
        u32 opcode = move_bits_32(ins,31,26,0);
        state.mm_stat =  ((opcode==0x1b||opcode==0x1f)?opcode-0x18:opcode)<<4 | (flags & ACCESS_WRITE) | ((flags&ACCESS_WRITE)?8:4);
        if (state.pal_vms)
        {
          if (res = vmspal_ent_dfault(flags))
            return res;
        }
        else
        {
          state.pc = state.pal_base + 0x381;
          return -1;
        }
      }
    }
  }
  // all is ok...


  *phys = state.tb[t][i].phys | (virt & state.tb[t][i].keep_mask);
  if (asm_bit)
    *asm_bit = state.tb[t][i].asm_bit?true:false;

#if defined(DEBUG_TB)
  if (forreal)
#if defined(IDB)
  if (bTB_Debug)
#endif
    printf("phys: %" LL "x - OK\n", *phys);
#endif
  
  return 0;
}

#ifdef IDB
u64 CAlphaCPU::get_instruction_count()
{
  return state.instruction_count;

}
#endif

void CAlphaCPU::add_tb(u64 virt, u64 pte, int flags)
{
  int t = (flags&ACCESS_EXEC)?1:0;
  u64 match_mask = 0;
  u64 keep_mask = 0;
  u64 phys_mask = 0;
  int i;
  int asn = (flags & ACCESS_EXEC)?state.asn:state.asn0;

  switch (pte & 0x60)
  {
	case 0:
	  match_mask = make_mask_64(42,13);
	  phys_mask  = make_mask_64(63,13);
      keep_mask  = make_mask_64(12,0);
	  break;
	case 0x20:
	  match_mask = make_mask_64(42,16);
	  phys_mask  = make_mask_64(63,16);
      keep_mask  = make_mask_64(15,0);
	  break;
	case 0x40:
	  match_mask = make_mask_64(42,19);
	  phys_mask  = make_mask_64(63,19);
      keep_mask  = make_mask_64(18,0);
	  break;
	case 0x60:
	  match_mask = make_mask_64(42,22);
	  phys_mask  = make_mask_64(63,22);
      keep_mask  = make_mask_64(21,0);
	  break;
  }
  i = FindTBEntry(virt,flags);

  if (i<0)
  {
      i = state.next_tb[t];
      state.next_tb[t]++;
      if (state.next_tb[t] == TB_ENTRIES)
        state.next_tb[t] = 0;
  }
  state.tb[t][i].match_mask = match_mask;
  state.tb[t][i].keep_mask = keep_mask;
  state.tb[t][i].virt = virt & match_mask;
  state.tb[t][i].phys = move_bits_64(pte,62,32,13) & phys_mask;
  state.tb[t][i].fault[0] = (int)pte & 2;
  state.tb[t][i].fault[1] = (int)pte & 4;
  state.tb[t][i].fault[2] = (int)pte & 8;
  state.tb[t][i].access[0][0] = (int)pte & 0x100;
  state.tb[t][i].access[1][0] = (int)pte & 0x1000;
  state.tb[t][i].access[0][1] = (int)pte & 0x200;
  state.tb[t][i].access[1][1] = (int)pte & 0x2000;
  state.tb[t][i].access[0][2] = (int)pte & 0x400;
  state.tb[t][i].access[1][2] = (int)pte & 0x4000;
  state.tb[t][i].access[0][3] = (int)pte & 0x800;
  state.tb[t][i].access[1][3] = (int)pte & 0x8000;
  state.tb[t][i].asm_bit = (int)pte & 0x10;
  state.tb[t][i].asn = asn;
  state.tb[t][i].valid = true;
  state.last_found_tb[t] = i;

#if defined(DEBUG_TB_)
#if defined(IDB)
  if (bTB_Debug)
#endif
  {
      printf("Add TB---------------------------------------\n");
      printf("Map VIRT    %016" LL "x\n",state.tb[i].virt);
      printf("Matching    %016" LL "x\n",state.tb[i].match_mask);
      printf("And keeping %016" LL "x\n",state.tb[i].keep_mask);
      printf("To PHYS     %016" LL "x\n",state.tb[i].phys);
      printf("Read : %c%c%c%c %c\n",state.tb[i].access[0][0]?'K':'-',
                                    state.tb[i].access[0][1]?'E':'-',
                                    state.tb[i].access[0][2]?'S':'-',
                                    state.tb[i].access[0][3]?'U':'-',
                                    state.tb[i].fault[0]?'F':'-');
      printf("Write: %c%c%c%c %c\n",state.tb[i].access[1][0]?'K':'-',
                                    state.tb[i].access[1][1]?'E':'-',
                                    state.tb[i].access[1][2]?'S':'-',
                                    state.tb[i].access[1][3]?'U':'-',
                                    state.tb[i].fault[1]?'F':'-');
      printf("Exec : %c%c%c%c %c\n",state.tb[i].access[1][0]?'K':'-',
                                    state.tb[i].access[1][1]?'E':'-',
                                    state.tb[i].access[1][2]?'S':'-',
                                    state.tb[i].access[1][3]?'U':'-',
                                    state.tb[i].fault[1]?'F':'-');
    }
#endif

}

void CAlphaCPU::add_tb_d(u64 virt, u64 pte)
{
  add_tb(virt,pte,ACCESS_READ);
}

void CAlphaCPU::add_tb_i(u64 virt, u64 pte)
{
  add_tb(virt, keep_bits_64(pte,12,0) | move_bits_64(pte,43,13,32), ACCESS_EXEC);
}

void CAlphaCPU::tbia(int flags)
{
  int t = (flags&ACCESS_EXEC)?1:0;
  int i;
  for (i=0; i<TB_ENTRIES; i++)
    state.tb[t][i].valid = false;
  state.last_found_tb[t] = 0;
  state.next_tb[t] = 0;
}

void CAlphaCPU::tbiap(int flags)
{
  int t = (flags&ACCESS_EXEC)?1:0;
  int i;
  for (i=0; i<TB_ENTRIES; i++)
    if (!state.tb[t][i].asm_bit)
      state.tb[t][i].valid = false;
}

void CAlphaCPU::tbis(u64 virt,int flags)
{
  int t = (flags&ACCESS_EXEC)?1:0;
  int i = FindTBEntry(virt,flags);
  if (i>=0)
    state.tb[t][i].valid = false;
}

#if defined(IDB)

char * PAL_NAME[] = {
  "HALT"		,"CFLUSH"   ,"DRAINA"		,"LDQP"			,"STQP"			,"SWPCTX"		,"MFPR_ASN"		,"MTPR_ASTEN"	,
  "MTPR_ASTSR","CSERVE"   ,"SWPPAL"		,"MFPR_FEN"		,"MTPR_FEN"		,"MTPR_IPIR"	,"MFPR_IPL"		,"MTPR_IPL"	,
  "MFPR_MCES"	,"MTPR_MCES","MFPR_PCBB"	,"MFPR_PRBR"	,"MTPR_PRBR"	,"MFPR_PTBR"	,"MFPR_SCBB"	,"MTPR_SCBB"	,
  "MTPR_SIRR" ,"MFPR_SISR","MFPR_TBCHK"	,"MTPR_TBIA"	,"MTPR_TBIAP"	,"MTPR_TBIS"	,"MFPR_ESP"		,"MTPR_ESP"	,
  "MFPR_SSP"	,"MTPR_SSP" ,"MFPR_USP"		,"MTPR_USP"		,"MTPR_TBISD"	,"MTPR_TBISI"	,"MFPR_ASTEN"	,"MFPR_ASTSR"	,
  "28"        ,"MFPR_VPTB","MTPR_VPTB"	,"MTPR_PERFMON"	,"2C"			,"2D"			,"MTPR_DATFX"	,"2F"			,
  "30"		,"31"		,"32"			,"33"			,"34"			,"35"			,"36"			,"37"			,
  "38"		,"39"		,"3A"			,"3B"			,"3C"			,"3D"			,"WTINT"		,"MFPR_WHAMI"	,
  "-"			,"-"		,"-"			,"-"			,"-"			,"-"			,"-"			,"-"			,"-","-","-","-","-","-","-","-",
  "-"			,"-"		,"-"			,"-"			,"-"			,"-"			,"-"			,"-"			,"-","-","-","-","-","-","-","-",
  "-"			,"-"		,"-"			,"-"			,"-"			,"-"			,"-"			,"-"			,"-","-","-","-","-","-","-","-",
  "-"			,"-"		,"-"			,"-"			,"-"			,"-"			,"-"			,"-"			,"-","-","-","-","-","-","-","-",
  "BPT"		,"BUGCHK"	,"CHME"			,"CHMK"			,"CHMS"			,"CHMU"			,"IMB"			,"INSQHIL"		,
  "INSQTIL"	,"INSQHIQ"	,"INSQTIQ"		,"INSQUEL"		,"INSQUEQ"		,"INSQUEL/D"	,"INSQUEQ/D"	,"PROBER"		,
  "PROBEW"	,"RD_PS"	,"REI"			,"REMQHIL"		,"REMQTIL"		,"REMQHIQ"		,"REMQTIQ"		,"REMQUEL"		,
  "REMQUEQ"	,"REMQUEL/D","REMQUEQ/D"	,"SWASTEN"		,"WR_PS_SW"		,"RSCC"			,"READ_UNQ"		,"WRITE_UNQ"	,
  "AMOVRR"	,"AMOVRM"	,"INSQHILR"		,"INSQTILR"		,"INSQHIQR"		,"INSQTIQR"		,"REMQHILR"		,"REMQTILR"		,
  "REMQHIQR"	,"REMQTIQR"	,"GENTRAP"		,"AB"			,"AC"			,"AD"			,"CLRFEN"		,"AF"			,
  "B0","B1","B2","B3","B4","B5","B6","B7","B8","B9","BA","BB","BC","BD","BE","BF"};

char * IPR_NAME[] = {
  "ITB_TAG",	"ITB_PTE",	"ITB_IAP",	"ITB_IA",	"ITB_IS",	"PMPC",		"EXC_ADDR",	"IVA_FORM",
  "IER_CM",	"CM",		"IER",		"IER_CM",	"SIRR",		"ISUM",		"HW_INT_CLR",	"EXC_SUM",
  "PAL_BASE",	"I_CTL",	"IC_FLUSH_ASM",	"IC_FLUSH",	"PCTR_CTL",	"CLR_MAP",	"I_STAT",	"SLEEP",
  "?0001.1000?","?0001.1001?",	"?0001.1010?",	"?0001.1011?",	"?0001.1100?",	"?0001.1101?",	"?0001.1110?",	"?0001.1111?",
  "DTB_TAG0",	"DTB_PTE0",	"?0010.0010?",	"?0010.0011?",	"DTB_IS0",	"DTB_ASN0",	"DTB_ALTMODE",	"MM_STAT",
  "M_CTL",	"DC_CTL",	"DC_STAT",	"C_DATA",	"C_SHFT",	"M_FIX",	"?0010.1110?",	"?0010.1111?",
  "?0011.0000?","?0011.0001?",	"?0011.0010?",	"?0011.0011?",	"?0011.0100?",	"?0010.0101?",	"?0010.0110?",	"?0010.0111?",
  "?0011.1000?","?0011.1001?",	"?0011.1010?",	"?0011.1011?",	"?0011.1100?",	"?0010.1101?",	"?0010.1110?",	"?0010.1111?",
  "PCTX.00000",	"PCTX.00001",	"PCTX.00010",	"PCTX.00011",	"PCTX.00100",	"PCTX.00101",	"PCTX.00110",	"PCTX.00111",
  "PCTX.01000",	"PCTX.01001",	"PCTX.01010",	"PCTX.01011",	"PCTX.01100",	"PCTX.01101",	"PCTX.01110",	"PCTX.01111",
  "PCTX.10000",	"PCTX.10001",	"PCTX.10010",	"PCTX.10011",	"PCTX.10100",	"PCTX.10101",	"PCTX.10110",	"PCTX.10111",
  "PCTX.11000",	"PCTX.11001",	"PCTX.11010",	"PCTX.11011",	"PCTX.11100",	"PCTX.11101",	"PCTX.11110",	"PCTX.11111",
  "PCTX.00000",	"PCTX.00001",	"PCTX.00010",	"PCTX.00011",	"PCTX.00100",	"PCTX.00101",	"PCTX.00110",	"PCTX.00111",
  "PCTX.01000",	"PCTX.01001",	"PCTX.01010",	"PCTX.01011",	"PCTX.01100",	"PCTX.01101",	"PCTX.01110",	"PCTX.01111",
  "PCTX.10000",	"PCTX.10001",	"PCTX.10010",	"PCTX.10011",	"PCTX.10100",	"PCTX.10101",	"PCTX.10110",	"PCTX.10111",
  "PCTX.11000",	"PCTX.11001",	"PCTX.11010",	"PCTX.11011",	"PCTX.11100",	"PCTX.11101",	"PCTX.11110",	"PCTX.11111",
  "?1000.0000?","?1000.0001?",	"?1000.0010?",	"?1000.0011?",	"?1000.0100?",	"?1000.0101?",	"?1000.0110?",	"?1000.0111?",
  "?1000.1000?","?1000.1001?",	"?1000.1010?",	"?1000.1011?",	"?1000.1100?",	"?1000.1101?",	"?1000.1110?",	"?1000.1111?",
  "?1001.0000?","?1001.0001?",	"?1001.0010?",	"?1001.0011?",	"?1001.0100?",	"?1001.0101?",	"?1001.0110?",	"?1001.0111?",
  "?1001.1000?","?1001.1001?",	"?1001.1010?",	"?1001.1011?",	"?1001.1100?",	"?1001.1101?",	"?1001.1110?",	"?1001.1111?",
  "DTB_TAG1",	"DTB_PTE1",	"DTB_IAP",	"DTB_IA",	"DTB_IS1",	"DTB_ASN1",	"?1010.0110?",	"?1010.0111?",
  "?1010.1000?","?1010.1001?",	"?1010.1010?",	"?1010.1011?",	"?1010.1100?",	"?1010.1101?",	"?1010.1110?",	"?1010.1111?",
  "?1011.0000?","?1011.0001?",	"?1011.0010?",	"?1011.0011?",	"?1011.0100?",	"?1011.0101?",	"?1011.0110?",	"?1011.0111?",
  "?1011.1000?","?1011.1001?",	"?1011.1010?",	"?1011.1011?",	"?1011.1100?",	"?1011.1101?",	"?1011.1110?",	"?1011.1111?",
  "CC",		"CC_CTL",	"VA",		"VA_FORM",	"VA_CTL",	"?1100.0101?",	"?1100.0110?",	"?1100.0111?",
  "?1100.1000?","?1100.1001?",	"?1100.1010?",	"?1100.1011?",	"?1100.1100?",	"?1100.1101?",	"?1100.1110?",	"?1100.1111?",
  "?1101.0000?","?1101.0001?",	"?1101.0010?",	"?1101.0011?",	"?1101.0100?",	"?1101.0101?",	"?1101.0110?",	"?1101.0111?",
  "?1101.1000?","?1101.1001?",	"?1101.1010?",	"?1101.1011?",	"?1101.1100?",	"?1101.1101?",	"?1101.1110?",	"?1101.1111?",
  "?1110.0000?","?1110.0001?",	"?1110.0010?",	"?1110.0011?",	"?1110.0100?",	"?1110.0101?",	"?1110.0110?",	"?1110.0111?",
  "?1110.1000?","?1110.1001?",	"?1110.1010?",	"?1110.1011?",	"?1110.1100?",	"?1110.1101?",	"?1110.1110?",	"?1110.1111?",
  "?1111.0000?","?1111.0001?",	"?1111.0010?",	"?1111.0011?",	"?1111.0100?",	"?1111.0101?",	"?1111.0110?",	"?1111.0111?",
  "?1111.1000?","?1111.1001?",	"?1111.1010?",	"?1111.1011?",	"?1111.1100?",	"?1111.1101?",	"?1111.1110?",	"?1111.1111?",
};

#endif
