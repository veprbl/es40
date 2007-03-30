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
 * Contains code macros for miscellaneous processor instructions.
 * Based on ARM chapter 4.11.
 *
 * X-1.2        Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.1        Camiel Vanderhoeven                             18-FEB-2007
 *      File created. Contains code previously found in AlphaCPU.h
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#define DO_AMASK r[REG_3] = V_2 & ~CPU_AMASK; 

#define DO_CALL_PAL				\
      if (   (   (function < 0x40)		\
		 && ((cm != 0)			\
		 ))				\
	     || (   (function > 0x3f)		\
    		    && (function < 0x80))	\
	     || (function > 0xbf))		\
	{					\
	  UNKNOWN2				\
	}					\
      else					\
      {						\
	  if (function == 0x92)	/* REI */	\
		  lock_flag = false;		\
	  r[32+23] = pc;			\
	  pc = pal_base				\
	    | (X64(1)<<13 )			\
	    | (((u64)(function & 0x80)) <<5 )	\
	    | (((u64)(function & 0x3f)) << 6 )	\
	    | X64(1);				\
	  TRC(true,false)			\
      }

#define DO_IMPLVER r[REG_3] = CPU_IMPLVER;

#define DO_RPCC r[REG_1] = ((u64)cc_offset)<<32 | cc;

// The following ops have no function right now (at least, not until multiple CPU's are supported).
#define DO_TRAPB ;
#define DO_EXCB ;
#define DO_MB ;
#define DO_WMB ;
#define DO_FETCH ;
#define DO_FETCH_M ;
#define DO_ECB ;
#define DO_WH64 ;
#define DO_WH64EN ;


