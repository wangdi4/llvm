/*******************************************************************************
* INTEL CONFIDENTIAL
* Copyright 1996-2022 Intel Corporation.
*
* This software and the related documents are Intel copyrighted  materials,  and
* your use of  them is  governed by the  express license  under which  they were
* provided to you (License).  Unless the License provides otherwise, you may not
* use, modify, copy, publish, distribute,  disclose or transmit this software or
* the related documents without Intel's prior written permission.
*
* This software and the related documents  are provided as  is,  with no express
* or implied  warranties,  other  than those  that are  expressly stated  in the
* License.
*******************************************************************************/
/*
// ALGORITHM DESCRIPTION:
// 
//   CONVENTIONS
//   A = B  denotes that A is equal to B.
//   A := B denotes assignment of a value of the expression B to
//          the variable A. All operations and roundings in B are considered
//          to be in the target precision.
//   <A>    denotes the rounding the IEEE-754 floating-point value A to the
//          target precision.
//   {A}    denotes the rounding the IEEE-754 floating-point value A to the
//          nearest integer.
//   ex(A)  denotes unbiased binary exponent of a number A so that
//          A = significand(A) * 2^ex(A).
// 
//   HIGH LEVEL OVERVIEW
// 
//   Denote x = a[i], y = b[i].
// 
//   "Main" path.
//       When input arguments x, y are nonzero finite numbers, |x|<>1, and
//       x>0 or x<0 and y is integer, then we use the formula:
// 
//           |x^y| = 2^( y * log2|x| ), where
// 
//       x^y>0 if x>0 or x<0 and y is an even integer,
//       x^y<0 if x<0 and y is an odd integer.
// 
//   Other paths.
//       Cases for other combinations of input arguments are
//       described in IEEE SPECIAL CONDITIONS table below.
// 
//   IEEE SPECIAL CONDITIONS:
//   The following table describes the results for pow(x,y) expected by C99
//   standard. In case of a cell is empty, the result must be computed using
//   mathematical formula and rounded to target precision: pow(x,y) = <x^y>.
//      \   x ||     |      |    |      |    |    |      |    |      |     |
//        \   ||-Inf |finite| -1 |-1<x<0| -0 | +0 |0<x<1 |  1 |finite|+Inf | NaN
//    y     \ ||     | x<-1 |    |      |    |    |      |    |  x>1 |     |
//   =========++=====+======+====+======+====+====+======+====+======+=====+=====
//     +Inf   ||+Inf | +Inf |  1 |  +0  | +0 | +0 |  +0  |  1 | +Inf |+Inf | NaN
//   ---------++-----+------+----+------+----+----+------+----+------+-----+-----
//      odd   ||     |      |    |      |    |    |      |    |      |     |
//    integer ||-Inf |      | -1 |      | -0 | +0 |      |  1 |      |+Inf | NaN
//      y>0   ||     |      |    |      |    |    |      |    |      |     |
//   ---------++-----+------+----+------+----+----+------+----+------+-----+-----
//     even   ||     |      |    |      |    |    |      |    |      |     |
//    integer ||+Inf |      |  1 |      | +0 | +0 |      |  1 |      |+Inf | NaN
//      y>0   ||     |      |    |      |    |    |      |    |      |     |
//   ---------++-----+------+----+------+----+----+------+----+------+-----+-----
//    finite  ||     | EDOM |EDOM| EDOM |    |    |      |    |      |     |
//    y>0 not ||+Inf |  NaN | NaN|  NaN | +0 | +0 |      |  1 |      |+Inf | NaN
//    integer ||     |  INV | INV|  INV |    |    |      |    |      |     |
//   ---------++-----+------+----+------+----+----+------+----+------+-----+-----
//      +0    ||  1  |   1  |  1 |   1  |  1 |  1 |   1  |  1 |   1  |  1  |  1
//   ---------++-----+------+----+------+----+----+------+----+------+-----+-----
//      -0    ||  1  |   1  |  1 |   1  |  1 |  1 |   1  |  1 |   1  |  1  |  1
//   ---------++-----+------+----+------+----+----+------+----+------+-----+-----
//    finite  ||     | EDOM |EDOM| EDOM |EDOM|EDOM|      |    |      |     |
//    y<0 not || +0  |  NaN | NaN|  NaN |+Inf|+Inf|      |  1 |      | +0  | NaN
//    integer ||     |  INV | INV|  INV |DIVZ|DIVZ|      |    |      |     |
//   ---------++-----+------+----+------+----+----+------+----+------+-----+-----
//     even   ||     |      |    |      |EDOM|EDOM|      |    |      |     |
//    integer || +0  |      |  1 |      |+Inf|+Inf|      |  1 |      | +0  | NaN
//      y<0   ||     |      |    |      |DIVZ|DIVZ|      |    |      |     |
//   ---------++-----+------+----+------+----+----+------+----+------+-----+-----
//      odd   ||     |      |    |      |EDOM|EDOM|      |    |      |     |
//    integer || -0  |      | -1 |      |-Inf|+Inf|      |  1 |      | +0  | NaN
//      y<0   ||     |      |    |      |DIVZ|DIVZ|      |    |      |     |
//   ---------++-----+------+----+------+----+----+------+----+------+-----+-----
//            ||     |      |    |      |EDOM|EDOM|      |    |      |     |
//     -Inf   || +0  |  +0  |  1 | +Inf |+Inf|+Inf| +Inf |  1 |  +0  | +0  | NaN
//            ||     |      |    |      |DIVZ|DIVZ|      |    |      |     |
//   ---------++-----+------+----+------+----+----+------+----+------+-----+-----
//      NaN   || NaN |  NaN | NaN|  NaN | NaN| NaN|  NaN |  1 |  NaN | NaN | NaN
// 
//   Here:
//       EDOM means Domain error,
//       INV  means Invalid floating point exception,
//       DIVZ means Divide-by-zero floating point exception.
// 
//   Invalid floating point exception is raised when one of arguments is SNaN.
// 
//   Possible deviations of the algorithm from C99 standard:
//       Inexact flag can be raised even in some cases when x^y is exact finite
//       normalized number. Inexact and Underflow flags are raised in all cases
//       when x^y is exact denormalized.
// 
// 
//   ALGORITHM DETAILS
//   A careful algorithm must be used to realize mathematical ideas accurately.
//   In addition a number of execution paths required to handle special and
//   subtle cases.
// 
//   At first, we check whether y is integer and save the result of this check
//   into iYIsInt variable.
//       The result is NOTINT when y is not an integer (here NOTINT=0).
//       The result is ODD    when y is an odd  integer (here ODD=1).
//       The result is EVEN   when y is an even integer (here EVEN=2).
// 
//       The idea of the algorithm for this check is as follows:
// 
//       If y is Inf or NaN then iYIsInt:=NOTINT,
//       else if |y| >= 2^53 then iYIsInt:=EVEN,
//       else if y=0 then iYIsInt:=EVEN,
//       else if |y|<1 then iYIsInt:=NOTINT,
//       else if fractional part of y is not zero then iYIsInt:=NOTINT,
//       else if the lowest bit in integer part of y is zero then iYIsInt:=EVEN,
//       else iYIsInt:=ODD.
// 
//   After that pow algorithm follows.
// 
//   1)  x=1 or y=0
//       r[i] := 1
// 
//   2)  One of arguments is NaN, but x<>1, y<>0
//       r[i] := x + y
// 
//   3)  x=0, y<>NaN, y<>0
// 
//       3.1) y<0, x=0, y<>NaN
//           If x=-0, y is negative odd integer
//           then r[i] := -Inf
//           else r[i] := +Inf
//           Raise DIVZ
// 
//           Error handling routine is called with IML_STATUS_ERRDOM.
// 
//       3.2) y>0, x=0, y<>NaN
//           If x=-0, y is positive odd integer
//           then r[i] := -0
//           else r[i] := +0
// 
//   4)  x=-1, y is non-zero integer or Inf
//       If y is odd integer
//       then r[i] := -1
//       else r[i] := +1
// 
//   5)  one of arguments is Inf, but none of them is zero or NaN, and |x|<>1
// 
//       5.1) |x|<1, y=-Inf
//           r[i] := +Inf
// 
//       5.2) |x|<1, y=+Inf
//           r[i] := +0
// 
//       5.3) |x|>1, y<0
//           If x<0, y is negative odd integer
//           then r[i] := -0
//           else r[i] := +0
// 
//       5.4) |x|>1, y>0
//           If x<0, y is positive odd integer
//           then r[i] := -Inf
//           else r[i] := +Inf
//           (Here we use equality x*x*y=+Inf.)
// 
//   6)  -Inf<x<0, y is finite non-integer
//       r[i] := 0/0 (resulting with NaN, and raising INV)
// 
//       Error handling routine is called with IML_STATUS_ERRDOM.
// 
//   7)  "Main" path: x,y are finite nonzero numbers, |x|<>1,
//       and if x<0 then y is integer
// 
//       7.a) Get sign of the result into SignRes
//           Sign of result here is negative only if x<0 and y is odd.
// 
//       7.b) Start calculating log2|x|
// 
//           Here we use the following formula.
//           Let |x|=2^k1*X1, where k1 is integer, 1<=X1<2.
//           Let C ~= 1/ln(2),
//           Rcp1 ~= 1/X1,   X2=Rcp1*X1,
//           Rcp2 ~= 1/X2,   X3=Rcp2*X2,
//           Rcp3 ~= 1/X3,   Rcp3C ~= C/X3.
//           Then
// 
//               log2|x| = k1 + log2(1/Rcp1) + log2(1/Rcp2) + log2(C/Rcp3C) +
//                       + log2(X1*Rcp1*Rcp2*Rcp3C/C),
// 
//           where X1*Rcp1*Rcp2*Rcp3C = C*(1+q), q is very small.
// 
//           The values of Rcp1, log2(1/Rcp1), Rcp2, log2(1/Rcp2),
//           Rcp3C, log2(C/Rcp3C) are taken from tables.
//           Values of Rcp1, Rcp2, Rcp3C are such that RcpC=Rcp1*Rcp2*Rcp3C
//           is exactly represented in target precision.
// 
//           log2(X1*Rcp1*Rcp2*Rcp3C/C) = log2(1+q) = ln(1+q)/ln2 =
//               = 1/(ln2)*q - 1/(2ln2)*q^2 + 1/(3ln2)*q^3 - ... =
//               = 1/(C*ln2)*cq - 1/(2*C^2*ln2)*cq^2 + 1/(3*C^3*ln2)*cq^3 - ... =
//               = (1 + a1)*cq + a2*cq^2 + a3*cq^3 + ...,
//           where
//               cq=X1*Rcp1*Rcp2*Rcp3C-C,
//               a1=1/(C*ln(2))-1 is small,
//               a2=1/(2*C^2*ln2),
//               a3=1/(3*C^3*ln2),
//               ...
// 
//           Calculation of log2|x| is performed as follows.
// 
//           7.b.1) Getting X1
//               At first, represent |x| in the form |x| = 2^iDenoExpAdd * AX,
//               where AX is normalized.
// 
//               Then get X1 by copying
// 
//                   X1 := AX
// 
//               and setting exponent field of X1 to biased 0.
// 
//           7.b.2) Getting k
//               Get high 32 bits of AX into XHi.
// 
//               Get k using XHi:
// 
//                   k := XHi - K_SUB
//                   k := k >> IML_DP_NUM_HI_SIG_BITS
//                   k := k + DenoExpAdd
// 
//               where K_SUB is high 32 bits of (1.5-2^(-rcpK1-1))/2,
//               rcpK1=5 in this implementation.
// 
//               So we have:
// 
//                   k=k1   if X1< 1.5-2^(-rcpK1-1),
//                   k=k1+1 if X1>=1.5-2^(-rcpK1-1).
// 
//               Instead of k1, we will use k.
// 
//           7.b.3) Get Rcp1, log2(1/Rcp1) from tables
//               Get index i1 from rcpK1 most significand bits of X1.
//               Get Rcp1.
//               Get log2(1/Rcp1) from a table as sum of two values L1Hi+L1Lo:
//                   L1Hi+L1Lo~=log2(1/Rcp1)   if X1< 1.5-2^(-rcpK1-1),
//                   L1Hi+L1Lo~=log2(1/Rcp1)-1 if X1>=1.5-2^(-rcpK1-1).
// 
//           7.b.4) Get Rcp2, log2(1/Rcp2) from tables
//               Get X2.
//               Get index i2 from rcpK2 bits of significand of X2.
//               rcpK2=5 in this implementation.
//               Get Rcp2.
//               Get log2(1/Rcp2) from a table as sum of two values
//               L2Hi+L2Lo ~= log2(1/Rcp2).
// 
//           7.b.5) Get Rcp3C, log2(C/Rcp3C) from tables
//               Get X3.
//               Get index i3 from rcpK3 bits of significand of X3.
//               rcpK3=7 in this implementation.
//               Get Rcp3C.
//               Get log2(C/Rcp3C) from a table as sum of two values
//               L3Hi+L3Lo ~= log2(C/Rcp3C).
// 
//           7.b.6) Recombine k+log2(1/Rcp1)+log2(1/Rcp2)+log2(C/Rcp3C)
//               T := k + L1Hi + L2Hi + L3Hi
//               D :=     L1Lo + L2Lo + L3Lo
// 
//               Now we have
// 
//                   log2|x| ~= T + D + log2(Rcp1*Rcp2*Rcp3C*X1/C).
// 
//           7.b.7) Get approximation CQ to cq
//               R1 := <<<X1*Rcp1>*Rcp2>*Rcp3C>
//               CQ := R1 - C  (the subrtaction is computed exactly)
// 
//           7.b.8) Get the correction term E for CQ
//               We have cq=X1*RcpC-C, CQ=R1-C, cq=CQ+e.
//               So the exact correction term e=X1*RcpC-R1.
//               Approximation E to e is computed in multiprecision:
// 
//               RcpC := Rcp1 * Rcp2 * Rcp3C
// 
//               Split X1 into sum X1Hi+X1Lo so that X1Hi^2 is exactly
//               representable in target precision.
// 
//               Split RcpC into sum RcpCHi+RcpCLo so that RcpCHi^2 is exactly
//               representable in target precision.
// 
//               Computing E:
//               E := X1Hi*RcpCHi-R1
//               E := E + X1Lo*RcpCHi
//               E := E + X1Hi*RcpCLo
//               E := E + X1Lo*RcpCLo
// 
//               Now we have CQ+E that represent cq more exactly than CQ.
// 
//       Now we have
// 
//           log2|x| ~= T + D + CQ + E + a1*CQ + a2*CQ^2 + a3*CQ^3 + ...
// 
//       7.c) Get high part and exponent of log2|x|
//           Rebreak T + CQ into sum of high and low parts T_CQHi + CQLo.
//           Get exponent of T_CQHi into ELogAX variable.
// 
//       7.d) Estimate |y*log2|x||
//           Using EYB=ex(y) and ELogAX, we estimate whether |y*log2|x||
//           is such that 2^(y*log2|x|) rounds to Inf, or 1, or 0 in target
//           precision, or it should be computed more accurately.
// 
//       7.1) Here if ex(y) + ex(log2|x|) >= 11.
//           Here we have 2^11 <= |y*log2|x|| < Inf.
// 
//           Get sign of y*log|x|.
// 
//           If y*log|x|>0 then Tmp1=BIG_VALUE else Tmp1=SMALL_VALUE, where
//           BIG_VALUE=2^1023, SMALL_VALUE=2^(-1022) in this implementation.
// 
//           Tmp1 := Tmp1 * Tmp1
// 
//           r[i] := Tmp1 * SignRes
// 
//       7.2) Here if ex(y) + ex(log2|x|) <= -62.
//           Here we have 0 < |y*log2|x|| <= 4*2^(-62).
// 
//           Tmp1 := ONE
//           Tmp1 := Tmp1 + SMALL_VALUE
//           r[i] := Tmp1 * SignRes
// 
// 
//       7.3) Here if -62 < ex(y) + ex(log2|x|) < 11.
//           Here we have 2^(-61) <= |y*log2|x|| < 4*2^10.
// 
//           7.3.a) R := CQ + E.
//               R represents cq more exactly than CQ.
// 
//           7.3.b) Polynomial.
//               Log2Poly := A1*R + A2*R^2 + A3*R^3 + A4*R^4,
// 
//               where A1=<a1>, ..., A4=<a4>.
// 
//           7.3.c) Get 3 parts of log2|x|.
//               We have log2|x| ~= T_CQHi + Log2Poly + D + CQLo + E.
//               Represent log2|x| in the form of sum HH+HL+HLL.
// 
//               LogPart3 := CQLo + E + D
//               Rebreak T_CQHi + Log2Poly into HH + HL
//               Now we have HH + HL + LogPart3 ~= log2|x|.
// 
//               Rebreak HH + LogPart3 into HH + HLL.
//               HLL := HLL + HL
// 
//               Split HH into HH + HL so that HH^2 is exactly representable
//               in target precision.
// 
//               Now we have HH+HL+HLL ~= log2|x|.
// 
//           7.3.d) Calculation of y*(HH+HL+HLL).
//               Split y into YHi+YLo.
//               Get high PH and medium PL parts of y*log2|x|.
//               Get low PLL part of y*log2|x|.
//               Now we have PH+PL+PLL ~= y*log2|x|.
// 
//           7.3.e) Calculation of 2^(PH+PL+PLL).
// 
//               Mathematical idea of computing 2^(PH+PL+PLL) is the following.
//               Let's represent PH+PL+PLL in the form N + j/2^expK + Z,
//               where expK=7 in this implementation, N and j are integers,
//               0<=j<=2^expK-1, |Z|<2^(-expK-1). Hence
// 
//                   2^(PH+PL+PLL) ~= 2^N * 2^(j/2^expK) * 2^Z,
// 
//               where 2^(j/2^expK) is stored in a table, and
// 
//                   2^Z ~= 1 + B1*Z + B2*Z^2 ... + B5*Z^5.
// 
//               We compute 2^(PH+PL+PLL) as follows.
// 
//               Break PH into PHH + PHL, where PHH = N + j/2^expK.
//               Z = PHL + PL + PLL
//               Exp2Poly = B1*Z + B2*Z^2 ... + B5*Z^5
//               Get 2^(j/2^expK) from table in the form THI+TLO.
//               Now we have 2^(PH+PL+PLL) ~= 2^N * (THI + TLO) * (1 + Exp2Poly).
// 
//               Get significand of 2^(PH+PL+PLL) in the form ResHi+ResLo:
//               ResHi := THI
//               ResLo := THI * Exp2Poly + TLO
// 
//               Get exponent ERes of the result:
//               Res := ResHi + ResLo:
//               ERes := ex(Res) + N
// 
//               Now we can check whether result is normalized, denormalized,
//               overflowed or underflowed.
// 
//               7.3.e.1) Here if ERes >= 1024.
//                   The result is overflowed.
// 
//                   Tmp1 := BIG_VALUE * BIG_VALUE
//                   r[i] := Tmp1 * SignRes
// 
//               7.3.e.2) Here if ERes < -1074-10.
//                   The result is underflowed.
// 
//                   Tmp1 := SMALL_VALUE * SMALL_VALUE
//                   r[i] := Tmp1 * SignRes
// 
//               7.3.e.3) Here if -1074-10 <= ERes < -1022-10.
//                   The result is a small denormalized number.
// 
//                   SignRes := SignRes * DENO_UNSCALE
//                   N       := N + DENO_SCALE_EXP
// 
//                   where DENO_UNSCALE=2^(-200),
//                   DENO_SCALE_EXP=200 in this implementation.
// 
//                   TwoPowN := 2^N
//                   r[i] := Res * TwoPowN * SignRes
// 
//               7.3.e.4) Here if -1022-10 <= ERes < -1022.
//                   The result is a big denormalized number.
// 
//                   Rebreak ResHi+ResLo
// 
//                   SignRes := SignRes * DENO_UNSCALE
//                   N       := N + DENO_SCALE_EXP
// 
//                   TwoPowN := 2^N
// 
//                   ResHi := ResHi * TwoPowN * SignRes
//                   ResLo := ResLo * TwoPowN * SignRes
// 
//                   Res  := ResHi + ResLo
//                   r[i] := Res + SMALL_VALUE * SMALL_VALUE
// 
//               7.3.e.5) Here if -1022 <= ERes <= 1023.
//                   The result is normalized.
// 
//                   Res  := 2^N * Res
//                   r[i] := Res * SignRes
// 
// 
*/
#include "_imf_include_fp32.hpp"
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
#pragma omp declare target
#endif
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_pow_s_la {
namespace {
// LUT-free pow
// #define _POW_S_NOLUT_
/* file: _vspow_cout_ats.i */
// Macros
// biased exponent when x>0
// exponent bits
// fractional mantissa bits
// 32-bit multiply, low 32 bits
// signed 32-bit multiply, high 32 bits
// unsigned 32-bit multiply, high 32 bits
// unsigned 32-bit multiply, full bits
// signed 32-bit multiply, full bits
// unsigned 32-bit multiply, high 32 bits
// shift 64-bit integer and save upper 32 bits
// unsigned 32-bit shift
// signed 32-bit shift
// unsigned 64-bit shift
// signed 64-bit shift
// rcp(1 + index/16)*2^8 - 1
static const unsigned char __spow_la___rcp_tbl[] = {
    0xff, 0xf0, 0xe3, 0xd7, 0xcc, 0xc2, 0xb9, 0xb1, 0xaa,
    0xa3, 0x9d, 0x97, 0x91, 0x8c, 0x88, 0x83, 0x7f,
};
// -log2(_VSTATIC(__rcp_tbl)[i]/2^8))*2^(23+32)
static const uint64_t __spow_la___log2_tbl[] = {
    0x0000000000000000UL, 0x000b2671360338acUL, 0x001563dc29ffacb2UL,
    0x001f5fd8a9063e36UL, 0x002906cbcd2baf2eUL, 0x003243001249ba76UL,
    0x003afcd815786af2UL, 0x00431b2abc31565cUL, 0x004a83cf0d01c170UL,
    0x00523bbc64c5e644UL, 0x00591db662b66428UL, 0x006043e946fd97f4UL,
    0x0067b3d42fd0fc50UL, 0x006e232e68aad484UL, 0x007373af48dce654UL,
    0x007a514b229c40a0UL, 0x0080000000000000UL,
};
// polynomial coefficients
// c6*2^31
static const int32_t __spow_la___lc6 = 0xE158260E;
// c5*2^31
static const int32_t __spow_la___lc5 = 0x24F7FD36;
// c4*2^31
static const int32_t __spow_la___lc4 = 0xD1D568F0;
// c3*2^31
static const int32_t __spow_la___lc3 = 0x3D8E12ED;
// c2*2^31
static const int32_t __spow_la___lc2 = 0xA3AAE26C;
// c1*2^(23+32)
static const uint64_t __spow_la___lc1 = 0xB8AA3B295EBB00UL;
// exp2 coefficients
// c7*2^32
static const int32_t __spow_la___sc7 = 0x00016B68;
// c6*2^32
static const int32_t __spow_la___sc6 = 0x00095E83;
// c5*2^32
static const int32_t __spow_la___sc5 = 0x00580436;
// c4*2^32
static const int32_t __spow_la___sc4 = 0x027607DE;
// c3*2^32
static const int32_t __spow_la___sc3 = 0x0E359872;
// c2*2^32
static const int32_t __spow_la___sc2 = 0x3D7F7977;
// c1*2^32
static const int32_t __spow_la___sc1 = 0xB1721817;
static uint32_t __spow_la_powf_cout(uint32_t xin, uint32_t yin, int *errcode) {
  int32_t mant, expon, index, sgn_y, R, poly, N;
  int32_t expon_y, is_int, mant_y, mi_y;
  uint32_t rcp, res, shift, abs_y, poly_low, poly_h, sgn_x = 0, p_inf;
  uint64_t poly64, exp64, poly_s1;
  // unpack mantissa, unbiased exponent
  mant = ((xin)&0x7fffff);
  expon = ((xin) >> 23) - 0x7f;
  abs_y = yin & 0x7fffffff;
  sgn_y = (((int32_t)(yin)) >> (31));
  if ((((uint32_t)(abs_y - 1)) >= (0x7F800000 - 1)))
    goto SPOW_SPECIAL_Y;
  // filter out special and negative cases, as well as denormals
  if ((((uint32_t)(xin - 0x00800000)) >= (0x7F800000 - 0x00800000)))
    goto SPOW_SPECIAL_X;
SPOW_LOG_MAIN:
  // add leading mantissa bit
  mant |= 0x00800000;
  // table index
  index = ((mant + 0x00040000) >> (23 - 4)) - 0x10;
  // rcp ~ 2^8/mant
  rcp = 1 + __spow_la___rcp_tbl[index];
  // reduced argument R = mant*rcp - 1, scale 2^32
  R = (((uint32_t)mant) * ((uint32_t)rcp)); // scale 2^31
  R = R + R;
  // (c6*R+c5)*2^31
  poly = ((((int64_t)((int32_t)(__spow_la___lc6))) * ((int32_t)(R))) >> 32);
  poly = poly + __spow_la___lc5;
  // poly*R+c4, scale 2^31
  poly = ((((int64_t)((int32_t)(poly))) * ((int32_t)(R))) >> 32);
  poly = poly + __spow_la___lc4;
  // poly*R+c3, scale 2^31
  poly = ((((int64_t)((int32_t)(poly))) * ((int32_t)(R))) >> 32);
  poly = poly + __spow_la___lc3;
  // poly*R+c2, scale 2^31
  poly = ((((int64_t)((int32_t)(poly))) * ((int32_t)(R))) >> 32);
  poly = poly + __spow_la___lc2;
  // poly*2^(23+32)
  poly_low = poly << (32 - 8);
  poly_h = (((int32_t)(poly)) >> (8));
  // c1+R*poly, scale 2^(23+32)
  poly64 = (((int64_t)((int32_t)(poly_h))) * ((int32_t)(R))) + __spow_la___lc1;
  // poly_low to be treated as positive value
  poly_low = (((uint32_t)(poly_low)) >> (1));
  poly_low = ((((int64_t)((int32_t)(poly_low))) * ((int32_t)(R))) >> 32);
  poly_low += poly_low;
  poly64 += (int64_t)((int32_t)poly_low);
  // adjustment for x near 1.0
  shift = 0x7f + 21;
  if (!((expon << 4) + index)) {
    poly64 <<= 7;
    shift = 7 + 0x7f + 21;
    // is x exactly 1.0?
    if (!R)
      return sgn_x | 0x3f800000;
  }
  // T+R*poly, scale 2^(2+32+(shift-bias))
  poly_low = (uint32_t)poly64;
  poly_h = (uint32_t)(poly64 >> 32);
  poly64 = (((int64_t)((int32_t)(poly_h))) * ((int32_t)(R))) +
           __spow_la___log2_tbl[index];
  // adjust for sign of poly_low
  poly_low = (((uint32_t)(poly_low)) >> (1));
  poly_low = ((((int64_t)((int32_t)(poly_low))) * ((int32_t)(R))) >> 32);
  poly_low += poly_low;
  poly64 += (int64_t)((int32_t)poly_low);
  // log2(x) ~ expon+T+R*poly, sc 2^(2+32+(shift-bias))
  expon <<= 23;
  exp64 = (uint64_t)expon;
  poly64 += (exp64 << 32);
  poly_s1 = poly64 << 1;
  while (poly_s1 && (((int64_t)(poly_s1 ^ poly64)) >= 0)) {
    poly64 = poly_s1;
    poly_s1 <<= 1;
    shift++;
  }
  // y, sc 2^(30-expon_y)
  // unpack mantissa, biased exponent
  expon_y = shift - ((abs_y) >> 23);
  mant = ((abs_y)&0x7fffff);
  // denormal y?
  if (abs_y < 0x00800000)
    expon_y = shift - 1;
  else
    mant |= 0x00800000;
  // apply sign to mantissa bits
  mant = (mant ^ sgn_y) - sgn_y;
  // mant, scale 2^30
  mant <<= 7;
  // y*log2(x), sc 2^(2+30 + expon_y)
  poly_low = (uint32_t)poly64;
  poly_h = (uint32_t)(poly64 >> 32);
  poly64 = (((int64_t)((int32_t)(poly_h))) * ((int32_t)(mant)));
  // adjust for sign of poly_low
  poly_low = (((uint32_t)(poly_low)) >> (1));
  poly_low = ((((int64_t)((int32_t)(poly_low))) * ((int32_t)(mant))) >> 32);
  poly_low += poly_low;
  poly64 += (int64_t)((int32_t)poly_low);
  if (expon_y < 0) // overflow/underflow
  {
    poly_h = (uint32_t)(poly64 >> 32);
    if (((int32_t)poly_h) < 0)
      goto SPOW_UF;
    goto SPOW_OF;
  }
  if (expon_y >= 32) {
    expon_y -= 32;
    poly64 = (((int64_t)(poly64)) >> (32));
    if (expon_y >= 32)
      return sgn_x | 0x3f800000;
  }
  // integer part in high 32 bits, fractional bits in low part
  poly64 = (((int64_t)(poly64)) >> (expon_y));
  N = (uint32_t)(poly64 >> 32);
  // reduced exp2 argument, sc 2^32
  R = (uint32_t)poly64;
  // (c7*R+c6)*2^32
  poly = ((((uint64_t)((uint32_t)(__spow_la___sc7))) * ((uint32_t)(R))) >> 32);
  poly = poly + __spow_la___sc6;
  // poly*2^32
  poly = ((((uint64_t)((uint32_t)(poly))) * ((uint32_t)(R))) >> 32);
  poly = poly + __spow_la___sc5;
  // poly*2^32
  poly = ((((uint64_t)((uint32_t)(poly))) * ((uint32_t)(R))) >> 32);
  poly = poly + __spow_la___sc4;
  // poly*2^32
  poly = ((((uint64_t)((uint32_t)(poly))) * ((uint32_t)(R))) >> 32);
  poly = poly + __spow_la___sc3;
  // poly*2^32
  poly = ((((uint64_t)((uint32_t)(poly))) * ((uint32_t)(R))) >> 32);
  poly = poly + __spow_la___sc2;
  // poly*2^32
  poly = ((((uint64_t)((uint32_t)(poly))) * ((uint32_t)(R))) >> 32);
  poly = poly + __spow_la___sc1;
  // poly*2^32
  poly = ((((uint64_t)((uint32_t)(poly))) * ((uint32_t)(R))) >> 32);
  // rounding and overflow/underflow checking
  // poly*2^31
  poly = (((uint32_t)(poly)) >> (1)) + 128;
  expon = N + 0x7f;
  N = expon + (((uint32_t)(poly)) >> (31));
  // overflow?
  if (N >= 0xff)
    goto SPOW_OF;
  // underflow, possibly gradual?
  if (N <= 0)
    goto SPOW_GRAD_UF;
  res = sgn_x | ((expon << 23) + (((uint32_t)(poly)) >> (8)));
  return res;
SPOW_OF:
  res = sgn_x | 0x7f800000;
  *errcode = 3;
  return res; // goto POWF_ERRCALL;
SPOW_GRAD_UF:
  if (N < -24)
    goto SPOW_UF;
  // poly*2^31, undo rounding to 24 bits
  poly = poly + 0x80000000 - 128;
  N = expon;
  while (N < 1) {
    poly = (((uint32_t)(poly)) >> (1));
    N++;
  }
  poly = (((uint32_t)(poly + 128)) >> (8));
  if (poly)
    return sgn_x | poly;
SPOW_UF:
  res = sgn_x;
  *errcode = 4;
  return res; // goto POWF_ERRCALL;
SPOW_SPECIAL_Y:
  // 0, Inf, NaN
  // 0?
  if (!abs_y)
    return 0x3f800000;
  // NaN?
  if (abs_y > 0x7f800000)
    return ((xin == 0x3f800000) ? xin : 0xffc00000);
  // +/-Inf
  // x is NaN?
  if (((uint32_t)(xin + xin)) > 0xff000000u)
    return 0xffc00000;
  // |x| == 1?
  R = (xin & 0x7fffffff) - 0x3f800000;
  if (R == 0)
    return 0x3f800000;
  R ^= sgn_y;
  if (((int32_t)R) < 0)
    return 0;
  res = 0x7f800000;
  if (!(xin + xin)) {
    *errcode = 1;
  }
  return res;
SPOW_SPECIAL_X:
  p_inf = 0x7f800000;
  // +Inf?
  if (xin == p_inf)
    return (sgn_y ? 0 : xin);
  // NaN
  if (((uint32_t)(xin + xin)) > 0xff000000u)
    return 0xffc00000;
  if (((int32_t)xin) > 0) {
  SPOW_DENORM_X:
    // denormal input, normalize
    expon = 1 - 0x7f;
    while (mant < 0x00800000) {
      expon--;
      mant <<= 1;
    }
    // return to main computation
    goto SPOW_LOG_MAIN;
  }
  // is y an integer?
  is_int = 0;
  if (abs_y >= 0x3f800000) {
    if (abs_y >= 0x4b800000)
      is_int = 1; // and even integer (>=2^24)
    else {
      shift = 23 + 0x7f - (((uint32_t)(abs_y)) >> (23));
      mant_y = ((abs_y)&0x7fffff) | 0x00800000;
      mi_y = (((uint32_t)(mant_y)) >> (shift));
      if (mant_y == (mi_y << shift)) {
        is_int = 1;
        // set sign for odd integers
        sgn_x = mi_y << 31;
      }
    }
  }
  // +/-zero?
  if (!(xin + xin)) {
    if (!sgn_y)
      return 0;
    sgn_x &= xin;
    res = sgn_x | 0x7f800000;
    *errcode = 1;
    return res; // goto POWF_ERRCALL;
  }
  // negative?
  if (((int32_t)xin) < 0) {
    if (xin == 0xff800000)
      return (sgn_y ? sgn_x : (sgn_x | 0x7f800000));
    if (!is_int) {
      *errcode = 1;
      res = 0xffc00000;
      return res; // goto POWF_ERRCALL;
    }
    expon -= 0x100;
    if (xin == 0xbf800000)
      return sgn_x | 0x3f800000;
    if (expon >= -126)
      goto SPOW_LOG_MAIN;
    goto SPOW_DENORM_X;
  }
  return xin;
}
static const union {
  uint32_t w;
  float f;
} __spow_la_slog_c1 = {0x3eaaaaa8};
static const union {
  uint32_t w;
  float f;
} __spow_la_slog_c2 = {0x3e4cd0b0};
static const union {
  uint32_t w;
  float f;
} __spow_la_slog_c3 = {0x3e1166f0};
static const union {
  uint32_t w;
  float f;
} __spow_la_slog_c4 = {0x3e046000};
static const union {
  uint32_t w;
  float f;
} __spow_la_slog_m181o256 = {0x3f350000};
static const union {
  uint32_t w;
  float f;
} __spow_la_slog_half = {0x3f000000};
static const union {
  uint32_t w;
  float f;
} __spow_la_slog_two = {0x40000000};
static const union {
  uint32_t w;
  float f;
} __spow_la_slog_log2hi = {0x3f317218};
static const union {
  uint32_t w;
  float f;
} __spow_la_slog_log2lo = {0xb102e308};
static const union {
  uint32_t w;
  float f;
} __spow_la_sexp_shft = {0x4ac000feu};
static const union {
  uint32_t w;
  float f;
} __spow_la_sexp_l2e = {0x3FB8AA3Bu};
static const union {
  uint32_t w;
  float f;
} __spow_la_sexp_l2h = {0x3f317218u};
static const union {
  uint32_t w;
  float f;
} __spow_la_sexp_l2l = {0xb102E308u};
static const union {
  uint32_t w;
  float f;
} __spow_la_sexp_c5 = {0x3c08ba8bu};
static const union {
  uint32_t w;
  float f;
} __spow_la_sexp_c4 = {0x3d2aec4eu};
static const union {
  uint32_t w;
  float f;
} __spow_la_sexp_c3 = {0x3e2aaa9cu};
static const union {
  uint32_t w;
  float f;
} __spow_la_sexp_c2 = {0x3effffe8u};
static const union {
  uint32_t w;
  float f;
} __spow_la_sexp_c1 = {0x3f800000u};
static inline float __internal_frexpf(float arg, int *exp_res) {
  uint32_t uX;
  float fS, fR, fOne;
  uX = ((*(int *)&arg) & ~0x80000000) - 0x00800000;
  // Normal args
  if (uX < 0x7f800000 - 0x00800000) {
    (*(int *)&fR) = (*(int *)&arg) & 0x807fffff;
    (*(int *)&fR) |= 0x3f000000;
    *exp_res = ((int)(uX >> 23) - (0x007F - 1)) + 1;
  }
  // Subnormal args
  else {
    (*(int *)&fR) = (*(int *)&arg) | 0x3f000000;
    (*(int *)&fS) = 0x3f000000 | ((*(int *)&fR)) & 0x80000000;
    fR = fR - fS;
    uX = (*(int *)&fR);
    uX = uX & 0x7f800000;
    (*(int *)&fR) &= ~0x7f800000;
    (*(int *)&fR) |= 0x3f000000;
    *exp_res = ((int)(uX >> 23) - (0x007F - 1)) - 125;
  }
  return fR;
}
inline int __devicelib_imf_internal_spow(const float *pxin, const float *pyin,
                                         float *pres) {
  int nRet = 0;
  union {
    uint32_t w;
    float f;
  } fwX, fwY;
  union {
    uint32_t w;
    float f;
  } fwYLogX;
  union {
    uint32_t w;
    float f;
  } fwS, fwTh, fwTh2, fwRes;
  float fX, fY;
  float fN, fR, fPoly;
  float fExpArgHi, fExpArgLo;
  float fLogResHi, fLogResLo, fLogTHi, fLogTLo, fLogPolyHi, fLogPolyLo, fLogR;
  float fLogMant, fLogExp, fLogV3, fLogV2, fLogV1, fLogFHi, fLogFLo;
  uint32_t uXa32, uSgnX, uExpCorr;
  uint32_t uExpX;
  uint32_t uAbsYLogX;
  int32_t iIdxMask;
  int32_t iExp32, iMask32, iMaskH;
  int32_t iExpX, iSmallX;
  fX = *pxin;
  fY = *pyin;
  fwX.f = *pxin;
  fwY.f = *pyin;
  // Argument reduction to range from 181/256 to 362/256:
  // Get mantissa and exponent of argument
  fLogMant = __internal_frexpf(fX, &iExpX);
  // If mantissa less than 181/256 then multiply it by 2 and decrease exponent
  // by 1
  iSmallX = (fLogMant < __spow_la_slog_m181o256.f);
  fLogMant = (iSmallX) ? (2.0f * fLogMant) : fLogMant;
  iExpX = (iSmallX) ? (iExpX - 1) : iExpX;
  // Fraction f = (fLogMant-1)/(fLogMant+1)
  fLogV1 = __fma(fLogMant, 1.0f, 1.0f);
  fLogMant = __fma(fLogMant, 1.0f, -1.0f);
  fLogR = 1.0f / fLogV1;
  fLogFHi = __fma(fLogMant, fLogR, 0.0f);
  fLogV2 = __fma(fLogFHi, -__spow_la_slog_two.f, fLogMant);
  fLogV3 = __fma(fLogFHi, -fLogMant, fLogV2);
  fLogFLo = __fma(fLogR, fLogV3, 0.0f);
  // atanh(f) approximation
  fLogV3 = fLogFHi * fLogFHi;
  fLogR =
      __fma(__spow_la_slog_c4.f, fLogV3, __spow_la_slog_c3.f);
  fLogR = __fma(fLogR, fLogV3, __spow_la_slog_c2.f);
  fLogR = __fma(fLogR, fLogV3, __spow_la_slog_c1.f);
  fLogV2 =
      __fma(fLogFHi, fLogFLo + fLogFLo,
                             __fma(fLogFHi, fLogFHi, -fLogV3));
  fLogV1 = __fma(fLogV3, fLogFHi, 0.0f);
  fLogV2 = __fma(
      fLogV3, fLogFLo,
      __fma(fLogV2, fLogFHi,
                             __fma(fLogV3, fLogFHi, -fLogV1)));
  fLogV3 = __fma(
      fLogR, fLogV1, __fma(fLogR, fLogV2, fLogFLo));
  // Log exponent as floating point
  fLogExp = (float)iExpX;
  // log = 2 * atanh(f) + fLogExp * log(2)
  fLogV2 = __fma(
      __fma(__spow_la_slog_half.f, __spow_la_slog_log2hi.f,
                             0.0f),
      fLogExp, fLogFHi);
  fLogV1 = __fma(
      __fma(__spow_la_slog_half.f, -__spow_la_slog_log2hi.f,
                             0.0f),
      fLogExp, fLogV2);
  fLogV3 = __fma(
      __fma(fLogFHi, 1.0f, -fLogV1), 1.0f, fLogV3);
  fLogV3 = __fma(
      __spow_la_slog_log2lo.f * __spow_la_slog_half.f, fLogExp, fLogV3);
  fLogR = __fma(fLogV2, 1.0f, fLogV2);
  // log result in high and low parts
  fLogResHi = __fma(__spow_la_slog_two.f, fLogV3, fLogR);
  fLogResLo =
      __fma(__spow_la_slog_two.f, fLogV3,
                             __fma(fLogR, 1.0f, -fLogResHi));
  // Multiply  y * log(x)
  fLogTHi = __fma(fLogResHi, fY, 0.0f);
  // Check for special args processing branch
  fwYLogX.f = (float)(fLogTHi);
  uExpX = fwX.w >> 23;
  uExpX--;
  uAbsYLogX = fwYLogX.w & 0x7fffffffu;
  if ((uExpX >= 0xfe) || (uAbsYLogX >= 0x42afb6e0)) {
    goto SPOW_MAIN_SPECIAL;
  }
  // Rest of multi-precision  y * log(x)
  fLogTLo = __fma(fLogResHi, fY, -fLogTHi);
  fLogTLo = __fma(fLogResLo, fY, +fLogTLo);
  fLogPolyHi = __fma(fLogTHi, 1.0f, fLogTLo);
  fLogPolyLo = __fma(
      __fma(fLogTHi, 1.0f, -fLogPolyHi), 1.0f, +fLogTLo);
  fExpArgHi = fLogPolyHi;
  fExpArgLo = fLogPolyLo;
  // Exp part computation
  fwS.f = __fma(fExpArgHi, __spow_la_sexp_l2e.f,
                                 __spow_la_sexp_shft.f);
  fN = fwS.f - __spow_la_sexp_shft.f;
  fR = __fma(-(fN), __spow_la_sexp_l2h.f, fExpArgHi);
  fR = __fma(-(fN), __spow_la_sexp_l2l.f, fR);
  fR += fExpArgLo;
  // Set exponent in place
  fwTh.w = fwS.w << 22;
  // iIdxMask is based on last bit of fwS.w
  iIdxMask = 0 - (fwS.w & 1);
  // Set fwTh mantissa
  fwTh.w ^= (iIdxMask & 0x7504F3u);
  // Exp polynomial
  fPoly = __fma(fR, __spow_la_sexp_c5.f, __spow_la_sexp_c4.f);
  fPoly = __fma(fR, fPoly, __spow_la_sexp_c3.f);
  fPoly = __fma(fR, fPoly, __spow_la_sexp_c2.f);
  fPoly = __fma(fR, fPoly, __spow_la_sexp_c1.f);
  fPoly = fR * fPoly;
  // Big abs exp arg branch
  if (uAbsYLogX > 0x42AEAC4Fu) {
    fwS.w += 0xfe;
    fwTh2.w = (fwS.w >> 2) & 0xff;
    fwS.w -= (fwTh2.w << 1);
    fwTh2.w <<= 23;
    fwTh.w = fwS.w << 22;
    fwTh.w ^= (iIdxMask & 0x7504F3u);
    fwRes.f = __fma(fPoly, fwTh.f, fwTh.f);
    fwRes.f *= fwTh2.f;
  } else {
    fwRes.f = __fma(fPoly, fwTh.f, fwTh.f);
  }
  *pres = fwRes.f;
  return nRet;
SPOW_MAIN_SPECIAL:
  fwRes.w = __spow_la_powf_cout(fwX.w, fwY.w, &nRet);
  *pres = fwRes.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_pow_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_powf(float a, float b) {
  using namespace __imf_impl_pow_s_la;
  float r;
  __devicelib_imf_internal_spow(&a, &b, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
DEVICE_EXTERN_C_DECLSIMD_INLINE
float __svml_device_powf(float x, float y) {
  return __devicelib_imf_powf(x, y);
}
#pragma omp end declare target
#endif
