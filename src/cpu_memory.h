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
 * Contains code macros for the processor memory load/store instructions.
 * Based on ARM chapter 4.2.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#define DO_LDA r[REG_1] = r[REG_2] + DISP_16;

#define DO_LDAH r[REG_1] = r[REG_2] + (DISP_16<<16);


#define DO_LDBU									\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  r[REG_1] = READ_PHYS(8);

#define DO_LDL									\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  r[REG_1] = SEXT(READ_PHYS(32),32);

#define DO_LDL_L								\
	  lock_flag = true;							\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  r[REG_1] = SEXT(READ_PHYS(32),32);

#define DO_LDQ									\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  r[REG_1] = READ_PHYS(64);

#define DO_LDQ_L								\
	  lock_flag = true;							\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  r[REG_1] = READ_PHYS(64);

#define DO_LDQ_U									\
	  DATA_PHYS((r[REG_2] + DISP_16)& ~X64(7), ACCESS_READ, true, false, false);	\
	  r[REG_1] = READ_PHYS(64);

#define DO_LDWU									\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_READ, true, false, false);	\
	  r[REG_1] = READ_PHYS(16);



#define DO_STB									\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(r[REG_1],8);

#define DO_STL									\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(r[REG_1],32);

#define DO_STL_C								\
	  if (lock_flag) {							\
	      DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	      WRITE_PHYS(r[REG_1],32);						\
	    }									\
	  r[REG_1] = lock_flag?1:0;						\
	  lock_flag = false;

#define DO_STQ									\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(r[REG_1],64);

#define DO_STQ_C								\
	  if (lock_flag) {							\
	      DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	      WRITE_PHYS(r[REG_1],64);						\
	    }									\
	  r[REG_1] = lock_flag?1:0;						\
	  lock_flag = false;

#define DO_STQ_U									\
	  DATA_PHYS((r[REG_2] + DISP_16)& ~X64(7), ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(r[REG_1],64);

#define DO_STW									\
	  DATA_PHYS(r[REG_2] + DISP_16, ACCESS_WRITE, true, false, false);	\
	  WRITE_PHYS(r[REG_1],16);

