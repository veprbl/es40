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
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#define DO_HW_MFPR						\
      if ((function & 0xc0) == 0x40) {	/* PCTX */		\
	  if (DO_ACTION)					\
 	    r[REG_1] = ((u64)asn << 39)				\
	      | ((u64)astrr << 9)				\
	      | ((u64)aster <<5)				\
	      | (fpen?X64(1)<<3:0)				\
	      | (ppcen?X64(1)<<1:0);				\
      } else {							\
	switch (function)					\
        {							\
        case 0x05: /* PMPC     */				\
 	    r[REG_1] = pmpc;					\
	    break;						\
        case 0x06: /* EXC_ADDR */				\
 	    r[REG_1] = exc_addr;				\
	    break;						\
        case 0x07: /* IVA_FORM */				\
 	    r[REG_1] = va_form(exc_addr,i_ctl_va_mode,i_ctl_vptb);	\
	    break;						\
        case 0x08: /* IER_CM   */				\
        case 0x09: /* CM       */				\
        case 0x0a: /* IER      */				\
        case 0x0b: /* IER_CM   */				\
 	    r[REG_1] = (((u64)eien) << 33)			\
	      | (((u64)slen) << 32)				\
	      | (((u64)cren) << 31)				\
	      | (((u64)pcen) << 29)				\
	      | (((u64)sien) << 14)				\
	      | (((u64)asten) << 13)				\
	      | (((u64)cm) << 3);				\
	    break;						\
        case 0x0c: /* SIRR */					\
 	    r[REG_1] = ((u64)sir) << 14;			\
	    break;						\
        case 0x0d: /* ISUM */					\
 	    r[REG_1] = (((u64)(eir & eien)) << 33)		\
	      | (((u64)(slr & slen)) << 32)			\
	      | (((u64)(crr & cren)) << 31)			\
	      | (((u64)(pcr & pcen)) << 29)			\
	      | (((u64)(sir & sien)) << 14)			\
	      | (((u64)( ((X64(1)<<(cm+1))-1) & aster & astrr & (asten * 0x3))) << 3)	\
	      | (((u64)( ((X64(1)<<(cm+1))-1) & aster & astrr & (asten * 0xc))) << 7);	\
	    break;						\
        case 0x0f: /* EXC_SUM */				\
 	    r[REG_1] = exc_sum;					\
	    break;						\
        case 0x10: /* PAL_BASE */				\
 	    r[REG_1] = pal_base;				\
	    break;						\
        case 0x11: /* i_ctl */					\
 	    r[REG_1] = i_ctl_other				\
	      | (((u64)CPU_CHIP_ID)<<24)			\
	      | (u64)i_ctl_vptb					\
	      | (((u64)i_ctl_va_mode) << 15)			\
	      | (hwe?X64(1)<<12:0)				\
	      | (sde?X64(1)<<7:0)				\
	      | (((u64)i_ctl_spe) << 3);			\
	    break;						\
        case 0x14: /* PCTR_CTL */				\
 	    r[REG_1] = pctr_ctl;				\
	    break;						\
        case 0x16: /* I_STAT */					\
 	    r[REG_1] = i_stat;					\
	    break;						\
        case 0x27: /* MM_STAT */				\
 	    r[REG_1] = mm_stat;					\
	    break;						\
        case 0x2a: /* DC_STAT */				\
 	    r[REG_1] = dc_stat;					\
	    break;						\
        case 0x2b: /* C_DATA */					\
 	    r[REG_1] = 0;					\
	    break;						\
        case 0xc0: /* CC */					\
 	    r[REG_1] = (((u64)cc_offset) << 32) |  cc;		\
	    break;						\
        case 0xc2: /* VA */					\
 	    r[REG_1] = fault_va;				\
	    break;						\
        case 0xc3: /* VA_FORM */				\
 	    r[REG_1] = va_form(fault_va, va_ctl_va_mode, va_ctl_vptb);	\
	    break;						\
        default:						\
	  UNKNOWN2;						\
        }							\
      }


#define DO_HW_MTPR								\
    if ((function & 0xc0) == 0x40) {						\
	    if (function & 1)							\
	      asn = (int)(r[REG_2]>>39) & 0xff;					\
	    if (function & 2)							\
	      aster = (int)(r[REG_2]>>5) & 0xf;					\
	    if (function & 4)							\
	      astrr = (int)(r[REG_2]>>9) & 0xf;					\
	    if (function & 8)							\
	      ppcen = (int)(r[REG_2]>>1) & 1;					\
	    if (function & 16)							\
	      fpen = (int)(r[REG_2]>>3) & 1;					\
    } else {									\
      switch (function)								\
        {									\
        case 0x00: /* ITB_TAG */						\
 	    itb->write_tag(0,r[REG_2]);						\
	    break;								\
        case 0x01: /* ITB_PTE */						\
 	    itb->write_pte(0,r[REG_2],asn);				\
            break;								\
        case 0x02: /* ITB_IAP */						\
 	    itb->InvalidateAllProcess();					\
	    break;								\
        case 0x03: /* ITB_IA */							\
 	    itb->InvalidateAll();						\
	    break;								\
        case 0x04: /* ITB_IS */							\
 	    itb->InvalidateSingle(r[REG_2],asn);				\
	    break;								\
        case 0x09: /* CM */							\
 	    cm = (int)(r[REG_2]>>3) & 3;					\
	    break;								\
        case 0x0b: /* IER_CM */							\
 	    cm = (int)(r[REG_2]>>3) & 3;					\
        case 0x0a: /* IER */							\
 	    asten = (int)(r[REG_2]>>13) & 1;					\
	    sien  = (int)(r[REG_2]>>14) & 0x3fff;				\
	    pcen  = (int)(r[REG_2]>>29) & 3;					\
	    cren  = (int)(r[REG_2]>>31) & 1;					\
	    slen  = (int)(r[REG_2]>>32) & 1;					\
	    eien  = (int)(r[REG_2]>>33) & 0x3f;					\
	    break;								\
        case 0x0c: /* SIRR */							\
 	    sir = (int)(r[REG_2]>>14) & 0x3fff;					\
	    break;								\
        case 0x0e: /* HW_INT_CLR */						\
	    pcr &= ~((r[REG_2]>>29)&X64(3));					\
	    crr &= ~((r[REG_2]>>31)&X64(1));					\
	    slr &= ~((r[REG_2]>>32)&X64(1));					\
	    break;								\
        case 0x10: /* PAL_BASE */						\
 	    pal_base = r[REG_2] & X64(00000fffffff8000);			\
	    break;								\
        case 0x11: /* i_ctl */							\
 	    i_ctl_other = r[REG_2]    & X64(00000000007e2f67);			\
	    i_ctl_vptb  = SEXT (r[REG_2] & X64(0000ffffc0000000),48);		\
	    i_ctl_spe   = (int)(r[REG_2]>>3) & 3;				\
	    sde         = (r[REG_2]>>7) & 1;					\
	    hwe         = (r[REG_2]>>12) & 1;					\
	    i_ctl_va_mode = (int)(r[REG_2]>>15) & 3;				\
	    break;								\
        case 0x12: /* ic_flush_asm */						\
 	    flush_icache_asm();							\
	    break;								\
        case 0x13: /* IC_FLUSH */						\
 	    flush_icache();							\
	    break;								\
        case 0x14: /* PCTR_CTL */						\
 	    pctr_ctl = r[REG_2] & X64(ffffffffffffffdf);			\
	    break;								\
        case 0x15: /* CLR_MAP */						\
        case 0x17: /* SLEEP   */						\
        case 0x27: /* MM_STAT */						\
        case 0x2b: /* C_DATA  */						\
        case 0x2c: /* C_SHIFT */						\
	    break;								\
        case 0x16: /* I_STAT */							\
 	    i_stat &= ~r[REG_2]; /* W1C */					\
	    break;								\
        case 0x20: /* DTB_TAG0 */						\
 	    dtb->write_tag(0,r[REG_2]);						\
	    break;								\
        case 0x21: /* DTB_PTE0 */						\
 	    dtb->write_pte(0,r[REG_2],asn0);					\
	    break;								\
        case 0x24: /* DTB_IS0 */						\
 	    dtb->InvalidateSingle(r[REG_2],asn0);				\
	    break;								\
        case 0x25: /* DTB_ASN0 */						\
 	    asn0 = (int)(r[REG_2] >> 56);					\
	    break;								\
        case 0x26: /* DTB_ALTMODE */						\
 	    alt_cm = (int)(r[REG_2] & 3);					\
	    break;								\
        case 0x28: /* M_CTL */							\
 	    smc = (int)(r[REG_2]>>4) & 3;					\
	    m_ctl_spe = (int)(r[REG_2]>>1) & 7;					\
	    break;								\
        case 0x29: /* DC_CTL */							\
 	    dc_ctl = r[REG_2];							\
	    break;								\
        case 0x2a: /* DC_STAT */						\
 	    dc_stat &= ~r[REG_2];						\
	    break;								\
	case 0x2d: /* NEED TO FIND OUT WHAT THIS IS!!!!!! */			\
	    break;								\
        case 0xa0: /* DTB_TAG1 */						\
 	    dtb->write_tag(1,r[REG_2]);						\
	    break;								\
        case 0xa1: /* DTB_PTE1 */						\
 	    dtb->write_pte(1,r[REG_2],asn1);					\
	    break;								\
        case 0xa2: /* DTB_IAP */						\
 	    dtb->InvalidateAllProcess();					\
	    break;								\
        case 0xa3: /* DTB_IA */							\
 	    dtb->InvalidateAll();						\
	    break;								\
        case 0xa4: /* DTB_IS1 */						\
 	    dtb->InvalidateSingle(r[REG_2],asn1);				\
	    break;								\
        case 0xa5: /* DTB_ASN1 */						\
 	    asn1 = (int)(r[REG_2] >> 56);					\
	    break;								\
        case 0xc0: /* CC */							\
 	    cc_offset = (u32)(r[REG_2] >> 32);					\
	    break;								\
        case 0xc1: /* CC_CTL */							\
 	    cc_ena = (r[REG_2] >> 32) & 1;					\
	    cc    = (u32)(r[REG_2] & X64(fffffff0));				\
	    break;								\
        case 0xc4: /* VA_CTL */							\
 	    va_ctl_vptb = SEXT(r[REG_2] & X64(0000ffffc0000000),48);		\
	    va_ctl_va_mode = (int)(r[REG_2]>>1) & 3;				\
	    break;								\
        default:								\
	  UNKNOWN2;								\
      }										\
    }

#define DO_HW_RET pc = r[REG_2];

#define DO_HW_LDL								\
      switch(function)								\
        {									\
        case 0: /* longword physical */						\
	      phys_address = r[REG_2] + DISP_12;				\
	      r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        case 2: /* longword physical locked */					\
	      lock_flag = true;							\
	      phys_address = r[REG_2] + DISP_12;				\
	      r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        case 4: /* longword virtual vpte                 chk   alt    vpte */	\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, false, true);	\
	      r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        case 8: /* longword virtual */						\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, false, false);	\
	      r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        case 10: /* longword virtual check */					\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, false, false);	\
	      r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        case 12: /* longword virtual alt */					\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, true, false);	\
	      r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        case 14: /* longword virtual alt check */				\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, true, false);	\
	      r[REG_1] = READ_PHYS_NT(32);					\
	      break;								\
        default:								\
	  UNKNOWN2;								\
        }

#define DO_HW_LDQ								\
      switch(function)								\
        {									\
        case 1: /* quadword physical */						\
	      phys_address = r[REG_2] + DISP_12;				\
	      r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        case 3: /* quadword physical locked */					\
	      lock_flag = true;							\
	      phys_address = r[REG_2] + DISP_12;				\
	      r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        case 5: /* quadword virtual vpte                 chk   alt    vpte */	\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, false, true);	\
	      r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        case 9: /* quadword virtual */						\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, false, false);	\
	      r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        case 11: /* quadword virtual check */					\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, false, false);	\
	      r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        case 13: /* quadword virtual alt */					\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, true, false);	\
	      r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        case 15: /* quadword virtual alt check */				\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, true, true, false);	\
	      r[REG_1] = READ_PHYS_NT(64);					\
	      break;								\
        default:								\
	  UNKNOWN2;								\
        }

#define DO_HW_STL								\
      switch(function)								\
        {									\
        case 0: /* longword physical */						\
	      phys_address = r[REG_2] + DISP_12;				\
	      WRITE_PHYS_NT(r[REG_1],32);					\
	      break;								\
        case 2: /* longword physical conditional */				\
	      if (lock_flag) {							\
		  phys_address = r[REG_2] + DISP_12;				\
		  WRITE_PHYS_NT(r[REG_1],32);					\
		}								\
	      r[REG_1] = lock_flag?1:0;						\
	      lock_flag = false;						\
	      break;								\
        case 4: /* longword virtual                      chk   alt    vpte */	\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, false, false);	\
	      WRITE_PHYS_NT(r[REG_1],32);					\
	      break;								\
        case 12: /* longword virtual alt */					\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, true, false);	\
	      WRITE_PHYS_NT(r[REG_1],32);					\
	      break;								\
        default:								\
	  UNKNOWN2;								\
        }

#define DO_HW_STQ								\
      switch(function)								\
        {									\
        case 1: /* quadword physical */						\
	      phys_address = r[REG_2] + DISP_12;				\
	      WRITE_PHYS_NT(r[REG_1],64);					\
	      break;								\
        case 3: /* quadword physical conditional */				\
	      if (lock_flag) {							\
		  phys_address = r[REG_2] + DISP_12;				\
		  WRITE_PHYS_NT(r[REG_1],64);					\
		}								\
	      r[REG_1] = lock_flag?1:0;						\
	      lock_flag = false;						\
	      break;								\
        case 5: /* quadword virtual                      chk    alt    vpte */	\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, false, false);	\
	      WRITE_PHYS_NT(r[REG_1],64);					\
	      break;								\
        case 13: /* quadword virtual alt */					\
	      DATA_PHYS(r[REG_2] + DISP_12, ACCESS_READ, false, true, false);	\
	      WRITE_PHYS_NT(r[REG_1],64);					\
	      break;								\
        default:								\
	  UNKNOWN2;								\
        }

