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
 * $Id: cpu_defs.h,v 1.9 2008/02/05 15:55:11 iamcamiel Exp $
 *
 * X-1.9        Camiel Vanderhoeven                             05-FEB-2008
 *      Bug description added.
 *
 * X-1.8        Camiel Vanderhoeven                             01-FEB-2008
 *      Disable unaligned access check alltogether; it doesn't work
 *      properly for some reason.
 *
 * X-1.7        Camiel Vanderhoeven                             01-FEB-2008
 *      Avoid unnecessary shift-operations to calculate constant values.
 *
 * X-1.6        Camiel Vanderhoeven                             28-JAN-2008
 *      Better floating-point exception handling.
 *
 * X-1.5        Brian Wheeler                                   26-JAN-2008
 *      Make file end in newline.
 *
 * X-1.4        Camiel Vanderhoeven                             26-JAN-2008
 *      Do unaligned trap only when a page boundary is crossed. Something
 *      is causing alignment traps in the SRM console, with the DAT bit set
 *      to false, and no OS handler in place. Also, when OpenVMS boots there
 *      are alignment traps that shouldn't happen. None of these cross page
 *      boundaries, so we're safe for now.
 *
 * X-1.3        Camiel Vanderhoeven                             25-JAN-2008
 *      Trap on unalogned memory access. The previous implementation where
 *      unaligned accesses were silently allowed could go wrong when page
 *      boundaries are crossed.
 *
 * X-1.2        Camiel Vanderhoeven                             22-JAN-2008
 *      Added RA, RAV style macro's for integer registers.
 *
 * X-1.1        Camiel Vanderhoeven                             21-JAN-2008
 *      File created. Contains code pulled from various older source files,
 *      and some floating-point definitions based upon the SIMH Alpha pre-
 *      implementation, which is Copyright (c) 2003, Robert M Supnik.
 *
 * \bug Fix unaligned access traps.
 **/

#if !defined(__CPU_DEFS__)
#define __CPU_DEFS__

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
#define UF_NM		X64(8000000000000000)		/* normalized */

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

/* Fraction square root routine - code from SoftFloat */
inline u64 fsqrt64 (u64 asig, s32 exp)
{
static const u32 sqrtOdd[] = {
	0x0004, 0x0022, 0x005D, 0x00B1, 0x011D, 0x019F, 0x0236, 0x02E0,
	0x039C, 0x0468, 0x0545, 0x0631, 0x072B, 0x0832, 0x0946, 0x0A67 };
static const u32 sqrtEven[] = {
	0x0A2D, 0x08AF, 0x075A, 0x0629, 0x051A, 0x0429, 0x0356, 0x029E,
	0x0200, 0x0179, 0x0109, 0x00AF, 0x0068, 0x0034, 0x0012, 0x0002 };

u64 zsig, remh, reml, t;
u32 index, z, a, sticky = 0;

/* Calculate an approximation to the square root of the 32-bit significand given
   by 'a'.  Considered as an integer, 'a' must be at least 2^31.  If bit 0 of
   'exp' (the least significant bit) is 1, the integer returned approximates
   2^31*sqrt('a'/2^31), where 'a' is considered an integer.  If bit 0 of 'exp'
   is 0, the integer returned approximates 2^31*sqrt('a'/2^30).  In either
   case, the approximation returned lies strictly within +/-2 of the exact
   value. */

a = (u32) (asig >> 32);				/* high order frac */
index = (a >> 27) & 0xF;				/* bits<30:27> */
if (exp & 1) {						/* odd exp? */
	z = 0x4000 + (a >> 17) - sqrtOdd[index];	/* initial guess */
	z = ((a / z) << 14) + (z << 15);		/* Newton iteration */
	a = a >> 1;  }
else {	z = 0x8000 + (a >> 17) - sqrtEven[index];	/* initial guess */
        z = (a / z) + z;				/* Newton iteration */
        z = (z >= 0x20000) ? 0xFFFF8000: (z << 15);
        if (z <= a) z = (a >> 1) | 0x80000000;      }
zsig = (((((u64) a) << 31) / ((u64) z)) + (z >> 1)) & X64_LONG;

/* Calculate the final answer in two steps.  First, do one iteration of
   Newton's approximation.  The divide-by-2 is accomplished by clever
   positioning of the operands.  Then, check the bits just below the
   (double precision) rounding bit to see if they are close to zero
   (that is, the rounding bits are close to midpoint).  If so, make
   sure that the result^2 is <below> the input operand */

asig = asig >> ((exp & 1)? 3: 2);			/* leave 2b guard */
zsig = ufdiv64 (asig, zsig << 32, 64, NULL) + (zsig << 30); /* Newton iteration */
if ((zsig & 0x1FF) <= 5) {				/* close to even? */
	remh = uemul64 (zsig, zsig, &reml);		/* result^2 */
	remh = (asig - remh - (reml? 1:0)) & X64_QUAD;	/* arg - result^2 */
	reml = NEG_Q (reml);
	while (Q_GETSIGN (remh) != 0) {			/* if arg < result^2 */
            zsig = (zsig - 1) & X64_QUAD;			/* decr result */
	    t = ((zsig << 1) & X64_QUAD) | 1;		/* incr result^2 */
	    reml = (reml + t) & X64_QUAD;			/* and retest */
	    remh = (remh + (zsig >> 63) + ((reml < t)? 1: 0)) & X64_QUAD;  }
        if ((remh | reml) != 0 ) sticky = 1;  }		/* not exact? */
zsig = (zsig << 1) | sticky;				/* left justify result */
return zsig;
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

#define DATA_PHYS(addr,flags,align)                       \
  if ((addr) & (align))                                   \
  {                                                       \
    u64 a1 = (addr);                                      \
    u64 a2 = (addr) + (align);                            \
    if ((a1 ^ a2) & ~X64(fff)) /*page boundary crossed*/  \
    {                                                     \
      state.fault_va = addr;                              \
	  state.exc_sum = ((REG_1 & 0x1f) << 8);  			  \
      state.mm_stat = I_GETOP(ins)<<4                     \
                    | ((flags&ACCESS_WRITE)?1:0);         \
      printf("data_phys %" LL "x, %d, %d -> trap!\n",addr,flags,align); \
      printf("exc_sum = %" LL "x, fault_va = %" LL "x, mm_stat = %" LL "x.\n",state.exc_sum, state.fault_va, state.mm_stat);  \
      printf("datfx_qword = %016" LL "x.\n",cSystem->ReadMem(cSystem->ReadMem(state.r[32+21]+0x10,64)+0x38,64));  \
      if (cSystem->ReadMem(state.r[32+21]+0x170,64)==0) printf("ignored; no OS!\n"); else                        \
      GO_PAL(UNALIGN);                                    \
    }                                                     \
  }                                                       \
  if (virt2phys(addr, &phys_address, flags, NULL, ins))   \
    return 0


#define DATA_PHYS_NT(addr,flags) 				          \
    if (virt2phys(addr, &phys_address, flags, NULL, ins)) \
      return 0

#undef DATA_PHYS
#define DATA_PHYS(addr,flags,align) DATA_PHYS_NT(addr,flags)

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

#define REG_1 RREG(I_GETRA(ins))
#define REG_2 RREG(I_GETRB(ins))
#define REG_3 RREG(I_GETRC(ins))

#define FREG_1 (I_GETRA(ins))
#define FREG_2 (I_GETRB(ins))
#define FREG_3 (I_GETRC(ins))

#define RA REG_1
#define RAV state.r[RA]
#define RB REG_2
#define RBV ( (ins&0x1000)?((ins>>13)&0xff):state.r[RB] )
#define V_2 RBV
#define RC REG_3
#define RCV state.r[RC]

#define ACCESS_READ  0
#define ACCESS_WRITE 1
#define ACCESS_EXEC  2
#define ACCESS_MODE  3
#define NO_CHECK     4
#define VPTE         8
#define FAKE        16
#define ALT         32
#define RECUR      128
#define PROBE      256
#define PROBEW     512

#define FPSTART                                     \
  if (state.fpen == 0) /* flt point disabled? */    \
  {                                                 \
    GO_PAL (FEN);	/* set trap */                  \
    break;      /* and stop current instruction */  \
  }                                                 \
  state.exc_sum = 0

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
  {                                                                 \
    state.exc_sum |= flags;             /* cause of trap */         \
    state.exc_sum |= (reg & 0x1f) << 8; /* destination register */  \
    GO_PAL(ARITH);                     /* trap */                   \
  }

#define ARITH_TRAP_I(flags, reg)                                    \
  {                                                                 \
    state.exc_sum = 0;                                              \
    ARITH_TRAP(TRAP_INT | flags, reg)                               \
  }

#define SPE_0_MASK    X64(0000ffffc0000000) /* <47:30> */
#define SPE_0_MATCH   X64(0000ffff80000000) /* <47:31> */
#define SPE_0_MAP     X64(000000003fffffff) /* <29:0>  */

#define SPE_1_MASK    X64(0000fe0000000000) /* <47:41> */
#define SPE_1_MATCH   X64(0000fc0000000000) /* <47:42> */
#define SPE_1_MAP     X64(000001ffffffffff) /* <40:0>  */
#define SPE_1_TEST    X64(0000010000000000) /* <40>    */
#define SPE_1_ADD     X64(00000e0000000000) /* <43:41> */

#define SPE_2_MASK    X64(0000c00000000000) /* <47:46> */
#define SPE_2_MATCH   X64(0000800000000000) /* <47>    */
#define SPE_2_MAP     X64(00000fffffffffff) /* <43:0>  */


#endif
