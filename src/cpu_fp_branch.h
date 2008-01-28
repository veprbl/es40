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
 * X-1.6        Camiel Vanderhoeven                             28-JAN-2008
 *      Better floating-point exception handling.
 *
 * X-1.5        Camiel Vanderhoeven                             22-JAN-2008
 *      Implement new floating-point code.
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

#define DO_FBEQ                                                         \
  FPSTART; \
  if ((state.f[FREG_1] & ~FPR_SIGN) == 0)            /* +0 or - 0? */ \
    state.pc += (DISP_21 * 4);

#define DO_FBGE                                                         \
  FPSTART; \
  if (state.f[FREG_1] <= FPR_SIGN)                   /* +0 to + n? */ \
    state.pc += (DISP_21 * 4);

#define DO_FBGT                                                         \
  FPSTART; \
  if (!(state.f[FREG_1] & FPR_SIGN) && (state.f[FREG_1] != 0))        \
                                               /* not - and not 0? */ \
    state.pc += (DISP_21 * 4);

#define DO_FBLE                                                         \
  FPSTART; \
  if ((state.f[FREG_1] & FPR_SIGN) || (state.f[FREG_1] == 0))         \
                                                        /* - or 0? */ \
    state.pc += (DISP_21 * 4);

#define DO_FBLT                                                         \
  FPSTART; \
  if (state.f[FREG_1] > FPR_SIGN)                     /* -0 to -n? */ \
    state.pc += (DISP_21 * 4);

#define DO_FBNE                                                         \
  FPSTART; \
  if ((state.f[FREG_1] & ~FPR_SIGN) != 0)         /* not +0 or -0? */ \
    state.pc += (DISP_21 * 4);
