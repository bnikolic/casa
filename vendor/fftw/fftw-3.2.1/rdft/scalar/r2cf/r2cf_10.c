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
/* Generated on Mon Feb  9 19:53:45 EST 2009 */

#include "codelet-rdft.h"

#ifdef HAVE_FMA

/* Generated by: ../../../genfft/gen_r2cf -fma -reorder-insns -schedule-for-pipeline -compact -variables 4 -pipeline-latency 4 -n 10 -name r2cf_10 -include r2cf.h */

/*
 * This function contains 34 FP additions, 14 FP multiplications,
 * (or, 24 additions, 4 multiplications, 10 fused multiply/add),
 * 29 stack variables, 4 constants, and 20 memory accesses
 */
#include "r2cf.h"

static void r2cf_10(R *R0, R *R1, R *Cr, R *Ci, stride rs, stride csr, stride csi, INT v, INT ivs, INT ovs)
{
     DK(KP559016994, +0.559016994374947424102293417182819058860154590);
     DK(KP250000000, +0.250000000000000000000000000000000000000000000);
     DK(KP618033988, +0.618033988749894848204586834365638117720309180);
     DK(KP951056516, +0.951056516295153572116439333379382143405698634);
     INT i;
     for (i = v; i > 0; i = i - 1, R0 = R0 + ivs, R1 = R1 + ivs, Cr = Cr + ovs, Ci = Ci + ovs, MAKE_VOLATILE_STRIDE(rs), MAKE_VOLATILE_STRIDE(csr), MAKE_VOLATILE_STRIDE(csi)) {
	  E Tt, T3, T7, Tq, T6, Tv, Tp, Tm, Th, T8, T1, T2, T9, Tr;
	  T1 = R0[0];
	  T2 = R1[WS(rs, 2)];
	  {
	       E Te, Tn, Td, Tf, Tb, Tc;
	       Tb = R0[WS(rs, 2)];
	       Tc = R1[WS(rs, 4)];
	       Te = R0[WS(rs, 3)];
	       Tt = T1 + T2;
	       T3 = T1 - T2;
	       Tn = Tb + Tc;
	       Td = Tb - Tc;
	       Tf = R1[0];
	       {
		    E T4, T5, To, Tg;
		    T4 = R0[WS(rs, 1)];
		    T5 = R1[WS(rs, 3)];
		    T7 = R0[WS(rs, 4)];
		    To = Te + Tf;
		    Tg = Te - Tf;
		    Tq = T4 + T5;
		    T6 = T4 - T5;
		    Tv = Tn + To;
		    Tp = Tn - To;
		    Tm = Tg - Td;
		    Th = Td + Tg;
		    T8 = R1[WS(rs, 1)];
	       }
	  }
	  T9 = T7 - T8;
	  Tr = T7 + T8;
	  {
	       E Ty, Tk, Tx, Tj, Tu, Ts;
	       Tu = Tq + Tr;
	       Ts = Tq - Tr;
	       {
		    E Ta, Tl, Tw, Ti;
		    Ta = T6 + T9;
		    Tl = T6 - T9;
		    Ci[WS(csi, 4)] = KP951056516 * (FMA(KP618033988, Tp, Ts));
		    Ci[WS(csi, 2)] = KP951056516 * (FNMS(KP618033988, Ts, Tp));
		    Ty = Tu - Tv;
		    Tw = Tu + Tv;
		    Ci[WS(csi, 3)] = KP951056516 * (FMA(KP618033988, Tl, Tm));
		    Ci[WS(csi, 1)] = -(KP951056516 * (FNMS(KP618033988, Tm, Tl)));
		    Tk = Ta - Th;
		    Ti = Ta + Th;
		    Cr[0] = Tt + Tw;
		    Tx = FNMS(KP250000000, Tw, Tt);
		    Cr[WS(csr, 5)] = T3 + Ti;
		    Tj = FNMS(KP250000000, Ti, T3);
	       }
	       Cr[WS(csr, 4)] = FMA(KP559016994, Ty, Tx);
	       Cr[WS(csr, 2)] = FNMS(KP559016994, Ty, Tx);
	       Cr[WS(csr, 3)] = FNMS(KP559016994, Tk, Tj);
	       Cr[WS(csr, 1)] = FMA(KP559016994, Tk, Tj);
	  }
     }
}

static const kr2c_desc desc = { 10, "r2cf_10", {24, 4, 10, 0}, &GENUS };

void X(codelet_r2cf_10) (planner *p) {
     X(kr2c_register) (p, r2cf_10, &desc);
}

#else				/* HAVE_FMA */

/* Generated by: ../../../genfft/gen_r2cf -compact -variables 4 -pipeline-latency 4 -n 10 -name r2cf_10 -include r2cf.h */

/*
 * This function contains 34 FP additions, 12 FP multiplications,
 * (or, 28 additions, 6 multiplications, 6 fused multiply/add),
 * 26 stack variables, 4 constants, and 20 memory accesses
 */
#include "r2cf.h"

static void r2cf_10(R *R0, R *R1, R *Cr, R *Ci, stride rs, stride csr, stride csi, INT v, INT ivs, INT ovs)
{
     DK(KP250000000, +0.250000000000000000000000000000000000000000000);
     DK(KP559016994, +0.559016994374947424102293417182819058860154590);
     DK(KP951056516, +0.951056516295153572116439333379382143405698634);
     DK(KP587785252, +0.587785252292473129168705954639072768597652438);
     INT i;
     for (i = v; i > 0; i = i - 1, R0 = R0 + ivs, R1 = R1 + ivs, Cr = Cr + ovs, Ci = Ci + ovs, MAKE_VOLATILE_STRIDE(rs), MAKE_VOLATILE_STRIDE(csr), MAKE_VOLATILE_STRIDE(csi)) {
	  E Ti, Tt, Ta, Tn, Td, To, Te, Tv, T3, Tq, T6, Tr, T7, Tu, Tg;
	  E Th;
	  Tg = R0[0];
	  Th = R1[WS(rs, 2)];
	  Ti = Tg - Th;
	  Tt = Tg + Th;
	  {
	       E T8, T9, Tb, Tc;
	       T8 = R0[WS(rs, 2)];
	       T9 = R1[WS(rs, 4)];
	       Ta = T8 - T9;
	       Tn = T8 + T9;
	       Tb = R0[WS(rs, 3)];
	       Tc = R1[0];
	       Td = Tb - Tc;
	       To = Tb + Tc;
	  }
	  Te = Ta + Td;
	  Tv = Tn + To;
	  {
	       E T1, T2, T4, T5;
	       T1 = R0[WS(rs, 1)];
	       T2 = R1[WS(rs, 3)];
	       T3 = T1 - T2;
	       Tq = T1 + T2;
	       T4 = R0[WS(rs, 4)];
	       T5 = R1[WS(rs, 1)];
	       T6 = T4 - T5;
	       Tr = T4 + T5;
	  }
	  T7 = T3 + T6;
	  Tu = Tq + Tr;
	  {
	       E Tl, Tm, Tf, Tj, Tk;
	       Tl = Td - Ta;
	       Tm = T3 - T6;
	       Ci[WS(csi, 1)] = FNMS(KP951056516, Tm, KP587785252 * Tl);
	       Ci[WS(csi, 3)] = FMA(KP587785252, Tm, KP951056516 * Tl);
	       Tf = KP559016994 * (T7 - Te);
	       Tj = T7 + Te;
	       Tk = FNMS(KP250000000, Tj, Ti);
	       Cr[WS(csr, 1)] = Tf + Tk;
	       Cr[WS(csr, 5)] = Ti + Tj;
	       Cr[WS(csr, 3)] = Tk - Tf;
	  }
	  {
	       E Tp, Ts, Ty, Tw, Tx;
	       Tp = Tn - To;
	       Ts = Tq - Tr;
	       Ci[WS(csi, 2)] = FNMS(KP587785252, Ts, KP951056516 * Tp);
	       Ci[WS(csi, 4)] = FMA(KP951056516, Ts, KP587785252 * Tp);
	       Ty = KP559016994 * (Tu - Tv);
	       Tw = Tu + Tv;
	       Tx = FNMS(KP250000000, Tw, Tt);
	       Cr[WS(csr, 2)] = Tx - Ty;
	       Cr[0] = Tt + Tw;
	       Cr[WS(csr, 4)] = Ty + Tx;
	  }
     }
}

static const kr2c_desc desc = { 10, "r2cf_10", {28, 6, 6, 0}, &GENUS };

void X(codelet_r2cf_10) (planner *p) {
     X(kr2c_register) (p, r2cf_10, &desc);
}

#endif				/* HAVE_FMA */
