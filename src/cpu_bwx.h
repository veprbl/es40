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
 * Contains code macros for the processor BWX (byte and word extension) instructions.
 * Based on ARM chapter 4.6.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#define DO_CMPBGE								\
 	    r[REG_3] =								\
		(((u8)( r[REG_1]     &0xff)>=(u8)( V_2      & 0xff))?  1:0)	\
	      | (((u8)((r[REG_1]>> 8)&0xff)>=(u8)((V_2>> 8) & 0xff))?  2:0)	\
	      | (((u8)((r[REG_1]>>16)&0xff)>=(u8)((V_2>>16) & 0xff))?  4:0)	\
	      | (((u8)((r[REG_1]>>24)&0xff)>=(u8)((V_2>>24) & 0xff))?  8:0)	\
	      | (((u8)((r[REG_1]>>32)&0xff)>=(u8)((V_2>>32) & 0xff))? 16:0)	\
	      | (((u8)((r[REG_1]>>40)&0xff)>=(u8)((V_2>>40) & 0xff))? 32:0)	\
	      | (((u8)((r[REG_1]>>48)&0xff)>=(u8)((V_2>>48) & 0xff))? 64:0)	\
	      | (((u8)((r[REG_1]>>56)&0xff)>=(u8)((V_2>>56) & 0xff))?128:0);


#define DO_EXTBL r[REG_3] = (r[REG_1] >> ((V_2&7)*8)) & X64_BYTE;
#define DO_EXTWL r[REG_3] = (r[REG_1] >> ((V_2&7)*8)) & X64_WORD;
#define DO_EXTLL r[REG_3] = (r[REG_1] >> ((V_2&7)*8)) & X64_LONG;
#define DO_EXTQL r[REG_3] = (r[REG_1] >> ((V_2&7)*8));
#define DO_EXTWH r[REG_3] = (r[REG_1] << ((64-((V_2&7)*8))&63)) & X64_WORD;
#define DO_EXTLH r[REG_3] = (r[REG_1] << ((64-((V_2&7)*8))&63)) & X64_LONG;
#define DO_EXTQH r[REG_3] = (r[REG_1] << ((64-((V_2&7)*8))&63)) & X64_QUAD;

#define DO_INSBL r[REG_3] = (r[REG_1]&X64_BYTE) << ((V_2&7)*8);
#define DO_INSWL r[REG_3] = (r[REG_1]&X64_WORD) << ((V_2&7)*8);
#define DO_INSLL r[REG_3] = (r[REG_1]&X64_LONG) << ((V_2&7)*8);
#define DO_INSQL r[REG_3] = (r[REG_1]         ) << ((V_2&7)*8);
#define DO_INSWH r[REG_3] = (r[REG_1]&X64_WORD) >> ((64-((V_2&7)*8))&63);
#define DO_INSLH r[REG_3] = (r[REG_1]&X64_LONG) >> ((64-((V_2&7)*8))&63);
#define DO_INSQH r[REG_3] = (r[REG_1]&X64_QUAD) >> ((64-((V_2&7)*8))&63);

#define DO_MSKBL r[REG_3] = r[REG_1] & ~(X64_BYTE<<((V_2&7)*8));
#define DO_MSKWL r[REG_3] = r[REG_1] & ~(X64_WORD<<((V_2&7)*8));
#define DO_MSKLL r[REG_3] = r[REG_1] & ~(X64_LONG<<((V_2&7)*8));
#define DO_MSKQL r[REG_3] = r[REG_1] & ~(X64_QUAD<<((V_2&7)*8));
#define DO_MSKWH r[REG_3] = r[REG_1] & ~(X64_WORD>>((64-((V_2&7)*8))&63));
#define DO_MSKLH r[REG_3] = r[REG_1] & ~(X64_LONG>>((64-((V_2&7)*8))&63));
#define DO_MSKQH r[REG_3] = r[REG_1] & ~(X64_QUAD>>((64-((V_2&7)*8))&63));

#define DO_SEXTB r[REG_3] = SEXT(V_2,8);
#define DO_SEXTW r[REG_3] = SEXT(V_2,16);

#define DO_ZAP								\
	  r[REG_3] = r[REG_1] & (  ((V_2&  1)?0:              X64(ff))	\
				 | ((V_2&  2)?0:            X64(ff00))	\
				 | ((V_2&  4)?0:          X64(ff0000))	\
				 | ((V_2&  8)?0:        X64(ff000000))	\
				 | ((V_2& 16)?0:      X64(ff00000000))	\
				 | ((V_2& 32)?0:    X64(ff0000000000))	\
				 | ((V_2& 64)?0:  X64(ff000000000000))	\
				 | ((V_2&128)?0:X64(ff00000000000000)));

#define DO_ZAPNOT							\
 	  r[REG_3] = r[REG_1] & (  ((V_2&  1)?              X64(ff):0)	\
				 | ((V_2&  2)?            X64(ff00):0)	\
				 | ((V_2&  4)?          X64(ff0000):0)	\
				 | ((V_2&  8)?        X64(ff000000):0)	\
				 | ((V_2& 16)?      X64(ff00000000):0)	\
				 | ((V_2& 32)?    X64(ff0000000000):0)	\
				 | ((V_2& 64)?  X64(ff000000000000):0)	\
				 | ((V_2&128)?X64(ff00000000000000):0) );

