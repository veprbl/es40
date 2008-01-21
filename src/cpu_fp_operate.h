/* ES40 emulator.
 * Copyright (C) 2007-2008 by the ES40 Emulator Project
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
 * Contains code macros for the processor floating-point operate instructions.
 * Based on ARM chapter 4.10.
 *
 * $Id: cpu_fp_operate.h,v 1.12 2008/01/21 22:39:25 iamcamiel Exp $
 *
 * X-1.12       Camiel Vanderhoeven                             21-JAN-2008
 *      Implement new floating-point code for most operations.
 *
 * X-1.11       Camiel Vanderhoeven                             18-JAN-2008
 *      Replaced sext_64 inlines with sext_u64_<bits> inlines for
 *      performance reasons (thanks to David Hittner for spotting this!);
 *
 * X-1.10       Camiel Vanderhoeven                             2-DEC-2007
 *      Use sext_64 inline. 
 *
 * X-1.9        Camiel Vanderhoeven                             16-NOV-2007
 *      Avoid more compiler warnings.
 *
 * X-1.8        Camiel Vanderhoeven                             08-NOV-2007
 *      Added ITOFS, ITOFF.
 *
 * X-1.7        Camiel Vanderhoeven                             08-NOV-2007
 *      Restructured conversion routines.
 *
 * X-1.6        Camiel Vanderhoeven                             02-NOV-2007
 *      Added missing floating point instructions.
 *
 * X-1.5        Marcelo Eduardo Serrat                          31-OCT-2007
 *      Added CVTDG, CVTGD, CVTGF, MULG instructions.
 *
 * X-1.4        Camiel Vanderhoeven                             11-APR-2007
 *      Moved all data that should be saved to a state file to a structure
 *      "state".
 *
 * X-1.3        Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.2        Camiel Vanderhoeven                             13-MAR-2007
 *      Basic floating point support added.
 *
 * X-1.1        Camiel Vanderhoeven                             18-FEB-2007
 *      File created. Contains code previously found in AlphaCPU.h
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#define DO_CPYS  state.f[FREG_3] = (state.f[FREG_1] & X64(8000000000000000))		\
		  	   | (state.f[FREG_2] & X64(7fffffffffffffff));

#define DO_CPYSN state.f[FREG_3] = (state.f[FREG_1] & X64(8000000000000000) ^ X64(8000000000000000)) 	\
			   | (state.f[FREG_2] & X64(7fffffffffffffff));

#define DO_CPYSE state.f[FREG_3] = (state.f[FREG_1] & X64(fff0000000000000))		\
		  	   | (state.f[FREG_2] & X64(000fffffffffffff));

#define DO_CVTQL state.f[FREG_3] = ((state.f[FREG_2] & X64(00000000c0000000)) << 32)	\
	                   | ((state.f[FREG_2] & X64(000000003fffffff)) << 29);

#define DO_CVTLQ state.f[FREG_3] = sext_u64_32(  ((state.f[FREG_2] >> 32) & X64(00000000c0000000))	\
	                          | ((state.f[FREG_2] >> 29) & X64(000000003fffffff)));

#define DO_FCMOVEQ  if (state.f[FREG_1] == X64(0000000000000000) || state.f[FREG_1] == X64(8000000000000000))	state.f[FREG_3] = state.f[FREG_2];
#define DO_FCMOVGE  if (!(state.f[FREG_1]& X64(8000000000000000)) || state.f[FREG_1] == X64(8000000000000000))	state.f[FREG_3] = state.f[FREG_2];
#define DO_FCMOVGT  if (!(state.f[FREG_1]& X64(8000000000000000)) && state.f[FREG_1] != X64(0000000000000000))	state.f[FREG_3] = state.f[FREG_2];
#define DO_FCMOVLE  if ((state.f[FREG_1]& X64(8000000000000000)) || state.f[FREG_1] == X64(0000000000000000))	state.f[FREG_3] = state.f[FREG_2];
#define DO_FCMOVLT  if ((state.f[FREG_1]& X64(8000000000000000)) && state.f[FREG_1] != X64(8000000000000000))	state.f[FREG_3] = state.f[FREG_2];
#define DO_FCMOVNE  if (state.f[FREG_1] != X64(0000000000000000) && state.f[FREG_1] != X64(8000000000000000))	state.f[FREG_3] = state.f[FREG_2];

#define DO_MF_FPCR state.f[FREG_1] = state.fpcr;
#define DO_MT_FPCR                                      \
  state.fpcr = state.f[FREG_1] & X64(7fff800000000000); \
  if (state.fpcr & X64(03f0000000000000))               \
     state.fpcr |= X64(8000000000000000); /* SUM */

/** 2008/1/21: all code beyond this point has been updated **/

/* add */
#define DO_ADDG state.f[FREG_3] = vax_fadd (state.f[FREG_1], state.f[FREG_2], ins, DT_G, 0);
#define DO_ADDF state.f[FREG_3] = vax_fadd (state.f[FREG_1], state.f[FREG_2], ins, DT_F, 0);
#define DO_ADDT state.f[FREG_3] = ieee_fadd (state.f[FREG_1], state.f[FREG_2], ins, DT_T, 0);
#define DO_ADDS state.f[FREG_3] = ieee_fadd (state.f[FREG_1], state.f[FREG_2], ins, DT_S, 0);

/* subtract */
#define DO_SUBG state.f[FREG_3] = vax_fadd (state.f[FREG_1], state.f[FREG_2], ins, DT_G, 1);
#define DO_SUBF state.f[FREG_3] = vax_fadd (state.f[FREG_1], state.f[FREG_2], ins, DT_F, 1);
#define DO_SUBT state.f[FREG_3] = ieee_fadd (state.f[FREG_1], state.f[FREG_2], ins, DT_T, 1);
#define DO_SUBS state.f[FREG_3] = ieee_fadd (state.f[FREG_1], state.f[FREG_2], ins, DT_S, 1);

/* comparison */
#define DO_CMPGEQ state.f[FREG_3] = (vax_fcmp (state.f[FREG_1], state.f[FREG_2], ins) == 0) ? FP_TRUE : 0;
#define DO_CMPGLE state.f[FREG_3] = (vax_fcmp (state.f[FREG_1], state.f[FREG_2], ins) <= 0) ? FP_TRUE : 0;
#define DO_CMPGLT state.f[FREG_3] = (vax_fcmp (state.f[FREG_1], state.f[FREG_2], ins) < 0) ? FP_TRUE : 0;
#define DO_CMPTEQ state.f[FREG_3] = (ieee_fcmp (state.f[FREG_1], state.f[FREG_2], ins, 0) == 0) ? FP_TRUE : 0;
#define DO_CMPTLE state.f[FREG_3] = (ieee_fcmp (state.f[FREG_1], state.f[FREG_2], ins, 1) <= 0) ? FP_TRUE : 0;
#define DO_CMPTLT state.f[FREG_3] = (ieee_fcmp (state.f[FREG_1], state.f[FREG_2], ins, 1) < 0) ? FP_TRUE : 0;

#define DO_CMPTUN                                                                                 \
	  state.f[FREG_3] = ((ieee_unpack (state.f[FREG_1], &ufp1, ins) == UFT_NAN)                   \
                      || (ieee_unpack (state.f[FREG_2], &ufp2, ins) == UFT_NAN)) ? FP_TRUE : 0;   \

/* format conversions */
#define DO_CVTGQ state.f[FREG_3] = vax_cvtfi (state.f[FREG_2], ins);
#define DO_CVTQG state.f[FREG_3] = vax_cvtif (state.f[FREG_2], ins, DT_G);
#define DO_CVTQF state.f[FREG_3] = vax_cvtif (state.f[FREG_2], ins, DT_F);
#define DO_CVTTQ state.f[FREG_3] = ieee_cvtfi (state.f[FREG_2], ins);
#define DO_CVTQT state.f[FREG_3] = ieee_cvtif (state.f[FREG_2], ins, DT_T);
#define DO_CVTQS state.f[FREG_3] = ieee_cvtif (state.f[FREG_2], ins, DT_S);
#define DO_CVTGD state.f[FREG_3] = (vax_unpack (state.f[FREG_2], &ufp2, ins)) ? 0 : vax_rpack_d (&ufp2, ins);
#define DO_CVTDG state.f[FREG_3] = (vax_unpack_d (state.f[FREG_2], &ufp2, ins))? 0 : vax_rpack (&ufp2, ins, DT_G);;
#define DO_CVTGF state.f[FREG_3] = (vax_unpack (state.f[FREG_2], &ufp2, ins)) ? 0 : vax_rpack (&ufp2, ins, DT_F);
#define DO_CVTST state.f[FREG_3] = ieee_cvtst(state.f[FREG_2], ins);
#define DO_CVTTS state.f[FREG_3] = ieee_cvtts(state.f[FREG_2], ins);

/* float <-> integer register moves */
#define DO_FTOIS                                                \
  if (state.fpen == 0) GO_PAL(FEN); /* flt point disabled? */   \
	    state.r[REG_3] = ieee_sts (state.f[FREG_1]);            \

#define DO_FTOIT                                                \
  if (state.fpen == 0) GO_PAL(FEN); /* flt point disabled? */   \
	    state.r[REG_3] = state.f[FREG_1];                       \

#define DO_ITOFT state.f[FREG_3] = state.r[REG_1];
#define DO_ITOFS state.f[FREG_3] = ieee_lds((u32)state.r[REG_1]);
#define DO_ITOFF state.f[FREG_3] = vax_ldf(SWAP_VAXF((u32)state.r[REG_1]));

/* Multiply */
#define DO_MULG state.f[FREG_3] = vax_fmul (state.f[FREG_1], state.f[FREG_2], ins, DT_G);
#define DO_MULF state.f[FREG_3] = vax_fmul (state.f[FREG_1], state.f[FREG_2], ins, DT_F);
#define DO_MULT state.f[FREG_3] = ieee_fmul (state.f[FREG_1], state.f[FREG_2], ins, DT_T);
#define DO_MULS state.f[FREG_3] = ieee_fmul (state.f[FREG_1], state.f[FREG_2], ins, DT_S);

/* Divide */
#define DO_DIVG state.f[FREG_3] = vax_fdiv (state.f[FREG_1], state.f[FREG_2], ins, DT_G);
#define DO_DIVF state.f[FREG_3] = vax_fdiv (state.f[FREG_1], state.f[FREG_2], ins, DT_F);
#define DO_DIVT state.f[FREG_3] = ieee_fdiv (state.f[FREG_1], state.f[FREG_2], ins, DT_T);
#define DO_DIVS state.f[FREG_3] = ieee_fdiv (state.f[FREG_1], state.f[FREG_2], ins, DT_S);

/* Square-root */
#define DO_SQRTG state.f[FREG_3] = vax_sqrt(ins, DT_G);
#define DO_SQRTF state.f[FREG_3] = vax_sqrt(ins, DT_F);
#define DO_SQRTT state.f[FREG_3] = ieee_sqrt(ins, DT_T);
#define DO_SQRTS state.f[FREG_3] = ieee_sqrt(ins, DT_S);
