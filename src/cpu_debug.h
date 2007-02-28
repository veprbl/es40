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
 * Contains debugging macros used by AlphaCPU.cpp
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

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
  "M_CTL",	"DC_CTL",	"DC_STAT",	"C_DATA",	"C_SHFT",	"?0010.1101?",	"?0010.1110?",	"?0010.1111?",
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


#define TRC_(down,up,x,y) {						\
    if (bTrace)								\
      trc->trace(this, current_pc,pc,down,up,x,y); }

#define TRC(down,up) {							\
    if (bTrace)								\
      trc->trace(this, current_pc,pc,down,up,(char*)0,0); }

#define TRC_BR {							\
    if (bTrace) trc->trace_br(this, current_pc,pc); }

#define GO_PAL(offset) {						\
    exc_addr = current_pc;						\
    pc =  pal_base | offset | 1;					\
    if ((offset==DTBM_SINGLE || offset==ITB_MISS) && bTrace)		\
      trc->set_waitfor(this, exc_addr&~X64(3));				\
    else								\
      TRC_(true,false,"GO_PAL %04x",offset); }

#else

#define TRC_(down,up,x,y) ;
#define TRC(down,up) ;
#define TRC_BR ;
#define GO_PAL(offset) {						\
    exc_addr = current_pc;						\
    pc = pal_base | offset | 1; }

#endif

#if defined(IDB)

#define DEBUG_XX							\
  if (trc->get_fnc_name(current_pc&~X64(3),&funcname))			\
    {									\
      if (bListing && !strcmp(funcname,""))				\
        {								\
	  printf("%08x: \"%s\"\n",(u32)current_pc,			\
		 cSystem->PtrToMem(current_pc));		        \
	  pc = (current_pc + strlen(cSystem->PtrToMem(current_pc)) + 4)	\
	    & ~X64(3);							\
	  while (pc < 0x600000 && cSystem->ReadMem(pc,32)==0) pc += 4;	\
	  return 0;							\
        }								\
      else if (bListing && !strcmp(funcname,"!SKIP"))			\
        {								\
	  while (pc < 0x600000 && cSystem->ReadMem(pc,32)==0) pc += 4;	\
	  return 0;							\
        }								\
      else if (bListing && !strncmp(funcname,"!CHAR-",6))		\
        {								\
	  u64 xx_upto;							\
	  int xx_result;						\
	  xx_result = sscanf(&(funcname[6]),"%" LL "x",&xx_upto);	\
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
	      return 0;							\
	    }								\
        }								\
      else if (bListing && !strncmp(funcname,"!LCHAR-",7))		\
        {								\
	  char stringval[300];						\
	  int  stringlen;						\
	  u64 xx_upto;							\
	  int xx_result;						\
	  xx_result = sscanf(&(funcname[7]),"%" LL "x",&xx_upto);	\
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
	      return 0;							\
	    }								\
        }								\
      else if (bListing && !strncmp(funcname,"!X64-",5))		\
        {								\
	  printf("\n%s:\n",&(funcname[5]));				\
	  pc = current_pc;						\
	  while (   (pc==current_pc)					\
		    || !trc->get_fnc_name(pc,&funcname) )		\
	    {								\
              printf("%08x: %016" LL "x\n",(u32)pc, cSystem->ReadMem(pc,64)); \
	      pc += 8;							\
	    }								\
	  return 0;							\
        }								\
      else if (bListing&& !strncmp(funcname,"!X32-",5))			\
        {								\
	  printf("\n%s:\n",&(funcname[5]));				\
	  pc = current_pc;						\
	  while (   (pc==current_pc)					\
		    || !trc->get_fnc_name(pc,&funcname) )		\
	    {								\
	      printf("%08x: %08" LL "x\n",(u32)pc, cSystem->ReadMem(pc,32)); \
	      pc += 4;							\
	    }								\
	  return 0;							\
        }								\
      else if (bListing && !strncmp(funcname,":",1)) {			\
	sprintf(dbg_strptr,"%s:\n",funcname);				\
	dbg_strptr += strlen(dbg_strptr);				\
      } else {								\
	sprintf(dbg_strptr,"\n%s:\n",funcname);				\
	dbg_strptr += strlen(dbg_strptr);				\
      }                                                                 \
    }									\
  sprintf(dbg_strptr,"%08x: ", (u32)current_pc);			\
  dbg_strptr += strlen(dbg_strptr);					\
  if (bListing) {							\
    sprintf(dbg_strptr,"%08x %c%c%c%c: ", (u32)ins,			\
	   printable((char)(ins)),     printable((char)(ins>>8)),	\
	   printable((char)(ins>>16)), printable((char)(ins>>24)));	\
    dbg_strptr += strlen(dbg_strptr);					\
  }

#define UNKNOWN1 							\
  if (bDisassemble) {							\
    DEBUG_XX								\
  }									\
  sprintf(dbg_strptr,"Unknown opcode: %02x", opcode);			\
  dbg_strptr += strlen(dbg_strptr);					\
  handle_debug_string(dbg_string);					\
  return 0;

#define UNKNOWN2							\
  if (bDisassemble) {							\
    DEBUG_XX								\
  }									\
  sprintf(dbg_strptr,"Unknown opcode: %02x.%02x", opcode, function);	\
  dbg_strptr += strlen(dbg_strptr);					\
  handle_debug_string(dbg_string);					\
  return 0;

#define POST_X64(a)							\
  if (bDisassemble) {							\
    if (!bListing) {							\
      sprintf(dbg_strptr, " ==> %" LL "x", a);				\
      dbg_strptr += strlen(dbg_strptr);					\
    }									\
  }

#define PRE_PAL(mnemonic)						\
  if (bDisassemble) {							\
      DEBUG_XX;								\
      if (function < 0x40 || ( function > 0x7f && function < 0xc0))	\
        sprintf(dbg_strptr, #mnemonic " %s", PAL_NAME[function]);	\
      else								\
        sprintf(dbg_strptr,#mnemonic " ?%x?", function);		\
      dbg_strptr += strlen(dbg_strptr);					\
    }

#define POST_PAL							\
  TRC(true, false);						


#define PRE_BR(mnemonic)						\
  if (bDisassemble) {							\
      u64 dbg_x = (current_pc + 4 + (DISP_21 * 4))&~X64(3);		\
      DEBUG_XX								\
      sprintf(dbg_strptr,#mnemonic " r%d, ", REG_1&31);			\
      dbg_strptr += strlen(dbg_strptr);					\
      if (trc->get_fnc_name(dbg_x,&funcname))				\
	sprintf(dbg_strptr,"%s",funcname);				\
      else								\
	sprintf (dbg_strptr,"%" LL "x", dbg_x);				\
      dbg_strptr += strlen(dbg_strptr);					\
    }

#define POST_BR								\
  TRC_BR;

#define PRE_COND(mnemonic)						\
  if (bDisassemble) {							\
      u64 dbg_x = (current_pc + 4 + (DISP_21 * 4))&~X64(3);		\
      DEBUG_XX								\
      sprintf(dbg_strptr,#mnemonic " r%d, ", REG_1&31);			\
      dbg_strptr += strlen(dbg_strptr);					\
      if (trc->get_fnc_name(dbg_x,&funcname))				\
	sprintf(dbg_strptr,"%s",funcname);				\
      else								\
	sprintf (dbg_strptr,"%" LL "x", dbg_x);				\
      dbg_strptr += strlen(dbg_strptr);					\
      if (!bListing) {							\
        sprintf(dbg_strptr,": (%" LL "x)", r[REG_1]);			\
        dbg_strptr += strlen(dbg_strptr);				\
      }									\
    }

#define POST_COND							\
  TRC_BR;

#define PRE_BSR(mnemonic)						\
  if (bDisassemble) {							\
      u64 dbg_x = (current_pc + 4 + (DISP_21 * 4))&~X64(3);		\
      DEBUG_XX								\
      sprintf(dbg_strptr,#mnemonic " r%d, ", REG_1&31);			\
      dbg_strptr += strlen(dbg_strptr);					\
      if (trc->get_fnc_name(dbg_x,&funcname))				\
	sprintf(dbg_strptr,"%s",funcname);				\
      else								\
	sprintf (dbg_strptr,"%" LL "x", dbg_x);				\
      dbg_strptr += strlen(dbg_strptr);					\
    }

#define POST_BSR							\
  if (REG_1==31) {							\
    TRC(0,1);								\
  } else {								\
    TRC(1,1);								\
  }

#define PRE_JMP(mnemonic)						\
  if (bDisassemble)							\
    {									\
      DEBUG_XX								\
      sprintf(dbg_strptr,#mnemonic " r%d, r%d", REG_1&31, REG_2&31);	\
      dbg_strptr += strlen(dbg_strptr);					\
      if (!bListing) { 							\
        sprintf(dbg_strptr,": (%" LL "x)", r[REG_2]);			\
        dbg_strptr += strlen(dbg_strptr);				\
      }									\
    }

#define POST_JMP							\
  if (REG_1==31) {							\
    TRC(0,1);								\
  } else {								\
    TRC(1,1);								\
  }

#define PRE_RET(mnemonic)						\
  if (bDisassemble)							\
    {									\
      DEBUG_XX								\
      sprintf(dbg_strptr,#mnemonic " r%d", REG_2&31);			\
      dbg_strptr += strlen(dbg_strptr);					\
      if (!bListing) {							\
        sprintf(dbg_strptr,": (%" LL "x)", r[REG_2]);			\
        dbg_strptr += strlen(dbg_strptr);				\
      }									\
    }

#define POST_RET							\
    TRC(0,1);

#define PRE_MFPR(mnemonic)						\
  if (bDisassemble) {							\
      DEBUG_XX;								\
      sprintf(dbg_strptr,#mnemonic " r%d, %s", REG_1&31, IPR_NAME[function]);	\
      dbg_strptr += strlen(dbg_strptr);					\
    }

#define POST_MFPR							\
  POST_X64(r[REG_1]);

#define PRE_MTPR(mnemonic)						\
  if (bDisassemble) {							\
      DEBUG_XX;								\
      sprintf(dbg_strptr,#mnemonic " r%d, %s", REG_2&31, IPR_NAME[function]);	\
      dbg_strptr += strlen(dbg_strptr);					\
    }

#define POST_MTPR							\
  POST_X64(r[REG_2]);

#define PRE_NOP(mnemonic)						\
  if (bDisassemble) {							\
      DEBUG_XX;								\
      sprintf(dbg_strptr,#mnemonic "");					\
      dbg_strptr += strlen(dbg_strptr);					\
    }

#define POST_NOP ;

#define PRE_MEM(mnemonic)						\
  if (bDisassemble) {							\
      DEBUG_XX;								\
      sprintf(dbg_strptr, #mnemonic " r%d, %04xH(r%d)", REG_1&31, (u32)DISP_16, REG_2&31);	\
      dbg_strptr += strlen(dbg_strptr);					\
      if (!bListing) {							\
        sprintf(dbg_strptr,": (%" LL "x)", r[REG_2]);			\
        dbg_strptr += strlen(dbg_strptr);				\
      }									\
    }	

#define POST_MEM							\
  POST_X64(r[REG_1]);

#define PRE_R12_R3(mnemonic)						\
  if (bDisassemble)							\
    {									\
      DEBUG_XX;								\
      sprintf(dbg_strptr,#mnemonic " r%d, ", REG_1&31);			\
      dbg_strptr += strlen(dbg_strptr);					\
      if (ins&0x1000)							\
        sprintf(dbg_strptr,"%02xH",V_2);				\
      else								\
	sprintf(dbg_strptr,"r%d",REG_2&31);				\
      dbg_strptr += strlen(dbg_strptr);					\
      sprintf(dbg_strptr,", r%d", REG_3&31);				\
      dbg_strptr += strlen(dbg_strptr);					\
      if (!bListing) {							\
        sprintf(dbg_strptr,": (%" LL "x,%" LL "x)",r[REG_1],V_2);	\
        dbg_strptr += strlen(dbg_strptr);				\
      }									\
    }

#define POST_R12_R3							\
	POST_X64(r[REG_3]);
	
#define PRE_R1_F3(mnemonic)						\
  if (bDisassemble)							\
    {									\
      DEBUG_XX;								\
      sprintf(dbg_strptr,#mnemonic " r%d, f%d ", REG_1&31, FREG_3);	\
      dbg_strptr += strlen(dbg_strptr);					\
      if (!bListing) {							\
        sprintf(dbg_strptr,": (%" LL "x)",r[REG_1]);			\
        dbg_strptr += strlen(dbg_strptr);				\
      }									\
    }

#define POST_R1_F3							\
	POST_X64(f[FREG_3]);

#define PRE_F1_R3(mnemonic)						\
  if (bDisassemble)							\
    {									\
      DEBUG_XX;								\
      sprintf(dbg_strptr,#mnemonic " f%d, r%d ", FREG_1, REG_3&31);	\
      dbg_strptr += strlen(dbg_strptr);					\
      if (!bListing) {							\
        sprintf(dbg_strptr,": (%" LL "x)",f[FREG_1]);			\
        dbg_strptr += strlen(dbg_strptr);				\
      }									\
    }

#define POST_F1_R3							\
	POST_X64(r[REG_3]);

#define PRE_X_F1(mnemonic)						\
  if (bDisassemble)							\
    {									\
      DEBUG_XX;								\
      sprintf(dbg_strptr,#mnemonic " f%d ", FREG_1&31);			\
      dbg_strptr += strlen(dbg_strptr);					\
    }

#define POST_X_F1							\
	POST_X64(f[FREG_1]);
	
#define PRE_R2_R3(mnemonic)						\
  if (bDisassemble)							\
    {									\
      DEBUG_XX;								\
      sprintf(dbg_strptr,#mnemonic " ");				\
     dbg_strptr += strlen(dbg_strptr);					\
      if (ins&0x1000)							\
	sprintf(dbg_strptr,"%02xH",V_2);				\
      else								\
	sprintf(dbg_strptr,"r%d",REG_2&31);				\
      dbg_strptr += strlen(dbg_strptr);					\
      sprintf(dbg_strptr,", r%d", REG_3&31);				\
      dbg_strptr += strlen(dbg_strptr);					\
      if (!bListing) {							\
        sprintf(dbg_strptr,": (%" LL "x)",V_2);				\
        dbg_strptr += strlen(dbg_strptr);				\
      }									\
    }

#define POST_R2_R3							\
	POST_X64(r[REG_3]);
	
#define PRE_X_R1(mnemonic)						\
  if (bDisassemble)							\
    {									\
        DEBUG_XX;							\
	sprintf(dbg_strptr,#mnemonic " r%d", REG_1&31);			\
        dbg_strptr += strlen(dbg_strptr);				\
    }

#define POST_X_R1							\
	POST_X64(r[REG_1]);

#define PRE_X_R3(mnemonic)						\
  if (bDisassemble)							\
    {									\
        DEBUG_XX;							\
	sprintf(dbg_strptr,#mnemonic " r%d", REG_3&31);			\
        dbg_strptr += strlen(dbg_strptr);				\
    }

#define POST_X_R3							\
	POST_X64(r[REG_3]);
	
#define PRE_HW_LD(mnemonic)						\
  if (bDisassemble)							\
  {									\
      DEBUG_XX;								\
      sprintf(dbg_strptr,#mnemonic);					\
      dbg_strptr += strlen(dbg_strptr);					\
      switch(function&~1)						\
        {								\
        case 0:								\
	  sprintf(dbg_strptr,"/Phys");					\
	  break;							\
        case 2:								\
	  sprintf(dbg_strptr,"/Phys/Lock");				\
	  break;							\
        case 4:								\
	  sprintf(dbg_strptr,"/Vpte");					\
	  break;							\
        case 10:							\
	  sprintf(dbg_strptr,"/Chk");					\
	  break;							\
        case 12:							\
	  sprintf(dbg_strptr,"/Alt");					\
	  break;							\
        case 14:							\
	  sprintf(dbg_strptr,"/Alt/Chk");				\
	  break;							\
        }								\
      dbg_strptr += strlen(dbg_strptr);					\
      sprintf(dbg_strptr," r%d, %04xH(r%d)", REG_1&31, (u32)DISP_16, REG_2&31);	\
      dbg_strptr += strlen(dbg_strptr);					\
      if (!bListing) { 							\
        sprintf(dbg_strptr,": (%" LL "x)", r[REG_2]);			\
        dbg_strptr += strlen(dbg_strptr);				\
      }									\
    }	

#define POST_HW_LD							\
  POST_X64(r[REG_1]);

#define PRE_HW_ST(mnemonic)						\
  if (bDisassemble)							\
  {									\
      DEBUG_XX;								\
      sprintf(dbg_strptr,#mnemonic);					\
      dbg_strptr += strlen(dbg_strptr);					\
      switch(function&~1)						\
        {								\
        case 0:								\
	  sprintf(dbg_strptr,"/Phys");					\
	  break;							\
        case 2:								\
	  sprintf(dbg_strptr,"/Phys/Cond");				\
	  break;							\
        case 12:							\
	  sprintf(dbg_strptr,"/Alt");					\
	  break;							\
        }								\
      dbg_strptr += strlen(dbg_strptr);					\
      sprintf(dbg_strptr," r%d, %04xH(r%d)", REG_1&31, (u32)DISP_16, REG_2&31);	\
      dbg_strptr += strlen(dbg_strptr);					\
      if (!bListing) {							\
        sprintf(dbg_strptr,": (%" LL "x)", r[REG_2]);			\
        dbg_strptr += strlen(dbg_strptr);				\
      }									\
    }	

#define POST_HW_ST							\
  POST_X64(r[REG_1]);

#else

#define UNKNOWN1							\
      printf("Unknown opcode: %02x\n", opcode);				\
      return 0;		

#define UNKNOWN2							\
      printf("Unknown opcode: %02x.%02x\n", opcode, function);		\
      return 0;	

#endif
