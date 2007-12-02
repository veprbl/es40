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
 * Contains code macros for the processor floating-point operate instructions.
 * Based on ARM chapter 4.10.
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

#define DO_CVTLQ state.f[FREG_3] = sext_64(  ((state.f[FREG_2] >> 32) & X64(00000000c0000000))	\
	                          | ((state.f[FREG_2] >> 29) & X64(000000003fffffff)), 32);

#define DO_FCMOVEQ  if (state.f[FREG_1] == X64(0000000000000000) || state.f[FREG_1] == X64(8000000000000000))	state.f[FREG_3] = state.f[FREG_2];
#define DO_FCMOVGE  if (!(state.f[FREG_1]& X64(8000000000000000)) || state.f[FREG_1] == X64(8000000000000000))	state.f[FREG_3] = state.f[FREG_2];
#define DO_FCMOVGT  if (!(state.f[FREG_1]& X64(8000000000000000)) && state.f[FREG_1] != X64(0000000000000000))	state.f[FREG_3] = state.f[FREG_2];
#define DO_FCMOVLE  if ((state.f[FREG_1]& X64(8000000000000000)) || state.f[FREG_1] == X64(0000000000000000))	state.f[FREG_3] = state.f[FREG_2];
#define DO_FCMOVLT  if ((state.f[FREG_1]& X64(8000000000000000)) && state.f[FREG_1] != X64(8000000000000000))	state.f[FREG_3] = state.f[FREG_2];
#define DO_FCMOVNE  if (state.f[FREG_1] != X64(0000000000000000) && state.f[FREG_1] != X64(8000000000000000))	state.f[FREG_3] = state.f[FREG_2];

#define DO_MF_FPCR state.f[FREG_1] = state.fpcr;
#define DO_MT_FPCR state.fpcr = state.f[FREG_1];

#define DO_ADDG state.f[FREG_3] = host2g(g2host(state.f[FREG_1])+g2host(state.f[FREG_2]));
#define DO_ADDF state.f[FREG_3] = host2f(f2host(state.f[FREG_1])+f2host(state.f[FREG_2]));
#define DO_ADDT state.f[FREG_3] = host2t(t2host(state.f[FREG_1])+t2host(state.f[FREG_2]));
#define DO_ADDS state.f[FREG_3] = host2s(s2host(state.f[FREG_1])+s2host(state.f[FREG_2]));

#define DO_SUBG state.f[FREG_3] = host2g(g2host(state.f[FREG_1])-g2host(state.f[FREG_2]));
#define DO_SUBF state.f[FREG_3] = host2f(f2host(state.f[FREG_1])-f2host(state.f[FREG_2]));
#define DO_SUBT state.f[FREG_3] = host2t(t2host(state.f[FREG_1])-t2host(state.f[FREG_2]));
#define DO_SUBS state.f[FREG_3] = host2s(s2host(state.f[FREG_1])-s2host(state.f[FREG_2]));

#define DO_CMPGEQ state.f[FREG_3] = (g2host(state.f[FREG_1])==g2host(state.f[FREG_2]))?X64(4000000000000000):0;
#define DO_CMPGLE state.f[FREG_3] = (g2host(state.f[FREG_1])<=g2host(state.f[FREG_2]))?X64(4000000000000000):0;
#define DO_CMPGLT state.f[FREG_3] = (g2host(state.f[FREG_1])<g2host(state.f[FREG_2]))?X64(4000000000000000):0;

#define DO_CMPTEQ state.f[FREG_3] = (t2host(state.f[FREG_1])==t2host(state.f[FREG_2]))?X64(4000000000000000):0;
#define DO_CMPTLE state.f[FREG_3] = (t2host(state.f[FREG_1])<=t2host(state.f[FREG_2]))?X64(4000000000000000):0;
#define DO_CMPTLT state.f[FREG_3] = (t2host(state.f[FREG_1])<t2host(state.f[FREG_2]))?X64(4000000000000000):0;
#define DO_CMPTUN state.f[FREG_3] = (i_isnan(state.f[FREG_1]) || i_isnan(state.f[FREG_2]))?X64(4000000000000000):0;

#define DO_CVTGQ state.f[FREG_3] = (u64)((s64)g2host(state.f[FREG_2]));
#define DO_CVTQG state.f[FREG_3] = host2g((double)((s64)state.f[FREG_2]));
#define DO_CVTQF state.f[FREG_3] = host2f((double)((s64)state.f[FREG_2]));

#define DO_CVTTQ state.f[FREG_3] = (u64)((s64)t2host(state.f[FREG_2]));
#define DO_CVTQT state.f[FREG_3] = host2t((double)((s64)state.f[FREG_2]));
#define DO_CVTQS state.f[FREG_3] = host2s((double)((s64)state.f[FREG_2]));
#define DO_CVTGD state.f[FREG_3] = host2d(g2host(state.f[FREG_2]));
#define DO_CVTDG state.f[FREG_3] = host2g(d2host(state.f[FREG_2]));
#define DO_CVTGF state.f[FREG_3] = host2f(g2host(state.f[FREG_2]));

#define DO_FTOIS								\
 	    temp_64 = state.f[FREG_1];						\
	    state.r[REG_3] = (temp_64 & X64(000000003fffffff))			\
	      |((temp_64 & X64(c000000000000000)) >> 32)			\
	      |(((temp_64 & X64(8000000000000000)) >>31) * X64(ffffffff));

#define DO_FTOIT state.r[REG_3] = state.f[FREG_1];
#define DO_ITOFT state.f[FREG_3] = state.r[REG_1];
#define DO_ITOFS state.f[FREG_3] = load_s((u32)state.r[REG_1]);
#define DO_ITOFF state.f[FREG_3] = itof_f(state.r[REG_1]);

#define DO_MULG state.f[FREG_3] = host2g(g2host(state.f[FREG_1]) * g2host(state.f[FREG_2]));
#define DO_MULF state.f[FREG_3] = host2f(f2host(state.f[FREG_1]) * f2host(state.f[FREG_2]));
#define DO_MULT state.f[FREG_3] = host2t(t2host(state.f[FREG_1]) * t2host(state.f[FREG_2]));
#define DO_MULS state.f[FREG_3] = host2s(s2host(state.f[FREG_1]) * s2host(state.f[FREG_2]));

#define DO_DIVG state.f[FREG_3] = host2g(g2host(state.f[FREG_1]) / g2host(state.f[FREG_2]));
#define DO_DIVF state.f[FREG_3] = host2f(f2host(state.f[FREG_1]) / f2host(state.f[FREG_2]));
#define DO_DIVT state.f[FREG_3] = host2t(t2host(state.f[FREG_1]) / t2host(state.f[FREG_2]));
#define DO_DIVS state.f[FREG_3] = host2s(s2host(state.f[FREG_1]) / s2host(state.f[FREG_2]));

#define DO_SQRTG state.f[FREG_3] = host2g(sqrt(g2host(state.f[FREG_2])));
#define DO_SQRTF state.f[FREG_3] = host2f(sqrt(f2host(state.f[FREG_2])));
#define DO_SQRTT state.f[FREG_3] = host2t(sqrt(t2host(state.f[FREG_2])));
#define DO_SQRTS state.f[FREG_3] = host2s(sqrt(s2host(state.f[FREG_2])));

#define DO_CVTST state.f[FREG_3] = host2t(s2host(state.f[FREG_2]));
#define DO_CVTTS state.f[FREG_3] = host2s(t2host(state.f[FREG_2]));
