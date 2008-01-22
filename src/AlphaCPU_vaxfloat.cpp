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
 * Contains VAX floating point code for the Alpha CPU.
 *
 * $Id: AlphaCPU_vaxfloat.cpp,v 1.2 2008/01/22 21:33:42 iamcamiel Exp $
 *
 * X-1.2        Camiel Vanderhoeven                             22-JAN-2008
 *      Minor cleanup.
 *
 * X-1.1        Camiel Vanderhoeven                             21-JAN-2008
 *      File created. Contains code based upon the SIMH Alpha pre-
 *      implementation, which is Copyright (c) 2003, Robert M Supnik.
 **/

#include "StdAfx.h"
#include "AlphaCPU.h"

#include "cpu_debug.h"

/* VAX */

#define IPMAX		X64(7FFFFFFFFFFFFFFF)		/* plus MAX (int) */
#define IMMAX		X64(8000000000000000)		/* minus MAX (int) */

/* Unpacked rounding constants */
/* VAX */

#define UF_FRND		X64(0000008000000000)		/* F round */
#define UF_DRND		X64(0000000000000080)		/* D round */
#define UF_GRND		X64(0000000000000400)		/* G round */

/* VAX floating point loads and stores */

u64 CAlphaCPU::vax_ldf (u32 op)
{
u32 exp = F_GETEXP (op);

if (exp != 0) exp = exp + G_BIAS - F_BIAS;		/* zero? */	
return (((u64) (op & F_SIGN))? FPR_SIGN: 0) |	/* finite non-zero */
	(((u64) exp) << FPR_V_EXP) |
	(((u64) SWAP_VAXF (op & ~(F_SIGN|F_EXP))) << F_V_FRAC);
}

u64 CAlphaCPU::vax_ldg (u64 op)
{
return SWAP_VAXG (op);					/* swizzle bits */
}

u32 CAlphaCPU::vax_stf (u64 op)
{
u32 sign = FPR_GETSIGN (op)? F_SIGN: 0;
u32 exp = ((u32) (op >> (FPR_V_EXP - F_V_EXP))) & F_EXP;
u32 frac = (u32) (op >> F_V_FRAC);

return sign | exp | (SWAP_VAXF (frac) & ~(F_SIGN|F_EXP));
}

u64 CAlphaCPU::vax_stg (u64 op)
{
return SWAP_VAXG (op);					/* swizzle bits */
}

/* Set arithmetic trap */

void CAlphaCPU::arith_trap(u64 mask, u32 ins)
{
  ARITH_TRAP(mask | ((ins & I_FTRP_S) ? TRAP_SWC : 0)
                  , I_GETRC(ins));
}

/* Support routines */

bool CAlphaCPU::vax_unpack (u64 op, UFP *r, u32 ins)
{
r->sign = FPR_GETSIGN (op);				/* get sign */
r->exp = FPR_GETEXP (op);				/* get exponent */
r->frac = FPR_GETFRAC (op);				/* get fraction */
if (r->exp == 0) {					/* exp = 0? */
	if (op != 0) arith_trap (TRAP_INV, ins); 	/* rsvd op? */
	r->frac = r->sign = 0;
	return TRUE;  }
r->frac = (r->frac | FPR_HB) << FPR_GUARD;		/* ins hidden bit, guard */
return FALSE;
}

bool CAlphaCPU::vax_unpack_d (u64 op, UFP *r, u32 ins)
{
r->sign = FDR_GETSIGN (op);				/* get sign */
r->exp = FDR_GETEXP (op);				/* get exponent */
r->frac = FDR_GETFRAC (op);				/* get fraction */
if (r->exp == 0) {					/* exp = 0? */
	if (op != 0) arith_trap (TRAP_INV, ins); 	/* rsvd op? */
	r->frac = r->sign = 0;
	return TRUE;  }
r->exp = r->exp + G_BIAS - D_BIAS;			/* change to G bias */
r->frac = (r->frac | FDR_HB) << FDR_GUARD;		/* ins hidden bit, guard */
return FALSE;
}

/* VAX normalize */

void CAlphaCPU::vax_norm (UFP *r)
{
s32 i;
static u64 normmask[5] = {
 0xc000000000000000, 0xf000000000000000, 0xff00000000000000,
 0xffff000000000000, 0xffffffff00000000 };
static s32 normtab[6] = { 1, 2, 4, 8, 16, 32};

r->frac = r->frac & X64_QUAD;
if (r->frac == 0) {					/* if fraction = 0 */
	r->sign = r->exp = 0;				/* result is 0 */
	return;  }
while ((r->frac & UF_NM) == 0) {			/* normalized? */
	for (i = 0; i < 5; i++) {			/* find first 1 */
	    if (r->frac & normmask[i]) break;  }
	r->frac = r->frac << normtab[i];		/* shift frac */
	r->exp = r->exp - normtab[i];  }		/* decr exp */
return;
}

/* VAX round and pack */

u64 CAlphaCPU::vax_rpack (UFP *r, u32 ins, u32 dp)
{
u32 rndm = I_GETFRND (ins);
static const u64 roundbit[2] = { UF_FRND, UF_GRND };
static const s32 expmax[2] = { G_BIAS - F_BIAS + F_M_EXP, G_M_EXP };
static const s32 expmin[2] = { G_BIAS - F_BIAS, 0 };

if (r->frac == 0) return 0;				/* result 0? */
if (rndm) {						/* round? */
	r->frac = (r->frac + roundbit[dp]) & X64_QUAD;	/* add round bit */
	if ((r->frac & UF_NM) == 0) {			/* carry out? */
	    r->frac = (r->frac >> 1) | UF_NM;		/* renormalize */
	    r->exp = r->exp + 1;  }  }
if (r->exp > expmax[dp]) {				/* ovflo? */
	arith_trap (TRAP_OVF, ins);			/* set trap */
	r->exp = expmax[dp];  }				/* return max */
if (r->exp <= expmin[dp]) {				/* underflow? */
	if (ins & I_FTRP_V) arith_trap (TRAP_UNF, ins);	/* enabled? set trap */
	return 0;  }					/* underflow to 0 */
return (((u64) r->sign) << FPR_V_SIGN) |
	(((u64) r->exp) << FPR_V_EXP) |
	((r->frac >> FPR_GUARD) & FPR_FRAC);
}

u64 CAlphaCPU::vax_rpack_d (UFP *r, u32 ins)
{
if (r->frac == 0) return 0;				/* result 0? */
r->exp = r->exp + D_BIAS - G_BIAS;			/* rebias */
if (r->exp > FDR_M_EXP) {				/* ovflo? */
	arith_trap (TRAP_OVF, ins);			/* set trap */
	r->exp = FDR_M_EXP;  }				/* return max */
if (r->exp <= 0) {					/* underflow? */
	if (ins & I_FTRP_V) arith_trap (TRAP_UNF, ins);	/* enabled? set trap */
	return 0;  }					/* underflow to 0 */
return (((u64) r->sign) << FDR_V_SIGN) |
	(((u64) r->exp) << FDR_V_EXP) |
	((r->frac >> FDR_GUARD) & FDR_FRAC);
}

/* VAX floating compare */

s32 CAlphaCPU:: vax_fcmp (u64 s1, u64 s2, u32 ins)
{
UFP a, b;

if (vax_unpack (s1, &a, ins)) return +1;			/* unpack, rsv? */
if (vax_unpack (s2, &b, ins)) return +1;			/* unpack, rsv? */
if (s1 == s2) return 0;					/* equal? */
if (a.sign != b.sign) return (a.sign? -1: +1);		/* opp signs? */
return (((s1 < s2) ^ a.sign)? -1: +1);			/* like signs */
}

/* VAX integer to floating convert */

u64 CAlphaCPU::vax_cvtif (u64 val, u32 ins, u32 dp)
{
UFP a;

if (val == 0) return 0;					/* 0? return +0 */
if (val < 0) {						/* < 0? */
	a.sign = 1;					/* set sign */
	val = NEG_Q (val);  }				/* |val| */
else a.sign = 0;
a.exp = 64 + G_BIAS;					/* set exp */
a.frac = val;						/* set frac */
vax_norm (&a);						/* normalize */
return vax_rpack (&a, ins, dp);				/* round and pack */
}

/* VAX floating to integer convert - note that rounding cannot cause a
   carry unless the fraction has been shifted right at least FP_GUARD
   places; in which case a carry out is impossible */

u64 CAlphaCPU::vax_cvtfi (u64 op, u32 ins)
{
UFP a;
u32 rndm = I_GETFRND (ins);
s32 ubexp;

if (vax_unpack (op, &a, ins)) return 0;			/* unpack, rsv? */
ubexp = a.exp - G_BIAS;					/* unbiased exp */
if (ubexp < 0) return 0;				/* zero or too small? */
if (ubexp <= UF_V_NM) {					/* in range? */
	a.frac = a.frac >> (UF_V_NM - ubexp);		/* leave rnd bit */
	if (rndm) a.frac = a.frac + 1;			/* not chopped, round */
	a.frac = a.frac >> 1;				/* now justified */
	if ((a.frac > (a.sign? IMMAX: IPMAX)) &&	/* out of range? */
	    (ins & I_FTRP_V))				/* trap enabled? */
	    arith_trap (TRAP_IOV, ins);  }		/* set overflow */
else {	if (ubexp > (UF_V_NM + 64)) a.frac = 0;		/* out of range */
	else a.frac = (a.frac << (ubexp - UF_V_NM - 1)) & X64_QUAD;	/* no rnd bit */
	if (ins & I_FTRP_V)				/* trap enabled? */
	    arith_trap (TRAP_IOV, ins);  }		/* set overflow */
return (a.sign? NEG_Q (a.frac): a.frac);
}

/* VAX floating add */

u64 CAlphaCPU::vax_fadd (u64 s1, u64 s2, u32 ins, u32 dp, bool sub)
{
UFP a, b, t;
u32 sticky;
s32 ediff;

if (vax_unpack (s1, &a, ins)) return 0;			/* unpack, rsv? */
if (vax_unpack (s2, &b, ins)) return 0;			/* unpack, rsv? */
if (sub) b.sign = b.sign ^ 1;				/* sub? invert b sign */
if (a.exp == 0) a = b;					/* s1 = 0? */
else if (b.exp) {					/* s2 != 0? */
	if ((a.exp < b.exp) ||				/* |s1| < |s2|? swap */
	    ((a.exp == b.exp) && (a.frac < b.frac))) {
	    t = a;
	    a = b;
	    b = t;  }
	ediff = a.exp - b.exp;				/* exp diff */
	if (a.sign ^ b.sign) {				/* eff sub? */
	    if (ediff > 63) b.frac = 1;			/* >63? retain sticky */
	    else if (ediff) {				/* [1,63]? shift */
		sticky = ((b.frac << (64 - ediff)) & X64_QUAD)? 1: 0; /* lost bits */
		b.frac = (b.frac >> ediff) | sticky;  }
	    a.frac = (a.frac - b.frac) & X64_QUAD;		/* subtract fractions */
	    vax_norm (&a);  }				/* normalize */
	else {						/* eff add */
	    if (ediff > 63) b.frac = 0;			/* >63? b disappears */
	    else if (ediff) b.frac = b.frac >> ediff;	/* denormalize */
	    a.frac = (a.frac + b.frac) & X64_QUAD;		/* add frac */
	    if (a.frac < b.frac) {			/* chk for carry */
		a.frac = UF_NM | (a.frac >> 1);		/* shift in carry */
		a.exp = a.exp + 1;  }  }		/* skip norm */
	}						/* end else if */
return vax_rpack (&a, ins, dp);				/* round and pack */
}

/* VAX floating multiply */

u64 CAlphaCPU::vax_fmul (u64 s1, u64 s2, u32 ins, u32 dp)
{
UFP a, b;

if (vax_unpack (s1, &a, ins)) return 0;			/* unpack, rsv? */
if (vax_unpack (s2, &b, ins)) return 0;			/* unpack, rsv? */
if ((a.exp == 0) || (b.exp == 0)) return 0;		/* zero argument? */
a.sign = a.sign ^ b.sign;				/* sign of result */
a.exp = a.exp + b.exp - G_BIAS;				/* add exponents */
uemul64 (a.frac, b.frac, &a.frac);			/* mpy fractions */
vax_norm (&a);						/* normalize */
return vax_rpack (&a, ins, dp);				/* round and pack */
}

/* VAX floating divide
   Needs to develop at least one rounding bit.  Since the first
   divide step can fail, develop 2 more bits than the precision of
   the fraction. */

u64 CAlphaCPU::vax_fdiv (u64 s1, u64 s2, u32 ins, u32 dp)
{
UFP a, b;

if (vax_unpack (s1, &a, ins)) return 0;			/* unpack, rsv? */
if (vax_unpack (s2, &b, ins)) return 0;			/* unpack, rsv? */
if (b.exp == 0) {					/* divr = 0? */
	arith_trap (TRAP_DZE, ins);			/* dze trap */
	return 0;  }
if (a.exp == 0) return 0;				/* divd = 0? */
a.sign = a.sign ^ b.sign;				/* result sign */
a.exp = a.exp - b.exp + G_BIAS + 1;			/* unbiased exp */
a.frac = a.frac >> 1;					/* allow 1 bit left */
b.frac = b.frac >> 1;
a.frac = ufdiv64 (a.frac, b.frac, 55, NULL);		/* divide */
vax_norm (&a);						/* normalize */
return vax_rpack (&a, ins, dp);				/* round and pack */
}

/* VAX floating square root */

u64 CAlphaCPU::vax_sqrt (u32 ins, u32 dp)
{
u64 op;
UFP b;

op = state.f[I_GETRB (ins)];					/* get F[rb] */
if (vax_unpack (op, &b, ins)) return 0;			/* unpack, rsv? */
if (b.exp == 0) return 0;				/* zero? */
if (b.sign) {						/* minus? */
	arith_trap (TRAP_INV, ins);			/* invalid operand */
	return 0;  }
b.exp = ((b.exp + 1 - G_BIAS) >> 1) + G_BIAS;		/* result exponent */
b.frac = fsqrt64 (b.frac, b.exp);			/* result fraction */
return vax_rpack (&b, ins, dp);				/* round and pack */
}
