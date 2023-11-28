/*******************************************************************************
* INTEL CONFIDENTIAL
* Copyright 1996 Intel Corporation.
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
//      HIGH LEVEL OVERVIEW
//      Here we use polynomial and reciprocal calculation for 32 subintervals
//      at reduction interval.
// 
//      For large arguments ( |a[i]| >= LARGE_ARG_HIBITS,
//      ( where LARGE_ARG_HIBITS = 16 high bits of 12800.0 value )
//      the Payne/Hanek "pre-reduction" performed. Result of this routine
//      becomes argument for regular reduction.
// 
//      The regular range reduction scheme is:
// 
//          a[i] = N * pi/2^R + z
//          where R = 5 for this implementation - reduction value (2^R = 32),
//          and |z| <= pi/(2*2^R)) - this is reduced argument.
// 
//      Also alternative reduction performed in parallel:
// 
//          a[i] = NN * pi/2 + zz,
//          (NN = N mod 16)
// 
//      The reason is getting remainder modulo Pi/2. The value zz is used
//      for reciprocal calculation.
// 
//      Futher tan calculation performed by this way:
// 
//         r[i] := TAU * 1/zz + C0 + C1*r + C1*c + C2*r^2 + ... + C15*r^15
// 
//         (TAU - multiplier for the reciprocal 1/zz,
//         and always -1 or 0 depending on subinterval)
// 
//      For tiny arguments ( |a[i]| < 2^TINY_ARG_EXP )
//      the simple separate branch used:
// 
//          r[i] = a[i]
// 
//      IEEE SPECIAL CONDITIONS:
//      a[i] = +/-Inf, r[i] = QNaN
//      a[i] = +/-0,   r[i] = +/-0
//      a[i] = QNaN,   r[i] = QNaN
//      a[i] = SNaN,   r[i] = QNaN
// 
// 
//      ALGORITHM DETAILS
//      Executable parts:
// 
//      1) a[i]  = +/-Inf
//         Return r[i] := a[i]*0.0
//         and error handler called with IML_STATUS_ERRDOM error code
// 
//      2) a[i]  = NaN
//         Return r[i] := a[i] * a[i]
// 
//      3) Tiny arguments path
//         |a[i]| < 2^TINY_ARG_EXP,
//         where TINY_ARG_EXP = -252
// 
//         3.1) a[i]  = 0.0
//              Return r[i] := a[i]
// 
//         3.2) 0 < |a[i]| < 2^TINY_ARG_EXP
//              Return r[i] := TWOp55 * ( TWOpM55*a[i] - a[i] ),
//              where TWOp55 = 2^55, TWOpM55 = 2^-55
// 
//              Here is path where underflow or denormal exceptions can happen
//              during intermediate computations.
//              For correct work in all rounding modes we need to
//              return a[i] - TWOpM55 * a[i]
//              To avoid disappearing of second term we using scaling
//              like this TWOp55 * ( TWOpM55*a[i] - a[i] )
// 
// 
//      4) Main path (the most frequently used and the most wide)
//         2^TINY_ARG_EXP <= |a[i]| < LARGE_ARG_HIBITS
// 
//         a) Pre-reduction.
// 
//            For large arguments |a[i]| >= LARGE_ARG_HIBITS
//            special argument pre-range-reduction routine is called:
//            NR := _vml_reduce_pio2d( a[i], rr ),
//            where NR - number of octants of pre-reduction,
//            rr[0], rr[1] - high and low parts of pre-reduced argument.
//            The Payne/Hanek algorithm is used there (not described).
//            Assign   x := rr[0]
//            In case of no pre-reduction   x := a[i]
// 
//         b) Main reduction.
// 
//            The standard range reduction scheme is
//            zc := x - N * (PIo32_HI + PIo32_LO + PIo32_TAIL)
//            zc - reduced argument
// 
//            Integer N obtained by famous "right-shifter" technique -
//            add and subtract RS = 2^52+2^51 value.
// 
//            After that we add N := N + NR*(2^(R-1))
//            if large arguments pre-reduction
//            routine called.  NR = result octant number of Pi/2
//            pre-reduction.
//            For a[i] < LARGE_ARG_HIBITS the NR = 0 and N is unchanged.
// 
//            PIo32_HI and PIo32_LO are 32-bit numbers (so multiplication
//            by N is exact) and PIo32_TAIL is a 53-bit number. Together, these
//            approximate pi well enough for all cases in this restricted
//            range. Reduction performed in accurate way with low part (c) of
//            result correct processing.
//            For large arguments added c = c + rr[1].
//            Finally we have zc = z + c multiprecision value.
// 
//            In parallel we are doing another reduction for
//            getting remainder modulo Pi/2.  Here we perform
//            a sort of "more rounding".
//            It means have the same computation sequences but using N = (N mod
// 16)
//            that is also obtained by "right shifter" technique,
//            where right shifter value is (2^55+2^56) instead of usual
// (2^51+2^52)
//            Pi values presented by 38+38+53 form for accurate multiplication by
//            14-bit of (N mod 16).
//            The result is zzc = zz+cc multiprecision value.
// 
//            For existing large arguments reduction we need to add
//            extra low part rr[1] to c and cc correction terms.
//            c := c + rr[1],  cc := cc + rr[1]
//            but it is necessary to resplit z+c and zz + cc values
//            to preserve proportions betwee high and low parts.
//            Doing it this way:
// 
//               v1 := z + c;   v2 := z - v1;   c := v2 + c;   z := v1;
//               v1 := zz + cc; v2 := zz - v1;  cc := v2 + cc; zz := v1;
// 
//         c) General computations.
// 
//            The whole computation range (Pi/2) is splitted to
//            32 even ranges and for each breakpoint we have
//            unique set of coefficients stored as table.
//            The table lookup performed by index that is 5 least
//            significant bits of integer N (octant number) value.
// 
//            The constants are:
//            1) C2 ... C15 polynomial coefficients for r^2 ... r^15
//            2) C0_HI + C0_LO - accurate constant C0 term in power series
//            3) C1_HI + C1_LO - accurate coefficient C1 of r in power series
//            4) TAU - multiplier for the reciprocal, always -1 or 0
//            5) MSK - 35 significant bit mask for the reciprocal
// 
// 
//            The basic reconstruction formula using these constants is:
// 
//               High := TAU*recip_hi + C0_HI
//               Med + Low := C1_HI*r + C1_LO*r (accurate sum)
//               Low  := Low + TAU*recip_lo + C0_LO + (C1_LO+C1_HI)*c + pol,
//                 where pol := C2*r^2 + ... + C15*r^15
// 
//            The recip_hi + recip_lo is an accurate reciprocal of the remainder
//            modulo pi/2 = 1/zz
//            Finally we doing a compensated sum High + Med + Low:
// 
//            Return r[i] := (High + (Med + Low))
// --
// 
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_tan_s_la {
namespace {
/* VML  8 bit optimal UF :========= HA LA EP */
/* VML 64 bit optimal UF:========== HA LA EP */
/* file: _vsreduction_data.i */
typedef struct {
  VUINT32 _sPtable[256][3];
} __stan_la_ReductionTab_t;
static const __stan_la_ReductionTab_t
    __devicelib_imf_internal_stan_reduction_data = {{
        /*     P_hi                  P_med               P_lo                */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 0 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 1 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 2 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 3 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 4 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 5 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 6 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 7 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 8 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 9 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 10 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 11 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 12 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 13 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 14 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 15 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 16 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 17 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 18 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 19 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 20 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 21 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 22 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 23 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 24 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 25 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 26 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 27 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 28 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 29 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 30 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 31 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 32 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 33 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 34 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 35 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 36 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 37 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 38 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 39 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 40 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 41 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 42 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 43 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 44 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 45 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 46 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 47 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 48 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 49 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 50 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 51 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 52 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 53 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 54 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 55 */
        {0x00000000u, 0x00000000u, 0x00000000u}, /* 56 */
        {0x00000000u, 0x00000000u, 0x00000001u}, /* 57 */
        {0x00000000u, 0x00000000u, 0x00000002u}, /* 58 */
        {0x00000000u, 0x00000000u, 0x00000005u}, /* 59 */
        {0x00000000u, 0x00000000u, 0x0000000Au}, /* 60 */
        {0x00000000u, 0x00000000u, 0x00000014u}, /* 61 */
        {0x00000000u, 0x00000000u, 0x00000028u}, /* 62 */
        {0x00000000u, 0x00000000u, 0x00000051u}, /* 63 */
        {0x00000000u, 0x00000000u, 0x000000A2u}, /* 64 */
        {0x00000000u, 0x00000000u, 0x00000145u}, /* 65 */
        {0x00000000u, 0x00000000u, 0x0000028Bu}, /* 66 */
        {0x00000000u, 0x00000000u, 0x00000517u}, /* 67 */
        {0x00000000u, 0x00000000u, 0x00000A2Fu}, /* 68 */
        {0x00000000u, 0x00000000u, 0x0000145Fu}, /* 69 */
        {0x00000000u, 0x00000000u, 0x000028BEu}, /* 70 */
        {0x00000000u, 0x00000000u, 0x0000517Cu}, /* 71 */
        {0x00000000u, 0x00000000u, 0x0000A2F9u}, /* 72 */
        {0x00000000u, 0x00000000u, 0x000145F3u}, /* 73 */
        {0x00000000u, 0x00000000u, 0x00028BE6u}, /* 74 */
        {0x00000000u, 0x00000000u, 0x000517CCu}, /* 75 */
        {0x00000000u, 0x00000000u, 0x000A2F98u}, /* 76 */
        {0x00000000u, 0x00000000u, 0x00145F30u}, /* 77 */
        {0x00000000u, 0x00000000u, 0x0028BE60u}, /* 78 */
        {0x00000000u, 0x00000000u, 0x00517CC1u}, /* 79 */
        {0x00000000u, 0x00000000u, 0x00A2F983u}, /* 80 */
        {0x00000000u, 0x00000000u, 0x0145F306u}, /* 81 */
        {0x00000000u, 0x00000000u, 0x028BE60Du}, /* 82 */
        {0x00000000u, 0x00000000u, 0x0517CC1Bu}, /* 83 */
        {0x00000000u, 0x00000000u, 0x0A2F9836u}, /* 84 */
        {0x00000000u, 0x00000000u, 0x145F306Du}, /* 85 */
        {0x00000000u, 0x00000000u, 0x28BE60DBu}, /* 86 */
        {0x00000000u, 0x00000000u, 0x517CC1B7u}, /* 87 */
        {0x00000000u, 0x00000000u, 0xA2F9836Eu}, /* 88 */
        {0x00000000u, 0x00000001u, 0x45F306DCu}, /* 89 */
        {0x00000000u, 0x00000002u, 0x8BE60DB9u}, /* 90 */
        {0x00000000u, 0x00000005u, 0x17CC1B72u}, /* 91 */
        {0x00000000u, 0x0000000Au, 0x2F9836E4u}, /* 92 */
        {0x00000000u, 0x00000014u, 0x5F306DC9u}, /* 93 */
        {0x00000000u, 0x00000028u, 0xBE60DB93u}, /* 94 */
        {0x00000000u, 0x00000051u, 0x7CC1B727u}, /* 95 */
        {0x00000000u, 0x000000A2u, 0xF9836E4Eu}, /* 96 */
        {0x00000000u, 0x00000145u, 0xF306DC9Cu}, /* 97 */
        {0x00000000u, 0x0000028Bu, 0xE60DB939u}, /* 98 */
        {0x00000000u, 0x00000517u, 0xCC1B7272u}, /* 99 */
        {0x00000000u, 0x00000A2Fu, 0x9836E4E4u}, /* 100 */
        {0x00000000u, 0x0000145Fu, 0x306DC9C8u}, /* 101 */
        {0x00000000u, 0x000028BEu, 0x60DB9391u}, /* 102 */
        {0x00000000u, 0x0000517Cu, 0xC1B72722u}, /* 103 */
        {0x00000000u, 0x0000A2F9u, 0x836E4E44u}, /* 104 */
        {0x00000000u, 0x000145F3u, 0x06DC9C88u}, /* 105 */
        {0x00000000u, 0x00028BE6u, 0x0DB93910u}, /* 106 */
        {0x00000000u, 0x000517CCu, 0x1B727220u}, /* 107 */
        {0x00000000u, 0x000A2F98u, 0x36E4E441u}, /* 108 */
        {0x00000000u, 0x00145F30u, 0x6DC9C882u}, /* 109 */
        {0x00000000u, 0x0028BE60u, 0xDB939105u}, /* 110 */
        {0x00000000u, 0x00517CC1u, 0xB727220Au}, /* 111 */
        {0x00000000u, 0x00A2F983u, 0x6E4E4415u}, /* 112 */
        {0x00000000u, 0x0145F306u, 0xDC9C882Au}, /* 113 */
        {0x00000000u, 0x028BE60Du, 0xB9391054u}, /* 114 */
        {0x00000000u, 0x0517CC1Bu, 0x727220A9u}, /* 115 */
        {0x00000000u, 0x0A2F9836u, 0xE4E44152u}, /* 116 */
        {0x00000000u, 0x145F306Du, 0xC9C882A5u}, /* 117 */
        {0x00000000u, 0x28BE60DBu, 0x9391054Au}, /* 118 */
        {0x00000000u, 0x517CC1B7u, 0x27220A94u}, /* 119 */
        {0x00000000u, 0xA2F9836Eu, 0x4E441529u}, /* 120 */
        {0x00000001u, 0x45F306DCu, 0x9C882A53u}, /* 121 */
        {0x00000002u, 0x8BE60DB9u, 0x391054A7u}, /* 122 */
        {0x00000005u, 0x17CC1B72u, 0x7220A94Fu}, /* 123 */
        {0x0000000Au, 0x2F9836E4u, 0xE441529Fu}, /* 124 */
        {0x00000014u, 0x5F306DC9u, 0xC882A53Fu}, /* 125 */
        {0x00000028u, 0xBE60DB93u, 0x91054A7Fu}, /* 126 */
        {0x00000051u, 0x7CC1B727u, 0x220A94FEu}, /* 127 */
        {0x000000A2u, 0xF9836E4Eu, 0x441529FCu}, /* 128 */
        {0x00000145u, 0xF306DC9Cu, 0x882A53F8u}, /* 129 */
        {0x0000028Bu, 0xE60DB939u, 0x1054A7F0u}, /* 130 */
        {0x00000517u, 0xCC1B7272u, 0x20A94FE1u}, /* 131 */
        {0x00000A2Fu, 0x9836E4E4u, 0x41529FC2u}, /* 132 */
        {0x0000145Fu, 0x306DC9C8u, 0x82A53F84u}, /* 133 */
        {0x000028BEu, 0x60DB9391u, 0x054A7F09u}, /* 134 */
        {0x0000517Cu, 0xC1B72722u, 0x0A94FE13u}, /* 135 */
        {0x0000A2F9u, 0x836E4E44u, 0x1529FC27u}, /* 136 */
        {0x000145F3u, 0x06DC9C88u, 0x2A53F84Eu}, /* 137 */
        {0x00028BE6u, 0x0DB93910u, 0x54A7F09Du}, /* 138 */
        {0x000517CCu, 0x1B727220u, 0xA94FE13Au}, /* 139 */
        {0x000A2F98u, 0x36E4E441u, 0x529FC275u}, /* 140 */
        {0x00145F30u, 0x6DC9C882u, 0xA53F84EAu}, /* 141 */
        {0x0028BE60u, 0xDB939105u, 0x4A7F09D5u}, /* 142 */
        {0x00517CC1u, 0xB727220Au, 0x94FE13ABu}, /* 143 */
        {0x00A2F983u, 0x6E4E4415u, 0x29FC2757u}, /* 144 */
        {0x0145F306u, 0xDC9C882Au, 0x53F84EAFu}, /* 145 */
        {0x028BE60Du, 0xB9391054u, 0xA7F09D5Fu}, /* 146 */
        {0x0517CC1Bu, 0x727220A9u, 0x4FE13ABEu}, /* 147 */
        {0x0A2F9836u, 0xE4E44152u, 0x9FC2757Du}, /* 148 */
        {0x145F306Du, 0xC9C882A5u, 0x3F84EAFAu}, /* 149 */
        {0x28BE60DBu, 0x9391054Au, 0x7F09D5F4u}, /* 150 */
        {0x517CC1B7u, 0x27220A94u, 0xFE13ABE8u}, /* 151 */
        {0xA2F9836Eu, 0x4E441529u, 0xFC2757D1u}, /* 152 */
        {0x45F306DCu, 0x9C882A53u, 0xF84EAFA3u}, /* 153 */
        {0x8BE60DB9u, 0x391054A7u, 0xF09D5F47u}, /* 154 */
        {0x17CC1B72u, 0x7220A94Fu, 0xE13ABE8Fu}, /* 155 */
        {0x2F9836E4u, 0xE441529Fu, 0xC2757D1Fu}, /* 156 */
        {0x5F306DC9u, 0xC882A53Fu, 0x84EAFA3Eu}, /* 157 */
        {0xBE60DB93u, 0x91054A7Fu, 0x09D5F47Du}, /* 158 */
        {0x7CC1B727u, 0x220A94FEu, 0x13ABE8FAu}, /* 159 */
        {0xF9836E4Eu, 0x441529FCu, 0x2757D1F5u}, /* 160 */
        {0xF306DC9Cu, 0x882A53F8u, 0x4EAFA3EAu}, /* 161 */
        {0xE60DB939u, 0x1054A7F0u, 0x9D5F47D4u}, /* 162 */
        {0xCC1B7272u, 0x20A94FE1u, 0x3ABE8FA9u}, /* 163 */
        {0x9836E4E4u, 0x41529FC2u, 0x757D1F53u}, /* 164 */
        {0x306DC9C8u, 0x82A53F84u, 0xEAFA3EA6u}, /* 165 */
        {0x60DB9391u, 0x054A7F09u, 0xD5F47D4Du}, /* 166 */
        {0xC1B72722u, 0x0A94FE13u, 0xABE8FA9Au}, /* 167 */
        {0x836E4E44u, 0x1529FC27u, 0x57D1F534u}, /* 168 */
        {0x06DC9C88u, 0x2A53F84Eu, 0xAFA3EA69u}, /* 169 */
        {0x0DB93910u, 0x54A7F09Du, 0x5F47D4D3u}, /* 170 */
        {0x1B727220u, 0xA94FE13Au, 0xBE8FA9A6u}, /* 171 */
        {0x36E4E441u, 0x529FC275u, 0x7D1F534Du}, /* 172 */
        {0x6DC9C882u, 0xA53F84EAu, 0xFA3EA69Bu}, /* 173 */
        {0xDB939105u, 0x4A7F09D5u, 0xF47D4D37u}, /* 174 */
        {0xB727220Au, 0x94FE13ABu, 0xE8FA9A6Eu}, /* 175 */
        {0x6E4E4415u, 0x29FC2757u, 0xD1F534DDu}, /* 176 */
        {0xDC9C882Au, 0x53F84EAFu, 0xA3EA69BBu}, /* 177 */
        {0xB9391054u, 0xA7F09D5Fu, 0x47D4D377u}, /* 178 */
        {0x727220A9u, 0x4FE13ABEu, 0x8FA9A6EEu}, /* 179 */
        {0xE4E44152u, 0x9FC2757Du, 0x1F534DDCu}, /* 180 */
        {0xC9C882A5u, 0x3F84EAFAu, 0x3EA69BB8u}, /* 181 */
        {0x9391054Au, 0x7F09D5F4u, 0x7D4D3770u}, /* 182 */
        {0x27220A94u, 0xFE13ABE8u, 0xFA9A6EE0u}, /* 183 */
        {0x4E441529u, 0xFC2757D1u, 0xF534DDC0u}, /* 184 */
        {0x9C882A53u, 0xF84EAFA3u, 0xEA69BB81u}, /* 185 */
        {0x391054A7u, 0xF09D5F47u, 0xD4D37703u}, /* 186 */
        {0x7220A94Fu, 0xE13ABE8Fu, 0xA9A6EE06u}, /* 187 */
        {0xE441529Fu, 0xC2757D1Fu, 0x534DDC0Du}, /* 188 */
        {0xC882A53Fu, 0x84EAFA3Eu, 0xA69BB81Bu}, /* 189 */
        {0x91054A7Fu, 0x09D5F47Du, 0x4D377036u}, /* 190 */
        {0x220A94FEu, 0x13ABE8FAu, 0x9A6EE06Du}, /* 191 */
        {0x441529FCu, 0x2757D1F5u, 0x34DDC0DBu}, /* 192 */
        {0x882A53F8u, 0x4EAFA3EAu, 0x69BB81B6u}, /* 193 */
        {0x1054A7F0u, 0x9D5F47D4u, 0xD377036Du}, /* 194 */
        {0x20A94FE1u, 0x3ABE8FA9u, 0xA6EE06DBu}, /* 195 */
        {0x41529FC2u, 0x757D1F53u, 0x4DDC0DB6u}, /* 196 */
        {0x82A53F84u, 0xEAFA3EA6u, 0x9BB81B6Cu}, /* 197 */
        {0x054A7F09u, 0xD5F47D4Du, 0x377036D8u}, /* 198 */
        {0x0A94FE13u, 0xABE8FA9Au, 0x6EE06DB1u}, /* 199 */
        {0x1529FC27u, 0x57D1F534u, 0xDDC0DB62u}, /* 200 */
        {0x2A53F84Eu, 0xAFA3EA69u, 0xBB81B6C5u}, /* 201 */
        {0x54A7F09Du, 0x5F47D4D3u, 0x77036D8Au}, /* 202 */
        {0xA94FE13Au, 0xBE8FA9A6u, 0xEE06DB14u}, /* 203 */
        {0x529FC275u, 0x7D1F534Du, 0xDC0DB629u}, /* 204 */
        {0xA53F84EAu, 0xFA3EA69Bu, 0xB81B6C52u}, /* 205 */
        {0x4A7F09D5u, 0xF47D4D37u, 0x7036D8A5u}, /* 206 */
        {0x94FE13ABu, 0xE8FA9A6Eu, 0xE06DB14Au}, /* 207 */
        {0x29FC2757u, 0xD1F534DDu, 0xC0DB6295u}, /* 208 */
        {0x53F84EAFu, 0xA3EA69BBu, 0x81B6C52Bu}, /* 209 */
        {0xA7F09D5Fu, 0x47D4D377u, 0x036D8A56u}, /* 210 */
        {0x4FE13ABEu, 0x8FA9A6EEu, 0x06DB14ACu}, /* 211 */
        {0x9FC2757Du, 0x1F534DDCu, 0x0DB62959u}, /* 212 */
        {0x3F84EAFAu, 0x3EA69BB8u, 0x1B6C52B3u}, /* 213 */
        {0x7F09D5F4u, 0x7D4D3770u, 0x36D8A566u}, /* 214 */
        {0xFE13ABE8u, 0xFA9A6EE0u, 0x6DB14ACCu}, /* 215 */
        {0xFC2757D1u, 0xF534DDC0u, 0xDB629599u}, /* 216 */
        {0xF84EAFA3u, 0xEA69BB81u, 0xB6C52B32u}, /* 217 */
        {0xF09D5F47u, 0xD4D37703u, 0x6D8A5664u}, /* 218 */
        {0xE13ABE8Fu, 0xA9A6EE06u, 0xDB14ACC9u}, /* 219 */
        {0xC2757D1Fu, 0x534DDC0Du, 0xB6295993u}, /* 220 */
        {0x84EAFA3Eu, 0xA69BB81Bu, 0x6C52B327u}, /* 221 */
        {0x09D5F47Du, 0x4D377036u, 0xD8A5664Fu}, /* 222 */
        {0x13ABE8FAu, 0x9A6EE06Du, 0xB14ACC9Eu}, /* 223 */
        {0x2757D1F5u, 0x34DDC0DBu, 0x6295993Cu}, /* 224 */
        {0x4EAFA3EAu, 0x69BB81B6u, 0xC52B3278u}, /* 225 */
        {0x9D5F47D4u, 0xD377036Du, 0x8A5664F1u}, /* 226 */
        {0x3ABE8FA9u, 0xA6EE06DBu, 0x14ACC9E2u}, /* 227 */
        {0x757D1F53u, 0x4DDC0DB6u, 0x295993C4u}, /* 228 */
        {0xEAFA3EA6u, 0x9BB81B6Cu, 0x52B32788u}, /* 229 */
        {0xD5F47D4Du, 0x377036D8u, 0xA5664F10u}, /* 230 */
        {0xABE8FA9Au, 0x6EE06DB1u, 0x4ACC9E21u}, /* 231 */
        {0x57D1F534u, 0xDDC0DB62u, 0x95993C43u}, /* 232 */
        {0xAFA3EA69u, 0xBB81B6C5u, 0x2B327887u}, /* 233 */
        {0x5F47D4D3u, 0x77036D8Au, 0x5664F10Eu}, /* 234 */
        {0xBE8FA9A6u, 0xEE06DB14u, 0xACC9E21Cu}, /* 235 */
        {0x7D1F534Du, 0xDC0DB629u, 0x5993C439u}, /* 236 */
        {0xFA3EA69Bu, 0xB81B6C52u, 0xB3278872u}, /* 237 */
        {0xF47D4D37u, 0x7036D8A5u, 0x664F10E4u}, /* 238 */
        {0xE8FA9A6Eu, 0xE06DB14Au, 0xCC9E21C8u}, /* 239 */
        {0xD1F534DDu, 0xC0DB6295u, 0x993C4390u}, /* 240 */
        {0xA3EA69BBu, 0x81B6C52Bu, 0x32788720u}, /* 241 */
        {0x47D4D377u, 0x036D8A56u, 0x64F10E41u}, /* 242 */
        {0x8FA9A6EEu, 0x06DB14ACu, 0xC9E21C82u}, /* 243 */
        {0x1F534DDCu, 0x0DB62959u, 0x93C43904u}, /* 244 */
        {0x3EA69BB8u, 0x1B6C52B3u, 0x27887208u}, /* 245 */
        {0x7D4D3770u, 0x36D8A566u, 0x4F10E410u}, /* 246 */
        {0xFA9A6EE0u, 0x6DB14ACCu, 0x9E21C820u}, /* 247 */
        {0xF534DDC0u, 0xDB629599u, 0x3C439041u}, /* 248 */
        {0xEA69BB81u, 0xB6C52B32u, 0x78872083u}, /* 249 */
        {0xD4D37703u, 0x6D8A5664u, 0xF10E4107u}, /* 250 */
        {0xA9A6EE06u, 0xDB14ACC9u, 0xE21C820Fu}, /* 251 */
        {0x534DDC0Du, 0xB6295993u, 0xC439041Fu}, /* 252 */
        {0xA69BB81Bu, 0x6C52B327u, 0x8872083Fu}, /* 253 */
        {0x4D377036u, 0xD8A5664Fu, 0x10E4107Fu}, /* 254 */
        {0x9A6EE06Du, 0xB14ACC9Eu, 0x21C820FFu}  /* 255 */
    }};                                          /*sReduction_Table*/
                                                 /* Table parameters */
typedef struct {
  VUINT32 _sInvPI_int;
  VUINT32 _sPI1_int;
  VUINT32 _sPI2_int;
  VUINT32 _sPI3_int;
  VUINT32 _sPI2_ha_int;
  VUINT32 _sPI3_ha_int;
  VUINT32 Th_tbl_int[32];
  VUINT32 Tl_tbl_int[32];
  VUINT32 _sPC3_int;
  VUINT32 _sPC5_int;
  VUINT32 _sRangeReductionVal_int;
  VUINT32 _sInvPi;
  VUINT32 _sSignMask;
  VUINT32 _sAbsMask;
  VUINT32 _sRangeVal;
  VUINT32 _sRShifter;
  VUINT32 _sOne;
  VUINT32 _sRangeReductionVal;
  VUINT32 _sPI1;
  VUINT32 _sPI2;
  VUINT32 _sPI3;
  VUINT32 _sPI4;
  VUINT32 _sPI1_FMA;
  VUINT32 _sPI2_FMA;
  VUINT32 _sPI3_FMA;
  VUINT32 _sP0;
  VUINT32 _sP1;
  VUINT32 _sQ0;
  VUINT32 _sQ1;
  VUINT32 _sQ2;
  VUINT32 _sTwo;
  VUINT32 _sCoeffs[128][10];
} __devicelib_imf_internal_stan_data_t;
static const __devicelib_imf_internal_stan_data_t
    __devicelib_imf_internal_stan_data = {
        0x4122f983u, /* _sInvPI_int */
        0x3dc90fdau, /* _sPI1_int */
        0x31a22168u, /* _sPI2_int */
        0x25c234c5u, /* _sPI3_int */
        0x31a22000u, /* _sPI2_ha_int */
        0x2a34611au, /* _sPI3_ha_int */
        /* Th_tbl_int for i from 0 to 31 do printsingle(tan(i*Pi/32)); */
        {
            0x80000000u, 0x3dc9b5dcu, 0x3e4bafafu, 0x3e9b5042u, 0x3ed413cdu,
            0x3f08d5b9u, 0x3f2b0dc1u, 0x3f521801u, 0x3f800000u, 0x3f9bf7ecu,
            0x3fbf90c7u, 0x3fef789eu, 0x401a827au, 0x4052facfu, 0x40a0dff7u,
            0x41227363u, 0xff7fffffu, 0xc1227363u, 0xc0a0dff7u, 0xc052facfu,
            0xc01a827au, 0xbfef789eu, 0xbfbf90c7u, 0xbf9bf7ecu, 0xbf800000u,
            0xbf521801u, 0xbf2b0dc1u, 0xbf08d5b9u, 0xbed413cdu, 0xbe9b5042u,
            0xbe4bafafu, 0xbdc9b5dcu,
        }, /* Th_tbl_int */
/*
//          Tl_tbl_int for i from 0 to 31 do
//            printsingle(tan(i*Pi/32)-round(tan(i*Pi/32),SG,RN)); 
*/
        {
            0x80000000u, 0x3145b2dau, 0x2f2a62b0u, 0xb22a39c2u, 0xb1c0621au,
            0xb25ef963u, 0x32ab7f99u, 0x32ae4285u, 0x00000000u, 0x33587608u,
            0x32169d18u, 0xb30c3ec0u, 0xb3cc0622u, 0x3390600eu, 0x331091dcu,
            0xb454a046u, 0xf3800000u, 0x3454a046u, 0xb31091dcu, 0xb390600eu,
            0x33cc0622u, 0x330c3ec0u, 0xb2169d18u, 0xb3587608u, 0x00000000u,
            0xb2ae4285u, 0xb2ab7f99u, 0x325ef963u, 0x31c0621au, 0x322a39c2u,
            0xaf2a62b0u, 0xb145b2dau,
        },           /* Tl_tbl_int */
        0x3eaaaaa6u, /* _sPC3_int */
        0x3e08b888u, /* _sPC5_int */
        0x46010000u, /* _sRangeReductionVal_int */
        0x3F22F983u, /* _sInvPi */
        0x80000000u, /* _sSignMask */
        0x7FFFFFFFu, /* _sAbsMask  */
        0x7f800000u, /* _sRangeVal  */
        0x4B400000u, /* _sRShifter  */
        0x3f800000u, /* _sOne */
        0x46010000u, /* _sRangeVal */
        0x3FC90000u, /* _sPI1  */
        0x39FDA000u, /* _sPI2  */
        0x33A22000u, /* _sPI3  */
        0x2C34611Au, /* _sPI4  */
        // PI1, PI2, and PI3 when FMA is available
        0x3FC90FDBu, /* _sPI1_FMA  */
        0xB33BBD2Eu, /* _sPI2_FMA  */
        0xA6F72CEDu, /* _sPI3_FMA  */
        0x3F7FFFFCu, /* _sP0 */
        0xBDC433B4u, /* _sP1 */
        0x3F7FFFFCu, /* _sQ0 */
        0xBEDBB7ABu, /* _sQ1 */
        0x3C1F336Bu, /* _sQ2 */
        0x40000000u, /* _sTwo */
        {
            // _sCoeffs Breakpoint B = 0 * pi/128, function tan(B + x)
            {
                0x3FC90FDBu, // B' = pi/2 - B (high single)
                0xB33BBD2Eu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x00000000u, // c0 (high single)
                0x00000000u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x00000000u, // c1 (low single)
                0x00000000u, // c2
                0x3EAAACDDu, // c3
                0x00000000u  // c4
            },
            // Breakpoint B = 1 * pi/128, function tan(B + x)
            {
                0x3FC5EB9Bu, // B' = pi/2 - B (high single)
                0x32DE638Cu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3CC91A31u, // c0 (high single)
                0x2F8E8D1Au, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3A1DFA00u, // c1 (low single)
                0x3CC9392Du, // c2
                0x3EAB1889u, // c3
                0x3C885D3Bu  // c4
            },
            // Breakpoint B = 2 * pi/128, function tan(B + x)
            {
                0x3FC2C75Cu, // B' = pi/2 - B (high single)
                0xB2CBBE8Au, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3D49393Cu, // c0 (high single)
                0x30A39F5Bu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3B1E2B00u, // c1 (low single)
                0x3D49B5D4u, // c2
                0x3EAC4F10u, // c3
                0x3CFD9425u  // c4
            },
            // Breakpoint B = 3 * pi/128, function tan(B + x)
            {
                0x3FBFA31Cu, // B' = pi/2 - B (high single)
                0x33450FB0u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3D9711CEu, // c0 (high single)
                0x314FEB28u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3BB24C00u, // c1 (low single)
                0x3D97E43Au, // c2
                0x3EAE6A89u, // c3
                0x3D4D07E0u  // c4
            },
            // Breakpoint B = 4 * pi/128, function tan(B + x)
            {
                0x3FBC7EDDu, // B' = pi/2 - B (high single)
                0xB1800ADDu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3DC9B5DCu, // c0 (high single)
                0x3145AD86u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3C1EEF20u, // c1 (low single)
                0x3DCBAAEAu, // c2
                0x3EB14E5Eu, // c3
                0x3D858BB2u  // c4
            },
            // Breakpoint B = 5 * pi/128, function tan(B + x)
            {
                0x3FB95A9Eu, // B' = pi/2 - B (high single)
                0xB3651267u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3DFC98C2u, // c0 (high single)
                0xB0AE525Cu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3C793D20u, // c1 (low single)
                0x3E003845u, // c2
                0x3EB5271Fu, // c3
                0x3DAC669Eu  // c4
            },
            // Breakpoint B = 6 * pi/128, function tan(B + x)
            {
                0x3FB6365Eu, // B' = pi/2 - B (high single)
                0x328BB91Cu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3E17E564u, // c0 (high single)
                0xB1C5A2E4u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3CB440D0u, // c1 (low single)
                0x3E1B3D00u, // c2
                0x3EB9F664u, // c3
                0x3DD647C0u  // c4
            },
            // Breakpoint B = 7 * pi/128, function tan(B + x)
            {
                0x3FB3121Fu, // B' = pi/2 - B (high single)
                0xB30F347Du, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3E31AE4Du, // c0 (high single)
                0xB1F32251u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3CF6A500u, // c1 (low single)
                0x3E3707DAu, // c2
                0x3EBFA489u, // c3
                0x3DFBD9C7u  // c4
            },
            // Breakpoint B = 8 * pi/128, function tan(B + x)
            {
                0x3FAFEDDFu, // B' = pi/2 - B (high single)
                0x331BBA77u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3E4BAFAFu, // c0 (high single)
                0x2F2A29E0u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3D221018u, // c1 (low single)
                0x3E53BED0u, // c2
                0x3EC67E26u, // c3
                0x3E1568E2u  // c4
            },
            // Breakpoint B = 9 * pi/128, function tan(B + x)
            {
                0x3FACC9A0u, // B' = pi/2 - B (high single)
                0xB2655A50u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3E65F267u, // c0 (high single)
                0x31B4B1DFu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3D4E8B90u, // c1 (low single)
                0x3E718ACAu, // c2
                0x3ECE7164u, // c3
                0x3E2DC161u  // c4
            },
            // Breakpoint B = 10 * pi/128, function tan(B + x)
            {
                0x3FA9A560u, // B' = pi/2 - B (high single)
                0x33719861u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3E803FD4u, // c0 (high single)
                0xB2279E66u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3D807FC8u, // c1 (low single)
                0x3E884BD4u, // c2
                0x3ED7812Du, // c3
                0x3E4636EBu  // c4
            },
            // Breakpoint B = 11 * pi/128, function tan(B + x)
            {
                0x3FA68121u, // B' = pi/2 - B (high single)
                0x31E43AACu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3E8DB082u, // c0 (high single)
                0xB132A234u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3D9CD7D0u, // c1 (low single)
                0x3E988A60u, // c2
                0x3EE203E3u, // c3
                0x3E63582Cu  // c4
            },
            // Breakpoint B = 12 * pi/128, function tan(B + x)
            {
                0x3FA35CE2u, // B' = pi/2 - B (high single)
                0xB33889B6u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3E9B5042u, // c0 (high single)
                0xB22A3AEEu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3DBC7490u, // c1 (low single)
                0x3EA99AF5u, // c2
                0x3EEDE107u, // c3
                0x3E80E9AAu  // c4
            },
            // Breakpoint B = 13 * pi/128, function tan(B + x)
            {
                0x3FA038A2u, // B' = pi/2 - B (high single)
                0x32E4CA7Eu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3EA92457u, // c0 (high single)
                0x30B80830u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3DDF8200u, // c1 (low single)
                0x3EBB99E9u, // c2
                0x3EFB4AA8u, // c3
                0x3E9182BEu  // c4
            },
            // Breakpoint B = 14 * pi/128, function tan(B + x)
            {
                0x3F9D1463u, // B' = pi/2 - B (high single)
                0xB2C55799u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3EB73250u, // c0 (high single)
                0xB2028823u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E0318F8u, // c1 (low single)
                0x3ECEA678u, // c2
                0x3F053C67u, // c3
                0x3EA41E53u  // c4
            },
            // Breakpoint B = 15 * pi/128, function tan(B + x)
            {
                0x3F99F023u, // B' = pi/2 - B (high single)
                0x33484328u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3EC5800Du, // c0 (high single)
                0xB214C3C1u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E185E54u, // c1 (low single)
                0x3EE2E342u, // c2
                0x3F0DCA73u, // c3
                0x3EB8CC21u  // c4
            },
            // Breakpoint B = 16 * pi/128, function tan(B + x)
            {
                0x3F96CBE4u, // B' = pi/2 - B (high single)
                0xB14CDE2Eu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3ED413CDu, // c0 (high single)
                0xB1C06152u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E2FB0CCu, // c1 (low single)
                0x3EF876CBu, // c2
                0x3F177807u, // c3
                0x3ED08437u  // c4
            },
            // Breakpoint B = 17 * pi/128, function tan(B + x)
            {
                0x3F93A7A5u, // B' = pi/2 - B (high single)
                0xB361DEEEu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3EE2F439u, // c0 (high single)
                0xB1F4399Eu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E49341Cu, // c1 (low single)
                0x3F07C61Au, // c2
                0x3F22560Fu, // c3
                0x3EEAA81Eu  // c4
            },
            // Breakpoint B = 18 * pi/128, function tan(B + x)
            {
                0x3F908365u, // B' = pi/2 - B (high single)
                0x3292200Du, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3EF22870u, // c0 (high single)
                0x325271F4u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E65107Au, // c1 (low single)
                0x3F1429F0u, // c2
                0x3F2E8AFCu, // c3
                0x3F040498u  // c4
            },
            // Breakpoint B = 19 * pi/128, function tan(B + x)
            {
                0x3F8D5F26u, // B' = pi/2 - B (high single)
                0xB30C0105u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F00DC0Du, // c0 (high single)
                0xB214AF72u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E81B994u, // c1 (low single)
                0x3F218233u, // c2
                0x3F3C4531u, // c3
                0x3F149688u  // c4
            },
            // Breakpoint B = 20 * pi/128, function tan(B + x)
            {
                0x3F8A3AE6u, // B' = pi/2 - B (high single)
                0x331EEDF0u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F08D5B9u, // c0 (high single)
                0xB25EF98Eu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E92478Du, // c1 (low single)
                0x3F2FEDC9u, // c2
                0x3F4BCD58u, // c3
                0x3F27AE9Eu  // c4
            },
            // Breakpoint B = 21 * pi/128, function tan(B + x)
            {
                0x3F8716A7u, // B' = pi/2 - B (high single)
                0xB2588C6Du, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F1105AFu, // c0 (high single)
                0x32F045B0u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3EA44EE2u, // c1 (low single)
                0x3F3F8FDBu, // c2
                0x3F5D3FD0u, // c3
                0x3F3D0A23u  // c4
            },
            // Breakpoint B = 22 * pi/128, function tan(B + x)
            {
                0x3F83F267u, // B' = pi/2 - B (high single)
                0x3374CBD9u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F1970C4u, // c0 (high single)
                0x32904848u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3EB7EFF8u, // c1 (low single)
                0x3F50907Cu, // c2
                0x3F710FEAu, // c3
                0x3F561FEDu  // c4
            },
            // Breakpoint B = 23 * pi/128, function tan(B + x)
            {
                0x3F80CE28u, // B' = pi/2 - B (high single)
                0x31FDD672u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F221C37u, // c0 (high single)
                0xB20C61DCu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3ECD4F71u, // c1 (low single)
                0x3F631DAAu, // c2
                0x3F83B471u, // c3
                0x3F7281EAu  // c4
            },
            // Breakpoint B = 24 * pi/128, function tan(B + x)
            {
                0x3F7B53D1u, // B' = pi/2 - B (high single)
                0x32955386u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F2B0DC1u, // c0 (high single)
                0x32AB7EBAu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3EE496C2u, // c1 (low single)
                0x3F776C40u, // c2
                0x3F9065C1u, // c3
                0x3F89AFB6u  // c4
            },
            // Breakpoint B = 25 * pi/128, function tan(B + x)
            {
                0x3F750B52u, // B' = pi/2 - B (high single)
                0x32EB316Fu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F344BA9u, // c0 (high single)
                0xB2B8B0EAu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3EFDF4F7u, // c1 (low single)
                0x3F86DCA8u, // c2
                0x3F9ED53Bu, // c3
                0x3F9CBEDEu  // c4
            },
            // Breakpoint B = 26 * pi/128, function tan(B + x)
            {
                0x3F6EC2D4u, // B' = pi/2 - B (high single)
                0xB2BEF0A7u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F3DDCCFu, // c0 (high single)
                0x32D29606u, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xBEE6606Fu, // c1 (low single)
                0x3F9325D6u, // c2
                0x3FAF4E69u, // c3
                0x3FB3080Cu  // c4
            },
            // Breakpoint B = 27 * pi/128, function tan(B + x)
            {
                0x3F687A55u, // B' = pi/2 - B (high single)
                0xB252257Bu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F47C8CCu, // c0 (high single)
                0xB200F51Au, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xBEC82C6Cu, // c1 (low single)
                0x3FA0BAE9u, // c2
                0x3FC2252Fu, // c3
                0x3FCD24C7u  // c4
            },
            // Breakpoint B = 28 * pi/128, function tan(B + x)
            {
                0x3F6231D6u, // B' = pi/2 - B (high single)
                0xB119A6A2u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F521801u, // c0 (high single)
                0x32AE4178u, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xBEA72938u, // c1 (low single)
                0x3FAFCC22u, // c2
                0x3FD7BD4Au, // c3
                0x3FEBB01Bu  // c4
            },
            // Breakpoint B = 29 * pi/128, function tan(B + x)
            {
                0x3F5BE957u, // B' = pi/2 - B (high single)
                0x3205522Au, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F5CD3BEu, // c0 (high single)
                0x31460308u, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xBE8306C5u, // c1 (low single)
                0x3FC09232u, // c2
                0x3FF09632u, // c3
                0x4007DB00u  // c4
            },
            // Breakpoint B = 30 * pi/128, function tan(B + x)
            {
                0x3F55A0D8u, // B' = pi/2 - B (high single)
                0x329886FFu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F68065Eu, // c0 (high single)
                0x32670D1Au, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xBE36D1D6u, // c1 (low single)
                0x3FD35007u, // c2
                0x4006A861u, // c3
                0x401D4BDAu  // c4
            },
            // Breakpoint B = 31 * pi/128, function tan(B + x)
            {
                0x3F4F5859u, // B' = pi/2 - B (high single)
                0x32EE64E8u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0x3F73BB75u, // c0 (high single)
                0x32FC908Du, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xBDBF94B0u, // c1 (low single)
                0x3FE8550Fu, // c2
                0x40174F67u, // c3
                0x4036C608u  // c4
            },
            // Breakpoint B = 32 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3F490FDBu, // B' = pi/2 - B (high single)
                0xB2BBBD2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE8BE60Eu, // c0 (high single)
                0x320D8D84u, // c0 (low single)
                0x3F000000u, // c1 (high 1 bit)
                0xBDF817B1u, // c1 (low single)
                0xBD8345EBu, // c2
                0x3D1DFDACu, // c3
                0xBC52CF6Fu  // c4
            },
            // Breakpoint B = 33 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3F42C75Cu, // B' = pi/2 - B (high single)
                0xB24BBE8Au, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE87283Fu, // c0 (high single)
                0xB268B966u, // c0 (low single)
                0x3F000000u, // c1 (high 1 bit)
                0xBDFE6529u, // c1 (low single)
                0xBD7B1953u, // c2
                0x3D18E109u, // c3
                0xBC4570B0u  // c4
            },
            // Breakpoint B = 34 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3F3C7EDDu, // B' = pi/2 - B (high single)
                0xB1000ADDu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE827420u, // c0 (high single)
                0x320B8B4Du, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DFB9428u, // c1 (low single)
                0xBD7002B4u, // c2
                0x3D142A6Cu, // c3
                0xBC3A47FFu  // c4
            },
            // Breakpoint B = 35 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3F36365Eu, // B' = pi/2 - B (high single)
                0x320BB91Cu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE7B9282u, // c0 (high single)
                0xB13383D2u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DF5D211u, // c1 (low single)
                0xBD6542B3u, // c2
                0x3D0FE5E5u, // c3
                0xBC31FB14u  // c4
            },
            // Breakpoint B = 36 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3F2FEDDFu, // B' = pi/2 - B (high single)
                0x329BBA77u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE724E73u, // c0 (high single)
                0x3120C3E2u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DF05283u, // c1 (low single)
                0xBD5AD45Eu, // c2
                0x3D0BAFBFu, // c3
                0xBC27B8BBu  // c4
            },
            // Breakpoint B = 37 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3F29A560u, // B' = pi/2 - B (high single)
                0x32F19861u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE691B44u, // c0 (high single)
                0x31F18936u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DEB138Bu, // c1 (low single)
                0xBD50B2F7u, // c2
                0x3D07BE3Au, // c3
                0xBC1E46A7u  // c4
            },
            // Breakpoint B = 38 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3F235CE2u, // B' = pi/2 - B (high single)
                0xB2B889B6u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE5FF82Cu, // c0 (high single)
                0xB170723Au, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DE61354u, // c1 (low single)
                0xBD46DA06u, // c2
                0x3D0401F8u, // c3
                0xBC14E013u  // c4
            },
            // Breakpoint B = 39 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3F1D1463u, // B' = pi/2 - B (high single)
                0xB2455799u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE56E46Bu, // c0 (high single)
                0x31E3F001u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DE15025u, // c1 (low single)
                0xBD3D4550u, // c2
                0x3D00462Du, // c3
                0xBC092C98u  // c4
            },
            // Breakpoint B = 40 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3F16CBE4u, // B' = pi/2 - B (high single)
                0xB0CCDE2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE4DDF41u, // c0 (high single)
                0xB1AEA094u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DDCC85Cu, // c1 (low single)
                0xBD33F0BEu, // c2
                0x3CFA23B0u, // c3
                0xBC01FCF7u  // c4
            },
            // Breakpoint B = 41 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3F108365u, // B' = pi/2 - B (high single)
                0x3212200Du, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE44E7F8u, // c0 (high single)
                0xB1CAA3CBu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DD87A74u, // c1 (low single)
                0xBD2AD885u, // c2
                0x3CF3C785u, // c3
                0xBBF1E348u  // c4
            },
            // Breakpoint B = 42 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3F0A3AE6u, // B' = pi/2 - B (high single)
                0x329EEDF0u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE3BFDDCu, // c0 (high single)
                0xB132521Au, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DD464FCu, // c1 (low single)
                0xBD21F8F1u, // c2
                0x3CEE3076u, // c3
                0xBBE6D263u  // c4
            },
            // Breakpoint B = 43 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3F03F267u, // B' = pi/2 - B (high single)
                0x32F4CBD9u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE33203Eu, // c0 (high single)
                0x31FEF5BEu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DD0869Cu, // c1 (low single)
                0xBD194E8Cu, // c2
                0x3CE8DCA9u, // c3
                0xBBDADA55u  // c4
            },
            // Breakpoint B = 44 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3EFB53D1u, // B' = pi/2 - B (high single)
                0x32155386u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE2A4E71u, // c0 (high single)
                0xB19CFCECu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DCCDE11u, // c1 (low single)
                0xBD10D605u, // c2
                0x3CE382A7u, // c3
                0xBBC8BD97u  // c4
            },
            // Breakpoint B = 45 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3EEEC2D4u, // B' = pi/2 - B (high single)
                0xB23EF0A7u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE2187D0u, // c0 (high single)
                0xB1B7C7F7u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DC96A2Bu, // c1 (low single)
                0xBD088C22u, // c2
                0x3CDE950Eu, // c3
                0xBBB89AD1u  // c4
            },
            // Breakpoint B = 46 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3EE231D6u, // B' = pi/2 - B (high single)
                0xB099A6A2u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE18CBB7u, // c0 (high single)
                0xAFE28430u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DC629CEu, // c1 (low single)
                0xBD006DCDu, // c2
                0x3CDA5A2Cu, // c3
                0xBBB0B3D2u  // c4
            },
            // Breakpoint B = 47 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3ED5A0D8u, // B' = pi/2 - B (high single)
                0x321886FFu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE101985u, // c0 (high single)
                0xB02FB2B8u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DC31BF3u, // c1 (low single)
                0xBCF0F04Du, // c2
                0x3CD60BC7u, // c3
                0xBBA138BAu  // c4
            },
            // Breakpoint B = 48 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3EC90FDBu, // B' = pi/2 - B (high single)
                0xB23BBD2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBE07709Du, // c0 (high single)
                0xB18A2A83u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DC03FA2u, // c1 (low single)
                0xBCE15096u, // c2
                0x3CD26472u, // c3
                0xBB9A1270u  // c4
            },
            // Breakpoint B = 49 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3EBC7EDDu, // B' = pi/2 - B (high single)
                0xB0800ADDu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBDFDA0CBu, // c0 (high single)
                0x2F14FCA0u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DBD93F7u, // c1 (low single)
                0xBCD1F71Bu, // c2
                0x3CCEDD2Bu, // c3
                0xBB905946u  // c4
            },
            // Breakpoint B = 50 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3EAFEDDFu, // B' = pi/2 - B (high single)
                0x321BBA77u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBDEC708Cu, // c0 (high single)
                0xB14895C4u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DBB181Eu, // c1 (low single)
                0xBCC2DEA6u, // c2
                0x3CCB5027u, // c3
                0xBB7F3969u  // c4
            },
            // Breakpoint B = 51 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3EA35CE2u, // B' = pi/2 - B (high single)
                0xB23889B6u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBDDB4F55u, // c0 (high single)
                0x30F6437Eu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DB8CB52u, // c1 (low single)
                0xBCB40210u, // c2
                0x3CC82D45u, // c3
                0xBB643075u  // c4
            },
            // Breakpoint B = 52 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3E96CBE4u, // B' = pi/2 - B (high single)
                0xB04CDE2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBDCA3BFFu, // c0 (high single)
                0x311C95EAu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DB6ACDEu, // c1 (low single)
                0xBCA55C5Bu, // c2
                0x3CC5BC04u, // c3
                0xBB63A969u  // c4
            },
            // Breakpoint B = 53 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3E8A3AE6u, // B' = pi/2 - B (high single)
                0x321EEDF0u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBDB93569u, // c0 (high single)
                0xAFB9ED00u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DB4BC1Fu, // c1 (low single)
                0xBC96E905u, // c2
                0x3CC2E6F5u, // c3
                0xBB3E10A6u  // c4
            },
            // Breakpoint B = 54 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3E7B53D1u, // B' = pi/2 - B (high single)
                0x31955386u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBDA83A77u, // c0 (high single)
                0x316D967Au, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DB2F87Cu, // c1 (low single)
                0xBC88A31Fu, // c2
                0x3CC0E763u, // c3
                0xBB3F1666u  // c4
            },
            // Breakpoint B = 55 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3E6231D6u, // B' = pi/2 - B (high single)
                0xB019A6A2u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBD974A0Du, // c0 (high single)
                0xB14F365Bu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DB1616Fu, // c1 (low single)
                0xBC750CD8u, // c2
                0x3CBEB595u, // c3
                0xBB22B883u  // c4
            },
            // Breakpoint B = 56 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3E490FDBu, // B' = pi/2 - B (high single)
                0xB1BBBD2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBD866317u, // c0 (high single)
                0xAFF02140u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DAFF67Du, // c1 (low single)
                0xBC591CD0u, // c2
                0x3CBCBEADu, // c3
                0xBB04BBECu  // c4
            },
            // Breakpoint B = 57 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3E2FEDDFu, // B' = pi/2 - B (high single)
                0x319BBA77u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBD6B08FFu, // c0 (high single)
                0xB0EED236u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DAEB739u, // c1 (low single)
                0xBC3D6D51u, // c2
                0x3CBB485Du, // c3
                0xBAFFF5BAu  // c4
            },
            // Breakpoint B = 58 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3E16CBE4u, // B' = pi/2 - B (high single)
                0xAFCCDE2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBD495A6Cu, // c0 (high single)
                0xB0A427BDu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DADA345u, // c1 (low single)
                0xBC21F648u, // c2
                0x3CB9D1B4u, // c3
                0xBACB5567u  // c4
            },
            // Breakpoint B = 59 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3DFB53D1u, // B' = pi/2 - B (high single)
                0x31155386u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBD27B856u, // c0 (high single)
                0xB0F7EE91u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DACBA4Eu, // c1 (low single)
                0xBC06AEE3u, // c2
                0x3CB8E5DCu, // c3
                0xBAEC00EEu  // c4
            },
            // Breakpoint B = 60 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3DC90FDBu, // B' = pi/2 - B (high single)
                0xB13BBD2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBD0620A3u, // c0 (high single)
                0xB0ECAB40u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DABFC11u, // c1 (low single)
                0xBBD7200Fu, // c2
                0x3CB79475u, // c3
                0xBA2B0ADCu  // c4
            },
            // Breakpoint B = 61 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3D96CBE4u, // B' = pi/2 - B (high single)
                0xAF4CDE2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBCC92278u, // c0 (high single)
                0x302F2E68u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DAB6854u, // c1 (low single)
                0xBBA1214Fu, // c2
                0x3CB6C1E9u, // c3
                0x3843C2F3u  // c4
            },
            // Breakpoint B = 62 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3D490FDBu, // B' = pi/2 - B (high single)
                0xB0BBBD2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBC861015u, // c0 (high single)
                0xAFD68E2Eu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DAAFEEBu, // c1 (low single)
                0xBB569F3Fu, // c2
                0x3CB6A84Eu, // c3
                0xBAC64194u  // c4
            },
            // Breakpoint B = 63 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x3CC90FDBu, // B' = pi/2 - B (high single)
                0xB03BBD2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0xBC060BF3u, // c0 (high single)
                0x2FE251AEu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DAABFB9u, // c1 (low single)
                0xBAD67C60u, // c2
                0x3CB64CA5u, // c3
                0xBACDE881u  // c4
            },
            // Breakpoint B = 64 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0x00000000u, // B' = pi/2 - B (high single)
                0x00000000u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x00000000u, // c0 (high single)
                0x00000000u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DAAAAABu, // c1 (low single)
                0x00000000u, // c2
                0x3CB5E28Bu, // c3
                0x00000000u  // c4
            },
            // Breakpoint B = 65 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBCC90FDBu, // B' = pi/2 - B (high single)
                0x303BBD2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3C060BF3u, // c0 (high single)
                0xAFE251AEu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DAABFB9u, // c1 (low single)
                0x3AD67C60u, // c2
                0x3CB64CA5u, // c3
                0x3ACDE881u  // c4
            },
            // Breakpoint B = 66 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBD490FDBu, // B' = pi/2 - B (high single)
                0x30BBBD2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3C861015u, // c0 (high single)
                0x2FD68E2Eu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DAAFEEBu, // c1 (low single)
                0x3B569F3Fu, // c2
                0x3CB6A84Eu, // c3
                0x3AC64194u  // c4
            },
            // Breakpoint B = 67 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBD96CBE4u, // B' = pi/2 - B (high single)
                0x2F4CDE2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3CC92278u, // c0 (high single)
                0xB02F2E68u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DAB6854u, // c1 (low single)
                0x3BA1214Fu, // c2
                0x3CB6C1E9u, // c3
                0xB843C2F2u  // c4
            },
            // Breakpoint B = 68 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBDC90FDBu, // B' = pi/2 - B (high single)
                0x313BBD2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3D0620A3u, // c0 (high single)
                0x30ECAB40u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DABFC11u, // c1 (low single)
                0x3BD7200Fu, // c2
                0x3CB79475u, // c3
                0x3A2B0ADCu  // c4
            },
            // Breakpoint B = 69 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBDFB53D1u, // B' = pi/2 - B (high single)
                0xB1155386u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3D27B856u, // c0 (high single)
                0x30F7EE91u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DACBA4Eu, // c1 (low single)
                0x3C06AEE3u, // c2
                0x3CB8E5DCu, // c3
                0x3AEC00EEu  // c4
            },
            // Breakpoint B = 70 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBE16CBE4u, // B' = pi/2 - B (high single)
                0x2FCCDE2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3D495A6Cu, // c0 (high single)
                0x30A427BDu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DADA345u, // c1 (low single)
                0x3C21F648u, // c2
                0x3CB9D1B4u, // c3
                0x3ACB5567u  // c4
            },
            // Breakpoint B = 71 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBE2FEDDFu, // B' = pi/2 - B (high single)
                0xB19BBA77u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3D6B08FFu, // c0 (high single)
                0x30EED236u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DAEB739u, // c1 (low single)
                0x3C3D6D51u, // c2
                0x3CBB485Du, // c3
                0x3AFFF5BAu  // c4
            },
            // Breakpoint B = 72 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBE490FDBu, // B' = pi/2 - B (high single)
                0x31BBBD2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3D866317u, // c0 (high single)
                0x2FF02140u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DAFF67Du, // c1 (low single)
                0x3C591CD0u, // c2
                0x3CBCBEADu, // c3
                0x3B04BBECu  // c4
            },
            // Breakpoint B = 73 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBE6231D6u, // B' = pi/2 - B (high single)
                0x3019A6A2u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3D974A0Du, // c0 (high single)
                0x314F365Bu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DB1616Fu, // c1 (low single)
                0x3C750CD8u, // c2
                0x3CBEB595u, // c3
                0x3B22B883u  // c4
            },
            // Breakpoint B = 74 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBE7B53D1u, // B' = pi/2 - B (high single)
                0xB1955386u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3DA83A77u, // c0 (high single)
                0xB16D967Au, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DB2F87Cu, // c1 (low single)
                0x3C88A31Fu, // c2
                0x3CC0E763u, // c3
                0x3B3F1666u  // c4
            },
            // Breakpoint B = 75 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBE8A3AE6u, // B' = pi/2 - B (high single)
                0xB21EEDF0u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3DB93569u, // c0 (high single)
                0x2FB9ED00u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DB4BC1Fu, // c1 (low single)
                0x3C96E905u, // c2
                0x3CC2E6F5u, // c3
                0x3B3E10A6u  // c4
            },
            // Breakpoint B = 76 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBE96CBE4u, // B' = pi/2 - B (high single)
                0x304CDE2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3DCA3BFFu, // c0 (high single)
                0xB11C95EAu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DB6ACDEu, // c1 (low single)
                0x3CA55C5Bu, // c2
                0x3CC5BC04u, // c3
                0x3B63A969u  // c4
            },
            // Breakpoint B = 77 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBEA35CE2u, // B' = pi/2 - B (high single)
                0x323889B6u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3DDB4F55u, // c0 (high single)
                0xB0F6437Eu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DB8CB52u, // c1 (low single)
                0x3CB40210u, // c2
                0x3CC82D45u, // c3
                0x3B643075u  // c4
            },
            // Breakpoint B = 78 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBEAFEDDFu, // B' = pi/2 - B (high single)
                0xB21BBA77u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3DEC708Cu, // c0 (high single)
                0x314895C4u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DBB181Eu, // c1 (low single)
                0x3CC2DEA6u, // c2
                0x3CCB5027u, // c3
                0x3B7F3969u  // c4
            },
            // Breakpoint B = 79 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBEBC7EDDu, // B' = pi/2 - B (high single)
                0x30800ADDu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3DFDA0CBu, // c0 (high single)
                0xAF14FCA0u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DBD93F7u, // c1 (low single)
                0x3CD1F71Bu, // c2
                0x3CCEDD2Bu, // c3
                0x3B905946u  // c4
            },
            // Breakpoint B = 80 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBEC90FDBu, // B' = pi/2 - B (high single)
                0x323BBD2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E07709Du, // c0 (high single)
                0x318A2A83u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DC03FA2u, // c1 (low single)
                0x3CE15096u, // c2
                0x3CD26472u, // c3
                0x3B9A1270u  // c4
            },
            // Breakpoint B = 81 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBED5A0D8u, // B' = pi/2 - B (high single)
                0xB21886FFu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E101985u, // c0 (high single)
                0x302FB2B8u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DC31BF3u, // c1 (low single)
                0x3CF0F04Du, // c2
                0x3CD60BC7u, // c3
                0x3BA138BAu  // c4
            },
            // Breakpoint B = 82 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBEE231D6u, // B' = pi/2 - B (high single)
                0x3099A6A2u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E18CBB7u, // c0 (high single)
                0x2FE28430u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DC629CEu, // c1 (low single)
                0x3D006DCDu, // c2
                0x3CDA5A2Cu, // c3
                0x3BB0B3D2u  // c4
            },
            // Breakpoint B = 83 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBEEEC2D4u, // B' = pi/2 - B (high single)
                0x323EF0A7u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E2187D0u, // c0 (high single)
                0x31B7C7F7u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DC96A2Bu, // c1 (low single)
                0x3D088C22u, // c2
                0x3CDE950Eu, // c3
                0x3BB89AD1u  // c4
            },
            // Breakpoint B = 84 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBEFB53D1u, // B' = pi/2 - B (high single)
                0xB2155386u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E2A4E71u, // c0 (high single)
                0x319CFCECu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DCCDE11u, // c1 (low single)
                0x3D10D605u, // c2
                0x3CE382A7u, // c3
                0x3BC8BD97u  // c4
            },
            // Breakpoint B = 85 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBF03F267u, // B' = pi/2 - B (high single)
                0xB2F4CBD9u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E33203Eu, // c0 (high single)
                0xB1FEF5BEu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DD0869Cu, // c1 (low single)
                0x3D194E8Cu, // c2
                0x3CE8DCA9u, // c3
                0x3BDADA55u  // c4
            },
            // Breakpoint B = 86 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBF0A3AE6u, // B' = pi/2 - B (high single)
                0xB29EEDF0u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E3BFDDCu, // c0 (high single)
                0x3132521Au, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DD464FCu, // c1 (low single)
                0x3D21F8F1u, // c2
                0x3CEE3076u, // c3
                0x3BE6D263u  // c4
            },
            // Breakpoint B = 87 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBF108365u, // B' = pi/2 - B (high single)
                0xB212200Du, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E44E7F8u, // c0 (high single)
                0x31CAA3CBu, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DD87A74u, // c1 (low single)
                0x3D2AD885u, // c2
                0x3CF3C785u, // c3
                0x3BF1E348u  // c4
            },
            // Breakpoint B = 88 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBF16CBE4u, // B' = pi/2 - B (high single)
                0x30CCDE2Eu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E4DDF41u, // c0 (high single)
                0x31AEA094u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DDCC85Cu, // c1 (low single)
                0x3D33F0BEu, // c2
                0x3CFA23B0u, // c3
                0x3C01FCF7u  // c4
            },
            // Breakpoint B = 89 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBF1D1463u, // B' = pi/2 - B (high single)
                0x32455799u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E56E46Bu, // c0 (high single)
                0xB1E3F001u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DE15025u, // c1 (low single)
                0x3D3D4550u, // c2
                0x3D00462Du, // c3
                0x3C092C98u  // c4
            },
            // Breakpoint B = 90 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBF235CE2u, // B' = pi/2 - B (high single)
                0x32B889B6u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E5FF82Cu, // c0 (high single)
                0x3170723Au, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DE61354u, // c1 (low single)
                0x3D46DA06u, // c2
                0x3D0401F8u, // c3
                0x3C14E013u  // c4
            },
            // Breakpoint B = 91 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBF29A560u, // B' = pi/2 - B (high single)
                0xB2F19861u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E691B44u, // c0 (high single)
                0xB1F18936u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DEB138Bu, // c1 (low single)
                0x3D50B2F7u, // c2
                0x3D07BE3Au, // c3
                0x3C1E46A7u  // c4
            },
            // Breakpoint B = 92 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBF2FEDDFu, // B' = pi/2 - B (high single)
                0xB29BBA77u, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E724E73u, // c0 (high single)
                0xB120C3E2u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DF05283u, // c1 (low single)
                0x3D5AD45Eu, // c2
                0x3D0BAFBFu, // c3
                0x3C27B8BBu  // c4
            },
            // Breakpoint B = 93 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBF36365Eu, // B' = pi/2 - B (high single)
                0xB20BB91Cu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E7B9282u, // c0 (high single)
                0x313383D2u, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DF5D211u, // c1 (low single)
                0x3D6542B3u, // c2
                0x3D0FE5E5u, // c3
                0x3C31FB14u  // c4
            },
            // Breakpoint B = 94 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBF3C7EDDu, // B' = pi/2 - B (high single)
                0x31000ADDu, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E827420u, // c0 (high single)
                0xB20B8B4Du, // c0 (low single)
                0x3E800000u, // c1 (high 1 bit)
                0x3DFB9428u, // c1 (low single)
                0x3D7002B4u, // c2
                0x3D142A6Cu, // c3
                0x3C3A47FFu  // c4
            },
            // Breakpoint B = 95 * pi/128, function tan(B + x) - 1/(pi/2 - (B +
            // x))
            {
                0xBF42C75Cu, // B' = pi/2 - B (high single)
                0x324BBE8Au, // B' = pi/2 - B (low single)
                0x3F800000u, // tau (1 for cot path)
                0x3E87283Fu, // c0 (high single)
                0x3268B966u, // c0 (low single)
                0x3F000000u, // c1 (high 1 bit)
                0xBDFE6529u, // c1 (low single)
                0x3D7B1953u, // c2
                0x3D18E109u, // c3
                0x3C4570B0u  // c4
            },
            // Breakpoint B = 96 * pi/128, function tan(B + x)
            {
                0xBF490FDBu, // B' = pi/2 - B (high single)
                0x32BBBD2Eu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF800000u, // c0 (high single)
                0x2B410000u, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xB3000000u, // c1 (low single)
                0xC0000000u, // c2
                0x402AB7C8u, // c3
                0xC05561DBu  // c4
            },
            // Breakpoint B = 97 * pi/128, function tan(B + x)
            {
                0xBF4F5859u, // B' = pi/2 - B (high single)
                0xB2EE64E8u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF73BB75u, // c0 (high single)
                0xB2FC908Du, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xBDBF94B0u, // c1 (low single)
                0xBFE8550Fu, // c2
                0x40174F67u, // c3
                0xC036C608u  // c4
            },
            // Breakpoint B = 98 * pi/128, function tan(B + x)
            {
                0xBF55A0D8u, // B' = pi/2 - B (high single)
                0xB29886FFu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF68065Eu, // c0 (high single)
                0xB2670D1Au, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xBE36D1D6u, // c1 (low single)
                0xBFD35007u, // c2
                0x4006A861u, // c3
                0xC01D4BDAu  // c4
            },
            // Breakpoint B = 99 * pi/128, function tan(B + x)
            {
                0xBF5BE957u, // B' = pi/2 - B (high single)
                0xB205522Au, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF5CD3BEu, // c0 (high single)
                0xB1460308u, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xBE8306C5u, // c1 (low single)
                0xBFC09232u, // c2
                0x3FF09632u, // c3
                0xC007DB00u  // c4
            },
            // Breakpoint B = 100 * pi/128, function tan(B + x)
            {
                0xBF6231D6u, // B' = pi/2 - B (high single)
                0x3119A6A2u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF521801u, // c0 (high single)
                0xB2AE4178u, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xBEA72938u, // c1 (low single)
                0xBFAFCC22u, // c2
                0x3FD7BD4Au, // c3
                0xBFEBB01Bu  // c4
            },
            // Breakpoint B = 101 * pi/128, function tan(B + x)
            {
                0xBF687A55u, // B' = pi/2 - B (high single)
                0x3252257Bu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF47C8CCu, // c0 (high single)
                0x3200F51Au, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xBEC82C6Cu, // c1 (low single)
                0xBFA0BAE9u, // c2
                0x3FC2252Fu, // c3
                0xBFCD24C7u  // c4
            },
            // Breakpoint B = 102 * pi/128, function tan(B + x)
            {
                0xBF6EC2D4u, // B' = pi/2 - B (high single)
                0x32BEF0A7u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF3DDCCFu, // c0 (high single)
                0xB2D29606u, // c0 (low single)
                0x40000000u, // c1 (high 1 bit)
                0xBEE6606Fu, // c1 (low single)
                0xBF9325D6u, // c2
                0x3FAF4E69u, // c3
                0xBFB3080Cu  // c4
            },
            // Breakpoint B = 103 * pi/128, function tan(B + x)
            {
                0xBF750B52u, // B' = pi/2 - B (high single)
                0xB2EB316Fu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF344BA9u, // c0 (high single)
                0x32B8B0EAu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3EFDF4F7u, // c1 (low single)
                0xBF86DCA8u, // c2
                0x3F9ED53Bu, // c3
                0xBF9CBEDEu  // c4
            },
            // Breakpoint B = 104 * pi/128, function tan(B + x)
            {
                0xBF7B53D1u, // B' = pi/2 - B (high single)
                0xB2955386u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF2B0DC1u, // c0 (high single)
                0xB2AB7EBAu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3EE496C2u, // c1 (low single)
                0xBF776C40u, // c2
                0x3F9065C1u, // c3
                0xBF89AFB6u  // c4
            },
            // Breakpoint B = 105 * pi/128, function tan(B + x)
            {
                0xBF80CE28u, // B' = pi/2 - B (high single)
                0xB1FDD672u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF221C37u, // c0 (high single)
                0x320C61DCu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3ECD4F71u, // c1 (low single)
                0xBF631DAAu, // c2
                0x3F83B471u, // c3
                0xBF7281EAu  // c4
            },
            // Breakpoint B = 106 * pi/128, function tan(B + x)
            {
                0xBF83F267u, // B' = pi/2 - B (high single)
                0xB374CBD9u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF1970C4u, // c0 (high single)
                0xB2904848u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3EB7EFF8u, // c1 (low single)
                0xBF50907Cu, // c2
                0x3F710FEAu, // c3
                0xBF561FEDu  // c4
            },
            // Breakpoint B = 107 * pi/128, function tan(B + x)
            {
                0xBF8716A7u, // B' = pi/2 - B (high single)
                0x32588C6Du, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF1105AFu, // c0 (high single)
                0xB2F045B0u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3EA44EE2u, // c1 (low single)
                0xBF3F8FDBu, // c2
                0x3F5D3FD0u, // c3
                0xBF3D0A23u  // c4
            },
            // Breakpoint B = 108 * pi/128, function tan(B + x)
            {
                0xBF8A3AE6u, // B' = pi/2 - B (high single)
                0xB31EEDF0u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF08D5B9u, // c0 (high single)
                0x325EF98Eu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E92478Du, // c1 (low single)
                0xBF2FEDC9u, // c2
                0x3F4BCD58u, // c3
                0xBF27AE9Eu  // c4
            },
            // Breakpoint B = 109 * pi/128, function tan(B + x)
            {
                0xBF8D5F26u, // B' = pi/2 - B (high single)
                0x330C0105u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBF00DC0Du, // c0 (high single)
                0x3214AF72u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E81B994u, // c1 (low single)
                0xBF218233u, // c2
                0x3F3C4531u, // c3
                0xBF149688u  // c4
            },
            // Breakpoint B = 110 * pi/128, function tan(B + x)
            {
                0xBF908365u, // B' = pi/2 - B (high single)
                0xB292200Du, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBEF22870u, // c0 (high single)
                0xB25271F4u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E65107Au, // c1 (low single)
                0xBF1429F0u, // c2
                0x3F2E8AFCu, // c3
                0xBF040498u  // c4
            },
            // Breakpoint B = 111 * pi/128, function tan(B + x)
            {
                0xBF93A7A5u, // B' = pi/2 - B (high single)
                0x3361DEEEu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBEE2F439u, // c0 (high single)
                0x31F4399Eu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E49341Cu, // c1 (low single)
                0xBF07C61Au, // c2
                0x3F22560Fu, // c3
                0xBEEAA81Eu  // c4
            },
            // Breakpoint B = 112 * pi/128, function tan(B + x)
            {
                0xBF96CBE4u, // B' = pi/2 - B (high single)
                0x314CDE2Eu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBED413CDu, // c0 (high single)
                0x31C06152u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E2FB0CCu, // c1 (low single)
                0xBEF876CBu, // c2
                0x3F177807u, // c3
                0xBED08437u  // c4
            },
            // Breakpoint B = 113 * pi/128, function tan(B + x)
            {
                0xBF99F023u, // B' = pi/2 - B (high single)
                0xB3484328u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBEC5800Du, // c0 (high single)
                0x3214C3C1u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E185E54u, // c1 (low single)
                0xBEE2E342u, // c2
                0x3F0DCA73u, // c3
                0xBEB8CC21u  // c4
            },
            // Breakpoint B = 114 * pi/128, function tan(B + x)
            {
                0xBF9D1463u, // B' = pi/2 - B (high single)
                0x32C55799u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBEB73250u, // c0 (high single)
                0x32028823u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3E0318F8u, // c1 (low single)
                0xBECEA678u, // c2
                0x3F053C67u, // c3
                0xBEA41E53u  // c4
            },
            // Breakpoint B = 115 * pi/128, function tan(B + x)
            {
                0xBFA038A2u, // B' = pi/2 - B (high single)
                0xB2E4CA7Eu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBEA92457u, // c0 (high single)
                0xB0B80830u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3DDF8200u, // c1 (low single)
                0xBEBB99E9u, // c2
                0x3EFB4AA8u, // c3
                0xBE9182BEu  // c4
            },
            // Breakpoint B = 116 * pi/128, function tan(B + x)
            {
                0xBFA35CE2u, // B' = pi/2 - B (high single)
                0x333889B6u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBE9B5042u, // c0 (high single)
                0x322A3AEEu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3DBC7490u, // c1 (low single)
                0xBEA99AF5u, // c2
                0x3EEDE107u, // c3
                0xBE80E9AAu  // c4
            },
            // Breakpoint B = 117 * pi/128, function tan(B + x)
            {
                0xBFA68121u, // B' = pi/2 - B (high single)
                0xB1E43AACu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBE8DB082u, // c0 (high single)
                0x3132A234u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3D9CD7D0u, // c1 (low single)
                0xBE988A60u, // c2
                0x3EE203E3u, // c3
                0xBE63582Cu  // c4
            },
            // Breakpoint B = 118 * pi/128, function tan(B + x)
            {
                0xBFA9A560u, // B' = pi/2 - B (high single)
                0xB3719861u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBE803FD4u, // c0 (high single)
                0x32279E66u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3D807FC8u, // c1 (low single)
                0xBE884BD4u, // c2
                0x3ED7812Du, // c3
                0xBE4636EBu  // c4
            },
            // Breakpoint B = 119 * pi/128, function tan(B + x)
            {
                0xBFACC9A0u, // B' = pi/2 - B (high single)
                0x32655A50u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBE65F267u, // c0 (high single)
                0xB1B4B1DFu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3D4E8B90u, // c1 (low single)
                0xBE718ACAu, // c2
                0x3ECE7164u, // c3
                0xBE2DC161u  // c4
            },
            // Breakpoint B = 120 * pi/128, function tan(B + x)
            {
                0xBFAFEDDFu, // B' = pi/2 - B (high single)
                0xB31BBA77u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBE4BAFAFu, // c0 (high single)
                0xAF2A29E0u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3D221018u, // c1 (low single)
                0xBE53BED0u, // c2
                0x3EC67E26u, // c3
                0xBE1568E2u  // c4
            },
            // Breakpoint B = 121 * pi/128, function tan(B + x)
            {
                0xBFB3121Fu, // B' = pi/2 - B (high single)
                0x330F347Du, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBE31AE4Du, // c0 (high single)
                0x31F32251u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3CF6A500u, // c1 (low single)
                0xBE3707DAu, // c2
                0x3EBFA489u, // c3
                0xBDFBD9C7u  // c4
            },
            // Breakpoint B = 122 * pi/128, function tan(B + x)
            {
                0xBFB6365Eu, // B' = pi/2 - B (high single)
                0xB28BB91Cu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBE17E564u, // c0 (high single)
                0x31C5A2E4u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3CB440D0u, // c1 (low single)
                0xBE1B3D00u, // c2
                0x3EB9F664u, // c3
                0xBDD647C0u  // c4
            },
            // Breakpoint B = 123 * pi/128, function tan(B + x)
            {
                0xBFB95A9Eu, // B' = pi/2 - B (high single)
                0x33651267u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBDFC98C2u, // c0 (high single)
                0x30AE525Cu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3C793D20u, // c1 (low single)
                0xBE003845u, // c2
                0x3EB5271Fu, // c3
                0xBDAC669Eu  // c4
            },
            // Breakpoint B = 124 * pi/128, function tan(B + x)
            {
                0xBFBC7EDDu, // B' = pi/2 - B (high single)
                0x31800ADDu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBDC9B5DCu, // c0 (high single)
                0xB145AD86u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3C1EEF20u, // c1 (low single)
                0xBDCBAAEAu, // c2
                0x3EB14E5Eu, // c3
                0xBD858BB2u  // c4
            },
            // Breakpoint B = 125 * pi/128, function tan(B + x)
            {
                0xBFBFA31Cu, // B' = pi/2 - B (high single)
                0xB3450FB0u, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBD9711CEu, // c0 (high single)
                0xB14FEB28u, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3BB24C00u, // c1 (low single)
                0xBD97E43Au, // c2
                0x3EAE6A89u, // c3
                0xBD4D07E0u  // c4
            },
            // Breakpoint B = 126 * pi/128, function tan(B + x)
            {
                0xBFC2C75Cu, // B' = pi/2 - B (high single)
                0x32CBBE8Au, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBD49393Cu, // c0 (high single)
                0xB0A39F5Bu, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3B1E2B00u, // c1 (low single)
                0xBD49B5D4u, // c2
                0x3EAC4F10u, // c3
                0xBCFD9425u  // c4
            },
            // Breakpoint B = 127 * pi/128, function tan(B + x)
            {
                0xBFC5EB9Bu, // B' = pi/2 - B (high single)
                0xB2DE638Cu, // B' = pi/2 - B (low single)
                0x00000000u, // tau (1 for cot path)
                0xBCC91A31u, // c0 (high single)
                0xAF8E8D1Au, // c0 (low single)
                0x3F800000u, // c1 (high 1 bit)
                0x3A1DFA00u, // c1 (low single)
                0xBCC9392Du, // c2
                0x3EAB1889u, // c3
                0xBC885D3Bu  // c4
            },
        },
}; /*sTan_Table*/

static const _iml_sp_union_t __stan_la__vmlsTanTab[2] = {
    0x00000000, /* ZERO = 0.0 */
    0x7F800000  /* INF = 0x7ff00000 00000000 */
};

inline int __devicelib_imf_internal_stan(const float *a, float *r) {
  int nRet = 0;
  float x, absx;
  /* Get absolute value of argument */
  absx = ((*a));
  (((_iml_sp_union_t *)&absx)->bits.sign = 0);
  if (!(((((_iml_sp_union_t *)&(*a))->bits.exponent) !=
         0xFF))) /********* |(*a)| = Inf,NaN *************/
  {
    if (((_iml_sp_union_t *)&(absx))->hex[0] ==
        ((const _iml_sp_union_t *)&(((const float *)__stan_la__vmlsTanTab)[1]))
            ->hex[0]) /****** |(*a)|=Inf (path 1) ***********/
    {
      /* Return NaN with invalid exception */
      (*r) = (float)((*a) * ((const float *)__stan_la__vmlsTanTab)[0]);
      /* Set ERRDOM error code */
      nRet = 1;
    }    /* |(*a)| = Inf */
    else /**************************** |(*a)|=NaN (path 2) ***********/
    {
      /* Just return NaN and set necessary flags */
      (*r) = (float)((*a) * (*a));
    } /* |(*a)| = NaN */
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_tan_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_tanf(float a) {
  using namespace __imf_impl_tan_s_la;
  VUINT32 vm;
  float va1;
  float vr1;
  float r;
  va1 = a;
  ;
  {
    /* Legacy Code */
    /* Here HW FMA can be unavailable */
    float sAbsX;
    float sAbsMask;
    float sRangeReductionVal;
    float sRangeReductionMask;
    VUINT32 iRangeReductionMask;
    VUINT32 mRangeReductionMask;
    float sZero;
    float sSignX;
    float sInvPi;
    float sY;
    float sRShift;
    float sN;
    float sR;
    float sR2;
    float sPI1;
    float sPI2;
    float sPI3;
    float sPI4;
    float sSignRes;
    float sInvMask;
    float sP1;
    float sP0;
    float sP;
    float sQ2;
    float sQ1;
    float sQ;
    float sQ0;
    float sNumP;
    float sNumQ;
    float sNum;
    float sDenP;
    float sDenQ;
    float sDen;
    float sRes;
    float sTwo;
    float sTmp1;
    float sResLarge;
    vm = 0;
    sZero = as_float(0);
    sAbsMask = as_float(__devicelib_imf_internal_stan_data._sAbsMask);
    sAbsX = as_float((as_uint(va1) & as_uint(sAbsMask)));
    sSignX = as_float((~(as_uint(sAbsMask)) & as_uint(va1)));
    /* Large values check */
    sRangeReductionVal =
        as_float(__devicelib_imf_internal_stan_data._sRangeReductionVal);
    sRangeReductionMask =
        as_float(((VUINT32)(-(VSINT32)(!(sAbsX <= sRangeReductionVal)))));
    iRangeReductionMask = as_uint(sRangeReductionMask);
    mRangeReductionMask = 0;
    mRangeReductionMask = iRangeReductionMask;
    {
      /* ---------------------------------------------------------- */
      /* Main path (_LA_ and _EP_) */
      /* ---------------------------------------------------------- */
      /* Octant calculation */
      sInvPi = as_float(__devicelib_imf_internal_stan_data._sInvPi);
      sRShift = as_float(__devicelib_imf_internal_stan_data._sRShifter);
      sN = __fma(sAbsX, sInvPi, sRShift);
      sY = (sN - sRShift);
      /* Range reduction */
      sPI1 = as_float(__devicelib_imf_internal_stan_data._sPI1_FMA);
      sR = __fma(-(sY), sPI1, sAbsX);
      sPI2 = as_float(__devicelib_imf_internal_stan_data._sPI2_FMA);
      sR = __fma(-(sY), sPI2, sR);
      sPI3 = as_float(__devicelib_imf_internal_stan_data._sPI3_FMA);
      sR = __fma(-(sY), sPI3, sR);
      sR2 = (sR * sR);
      /* Inversion mask and sign calculation */
      sSignRes = as_float(((VUINT32)as_uint(sN) << (31)));
      sSignRes = as_float((as_uint(sSignRes) ^ as_uint(sSignX)));
      sInvMask = as_float(((VUINT32)as_uint(sN) << (30)));
      sInvMask = as_float(((VUINT32)(-(VSINT32)(!(sInvMask == sZero)))));
      /* Rational approximation */
      sP1 = as_float(__devicelib_imf_internal_stan_data._sP1);
      sP0 = as_float(__devicelib_imf_internal_stan_data._sP0);
      sP = __fma(sP1, sR2, sP0);
      sQ2 = as_float(__devicelib_imf_internal_stan_data._sQ2);
      sQ1 = as_float(__devicelib_imf_internal_stan_data._sQ1);
      sQ = __fma(sQ2, sR2, sQ1);
      sP = (sP * sR);
      sQ0 = as_float(__devicelib_imf_internal_stan_data._sQ0);
      sQ = __fma(sQ, sR2, sQ0);
      /* Exchanged numerator and denominator if necessary */
      sNumP = as_float((~(as_uint(sInvMask)) & as_uint(sP)));
      sNumQ = as_float((as_uint(sInvMask) & as_uint(sQ)));
      sNum = as_float((as_uint(sNumP) | as_uint(sNumQ)));
      sDenP = as_float((as_uint(sInvMask) & as_uint(sP)));
      sDenQ = as_float((~(as_uint(sInvMask)) & as_uint(sQ)));
      sDen = as_float((as_uint(sDenP) | as_uint(sDenQ)));
      /* Division */
      sRes = (sNum / sDen);
      /* Sign setting */
      vr1 = as_float((as_uint(sRes) ^ as_uint(sSignRes)));
      /* ---------------------------------------------------------- */
      /* End of main path (_LA_ and _EP_) */
      /* ---------------------------------------------------------- */
    }
    if (__builtin_expect((mRangeReductionMask) != 0, 0)) {
      /* ---------------------------------------------------------- */
      /* Large arguments path (_HA_, _LA_ and _EP_) */
      /* ---------------------------------------------------------- */
      /***************************************************************/
      /***************************** LA *******************************/
      /***************************************************************/
      /*
      //       
      //           The basic structure of the algorithm is very similar to the HA one
      //           (described in the comments at the top). But some of the elaborate
      //           2-part computations become simpler.
      //       
      */
      float sX;
      float sRangeMask;
      VUINT32 iRangeMask;
      VUINT32 mRangeMask;
      float sN;
      float sY;
      VUINT32 iY;
      VUINT32 iIndex;
      float sE1;
      float sE2;
      float sE3;
      float sE4;
      float sE;
      float sR1;
      float sR2;
      float sR3;
      float sR4;
      float sR;
      float sRMed;
      float sRp2;
      float sPS;
      float sPC;
      float sPolS;
      float sPolC;
      float sRSigma;
      float sMed;
      float sD;
      float sCorr;
      float sK0;
      float sK1;
      float sK2;
      float sK3;
      float sResLo;
      float sResLo0;
      float sResInt;
      float sResHi;
      float sAbsMask;
      float sRangeVal;
      float sPIu;
      float sRShifter;
      VUINT32 iIndexMask;
      float sPIoHi;
      float sPIoLo1;
      float sPIoLo2;
      float sPIoTail;
      float sSigma;
      float sCHL;
      float sSHi;
      float sSLo;
      float sS1;
      float sS2;
      float sC1;
      float sC2;
      VUINT32 iIndexPeriodMask;
      float sB_hi;
      float sB_lo;
      float sR_full;
      float sR_hi;
      float sR_lo;
      float sHalfMask;
      float sOne;
      float sTau;
      float sRecip_hi;
      float sRecip_lo;
      float sEr;
      float dR_RE;
      float dRE;
      float sRecip_ok;
      float dD_E;
      float sD2;
      float sZ2;
      float sZ4;
      float sH1;
      float sH2;
      float sH3;
      float sH4;
      float sH5;
      float sH6;
      float sH7;
      float sH8;
      float sH9;
      float sC0_hi;
      float sC0_lo;
      float sC1_hi;
      float sC1_lo;
      float sC3;
      float sC4;
      float sC5;
      float sC6;
      float sC7;
      float sEC1;
      float sP1;
      float sP2;
      float sP3;
      float sP4;
      float sP5;
      float sP6;
      float sP7;
      float sP8;
      float sP9;
      float sP10;
      float sP11;
      float sP12;
      float sP13;
      float sP14;
      float sP15;
      float sP16;
      float sLoad[10];
      float sExpMask;
      float sEMax;
      float sSpecialMask;
      VUINT32 iSpecialMask;
      /* -------------------- Implementation --------------------- */
      // Hence set _VRANGEMASK bit iff corresponding exponent is above EMax
      sExpMask = as_float(0x7f800000u);
      ;
      sX = as_float((as_uint(va1) & as_uint(sExpMask)));
      sEMax = as_float(0x7f800000u);
      ;
      sSpecialMask = as_float(((VUINT32)(-(VSINT32)(sX == sEMax))));
      iSpecialMask = as_uint(sSpecialMask);
      vm = 0;
      vm = iSpecialMask;
      // Perform full argument reduction (could make this conditional)
      {
        /* file: _vsreduction_core.i */
        //
        // Vectorized huge argument reduction for trig functions
        // The scale factor is Pi/2^8
        // The input is argument for reduction _VARG_A(of type S)
        // The output is high _VRES_Z (of type S) + low _VRES_E (S), and the
        // integer part is _VRES_IND (I)
        //
        {
          VUINT32 iInput;
          VUINT32 iExponent;
          VUINT32 iSignificand;
          VUINT32 iIntegerBit;
          float sP_hi;
          float sP_med;
          float sP_lo;
          VUINT32 iP_hi;
          VUINT32 iP_med;
          VUINT32 iP_lo;
          VUINT32 iLowMask;
          VUINT32 iP5;
          VUINT32 iP4;
          VUINT32 iP3;
          VUINT32 iP2;
          VUINT32 iP1;
          VUINT32 iP0;
          VUINT32 iM1;
          VUINT32 iM0;
          VUINT32 iM15;
          VUINT32 iM14;
          VUINT32 iM13;
          VUINT32 iM12;
          VUINT32 iM11;
          VUINT32 iM10;
          VUINT32 iM05;
          VUINT32 iM04;
          VUINT32 iM03;
          VUINT32 iM02;
          VUINT32 iM01;
          VUINT32 iM00;
          VUINT32 iN14;
          VUINT32 iN13;
          VUINT32 iN12;
          VUINT32 iN11;
          VUINT32 iP15;
          VUINT32 iP14;
          VUINT32 iP13;
          VUINT32 iP12;
          VUINT32 iP11;
          VUINT32 iQ14;
          VUINT32 iQ13;
          VUINT32 iQ12;
          VUINT32 iQ11;
          VUINT32 iReduceHi;
          VUINT32 iReduceMed;
          VUINT32 iReducedLo;
          VUINT32 iRoundBump;
          VUINT32 iShiftedN;
          VUINT32 iNMask;
          float sReducedHi;
          float sReducedMed;
          float sReducedLo;
          VUINT32 iExponentPart;
          VUINT32 iShiftedSig;
          float sShifter;
          float sIntegerPart;
          float sRHi;
          float sRLo;
          VUINT32 iSignBit;
          float s2pi_full;
          float s2pi_lead;
          float s2pi_trail;
          float sLeadmask;
          float sRHi_lead;
          float sRHi_trail;
          float sPir1;
          float sPir2;
          float sPir3;
          float sPir4;
          float sPir12;
          float sPir34;
          float sRedPreHi;
          float sRedHi;
          float sRedLo;
          float sMinInput;
          float sAbs;
          float sMultiplex;
          float sNotMultiplex;
          float sMultiplexedInput;
          float sMultiplexedOutput;
          // Cast the input to an integer
          iInput = as_uint(sAbsX);
          // Grab exponent and convert it to a table offset
          iExponent = 0x7f800000u;
          ;
          iExponent = (iExponent & iInput);
          iExponent = ((VUINT32)(iExponent) >> (23));
          // Get the (2^a / 2pi) mod 1 values from the table.
          // Because VLANG doesn't have I-type gather, we need a trivial cast
          sP_hi = as_float(((
              const VUINT32
                  *)(__devicelib_imf_internal_stan_reduction_data
                         ._sPtable))[(((0 + iExponent) * (3 * 4)) >> (2)) + 0]);
          sP_med = as_float(((
              const VUINT32
                  *)(__devicelib_imf_internal_stan_reduction_data
                         ._sPtable))[(((0 + iExponent) * (3 * 4)) >> (2)) + 1]);
          sP_lo = as_float(((
              const VUINT32
                  *)(__devicelib_imf_internal_stan_reduction_data
                         ._sPtable))[(((0 + iExponent) * (3 * 4)) >> (2)) + 2]);
          iP_hi = as_uint(sP_hi);
          iP_med = as_uint(sP_med);
          iP_lo = as_uint(sP_lo);
          // Also get the significand as an integer
          // NB: adding in the integer bit is wrong for denorms!
          // To make this work for denorms we should do something slightly
          // different
          iSignificand = 0x007fffffu;
          ;
          iIntegerBit = 0x00800000u;
          iSignificand = (iSignificand & iInput);
          iSignificand = (iSignificand + iIntegerBit);
          // Break the P_xxx and m into 16-bit chunks ready for
          // the long multiplication via 16x16->32 multiplications
          iLowMask = 0x0000FFFFu;
          iP5 = ((VUINT32)(iP_hi) >> (16));
          iP4 = (iP_hi & iLowMask);
          iP3 = ((VUINT32)(iP_med) >> (16));
          iP2 = (iP_med & iLowMask);
          iP1 = ((VUINT32)(iP_lo) >> (16));
          iP0 = (iP_lo & iLowMask);
          iM1 = ((VUINT32)(iSignificand) >> (16));
          iM0 = (iSignificand & iLowMask);
          // Now do the big multiplication and carry propagation
          iM15 = (iM1 * iP5);
          iM14 = (iM1 * iP4);
          iM13 = (iM1 * iP3);
          iM12 = (iM1 * iP2);
          iM11 = (iM1 * iP1);
          iM10 = (iM1 * iP0);
          iM05 = (iM0 * iP5);
          iM04 = (iM0 * iP4);
          iM03 = (iM0 * iP3);
          iM02 = (iM0 * iP2);
          iM01 = (iM0 * iP1);
          iM00 = (iM0 * iP0);
          iN11 = ((VUINT32)(iM01) >> (16));
          iN12 = ((VUINT32)(iM02) >> (16));
          iN13 = ((VUINT32)(iM03) >> (16));
          iN14 = ((VUINT32)(iM04) >> (16));
          iN11 = (iM11 + iN11);
          iN12 = (iM12 + iN12);
          iN13 = (iM13 + iN13);
          iN14 = (iM14 + iN14);
          iP11 = (iM02 & iLowMask);
          iP12 = (iM03 & iLowMask);
          iP13 = (iM04 & iLowMask);
          iP14 = (iM05 & iLowMask);
          iP15 = ((VUINT32)(iM05) >> (16));
          iP11 = (iP11 + iN11);
          iP12 = (iP12 + iN12);
          iP13 = (iP13 + iN13);
          iP14 = (iP14 + iN14);
          iP15 = (iP15 + iM15);
          iQ11 = ((VUINT32)(iM10) >> (16));
          iQ11 = (iQ11 + iP11);
          iQ12 = ((VUINT32)(iQ11) >> (16));
          iQ12 = (iQ12 + iP12);
          iQ13 = ((VUINT32)(iQ12) >> (16));
          iQ13 = (iQ13 + iP13);
          iQ14 = ((VUINT32)(iQ13) >> (16));
          iQ14 = (iQ14 + iP14);
          // Assemble reduced argument from the pieces
          iQ11 = (iQ11 & iLowMask);
          iQ13 = (iQ13 & iLowMask);
          iReduceHi = ((VUINT32)(iQ14) << (16));
          iReducedLo = ((VUINT32)(iQ12) << (16));
          iReduceHi = (iReduceHi + iQ13);
          iReducedLo = (iReducedLo + iQ11);
          // We want to incorporate the original sign now too.
          // Do it here for convenience in getting the right N value,
          // though we could wait right to the end if we were prepared
          // to modify the sign of N later too.
          // So get the appropriate sign mask now (or sooner).
          iSignBit = 0x80000000u;
          ;
          iSignBit = (iSignBit & iInput);
          // Create floating-point high part, implicitly adding integer bit 1
          // Incorporate overall sign at this stage too.
          iExponentPart = 0x3F800000u;
          iExponentPart = (iSignBit ^ iExponentPart);
          iShiftedSig = ((VUINT32)(iReduceHi) >> (9));
          iShiftedSig = (iShiftedSig | iExponentPart);
          sReducedHi = as_float(iShiftedSig);
          // Now round at the 2^-8 bit position for reduction mod pi/2^7
          // instead of the original 2pi (but still with the same 2pi scaling).
          // Use a shifter of 2^15 + 2^14.
          // The N we get is our final version; it has an offset of
          // 2^8 because of the implicit integer bit, and anyway for negative
          // starting value it's a 2s complement thing. But we need to mask
          // off the exponent part anyway so it's fine.
          sShifter = as_float(0x47400000u);
          sIntegerPart = (sShifter + sReducedHi);
          sN = (sIntegerPart - sShifter);
          sReducedHi = (sReducedHi - sN);
          // Grab our final N value as an integer, appropriately masked mod 2^8
          iIndex = as_uint(sIntegerPart);
          iNMask = 0x000000FFu;
          iIndex = (iIndex & iNMask);
          // Create floating-point low and medium parts, respectively
          //
          // lo_17, ... lo_0, 0, ..., 0
          // hi_8, ... hi_0, lo_31, ..., lo_18
          //
          // then subtract off the implicitly added integer bits,
          // 2^-46 and 2^-23, respectively.
          // Put the original sign into all of them at this stage.
          iExponentPart = 0x28800000u;
          iExponentPart = (iSignBit ^ iExponentPart);
          iShiftedSig = 0x0003FFFFu;
          iShiftedSig = (iShiftedSig & iReducedLo);
          iShiftedSig = ((VUINT32)(iShiftedSig) << (5));
          iShiftedSig = (iShiftedSig | iExponentPart);
          sReducedLo = as_float(iShiftedSig);
          sShifter = as_float(iExponentPart);
          sReducedLo = (sReducedLo - sShifter);
          iExponentPart = 0x34000000u;
          iExponentPart = (iSignBit ^ iExponentPart);
          iShiftedSig = 0x000001FFu;
          iShiftedSig = (iShiftedSig & iReduceHi);
          iShiftedSig = ((VUINT32)(iShiftedSig) << (14));
          iReducedLo = ((VUINT32)(iReducedLo) >> (18));
          iShiftedSig = (iShiftedSig | iReducedLo);
          iShiftedSig = (iShiftedSig | iExponentPart);
          sReducedMed = as_float(iShiftedSig);
          sShifter = as_float(iExponentPart);
          sReducedMed = (sReducedMed - sShifter);
          // Now add them up into 2 reasonably aligned pieces
          sRHi = (sReducedHi + sReducedMed);
          sReducedHi = (sReducedHi - sRHi);
          sReducedMed = (sReducedMed + sReducedHi);
          sRLo = (sReducedMed + sReducedLo);
          // Now multiply those numbers all by 2 pi, reasonably accurately.
          // (RHi + RLo) * (pi_lead + pi_trail) ~=
          // RHi * pi_lead + (RHi * pi_trail + RLo * pi_lead)
          s2pi_lead = as_float(0x40c90fdbu);
          s2pi_trail = as_float(0xb43bbd2eu);
          sRedHi = (sRHi * s2pi_lead);
          sRedLo = __fma(sRHi, s2pi_lead, -(sRedHi));
          sRedLo = __fma(sRHi, s2pi_trail, sRedLo);
          sRedLo = __fma(sRLo, s2pi_lead, sRedLo);
          // If the magnitude of the input is <= 2^-20, then
          // just pass through the input, since no reduction will be needed and
          // the main path will only work accurately if the reduced argument is
          // about >= 2^-40 (which it is for all large pi multiples)
          sAbsMask = as_float(0x7FFFFFFFu);
          sMinInput = as_float(0x35800000u);
          sAbs = as_float((as_uint(sAbsX) & as_uint(sAbsMask)));
          sMultiplex = as_float(((VUINT32)(-(VSINT32)(sAbs > sMinInput))));
          sNotMultiplex = as_float(((VUINT32)(-(VSINT32)(sAbs <= sMinInput))));
          sMultiplexedInput =
              as_float((as_uint(sNotMultiplex) & as_uint(sAbsX)));
          sMultiplexedOutput =
              as_float((as_uint(sMultiplex) & as_uint(sRedHi)));
          sR = as_float(
              (as_uint(sMultiplexedInput) | as_uint(sMultiplexedOutput)));
          sE = as_float((as_uint(sMultiplex) & as_uint(sRedLo)));
          // The output is _VRES_R (high) + _VRES_E (low), and the integer part
          // is _VRES_IND Set sRp2 = _VRES_R^2 and then resume the original
          // code.
        }
      }
      // Argument reduction is now finished: x = n * pi/128 + r
      // where n = iIndex and r = sR (high) + sE (low).
      // But we have n modulo 256, needed for sin/cos with period 2pi
      // but we want it modulo 128 since tan has period pi.
      iIndexPeriodMask = 0x0000007Fu;
      iIndex = (iIndex & iIndexPeriodMask);
      // Simply combine the two parts of the reduced argument
      // since we can afford a few ulps in this case.
      sR = (sR + sE);
      // Load constants by index
      sB_hi = as_float(
          ((const VUINT32
                *)(__devicelib_imf_internal_stan_data
                       ._sCoeffs))[(((0 + iIndex) * (10 * 4)) >> (2)) + 0]);
      sB_lo = as_float(
          ((const VUINT32
                *)(__devicelib_imf_internal_stan_data
                       ._sCoeffs))[(((0 + iIndex) * (10 * 4)) >> (2)) + 1]);
      sTau = as_float(
          ((const VUINT32
                *)(__devicelib_imf_internal_stan_data
                       ._sCoeffs))[(((0 + iIndex) * (10 * 4)) >> (2)) + 2]);
      sC0_hi = as_float(
          ((const VUINT32
                *)(__devicelib_imf_internal_stan_data
                       ._sCoeffs))[(((0 + iIndex) * (10 * 4)) >> (2)) + 3]);
      sC0_lo = as_float(
          ((const VUINT32
                *)(__devicelib_imf_internal_stan_data
                       ._sCoeffs))[(((0 + iIndex) * (10 * 4)) >> (2)) + 4]);
      sC1_hi = as_float(
          ((const VUINT32
                *)(__devicelib_imf_internal_stan_data
                       ._sCoeffs))[(((0 + iIndex) * (10 * 4)) >> (2)) + 5]);
      sC1_lo = as_float(
          ((const VUINT32
                *)(__devicelib_imf_internal_stan_data
                       ._sCoeffs))[(((0 + iIndex) * (10 * 4)) >> (2)) + 6]);
      sC2 = as_float(
          ((const VUINT32
                *)(__devicelib_imf_internal_stan_data
                       ._sCoeffs))[(((0 + iIndex) * (10 * 4)) >> (2)) + 7]);
      sC3 = as_float(
          ((const VUINT32
                *)(__devicelib_imf_internal_stan_data
                       ._sCoeffs))[(((0 + iIndex) * (10 * 4)) >> (2)) + 8]);
      sC4 = as_float(
          ((const VUINT32
                *)(__devicelib_imf_internal_stan_data
                       ._sCoeffs))[(((0 + iIndex) * (10 * 4)) >> (2)) + 9]);
      // Construct a separate reduced argument modulo pi near pi/2 multiples.
      // i.e. (pi/2 - x) mod pi, simply by subtracting the reduced argument
      // from an accurate B_hi + B_lo = (128 - n) pi/128. Force the upper part
      // of this reduced argument to half-length to simplify accurate
      // reciprocation later on.
/*
//       ................. Compute 2-part reciprocal component
//        * ..................
*/
      sR_full = (sB_hi - sR);
      sHalfMask = as_float(0xFFFFF000u);
      sR_hi = as_float((as_uint(sR_full) & as_uint(sHalfMask)));
      sR_lo = (sB_hi - sR_full);
      sR_lo = (sR_lo - sR);
      sR_full = (sR_full - sR_hi);
      sR_full = (sR_full + sB_lo);
      sR_lo = (sR_lo + sR_full);
      // Now compute an approximate reciprocal to mix into the computation
      // To avoid any danger of nonportability, force it to 12 bits,
      // though I suspect it always is anyway on current platforms.
      sRecip_hi = (1.0f / (sR_hi));
      sRecip_hi = as_float((as_uint(sRecip_hi) & as_uint(sHalfMask)));
      // Now compute the error sEr where sRecip_hi = (1/R_hi) * (1 - sEr)
      // so that we can compensate for it.
      sOne = as_float(__devicelib_imf_internal_stan_data._sOne);
      sEr = __fma(-(sR_hi), sRecip_hi, sOne);
      // Get a better approximation to  1/sR_hi (not far short of an ulp)
      // using a third-order polynomial approximation
      dR_RE = __fma(sRecip_hi, sEr, sRecip_hi);
      sE2 = __fma(sEr, sEr, sOne);
      sRecip_ok = (dR_RE * sE2);
      // Multiply by sRecip_ok to make sR_lo relative to sR_hi
      // Since sR_lo is shifted off by about 12 bits, this is accurate enough.
      sR_lo = (sR_lo * sRecip_ok);
      // Now create a low reciprocal using
      //
      //     (Recip_hi + Er * Recip_ok) * (1 + sR_lo^2 - sR_lo)
      // =~= Recip_hi + Recip_ok * (Er + sR_lo^2 - sR_lo)
      dD_E = (sR_lo - sEr);
      sRecip_lo = __fma(sR_lo, sR_lo, -(dD_E));
      sRecip_lo = (sRecip_lo * sRecip_ok);
      sH2 = __fma(sC1_hi, sR, sC0_hi);
      sH3 = (sC0_hi - sH2);
      sH4 = __fma(sRecip_hi, sTau, sH2);
      sH5 = __fma(sC1_hi, sR, sH3);
      sH6 = __fma(sRecip_hi, sTau, -(sH4));
      sH7 = __fma(sRecip_lo, sTau, sH5);
      sH8 = (sH6 + sH2);
      sH9 = (sH7 + sH8);
      /*............... Higher polynomial terms .................*/
      // Stage 1 (with unlimited parallelism)
      // P3 = C1_lo + C2 * Z
      sP3 = __fma(sC2, sR, sC1_lo);
      // P4 = C3 + C4 * Z
      sP4 = __fma(sC4, sR, sC3);
      // Z2 = Z^2
      sZ2 = (sR * sR);
      // C1 with both pieces combined
      sC1 = (sC1_hi + sC1_lo);
      // Stage 2 (with unlimited parallelism)
      // P6 = C1_lo + C2 * Z + C3 * Z^2 + C4 * Z^3
      sP6 = __fma(sZ2, sP4, sP3);
      // P9 = trail(dominant part) + C0_lo
      sP9 = (sC0_lo + sH9);
      // Final accumulation of low part
      sP10 = __fma(sP6, sR, sP9);
      // And now the very final summation
      sResLarge = (sH4 + sP10);
      sResLarge = as_float((as_uint(sResLarge) ^ as_uint(sSignX)));
      /* Merge results from main and large paths: */
      vr1 = as_float((((~as_uint(sRangeReductionMask)) & as_uint(vr1)) |
                      (as_uint(sRangeReductionMask) & as_uint(sResLarge))));
    }
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    float __cout_r1;
    ((float *)&__cout_a1)[0] = va1;
    ((float *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_stan(&__cout_a1, &__cout_r1);
    vr1 = ((const float *)&__cout_r1)[0];
  }
  r = vr1;
  ;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
