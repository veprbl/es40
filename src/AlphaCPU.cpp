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
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
 **/

#include "StdAfx.h"
#include "AlphaCPU.h"
#include "TraceEngine.h"
#include "cpu_debug.h"

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

  // SROM imitatie...

  dtb->write_tag(0,0);
  dtb->write_pte(0,X64(ff61),get_asn());

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
    dp_result = dtb->convert_address(addr, &phys_address, access, false /*check*/, alt?get_altcm():get_cm(),get_asn(),get_d_spe()); \
    if (dp_result) {							\
      fault_va = addr;							\
      switch (dp_result) {						\
      case E_NOT_FOUND:							\
	if (vpte)							\
	  GO_PAL(DTBM_DOUBLE_3)						\
	  else								\
	    GO_PAL(DTBM_SINGLE)						\
	      break;							\
      case E_ACCESS:							\
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

  int opcode;
  int function;

#if defined(IDB)
  if (!bListing)
  {
#endif
      // known speedups
      if  (     get_clean_pc()==X64(14248) 
	     || get_clean_pc()==X64(14288)
	     || get_clean_pc()==X64(142c8)
	     || get_clean_pc()==X64(68320)
	     || get_clean_pc()==X64(8bb78)	// write in memory test (aa)
	     || get_clean_pc()==X64(8bc0c)	// write in memory test (bb)
	     || get_clean_pc()==X64(8bc94)	// write in memory test (00)
	     )
	next_pc();
#if defined(IDB)
  }
#endif

   current_pc = pc;

   if (DO_ACTION)
    {
      // check for interrupts
      if ((!(pc&X64(1))) && (eien & eir))
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
      if (   (   (function < 0x40) 
		 && ((cm != 0)

		 && DO_ACTION

		 ))
	     || (   (function > 0x3f)
    		    && (function < 0x80))
	     || (function > 0xbf))
	{
	  UNKNOWN2
	  if (DO_ACTION)
	    {

	      GO_PAL(OPCDEC);

	    }
	  return 0;
	}
      if (DO_ACTION)
	{
	  if (function == 0x92) // REI
		  lock_flag = false;
	  r[32+23] = pc;
	  pc = pal_base
	    | (X64(1)<<13 )
	    | (((u64)(function & 0x80)) <<5 )
	    | (((u64)(function & 0x3f)) << 6 )
	    | X64(1);

	}
      DEBUG_PAL;
      if (DO_ACTION)
	{
	  TRC(true,false);
	}
      return 0;

    case 0x08: // LDA
      if (DO_ACTION)
        r[REG_1] = r[REG_2] + DISP_16;
      DEBUG_LD_ST("LDA");
      return 0;

    case 0x09: // LDAH
      if (DO_ACTION)
        r[REG_1] = r[REG_2] + (DISP_16<<16);
      DEBUG_LD_ST("LDAH");
      return 0;

    case 0x0a: // LDBU
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);
	  r[REG_1] = READ_PHYS(8);
	}
      DEBUG_LD_ST("LDBU");
      return 0;

    case 0x0b: // LDQ_U
      if (DO_ACTION)
	{
	  DATA_PHYS((r[REG_2] + DISP_16)& ~X64(7), ACCESS_READ, true, false, false);
	  r[REG_1] = READ_PHYS(64);
	}
      DEBUG_LD_ST("LDQ_U");
      return 0;

    case 0x0c: // LDWU
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);
	  r[REG_1] = READ_PHYS(16);
	}
      DEBUG_LD_ST("LDWU");
      return 0;

    case 0x0d: // STW
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);
	  WRITE_PHYS(r[REG_1],16);
	}
      DEBUG_LD_ST("STW");
      return 0;

    case 0x0e: // STB
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);
	  WRITE_PHYS(r[REG_1],8);
	}
      DEBUG_LD_ST("STB");
      return 0;

    case 0x0f: // STQ_U
      if (DO_ACTION)
	{
	  DATA_PHYS((r[REG_2] + DISP_16)& ~X64(7), ACCESS_WRITE, true, false, false);
	  WRITE_PHYS(r[REG_1],64);
	}
      DEBUG_LD_ST("STQ_U");
      return 0;

    case 0x10: // op
      function = (ins>>5) & 0x7f;
      switch (function)
        {
        case 0x00: // ADDL
	  if (DO_ACTION)
 	    r[REG_3] = SEXT(r[REG_1] + V_2,32);
	  DEBUG_OP("ADDL");
	  return 0;
        case 0x02: // S4ADDL
	  if (DO_ACTION)
	    r[REG_3] = SEXT((r[REG_1]*4) + V_2,32);
	  DEBUG_OP("S4ADDL");
	  return 0;
        case 0x09: // SUBL
	  if (DO_ACTION)
	    r[REG_3] = SEXT(r[REG_1] - V_2,32);
	  DEBUG_OP("SUBL");
	  return 0;
        case 0x0b: // S4SUBL
	  if (DO_ACTION)
	    r[REG_3] = SEXT((r[REG_1]*4) - V_2,32);
	  DEBUG_OP("S4SUBL");
	  return 0;
        case 0x0f:  // CMPBGE
	  if (DO_ACTION)
 	    r[REG_3] =   (((u8)( r[REG_1]     &0xff)>=(u8)( V_2      & 0xff))?  1:0)
	      | (((u8)((r[REG_1]>> 8)&0xff)>=(u8)((V_2>> 8) & 0xff))?  2:0)
	      | (((u8)((r[REG_1]>>16)&0xff)>=(u8)((V_2>>16) & 0xff))?  4:0)
	      | (((u8)((r[REG_1]>>24)&0xff)>=(u8)((V_2>>24) & 0xff))?  8:0)
	      | (((u8)((r[REG_1]>>32)&0xff)>=(u8)((V_2>>32) & 0xff))? 16:0)
	      | (((u8)((r[REG_1]>>40)&0xff)>=(u8)((V_2>>40) & 0xff))? 32:0)
	      | (((u8)((r[REG_1]>>48)&0xff)>=(u8)((V_2>>48) & 0xff))? 64:0)
	      | (((u8)((r[REG_1]>>56)&0xff)>=(u8)((V_2>>56) & 0xff))?128:0);
	  DEBUG_OP("CMPBGE");
	  return 0;
        case 0x12: // S8ADDL
	  if (DO_ACTION)
	    r[REG_3] = SEXT((r[REG_1]*8) + V_2,32);
	  DEBUG_OP("S8ADDL");
	  return 0;
        case 0x1b: // S8SUBL
	  if (DO_ACTION)
 	    r[REG_3] = SEXT((r[REG_1]*8) - V_2,32);
	  DEBUG_OP("S8SUBL");
	  return 0;
        case 0x1d: // CMPULT
	  if (DO_ACTION)
 	    r[REG_3] = ((u64)r[REG_1]<(u64)V_2)?1:0;
	  DEBUG_OP("CMPULT");
	  return 0;
        case 0x20: // ADDQ
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] + V_2;
	  DEBUG_OP("ADDQ");
	  return 0;
        case 0x22: // S4ADDQ
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1]*4) + V_2;
	  DEBUG_OP("S4ADDQ");
	  return 0;
        case 0x29: // SUBQ
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] - V_2;
	  DEBUG_OP("SUBQ");
	  return 0;
        case 0x2b: // S4SUBQ
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1]*4) - V_2;
	  DEBUG_OP("S4SUBQ");
	  return 0;
        case 0x2d: // CMPEQ
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1]==V_2)?1:0;
	  DEBUG_OP("CMPEQ");
	  return 0;
        case 0x32: // S8ADDQ
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1]*8) + V_2;
	  DEBUG_OP("S8ADDQ");
	  return 0;
        case 0x3b: // S8SUBQ
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1]*8) - V_2;
	  DEBUG_OP("S8SUBQ");
	  return 0;
        case 0x3d: // CMPULE
	  if (DO_ACTION)
 	    r[REG_3] = ((u64)r[REG_1]<=(u64)V_2)?1:0;
	  DEBUG_OP("CMPULE");
	  return 0;
        case 0x4d: // CMPLT
	  if (DO_ACTION)
 	    r[REG_3] = ((s64)r[REG_1]<(s64)V_2)?1:0;
	  DEBUG_OP("CMPLT");
	  return 0;
        case 0x6d: // CMPLE
	  if (DO_ACTION)
 	    r[REG_3] = ((s64)r[REG_1]<=(s64)V_2)?1:0;
	  DEBUG_OP("CMPLE");
	  return 0;
        default:
	  UNKNOWN2;
	  return 0;
        }
      break;

    case 0x11: // op
      function = (ins>>5) & 0x7f;
      switch (function)
        {
        case 0x00: // AND
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] & V_2;
	  DEBUG_OP("AND");
	  return 0;
        case 0x08: // BIC
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] & ~V_2;
	  DEBUG_OP("BIC");
	  return 0;
        case 0x14: // CMOVLBS
	  if (DO_ACTION)
	  {
 	    if (r[REG_1] & X64(1))
	      r[REG_3] = V_2;
	  }
	  DEBUG_OP("CMOVLBS");
	  return 0;
        case 0x16: // CMOVLBC
	  if (DO_ACTION)
	  {
            if (!(r[REG_1] & X64(1)))
	      r[REG_3] = V_2;
	  }
	  DEBUG_OP("CMOVLBC");
	  return 0;
        case 0x20: // BIS
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] | V_2;
	  DEBUG_OP("BIS");
	  return 0;
        case 0x24: // CMOVEQ
	  if (DO_ACTION)
	  {
	    if (!r[REG_1])
	      r[REG_3] = V_2;
	  }
	  DEBUG_OP("CMOVEQ");
	  return 0;
        case 0x26: // CMOVNE
	  if (DO_ACTION)
	  {
 	    if (r[REG_1])
	      r[REG_3] = V_2;
	  }
	  DEBUG_OP("CMOVNE");
	  return 0;
        case 0x28: // ORNOT
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] | ~V_2;
	  DEBUG_OP("ORNOT");
	  return 0;
        case 0x40: // XOR
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] ^ V_2;
	  DEBUG_OP("XOR");
	  return 0;
        case 0x44: // CMOVLT
	  if (DO_ACTION)
	  {
	    if ((s64)r[REG_1]<0)
	    r[REG_3] = V_2;
	  }
	  DEBUG_OP("CMOVLT");
	  return 0;
        case 0x46: // CMOVGE
	  if (DO_ACTION)
	  {
 	    if ((s64)r[REG_1]>=0)
	      r[REG_3] = V_2;
	  }
	  DEBUG_OP("CMOVGE");
	  return 0;
        case 0x48: // EQV
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] ^ ~V_2;
	  DEBUG_OP("EQV");
	  return 0;
        case 0x61: // AMASK
	  if (DO_ACTION)
 	    r[REG_3] = V_2 & ~CPU_AMASK; // BWX,CIX,MVI,trapPC,prefMod 
	  DEBUG_OP_R23("AMASK");
	  return 0;
        case 0x64: // CMOVLE
	  if (DO_ACTION)
	  {
 	    if ((s64)r[REG_1]<=0)
	      r[REG_3] = V_2;
	  }
	  DEBUG_OP("CMOVLE");
	  return 0;
        case 0x66: // CMOVGT
	  if (DO_ACTION)
	  {
 	    if ((s64)r[REG_1]>0)
	      r[REG_3] = V_2;
	  }
	  DEBUG_OP("CMOVGT");
	  return 0;
        case 0x6c: // IMPLVER
	  if (DO_ACTION)
 	    r[REG_3] = CPU_IMPLVER;
	  DEBUG_OP_R3("IMPLVER");
	  return 0;
        default:
	  UNKNOWN2;
	  return 0;
        }

    case 0x12:
      function = (ins>>5) & 0x7f;
      switch (function)
        {
        case 0x02: //MSKBL
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] & ~(X64_BYTE<<((V_2&7)*8));
	  DEBUG_OP("MSKBL");
	  return 0;
        case 0x06: // EXTBL
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1] >> ((V_2&7)*8)) & X64_BYTE;
	  DEBUG_OP("EXTBL");
	  return 0;
        case 0x0b: // INSBL
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1] & X64_BYTE) << ((V_2&7)*8);
	  DEBUG_OP("INSBL");
	  return 0;
        case 0x12: // MSKWL
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] & ~(X64_WORD<<((V_2&7)*8));
	  DEBUG_OP("MSKWL");
	  return 0;
        case 0x16: // EXTWL
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1] >> ((V_2&7)*8))&X64_WORD;
	  DEBUG_OP("EXTWL");
	  return 0;
        case 0x1b: // INSWL
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1]&X64_WORD) << ((V_2&7)*8);
	  DEBUG_OP("INSWL");
	  return 0;
        case 0x22: // MSKLL
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] & ~(X64_LONG<<((V_2&7)*8));
	  DEBUG_OP("MSKLL");
	  return 0;
        case 0x26: // EXTLL
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1] >> ((V_2&7)*8))&X64_LONG;
	  DEBUG_OP("EXTLL");
	  return 0;
        case 0x2b: // INSLL
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1]&X64_LONG) << ((V_2&7)*8);
	  DEBUG_OP("INSLL");
	  return 0;
        case 0x32: // MSKQL
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] & ~(X64_QUAD<<((V_2&7)*8));
	  DEBUG_OP("MSKQL");
	  return 0;
        case 0x36: // EXTQL
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] >> ((V_2&7)*8);
	  DEBUG_OP("EXTQL");
	  return 0;
        case 0x3b: // INSQL
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] << ((V_2&7)*8);
	  DEBUG_OP("INSQL");
	  return 0;
        case 0x30: // ZAP
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] & (  ((V_2&  1)?0:              X64(ff))
				 | ((V_2&  2)?0:            X64(ff00))
				 | ((V_2&  4)?0:          X64(ff0000))
				 | ((V_2&  8)?0:        X64(ff000000))
				 | ((V_2& 16)?0:      X64(ff00000000))
				 | ((V_2& 32)?0:    X64(ff0000000000))
				 | ((V_2& 64)?0:  X64(ff000000000000))
				 | ((V_2&128)?0:X64(ff00000000000000)));
	  DEBUG_OP("ZAP");
	  return 0;
        case 0x31: // ZAPNOT
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] & (  ((V_2&  1)?              X64(ff):0)
				 | ((V_2&  2)?            X64(ff00):0)
				 | ((V_2&  4)?          X64(ff0000):0)
				 | ((V_2&  8)?        X64(ff000000):0)
				 | ((V_2& 16)?      X64(ff00000000):0)
				 | ((V_2& 32)?    X64(ff0000000000):0)
				 | ((V_2& 64)?  X64(ff000000000000):0)
				 | ((V_2&128)?X64(ff00000000000000):0) );
	  DEBUG_OP("ZAPNOT");
	  return 0;
        case 0x34: // SRL
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] >> (V_2 & 63);
	  DEBUG_OP("SRL");
	  return 0;
        case 0x39: // SLL
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] << (V_2 & 63);
	  DEBUG_OP("SLL");
	  return 0;
        case 0x3c: // SRA
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] >> (V_2 & 63) |
	                ((r[REG_1]>>63)?(X64_QUAD<<(64-(V_2 & 63))):0);
	  DEBUG_OP("SRA");
	  return 0;
        case 0x52: //MSKWH
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] & ~(X64_WORD>>(64-((V_2 & 7)*8)));
	  DEBUG_OP("MSKWH");
	  return 0;
        case 0x57: // INSWH
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1]&X64_WORD) >> (64-((V_2&7)*8));
	  DEBUG_OP("INSWH");
	  return 0;
        case 0x5a: // EXTWH
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1] << (64-((V_2&7)*8))) & X64_WORD;
	  DEBUG_OP("EXTWH");
	  return 0;
        case 0x62: //MSKLH
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] & ~(X64_LONG>>(64-((V_2&7)*8)));
	  DEBUG_OP("MSKLH");
	  return 0;
        case 0x67: // INSLH
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1]&X64_LONG) >> (64-((V_2&7)*8));
	  DEBUG_OP("INSLH");
	  return 0;
        case 0x6a: // EXTLH
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1] << (64-((V_2&7)*8))) & X64_LONG;
	  DEBUG_OP("EXTLH");
	  return 0;
        case 0x72: //MSKQH
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1] & ~(X64_QUAD>>(64-((V_2&7)*8)));
	  DEBUG_OP("MSKQH");
	  return 0;
        case 0x77: // INSQH
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1]&X64_QUAD) >> (64-((V_2&7)*8));
	  DEBUG_OP("INSQH");
	  return 0;
        case 0x7a: // EXTQH
	  if (DO_ACTION)
 	    r[REG_3] = (r[REG_1] << (64-((V_2&7)*8))) & X64_QUAD;
	  DEBUG_OP("EXTQH");
	  return 0;
        default:
	  UNKNOWN2;
	  return 0;
        }

    case 0x13:
      function = (ins>>5) & 0x7f;
      switch (function)
        {
	case 0x00: // MULL
	  if (DO_ACTION)
 	    r[REG_3] = SEXT((u32)r[REG_1]*(u32)V_2,32);
	  DEBUG_OP("MULL");
	  return 0;
	case 0x20: // MULQ
	  if (DO_ACTION)
 	    r[REG_3] = r[REG_1]*V_2;
	  DEBUG_OP("MULQ");
	  return 0;
	case 0x30: // UMULH
	  /*
	    This algorithm was snagged from:
	    http://www.cs.uaf.edu/2004/fall/cs301/notes/notes/node47.html

	    which is very similar to the method used in the (unreleased)
	    simh alpha emulation.

	  */
          if (DO_ACTION)
	  {  
	    temp_64_a = (r[REG_1] >> 32) & X64_LONG;
	    temp_64_b = r[REG_1]  & X64_LONG;
	    temp_64_c = (V_2 >> 32) & X64_LONG;
	    temp_64_d = V_2 & X64_LONG;
	    
	    temp_64_lo = temp_64_b * temp_64_d;
	    temp_64_x = temp_64_a * temp_64_d + temp_64_c * temp_64_b;
	    temp_64_y = ((temp_64_lo >> 32) & X64_LONG) + temp_64_x;
	    temp_64_lo = (temp_64_lo & X64_LONG) | ((temp_64_y & X64_LONG) << 32);
	    temp_64_hi = (temp_64_y >> 32) & X64_LONG;
	    temp_64_hi += temp_64_a * temp_64_c;
	    r[REG_3] = temp_64_hi;
	  }
	  DEBUG_OP("UMULH");
	  return 0;
	default:
	  UNKNOWN2;
	  return 0;
	}

    case 0x17:
      function = (ins>>5) & 0x7ff;
      switch (function)
	{
	case 0x24: //MT_FPCR
	  if (DO_ACTION)
 	    fpcr = f[FREG_1];
	  return 0;
	case 0x25: //MF_FPCR
	  if (DO_ACTION)
 	    f[FREG_1] = fpcr;
	  return 0;
	default:
	  UNKNOWN2;
	  return 0;
	}

    case 0x18:
      function = (ins & 0xffff);
      switch (function)
	{
	case 0x0000: // TRAPB
	case 0x0400: // EXCB
	case 0x4000: // MB
	case 0x4400: // WMB
	case 0x8000: // FETCH
	case 0xA000: // FETCH_M
	case 0xE800: // ECB
	case 0xF800: // WH64
	case 0xFC00: // WH64EN
	  DEBUG_("NOP");
	  return 0;
	case 0xC000: // RPCC
	  if (DO_ACTION)
 	    r[REG_1] = ((u64)cc_offset)<<32 | cc;
	  DEBUG_OP_R1("RPCC");
	  return 0;
	case 0xE000: // RC
	  if (DO_ACTION)
	  {
 	    r[REG_1] = bIntrFlag?1:0;
	    bIntrFlag = false;
	  }
	  DEBUG_OP_R1("RC");
	  return 0;
	case 0xF000: // RS
	  if (DO_ACTION)
	  {
 	    r[REG_1] = bIntrFlag?1:0;
	    bIntrFlag = true;
	  }
	  DEBUG_OP_R1("RS");
	  return 0;
	default:
	  UNKNOWN2;
	  return 0;
	}

    case 0x19: // HW_MFPR
      function = (ins>>8) & 0xff;
      if ((function & 0xc0) == 0x40)
        {	// PCTX
	  if (DO_ACTION)
 	    r[REG_1] = ((u64)asn << 39)
	      | ((u64)astrr << 9)
	      | ((u64)aster <<5)
	      | (fpen?X64(1)<<3:0)
	      | (ppcen?X64(1)<<1:0);
	  DEBUG_MFPR("PCTX");
	  return 0;
        }
      switch (function)
        {
        case 0x05: // PMPC
	  if (DO_ACTION)
 	    r[REG_1] = pmpc;
	  DEBUG_MFPR("PMPC");
	  return 0;
        case 0x06: // EXC_ADDR
	  if (DO_ACTION)
 	    r[REG_1] = exc_addr;
	  DEBUG_MFPR("EXC_ADDR");
	  return 0;
        case 0x07: // IVA_FORM
	  if (DO_ACTION)
 	    r[REG_1] = va_form(exc_addr,i_ctl_va_mode,i_ctl_vptb);
	  DEBUG_MFPR("IVA_FORM");
	  return 0;
        case 0x08: // IER_CM
        case 0x09: // CM
        case 0x0a: // IER
        case 0x0b: // IER_CM
	  if (DO_ACTION)
 	    r[REG_1] = (((u64)eien) << 33)
	      | (((u64)slen) << 32)
	      | (((u64)cren) << 31)
	      | (((u64)pcen) << 29)
	      | (((u64)sien) << 14)
	      | (((u64)asten) << 13)
	      | (((u64)cm) << 3);
	  DEBUG_MFPR("IER_CM");
	  return 0;
        case 0x0c: // SIRR
	  if (DO_ACTION)
 	    r[REG_1] = ((u64)sir) << 14;
	  DEBUG_MFPR("SIRR");
	  return 0;
        case 0x0d: // ISUM
	  if (DO_ACTION)
 	    r[REG_1] = (((u64)(eir & eien)) << 33)
	      | (((u64)(slr & slen)) << 32)
	      | (((u64)(crr & cren)) << 31)
	      | (((u64)(pcr & pcen)) << 29)
	      | (((u64)(sir & sien)) << 14)
	      | (((u64)( ((X64(1)<<(cm+1))-1) & aster & astrr & (asten * 0x3))) << 3)
	      | (((u64)( ((X64(1)<<(cm+1))-1) & aster & astrr & (asten * 0xc))) << 7);
	  DEBUG_MFPR("ISUM");
	  return 0;
        case 0x0f: // EXC_SUM
	  if (DO_ACTION)
 	    r[REG_1] = exc_sum;
	  DEBUG_MFPR("EXC_SUM");
	  return 0;
        case 0x10: // PAL_BASE
	  if (DO_ACTION)
 	    r[REG_1] = pal_base;
	  DEBUG_MFPR("PAL_BASE");
	  return 0;
        case 0x11: // i_ctl
	  if (DO_ACTION)
 	    r[REG_1] = i_ctl_other
	      | (((u64)CPU_CHIP_ID)<<24)
	      | (u64)i_ctl_vptb
	      | (((u64)i_ctl_va_mode) << 15)
	      | (hwe?X64(1)<<12:0)
	      | (sde?X64(1)<<7:0)
	      | (((u64)i_ctl_spe) << 3);
	  DEBUG_MFPR("I_CTL");
	  return 0;
        case 0x14: // PCTR_CTL
	  if (DO_ACTION)
 	    r[REG_1] = pctr_ctl;
	  DEBUG_MFPR("PCTR_CTL");
	  return 0;
        case 0x16: // I_STAT
	  if (DO_ACTION)
 	    r[REG_1] = i_stat;
	  DEBUG_MFPR("I_STAT");
	  return 0;
        case 0x27: // MM_STAT
	  if (DO_ACTION)
 	    r[REG_1] = mm_stat;
	  DEBUG_MFPR("MM_STAT");
	  return 0;
        case 0x2a: // DC_STAT
	  if (DO_ACTION)
 	    r[REG_1] = dc_stat;
	  DEBUG_MFPR("DC_STAT");
	  return 0;
        case 0x2b: // C_DATA
	  if (DO_ACTION)
 	    r[REG_1] = 0;
	  DEBUG_MFPR("C_DATA");
	  return 0;
        case 0xc0: // CC
	  if (DO_ACTION)
 	    r[REG_1] = (((u64)cc_offset) << 32) |  cc;
	  DEBUG_MFPR("CC");
	  return 0;
        case 0xc2: // VA
	  if (DO_ACTION)
 	    r[REG_1] = fault_va;
	  DEBUG_MFPR("VA");
	  return 0;
        case 0xc3: // VA_FORM
	  if (DO_ACTION)
 	    r[REG_1] = va_form(fault_va, va_ctl_va_mode, va_ctl_vptb);
	  DEBUG_MFPR("VA_FORM");
	  return 0;
        default:
	  UNKNOWN2;
	  return 0;
        }

    case 0x1a: // JMP...
      if (DO_ACTION)
	{
	  temp_64 = r[REG_2] & ~X64(3);
	  r[REG_1] = pc & ~X64(3);
	  pc = temp_64 | (pc & 3);
	}
      DEBUG_JMP("JMP");
      if (DO_ACTION)
	{
	  if (REG_1==31)
	    TRC(false, true)
	    else
	      TRC(true, true)
	}
      return 0;

    case 0x1b: // HW_LD
      function = (ins>>12) & 0xf;
      switch(function)
        {
        case 0: // longword physical
	  if (DO_ACTION)
	    {
	      phys_address = r[REG_2] + DISP_12;
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","/Phys");
	  return 0;
        case 1: // quadword physical
	  if (DO_ACTION)
	    {
	      phys_address = r[REG_2] + DISP_12;
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","/Phys");
	  return 0;
        case 2: // longword physical locked
	  if (DO_ACTION)
	    {
	      lock_flag = true;
	      phys_address = r[REG_2] + DISP_12;
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","/Phys/Lock");
	  return 0;
        case 3: // quadword physical locked
	  if (DO_ACTION)
	    {
	      lock_flag = true;
	      phys_address = r[REG_2] + DISP_12;
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","/Phys/Lock");
	  return 0;
        case 4: // longword virtual vpte               //chk //alt  //vpte
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, false, true);
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","/Vpte");
	  return 0;
        case 5: // quadword virtual vpte               //chk //alt  //vpte
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, false, true);
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","/Vpte");
	  return 0;
        case 8: // longword virtual
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, false, false);
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","");
	  return 0;
        case 9: // quadword virtual
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, false, false);
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","");
	  return 0;
        case 10: // longword virtual check
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, false, false);
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","/Chk");
	  return 0;
        case 11: // quadword virtual check
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, false, false);
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","/Chk");
	  return 0;
        case 12: // longword virtual alt
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, true, false);
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","/Alt");
	  return 0;
        case 13: // quadword virtual alt
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, true, false);
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","/Alt");
	  return 0;
        case 14: // longword virtual alt check
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, true, false);
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","/Alt/Chk");
	  return 0;
        case 15: // quadword virtual alt check
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, true, false);
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","/Alt/Chk");
	  return 0;
        default:
	  UNKNOWN2;
	  return 0;
        }
    
    case 0x1c: // op
      function = (ins>>5) & 0x7f;
      switch (function)
        {
        case 0x00: //SEXTB
	  if (DO_ACTION)
 	    r[REG_3] = SEXT(V_2,8);
	  DEBUG_OP_R23("SEXTB");
	  return 0;
        case 0x01: // SEXTW
	  if (DO_ACTION)
 	    r[REG_3] = SEXT(V_2,16);
	  DEBUG_OP_R23("SEXTW");
	  return 0;
        case 0x30: // CTPOP
	  if (DO_ACTION)
	  {
 	    temp_64 = 0;
	    temp_64_2 = V_2;
	    for (i=0;i<64;i++)
	      if ((temp_64_2>>i)&1)
	        temp_64++;
	    r[REG_3] = temp_64;
	  }
	  DEBUG_OP_R23("CTPOP");
	  return 0;
        case 0x31: // PERR
	  if (DO_ACTION)
	  {
 	    temp_64 = 0;
	    temp_64_1 = r[REG_1];
	    temp_64_2 = V_2;
	    for(i=0;i<64;i+=8)
	      if ((s8)((temp_64_1>>i)&X64_BYTE) > (s8)((temp_64_2>>i)&X64_BYTE))
	        temp_64 |=    ((u64)((s8)((temp_64_1>>i)&X64_BYTE) - (s8)((temp_64_2>>i)&X64_BYTE))<<i);
	      else
	        temp_64 |=    ((u64)((s8)((temp_64_2>>i)&X64_BYTE) - (s8)((temp_64_1>>i)&X64_BYTE))<<i);
	    r[REG_3] = temp_64;
	  }
	  DEBUG_OP("PERR");
	  return 0;
        case 0x32: // CTLZ
	  if (DO_ACTION)
	  {
 	    temp_64 = 0;
	    temp_64_2 = V_2;
	    for (i=63;i>=0;i--)
	      if ((temp_64>>i)&1)
	        break;
	      else
	        temp_64++;
	    r[REG_3] = temp_64;
	  }
	  DEBUG_OP_R23("CTLZ");
	  return 0;
        case 0x33: // CTTZ
	  if (DO_ACTION)
	  {
 	    temp_64 = 0;
	    temp_64_2 = V_2;
	    for (i=0;i<64;i++)
	      if ((temp_64>>i)&1)
	        break;
	      else
	        temp_64++;
	    r[REG_3] = temp_64;
	  }
	  DEBUG_OP_R23("CTLZ");
	  return 0;
        case 0x34: // UNPKBW
	  if (DO_ACTION)
	  {
 	    temp_64_2 = V_2;
	    r[REG_3] =  (temp_64_2 & X64(000000ff))
	      | ((temp_64_2 & X64(0000ff00)) << 8)
	      | ((temp_64_2 & X64(00ff0000)) << 16)
	      | ((temp_64_2 & X64(ff000000)) << 24);
	  }
	  DEBUG_OP_R23("UNPKBW");
	  return 0;
        case 0x35: // UNPKBL
	  if (DO_ACTION)
	  {
 	    temp_64_2 = V_2;
	    r[REG_3] =  (temp_64_2 & X64(000000ff))
	      | ((temp_64_2 & X64(0000ff00)) << 24);
	  }
	  DEBUG_OP_R23("UNPKBL");
	  return 0;
        case 0x36: // PKBW
	  if (DO_ACTION)
	  {
 	    temp_64_2 = V_2;
	    r[REG_3] =  (temp_64_2 & X64(00000000000000ff))
	      | ((temp_64_2 & X64(0000000000ff0000)) >> 8)
	      | ((temp_64_2 & X64(000000ff00000000)) >> 16)
	      | ((temp_64_2 & X64(00ff000000000000)) >> 24);
	  }
	  DEBUG_OP_R23("PKBW");
	  return 0;
        case 0x37: // PKBL
	  if (DO_ACTION)
	  {
 	    temp_64_2 = V_2;
	    r[REG_3] =  (temp_64_2 & X64(00000000000000ff))
	      | ((temp_64_2 & X64(000000ff00000000)) >> 24);
	  }
	  DEBUG_OP_R23("PKBL");
	  return 0;
        case 0x38: // MINSB8
	  if (DO_ACTION)
	  {
 	    temp_64 = 0;
	    temp_64_1 = r[REG_1];
	    temp_64_2 = V_2;
	    for(i=0;i<64;i+= 8)
	    {
	      if ((s8)((temp_64_1>>i)&X64_BYTE) > (s8)((temp_64_2>>i)&X64_BYTE))
		temp_64 |=    (((temp_64_2>>i)&X64_BYTE)<<i);
	      else
		temp_64 |=    (((temp_64_1>>i)&X64_BYTE)<<i);
            }
	    r[REG_3] = temp_64;
	  }
	  DEBUG_OP("MINSB8");
	  return 0;
        case 0x39: // MINSW4
	  if (DO_ACTION)
	  {
 	    temp_64 = 0;
	    temp_64_1 = r[REG_1];
	    temp_64_2 = V_2;
	    for(i=0;i<64;i+= 16)
            {
	      if ((s16)((temp_64_1>>i)&X64_WORD) > (s16)((temp_64_2>>i)&X64_WORD))
		temp_64 |=    (((temp_64_2>>i)&X64_WORD)<<i);
	      else
		temp_64 |=    (((temp_64_1>>i)&X64_WORD)<<i);
            }
	    r[REG_3] = temp_64;
	  }
	  DEBUG_OP("MINSW4");
	  return 0;
        case 0x3a: // MINUB8
	  if (DO_ACTION)
	  {
 	    temp_64 = 0;
	    temp_64_1 = r[REG_1];
	    temp_64_2 = V_2;
	    for(i=0;i<64;i+= 8)
	    {
	      if ((u8)((temp_64_1>>i)&X64_BYTE) > (u8)((temp_64_2>>i)&X64_BYTE))
		temp_64 |=    (((temp_64_2>>i)&X64_BYTE)<<i);
	      else
		temp_64 |=    (((temp_64_1>>i)&X64_BYTE)<<i);
            }
	    r[REG_3] = temp_64;
	  }
	  DEBUG_OP("MINUB8");
	  return 0;
        case 0x3b: // MINUW4
	  if (DO_ACTION)
	  {
 	    temp_64 = 0;
	    temp_64_1 = r[REG_1];
	    temp_64_2 = V_2;
	    for(i=0;i<64;i+= 16)
            {
	      if ((u16)((temp_64_1>>i)&X64_WORD) > (u16)((temp_64_2>>i)&X64_WORD))
		temp_64 |=    (((temp_64_2>>i)&X64_WORD)<<i);
	      else
		temp_64 |=    (((temp_64_1>>i)&X64_WORD)<<i);
            }
	    r[REG_3] = temp_64;
	  }
	  DEBUG_OP("MINUW4");
	  return 0;
        case 0x3c: // MAXUB8
	  if (DO_ACTION)
	  {
 	    temp_64 = 0;
	    temp_64_1 = r[REG_1];
	    temp_64_2 = V_2;
	    for(i=0;i<64;i+= 8)
            {
	      if ((u8)((temp_64_1>>i)&X64_BYTE) > (u8)((temp_64_2>>i)&X64_BYTE))
		temp_64 |=    (((temp_64_1>>i)&X64_BYTE)<<i);
	      else
		temp_64 |=    (((temp_64_2>>i)&X64_BYTE)<<i);
            }
	    r[REG_3] = temp_64;
	  }
	  DEBUG_OP("MAXUB8");
	  return 0;
        case 0x3d: // MAXUW4
	  if (DO_ACTION)
	  {
 	    temp_64 = 0;
	    temp_64_1 = r[REG_1];
	    temp_64_2 = V_2;
	    for(i=0;i<64;i+= 16)
            {
	      if ((u16)((temp_64_1>>i)&X64_WORD) > (u16)((temp_64_2>>i)&X64_WORD))
		temp_64 |=    (((temp_64_1>>i)&X64_WORD)<<i);
	      else
		temp_64 |=    (((temp_64_2>>i)&X64_WORD)<<i);
            }
	    r[REG_3] = temp_64;
	  }
	  DEBUG_OP("MAXUW4");
	  return 0;
        case 0x3e: // MAXSB8
	  if (DO_ACTION)
	  {
 	    temp_64 = 0;
	    temp_64_1 = r[REG_1];
	    temp_64_2 = V_2;
	    for(i=0;i<64;i+= 8)
            {
	      if ((s8)((temp_64_1>>i)&X64_BYTE) > (s8)((temp_64_2>>i)&X64_BYTE))
		temp_64 |=    (((temp_64_1>>i)&X64_BYTE)<<i);
	      else
		temp_64 |=    (((temp_64_2>>i)&X64_BYTE)<<i);
            }
	    r[REG_3] = temp_64;
	  }
	  DEBUG_OP("MAXSB8");
	  return 0;
        case 0x3f: // MAXSW4
	  if (DO_ACTION)
	  {
 	    temp_64 = 0;
	    temp_64_1 = r[REG_1];
	    temp_64_2 = V_2;
	    for(i=0;i<64;i+= 16)
            {
	      if ((s16)((temp_64_1>>i)&X64_WORD) > (s16)((temp_64_2>>i)&X64_WORD))
		temp_64 |=    (((temp_64_1>>i)&X64_WORD)<<i);
	      else
		temp_64 |=    (((temp_64_2>>i)&X64_WORD)<<i);
            }
 	    r[REG_3] = temp_64;
	  }
	  DEBUG_OP("MAXSW4");
	  return 0;
        case 0x70: // FTOIT
	  if (DO_ACTION)
 	    r[REG_3] = f[FREG_1];
	  DEBUG_OP_F1_R3("FTOIT");
	  return 0;
        case 0x78: // FTOIS
	  if (DO_ACTION)
	  {
 	    temp_64 = f[FREG_1];
	    r[REG_3] = (temp_64 & X64(000000003fffffff))
	      |((temp_64 & X64(c000000000000000)) >> 32)
	      |(((temp_64 & X64(8000000000000000)) >>31) * X64(ffffffff));
	  }
	  DEBUG_OP_F1_R3("FTOIS");
	  return 0;
        default:
	  UNKNOWN2;
	  return 0;
        }

    case 0x1d: // HW_MTPR
      function = (ins>>8) & 0xff;
      if ((function & 0xc0) == 0x40)
        {
	  if (DO_ACTION)
	  {	    
	    if (function & 1)
	      asn = (int)(r[REG_2]>>39) & 0xff;
	    if (function & 2)
	      aster = (int)(r[REG_2]>>5) & 0xf;
	    if (function & 4)
	      astrr = (int)(r[REG_2]>>9) & 0xf;
	    if (function & 8)
	      ppcen = (int)(r[REG_2]>>1) & 1;
	    if (function & 16)
	      fpen = (int)(r[REG_2]>>3) & 1;
	    DEBUG_MTPR("PCTX");
	  }
	  return 0;
        }
      switch (function)
        {
        case 0x00: // ITB_TAG
	  if (DO_ACTION)
 	    itb->write_tag(0,r[REG_2]);
	  DEBUG_MTPR("ITB_TAG");
	  return 0;
        case 0x01: // ITB_PTE
	  if (DO_ACTION)
 	    itb->write_pte(0,r[REG_2],get_asn());
	  DEBUG_MTPR("ITB_PTE");
	  return 0;
        case 0x02: // ITB_IAP
	  if (DO_ACTION)
 	    itb->InvalidateAllProcess();
	  DEBUG_MTPR("ITB_IAP");
	  return 0;
        case 0x03: // ITB_IA
	  if (DO_ACTION)
 	    itb->InvalidateAll();
	  DEBUG_MTPR("ITB_IA");
	  return 0;
        case 0x04: // ITB_IS
	  if (DO_ACTION)
 	    itb->InvalidateSingle(r[REG_2],get_asn());
	  DEBUG_MTPR("ITB_IS");
	  return 0;
        case 0x09: // CM
	  if (DO_ACTION)
 	    cm = (int)(r[REG_2]>>3) & 3;
	  DEBUG_MTPR("CM");
	  return 0;
        case 0x0b: // IER_CM
	  if (DO_ACTION)
 	    cm = (int)(r[REG_2]>>3) & 3;
        case 0x0a: // IER
	  if (DO_ACTION)
	  {
 	    asten = (int)(r[REG_2]>>13) & 1;
	    sien  = (int)(r[REG_2]>>14) & 0x3fff;
	    pcen  = (int)(r[REG_2]>>29) & 3;
	    cren  = (int)(r[REG_2]>>31) & 1;
	    slen  = (int)(r[REG_2]>>32) & 1;
	    eien  = (int)(r[REG_2]>>33) & 0x3f;
	  }
	  DEBUG_MTPR("IER[_CM]");
	  return 0;
        case 0x0c: // SIRR
	  if (DO_ACTION)
 	    sir = (int)(r[REG_2]>>14) & 0x3fff;
	  DEBUG_MTPR("SIRR");
	  return 0;
        case 0x0e: // HW_INT_CLR
	  if (DO_ACTION)
	  {
	    pcr &= ~((r[REG_2]>>29)&X64(3));
	    crr &= ~((r[REG_2]>>31)&X64(1));
	    slr &= ~((r[REG_2]>>32)&X64(1));
	  }
	  DEBUG_MTPR("HW_INT_CLT");
	  return 0;
        case 0x10: // PAL_BASE
	  if (DO_ACTION)
 	    pal_base = r[REG_2] & X64(00000fffffff8000);
	  DEBUG_MTPR("PAL_BASE");
	  return 0;
        case 0x11: // i_ctl
	  if (DO_ACTION)
	  {
 	    i_ctl_other = r[REG_2]    & X64(00000000007e2f67);
	    i_ctl_vptb  = SEXT (r[REG_2] & X64(0000ffffc0000000),48);
	    i_ctl_spe   = (int)(r[REG_2]>>3) & 3;
	    sde         = (r[REG_2]>>7) & 1;
	    hwe         = (r[REG_2]>>12) & 1;
	    i_ctl_va_mode = (int)(r[REG_2]>>15) & 3;
	  }
	  DEBUG_MTPR("I_CTL");
	  return 0;
        case 0x12: // ic_flush_asm
	  if (DO_ACTION)
 	    flush_icache();
	  DEBUG_MTPR("IC_FLUSH_ASM");
	  return 0;
        case 0x13: // IC_FLUSH
	  if (DO_ACTION)
 	    flush_icache();
	  DEBUG_MTPR("IC_FLUSH");
	  return 0;
        case 0x14: // PCTR_CTL
	  if (DO_ACTION)
 	    pctr_ctl = r[REG_2] & X64(ffffffffffffffdf);
	  DEBUG_MTPR("PCTR_CTL");
	  return 0;
        case 0x15: // CLR_MAP
	  // NOP
	  DEBUG_MTPR("CLR_MAP");
	  return 0;
        case 0x16: // I_STAT
	  if (DO_ACTION)
 	    i_stat &= ~r[REG_2]; //W1C
	  DEBUG_MTPR("I_STAT");
	  return 0;
        case 0x17: // SLEEP
	  // NOP
	  DEBUG_MTPR("SLEEP");
	  return 0;
        case 0x20: // DTB_TAG0
	  if (DO_ACTION)
 	    dtb->write_tag(0,r[REG_2]);
	  DEBUG_MTPR("DTB_TAG0");
	  return 0;
        case 0x21: // DTB_PTE0
	  if (DO_ACTION)
 	    dtb->write_pte(0,r[REG_2],get_asn());
	  DEBUG_MTPR("DTB_PTE0");
	  return 0;
        case 0x24: // DTB_IS0
	  if (DO_ACTION)
 	    dtb->InvalidateSingle(r[REG_2],get_asn());
	  DEBUG_MTPR("DTB_IS0");
	  return 0;
        case 0x25: // DTB_ASN0
	  if (DO_ACTION)
 	    asn0 = (int)(r[REG_2] >> 56);
	  DEBUG_MTPR("DTB_ASN0");
	  return 0;
        case 0x26: // DTB_ALTMODE
	  if (DO_ACTION)
 	    alt_cm = (int)(r[REG_2] & 3);
	  DEBUG_MTPR("DTB_ALTMODE");
	  return 0;
        case 0x27: // MM_STAT
	  DEBUG_MTPR("MM_STAT");
	  return 0;
        case 0x28: // M_CTL
	  if (DO_ACTION)
	  {
 	    smc = (int)(r[REG_2]>>4) & 3;
	    m_ctl_spe = (int)(r[REG_2]>>1) & 7;
	  }
	  DEBUG_MTPR("M_CTL");
	  return 0;
        case 0x29: // DC_CTL
	  if (DO_ACTION)
 	    dc_ctl = r[REG_2];
	  DEBUG_MTPR("DC_CTL");
	  return 0;
        case 0x2a: // DC_STAT
	  if (DO_ACTION)
 	    dc_stat &= ~r[REG_2];
	  DEBUG_MTPR("DC_STAT");
	  return 0;
        case 0x2b: // C_DATA
	  DEBUG_MTPR("C_DATA");
	  return 0;
        case 0x2c: // C_SHIFT
	  DEBUG_MTPR("C_SHIFT");
	  return 0;
        case 0xa0: // DTB_TAG1
	  if (DO_ACTION)
 	    dtb->write_tag(1,r[REG_2]);
	  DEBUG_MTPR("DTB_TAG1");
	  return 0;
        case 0xa1: // DTB_PTE1
	  if (DO_ACTION)
 	    dtb->write_pte(1,r[REG_2],get_asn());
	  DEBUG_MTPR("DTB_PTE1");
	  return 0;
        case 0xa2: // DTB_IAP
	  if (DO_ACTION)
 	    dtb->InvalidateAllProcess();
	  DEBUG_MTPR("DTB_IAP");
	  return 0;
        case 0xa3: // DTB_IA
	  if (DO_ACTION)
 	    dtb->InvalidateAll();
	  DEBUG_MTPR("DTB_IA");
	  return 0;
        case 0xa4: // DTB_IS1
	  if (DO_ACTION)
 	    dtb->InvalidateSingle(r[REG_2],get_asn());
	  DEBUG_MTPR("DTB_IA1");
	  return 0;
        case 0xa5: // DTB_ASN1
	  if (DO_ACTION)
 	    asn1 = (int)(r[REG_2] >> 56);
	  DEBUG_MTPR("DTB_ASN1");
	  return 0;
        case 0xc0: // CC
	  if (DO_ACTION)
 	    cc_offset = (u32)(r[REG_2] >> 32);
	  DEBUG_MTPR("CC");
	  return 0;
        case 0xc1: // CC_CTL
	  if (DO_ACTION)
	  {
 	    cc_ena = (r[REG_2] >> 32) & 1;
	    cc    = (u32)(r[REG_2] & X64(fffffff0));
	  }
	  DEBUG_MTPR("CC_CTL");
	  return 0;
        case 0xc4: // VA_CTL
	  if (DO_ACTION)
	  {
 	    va_ctl_vptb = SEXT(r[REG_2] & X64(0000ffffc0000000),48);
	    va_ctl_va_mode = (int)(r[REG_2]>>1) & 3;
	  }
	  DEBUG_MTPR("VA_CTL");
	  return 0;
        default:
	  UNKNOWN2;
	  return 0;
        }

    case 0x1e: // HW_RET
      if (DO_ACTION)
	{
	  pc = r[REG_2];
	}
      DEBUG_RET("HW_RET");
      if (DO_ACTION)
	{
	  TRC(false, true);
	}
      return 0;

    case 0x1f: // HW_ST
      function = (ins>>12) & 0xf;
      switch(function)
        {
        case 0: // longword physical
	  if (DO_ACTION)
	    {
	      phys_address = r[REG_2] + DISP_12;
	      WRITE_PHYS_NT(r[REG_1],32);
	    }
	  DEBUG_HW("HW_STL","/Phys");
	  return 0;
        case 1: // quadword physical
	  if (DO_ACTION)
	    {
	      phys_address = r[REG_2] + DISP_12;
	      WRITE_PHYS_NT(r[REG_1],64);
	    }
	  DEBUG_HW("HW_STQ","/Phys");
	  return 0;
        case 2: // longword physical conditional
	  if (DO_ACTION)
	    {
	      if (lock_flag)
		{
		  phys_address = r[REG_2] + DISP_12;
		  WRITE_PHYS_NT(r[REG_1],32);
		}
	      r[REG_1] = lock_flag?1:0;
	      lock_flag = false;
	    }
	  DEBUG_HW("HW_STL","/Phys/Cond");
	  return 0;
        case 3: // quadword physical conditional
	  if (DO_ACTION)
	    {
	      if (lock_flag)
		{
		  phys_address = r[REG_2] + DISP_12;
		  WRITE_PHYS_NT(r[REG_1],64);
		}
	      r[REG_1] = lock_flag?1:0;
	      lock_flag = false;
	    }
	  DEBUG_HW("HW_STQ","/Phys/Cond");
	  return 0;
        case 4: // longword virtual                   //chk //alt  //vpte
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, false, false);
	      WRITE_PHYS_NT(r[REG_1],32);
	    }
	  DEBUG_HW("HW_STL","");
	  return 0;
        case 5: // quadword virtual                    //chk //alt  //vpte
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, false, false);
	      WRITE_PHYS_NT(r[REG_1],64);
	    }
	  DEBUG_HW("HW_STQ","");
	  return 0;
        case 12: // longword virtual alt
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, true, false);
	      WRITE_PHYS_NT(r[REG_1],32);
	    }
	  DEBUG_HW("HW_STL","/Alt");
	  return 0;
        case 13: // quadword virtual alt
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, true, false);
	      WRITE_PHYS_NT(r[REG_1],64);
	    }
	  DEBUG_HW("HW_STQ","/Alt");
	  return 0;
        default:
	  UNKNOWN2;
	  return 0;
        }

    case 0x28: // LDL
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);
	  r[REG_1] = SEXT(READ_PHYS(32),32);
	}
      DEBUG_LD_ST("LDL");
      return 0;

    case 0x29: // LDQ
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);
	  r[REG_1] = READ_PHYS(64);
	}
      DEBUG_LD_ST("LDQ");
      return 0;

    case 0x2a: // LDL_L
      if (DO_ACTION)
	{
	  lock_flag = true;
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);
	  r[REG_1] = SEXT(READ_PHYS(32),32);
	}
      DEBUG_LD_ST("LDL_L");
      return 0;

    case 0x2b: // LDQ_L
      if (DO_ACTION)
	{
	  lock_flag = true;
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);
	  r[REG_1] = READ_PHYS(64);
	}
      DEBUG_LD_ST("LDQ_L");
      return 0;

    case 0x2c: // STL
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);
	  WRITE_PHYS(r[REG_1],32);
	}
      DEBUG_LD_ST("STL");
      return 0;

    case 0x2d: // STQ
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);
	  WRITE_PHYS(r[REG_1],64);
	}
      DEBUG_LD_ST("STQ");
      return 0;

    case 0x2e: // STL_C
      if (DO_ACTION)
	{
	  if (lock_flag)
	    {
	      DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);
	      WRITE_PHYS(r[REG_1],32);
	    }
	  r[REG_1] = lock_flag?1:0;
	  lock_flag = false;
	}
      DEBUG_LD_ST("STL_C");
      return 0;

    case 0x2f: // STQ_C
      if (DO_ACTION)
	{
	  if (lock_flag)
	    {
	      DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);
	      WRITE_PHYS(r[REG_1],64);
	    }
	  r[REG_1] = lock_flag?1:0;
	  lock_flag = false;
	}
      DEBUG_LD_ST("STQ_C");
      return 0;

    case 0x30: // BR
      if (DO_ACTION)
	{
	  r[REG_1] = pc & ~X64(3);
	  pc += (DISP_21 * 4);
	}
      DEBUG_BR("BR");
      if (DO_ACTION)
	{
	  TRC_BR;
	}
      return 0;

    case 0x34: // BSR
      if (DO_ACTION)
	{
	  r[REG_1] = pc & ~X64(3);
	  pc += (DISP_21 * 4);
	}
      DEBUG_BR("BSR");
      if (DO_ACTION)
	{
	  if (REG_1==31)
	    TRC(false, true)
	    else
	      TRC(true, true)
	}
      return 0;

    case 0x38: // BLBC
      DEBUG_BR("BLBC");
      if (DO_ACTION)
	{
	  if (!(r[REG_1] & 1))
	    {
	      pc += (DISP_21 * 4);
	      TRC_BR;
	    }
	}
      return 0;

    case 0x39: // BEQ
      DEBUG_BR("BEQ");
      if (DO_ACTION)
	{
	  if (!r[REG_1])
	    {
	      pc += (DISP_21 * 4);
	      TRC_BR;
	    }
	}
      return 0;

    case 0x3a: // BLT
      DEBUG_BR("BLT");
      if (DO_ACTION)
	{
	  if ((s64)r[REG_1]<0)
	    {
	      pc += (DISP_21 * 4);
	      TRC_BR;
	    }
	}
      return 0;

    case 0x3b: // BLE
      DEBUG_BR("BLE");
      if (DO_ACTION)
	{
	  if ((s64)r[REG_1]<=0)
	    {
	      pc += (DISP_21 * 4);
	      TRC_BR;
	    }
	}
      return 0;

    case 0x3c: // BLBS
      DEBUG_BR("BLBS");
      if (DO_ACTION)
	{
	  if (r[REG_1] & 1)
	    {
	      pc += (DISP_21 * 4);
	      TRC_BR;
	    }
	}
      return 0;

    case 0x3d: // BNE
      DEBUG_BR("BNE");
      if (DO_ACTION)
	{
	  if (r[REG_1])
	    {
	      pc += (DISP_21 * 4);
	      TRC_BR;
	    }
	}
      return 0;

    case 0x3e: // BGE
      DEBUG_BR("BGE");
      if (DO_ACTION)
	{
	  if ((s64)r[REG_1]>=0)
	    {
	      pc += (DISP_21 * 4);
	      TRC_BR;
	    }
	}
      return 0;

    case 0x3f: // BGT
      DEBUG_BR("BGT");
      if (DO_ACTION)
	{
	  if ((s64)r[REG_1]>0)
	    {
	      pc += (DISP_21 * 4);
	      TRC_BR;
	    }
	}
      return 0;

    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    default:
      UNKNOWN1;
      return 0;
    }
    return 0;
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
  pc = from;
  for(;;)
  {
    DoClock();
    if (pc>to)
      break;
  }
  for(pc=from;pc<=to;DoClock())
  bListing = false;
  pc = iSavedPC;
  bDisassemble = bSavedDebug;
}
#endif

/**
 * Save state to a Virtual Machine State file.
 **/

void CAlphaCPU::SaveState(FILE *f)
{
  fwrite(r,1,64*8,f);
  fwrite(this->f,1,32*8,f);

  fwrite(&pal_base,1,8,f);
  fwrite(&pc,1,8,f);
  fwrite(&dc_stat,1,8,f);
  fwrite(&i_stat,1,8,f);
  fwrite(&pctr_ctl,1,8,f);
  fwrite(&dc_ctl,1,8,f);
  fwrite(&fault_va,1,8,f);
  fwrite(&exc_sum,1,8,f);
  fwrite(&i_ctl_vptb,1,8,f);
  fwrite(&va_ctl_vptb,1,8,f);
  fwrite(&i_ctl_other,1,8,f);
  fwrite(&mm_stat,1,8,f);
  fwrite(&exc_addr,1,8,f);
  fwrite(&pmpc,1,8,f);
  fwrite(&fpcr,1,8,f);

  fwrite(&cc,1,4,f);
  fwrite(&cc_offset,1,4,f);

  fwrite(&ppcen,1,sizeof(bool),f);
  fwrite(&cc_ena,1,sizeof(bool),f);
  fwrite(&fpen,1,sizeof(bool),f);
  fwrite(&sde,1,sizeof(bool),f);
  fwrite(&ppcen,1,sizeof(bool),f);
  fwrite(&hwe,1,sizeof(bool),f);
  fwrite(&bIntrFlag,1,sizeof(bool),f);

  //	u64 current_pc;

  fwrite(&alt_cm,1,sizeof(int),f);
  fwrite(&smc,1,sizeof(int),f);
  fwrite(&i_ctl_va_mode,1,sizeof(int),f);
  fwrite(&va_ctl_va_mode,1,sizeof(int),f);
  fwrite(&cm,1,sizeof(int),f);
  fwrite(&asn,1,sizeof(int),f);
  fwrite(&asn0,1,sizeof(int),f);
  fwrite(&asn1,1,sizeof(int),f);
  fwrite(&eien,1,sizeof(int),f);
  fwrite(&slen,1,sizeof(int),f);
  fwrite(&cren,1,sizeof(int),f);
  fwrite(&pcen,1,sizeof(int),f);
  fwrite(&sien,1,sizeof(int),f);
  fwrite(&asten,1,sizeof(int),f);
  fwrite(&sir,1,sizeof(int),f);
  fwrite(&eir,1,sizeof(int),f);
  fwrite(&slr,1,sizeof(int),f);
  fwrite(&crr,1,sizeof(int),f);
  fwrite(&pcr,1,sizeof(int),f);
  fwrite(&astrr,1,sizeof(int),f);
  fwrite(&aster,1,sizeof(int),f);
  fwrite(&m_ctl_spe,1,sizeof(int),f);
  fwrite(&i_ctl_spe,1,sizeof(int),f);

  fwrite(&next_icache,1,sizeof(int),f);
  fwrite(icache,1,1024*sizeof(struct SICache),f);

  itb->SaveState(f);
  dtb->SaveState(f);
}

/**
 * Restore state from a Virtual Machine State file.
 **/

void CAlphaCPU::RestoreState(FILE *f)
{
  fread(r,1,64*8,f);
  fread(this->f,1,32*8,f);

  fread(&pal_base,1,8,f);
  fread(&pc,1,8,f);
  fread(&dc_stat,1,8,f);
  fread(&i_stat,1,8,f);
  fread(&pctr_ctl,1,8,f);
  fread(&dc_ctl,1,8,f);
  fread(&fault_va,1,8,f);
  fread(&exc_sum,1,8,f);
  fread(&i_ctl_vptb,1,8,f);
  fread(&va_ctl_vptb,1,8,f);
  fread(&i_ctl_other,1,8,f);
  fread(&mm_stat,1,8,f);
  fread(&exc_addr,1,8,f);
  fread(&pmpc,1,8,f);
  fread(&fpcr,1,8,f);

  fread(&cc,1,4,f);
  fread(&cc_offset,1,4,f);

  fread(&ppcen,1,sizeof(bool),f);
  fread(&cc_ena,1,sizeof(bool),f);
  fread(&fpen,1,sizeof(bool),f);
  fread(&sde,1,sizeof(bool),f);
  fread(&ppcen,1,sizeof(bool),f);
  fread(&hwe,1,sizeof(bool),f);
  fread(&bIntrFlag,1,sizeof(bool),f);

  //	u64 current_pc;

  fread(&alt_cm,1,sizeof(int),f);
  fread(&smc,1,sizeof(int),f);
  fread(&i_ctl_va_mode,1,sizeof(int),f);
  fread(&va_ctl_va_mode,1,sizeof(int),f);
  fread(&cm,1,sizeof(int),f);
  fread(&asn,1,sizeof(int),f);
  fread(&asn0,1,sizeof(int),f);
  fread(&asn1,1,sizeof(int),f);
  fread(&eien,1,sizeof(int),f);
  fread(&slen,1,sizeof(int),f);
  fread(&cren,1,sizeof(int),f);
  fread(&pcen,1,sizeof(int),f);
  fread(&sien,1,sizeof(int),f);
  fread(&asten,1,sizeof(int),f);
  fread(&sir,1,sizeof(int),f);
  fread(&eir,1,sizeof(int),f);
  fread(&slr,1,sizeof(int),f);
  fread(&crr,1,sizeof(int),f);
  fread(&pcr,1,sizeof(int),f);
  fread(&astrr,1,sizeof(int),f);
  fread(&aster,1,sizeof(int),f);
  fread(&m_ctl_spe,1,sizeof(int),f);
  fread(&i_ctl_spe,1,sizeof(int),f);

  fread(&next_icache,1,sizeof(int),f);
  fread(icache,1,1024*sizeof(struct SICache),f);

  itb->RestoreState(f);
  dtb->RestoreState(f);
}
