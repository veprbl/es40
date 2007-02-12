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
