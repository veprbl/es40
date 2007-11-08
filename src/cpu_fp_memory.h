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
 * Contains code macros for the processor floating-point load/store instructions.
 * Based on ARM chapter 4.8.
 *
 * X-1.6        Camiel Vanderhoeven                             08-NOV-2007
 *      Restructured conversion routines.
 *
 * X-1.5        Eduardo Marcelo Serrat                          31-OCT-2007
 *      Fixed conversion routines.
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

#define DO_LDF									\
	if (FREG_1 != 31) {							\
	  DATA_PHYS(state.r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  state.f[FREG_1] = load_f((u32)READ_PHYS(32)); }

#define DO_LDG									\
	if (FREG_1 != 31) {							\
	  DATA_PHYS(state.r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  state.f[FREG_1] = load_g(READ_PHYS(64)); }

#define DO_LDS									\
	if (FREG_1 != 31) {							\
	  DATA_PHYS(state.r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  state.f[FREG_1] = load_s((u32)READ_PHYS(32)); }

#define DO_LDT									\
	if (FREG_1 != 31) {							\
	  DATA_PHYS(state.r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  state.f[FREG_1] = READ_PHYS(64); }

#define DO_STF									\
	  DATA_PHYS(state.r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(store_f(state.f[FREG_1]),32);
#define DO_STG									\
	  DATA_PHYS(state.r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(store_g(state.f[FREG_1]),64);

#define DO_STS									\
	  DATA_PHYS(state.r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(store_s(state.f[FREG_1]),32);

#define DO_STT									\
	  DATA_PHYS(state.r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(state.f[FREG_1],64);
