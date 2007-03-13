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
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#define DO_LDF									\
	if (FREG_1 != 31) {							\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  f[FREG_1] = f2g(swap_f((u32)READ_PHYS(32))); }

#define DO_LDG									\
	if (FREG_1 != 31) {							\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  f[FREG_1] = swap_g(READ_PHYS(64)); }

#define DO_LDS									\
	if (FREG_1 != 31) {							\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  f[FREG_1] = s2t((u32)READ_PHYS(32)); }

#define DO_LDT									\
	if (FREG_1 != 31) {							\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  f[FREG_1] = READ_PHYS(64); }

#define DO_STF									\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(swap_f(g2f(f[FREG_1])),32);

#define DO_STG									\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(swap_g(f[FREG_1]),64);

#define DO_STS									\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(t2s(f[FREG_1]),32);

#define DO_STT									\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(f[FREG_1],64);
