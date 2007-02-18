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
 * Contains code macros for the processor logical instructions.
 * Based on ARM chapter 4.5.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#define DO_AND r[REG_3] = r[REG_1] & V_2;
#define DO_BIC r[REG_3] = r[REG_1] & ~V_2;
#define DO_BIS r[REG_3] = r[REG_1] | V_2;
#define DO_EQV r[REG_3] = r[REG_1] ^ ~V_2;
#define DO_ORNOT r[REG_3] = r[REG_1] | ~V_2;
#define DO_XOR r[REG_3] = r[REG_1] ^ V_2;

#define DO_CMOVEQ if   (!r[REG_1])		r[REG_3] = V_2;
#define DO_CMOVGE if ((s64)r[REG_1]>=0)		r[REG_3] = V_2;
#define DO_CMOVGT if ((s64)r[REG_1]>0)		r[REG_3] = V_2;
#define DO_CMOVLBC if (!(r[REG_1] & X64(1)))	r[REG_3] = V_2;
#define DO_CMOVLBS if   (r[REG_1] & X64(1))	r[REG_3] = V_2;
#define DO_CMOVLE if ((s64)r[REG_1]<=0)		r[REG_3] = V_2;
#define DO_CMOVLT if ((s64)r[REG_1]<0)		r[REG_3] = V_2;
#define DO_CMOVNE if    (r[REG_1])		r[REG_3] = V_2;

#define DO_SLL r[REG_3] = r[REG_1] << (V_2 & 63);
#define DO_SRA r[REG_3] = r[REG_1] >> (V_2 & 63) | ((r[REG_1]>>63)?(X64_QUAD<<(64-(V_2 & 63))):0);
#define DO_SRL r[REG_3] = r[REG_1] >> (V_2 & 63);
