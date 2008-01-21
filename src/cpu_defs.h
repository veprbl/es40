/* ES40 emulator.
 * Copyright (C) 2007-2008 by the ES40 Emulator Project
 *
 * WWW    : http://sourceforge.net/projects/es40
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
 * Contains some macro definitions and some inline functions for the Alpha CPU.
 *
 * $Id: cpu_defs.h,v 1.1 2008/01/21 22:39:25 iamcamiel Exp $
 *
 * X-1.1        Camiel Vanderhoeven                             21-JAN-2008
 *      File created. Contains code pulled from various older source files,
 *      and some floating-point definitions based upon the SIMH Alpha pre-
 *      implementation, which is Copyright (c) 2003, Robert M Supnik.
 **/

#if !defined(__CPU_DEFS__)
#define __CPU_DEFS__

/* Traps - corresponds to arithmetic trap summary register */

#define TRAP_SWC	X64(01)				/* software completion */
#define TRAP_INV	X64(02)				/* invalid operand */
#define TRAP_DZE	X64(04)				/* divide by zero */
#define TRAP_OVF	X64(08)				/* overflow */
#define TRAP_UNF	X64(10)				/* underflow */
#define TRAP_INE	X64(20)				/* inexact */
#define TRAP_IOV	X64(40)				/* integer overflow */

#define TRAP_INT    X64(80)             /* exception register is integer reg */

#define ARITH_TRAP(flags, reg)                                      \
    {                                                               \
      state.exc_sum = flags              /* cause of trap */        \
                    | (reg & 0x1f) << 8; /* destination register */ \
      GO_PAL(ARITH);                     /* trap */                 \
    }

/* Instruction formats */

#define I_V_OP		26				/* opcode */
#define I_M_OP		0x3F
#define I_OP		(I_M_OP << I_V_OP)
#define I_V_RA		21				/* Ra */
#define I_M_RA		0x1F
#define I_V_RB		16				/* Rb */
#define I_M_RB		0x1F
#define I_V_FTRP	13				/* floating trap mode */
#define I_M_FTRP	0x7
#define I_FTRP		(I_M_FTRP << I_V_FTRP)
#define  I_F_VAXRSV	0x4800				/* VAX reserved */
#define  I_FTRP_V	0x2000				/* /V trap */
#define  I_FTRP_U	0x2000				/* /U trap */
#define  I_FTRP_S	0x8000				/* /S trap */
#define  I_FTRP_SUI	0xE000				/* /SUI trap */
#define  I_FTRP_SVI	0xE000				/* /SVI trap */
#define I_V_FRND	11				/* floating round mode */
#define I_M_FRND	0x3
#define I_FRND		(I_M_FRND << I_V_FRND)
#define  I_FRND_C	0				/* chopped */
#define  I_FRND_M	1				/* to minus inf */
#define  I_FRND_N	2				/* normal */
#define  I_FRND_D	3				/* dynamic */
#define  I_FRND_P	3				/* in FPCR: plus inf */
#define I_V_FSRC	9				/* floating source */
#define I_M_FSRC	0x3
#define I_FSRC		(I_M_FSRC << I_V_FSRC)
#define  I_FSRC_X	0x0200				/* data type X */
#define I_V_FFNC	5				/* floating function */
#define I_M_FFNC	0x3F
#define I_V_LIT8	13				/* integer 8b literal */
#define I_M_LIT8	0xFF
#define I_V_ILIT	12				/* literal flag */
#define I_ILIT		(1u << I_V_ILIT)
#define I_V_IFNC	5				/* integer function */
#define I_M_IFNC	0x3F
#define I_V_RC		0				/* Rc */
#define I_M_RC		0x1F
#define I_V_MDSP	0				/* memory displacement */
#define I_M_MDSP	0xFFFF
#define I_V_BDSP	0
#define I_M_BDSP	0x1FFFFF			/* branch displacement */
#define I_V_PALOP	0
#define I_M_PALOP	0x3FFFFFF			/* PAL subopcode */
#define I_GETOP(x)	(((x) >> I_V_OP) & I_M_OP)
#define I_GETRA(x)	(((x) >> I_V_RA) & I_M_RA)
#define I_GETRB(x)	(((x) >> I_V_RB) & I_M_RB)
#define I_GETLIT8(x)	(((x) >> I_V_LIT8) & I_M_LIT8)
#define I_GETIFNC(x)	(((x) >> I_V_IFNC) & I_M_IFNC)
#define I_GETFRND(x)	(((x) >> I_V_FRND) & I_M_FRND)
#define I_GETFFNC(x)	(((x) >> I_V_FFNC) & I_M_FFNC)
#define I_GETRC(x)	(((x) >> I_V_RC) & I_M_RC)
#define I_GETMDSP(x)	(((x) >> I_V_MDSP) & I_M_MDSP)
#define I_GETBDSP(x)	(((x) >> I_V_BDSP) & I_M_BDSP)
#define I_GETPAL(x)	(((x) >> I_V_PALOP) & I_M_PALOP)

/* Floating point types */

#define DT_F		0				/* type F */
#define DT_G		1				/* type G */
#define DT_S		0				/* type S */
#define DT_T		1				/* type T */

/* Floating point memory format (VAX F) */

#define F_V_SIGN	15
#define F_SIGN		(1u << F_V_SIGN)
#define F_V_EXP		7
#define F_M_EXP		0xFF
#define F_BIAS		0x80
#define F_EXP		(F_M_EXP << F_V_EXP)
#define F_V_FRAC	29
#define F_GETEXP(x)	(((x) >> F_V_EXP) & F_M_EXP)
#define SWAP_VAXF(x)	((((x) >> 16) & 0xFFFF) | (((x) & 0xFFFF) << 16))

/* Floating point memory format (VAX G) */

#define G_V_SIGN	15
#define G_SIGN		(1u << F_V_SIGN)
#define G_V_EXP		4
#define G_M_EXP		0x7FF
#define G_BIAS		0x400
#define G_EXP		(G_M_EXP << G_V_EXP)
#define G_GETEXP(x)	(((x) >> G_V_EXP) & G_M_EXP)
#define SWAP_VAXG(x)	((((x) & X64(000000000000FFFF)) << 48) | \
			 (((x) & X64(00000000FFFF0000)) << 16) | \
			 (((x) >> 16) & X64(00000000FFFF0000)) | \
			 (((x) >> 48) & X64(000000000000FFFF)))

/* Floating memory format (IEEE S) */

#define S_V_SIGN	31
#define S_SIGN		(1u << S_V_SIGN)
#define S_V_EXP		23
#define S_M_EXP		0xFF
#define S_BIAS		0x7F
#define S_NAN		0xFF
#define S_EXP		(S_M_EXP << S_V_EXP)
#define S_V_FRAC	29
#define S_GETEXP(x)	(((x) >> S_V_EXP) & S_M_EXP)

/* Floating point memory format (IEEE T) */

#define T_V_SIGN	63
#define T_SIGN		X64(8000000000000000)
#define T_V_EXP		52
#define T_M_EXP		0x7FF
#define T_BIAS		0x3FF
#define T_NAN		0x7FF
#define T_EXP		X64(7FF0000000000000)
#define T_FRAC		X64(000FFFFFFFFFFFFF)
#define T_GETEXP(x)	(((u32) ((x) >> T_V_EXP)) & T_M_EXP)

/* Floating point register format (all except VAX D) */

#define FPR_V_SIGN	63
#define FPR_SIGN	X64(8000000000000000)
#define FPR_V_EXP	52
#define FPR_M_EXP	0x7FF
#define FPR_NAN		0x7FF
#define FPR_EXP		X64(7FF0000000000000)
#define FPR_HB		X64(0010000000000000)
#define FPR_FRAC	X64(000FFFFFFFFFFFFF)
#define FPR_GUARD	(UF_V_NM - FPR_V_EXP)
#define FPR_GETSIGN(x)	(((u32) ((x) >> FPR_V_SIGN)) & 1)
#define FPR_GETEXP(x)	(((u32) ((x) >> FPR_V_EXP)) & FPR_M_EXP)
#define FPR_GETFRAC(x)	((x) & FPR_FRAC)

#define FP_TRUE		X64(4000000000000000)		/* 0.5/2.0 in reg */

/* Floating point register format (VAX D) */

#define FDR_V_SIGN	63
#define FDR_SIGN	X64(8000000000000000)
#define FDR_V_EXP	55
#define FDR_M_EXP	0xFF
#define FDR_EXP		X64(7F80000000000000)
#define FDR_HB		X64(0080000000000000)
#define FDR_FRAC	X64(007FFFFFFFFFFFFF)
#define FDR_GUARD	(UF_V_NM - FDR_V_EXP)
#define FDR_GETSIGN(x)	(((u32) ((x) >> FDR_V_SIGN)) & 1)
#define FDR_GETEXP(x)	(((u32) ((x) >> FDR_V_EXP)) & FDR_M_EXP)
#define FDR_GETFRAC(x)	((x) & FDR_FRAC)

#define D_BIAS		0x80

/* Unpacked floating point number */

struct ufp {
	u32		sign;
	s32		exp;
	u64	frac;  };

typedef struct ufp UFP;

#define UF_V_NM		63
#define UF_NM		0x8000000000000000		/* normalized */

/* Bit patterns */

#define X64_BYTE	X64(ff)
#define X64_WORD	X64(ffff)
#define X64_LONG	X64(ffffffff)
#define X64_QUAD	X64(ffffffffffffffff)

#define B_SIGN		X64(80)
#define W_SIGN		X64(8000)
#define L_SIGN		X64(80000000)
#define Q_SIGN		X64(8000000000000000)
#define Q_GETSIGN(x)	(((x) >> 63) & 1)

/* IEEE control register (left 32b only) */

#define FPCR_SUM	X64(8000000000000000)			/* summary */
#define FPCR_INED	X64(4000000000000000)			/* inexact disable */
#define FPCR_UNFD	X64(2000000000000000)			/* underflow disable */
#define FPCR_UNDZ	X64(1000000000000000)			/* underflow to 0 */
#define FPCR_V_RMOD	58              				/* rounding mode */
#define FPCR_M_RMOD	0x3
#define FPCR_IOV	X64(0200000000000000)			/* integer overflow */
#define FPCR_INE	X64(0100000000000000)			/* inexact */
#define FPCR_UNF	X64(0080000000000000)			/* underflow */
#define FPCR_OVF	X64(0040000000000000)			/* overflow */
#define FPCR_DZE	X64(0020000000000000)			/* div by zero */
#define FPCR_INV	X64(0010000000000000)			/* invalid operation */
#define FPCR_OVFD	X64(0008000000000000)			/* overflow disable */
#define FPCR_DZED	X64(0004000000000000)			/* div by zero disable */
#define FPCR_INVD	X64(0002000000000000)			/* invalid op disable */
#define FPCR_DNZ	X64(0001000000000000)			/* denormal to zero */
#define FPCR_DNOD	X64(0000800000000000)			/* denormal disable */
#define FPCR_RAZ	X64(00007FFF00000000)			/* zero */
#define FPCR_ERR	(FPCR_IOV|FPCR_INE|FPCR_UNF|FPCR_OVF|FPCR_DZE|FPCR_INV)
#define FPCR_GETFRND(x)	(((x) >> FPCR_V_RMOD) & FPCR_M_RMOD)

#define NEG_Q(x)	((~(x) + 1) & X64_QUAD)
#define ABS_Q(x)	(((x) & Q_SIGN)? NEG_Q (x): (x))

/* IEEE */

#define UFT_ZERO	0				/* unpacked: zero */
#define UFT_FIN		1				/* finite */
#define UFT_DENORM	2				/* denormal */
#define UFT_INF		3				/* infinity */
#define UFT_NAN		4				/* not a number */

#define Q_FINITE(x)	((x) <= UFT_FIN)		/* finite */
#define Q_SUI(x)	(((x) & I_FTRP) == I_FTRP_SVI)

/* 64b * 64b unsigned multiply */

inline u64 uemul64 (u64 a, u64 b, u64 *hi)
{
u64 ahi, alo, bhi, blo, rhi, rmid1, rmid2, rlo;

ahi = (a >> 32) & X64_LONG;
alo = a & X64_LONG;
bhi = (b >> 32) & X64_LONG;
blo = b & X64_LONG;
rhi = ahi * bhi;
rmid1 = ahi * blo;
rmid2 = alo * bhi;
rlo = alo * blo;
rhi = rhi + ((rmid1 >> 32) & X64_LONG) + ((rmid2 >> 32) & X64_LONG);
rmid1 = (rmid1 << 32) & X64_QUAD;
rmid2 = (rmid2 << 32) & X64_QUAD;
rlo = (rlo + rmid1) & X64_QUAD;
if (rlo < rmid1) rhi = rhi + 1;
rlo = (rlo + rmid2) & X64_QUAD;
if (rlo < rmid2) rhi = rhi + 1;
if (hi) *hi = rhi & X64_QUAD;
return rlo;
}

/* 64b / 64b unsigned fraction divide */

inline u64 ufdiv64 (u64 dvd, u64 dvr, u32 prec, u32 *sticky)
{
u64 quo;
u32 i;

quo = 0;						/* clear quotient */
for (i = 0; (i < prec) && dvd; i++) {			/* divide loop */
	quo = quo << 1;					/* shift quo */
	if (dvd >= dvr) {				/* div step ok? */
	    dvd = dvd - dvr;				/* subtract */
	    quo = quo + 1;  }				/* quo bit = 1 */
	dvd = dvd << 1;  }				/* shift divd */
quo = quo << (UF_V_NM - i + 1);				/* shift quo */
if (sticky) *sticky = (dvd? 1: 0);			/* set sticky bit */
return quo;						/* return quotient */
}

// INTERRUPT VECTORS
#define DTBM_DOUBLE_3 X64(100)
#define DTBM_DOUBLE_4 X64(180)
#define FEN           X64(200)
#define UNALIGN       X64(280)
#define DTBM_SINGLE   X64(300)
#define DFAULT        X64(380)
#define OPCDEC        X64(400)
#define IACV          X64(480)
#define MCHK          X64(500)
#define ITB_MISS      X64(580)
#define ARITH         X64(600)
#define INTERRUPT     X64(680)
#define MT_FPCR       X64(700)
#define RESET         X64(780)

/** Chip ID (EV68CB pass 4) [HRM p 5-16]; actual value derived from SRM-code */
#define CPU_CHIP_ID	0x21
/** Major CPU type (EV68CB) [ARM pp D-1..3] */
#define CPU_TYPE_MAJOR	12
/** Minor CPU type (pass 4) [ARM pp D-1..3] */
#define CPU_TYPE_MINOR	6
/** Implementation version [HRM p 2-38; ARM p D-5] */
#define CPU_IMPLVER	2
/** Architecture mask [HRM p 2-38; ARM p D-4]; FIX not implemented */
#define CPU_AMASK	X64(1305)

#define DISP_12 (sext_u64_12(ins))
#define DISP_13 (sext_u64_13(ins))
#define DISP_16 (sext_u64_16(ins))
#define DISP_21 (sext_u64_21(ins))

#define DATA_PHYS(addr,flags) 				\
    if (virt2phys(addr, &phys_address, flags, NULL, ins)) \
      return 0

#define ALIGN_PHYS(a) (phys_address & ~((u64)((a)-1)))

/**
 * Normal variant of read action
 * In reality, these would generate an alignment trap, and the exception
 * handler would put things straight. Instead, to speed things up, we'll
 * just perform the read as requested using the unaligned address.
 **/

#define READ_PHYS(size)				\
  cSystem->ReadMem(phys_address, size)

/**
 * Normal variant of write action
 * In reality, these would generate an alignment trap, and the exception
 * handler would put things straight. Instead, to speed things up, we'll
 * just perform the write as requested using the unaligned address.
 **/

#define WRITE_PHYS(data,size)			\
  cSystem->WriteMem(phys_address, size, data)

/**
 * NO-TRAP (NT) variants of read action.
 * This is used for HW_LD, where alignment traps are 
 * inhibited. We'll align the adress and read using the aligned
 * address.
 **/

#define READ_PHYS_NT(size)			\
  cSystem->ReadMem(ALIGN_PHYS((size)/8), size)

/**
 * NO-TRAP (NT) variants of write action.
 * This is used for HW_ST, where alignment traps are 
 * inhibited. We'll align the adress and write using the aligned
 * address.
 **/

#define WRITE_PHYS_NT(data,size) 				\
    cSystem->WriteMem(ALIGN_PHYS((size)/8), size, data)

#define REG_1 RREG(ins>>21)
#define REG_2 RREG(ins>>16)
#define REG_3 RREG(ins)

#define FREG_1 ((ins>>21) & 0x1f)
#define FREG_2 ((ins>>16) & 0x1f)
#define FREG_3 ( ins      & 0x1f)

#define V_2 ( (ins&0x1000)?((ins>>13)&0xff):state.r[REG_2] )

#endif