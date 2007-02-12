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

#ifdef IDB

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

#endif

/**
 * Constructor.
 **/

CAlphaCPU::CAlphaCPU(CSystem * system) : CSystemComponent (system)
{
  iProcNum = cSystem->RegisterCPU(this);
  cSystem = system;
  int i;

  cSystem->RegisterClock(this);

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
  va_ctl_va_mode = 1;
  va_ctl_vptb = 0;

  itb = new CTranslationBuffer (this, true);
  dtb = new CTranslationBuffer (this, false);

  // SROM imitatie...

  dtb->write_tag(0,0);
  dtb->write_pte(0,X64(ff61),get_asn());
    
  printf("%%CPU-I-INIT: Alpha AXP 21264 EV68 processor %d initialized.\n", iProcNum);
}

/**
 * Destructor.
 **/

CAlphaCPU::~CAlphaCPU()
{

}

#define SEXT_8(x)  ((x&      X64_BYTE) | (((x&      X64_BYTE)>>7 )?X64(ffffffffffffff00):0))
#define SEXT_12(x) ((x&     X64(fff)) | (((x&     X64(fff))>>11)?X64(fffffffffffff000):0))
#define SEXT_13(x) ((x&    X64(1fff)) | (((x&    X64(1fff))>>12)?X64(ffffffffffffe000):0))
#define SEXT_16(x) ((x&    X64_WORD) | (((x&    X64_WORD)>>15)?X64(ffffffffffff0000):0))
#define SEXT_21(x) ((x&  X64(1fffff)) | (((x&  X64(1fffff))>>20)?X64(ffffffffffe00000):0))
#define SEXT_32(x) ((x&X64_LONG) | (((x&X64_LONG)>>31)?X64(ffffffff00000000):0))

#ifdef IDB

#define TRC_(down,up,x,y) {					\
    if (bTrace)							\
      trc->trace(this, current_pc,pc,down,up,x,y); }

#define TRC(down,up) {							\
    if (bTrace)								\
      trc->trace(this, current_pc,pc,down,up,(char*)0,0); }

#define TRC_BR {							\
    if (bTrace) trc->trace_br(this, current_pc,pc); }

#define GO_PAL(offset) {					\
    exc_addr = current_pc;					\
    pc =  pal_base | offset | 1;				\
    if ((offset==DTBM_SINGLE || offset==ITB_MISS) && bTrace)	\
      trc->set_waitfor(this, exc_addr&~X64(3));	\
    else							\
      TRC_(true,false,"GO_PAL %04x",offset); }

#else

#define TRC_(down,up,x,y) ;
#define TRC(down,up) ;
#define TRC_BR ;
#define GO_PAL(offset) {					\
    exc_addr = current_pc;					\
    pc = pal_base | offset | 1; }

#endif



#define DISP_12 (SEXT_12(ins))
#define DISP_13 (SEXT_13(ins))
#define DISP_16 (SEXT_16(ins))
#define DISP_21 (SEXT_21(ins))

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
      return;	} }

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


#define REG_1 (((ins>>21) & 0x1f) + (((pc&1) && (((ins>>21)&0xc)==0x4) && sde)?32:0))
#define REG_2 (((ins>>16) & 0x1f) + (((pc&1) && (((ins>>16)&0xc)==0x4) && sde)?32:0))
#define REG_3 (( ins      & 0x1f) + (((pc&1) && (( ins     &0xc)==0x4) && sde)?32:0))

#define RREG(a) (((a) & 0x1f) + (((pc&1) && (((a)&0xc)==0x4) && sde)?32:0))


#define FREG_1 ((ins>>21) & 0x1f)
#define FREG_2 ((ins>>16) & 0x1f)
#define FREG_3 ( ins      & 0x1f)


#define V_2 ( (ins&0x1000)?((ins>>13)&0xff):r[REG_2] )

#ifdef IDB

#define DEBUG_XX							\
  char * funcname = 0;							\
  if (trc->get_fnc_name(current_pc&~X64(3),&funcname))	\
    {									\
      if (bListing && !strcmp(funcname,""))				\
        {								\
	  printf("%08x: \"%s\"\n",(u32)current_pc,			\
		 cSystem->PtrToMem(current_pc));		        \
	  pc = (current_pc + strlen(cSystem->PtrToMem(current_pc)) + 4)	\
	    & ~X64(3);							\
	  while (pc < 0x600000 && cSystem->ReadMem(pc,32)==0) pc += 4;	\
	  return;							\
        }								\
      else if (bListing && !strcmp(funcname,"!SKIP"))			\
        {								\
	  while (pc < 0x600000 && cSystem->ReadMem(pc,32)==0) pc += 4;	\
	  return;							\
        }								\
      else if (bListing && !strncmp(funcname,"!CHAR-",6))		\
        {								\
	  u64 xx_upto;							\
	  int xx_result;						\
	  xx_result = sscanf(&(funcname[6]),"%I64x",&xx_upto);		\
	  if (xx_result==1)						\
	    {								\
	      pc = current_pc;						\
	      while (pc < xx_upto)					\
		{							\
		  printf("%08x: \"%s\"\n",(u32)pc, cSystem->PtrToMem(pc)); \
		  pc += strlen(cSystem->PtrToMem(pc));			\
		  while (pc < xx_upto && cSystem->ReadMem(pc,8)==0)	\
		    pc++;						\
		}							\
	      return;							\
	    }								\
        }								\
      else if (bListing && !strncmp(funcname,"!LCHAR-",7))		\
        {								\
	  char stringval[300];						\
	  int  stringlen;						\
	  u64 xx_upto;							\
	  int xx_result;						\
	  xx_result = sscanf(&(funcname[7]),"%I64x",&xx_upto);		\
	  if (xx_result==1)						\
	    {								\
	      pc = current_pc;						\
	      while (pc < xx_upto)					\
		{							\
		  stringlen = (int)cSystem->ReadMem(pc++,8);		\
		  memset(stringval,0,300);				\
		  strncpy(stringval,cSystem->PtrToMem(pc),stringlen);	\
		  printf("%08x: \"%s\"\n",(u32)pc-1, stringval);	\
		  pc += stringlen;					\
		  while (pc < xx_upto && cSystem->ReadMem(pc,8)==0)	\
		    pc++;						\
		}							\
	      return;							\
	    }								\
        }								\
      else if (bListing && !strncmp(funcname,"!X64-",5))		\
        {								\
	  printf("\n%s:\n",&(funcname[5]));				\
	  pc = current_pc;						\
	  while (   (pc==current_pc)					\
		    || !trc->get_fnc_name(pc,&funcname) )	\
	    {								\
	      printf("%08x: %016I64x\n",(u32)pc, cSystem->ReadMem(pc,64)); \
	      pc += 8;							\
	    }								\
	  return;							\
        }								\
      else if (bListing&& !strncmp(funcname,"!X32-",5))		\
        {								\
	  printf("\n%s:\n",&(funcname[5]));				\
	  pc = current_pc;						\
	  while (   (pc==current_pc)					\
		    || !trc->get_fnc_name(pc,&funcname) )	\
	    {								\
	      printf("%08x: %08I64x\n",(u32)pc, cSystem->ReadMem(pc,32)); \
	      pc += 4;							\
	    }								\
	  return;							\
        }								\
      else if (bListing && !strncmp(funcname,":",1))			\
	printf("%s:\n",funcname);					\
      else								\
	printf("\n%s:\n",funcname);					\
    }									\
  printf("%08x: ", (u32)current_pc);					\
  if (bListing)							\
    printf("%08x %c%c%c%c: ", (u32)ins,					\
	   printable((char)(ins)),     printable((char)(ins>>8)),	\
	   printable((char)(ins>>16)), printable((char)(ins>>24)));

#define UNKNOWN1					\
  if (bDisassemble)					\
    {							\
      DEBUG_XX						\
      if (DO_ACTION)					\
	  printf("Unknown opcode: %02x\n", opcode);	\
	else						\
	  printf("\n");					\
    }

#define UNKNOWN2							\
  if (bDisassemble)							\
    {									\
      DEBUG_XX								\
	if (DO_ACTION)							\
	  printf("Unknown opcode: %02x.%02x\n", opcode, function);	\
	else								\
	  printf("\n");							\
    }

#define DEBUG_LD_ST(a)							\
  if (bDisassemble)							\
    {									\
      DEBUG_XX								\
	printf("%s r%d, %04xH(r%d)", a, REG_1&31, (u32)DISP_16, REG_2&31); \
      if (DO_ACTION)							\
	printf(" ==> %08x%08x", (u32)(r[REG_1]>>32), (u32)(r[REG_1]));	\
      printf("\n");							\
    }

#define DEBUG_HW(a,b)							\
  if (bDisassemble)								\
    {									\
      DEBUG_XX								\
	printf("%s r%d, %04xH(r%d)%s", a, REG_1&31, (u32)DISP_12, REG_2&31, b); \
      if (DO_ACTION)							\
	printf(" ==> %08x%08x",(u32)(r[REG_1]>>32), (u32)(r[REG_1]));	\
      printf("\n");							\
    }

#define DEBUG_OP(a)							\
  if (bDisassemble)								\
    {									\
      DEBUG_XX								\
	printf("%s r%d, ", a, REG_1&31);				\
      if (ins&0x1000)							\
	printf("%02xH",V_2);						\
      else								\
	printf("r%d",REG_2&31);						\
      printf(", r%d", REG_3&31);					\
      if (DO_ACTION)							\
	printf(" ==> %08x%08x", (u32)(r[REG_3]>>32), (u32)(r[REG_3]));	\
      printf("\n");							\
    }

#define DEBUG_OP_R1(a)							\
  if (bDisassemble)								\
    {									\
      DEBUG_XX								\
	printf("%s r%d", a, REG_1&31);					\
      if (DO_ACTION)							\
	printf(" ==> %08x%08x", (u32)(r[REG_1]>>32), (u32)(r[REG_1]));	\
      printf("\n");							\
    }

#define DEBUG_OP_R3(a)							\
  if (bDisassemble)								\
    {									\
      DEBUG_XX								\
	printf("%s r%d", a, REG_3&31);					\
      if (DO_ACTION)							\
	printf(" ==> %08x%08x", (u32)(r[REG_3]>>32), (u32)(r[REG_3]));	\
      printf("\n");							\
    }

#define DEBUG_OP_F1_R3(a)						\
  if (bDisassemble)								\
    {									\
      DEBUG_XX								\
	printf("%s f%d, r%d", a, FREG_1, REG_3&31);			\
      if (DO_ACTION)							\
	printf(" ==> %08x%08x", (u32)(r[REG_3]>>32), (u32)(r[REG_3]));	\
      printf("\n");							\
    }

#define DEBUG_OP_R23(a)							\
  if (bDisassemble)								\
    {									\
      DEBUG_XX								\
	printf("%s ", a);						\
      if								\
	(ins&0x1000) printf("%02xH",V_2);				\
      else								\
	printf("r%d",REG_2&31);						\
      printf(", r%d", REG_3&31);					\
      if (DO_ACTION)							\
	printf(" ==> %08x%08x", (u32)(r[REG_3]>>32), (u32)(r[REG_3]));	\
      printf("\n");							\
    }


#define DEBUG_BR(a)						\
  if (bDisassemble)							\
    {								\
      u64 dbg_x = (current_pc + 4 + (DISP_21 * 4))&~X64(3);	\
      DEBUG_XX							\
	printf("%s r%d, ", a, REG_1&31);			\
      if (trc->get_fnc_name(dbg_x,&funcname))	\
	printf("%s\n",funcname);				\
      else							\
	printf ("...%08x\n", dbg_x);				\
    }

#define DEBUG_JMP(a)					\
  if (bDisassemble)						\
    {							\
      DEBUG_XX						\
	printf("%s r%d, r%d\n", a, REG_1&31, REG_2&31);	\
    }

#define DEBUG_RET(a)				\
  if (bDisassemble)					\
    {						\
      DEBUG_XX					\
	printf("%s r%d\n", a, REG_2&31);	\
    }

#define DEBUG_fnc(a)				\
  if (bDisassemble)					\
    {						\
      DEBUG_XX					\
	printf("%s %02xH\n", a, function);	\
    }

#define DEBUG_PAL					\
  if (bDisassemble)						\
    {							\
      DEBUG_XX						\
	printf("CALL_PAL %s\n", PAL_NAME[function]);	\
    }

#define DEBUG_(a)				\
  if (bDisassemble)					\
    {						\
      DEBUG_XX					\
	printf("%s\n", a);			\
    }

#define DEBUG_MFPR(a)							\
  if (bDisassemble)								\
    {									\
      DEBUG_XX								\
	printf("HW_MFPR r%d, %s", REG_1&31, a);				\
      if (DO_ACTION)							\
	printf(" ==> %08x%08x", (u32)(r[REG_1]>>32), (u32)(r[REG_1]));	\
      printf("\n");							\
    }

#define DEBUG_MTPR(a)							\
  if (bDisassemble)								\
    {									\
      DEBUG_XX								\
	printf("HW_MTPR r%d, %s", REG_2&31, a);				\
      if (DO_ACTION)							\
	printf(" ==> %08x%08x", (u32)(r[REG_2]>>32), (u32)(r[REG_2]));	\
      printf("\n");							\
    }

#else

#define UNKNOWN1 ;
#define UNKNOWN2 ;
#define DEBUG_LD_ST(a) ;
#define DEBUG_HW(a,b) ;
#define DEBUG_OP(a) ;
#define DEBUG_OP_R1(a) ;
#define DEBUG_OP_R3(a) ;
#define DEBUG_OP_F1_R3(a) ;
#define DEBUG_OP_R23(a) ;
#define DEBUG_BR(a) ;
#define DEBUG_JMP(a) ;
#define DEBUG_RET(a) ;
#define DEBUG_fnc(a) ;
#define DEBUG_PAL ;
#define DEBUG_(a) ;
#define DEBUG_MFPR(a) ;
#define DEBUG_MTPR(a) ;

#endif

/**
 * Called each clock-cycle.
 * This is where the actual CPU emulation takes place. Each clocktick, one instruction
 * is processed by the processor. The instruction pipeline is not emulated, things are
 * complicated enough as it is. The one exception is the instruction cache, which is
 * implemented, to accomodate self-modifying code.
 **/

void CAlphaCPU::DoClock()
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
  u64 temp_64_z;

  int opcode;
  int function;

  current_pc = pc;

  if (DO_ACTION)
    {
      // check for interrupts
      if ((!(pc&X64(1))) && (eien & eir))
	{
	  //			printf("*** INTERRUPT %x ***\n",eir&eien);
	  GO_PAL(INTERRUPT);
	  return;
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
	  return;
	}
    }
  else
    {
      ins = (u32)(cSystem->ReadMem(pc,32));
    }

  pc+=4;
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
	  return;
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
      return;

    case 0x08: // LDA
      r[REG_1] = r[REG_2] + DISP_16;
      DEBUG_LD_ST("LDA");
      return;

    case 0x09: // LDAH
      r[REG_1] = r[REG_2] + (DISP_16<<16);
      DEBUG_LD_ST("LDAH");
      return;

    case 0x0a: // LDBU
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);
	  r[REG_1] = READ_PHYS(8);
	}
      DEBUG_LD_ST("LDBU");
      return;

    case 0x0b: // LDQ_U
      if (DO_ACTION)
	{
	  DATA_PHYS((r[REG_2] + DISP_16)& ~X64(7), ACCESS_READ, true, false, false);
	  r[REG_1] = READ_PHYS(64);
	}
      DEBUG_LD_ST("LDQ_U");
      return;

    case 0x0c: // LDWU
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);
	  r[REG_1] = READ_PHYS(16);
	}
      DEBUG_LD_ST("LDWU");
      return;

    case 0x0d: // STW
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);
	  WRITE_PHYS(r[REG_1],16);
	}
      DEBUG_LD_ST("STW");
      return;

    case 0x0e: // STB
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);
	  WRITE_PHYS(r[REG_1],8);
	}
      DEBUG_LD_ST("STB");
      return;

    case 0x0f: // STQ_U
      if (DO_ACTION)
	{
	  DATA_PHYS((r[REG_2] + DISP_16)& ~X64(7), ACCESS_WRITE, true, false, false);
	  WRITE_PHYS(r[REG_1],64);
	}
      DEBUG_LD_ST("STQ_U");
      return;

    case 0x10: // op
      function = (ins>>5) & 0x7f;
      switch (function)
        {
        case 0x00: // ADDL
	  r[REG_3] = SEXT_32(r[REG_1] + V_2);
	  DEBUG_OP("ADDL");
	  return;
        case 0x02: // S4ADDL
	  r[REG_3] = SEXT_32((r[REG_1]*4) + V_2);
	  DEBUG_OP("S4ADDL");
	  return;
        case 0x09: // SUBL
	  r[REG_3] = SEXT_32(r[REG_1] - V_2);
	  DEBUG_OP("SUBL");
	  return;
        case 0x0b: // S4SUBL
	  r[REG_3] = SEXT_32((r[REG_1]*4) - V_2);
	  DEBUG_OP("S4SUBL");
	  return;
        case 0x0f:  // CMPBGE
	  r[REG_3] =   (((u8)( r[REG_1]     &0xff)>=(u8)( V_2      & 0xff))?  1:0)
	    | (((u8)((r[REG_1]>> 8)&0xff)>=(u8)((V_2>> 8) & 0xff))?  2:0)
	    | (((u8)((r[REG_1]>>16)&0xff)>=(u8)((V_2>>16) & 0xff))?  4:0)
	    | (((u8)((r[REG_1]>>24)&0xff)>=(u8)((V_2>>24) & 0xff))?  8:0)
	    | (((u8)((r[REG_1]>>32)&0xff)>=(u8)((V_2>>32) & 0xff))? 16:0)
	    | (((u8)((r[REG_1]>>40)&0xff)>=(u8)((V_2>>40) & 0xff))? 32:0)
	    | (((u8)((r[REG_1]>>48)&0xff)>=(u8)((V_2>>48) & 0xff))? 64:0)
	    | (((u8)((r[REG_1]>>56)&0xff)>=(u8)((V_2>>56) & 0xff))?128:0);
	  DEBUG_OP("CMPBGE");
	  return;
        case 0x12: // S8ADDL
	  r[REG_3] = SEXT_32((r[REG_1]*8) + V_2);
	  DEBUG_OP("S8ADDL");
	  return;
        case 0x1b: // S8SUBL
	  r[REG_3] = SEXT_32((r[REG_1]*8) - V_2);
	  DEBUG_OP("S8SUBL");
	  return;
        case 0x1d: // CMPULT
	  r[REG_3] = ((u64)r[REG_1]<(u64)V_2)?1:0;
	  DEBUG_OP("CMPULT");
	  return;
        case 0x20: // ADDQ
	  r[REG_3] = r[REG_1] + V_2;
	  DEBUG_OP("ADDQ");
	  return;
        case 0x22: // S4ADDQ
	  r[REG_3] = (r[REG_1]*4) + V_2;
	  DEBUG_OP("S4ADDQ");
	  return;
        case 0x29: // SUBQ
	  r[REG_3] = r[REG_1] - V_2;
	  DEBUG_OP("SUBQ");
	  return;
        case 0x2b: // S4SUBQ
	  r[REG_3] = (r[REG_1]*4) - V_2;
	  DEBUG_OP("S4SUBQ");
	  return;
        case 0x2d: // CMPEQ
	  r[REG_3] = (r[REG_1]==V_2)?1:0;
	  DEBUG_OP("CMPEQ");
	  return;
        case 0x32: // S8ADDQ
	  r[REG_3] = (r[REG_1]*8) + V_2;
	  DEBUG_OP("S8ADDQ");
	  return;
        case 0x3b: // S8SUBQ
	  r[REG_3] = (r[REG_1]*8) - V_2;
	  DEBUG_OP("S8SUBQ");
	  return;
        case 0x3d: // CMPULE
	  r[REG_3] = ((u64)r[REG_1]<=(u64)V_2)?1:0;
	  DEBUG_OP("CMPULE");
	  return;
        case 0x4d: // CMPLT
	  r[REG_3] = ((s64)r[REG_1]<(s64)V_2)?1:0;
	  DEBUG_OP("CMPLT");
	  return;
        case 0x6d: // CMPLE
	  r[REG_3] = ((s64)r[REG_1]<=(s64)V_2)?1:0;
	  DEBUG_OP("CMPLE");
	  return;
        default:
	  UNKNOWN2;
	  return;
        }
      break;

    case 0x11: // op
      function = (ins>>5) & 0x7f;
      switch (function)
        {
        case 0x00: // AND
	  r[REG_3] = r[REG_1] & V_2;
	  DEBUG_OP("AND");
	  return;
        case 0x08: // BIC
	  r[REG_3] = r[REG_1] & ~V_2;
	  DEBUG_OP("BIC");
	  return;
        case 0x14: // CMOVLBS
	  if (r[REG_1] & X64(1))
	    r[REG_3] = V_2;
	  DEBUG_OP("CMOVLBS");
	  return;
        case 0x16: // CMOVLBC
	  if (!(r[REG_1] & X64(1)))
	    r[REG_3] = V_2;
	  DEBUG_OP("CMOVLBC");
	  return;
        case 0x20: // BIS
	  r[REG_3] = r[REG_1] | V_2;
	  DEBUG_OP("BIS");
	  return;
        case 0x24: // CMOVEQ
	  if (!r[REG_1])
	    r[REG_3] = V_2;
	  DEBUG_OP("CMOVEQ");
	  return;
        case 0x26: // CMOVNE
	  if (r[REG_1])
	    r[REG_3] = V_2;
	  DEBUG_OP("CMOVNE");
	  return;
        case 0x28: // ORNOT
	  r[REG_3] = r[REG_1] | ~V_2;
	  DEBUG_OP("ORNOT");
	  return;
        case 0x40: // XOR
	  r[REG_3] = r[REG_1] ^ V_2;
	  DEBUG_OP("XOR");
	  return;
        case 0x44: // CMOVLT
	  if ((s64)r[REG_1]<0)
	    r[REG_3] = V_2;
	  DEBUG_OP("CMOVLT");
	  return;
        case 0x46: // CMOVGE
	  if ((s64)r[REG_1]>=0)
	    r[REG_3] = V_2;
	  DEBUG_OP("CMOVGE");
	  return;
        case 0x48: // EQV
	  r[REG_3] = r[REG_1] ^ ~V_2;
	  DEBUG_OP("EQV");
	  return;
        case 0x61: // AMASK
	  r[REG_3] = V_2 & ~CPU_AMASK; // BWX,CIX,MVI,trapPC,prefMod 
	  DEBUG_OP_R23("AMASK");
	  return;
        case 0x64: // CMOVLE
	  if ((s64)r[REG_1]<=0)
	    r[REG_3] = V_2;
	  DEBUG_OP("CMOVLE");
	  return;
        case 0x66: // CMOVGT
	  if ((s64)r[REG_1]>0)
	    r[REG_3] = V_2;
	  DEBUG_OP("CMOVGT");
	  return;
        case 0x6c: // IMPLVER
	  r[REG_3] = CPU_IMPLVER;
	  DEBUG_OP_R3("IMPLVER");
	  return;
        default:
	  UNKNOWN2;
	  return;
        }

    case 0x12:
      function = (ins>>5) & 0x7f;
      switch (function)
        {
        case 0x02: //MSKBL
	  r[REG_3] = r[REG_1] & ~(X64_BYTE<<((V_2&7)*8));
	  DEBUG_OP("MSKBL");
	  return;
        case 0x06: // EXTBL
	  r[REG_3] = (r[REG_1] >> ((V_2&7)*8)) & X64_BYTE;
	  DEBUG_OP("EXTBL");
	  return;
        case 0x0b: // INSBL
	  r[REG_3] = (r[REG_1] & X64_BYTE) << ((V_2&7)*8);
	  DEBUG_OP("INSBL");
	  return;
        case 0x12: // MSKWL
	  r[REG_3] = r[REG_1] & ~(X64_WORD<<((V_2&7)*8));
	  DEBUG_OP("MSKWL");
	  return;
        case 0x16: // EXTWL
	  r[REG_3] = (r[REG_1] >> ((V_2&7)*8))&X64_WORD;
	  DEBUG_OP("EXTWL");
	  return;
        case 0x1b: // INSWL
	  r[REG_3] = (r[REG_1]&X64_WORD) << ((V_2&7)*8);
	  DEBUG_OP("INSWL");
	  return;
        case 0x22: // MSKLL
	  r[REG_3] = r[REG_1] & ~(X64_LONG<<((V_2&7)*8));
	  DEBUG_OP("MSKLL");
	  return;
        case 0x26: // EXTLL
	  r[REG_3] = (r[REG_1] >> ((V_2&7)*8))&X64_LONG;
	  DEBUG_OP("EXTLL");
	  return;
        case 0x2b: // INSLL
	  r[REG_3] = (r[REG_1]&X64_LONG) << ((V_2&7)*8);
	  DEBUG_OP("INSLL");
	  return;
        case 0x32: // MSKQL
	  r[REG_3] = r[REG_1] & ~(X64_QUAD<<((V_2&7)*8));
	  DEBUG_OP("MSKQL");
	  return;
        case 0x36: // EXTQL
	  r[REG_3] = r[REG_1] >> ((V_2&7)*8);
	  DEBUG_OP("EXTQL");
	  return;
        case 0x3b: // INSQL
	  r[REG_3] = r[REG_1] << ((V_2&7)*8);
	  DEBUG_OP("INSQL");
	  return;
        case 0x30: // ZAP
	  r[REG_3] = r[REG_1] & (  ((V_2&  1)?0:              X64(ff))
				 | ((V_2&  2)?0:            X64(ff00))
				 | ((V_2&  4)?0:          X64(ff0000))
				 | ((V_2&  8)?0:        X64(ff000000))
				 | ((V_2& 16)?0:      X64(ff00000000))
				 | ((V_2& 32)?0:    X64(ff0000000000))
				 | ((V_2& 64)?0:  X64(ff000000000000))
				 | ((V_2&128)?0:X64(ff00000000000000)));
	  DEBUG_OP("ZAP");
	  return;
        case 0x31: // ZAPNOT
	  r[REG_3] = r[REG_1] & (  ((V_2&  1)?              X64(ff):0)
				 | ((V_2&  2)?            X64(ff00):0)
				 | ((V_2&  4)?          X64(ff0000):0)
				 | ((V_2&  8)?        X64(ff000000):0)
				 | ((V_2& 16)?      X64(ff00000000):0)
				 | ((V_2& 32)?    X64(ff0000000000):0)
				 | ((V_2& 64)?  X64(ff000000000000):0)
				 | ((V_2&128)?X64(ff00000000000000):0) );
	  DEBUG_OP("ZAPNOT");
	  return;
        case 0x34: // SRL
	  r[REG_3] = r[REG_1] >> (V_2 & 63);
	  DEBUG_OP("SRL");
	  return;
        case 0x39: // SLL
	  r[REG_3] = r[REG_1] << (V_2 & 63);
	  DEBUG_OP("SLL");
	  return;
        case 0x3c: // SRA
	  r[REG_3] = r[REG_1] >> (V_2 & 63) |
	    ((r[REG_1]>>63)?(X64_QUAD<<(64-(V_2 & 63))):0);
	  DEBUG_OP("SRA");
	  return;
        case 0x52: //MSKWH
	  r[REG_3] = r[REG_1] & ~(X64_WORD>>(64-((V_2 & 7)*8)));
	  DEBUG_OP("MSKWH");
	  return;
        case 0x57: // INSWH
	  r[REG_3] = (r[REG_1]&X64_WORD) >> (64-((V_2&7)*8));
	  DEBUG_OP("INSWH");
	  return;
        case 0x5a: // EXTWH
	  r[REG_3] = (r[REG_1] << (64-((V_2&7)*8))) & X64_WORD;
	  DEBUG_OP("EXTWH");
	  return;
        case 0x62: //MSKLH
	  r[REG_3] = r[REG_1] & ~(X64_LONG>>(64-((V_2&7)*8)));
	  DEBUG_OP("MSKLH");
	  return;
        case 0x67: // INSLH
	  r[REG_3] = (r[REG_1]&X64_LONG) >> (64-((V_2&7)*8));
	  DEBUG_OP("INSLH");
	  return;
        case 0x6a: // EXTLH
	  r[REG_3] = (r[REG_1] << (64-((V_2&7)*8))) & X64_LONG;
	  DEBUG_OP("EXTLH");
	  return;
        case 0x72: //MSKQH
	  r[REG_3] = r[REG_1] & ~(X64_QUAD>>(64-((V_2&7)*8)));
	  DEBUG_OP("MSKQH");
	  return;
        case 0x77: // INSQH
	  r[REG_3] = (r[REG_1]&X64_QUAD) >> (64-((V_2&7)*8));
	  DEBUG_OP("INSQH");
	  return;
        case 0x7a: // EXTQH
	  r[REG_3] = (r[REG_1] << (64-((V_2&7)*8))) & X64_QUAD;
	  DEBUG_OP("EXTQH");
	  return;
        default:
	  UNKNOWN2;
	  return;
        }

    case 0x13:
      function = (ins>>5) & 0x7f;
      switch (function)
        {
	case 0x00: // MULL
	  r[REG_3] = SEXT_32((u32)r[REG_1]*(u32)V_2);
	  DEBUG_OP("MULL");
	  return;
	case 0x20: // MULQ
	  r[REG_3] = r[REG_1]*V_2;
	  DEBUG_OP("MULQ");
	  return;
	case 0x30: // UMULH
	  temp_64 = 0;
	  temp_64_1 = r[REG_1];
	  temp_64_2 = V_2;
	  temp_64_x = 0;
	  temp_64_y = 0;
	  temp_64_z = 0;
	  for(i=0;i<64;i++)
	    {
	      if (temp_64_2&(X64(1)<<(u64)i))
		{
		  temp_64_y = temp_64_z;
		  temp_64_x = temp_64_1<<(u64)i;
		  temp_64_z += temp_64_x;

		  if (temp_64_z<temp_64_x || temp_64_z<temp_64_y) // overflow
		    temp_64++;

		  temp_64 += temp_64_1>>(X64(40)-(u64)i);
		}
	    }
	  r[REG_3] = temp_64;
	  DEBUG_OP("UMULH");
	  return;
	default:
	  UNKNOWN2;
	  return;
	}

    case 0x17:
      function = (ins>>5) & 0x7ff;
      switch (function)
	{
	case 0x24: //MT_FPCR
	  fpcr = f[FREG_1];
	  return;
	case 0x25: //MF_FPCR
	  f[FREG_1] = fpcr;
	  return;
	default:
	  UNKNOWN2;
	  return;
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
	  return;
	case 0xC000: // RPCC
	  r[REG_1] = ((u64)cc_offset)<<32
	    | cc;
	  DEBUG_OP_R1("RPCC");
	  return;
	case 0xE000: // RC
	  r[REG_1] = bIntrFlag?1:0;
	  bIntrFlag = false;
	  DEBUG_OP_R1("RC");
	  return;
	case 0xF000: // RS
	  r[REG_1] = bIntrFlag?1:0;
	  bIntrFlag = true;
	  DEBUG_OP_R1("RS");
	  return;
	default:
	  UNKNOWN2;
	  return;
	}

    case 0x19: // HW_MFPR
      function = (ins>>8) & 0xff;
      if ((function & 0xc0) == 0x40)
        {	// PCTX
	  r[REG_1] = ((u64)asn << 39)
	    | ((u64)astrr << 9)
	    | ((u64)aster <<5)
	    | (fpen?X64(1)<<3:0)
	    | (ppcen?X64(1)<<1:0);
	  DEBUG_MFPR("PCTX");
	  return;
        }
      switch (function)
        {
        case 0x05: // PMPC
	  r[REG_1] = pmpc;
	  DEBUG_MFPR("PMPC");
	  return;
        case 0x06: // EXC_ADDR
	  r[REG_1] = exc_addr;
	  DEBUG_MFPR("EXC_ADDR");
	  return;
        case 0x07: // IVA_FORM
	  r[REG_1] = va_form(exc_addr,i_ctl_va_mode,i_ctl_vptb);
	  DEBUG_MFPR("IVA_FORM");
	  return;
        case 0x08: // IER_CM
        case 0x09: // CM
        case 0x0a: // IER
        case 0x0b: // IER_CM
	  r[REG_1] = (((u64)eien) << 33)
	    | (((u64)slen) << 32)
	    | (((u64)cren) << 31)
	    | (((u64)pcen) << 29)
	    | (((u64)sien) << 14)
	    | (((u64)asten) << 13)
	    | (((u64)cm) << 3);
	  DEBUG_MFPR("IER_CM");
	  return;
        case 0x0c: // SIRR
	  r[REG_1] = ((u64)sir) << 14;
	  DEBUG_MFPR("SIRR");
	  return;
        case 0x0d: // ISUM
	  r[REG_1] = (((u64)(eir & eien)) << 33)
	    | (((u64)(slr & slen)) << 32)
	    | (((u64)(crr & cren)) << 31)
	    | (((u64)(pcr & pcen)) << 29)
	    | (((u64)(sir & sien)) << 14)
	    | (((u64)( ((X64(1)<<(cm+1))-1) & aster & astrr & (asten * 0x3))) << 3)
	    | (((u64)( ((X64(1)<<(cm+1))-1) & aster & astrr & (asten * 0xc))) << 7);
	  DEBUG_MFPR("ISUM");
	  return;
        case 0x0f: // EXC_SUM
	  r[REG_1] = exc_sum;
	  DEBUG_MFPR("EXC_SUM");
	  return;
        case 0x10: // PAL_BASE
	  r[REG_1] = pal_base;
	  DEBUG_MFPR("PAL_BASE");
	  return;
        case 0x11: // i_ctl
	  r[REG_1] = i_ctl_other
	    | (((u64)CPU_CHIP_ID)<<24)
	    | (u64)i_ctl_vptb
	    | (((u64)i_ctl_va_mode) << 15)
	    | (hwe?X64(1)<<12:0)
	    | (sde?X64(1)<<7:0)
	    | (((u64)i_ctl_spe) << 3);
	  DEBUG_MFPR("I_CTL");
	  return;
        case 0x14: // PCTR_CTL
	  r[REG_1] = pctr_ctl;
	  DEBUG_MFPR("PCTR_CTL");
	  return;
        case 0x16: // I_STAT
	  r[REG_1] = i_stat;
	  DEBUG_MFPR("I_STAT");
	  return;
        case 0x27: // MM_STAT
	  r[REG_1] = mm_stat;
	  DEBUG_MFPR("MM_STAT");
	  return;
        case 0x2a: // DC_STAT
	  r[REG_1] = dc_stat;
	  DEBUG_MFPR("DC_STAT");
	  return;
        case 0x2b: // C_DATA
	  r[REG_1] = 0;
	  DEBUG_MFPR("C_DATA");
	  return;
        case 0xc0: // CC
	  r[REG_1] = (((u64)cc_offset) << 32)
	    |  cc;
	  DEBUG_MFPR("CC");
	  return;
        case 0xc2: // VA
	  r[REG_1] = fault_va;
	  DEBUG_MFPR("VA");
	  return;
        case 0xc3: // VA_FORM
	  r[REG_1] = va_form(fault_va, va_ctl_va_mode, va_ctl_vptb);
	  DEBUG_MFPR("VA_FORM");
	  return;
        default:
	  UNKNOWN2;
	  return;
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
      return;

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
	  return;
        case 1: // quadword physical
	  if (DO_ACTION)
	    {
	      phys_address = r[REG_2] + DISP_12;
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","/Phys");
	  return;
        case 2: // longword physical locked
	  if (DO_ACTION)
	    {
	      lock_flag = true;
	      phys_address = r[REG_2] + DISP_12;
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","/Phys/Lock");
	  return;
        case 3: // quadword physical locked
	  if (DO_ACTION)
	    {
	      lock_flag = true;
	      phys_address = r[REG_2] + DISP_12;
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","/Phys/Lock");
	  return;
        case 4: // longword virtual vpte               //chk //alt  //vpte
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, false, true);
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","/Vpte");
	  return;
        case 5: // quadword virtual vpte               //chk //alt  //vpte
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, false, true);
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","/Vpte");
	  return;
        case 8: // longword virtual
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, false, false);
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","");
	  return;
        case 9: // quadword virtual
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, false, false);
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","");
	  return;
        case 10: // longword virtual check
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, false, false);
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","/Chk");
	  return;
        case 11: // quadword virtual check
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, false, false);
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","/Chk");
	  return;
        case 12: // longword virtual alt
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, true, false);
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","/Alt");
	  return;
        case 13: // quadword virtual alt
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, true, false);
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","/Alt");
	  return;
        case 14: // longword virtual alt check
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, true, false);
	      r[REG_1] = READ_PHYS_NT(32);
	    }
	  DEBUG_HW("HW_LDL","/Alt/Chk");
	  return;
        case 15: // quadword virtual alt check
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, true, false);
	      r[REG_1] = READ_PHYS_NT(64);
	    }
	  DEBUG_HW("HW_LDQ","/Alt/Chk");
	  return;
        default:
	  UNKNOWN2;
	  return;
        }
    
    case 0x1c: // op
      function = (ins>>5) & 0x7f;
      switch (function)
        {
        case 0x00: //SEXTB
	  r[REG_3] = SEXT_8(V_2);
	  DEBUG_OP_R23("SEXTB");
	  return;
        case 0x01: // SEXTW
	  r[REG_3] = SEXT_16(V_2);
	  DEBUG_OP_R23("SEXTW");
	  return;
        case 0x30: // CTPOP
	  temp_64 = 0;
	  temp_64_2 = V_2;
	  for (i=0;i<64;i++)
	    if ((temp_64_2>>i)&1)
	      temp_64++;
	  r[REG_3] = temp_64;
	  DEBUG_OP_R23("CTPOP");
	  return;
        case 0x31: // PERR
	  temp_64 = 0;
	  temp_64_1 = r[REG_1];
	  temp_64_2 = V_2;
	  for(i=0;i<64;i+=8)
	    if ((s8)((temp_64_1>>i)&X64_BYTE) > (s8)((temp_64_2>>i)&X64_BYTE))
	      temp_64 |=    ((u64)((s8)((temp_64_1>>i)&X64_BYTE) - (s8)((temp_64_2>>i)&X64_BYTE))<<i);
	    else
	      temp_64 |=    ((u64)((s8)((temp_64_2>>i)&X64_BYTE) - (s8)((temp_64_1>>i)&X64_BYTE))<<i);
	  r[REG_3] = temp_64;
	  DEBUG_OP("PERR");
	  return;
        case 0x32: // CTLZ
	  temp_64 = 0;
	  temp_64_2 = V_2;
	  for (i=63;i>=0;i--)
	    if ((temp_64>>i)&1)
	      break;
	    else
	      temp_64++;
	  r[REG_3] = temp_64;
	  DEBUG_OP_R23("CTLZ");
	  return;
        case 0x33: // CTTZ
	  temp_64 = 0;
	  temp_64_2 = V_2;
	  for (i=0;i<64;i++)
	    if ((temp_64>>i)&1)
	      break;
	    else
	      temp_64++;
	  r[REG_3] = temp_64;
	  DEBUG_OP_R23("CTLZ");
	  return;
        case 0x34: // UNPKBW
	  temp_64_2 = V_2;
	  r[REG_3] =  (temp_64_2 & X64(000000ff))
	    | ((temp_64_2 & X64(0000ff00)) << 8)
	    | ((temp_64_2 & X64(00ff0000)) << 16)
	    | ((temp_64_2 & X64(ff000000)) << 24);
	  DEBUG_OP_R23("UNPKBW");
	  return;
        case 0x35: // UNPKBL
	  temp_64_2 = V_2;
	  r[REG_3] =  (temp_64_2 & X64(000000ff))
	    | ((temp_64_2 & X64(0000ff00)) << 24);
	  DEBUG_OP_R23("UNPKBL");
	  return;
        case 0x36: // PKBW
	  temp_64_2 = V_2;
	  r[REG_3] =  (temp_64_2 & X64(00000000000000ff))
	    | ((temp_64_2 & X64(0000000000ff0000)) >> 8)
	    | ((temp_64_2 & X64(000000ff00000000)) >> 16)
	    | ((temp_64_2 & X64(00ff000000000000)) >> 24);
	  DEBUG_OP_R23("PKBW");
	  return;
        case 0x37: // PKBL
	  temp_64_2 = V_2;
	  r[REG_3] =  (temp_64_2 & X64(00000000000000ff))
	    | ((temp_64_2 & X64(000000ff00000000)) >> 24);
	  DEBUG_OP_R23("PKBL");
	  return;
        case 0x38: // MINSB8
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
	  DEBUG_OP("MINSB8");
	  return;
        case 0x39: // MINSW4
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
	  DEBUG_OP("MINSW4");
	  return;
        case 0x3a: // MINUB8
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
	  DEBUG_OP("MINUB8");
	  return;
        case 0x3b: // MINUW4
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
	  DEBUG_OP("MINUW4");
	  return;
        case 0x3c: // MAXUB8
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
	  DEBUG_OP("MAXUB8");
	  return;
        case 0x3d: // MAXUW4
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
	  DEBUG_OP("MAXUW4");
	  return;
        case 0x3e: // MAXSB8
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
	  DEBUG_OP("MAXSB8");
	  return;
        case 0x3f: // MAXSW4
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
	  DEBUG_OP("MAXSW4");
	  return;
        case 0x70: // FTOIT
	  r[REG_3] = f[FREG_1];
	  DEBUG_OP_F1_R3("FTOIT");
	  return;

        case 0x78: // FTOIS
	  temp_64 = f[FREG_1];
	  r[REG_3] = (temp_64 & X64(000000003fffffff))
	    |((temp_64 & X64(c000000000000000)) >> 32)
	    |(((temp_64 & X64(8000000000000000)) >>31) * X64(ffffffff));
	  DEBUG_OP_F1_R3("FTOIS");
	  return;
        default:
	  UNKNOWN2;
	  return;
        }

    case 0x1d: // HW_MTPR
      function = (ins>>8) & 0xff;
      if ((function & 0xc0) == 0x40)
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
	  return;
        }
      switch (function)
        {
        case 0x00: // ITB_TAG
	  itb->write_tag(0,r[REG_2]);
	  DEBUG_MTPR("ITB_TAG");
	  return;
        case 0x01: // ITB_PTE
	  itb->write_pte(0,r[REG_2],get_asn());
	  DEBUG_MTPR("ITB_PTE");
	  return;
        case 0x02: // ITB_IAP
	  itb->InvalidateAllProcess();
	  DEBUG_MTPR("ITB_IAP");
	  return;
        case 0x03: // ITB_IA
	  itb->InvalidateAll();
	  DEBUG_MTPR("ITB_IA");
	  return;
        case 0x04: // ITB_IS
	  itb->InvalidateSingle(r[REG_2],get_asn());
	  DEBUG_MTPR("ITB_IS");
	  return;
        case 0x09: // CM
	  cm = (int)(r[REG_2]>>3) & 3;
	  DEBUG_MTPR("CM");
	  return;
        case 0x0b: // IER_CM
	  cm = (int)(r[REG_2]>>3) & 3;
        case 0x0a: // IER
	  asten = (int)(r[REG_2]>>13) & 1;
	  sien  = (int)(r[REG_2]>>14) & 0x3fff;
	  pcen  = (int)(r[REG_2]>>29) & 3;
	  cren  = (int)(r[REG_2]>>31) & 1;
	  slen  = (int)(r[REG_2]>>32) & 1;
	  eien  = (int)(r[REG_2]>>33) & 0x3f;
	  ////////////////////////////////
	  //printf("eien: 0x%x\n",eien);
	  //scanf("\n");
	  ////////////////////////////////
	  DEBUG_MTPR("IER[_CM]");
	  return;
        case 0x0c: // SIRR
	  sir = (int)(r[REG_2]>>14) & 0x3fff;
	  DEBUG_MTPR("SIRR");
	  return;
        case 0x0e: // HW_INT_CLR
	  pcr &= ~((r[REG_2]>>29)&X64(3));
	  crr &= ~((r[REG_2]>>31)&X64(1));
	  slr &= ~((r[REG_2]>>32)&X64(1));
	  DEBUG_MTPR("HW_INT_CLT");
	  return;
        case 0x10: // PAL_BASE
	  pal_base = r[REG_2] & X64(00000fffffff8000);
	  DEBUG_MTPR("PAL_BASE");
	  return;
        case 0x11: // i_ctl
	  i_ctl_other = r[REG_2]    & X64(00000000007e2f67);
	  i_ctl_vptb  =  (r[REG_2] & X64(0000ffffc0000000))
	    | ((r[REG_2] & X64(0000800000000000)) * X64(1fffe)); // SEXT
	  i_ctl_spe   = (int)(r[REG_2]>>3) & 3;
	  sde         = (r[REG_2]>>7) & 1;
	  hwe         = (r[REG_2]>>12) & 1;
	  i_ctl_va_mode = (int)(r[REG_2]>>15) & 3;
	  DEBUG_MTPR("I_CTL");
	  return;
        case 0x12: // ic_flush_asm
	  flush_icache();
	  DEBUG_MTPR("IC_FLUSH_ASM");
	  return;
        case 0x13: // IC_FLUSH
	  flush_icache();
	  DEBUG_MTPR("IC_FLUSH");
	  return;
        case 0x14: // PCTR_CTL
	  pctr_ctl = r[REG_2] & X64(ffffffffffffffdf);
	  DEBUG_MTPR("PCTR_CTL");
	  return;
        case 0x15: // CLR_MAP
	  // NOP
	  DEBUG_MTPR("CLR_MAP");
	  return;
        case 0x16: // I_STAT
	  i_stat &= ~r[REG_2]; //W1C
	  DEBUG_MTPR("I_STAT");
	  return;
        case 0x17: // SLEEP
	  // NOP
	  DEBUG_MTPR("SLEEP");
	  return;
        case 0x20: // DTB_TAG0
	  dtb->write_tag(0,r[REG_2]);
	  DEBUG_MTPR("DTB_TAG0");
	  return;
        case 0x21: // DTB_PTE0
	  dtb->write_pte(0,r[REG_2],get_asn());
	  DEBUG_MTPR("DTB_PTE0");
	  return;
        case 0x24: // DTB_IS0
	  dtb->InvalidateSingle(r[REG_2],get_asn());
	  DEBUG_MTPR("DTB_IS0");
	  return;
        case 0x25: // DTB_ASN0
	  asn0 = (int)(r[REG_2] >> 56);
	  DEBUG_MTPR("DTB_ASN0");
	  return;
        case 0x26: // DTB_ALTMODE
	  alt_cm = (int)(r[REG_2] & 3);
	  DEBUG_MTPR("DTB_ALTMODE");
	  return;
        case 0x27: // MM_STAT
	  DEBUG_MTPR("MM_STAT");
	  return;
        case 0x28: // M_CTL
	  smc = (int)(r[REG_2]>>4) & 3;
	  m_ctl_spe = (int)(r[REG_2]>>1) & 7;
	  DEBUG_MTPR("M_CTL");
	  return;
        case 0x29: // DC_CTL
	  dc_ctl = r[REG_2];
	  DEBUG_MTPR("DC_CTL");
	  return;
        case 0x2a: // DC_STAT
	  dc_stat &= ~r[REG_2];
	  DEBUG_MTPR("DC_STAT");
	  return;
        case 0x2b: // C_DATA
	  DEBUG_MTPR("C_DATA");
	  return;
        case 0x2c: // C_SHIFT
	  DEBUG_MTPR("C_SHIFT");
	  return;
        case 0xa0: // DTB_TAG1
	  dtb->write_tag(1,r[REG_2]);
	  DEBUG_MTPR("DTB_TAG1");
	  return;
        case 0xa1: // DTB_PTE1
	  dtb->write_pte(1,r[REG_2],get_asn());
	  DEBUG_MTPR("DTB_PTE1");
	  return;
        case 0xa2: // DTB_IAP
	  dtb->InvalidateAllProcess();
	  DEBUG_MTPR("DTB_IAP");
	  return;
        case 0xa3: // DTB_IA
	  dtb->InvalidateAll();
	  DEBUG_MTPR("DTB_IA");
	  return;
        case 0xa4: // DTB_IS1
	  dtb->InvalidateSingle(r[REG_2],get_asn());
	  DEBUG_MTPR("DTB_IA1");
	  return;
        case 0xa5: // DTB_ASN1
	  asn1 = (int)(r[REG_2] >> 56);
	  DEBUG_MTPR("DTB_ASN1");
	  return;
        case 0xc0: // CC
	  cc_offset = (u32)(r[REG_2] >> 32);
	  DEBUG_MTPR("CC");
	  return;
        case 0xc1: // CC_CTL
	  cc_ena = (r[REG_2] >> 32) & 1;
	  cc    = (u32)(r[REG_2] & X64(fffffff0));
	  DEBUG_MTPR("CC_CTL");
	  return;
        case 0xc4: // VA_CTL
	  va_ctl_vptb  =  (r[REG_2] & X64(0000ffffc0000000))
	    | ((r[REG_2] & X64(0000800000000000)) * X64(1fffe)); // SEXT
	  i_ctl_va_mode = (int)(r[REG_2]>>1) & 3;
	  DEBUG_MTPR("VA_CTL");
	  return;
        default:
	  UNKNOWN2;
	  return;
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
      return;

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
	  return;
        case 1: // quadword physical
	  if (DO_ACTION)
	    {
	      phys_address = r[REG_2] + DISP_12;
	      WRITE_PHYS_NT(r[REG_1],64);
	    }
	  DEBUG_HW("HW_STQ","/Phys");
	  return;
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
	  return;
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
	  return;
        case 4: // longword virtual                   //chk //alt  //vpte
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, false, false);
	      WRITE_PHYS_NT(r[REG_1],32);
	    }
	  DEBUG_HW("HW_STL","");
	  return;
        case 5: // quadword virtual                    //chk //alt  //vpte
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, false, false);
	      WRITE_PHYS_NT(r[REG_1],64);
	    }
	  DEBUG_HW("HW_STQ","");
	  return;
        case 12: // longword virtual alt
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, true, false);
	      WRITE_PHYS_NT(r[REG_1],32);
	    }
	  DEBUG_HW("HW_STL","/Alt");
	  return;
        case 13: // quadword virtual alt
	  if (DO_ACTION)
	    {
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, true, false);
	      WRITE_PHYS_NT(r[REG_1],64);
	    }
	  DEBUG_HW("HW_STQ","/Alt");
	  return;
        default:
	  UNKNOWN2;
	  return;
        }

    case 0x28: // LDL
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);
	  r[REG_1] = SEXT_32(READ_PHYS(32));
	}
      DEBUG_LD_ST("LDL");
      return;

    case 0x29: // LDQ
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);
	  r[REG_1] = READ_PHYS(64);
	}
      DEBUG_LD_ST("LDQ");
      return;

    case 0x2a: // LDL_L
      if (DO_ACTION)
	{
	  lock_flag = true;
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);
	  r[REG_1] = SEXT_32(READ_PHYS(32));
	}
      DEBUG_LD_ST("LDL_L");
      return;

    case 0x2b: // LDQ_L
      if (DO_ACTION)
	{
	  lock_flag = true;
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);
	  r[REG_1] = READ_PHYS(64);
	}
      DEBUG_LD_ST("LDQ_L");
      return;

    case 0x2c: // STL
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);
	  WRITE_PHYS(r[REG_1],32);
	}
      DEBUG_LD_ST("STL");
      return;

    case 0x2d: // STQ
      if (DO_ACTION)
	{
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);
	  WRITE_PHYS(r[REG_1],64);
	}
      DEBUG_LD_ST("STQ");
      return;

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
      return;

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
      return;

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
      return;

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
      return;

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
      return;

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
      return;

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
      return;

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
      return;

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
      return;

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
      return;

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
      return;

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
      return;

    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    default:
      UNKNOWN1;
      return;
    }
}

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

u64 CAlphaCPU::get_r(int i, bool translate)
{
  if (translate)
    return r[RREG(i)];
  else
    return r[i];
}

u64 CAlphaCPU::get_prbr(void)
{
  if (r[21+32] && (   (r[21+32]+0xa8)< (128*1024*1024)))
    {
      return cSystem->ReadMem(r[21+32] + 0xa8,64);
    }
  else
    return cSystem->ReadMem(0x70a8 + (0x200 * get_cpuid()),64);
}
