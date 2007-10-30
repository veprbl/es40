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
 * Contains code macros for the processor PALmode instructions.
 * Based on HRM.
 *
 * X-1.7        Camiel Vanderhoeven                             30-OCT-2007
 *      IPR 0x2d identified as M_FIX (Mbox fixed behaviour)
 *
 * X-1.6        Camiel Vanderhoeven                             11-APR-2007
 *      Moved all data that should be saved to a state file to a structure
 *      "state".
 *
 * X-1.5        Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.4        Camiel Vanderhoeven                             8-MAR-2007
 *      Different argument list for CTranslationBuffer methods, and for
 *      va_form function.
 *
 * X-1.3        Camiel Vanderhoeven                             3-MAR-2007
 *      Put 0x2d in HW_MTPR to get rid of error messages about this unknown
 *      IPR. WE STILL NEED TO FIND OUT WHAT THIS REGISTER DOES. 
 *
 * X-1.2        Camiel Vanderhoeven                             22-FEB-2007
 *      Got rid of calls to get_asn.
 *
 * X-1.1        Camiel Vanderhoeven                             18-FEB-2007
 *      File created. Contains code previously found in AlphaCPU.h
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#define DO_HW_MFPR						\
      if ((function & 0xc0) == 0x40) {	/* PCTX */		\
	  if (DO_ACTION)					\
 	    state.r[REG_1] = ((u64)state.asn << 39)				\
	      | ((u64)state.astrr << 9)				\
	      | ((u64)state.aster <<5)				\
	      | (state.fpen?X64(1)<<3:0)				\
	      | (state.ppcen?X64(1)<<1:0);				\
      } else {							\
	switch (function)					\
        {							\
        case 0x05: /* PMPC     */				\
 	    state.r[REG_1] = state.pmpc;					\
	    break;						\
        case 0x06: /* EXC_ADDR */				\
 	    state.r[REG_1] = state.exc_addr;				\
	    break;						\
        case 0x07: /* IVA_FORM */				\
 	    state.r[REG_1] = va_form(state.exc_addr,true);			\
	    break;						\
        case 0x08: /* IER_CM   */				\
        case 0x09: /* CM       */				\
        case 0x0a: /* IER      */				\
        case 0x0b: /* IER_CM   */				\
 	    state.r[REG_1] = (((u64)state.eien) << 33)			\
	      | (((u64)state.slen) << 32)				\
	      | (((u64)state.cren) << 31)				\
	      | (((u64)state.pcen) << 29)				\
	      | (((u64)state.sien) << 14)				\
	      | (((u64)state.asten) << 13)				\
	      | (((u64)state.cm) << 3);				\
	    break;						\
        case 0x0c: /* SIRR */					\
 	    state.r[REG_1] = ((u64)state.sir) << 14;			\
	    break;						\
        case 0x0d: /* ISUM */					\
 	    state.r[REG_1] = (((u64)(state.eir & state.eien)) << 33)		\
	      | (((u64)(state.slr & state.slen)) << 32)			\
	      | (((u64)(state.crr & state.cren)) << 31)			\
	      | (((u64)(state.pcr & state.pcen)) << 29)			\
	      | (((u64)(state.sir & state.sien)) << 14)			\
	      | (((u64)( ((X64(1)<<(state.cm+1))-1) & state.aster & state.astrr & (state.asten * 0x3))) << 3)	\
	      | (((u64)( ((X64(1)<<(state.cm+1))-1) & state.aster & state.astrr & (state.asten * 0xc))) << 7);	\
	    break;						\
        case 0x0f: /* EXC_SUM */				\
 	    state.r[REG_1] = state.exc_sum;					\
	    break;						\
        case 0x10: /* PAL_BASE */				\
 	    state.r[REG_1] = state.pal_base;				\
	    break;						\
        case 0x11: /* i_ctl */					\
 	    state.r[REG_1] = state.i_ctl_other				\
	      | (((u64)CPU_CHIP_ID)<<24)			\
	      | (u64)state.i_ctl_vptb					\
	      | (((u64)state.i_ctl_va_mode) << 15)			\
	      | (state.hwe?X64(1)<<12:0)				\
	      | (state.sde?X64(1)<<7:0)				\
	      | (((u64)state.i_ctl_spe) << 3);			\
	    break;						\
        case 0x14: /* PCTR_CTL */				\
 	    state.r[REG_1] = state.pctr_ctl;				\
	    break;						\
        case 0x16: /* I_STAT */					\
 	    state.r[REG_1] = state.i_stat;					\
	    break;						\
        case 0x27: /* MM_STAT */				\
 	    state.r[REG_1] = state.mm_stat;					\
	    break;						\
        case 0x2a: /* DC_STAT */				\
 	    state.r[REG_1] = state.dc_stat;					\
	    break;						\
        case 0x2b: /* C_DATA */					\
 	    state.r[REG_1] = 0;					\
	    break;						\
        case 0xc0: /* CC */					\
 	    state.r[REG_1] = (((u64)state.cc_offset) << 32) |  state.cc;		\
	    break;						\
        case 0xc2: /* VA */					\
 	    state.r[REG_1] = state.fault_va;				\
	    break;						\
        case 0xc3: /* VA_FORM */				\
 	    state.r[REG_1] = va_form(state.fault_va, false);		\
	    break;						\
        default:						\
	  UNKNOWN2;						\
        }							\
      }


#define DO_HW_MTPR								\
    if ((function & 0xc0) == 0x40) {						\
	    if (function & 1)							\
	      state.asn = (int)(state.r[REG_2]>>39) & 0xff;					\
	    if (function & 2)							\
	      state.aster = (int)(state.r[REG_2]>>5) & 0xf;					\
	    if (function & 4)							\
	      state.astrr = (int)(state.r[REG_2]>>9) & 0xf;					\
	    if (function & 8)							\
	      state.ppcen = (int)(state.r[REG_2]>>1) & 1;					\
	    if (function & 16)							\
	      state.fpen = (int)(state.r[REG_2]>>3) & 1;					\
    } else {									\
      switch (function)								\
        {									\
        case 0x00: /* ITB_TAG */						\
 	    itb->write_tag(0,state.r[REG_2]);						\
	    break;								\
        case 0x01: /* ITB_PTE */						\
 	    itb->write_pte(0,state.r[REG_2]);				\
            break;								\
        case 0x02: /* ITB_IAP */						\
 	    itb->InvalidateAllProcess();					\
	    break;								\
        case 0x03: /* ITB_IA */							\
 	    itb->InvalidateAll();						\
	    break;								\
        case 0x04: /* ITB_IS */							\
 	    itb->InvalidateSingle(state.r[REG_2]);				\
	    break;								\
        case 0x09: /* CM */							\
 	    state.cm = (int)(state.r[REG_2]>>3) & 3;					\
	    break;								\
        case 0x0b: /* IER_CM */							\
 	    state.cm = (int)(state.r[REG_2]>>3) & 3;					\
        case 0x0a: /* IER */							\
 	    state.asten = (int)(state.r[REG_2]>>13) & 1;					\
	    state.sien  = (int)(state.r[REG_2]>>14) & 0x3fff;				\
	    state.pcen  = (int)(state.r[REG_2]>>29) & 3;					\
	    state.cren  = (int)(state.r[REG_2]>>31) & 1;					\
	    state.slen  = (int)(state.r[REG_2]>>32) & 1;					\
	    state.eien  = (int)(state.r[REG_2]>>33) & 0x3f;					\
	    break;								\
        case 0x0c: /* SIRR */							\
 	    state.sir = (int)(state.r[REG_2]>>14) & 0x3fff;					\
	    break;								\
        case 0x0e: /* HW_INT_CLR */						\
	    state.pcr &= ~((state.r[REG_2]>>29)&X64(3));					\
	    state.crr &= ~((state.r[REG_2]>>31)&X64(1));					\
	    state.slr &= ~((state.r[REG_2]>>32)&X64(1));					\
	    break;								\
        case 0x10: /* PAL_BASE */						\
 	    state.pal_base = state.r[REG_2] & X64(00000fffffff8000);			\
	    break;								\
        case 0x11: /* i_ctl */							\
 	    state.i_ctl_other = state.r[REG_2]    & X64(00000000007e2f67);			\
	    state.i_ctl_vptb  = SEXT (state.r[REG_2] & X64(0000ffffc0000000),48);		\
	    state.i_ctl_spe   = (int)(state.r[REG_2]>>3) & 3;				\
	    state.sde         = (state.r[REG_2]>>7) & 1;					\
	    state.hwe         = (state.r[REG_2]>>12) & 1;					\
	    state.i_ctl_va_mode = (int)(state.r[REG_2]>>15) & 3;				\
	    break;								\
        case 0x12: /* ic_flush_asm */						\
 	    flush_icache_asm();							\
	    break;								\
        case 0x13: /* IC_FLUSH */						\
 	    flush_icache();							\
	    break;								\
        case 0x14: /* PCTR_CTL */						\
 	    state.pctr_ctl = state.r[REG_2] & X64(ffffffffffffffdf);			\
	    break;								\
        case 0x15: /* CLR_MAP */						\
        case 0x17: /* SLEEP   */						\
        case 0x27: /* MM_STAT */						\
        case 0x2b: /* C_DATA  */						\
        case 0x2c: /* C_SHIFT */						\
	  case 0x2d: /* M_FIX */						\
	    break;								\
        case 0x16: /* I_STAT */							\
 	    state.i_stat &= ~state.r[REG_2]; /* W1C */					\
	    break;								\
        case 0x20: /* DTB_TAG0 */						\
 	    dtb->write_tag(0,state.r[REG_2]);						\
	    break;								\
        case 0x21: /* DTB_PTE0 */						\
 	    dtb->write_pte(0,state.r[REG_2]);						\
	    break;								\
        case 0x24: /* DTB_IS0 */						\
 	    dtb->InvalidateSingle(state.r[REG_2]);					\
	    break;								\
        case 0x25: /* DTB_ASN0 */						\
 	    state.asn0 = (int)(state.r[REG_2] >> 56);					\
	    break;								\
        case 0x26: /* DTB_ALTMODE */						\
 	    state.alt_cm = (int)(state.r[REG_2] & 3);					\
	    break;								\
        case 0x28: /* M_CTL */							\
 	    state.smc = (int)(state.r[REG_2]>>4) & 3;					\
	    state.m_ctl_spe = (int)(state.r[REG_2]>>1) & 7;					\
	    break;								\
        case 0x29: /* DC_CTL */							\
 	    state.dc_ctl = state.r[REG_2];							\
	    break;								\
        case 0x2a: /* DC_STAT */						\
 	    state.dc_stat &= ~state.r[REG_2];						\
	    break;								\
        case 0xa0: /* DTB_TAG1 */						\
 	    dtb->write_tag(1,state.r[REG_2]);						\
	    break;								\
        case 0xa1: /* DTB_PTE1 */						\
 	    dtb->write_pte(1,state.r[REG_2]);						\
	    break;								\
        case 0xa2: /* DTB_IAP */						\
 	    dtb->InvalidateAllProcess();					\
	    break;								\
        case 0xa3: /* DTB_IA */							\
 	    dtb->InvalidateAll();						\
	    break;								\
        case 0xa4: /* DTB_IS1 */						\
 	    dtb->InvalidateSingle(state.r[REG_2]);					\
	    break;								\
        case 0xa5: /* DTB_ASN1 */						\
 	    state.asn1 = (int)(state.r[REG_2] >> 56);					\
	    break;								\
        case 0xc0: /* CC */							\
 	    state.cc_offset = (u32)(state.r[REG_2] >> 32);					\
	    break;								\
        case 0xc1: /* CC_CTL */							\
 	    state.cc_ena = (state.r[REG_2] >> 32) & 1;					\
	    state.cc    = (u32)(state.r[REG_2] & X64(fffffff0));				\
	    break;								\
        case 0xc4: /* VA_CTL */							\
 	    state.va_ctl_vptb = SEXT(state.r[REG_2] & X64(0000ffffc0000000),48);		\
	    state.va_ctl_va_mode = (int)(state.r[REG_2]>>1) & 3;				\
	    break;								\
        default:								\
	  UNKNOWN2;								\
      }										\
    }

#define DO_HW_RET state.pc = state.r[REG_2];

#define DO_HW_LDL								\
      switch(function)								\
        {									\
        case 0: /* longword physical */						\
	      phys_address = state.r[REG_2] + DISP_12;				\
	      state.r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        case 2: /* longword physical locked */					\
	      state.lock_flag = true;							\
	      phys_address = state.r[REG_2] + DISP_12;				\
	      state.r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        case 4: /* longword virtual vpte                 chk   alt    vpte */	\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, true, false, true);	\
	      state.r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        case 8: /* longword virtual */						\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, false, false, false);	\
	      state.r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        case 10: /* longword virtual check */					\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, true, false, false);	\
	      state.r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        case 12: /* longword virtual alt */					\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, false, true, false);	\
	      state.r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        case 14: /* longword virtual alt check */				\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, true, true, false);	\
	      state.r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        default:								\
	  UNKNOWN2;								\
        }

#define DO_HW_LDQ								\
      switch(function)								\
        {									\
        case 1: /* quadword physical */						\
	      phys_address = state.r[REG_2] + DISP_12;				\
	      state.r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        case 3: /* quadword physical locked */					\
	      state.lock_flag = true;							\
	      phys_address = state.r[REG_2] + DISP_12;				\
	      state.r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        case 5: /* quadword virtual vpte                 chk   alt    vpte */	\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, true, false, true);	\
	      state.r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        case 9: /* quadword virtual */						\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, false, false, false);	\
	      state.r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        case 11: /* quadword virtual check */					\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, true, false, false);	\
	      state.r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        case 13: /* quadword virtual alt */					\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, false, true, false);	\
	      state.r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        case 15: /* quadword virtual alt check */				\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, true, true, false);	\
	      state.r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        default:								\
	  UNKNOWN2;								\
        }

#define DO_HW_STL								\
      switch(function)								\
        {									\
        case 0: /* longword physical */						\
	      phys_address = state.r[REG_2] + DISP_12;				\
	      WRITE_PHYS_NT(state.r[REG_1],32);					\
	      break;								\
        case 2: /* longword physical conditional */				\
	      if (state.lock_flag) {							\
		  phys_address = state.r[REG_2] + DISP_12;				\
		  WRITE_PHYS_NT(state.r[REG_1],32);					\
		}								\
	      state.r[REG_1] = state.lock_flag?1:0;						\
	      state.lock_flag = false;						\
	      break;								\
        case 4: /* longword virtual                      chk   alt    vpte */	\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, false, false, false);	\
	      WRITE_PHYS_NT(state.r[REG_1],32);					\
	      break;								\
        case 12: /* longword virtual alt */					\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, false, true, false);	\
	      WRITE_PHYS_NT(state.r[REG_1],32);					\
	      break;								\
        default:								\
	  UNKNOWN2;								\
        }

#define DO_HW_STQ								\
      switch(function)								\
        {									\
        case 1: /* quadword physical */						\
	      phys_address = state.r[REG_2] + DISP_12;				\
	      WRITE_PHYS_NT(state.r[REG_1],64);					\
	      break;								\
        case 3: /* quadword physical conditional */				\
	      if (state.lock_flag) {							\
		  phys_address = state.r[REG_2] + DISP_12;				\
		  WRITE_PHYS_NT(state.r[REG_1],64);					\
		}								\
	      state.r[REG_1] = state.lock_flag?1:0;						\
	      state.lock_flag = false;						\
	      break;								\
        case 5: /* quadword virtual                      chk    alt    vpte */	\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, false, false, false);	\
	      WRITE_PHYS_NT(state.r[REG_1],64);					\
	      break;								\
        case 13: /* quadword virtual alt */					\
	      DATA_PHYS(state.r[REG_2] + DISP_12, ACCESS_READ, false, true, false);	\
	      WRITE_PHYS_NT(state.r[REG_1],64);					\
	      break;								\
        default:								\
	  UNKNOWN2;								\
        }

