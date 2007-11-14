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
 * X-1.5        Camiel Vanderhoeven                             14-NOV-2007
 *      Implemented most simple PALcode routines in C++.
 *
 * X-1.4        Camiel Vanderhoeven                             12-NOV-2007
 *      Made a start with implementing PALcode routines in C++.
 *
 * X-1.3        Camiel Vanderhoeven                             11-APR-2007
 *      Moved all data that should be saved to a state file to a structure
 *      "state".
 *
 * X-1.2        Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.1        Camiel Vanderhoeven                             18-FEB-2007
 *      File created. Contains code previously found in AlphaCPU.h
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#define DO_AMASK state.r[REG_3] = V_2 & ~CPU_AMASK; 

#define DO_CALL_PAL				                \
      if (   (   (function < 0x40)		        \
		 && ((state.cm != 0)			        \
		 ))				                        \
	     || (   (function > 0x3f)		        \
    		    && (function < 0x80))	        \
	     || (function > 0xbf))		            \
	  {					                        \
	  UNKNOWN2				                    \
	  } else {						            \
        switch (function) {                             \
        case 0x01: /* CFLUSH */                         \
          break;                                        \
        case 0x02: /* DRAINA */                         \
          break;                                        \
        case 0x03: /* LDQP */                           \
          phys_address = state.r[16];                   \
          state.r[0] = READ_PHYS_NT(64);                \
          break;                                        \
        case 0x04: /* STQP */                           \
          phys_address = state.r[16];                   \
          WRITE_PHYS_NT(state.r[17],64);                \
          break;                                        \
        case 0x06: /* MFPR_ASN */                       \
          state.r[0] = state.asn;                       \
          break;                                        \
        case 0x07: /* MTPR_ASTEN */                     \
          state.r[0] = state.aster;                     \
          state.aster = ((state.aster & state.r[16]) |  \
                         (state.r[16] >>4)) & 0xf;      \
          break;                                        \
        case 0x08: /* MTPR_ASTSR */                     \
          state.r[0] = state.astrr;                     \
          state.astrr = ((state.astrr & state.r[16]) |  \
                         (state.r[16] >>4)) & 0xf;      \
          break;                                        \
        case 0x0b: /* MFPR_FEN */                       \
          state.r[0] = state.fpen?1:0;                  \
          break;                                        \
        case 0x0e: /* MFPR_IPL */                       \
          state.r[0] = (state.r[32+22] >> 8) & 0x1f;    \
          break;                                        \
        case 0x10: /* MFPR_MCES */                      \
          state.r[0] = (state.r[32+22] >> 16) & 0xff;   \
          break;                                        \
        case 0x11: /* MTPR_MCES */                                        \
          state.r[32+22] = (state.r[32+22] & X64(ffffffffff00ffff))       \
                         | (state.r[32+22] & X64(0000000000070000)        \
                             & ~(state.r[16] << 16))                      \
                         | ((state.r[16] <<16) & X64(0000000000180000));  \
          break;                                                          \
        case 0x12: /* MFPR_PCBB */                      \
          phys_address = state.r[32+21]+0x10;           \
          state.r[0] = READ_PHYS_NT(64);                \
          break;                                        \
        case 0x13: /* MFPR_PRBR */                      \
          phys_address = state.r[32+21]+0xa8;           \
          state.r[0] = READ_PHYS_NT(64);                \
          break;                                        \
        case 0x14: /* MTPR_PRBR */                      \
          phys_address = state.r[32+21]+0xa8;           \
          WRITE_PHYS_NT(state.r[16],64);                \
          break;                                        \
        case 0x15: /* MFPR_PTBR */                      \
          phys_address = state.r[32+21]+0x8;            \
          state.r[0] = READ_PHYS_NT(64)>>0xd;           \
          break;                                        \
        case 0x16: /* MFPR_SCBB */                      \
          phys_address = state.r[32+21]+0x170;          \
          state.r[0] = READ_PHYS_NT(64)>>0xd;           \
          break;                                        \
        case 0x17: /* MTPR_SCBB */                                    \
          phys_address = state.r[32+21]+0x170;                        \
          WRITE_PHYS_NT((state.r[16]&X64(00000000ffffffff))<<0xd,64); \
          break;                                                      \
        case 0x18: /* MTPR_SIRR */              \
          if (state.r[16]>0 && state.r[16]<16)  \
            state.sir |= 1 << state.r[16];      \
          break;                                \
        case 0x19: /* MFPR_SISR */              \
          state.r[0] = state.sir;               \
          break;                                \
        case 0x1a: /* MFPR_TBCHK */             \
          state.r[0] = X64(8000000000000000);   \
          break;                                \
        case 0x1b: /* MTPR_TBIA */              \
          itb->InvalidateAll();                 \
          dtb->InvalidateAll();                 \
          flush_icache();                       \
          break;                                \
        case 0x1c: /* MTPR_TBIAP */             \
          itb->InvalidateAllProcess();          \
          dtb->InvalidateAllProcess();          \
          flush_icache_asm();                   \
          break;                                \
        case 0x1d: /* MTPR_TBIS */              \
          itb->InvalidateSingle(state.r[16]);   \
          dtb->InvalidateSingle(state.r[16]);   \
          break;                                \
        case 0x1e: /* MFPR_ESP */               \
          phys_address = state.r[32+21]+0x10;   \
          phys_address = READ_PHYS_NT(64)+0x08; \
          state.r[0] = READ_PHYS_NT(64);        \
          break;                                \
        case 0x1f: /* MTPR_ESP */               \
          phys_address = state.r[32+21]+0x10;   \
          phys_address = READ_PHYS_NT(64)+0x08; \
          WRITE_PHYS_NT(state.r[16],64);        \
          break;                                \
        case 0x20: /* MFPR_SSP */               \
          phys_address = state.r[32+21]+0x10;   \
          phys_address = READ_PHYS_NT(64)+0x10; \
          state.r[0] = READ_PHYS_NT(64);        \
          break;                                \
        case 0x21: /* MTPR_SSP */               \
          phys_address = state.r[32+21]+0x10;   \
          phys_address = READ_PHYS_NT(64)+0x10; \
          WRITE_PHYS_NT(state.r[16],64);        \
          break;                                \
        case 0x22: /* MFPR_USP */               \
          phys_address = state.r[32+21]+0x10;   \
          phys_address = READ_PHYS_NT(64)+0x18; \
          state.r[0] = READ_PHYS_NT(64);        \
          break;                                \
        case 0x23: /* MTPR_USP */               \
          phys_address = state.r[32+21]+0x10;   \
          phys_address = READ_PHYS_NT(64)+0x18; \
          WRITE_PHYS_NT(state.r[16],64);        \
          break;                                \
        case 0x24: /* MTPR_TBISD */             \
          dtb->InvalidateSingle(state.r[16]);   \
          break;                                \
        case 0x25: /* MTPR_TBISI */             \
          itb->InvalidateSingle(state.r[16]);   \
          break;                                \
        case 0x26: /* MFPR_ASTEN */             \
          state.r[0] = state.aster;             \
          break;                                \
        case 0x27: /* MFPR_ASTSR */             \
          state.r[0] = state.astrr;             \
          break;                                \
        case 0x29: /* MFPR_VPTB */              \
          phys_address = state.r[32+21];        \
          state.r[0] = READ_PHYS_NT(64);        \
          break;                                \
        case 0x2e: /* MTPR_DATFX */             \
          phys_address = state.r[32+21] + 0x10; \
          phys_address = READ_PHYS_NT(64);      \
          WRITE_PHYS_NT((READ_PHYS_NT(64) | (X64(1)<<0x3f)) &~(state.r[16]<<0x3f),64);   \
          break;                                \
        case 0x3f: /* MFPR_WHAMI */             \
          phys_address = state.r[32+21] + 0x98; \
          state.r[0] = READ_PHYS_NT(64);        \
          break;                                \
        case 0x91: /* RD_PS */                  \
          state.r[0] = state.r[32+22] & X64(ffff); \
          break;                                \
        case 0x9b: /* SWASTEN */                                            \
          state.r[0] = (state.aster & (1<<((state.r[32+22]>>3)&3)))?1:0;    \
          if (state.r[16]&1)                                                \
            state.aster |= (1<<((state.r[32+22]>>3)&3));                    \
          else                                                              \
            state.aster &= ~(1<<((state.r[32+22]>>3)&3));                   \
          break;                                                            \
        case 0x9c: /* WR_PS_SW */                               \
          state.r[32+22] &= ~X64(3);                            \
          state.r[32+33] |= state.r[16] & X64(3);               \
          break;                                                \
        case 0x9d: /* RSCC */                                   \
          phys_address = state.r[32+21] + 0xa0;                 \
          state.r[0] = READ_PHYS_NT(64);                        \
          if (state.cc>(state.r[0] & X64(00000000ffffffff)))    \
            state.r[0] += X64(0000000100000000);                \
          state.r[0] &= X64(ffffffff00000000);                  \
          state.r[0] |= state.cc;                               \
          WRITE_PHYS_NT(state.r[0],64);                         \
          break;                                                \
        case 0x9e: /* READ_UNQ */               \
          phys_address = state.r[32+21]+0x10;   \
          phys_address = READ_PHYS_NT(64)+0x48; \
          state.r[0] = READ_PHYS_NT(64);        \
          break;                                \
        case 0x9f: /* WRITE_UNQ */              \
          phys_address = state.r[32+21]+0x10;   \
          phys_address = READ_PHYS_NT(64)+0x48; \
          WRITE_PHYS_NT(state.r[16],64);        \
          break;                                \
        case 0x92:                              \
	      state.lock_flag = false;		        \
        default:                                \
	      state.r[32+23] = state.pc;			\
	      state.pc = state.pal_base				\
	        | (X64(1)<<13 )			            \
	        | (((u64)(function & 0x80)) <<5 )	\
	        | (((u64)(function & 0x3f)) << 6 )	\
	        | X64(1);				            \
	      TRC(true,false)			            \
        }                                       \
      }

#define DO_IMPLVER state.r[REG_3] = CPU_IMPLVER;

#define DO_RPCC state.r[REG_1] = ((u64)state.cc_offset)<<32 | state.cc;

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


