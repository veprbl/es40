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
 * Contains code macros for the processor integer arithmetic instructions.
 * Based on ARM chapter 4.4.
 *
 * X-1.5        Camiel Vanderhoeven                             2-DEC-2007
 *      Use sext_64 inline.
 *
 * X-1.4        Camiel Vanderhoeven                             11-APR-2007
 *      Moved all data that should be saved to a state file to a structure
 *      "state".
 *
 * X-1.3        Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.2        Camiel Vanderhoeven                             18-MAR-2007
 *      Bugfix in CTLZ and CTTZ instructions. Fixes the INCON_SCHED 
 *      bugcheck (bug # 1680064).
 *
 * X-1.1        Camiel Vanderhoeven                             18-FEB-2007
 *      File created. Contains code previously found in AlphaCPU.h
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#define DO_ADDQ state.r[REG_3] = state.r[REG_1] + V_2;
#define DO_S4ADDQ state.r[REG_3] = (state.r[REG_1]*4) + V_2;
#define DO_S8ADDQ state.r[REG_3] = (state.r[REG_1]*8) + V_2;

#define DO_CMPEQ state.r[REG_3] = (state.r[REG_1]==V_2)?1:0;
#define DO_CMPLT state.r[REG_3] = ((s64)state.r[REG_1]<(s64)V_2)?1:0;
#define DO_CMPLE state.r[REG_3] = ((s64)state.r[REG_1]<=(s64)V_2)?1:0;

#define DO_ADDL state.r[REG_3] = sext_64(state.r[REG_1] + V_2,32);
#define DO_S4ADDL state.r[REG_3] = sext_64((state.r[REG_1]*4) + V_2,32);
#define DO_S8ADDL state.r[REG_3] = sext_64((state.r[REG_1]*8) + V_2,32);

#define DO_CTLZ									\
 	    temp_64 = 0;							\
	    temp_64_2 = V_2;							\
	    for (i=63;i>=0;i--)							\
	      if ((temp_64_2>>i)&1)						\
	        break;								\
	      else								\
	        temp_64++;							\
	    state.r[REG_3] = temp_64;

#define DO_CTPOP								\
 	    temp_64 = 0;							\
	    temp_64_2 = V_2;							\
	    for (i=0;i<64;i++)							\
	      if ((temp_64_2>>i)&1)						\
	        temp_64++;							\
	    state.r[REG_3] = temp_64;

#define DO_CTTZ									\
	temp_64 = 0;								\
	    temp_64_2 = V_2;							\
	    for (i=0;i<64;i++)							\
	      if ((temp_64_2>>i)&1)						\
	        break;								\
	      else								\
	        temp_64++;							\
	    state.r[REG_3] = temp_64;

#define DO_CMPULT state.r[REG_3] = ((u64)state.r[REG_1]<(u64)V_2)?1:0;
#define DO_CMPULE state.r[REG_3] = ((u64)state.r[REG_1]<=(u64)V_2)?1:0;

#define DO_MULL state.r[REG_3] = sext_64((u32)state.r[REG_1]*(u32)V_2,32);
#define DO_MULQ state.r[REG_3] = state.r[REG_1]*V_2;

	 /*
	    The UMULH algorithm was snagged from:
	    http://www.cs.uaf.edu/2004/fall/cs301/notes/notes/node47.html

	    which is very similar to the method used in the (unreleased)
	    simh alpha emulation.

	  */
#define DO_UMULH							\
	    temp_64_a = (state.r[REG_1] >> 32) & X64_LONG;			\
	    temp_64_b = state.r[REG_1]  & X64_LONG;				\
	    temp_64_c = (V_2 >> 32) & X64_LONG;				\
	    temp_64_d = V_2 & X64_LONG;					\
									\
	    temp_64_lo = temp_64_b * temp_64_d;				\
	    temp_64_x = temp_64_a * temp_64_d + temp_64_c * temp_64_b;	\
	    temp_64_y = ((temp_64_lo >> 32) & X64_LONG) + temp_64_x;	\
	    temp_64_lo = (temp_64_lo & X64_LONG) | ((temp_64_y & X64_LONG) << 32);	\
	    temp_64_hi = (temp_64_y >> 32) & X64_LONG;			\
	    temp_64_hi += temp_64_a * temp_64_c;			\
	    state.r[REG_3] = temp_64_hi;

#define DO_SUBQ state.r[REG_3] = state.r[REG_1] - V_2;
#define DO_S4SUBQ state.r[REG_3] = (state.r[REG_1]*4) - V_2;
#define DO_S8SUBQ state.r[REG_3] = (state.r[REG_1]*8) - V_2;

#define DO_SUBL state.r[REG_3] = sext_64(state.r[REG_1] - V_2,32);
#define DO_S4SUBL state.r[REG_3] = sext_64((state.r[REG_1]*4) - V_2,32);
#define DO_S8SUBL state.r[REG_3] = sext_64((state.r[REG_1]*8) - V_2,32);

