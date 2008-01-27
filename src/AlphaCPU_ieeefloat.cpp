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
 * Contains IEEE floating point code for the Alpha CPU.
 *
 * $Id: AlphaCPU_ieeefloat.cpp,v 1.2 2008/01/27 09:56:06 iamcamiel Exp $
 *
 * X-1.2        Camiel Vanderhoeven                             27-JAN-2008
 *      Bugfix in ieee_sts.
 *
 * X-1.1        Camiel Vanderhoeven                             21-JAN-2008
 *      File created. Contains code based upon the SIMH Alpha pre-
 *      implementation, which is Copyright (c) 2003, Robert M Supnik.
 **/

#include "StdAfx.h"
#include "AlphaCPU.h"

/* Register format constants */
/* IEEE */

#define QNAN		X64(0008000000000000)		/* quiet NaN flag */
#define CQNAN		X64(FFF8000000000000)		/* canonical quiet NaN */
#define FPZERO		X64(0000000000000000)		/* plus zero (fp) */
#define FMZERO		X64(8000000000000000)		/* minus zero (fp) */
#define FPINF		X64(7FF0000000000000)		/* plus infinity (fp) */
#define FMINF		X64(FFF0000000000000)		/* minus infinity (fp) */
#define FPMAX		X64(7FFFFFFFFFFFFFFF)		/* plus MAX (fp) */
#define FMMAX		X64(FFFFFFFFFFFFFFFF)		/* minus MAX (fp) */
#define IPMAX		X64(7FFFFFFFFFFFFFFF)		/* plus MAX (int) */
#define IMMAX		X64(8000000000000000)		/* minus MAX (int) */

/* Unpacked rounding constants */
/* IEEE */

#define UF_SRND		X64(0000008000000000)		/* S normal round */
#define UF_SINF		X64(000000FFFFFFFFFF)		/* S infinity round */
#define UF_TRND		X64(0000000000000400)		/* T normal round */
#define UF_TINF		X64(00000000000007FF)		/* T infinity round */

/* IEEE S load */

u64 CAlphaCPU::ieee_lds (u32 op)
{
u32 exp = S_GETEXP (op);				/* get exponent */

if (exp == S_NAN) exp = FPR_NAN;			/* inf or NaN? */
else if (exp != 0) exp = exp + T_BIAS - S_BIAS;		/* zero or denorm? */
return (((u64) (op & S_SIGN))? FPR_SIGN: 0) |	/* reg format */
	(((u64) exp) << FPR_V_EXP) |
	(((u64) (op & ~(S_SIGN|S_EXP))) << S_V_FRAC);
}

/* IEEE S store */

u32 CAlphaCPU::ieee_sts (u64 op)
{
u32 sign = FPR_GETSIGN (op)? S_SIGN: 0;
u32 exp = FPR_GETEXP(op);
if (exp == FPR_NAN) exp = S_NAN;			/* inf or NaN? */
else if (exp != 0) exp = exp + S_BIAS - T_BIAS;		/* zero or denorm? */
exp = (exp << S_V_EXP) & S_EXP;
u32 frac = ((u32) (op >> S_V_FRAC)) & X64_LONG;

return sign | exp | (frac & ~(S_SIGN|S_EXP));
}

/* IEEE S to T convert - LDS doesn't handle denorms correctly */

u64 CAlphaCPU::ieee_cvtst (u64 op, u32 ins)
{
UFP b;
u32 ftpb;

ftpb = ieee_unpack (op, &b, ins);			/* unpack; norm dnorm */
if (ftpb == UFT_DENORM) {				/* denormal? */
	b.exp = b.exp + T_BIAS - S_BIAS;		/* change 0 exp to T */
	return ieee_rpack (&b, ins, DT_T);  }		/* round, pack */
else return op;						/* identity */
}

/* IEEE T to S convert */

u64 CAlphaCPU::ieee_cvtts (u64 op, u32 ins)
{
UFP b;
u32 ftpb;

ftpb = ieee_unpack (op, &b, ins);			/* unpack */
if (Q_FINITE (ftpb)) return ieee_rpack (&b, ins, DT_S);	/* finite? round, pack */
if (ftpb == UFT_NAN) return (op | QNAN);		/* nan? cvt to quiet */
if (ftpb == UFT_INF) return op;				/* inf? unchanged */
return 0;						/* denorm? 0 */
}

/* IEEE floating compare

   - Take care of NaNs
   - Force -0 to +0
   - Then normal compare will work (even on inf and denorms) */

s32 CAlphaCPU::ieee_fcmp (u64 s1, u64 s2, u32 ins, u32 trap_nan)
{
UFP a, b;
u32 ftpa, ftpb;

ftpa = ieee_unpack (s1, &a, ins);
ftpb = ieee_unpack (s2, &b, ins);
if ((ftpa == UFT_NAN) || (ftpb == UFT_NAN)) {		/* NaN involved? */
	if (trap_nan) ieee_trap (TRAP_INV, 1, FPCR_INVD, ins);
	return +1;  }					/* force failure */
if (ftpa == UFT_ZERO) a.sign = 0;			/* only +0 allowed */
if (ftpb == UFT_ZERO) b.sign = 0;
if (a.sign != b.sign) return (a.sign? -1: +1);		/* unequal signs? */
if (a.exp != b.exp) return ((a.sign ^ (a.exp < b.exp))? -1: +1);
if (a.frac != b.frac) return ((a.sign ^ (a.frac < b.frac))? -1: +1);
return 0;
}

/* IEEE integer to floating convert */

u64 CAlphaCPU::ieee_cvtif (u64 val, u32 ins, u32 dp)
{
UFP a;

if (val == 0) return 0;					/* 0? return +0 */
if (val < 0) {						/* < 0? */
	a.sign = 1;					/* set sign */
	val = NEG_Q (val);  }				/* |val| */
else a.sign = 0;
a.exp = 63 + T_BIAS;					/* set exp */
a.frac = val;						/* set frac */
ieee_norm (&a);						/* normalize */
return ieee_rpack (&a, ins, dp);				/* round and pack */
}

/* IEEE floating to integer convert - rounding code from SoftFloat
   The Alpha architecture specifies return of the low order bits of
   the true result, whereas the IEEE standard specifies the return
   of the maximum plus or minus value */

u64 CAlphaCPU::ieee_cvtfi (u64 op, u32 ins)
{
UFP a;
u64 sticky;
u32 rndm, ftpa, ovf = 0;
s32 ubexp;

ftpa = ieee_unpack (op, &a, ins);			/* unpack */
if (!Q_FINITE (ftpa)) {					/* inf, NaN, dnorm? */
	ieee_trap (TRAP_INV, 1, FPCR_INVD, ins);		/* inv operation */
	return 0;  }
if (ftpa == UFT_ZERO) return 0;				/* zero? */
ubexp = a.exp - T_BIAS;					/* unbiased exp */
if (ubexp < 0) {					/* < 1? */
	if (ubexp == -1) sticky = a.frac;		/* [.5,1)? */
	else sticky = 1;				/* (0,.5) */
	a.frac = 0;  }
else if (ubexp <= UF_V_NM) {				/* in range? */
	a.frac = a.frac >> (UF_V_NM - ubexp);		/* result */
	sticky = (a.frac << (64 - (UF_V_NM - ubexp))) & X64_QUAD;  }
else {	if ((ubexp - UF_V_NM) > 63) a.frac = 0;		/* out of range */
	else a.frac = (a.frac << (ubexp - UF_V_NM)) & X64_QUAD;
	ovf = 1;					/* overflow */
	sticky = 0;  }					/* no rounding */
rndm = I_GETFRND (ins);					/* get round mode */
if (((rndm == I_FRND_N) && (sticky & Q_SIGN)) ||	/* nearest? */
    ((rndm == I_FRND_P) && !a.sign && sticky) ||	/* +inf and +? */
    ((rndm == I_FRND_M) && a.sign && sticky)) {		/* -inf and -? */
	a.frac = (a.frac + 1) & X64_QUAD;
	if (a.frac == 0) ovf = 1;			/* overflow? */
	if ((rndm == I_FRND_N) && (sticky == Q_SIGN))	/* round nearest hack */
	    a.frac = a.frac & ~1;  }
if (a.frac > (a.sign? IMMAX: IPMAX)) ovf = 1;		/* overflow? */
ieee_trap (TRAP_IOV, ins & I_FTRP_V, 0, 0);		/* overflow trap */
if (ovf || sticky)					/* ovflo or round? */
	ieee_trap (TRAP_INE, Q_SUI (ins), FPCR_INED, ins);
return (a.sign? NEG_Q (a.frac): a.frac);
}

/* IEEE floating add

   - Take care of NaNs and infinites
   - Test for zero (fast exit)
   - Sticky logic for floating add
	> If result normalized, sticky in right place
	> If result carries out, renormalize, retain sticky
   - Sticky logic for floating subtract
	> If shift < guard, no sticky bits; 64b result is exact
	  If shift <= 1, result may require extensive normalization,
	  but there are no sticky bits to worry about
	> If shift >= guard, there is a sticky bit,
	  but normalization is at most 1 place, sticky bit is retained
	  for rounding purposes (but not in low order bit) */

u64 CAlphaCPU::ieee_fadd (u64 s1, u64 s2, u32 ins, u32 dp, bool sub)
{
UFP a, b, t;
u32 ftpa, ftpb;
u32 sticky;
s32 ediff;

ftpa = ieee_unpack (s1, &a, ins);			/* unpack operands */
ftpb = ieee_unpack (s2, &b, ins);
if (ftpb == UFT_NAN) return s2 | QNAN;			/* B = NaN? quiet B */
if (ftpa == UFT_NAN) return s1 | QNAN;			/* A = NaN? quiet A */
if (sub) b.sign = b.sign ^ 1;				/* sign of B */
if (ftpb == UFT_INF) {					/* B = inf? */
	if ((ftpa == UFT_INF) && (a.sign ^ b.sign)) {	/* eff sub of inf? */
	    ieee_trap (TRAP_INV, 1, FPCR_INVD, ins);	/* inv op trap */
	    return CQNAN;  }				/* canonical NaN */
	return (sub? (s2 ^ FPR_SIGN): s2);  }		/* return B */
if (ftpa == UFT_INF) return s1;				/* A = inf? ret A */
if (ftpa == UFT_ZERO) a = b;				/* s1 = 0? */
else if (ftpb != UFT_ZERO) {				/* s2 != 0? */
	if ((a.exp < b.exp) ||				/* s1 < s2? swap */
	   ((a.exp == b.exp) && (a.frac < b.frac))) {
	    t = a;
	    a = b;
	    b = t;  }
	ediff = a.exp - b.exp;				/* exp diff */
	if (ediff > 63) b.frac = 1;			/* >63? retain sticky */
	else if (ediff) {				/* [1,63]? shift */
	    sticky = ((b.frac << (64 - ediff)) & X64_QUAD)? 1: 0;	/* lost bits */
	    b.frac = ((b.frac >> ediff) & X64_QUAD) | sticky;  }
	if (a.sign ^ b.sign) {				/* eff sub? */
	    a.frac = (a.frac - b.frac) & X64_QUAD;		/* subtract fractions */
	    ieee_norm (&a);  }				/* normalize */
	else {						/* eff add */
	    a.frac = (a.frac + b.frac) & X64_QUAD;		/* add frac */
	    if (a.frac < b.frac) {			/* chk for carry */
		a.frac = UF_NM | (a.frac >> 1) |	/* shift in carry */
		    (a.frac & 1);			/* retain sticky */
		a.exp = a.exp + 1;  }  }		/* skip norm */
	}						/* end else if */
return ieee_rpack (&a, ins, dp);				/* round and pack */
}

/* IEEE floating multiply 

   - Take care of NaNs and infinites
   - Test for zero operands (fast exit)
   - 64b x 64b fraction multiply, yielding 128b result
   - Normalize (at most 1 bit)
   - Insert "sticky" bit in low order fraction, for rounding
   
   Because IEEE fractions have a range of [1,2), the result can have a range
   of [1,4).  Results in the range of [1,2) appear to be denormalized by one
   place, when in fact they are correct.  Results in the range of [2,4) appear
   to be in correct, when in fact they are 2X larger.  This problem is taken
   care of in the result exponent calculation. */

u64 CAlphaCPU::ieee_fmul (u64 s1, u64 s2, u32 ins, u32 dp)
{
UFP a, b;
u32 ftpa, ftpb;
u64 resl;

ftpa = ieee_unpack (s1, &a, ins);			/* unpack operands */
ftpb = ieee_unpack (s2, &b, ins);
if (ftpb == UFT_NAN) return s2 | QNAN;			/* B = NaN? quiet B */
if (ftpa == UFT_NAN) return s1 | QNAN;			/* A = NaN? quiet A */
a.sign = a.sign ^ b.sign;				/* sign of result */
if ((ftpa == UFT_ZERO) || (ftpb == UFT_ZERO)) {		/* zero operand? */
	if ((ftpa == UFT_INF) || (ftpb == UFT_INF)) {	/* 0 * inf? */
	    ieee_trap (TRAP_INV, 1, FPCR_INVD, ins);	/* inv op trap */
	    return CQNAN;  }				/* canonical NaN */
	return (a.sign? FMZERO: FPZERO);  }		/* return signed 0 */
if (ftpb == UFT_INF) return (a.sign? FMINF: FPINF);	/* B = inf? */
if (ftpa == UFT_INF) return (a.sign? FMINF: FPINF);	/* A = inf? */
a.exp = a.exp + b.exp + 1 - T_BIAS;			/* add exponents */
resl = uemul64 (a.frac, b.frac, &a.frac);		/* multiply fracs */
ieee_norm (&a);						/* normalize */
a.frac = a.frac | (resl? 1: 0);				/* sticky bit */
return ieee_rpack (&a, ins, dp);				/* round and pack */
}

/* Floating divide

   - Take care of NaNs and infinites
   - Check for zero cases
   - Divide fractions (55b to develop a rounding bit)
   - Set sticky bit if remainder non-zero
   
   Because IEEE fractions have a range of [1,2), the result can have a range
   of (.5,2).  Results in the range of [1,2) are correct.  Results in the
   range of (.5,1) need to be normalized by one place. */

u64 CAlphaCPU::ieee_fdiv (u64 s1, u64 s2, u32 ins, u32 dp)
{
UFP a, b;
u32 ftpa, ftpb, sticky;

ftpa = ieee_unpack (s1, &a, ins);
ftpb = ieee_unpack (s2, &b, ins);
if (ftpb == UFT_NAN) return s2 | QNAN;			/* B = NaN? quiet B */
if (ftpa == UFT_NAN) return s1 | QNAN;			/* A = NaN? quiet A */
a.sign = a.sign ^ b.sign;				/* sign of result */
if (ftpb == UFT_INF) {					/* B = inf? */
	if (ftpa == UFT_INF) {				/* inf/inf? */
	    ieee_trap (TRAP_INV, 1, FPCR_INVD, ins);	/* inv op trap */
	    return CQNAN;  }				/* canonical NaN */
	return (a.sign? FMZERO: FPZERO);  }		/* !inf/inf, ret 0 */
if (ftpa == UFT_INF) {					/* A = inf? */
	if (ftpb == UFT_ZERO)				/* inf/0? */
	    ieee_trap (TRAP_DZE, 1, FPCR_DZED, ins);	/* div by 0 trap */
	return (a.sign? FMINF: FPINF);  }		/* return inf */
if (ftpb == UFT_ZERO) {					/* B = 0? */
	if (ftpa == UFT_ZERO) {				/* 0/0? */
	    ieee_trap (TRAP_INV, 1, FPCR_INVD, ins);	/* inv op trap */
	    return CQNAN;  }				/* canonical NaN */
	ieee_trap (TRAP_DZE, 1, FPCR_DZED, ins);		/* div by 0 trap */
	return (a.sign? FMINF: FPINF);  }		/* return inf */
if (ftpa == UFT_ZERO) return (a.sign? FMZERO: FPZERO);	/* A = 0? */
a.exp = a.exp - b.exp + T_BIAS;				/* unbiased exp */
a.frac = a.frac >> 1;					/* allow 1 bit left */
b.frac = b.frac >> 1;
a.frac = ufdiv64 (a.frac, b.frac, 55, &sticky);		/* divide */
ieee_norm (&a);						/* normalize */
a.frac = a.frac | sticky;				/* insert sticky */
return ieee_rpack (&a, ins, dp);				/* round and pack */
}

/* IEEE floating square root

   - Take care of NaNs, +infinite, zero
   - Check for negative operand
   - Compute result exponent
   - Compute sqrt of fraction */

u64 CAlphaCPU::ieee_sqrt (u32 ins, u32 dp)
{
u64 op;
u32 ftpb;
UFP b;

op = state.f[I_GETRB (ins)];					/* get F[rb] */
ftpb = ieee_unpack (op, &b, ins);			/* unpack */
if (ftpb == UFT_NAN) return op | QNAN;			/* NaN? */
if ((ftpb == UFT_ZERO) ||				/* zero? */
    ((ftpb == UFT_INF) && !b.sign)) return op;		/* +infinity? */
if (b.sign) {						/* minus? */
	ieee_trap (TRAP_INV, 1, FPCR_INVD, ins);		/* signal inv op */
	return CQNAN;  }
b.exp = ((b.exp - T_BIAS) >> 1) + T_BIAS;		/* result exponent */
b.frac = fsqrt64 (b.frac, b.exp);			/* result fraction */
return ieee_rpack (&b, ins, dp);				/* round and pack */
}

/* Support routines */

int CAlphaCPU::ieee_unpack (u64 op, UFP *r, u32 ins)
{
r->sign = FPR_GETSIGN (op);				/* get sign */
r->exp = FPR_GETEXP (op);				/* get exponent */
r->frac = FPR_GETFRAC (op);				/* get fraction */
if (r->exp == 0) {					/* exponent = 0? */
	if (r->frac == 0) return UFT_ZERO;		/* frac = 0? then true 0 */
	if (state.fpcr & FPCR_DNZ) {				/* denorms to 0? */
	    r->frac = 0;				/* clear fraction */
	    return UFT_ZERO;  }
	r->frac = r->frac << FPR_GUARD;			/* guard fraction */
	ieee_norm (r);					/* normalize dnorm */
	ieee_trap (TRAP_INV, 1, FPCR_INVD, ins);		/* signal inv op */
    return UFT_DENORM;  }
if (r->exp == FPR_NAN) {				/* exponent = max? */
  if (r->frac == 0) return UFT_INF;		/* frac = 0? then inf */
	if (!(r->frac & QNAN))				/* signaling NaN? */
	    ieee_trap (TRAP_INV, 1, FPCR_INVD, ins);	/* signal inv op */
    return UFT_NAN;  }
r->frac = (r->frac | FPR_HB) << FPR_GUARD;		/* ins hidden bit, guard */
return UFT_FIN;						/* finite */
}

/* Normalize - input must be zero, finite, or denorm */

void CAlphaCPU::ieee_norm (UFP *r)
{
s32 i;
static u64 normmask[5] = {
 0xc000000000000000, 0xf000000000000000, 0xff00000000000000,
 0xffff000000000000, 0xffffffff00000000 };
static s32 normtab[6] = { 1, 2, 4, 8, 16, 32};

r->frac = r->frac & X64_QUAD;
if (r->frac == 0) {					/* if fraction = 0 */
	r->exp = 0;					/* result is signed 0 */
	return;  }
while ((r->frac & UF_NM) == 0) {			/* normalized? */
	for (i = 0; i < 5; i++) {			/* find first 1 */
	    if (r->frac & normmask[i]) break;  }
	r->frac = r->frac << normtab[i];		/* shift frac */
	r->exp = r->exp - normtab[i];  }		/* decr exp */
return;
}

/* Round and pack

   Much of the treachery of the IEEE standard is buried here
   - Rounding modes (chopped, +infinity, nearest, -infinity)
   - Inexact (set if there are any rounding bits, regardless of rounding)
   - Overflow (result is infinite if rounded, max if not)
   - Underflow (no denorms!)
   
   Underflow handling is particularly complicated
   - Result is always 0
   - UNF and INE are always set in FPCR
   - If /U is set,
     o If /S is clear, trap
     o If /S is set, UNFD is set, but UNFZ is clear, ignore UNFD and
       trap, because the hardware cannot produce denormals
     o If /S is set, UNFD is set, and UNFZ is set, do not trap
   - If /SUI is set, and INED is clear, trap */

u64 CAlphaCPU::ieee_rpack (UFP *r, u32 ins, u32 dp)
{
static const u64 stdrnd[2] = { UF_SRND, UF_TRND };
static const u64 infrnd[2] = { UF_SINF, UF_TINF };
static const s32 expmax[2] = { T_BIAS - S_BIAS + S_M_EXP - 1, T_M_EXP - 1 };
static const s32 expmin[2] = { T_BIAS - S_BIAS, 0 };
u64 rndadd, rndbits, res;
u64 rndm;

if (r->frac == 0) return (((u64)r->sign) << FPR_V_SIGN);	/* result 0? */
rndm = I_GETFRND (ins);					/* inst round mode */
if (rndm == I_FRND_D) rndm = FPCR_GETFRND (state.fpcr);	/* dynamic? use FPCR */
rndbits = r->frac & infrnd[dp];				/* isolate round bits */
if (rndm == I_FRND_N) rndadd = stdrnd[dp];		/* round to nearest? */
else if (((rndm == I_FRND_P) && !r->sign) ||		/* round to inf and */
	((rndm == I_FRND_M) && r->sign))		/* right sign? */
	rndadd = infrnd[dp];
else rndadd = 0;
r->frac = (r->frac + rndadd) & X64_QUAD;			/* round */
if ((r->frac & UF_NM) == 0) {				/* carry out? */
	r->frac = (r->frac >> 1) | UF_NM;		/* renormalize */
	r->exp = r->exp + 1;  }
if (rndbits)						/* inexact? */
	ieee_trap (TRAP_INE, Q_SUI (ins), FPCR_INED, ins);/* set inexact */
if (r->exp > expmax[dp]) {				/* ovflo? */
	ieee_trap (TRAP_OVF, 1, FPCR_OVFD, ins);		/* set overflow trap */
	ieee_trap (TRAP_INE, Q_SUI (ins), FPCR_INED, ins);/* set inexact */
	if (rndadd)					/* did we round? */
	    return (r->sign? FMINF: FPINF);		/* return infinity */
	return (r->sign? FMMAX: FPMAX);  }		/* no, return max */
if (r->exp <= expmin[dp]) {				/* underflow? */
	ieee_trap (TRAP_UNF, ins & I_FTRP_U,		/* set underflow trap */
	    (state.fpcr & FPCR_UNDZ)? FPCR_UNFD: 0, ins);	/* (dsbl only if UNFZ set) */
	ieee_trap (TRAP_INE, Q_SUI (ins), FPCR_INED, ins);/* set inexact */
	return 0;  }					/* underflow to +0 */
res = (((u64) r->sign) << FPR_V_SIGN) |		/* form result */
	(((u64) r->exp) << FPR_V_EXP) |
	((r->frac >> FPR_GUARD) & FPR_FRAC);
if ((rndm == I_FRND_N) && (rndbits == stdrnd[dp]))	/* nearest and halfway? */
	res = res & ~1;					/* clear lo bit */
return res;
}

/* IEEE arithmetic trap - only one can be set at a time! */

void CAlphaCPU::ieee_trap (u64 trap, u32 instenb, u64 fpcrdsb, u32 ins)
{
  u64 real_trap = X64(0);

  if (!state.fpcr & (trap<<51))     // trap bit not set in FPCR
    real_trap |= trap<<41;          // SET trap bit in EXC_SUM

  if ((instenb != 0) &&					/* not enabled in inst? ignore */
      !((ins & I_FTRP_S) && (state.fpcr & fpcrdsb)))	/* /S and disabled? ignore */
    real_trap |= trap;              // trap bit in EXC_SUM

  if (real_trap)
    arith_trap (trap, ins);					/* set Alpha trap */
  return;
}

/* Fraction square root routine - code from SoftFloat */

u64 CAlphaCPU::fsqrt64 (u64 asig, s32 exp)
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
