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
  iProcNum = cSystem->RegisterCPU(this);
  cSystem = system;
  int i;

  cSystem->RegisterClock(this, false);

  pc = 0;
  bIntrFlag = false;

  for(i=0;i<64;i++) 
    {
      r[i] = 0;
      f[i] = 0;
    }

  flush_icache();


  alt_cm = 0;
  asn = 0;
  asn0 = 0;
  asn1 = 0;
  asten = 0;
  aster = 0;
  astrr = 0;
  cc = 0;
  cc_ena = false;
  cc_offset = 0;
  cm = 0;
  cren = 0;
  crr = 0;
  current_pc = 0;
  dc_ctl = 3;
  dc_ctl = 0;
  dc_stat = 0;
  eien = 0;
  eir = 0;
  exc_addr = 0;
  exc_sum = 0;
  fault_va = 0;
  fpcr = X64(8ff0000000000000);
  fpen = true;
  hwe = false;
  i_ctl_other = X64(502086);
  i_ctl_va_mode = 0;
  i_ctl_vptb = 0;
  i_stat = 0;
  mm_stat = 0;
  pal_base = 0;
  pc = 0;
  pcen = 0;
  pcr = 0;
  pctr_ctl = 0;
  pmpc = 0;
  ppcen = false;
  sde = false;
  sien = 0;
  sir = 0;
  slen = 0;
  slr = 0;
  smc = 1;
  m_ctl_spe = 0;
  i_ctl_spe = 0;
  va_ctl_va_mode = 0;
  va_ctl_vptb = 0;

  itb = new CTranslationBuffer (this, true);
  dtb = new CTranslationBuffer (this, false);

  // SROM imitation...

  dtb->write_tag(0,0);
  dtb->write_pte(0,X64(ff61));

#if defined(IDB)
  bListing = false;
#endif
  
  printf("%%CPU-I-INIT: Alpha AXP 21264 EV68 processor %d initialized.\n", iProcNum);
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
    dp_result = dtb->convert_address(addr, &phys_address, access, check, alt?alt_cm:cm, &temp_bool, false, true); \
    if (dp_result) {							\
      fault_va = addr;							\
      switch (dp_result) {						\
      case E_NOT_FOUND:							\
        if (vpte) {							\
	  exc_sum = REG_3<<8;						\
	  GO_PAL(DTBM_DOUBLE_3);					\
	} else {							\
          mm_stat = (((opcode==0x1b || opcode==0x1f)?opcode-0x18:opcode)<<4) |	\
		    (access);						\
	  exc_sum = REG_3<<8;						\
	  GO_PAL(DTBM_SINGLE);						\
	}								\
	break;								\
      case E_ACCESS:							\
        if (!vpte)							\
	  mm_stat = (((opcode==0x1b || opcode==0x1f)?opcode-0x18:opcode)<<4) |	\
		    (access) | 2;					\
	exc_sum = REG_3<<8;						\
	GO_PAL(DFAULT);							\
	break;								\
      case E_FAULT:							\
       if (!vpte)							\
	  mm_stat = (((opcode==0x1b || opcode==0x1f)?opcode-0x18:opcode)<<4) |	\
	  (access?9:4);							\
	exc_sum = REG_3<<8;						\
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

#define V_2 ( (ins&0x1000)?((ins>>13)&0xff):r[REG_2] )

#if defined(IDB)

void handle_debug_string(char * s)
{
  if (*s)
  {
#if defined(LS_SLAVE) || defined(LS_MASTER)
    lockstep_compare(s);
#else
    printf("%s\n",s);
#endif
  }	
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
  char * dbg_strptr = dbg_string;
#endif

   current_pc = pc;

   if (DO_ACTION)
    {
      // check for interrupts
      if ((!(pc&X64(1))) && ((eien & eir) || (sien & sir) || (asten && (aster & astrr & ((1<<(cm+1))-1) ))))
	{
	  GO_PAL(INTERRUPT);
	  return 0;
	}

      // get next instruction
      result = get_icache(pc,&ins);
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
      ins = (u32)(cSystem->ReadMem(pc,32));
    }
    pc += 4;

  r[31] = 0;
  f[31] = 0;

  if (cc_ena)
  {
    if (DO_ACTION)
    {
      if (pc>X64(600000))
	cc+=X64(1654321);
      else
	cc+=83;
    }
  }

  opcode = ins >>26;

  switch (opcode)
    {
    case 0x00: // CALL_PAL
      function = ins&0x1fffffff;
      switch (function)
      {
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
      switch (function)
      {
        case 0x00: OP(ADDL,R12_R3);
        case 0x02: OP(S4ADDL,R12_R3);
        case 0x09: OP(SUBL,R12_R3);
        case 0x0b: OP(S4SUBL,R12_R3);
        case 0x0f: OP(CMPBGE,R12_R3);
        case 0x12: OP(S8ADDL,R12_R3);
        case 0x1b: OP(S8SUBL,R12_R3);
        case 0x1d: OP(CMPULT,R12_R3);
        case 0x20: OP(ADDQ,R12_R3);
        case 0x22: OP(S4ADDQ,R12_R3);
        case 0x29: OP(SUBQ,R12_R3);
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
        case 0x32: OP(MSKQL,R12_R3);
        case 0x36: OP(EXTQL,R12_R3);
        case 0x3b: OP(INSQL,R12_R3);
        case 0x30: OP(ZAP,R12_R3);
        case 0x31: OP(ZAPNOT,R12_R3);
        case 0x34: OP(SRL,R12_R3);
        case 0x39: OP(SLL,R12_R3);
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
      switch (function)
      {
	case 0x00: OP(MULL,R12_R3);
	case 0x20: OP(MULQ,R12_R3);
	case 0x30: OP(UMULH,R12_R3);
	default:   UNKNOWN2;
      }

    case 0x14:
      function = (ins>>5) & 0x7ff;
      switch(function)
      {
      case 0x24: OP(ITOFT,R1_F3);
      default:   UNKNOWN2;
      }

    case 0x15:
      function = (ins>>5) & 0x7ff;
      switch(function)
      {
      case 0xa0: OP(ADDG,F12_F3);
      case 0xa3: OP(DIVG,F12_F3);
      case 0xa5: OP(CMPGEQ,F12_F3);
      case 0xa6: OP(CMPGLT,F12_F3);
      case 0xa7: OP(CMPGLE,F12_F3);
      case 0xaf: OP(CVTGQ,F2_F3);
      case 0xbe: OP(CVTQG,F2_F3);
      default:   UNKNOWN2;
      }
    case 0x16:
      function = (ins>>5) & 0x7ff;
      switch(function)
      {
      case 0xa0: OP(ADDT,F12_F3);
      case 0xa3: OP(DIVT,F12_F3);
      case 0xa4: OP(CMPTUN,F12_F3);
      case 0xa5: OP(CMPTEQ,F12_F3);
      case 0xa6: OP(CMPTLT,F12_F3);
      case 0xa7: OP(CMPTLE,F12_F3);
      case 0xaf: OP(CVTTQ,F2_F3);
      case 0xbe: OP(CVTQT,F2_F3);
      default:   UNKNOWN2;
      }
    case 0x17:
      function = (ins>>5) & 0x7ff;
      switch (function)
      {
      case 0x20: OP(CPYS,F12_F3);
      case 0x21: OP(CPYSN,F12_F3);
      case 0x22: OP(CPYSE,F12_F3);
      case 0x24: OP(MT_FPCR,X_F1);
      case 0x25: OP(MF_FPCR,X_F1);
      case 0x2a: OP(FCMOVEQ,F12_F3);
      case 0x2b: OP(FCMOVNE,F12_F3);
      case 0x2c: OP(FCMOVLT,F12_F3);
      case 0x2d: OP(FCMOVGE,F12_F3);
      case 0x2e: OP(FCMOVLE,F12_F3);
      case 0x2f: OP(FCMOVGT,F12_F3);
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
	case 0xE800: OP(ECB,NOP);
	case 0xF800: OP(WH64,NOP);
	case 0xFC00: OP(WH64EN,NOP);
	case 0xC000: OP(RPCC,X_R1);
	case 0xE000: OP(RC,X_R1);
	case 0xF000: OP(RS,X_R1);
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

    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
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
  iSavedPC = pc;
  bSavedDebug = bDisassemble;
  bDisassemble = true;
  bListing = true;
  for(pc=from;pc<=to;DoClock());
  bListing = false;
  pc = iSavedPC;
  bDisassemble = bSavedDebug;
}
#endif

/**
 * Save state to a Virtual Machine State file.
 **/

void CAlphaCPU::SaveState(FILE *pF)
{
  fwrite(r,1,64*8,pF);
  fwrite(this->f,1,32*8,pF);
  fwrite(&pal_base,1,8,pF);
  fwrite(&pc,1,8,pF);
  fwrite(&dc_stat,1,8,pF);
  fwrite(&i_stat,1,8,pF);
  fwrite(&pctr_ctl,1,8,pF);
  fwrite(&dc_ctl,1,8,pF);
  fwrite(&fault_va,1,8,pF);
  fwrite(&exc_sum,1,8,pF);
  fwrite(&i_ctl_vptb,1,8,pF);
  fwrite(&va_ctl_vptb,1,8,pF);
  fwrite(&i_ctl_other,1,8,pF);
  fwrite(&mm_stat,1,8,pF);
  fwrite(&exc_addr,1,8,pF);
  fwrite(&pmpc,1,8,pF);
  fwrite(&fpcr,1,8,pF);

  fwrite(&cc,1,4,pF);
  fwrite(&cc_offset,1,4,pF);

  fwrite(&ppcen,1,sizeof(bool),pF);
  fwrite(&cc_ena,1,sizeof(bool),pF);
  fwrite(&fpen,1,sizeof(bool),pF);
  fwrite(&sde,1,sizeof(bool),pF);
  fwrite(&ppcen,1,sizeof(bool),pF);
  fwrite(&hwe,1,sizeof(bool),pF);
  fwrite(&bIntrFlag,1,sizeof(bool),pF);

  //	u64 current_pc;

  fwrite(&alt_cm,1,sizeof(int),pF);
  fwrite(&smc,1,sizeof(int),pF);
  fwrite(&i_ctl_va_mode,1,sizeof(int),pF);
  fwrite(&va_ctl_va_mode,1,sizeof(int),pF);
  fwrite(&cm,1,sizeof(int),pF);
  fwrite(&asn,1,sizeof(int),pF);
  fwrite(&asn0,1,sizeof(int),pF);
  fwrite(&asn1,1,sizeof(int),pF);
  fwrite(&eien,1,sizeof(int),pF);
  fwrite(&slen,1,sizeof(int),pF);
  fwrite(&cren,1,sizeof(int),pF);
  fwrite(&pcen,1,sizeof(int),pF);
  fwrite(&sien,1,sizeof(int),pF);
  fwrite(&asten,1,sizeof(int),pF);
  fwrite(&sir,1,sizeof(int),pF);
  fwrite(&eir,1,sizeof(int),pF);
  fwrite(&slr,1,sizeof(int),pF);
  fwrite(&crr,1,sizeof(int),pF);
  fwrite(&pcr,1,sizeof(int),pF);
  fwrite(&astrr,1,sizeof(int),pF);
  fwrite(&aster,1,sizeof(int),pF);
  fwrite(&m_ctl_spe,1,sizeof(int),pF);
  fwrite(&i_ctl_spe,1,sizeof(int),pF);

  fwrite(&next_icache,1,sizeof(int),pF);
  fwrite(icache,1,1024*sizeof(struct SICache),pF);

  itb->SaveState(pF);
  dtb->SaveState(pF);
}

/**
 * Restore state from a Virtual Machine State file.
 **/

void CAlphaCPU::RestoreState(FILE *pF)
{
  fread(r,1,64*8,pF);
  fread(this->f,1,32*8,pF);

  fread(&pal_base,1,8,pF);
  fread(&pc,1,8,pF);
  fread(&dc_stat,1,8,pF);
  fread(&i_stat,1,8,pF);
  fread(&pctr_ctl,1,8,pF);
  fread(&dc_ctl,1,8,pF);
  fread(&fault_va,1,8,pF);
  fread(&exc_sum,1,8,pF);
  fread(&i_ctl_vptb,1,8,pF);
  fread(&va_ctl_vptb,1,8,pF);
  fread(&i_ctl_other,1,8,pF);
  fread(&mm_stat,1,8,pF);
  fread(&exc_addr,1,8,pF);
  fread(&pmpc,1,8,pF);
  fread(&fpcr,1,8,pF);

  fread(&cc,1,4,pF);
  fread(&cc_offset,1,4,pF);

  fread(&ppcen,1,sizeof(bool),pF);
  fread(&cc_ena,1,sizeof(bool),pF);
  fread(&fpen,1,sizeof(bool),pF);
  fread(&sde,1,sizeof(bool),pF);
  fread(&ppcen,1,sizeof(bool),pF);
  fread(&hwe,1,sizeof(bool),pF);
  fread(&bIntrFlag,1,sizeof(bool),pF);

  //	u64 current_pc;

  fread(&alt_cm,1,sizeof(int),pF);
  fread(&smc,1,sizeof(int),pF);
  fread(&i_ctl_va_mode,1,sizeof(int),pF);
  fread(&va_ctl_va_mode,1,sizeof(int),pF);
  fread(&cm,1,sizeof(int),pF);
  fread(&asn,1,sizeof(int),pF);
  fread(&asn0,1,sizeof(int),pF);
  fread(&asn1,1,sizeof(int),pF);
  fread(&eien,1,sizeof(int),pF);
  fread(&slen,1,sizeof(int),pF);
  fread(&cren,1,sizeof(int),pF);
  fread(&pcen,1,sizeof(int),pF);
  fread(&sien,1,sizeof(int),pF);
  fread(&asten,1,sizeof(int),pF);
  fread(&sir,1,sizeof(int),pF);
  fread(&eir,1,sizeof(int),pF);
  fread(&slr,1,sizeof(int),pF);
  fread(&crr,1,sizeof(int),pF);
  fread(&pcr,1,sizeof(int),pF);
  fread(&astrr,1,sizeof(int),pF);
  fread(&aster,1,sizeof(int),pF);
  fread(&m_ctl_spe,1,sizeof(int),pF);
  fread(&i_ctl_spe,1,sizeof(int),pF);

  fread(&next_icache,1,sizeof(int),pF);
  fread(icache,1,1024*sizeof(struct SICache),pF);

  itb->RestoreState(pF);
  dtb->RestoreState(pF);
}

