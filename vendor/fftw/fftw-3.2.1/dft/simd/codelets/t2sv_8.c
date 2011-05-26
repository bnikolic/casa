/*
 * Copyright (c) 2003, 2007-8 Matteo Frigo
 * Copyright (c) 2003, 2007-8 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Mon Feb  9 19:53:27 EST 2009 */

#include "codelet-dft.h"

#ifdef HAVE_FMA

/* Generated by: ../../../genfft/gen_twiddle -fma -reorder-insns -schedule-for-pipeline -simd -compact -variables 4 -pipeline-latency 8 -twiddle-log3 -precompute-twiddles -n 8 -name t2sv_8 -include ts.h */

/*
 * This function contains 74 FP additions, 50 FP multiplications,
 * (or, 44 additions, 20 multiplications, 30 fused multiply/add),
 * 64 stack variables, 1 constants, and 32 memory accesses
 */
#include "ts.h"

static void t2sv_8(R *ri, R *ii, const R *W, stride rs, INT mb, INT me, INT ms)
{
     DVK(KP707106781, +0.707106781186547524400844362104849039284835938);
     INT m;
     for (m = mb, W = W + (mb * 6); m < me; m = m + (2 * VL), ri = ri + ((2 * VL) * ms), ii = ii + ((2 * VL) * ms), W = W + ((2 * VL) * 6), MAKE_VOLATILE_STRIDE(rs)) {
	  V T1m, T1l, T1k, T1u, T1n, T1o;
	  {
	       V T2, T3, Tl, Tn, T5, T6;
	       T2 = LDW(&(W[0]));
	       T3 = LDW(&(W[TWVL * 2]));
	       Tl = LDW(&(W[TWVL * 4]));
	       Tn = LDW(&(W[TWVL * 5]));
	       T5 = LDW(&(W[TWVL * 1]));
	       T6 = LDW(&(W[TWVL * 3]));
	       {
		    V T1, T1s, TK, T1r, Td, Tk, TG, TC, TY, Tu, TW, TL, TM, TO, TQ;
		    V Tx, Tz, TD, TH;
		    {
			 V T8, T4, Tm, Tr, Tc, Ta;
			 T1 = LD(&(ri[0]), ms, &(ri[0]));
			 T1s = LD(&(ii[0]), ms, &(ii[0]));
			 T8 = LD(&(ri[WS(rs, 4)]), ms, &(ri[0]));
			 T4 = VMUL(T2, T3);
			 Tm = VMUL(T2, Tl);
			 Tr = VMUL(T2, Tn);
			 Tc = LD(&(ii[WS(rs, 4)]), ms, &(ii[0]));
			 Ta = VMUL(T2, T6);
			 {
			      V Tp, Tt, Tg, T7, Tf, To, Ts, Ti, Tb, Tj;
			      Tp = LD(&(ri[WS(rs, 6)]), ms, &(ri[0]));
			      Tt = LD(&(ii[WS(rs, 6)]), ms, &(ii[0]));
			      Tg = LD(&(ri[WS(rs, 2)]), ms, &(ri[0]));
			      T7 = VFNMS(T5, T6, T4);
			      Tf = VFMA(T5, T6, T4);
			      To = VFMA(T5, Tn, Tm);
			      Ts = VFNMS(T5, Tl, Tr);
			      Ti = VFNMS(T5, T3, Ta);
			      Tb = VFMA(T5, T3, Ta);
			      Tj = LD(&(ii[WS(rs, 2)]), ms, &(ii[0]));
			      TK = LD(&(ri[WS(rs, 7)]), ms, &(ri[WS(rs, 1)]));
			      {
				   V T1q, T9, Th, TF;
				   T1q = VMUL(T7, Tc);
				   T9 = VMUL(T7, T8);
				   Th = VMUL(Tf, Tg);
				   TF = VMUL(Tf, Tn);
				   {
					V TB, TX, Tq, TV;
					TB = VMUL(Tf, Tl);
					TX = VMUL(To, Tt);
					Tq = VMUL(To, Tp);
					TV = VMUL(Tf, Tj);
					T1r = VFNMS(Tb, T8, T1q);
					Td = VFMA(Tb, Tc, T9);
					Tk = VFMA(Ti, Tj, Th);
					TG = VFNMS(Ti, Tl, TF);
					TC = VFMA(Ti, Tn, TB);
					TY = VFNMS(Ts, Tp, TX);
					Tu = VFMA(Ts, Tt, Tq);
					TW = VFNMS(Ti, Tg, TV);
					TL = VMUL(Tl, TK);
				   }
			      }
			      TM = LD(&(ii[WS(rs, 7)]), ms, &(ii[WS(rs, 1)]));
			      TO = LD(&(ri[WS(rs, 3)]), ms, &(ri[WS(rs, 1)]));
			      TQ = LD(&(ii[WS(rs, 3)]), ms, &(ii[WS(rs, 1)]));
			      Tx = LD(&(ri[WS(rs, 1)]), ms, &(ri[WS(rs, 1)]));
			      Tz = LD(&(ii[WS(rs, 1)]), ms, &(ii[WS(rs, 1)]));
			      TD = LD(&(ri[WS(rs, 5)]), ms, &(ri[WS(rs, 1)]));
			      TH = LD(&(ii[WS(rs, 5)]), ms, &(ii[WS(rs, 1)]));
			 }
		    }
		    {
			 V Te, T1p, T1g, T10, TS, T18, T1d, T1t, T1x, T1y, Tv, TJ, T11, T16;
			 {
			      V TN, T1a, TR, T1c, TA, T13, TI, T15;
			      {
				   V TU, T19, TP, T1b, Ty, T12, TE, T14, TZ;
				   TU = VSUB(T1, Td);
				   Te = VADD(T1, Td);
				   TN = VFMA(Tn, TM, TL);
				   T19 = VMUL(Tl, TM);
				   TP = VMUL(T3, TO);
				   T1b = VMUL(T3, TQ);
				   Ty = VMUL(T2, Tx);
				   T12 = VMUL(T2, Tz);
				   TE = VMUL(TC, TD);
				   T14 = VMUL(TC, TH);
				   T1p = VADD(TW, TY);
				   TZ = VSUB(TW, TY);
				   T1a = VFNMS(Tn, TK, T19);
				   TR = VFMA(T6, TQ, TP);
				   T1c = VFNMS(T6, TO, T1b);
				   TA = VFMA(T5, Tz, Ty);
				   T13 = VFNMS(T5, Tx, T12);
				   TI = VFMA(TG, TH, TE);
				   T15 = VFNMS(TG, TD, T14);
				   T1g = VSUB(TU, TZ);
				   T10 = VADD(TU, TZ);
			      }
			      TS = VADD(TN, TR);
			      T18 = VSUB(TN, TR);
			      T1d = VSUB(T1a, T1c);
			      T1m = VADD(T1a, T1c);
			      T1t = VADD(T1r, T1s);
			      T1x = VSUB(T1s, T1r);
			      T1y = VSUB(Tk, Tu);
			      Tv = VADD(Tk, Tu);
			      TJ = VADD(TA, TI);
			      T11 = VSUB(TA, TI);
			      T16 = VSUB(T13, T15);
			      T1l = VADD(T13, T15);
			 }
			 {
			      V Tw, T1w, T1v, TT;
			      {
				   V T1i, T1e, T1B, T1z, T1h, T17;
				   T1i = VADD(T18, T1d);
				   T1e = VSUB(T18, T1d);
				   T1B = VADD(T1y, T1x);
				   T1z = VSUB(T1x, T1y);
				   T1h = VSUB(T16, T11);
				   T17 = VADD(T11, T16);
				   T1k = VSUB(Te, Tv);
				   Tw = VADD(Te, Tv);
				   {
					V T1A, T1j, T1C, T1f;
					T1A = VADD(T1h, T1i);
					T1j = VSUB(T1h, T1i);
					T1C = VSUB(T1e, T17);
					T1f = VADD(T17, T1e);
					T1w = VSUB(T1t, T1p);
					T1u = VADD(T1p, T1t);
					T1v = VSUB(TS, TJ);
					TT = VADD(TJ, TS);
					ST(&(ii[WS(rs, 1)]), VFMA(LDK(KP707106781), T1A, T1z), ms, &(ii[WS(rs, 1)]));
					ST(&(ii[WS(rs, 5)]), VFNMS(LDK(KP707106781), T1A, T1z), ms, &(ii[WS(rs, 1)]));
					ST(&(ri[WS(rs, 3)]), VFMA(LDK(KP707106781), T1j, T1g), ms, &(ri[WS(rs, 1)]));
					ST(&(ri[WS(rs, 7)]), VFNMS(LDK(KP707106781), T1j, T1g), ms, &(ri[WS(rs, 1)]));
					ST(&(ii[WS(rs, 3)]), VFMA(LDK(KP707106781), T1C, T1B), ms, &(ii[WS(rs, 1)]));
					ST(&(ii[WS(rs, 7)]), VFNMS(LDK(KP707106781), T1C, T1B), ms, &(ii[WS(rs, 1)]));
					ST(&(ri[WS(rs, 1)]), VFMA(LDK(KP707106781), T1f, T10), ms, &(ri[WS(rs, 1)]));
					ST(&(ri[WS(rs, 5)]), VFNMS(LDK(KP707106781), T1f, T10), ms, &(ri[WS(rs, 1)]));
				   }
			      }
			      ST(&(ri[WS(rs, 4)]), VSUB(Tw, TT), ms, &(ri[0]));
			      ST(&(ri[0]), VADD(Tw, TT), ms, &(ri[0]));
			      ST(&(ii[WS(rs, 6)]), VSUB(T1w, T1v), ms, &(ii[0]));
			      ST(&(ii[WS(rs, 2)]), VADD(T1v, T1w), ms, &(ii[0]));
			 }
		    }
	       }
	  }
	  T1n = VSUB(T1l, T1m);
	  T1o = VADD(T1l, T1m);
	  ST(&(ii[0]), VADD(T1o, T1u), ms, &(ii[0]));
	  ST(&(ii[WS(rs, 4)]), VSUB(T1u, T1o), ms, &(ii[0]));
	  ST(&(ri[WS(rs, 2)]), VADD(T1k, T1n), ms, &(ri[0]));
	  ST(&(ri[WS(rs, 6)]), VSUB(T1k, T1n), ms, &(ri[0]));
     }
}

static const tw_instr twinstr[] = {
     VTW(0, 1),
     VTW(0, 3),
     VTW(0, 7),
     {TW_NEXT, (2 * VL), 0}
};

static const ct_desc desc = { 8, "t2sv_8", twinstr, &GENUS, {44, 20, 30, 0}, 0, 0, 0 };

void X(codelet_t2sv_8) (planner *p) {
     X(kdft_dit_register) (p, t2sv_8, &desc);
}
#else				/* HAVE_FMA */

/* Generated by: ../../../genfft/gen_twiddle -simd -compact -variables 4 -pipeline-latency 8 -twiddle-log3 -precompute-twiddles -n 8 -name t2sv_8 -include ts.h */

/*
 * This function contains 74 FP additions, 44 FP multiplications,
 * (or, 56 additions, 26 multiplications, 18 fused multiply/add),
 * 42 stack variables, 1 constants, and 32 memory accesses
 */
#include "ts.h"

static void t2sv_8(R *ri, R *ii, const R *W, stride rs, INT mb, INT me, INT ms)
{
     DVK(KP707106781, +0.707106781186547524400844362104849039284835938);
     INT m;
     for (m = mb, W = W + (mb * 6); m < me; m = m + (2 * VL), ri = ri + ((2 * VL) * ms), ii = ii + ((2 * VL) * ms), W = W + ((2 * VL) * 6), MAKE_VOLATILE_STRIDE(rs)) {
	  V T2, T5, T3, T6, T8, Tc, Tg, Ti, Tl, Tm, Tn, Tz, Tp, Tx;
	  {
	       V T4, Tb, T7, Ta;
	       T2 = LDW(&(W[0]));
	       T5 = LDW(&(W[TWVL * 1]));
	       T3 = LDW(&(W[TWVL * 2]));
	       T6 = LDW(&(W[TWVL * 3]));
	       T4 = VMUL(T2, T3);
	       Tb = VMUL(T5, T3);
	       T7 = VMUL(T5, T6);
	       Ta = VMUL(T2, T6);
	       T8 = VSUB(T4, T7);
	       Tc = VADD(Ta, Tb);
	       Tg = VADD(T4, T7);
	       Ti = VSUB(Ta, Tb);
	       Tl = LDW(&(W[TWVL * 4]));
	       Tm = LDW(&(W[TWVL * 5]));
	       Tn = VFMA(T2, Tl, VMUL(T5, Tm));
	       Tz = VFNMS(Ti, Tl, VMUL(Tg, Tm));
	       Tp = VFNMS(T5, Tl, VMUL(T2, Tm));
	       Tx = VFMA(Tg, Tl, VMUL(Ti, Tm));
	  }
	  {
	       V Tf, T1i, TL, T1d, TJ, T17, TV, TY, Ts, T1j, TO, T1a, TC, T16, TQ;
	       V TT;
	       {
		    V T1, T1c, Te, T1b, T9, Td;
		    T1 = LD(&(ri[0]), ms, &(ri[0]));
		    T1c = LD(&(ii[0]), ms, &(ii[0]));
		    T9 = LD(&(ri[WS(rs, 4)]), ms, &(ri[0]));
		    Td = LD(&(ii[WS(rs, 4)]), ms, &(ii[0]));
		    Te = VFMA(T8, T9, VMUL(Tc, Td));
		    T1b = VFNMS(Tc, T9, VMUL(T8, Td));
		    Tf = VADD(T1, Te);
		    T1i = VSUB(T1c, T1b);
		    TL = VSUB(T1, Te);
		    T1d = VADD(T1b, T1c);
	       }
	       {
		    V TF, TW, TI, TX;
		    {
			 V TD, TE, TG, TH;
			 TD = LD(&(ri[WS(rs, 7)]), ms, &(ri[WS(rs, 1)]));
			 TE = LD(&(ii[WS(rs, 7)]), ms, &(ii[WS(rs, 1)]));
			 TF = VFMA(Tl, TD, VMUL(Tm, TE));
			 TW = VFNMS(Tm, TD, VMUL(Tl, TE));
			 TG = LD(&(ri[WS(rs, 3)]), ms, &(ri[WS(rs, 1)]));
			 TH = LD(&(ii[WS(rs, 3)]), ms, &(ii[WS(rs, 1)]));
			 TI = VFMA(T3, TG, VMUL(T6, TH));
			 TX = VFNMS(T6, TG, VMUL(T3, TH));
		    }
		    TJ = VADD(TF, TI);
		    T17 = VADD(TW, TX);
		    TV = VSUB(TF, TI);
		    TY = VSUB(TW, TX);
	       }
	       {
		    V Tk, TM, Tr, TN;
		    {
			 V Th, Tj, To, Tq;
			 Th = LD(&(ri[WS(rs, 2)]), ms, &(ri[0]));
			 Tj = LD(&(ii[WS(rs, 2)]), ms, &(ii[0]));
			 Tk = VFMA(Tg, Th, VMUL(Ti, Tj));
			 TM = VFNMS(Ti, Th, VMUL(Tg, Tj));
			 To = LD(&(ri[WS(rs, 6)]), ms, &(ri[0]));
			 Tq = LD(&(ii[WS(rs, 6)]), ms, &(ii[0]));
			 Tr = VFMA(Tn, To, VMUL(Tp, Tq));
			 TN = VFNMS(Tp, To, VMUL(Tn, Tq));
		    }
		    Ts = VADD(Tk, Tr);
		    T1j = VSUB(Tk, Tr);
		    TO = VSUB(TM, TN);
		    T1a = VADD(TM, TN);
	       }
	       {
		    V Tw, TR, TB, TS;
		    {
			 V Tu, Tv, Ty, TA;
			 Tu = LD(&(ri[WS(rs, 1)]), ms, &(ri[WS(rs, 1)]));
			 Tv = LD(&(ii[WS(rs, 1)]), ms, &(ii[WS(rs, 1)]));
			 Tw = VFMA(T2, Tu, VMUL(T5, Tv));
			 TR = VFNMS(T5, Tu, VMUL(T2, Tv));
			 Ty = LD(&(ri[WS(rs, 5)]), ms, &(ri[WS(rs, 1)]));
			 TA = LD(&(ii[WS(rs, 5)]), ms, &(ii[WS(rs, 1)]));
			 TB = VFMA(Tx, Ty, VMUL(Tz, TA));
			 TS = VFNMS(Tz, Ty, VMUL(Tx, TA));
		    }
		    TC = VADD(Tw, TB);
		    T16 = VADD(TR, TS);
		    TQ = VSUB(Tw, TB);
		    TT = VSUB(TR, TS);
	       }
	       {
		    V Tt, TK, T1f, T1g;
		    Tt = VADD(Tf, Ts);
		    TK = VADD(TC, TJ);
		    ST(&(ri[WS(rs, 4)]), VSUB(Tt, TK), ms, &(ri[0]));
		    ST(&(ri[0]), VADD(Tt, TK), ms, &(ri[0]));
		    {
			 V T19, T1e, T15, T18;
			 T19 = VADD(T16, T17);
			 T1e = VADD(T1a, T1d);
			 ST(&(ii[0]), VADD(T19, T1e), ms, &(ii[0]));
			 ST(&(ii[WS(rs, 4)]), VSUB(T1e, T19), ms, &(ii[0]));
			 T15 = VSUB(Tf, Ts);
			 T18 = VSUB(T16, T17);
			 ST(&(ri[WS(rs, 6)]), VSUB(T15, T18), ms, &(ri[0]));
			 ST(&(ri[WS(rs, 2)]), VADD(T15, T18), ms, &(ri[0]));
		    }
		    T1f = VSUB(TJ, TC);
		    T1g = VSUB(T1d, T1a);
		    ST(&(ii[WS(rs, 2)]), VADD(T1f, T1g), ms, &(ii[0]));
		    ST(&(ii[WS(rs, 6)]), VSUB(T1g, T1f), ms, &(ii[0]));
		    {
			 V T11, T1k, T14, T1h, T12, T13;
			 T11 = VSUB(TL, TO);
			 T1k = VSUB(T1i, T1j);
			 T12 = VSUB(TT, TQ);
			 T13 = VADD(TV, TY);
			 T14 = VMUL(LDK(KP707106781), VSUB(T12, T13));
			 T1h = VMUL(LDK(KP707106781), VADD(T12, T13));
			 ST(&(ri[WS(rs, 7)]), VSUB(T11, T14), ms, &(ri[WS(rs, 1)]));
			 ST(&(ii[WS(rs, 5)]), VSUB(T1k, T1h), ms, &(ii[WS(rs, 1)]));
			 ST(&(ri[WS(rs, 3)]), VADD(T11, T14), ms, &(ri[WS(rs, 1)]));
			 ST(&(ii[WS(rs, 1)]), VADD(T1h, T1k), ms, &(ii[WS(rs, 1)]));
		    }
		    {
			 V TP, T1m, T10, T1l, TU, TZ;
			 TP = VADD(TL, TO);
			 T1m = VADD(T1j, T1i);
			 TU = VADD(TQ, TT);
			 TZ = VSUB(TV, TY);
			 T10 = VMUL(LDK(KP707106781), VADD(TU, TZ));
			 T1l = VMUL(LDK(KP707106781), VSUB(TZ, TU));
			 ST(&(ri[WS(rs, 5)]), VSUB(TP, T10), ms, &(ri[WS(rs, 1)]));
			 ST(&(ii[WS(rs, 7)]), VSUB(T1m, T1l), ms, &(ii[WS(rs, 1)]));
			 ST(&(ri[WS(rs, 1)]), VADD(TP, T10), ms, &(ri[WS(rs, 1)]));
			 ST(&(ii[WS(rs, 3)]), VADD(T1l, T1m), ms, &(ii[WS(rs, 1)]));
		    }
	       }
	  }
     }
}

static const tw_instr twinstr[] = {
     VTW(0, 1),
     VTW(0, 3),
     VTW(0, 7),
     {TW_NEXT, (2 * VL), 0}
};

static const ct_desc desc = { 8, "t2sv_8", twinstr, &GENUS, {56, 26, 18, 0}, 0, 0, 0 };

void X(codelet_t2sv_8) (planner *p) {
     X(kdft_dit_register) (p, t2sv_8, &desc);
}
#endif				/* HAVE_FMA */
