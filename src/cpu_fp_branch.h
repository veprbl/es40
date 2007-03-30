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
 * Contains code macros for the processor floating-point branch instructions.
 * Based on ARM chapter 4.9.
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

#define DO_FBEQ  if (f[FREG_1] == X64(0000000000000000) || f[FREG_1] == X64(8000000000000000))	pc += (DISP_21 * 4);
#define DO_FBGE  if (!(f[FREG_1]& X64(8000000000000000)) || f[FREG_1] == X64(8000000000000000))	pc += (DISP_21 * 4);
#define DO_FBGT  if (!(f[FREG_1]& X64(8000000000000000)) && f[FREG_1] != X64(0000000000000000))	pc += (DISP_21 * 4);
#define DO_FBLE  if ((f[FREG_1]& X64(8000000000000000)) || f[FREG_1] == X64(0000000000000000))	pc += (DISP_21 * 4);
#define DO_FBLT  if ((f[FREG_1]& X64(8000000000000000)) && f[FREG_1] != X64(8000000000000000))	pc += (DISP_21 * 4);
#define DO_FBNE  if (f[FREG_1] != X64(0000000000000000) && f[FREG_1] != X64(8000000000000000))	pc += (DISP_21 * 4);
