// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  mic_integer_functions.cpp
///////////////////////////////////////////////////////////

#if defined (__MIC__) || defined(__MIC2__)

#ifdef __cplusplus
extern "C" {
#endif

#include <intrin.h>

#include "mic_cl_integer_declaration.h"

///
#define OCL_INTR_P1_iGn_iGn(func)   \
    OCL_INTR_P1_iCn_iCn(func)       \
    OCL_INTR_P1_iSn_iSn(func)       \
    OCL_INTR_P1_iIn_iIn(func)       \
    OCL_INTR_P1_iLn_iLn(func)

#define OCL_INTR_P1_uGn_uGn(func)   \
    OCL_INTR_P1_uCn_uCn(func)       \
    OCL_INTR_P1_uSn_uSn(func)       \
    OCL_INTR_P1_uIn_uIn(func)       \
    OCL_INTR_P1_uLn_uLn(func)

#define OCL_INTR_P1_uGn_iGn(func)   \
    OCL_INTR_P1_uCn_iCn(func)       \
    OCL_INTR_P1_uSn_iSn(func)       \
    OCL_INTR_P1_uIn_iIn(func)       \
    OCL_INTR_P1_uLn_iLn(func)

#define OCL_INTR_P1_Gn_Gn(func)     \
    OCL_INTR_P1_iGn_iGn(func)       \
    OCL_INTR_P1_uGn_uGn(func)

#define OCL_INTR_P1_uGn_Gn(func)    \
    OCL_INTR_P1_uGn_iGn(func)       \
    OCL_INTR_P1_uGn_uGn(func)

///
#define OCL_INTR_P2_iGn_iGniGn(func)    \
    OCL_INTR_P2_iCn_iCn_iCn(func)       \
    OCL_INTR_P2_iSn_iSn_iSn(func)       \
    OCL_INTR_P2_iIn_iIn_iIn(func)       \
    OCL_INTR_P2_iLn_iLn_iLn(func)

#define OCL_INTR_P2_uGn_uGnuGn(func)    \
    OCL_INTR_P2_uCn_uCn_uCn(func)       \
    OCL_INTR_P2_uSn_uSn_uSn(func)       \
    OCL_INTR_P2_uIn_uIn_uIn(func)       \
    OCL_INTR_P2_uLn_uLn_uLn(func)

#define OCL_INTR_P2_uGn_iGniGn(func)    \
    OCL_INTR_P2_uCn_iCn_iCn(func)       \
    OCL_INTR_P2_uSn_iSn_iSn(func)       \
    OCL_INTR_P2_uIn_iIn_iIn(func)       \
    OCL_INTR_P2_uLn_iLn_iLn(func)

#define OCL_INTR_P2_Gn_GnGn(func)       \
    OCL_INTR_P2_iGn_iGniGn(func)        \
    OCL_INTR_P2_uGn_uGnuGn(func)

#define OCL_INTR_P2_uGn_GnGn(func)      \
    OCL_INTR_P2_uGn_iGniGn(func)        \
    OCL_INTR_P2_uGn_uGnuGn(func)

#define OCL_FUNC_P2_iGn_iGniG1(func)    \
    OCL_FUNC_P2_iCn_iCn_iC1(func)       \
    OCL_FUNC_P2_iSn_iSn_iS1(func)       \
    OCL_FUNC_P2_iIn_iIn_iI1(func)       \
    OCL_FUNC_P2_iLn_iLn_iL1(func)

#define OCL_FUNC_P2_uGn_uGnuG1(func)    \
    OCL_FUNC_P2_uCn_uCn_uC1(func)       \
    OCL_FUNC_P2_uSn_uSn_uS1(func)       \
    OCL_FUNC_P2_uIn_uIn_uI1(func)       \
    OCL_FUNC_P2_uLn_uLn_uL1(func)

#define OCL_FUNC_P2_Gn_GnG1(func)       \
    OCL_FUNC_P2_iGn_iGniG1(func)        \
    OCL_FUNC_P2_uGn_uGnuG1(func)

///
#define OCL_INTR_P3_iGn_iGniGniGn(func) \
    OCL_INTR_P3_iCn_iCn_iCn_iCn(func)   \
    OCL_INTR_P3_iSn_iSn_iSn_iSn(func)   \
    OCL_INTR_P3_iIn_iIn_iIn_iIn(func)   \
    OCL_INTR_P3_iLn_iLn_iLn_iLn(func)

#define OCL_INTR_P3_uGn_uGnuGnuGn(func) \
    OCL_INTR_P3_uCn_uCn_uCn_uCn(func)   \
    OCL_INTR_P3_uSn_uSn_uSn_uSn(func)   \
    OCL_INTR_P3_uIn_uIn_uIn_uIn(func)   \
    OCL_INTR_P3_uLn_uLn_uLn_uLn(func)

#define OCL_INTR_P3_Gn_GnGnGn(func)     \
    OCL_INTR_P3_iGn_iGniGniGn(func)     \
    OCL_INTR_P3_uGn_uGnuGnuGn(func)

#define OCL_INTR_P3_In_InInIn(func)     \
    OCL_INTR_P3_iIn_iIn_iIn_iIn(func)   \
    OCL_INTR_P3_uIn_uIn_uIn_uIn(func)

#define OCL_FUNC_P3_iGn_iGniG1iG1(func) \
    OCL_FUNC_P3_iCn_iCn_iC1_iC1(func)   \
    OCL_FUNC_P3_iSn_iSn_iS1_iS1(func)   \
    OCL_FUNC_P3_iIn_iIn_iI1_iI1(func)   \
    OCL_FUNC_P3_iLn_iLn_iL1_iL1(func)

#define OCL_FUNC_P3_uGn_uGnuG1uG1(func) \
    OCL_FUNC_P3_uCn_uCn_uC1_uC1(func)   \
    OCL_FUNC_P3_uSn_uSn_uS1_uS1(func)   \
    OCL_FUNC_P3_uIn_uIn_uI1_uI1(func)   \
    OCL_FUNC_P3_uLn_uLn_uL1_uL1(func)

#define OCL_FUNC_P3_Gn_GnG1G1(func)     \
    OCL_FUNC_P3_iGn_iGniG1iG1(func)     \
    OCL_FUNC_P3_uGn_uGnuG1uG1(func)

/// OpenCL Spec 1.2 (rev 15), Section 6.12.3, Table 6.10

OCL_INTR_P1_uGn_Gn          (abs)
OCL_INTR_P2_uGn_GnGn        (abs_diff)
OCL_INTR_P2_Gn_GnGn         (add_sat)
OCL_INTR_P2_Gn_GnGn         (hadd)
OCL_INTR_P2_Gn_GnGn         (rhadd)
OCL_INTR_P3_Gn_GnGnGn       (clamp)
    OCL_FUNC_P3_Gn_GnG1G1   (clamp)
OCL_INTR_P1_Gn_Gn           (clz)
OCL_INTR_P3_Gn_GnGnGn       (mad_hi)
OCL_INTR_P3_Gn_GnGnGn       (mad_sat)
OCL_INTR_P2_Gn_GnGn         (max)
    OCL_FUNC_P2_Gn_GnG1     (max)
OCL_INTR_P2_Gn_GnGn         (min)
    OCL_FUNC_P2_Gn_GnG1     (min)
OCL_INTR_P2_Gn_GnGn         (mul_hi)
OCL_INTR_P2_Gn_GnGn         (rotate)
OCL_INTR_P2_Gn_GnGn         (sub_sat)
OCL_INTR_P2_iSn_iCn_uCn     (upsample)
OCL_INTR_P2_uSn_uCn_uCn     (upsample)
OCL_INTR_P2_iIn_iSn_uSn     (upsample)
OCL_INTR_P2_uIn_uSn_uSn     (upsample)
OCL_INTR_P2_iLn_iIn_uIn     (upsample)
OCL_INTR_P2_uLn_uIn_uIn     (upsample)
OCL_INTR_P1_Gn_Gn           (popcount)

/// OpenCL Spec 1.2 (rev 15), Section 6.12.3, Table 6.11

OCL_INTR_P3_In_InInIn       (mad24)
OCL_INTR_P3_In_InInIn       (mul24)

/// native implementations
///

// 0a0b0c0d0e0f0g0h -> 00000000abcdefgh or
// a0b0c0d0e0f0g0h0 -> 00000000abcdefgh
//
uchar __attribute__((__always_inline__, overloadable))
_mask16z8(ushort m16)
{
    uint r = _mm_bitinterleave11_32(m16, m16); // 0a0b ->  00aa00bb or a0b0 -> aa00bb00
    uint hi = _mm_quadmask16_32(r >> 16);
    uint lo = _mm_quadmask16_32(r);
    return (hi << 4) | lo;
}

// 00000000abcdefgh -> 0a0b0c0d0e0f0g0h
ushort __attribute__((__always_inline__, overloadable))
_mask8z16e(uchar m8) // even
{
    return _mm_bitinterleave11_16(0, m8);
}

// 00000000abcdefgh -> a0b0c0d0e0f0g0h0
ushort __attribute__((__always_inline__, overloadable))
_mask8z16o(uchar m8) // odd
{
    return _mm_bitinterleave11_16(m8, 0);
}

// x < 0
uchar __attribute__((__always_inline__, overloadable))
_cmplt_zero(long8 x)
{
    ushort m16 = _mm512_mask_cmplt_pi(0x55, _mm512_swizzle_epi32(x, _MM_SWIZ_REG_CDAB), (int16)0);
    return _mask16z8(m16);
}

// x > 0
uchar __attribute__((__always_inline__, overloadable))
_cmpgt_zero(long8 x)
{
    ushort m16 = _mm512_mask_cmplt_pi(0x55, (int16)0, _mm512_swizzle_epi32(x, _MM_SWIZ_REG_CDAB));
    return _mask16z8(m16);
}

// addsetc
long8 __attribute__((__always_inline__, overloadable))
_addsetc(long8 x, long8 y, uchar *cout)
{
    ushort c16 = 0;
    uint16 r = _mm512_mask_addsetc_epi32(x, 0x55, c16, y, &c16);
    r = _mm512_mask_adc_epi32(r, 0xAA, c16 << 1, y, &c16);
    *cout = _mask16z8(c16);
    return as_long8(r);
}
ulong8 __attribute__((__always_inline__, overloadable))
_addsetc(ulong8 x, ulong8 y, uchar *cout)
{
    return as_ulong8(_addsetc(as_long8(x), as_long8(y), cout));
}

// adc
long8 __attribute__((__always_inline__, overloadable))
_adc(long8 x, uchar cin, long8 y, uchar *cout)
{
    ushort c16 = _mask8z16e(cin);
    uint16 r = _mm512_mask_adc_epi32(x, 0x55, c16, y, &c16);
    r = _mm512_mask_adc_epi32(x, 0xAA, c16 << 1, y, &c16);
    *cout = _mask16z8(c16);
    return as_long8(r);
}
ulong8 __attribute__((__always_inline__, overloadable))
_adc(ulong8 x, uchar cin, ulong8 y, uchar *cout)
{
    return as_ulong8(_adc(as_long8(x), cin, as_long8(y), cout));
}

/// abs
uint16 __attribute__((overloadable)) abs(int16 x)
{
    ushort m16 = _mm512_cmplt_pi(x, (int16)0);  // x < 0
    return _mm512_mask_mov_epi32(x, m16, -x);   // x < 0 ? -x : x
}
uint16 __attribute__((overloadable)) mask_abs(ushort m16, int16 x)
{
    return abs(x);
}

uint16 __attribute__((overloadable)) abs(uint16 x)
{
    return x;
}
uint16 __attribute__((overloadable)) mask_abs(ushort m16, uint16 x)
{
    return abs(x);
}

ulong8 __attribute__((overloadable)) abs(long8 x)
{
    uchar m8 = _cmplt_zero(x);
    return _mm512_mask_mov_epi64(x, m8, -x);        // x < 0 ? -x : x
}
ulong8 __attribute__((overloadable)) mask_abs(uchar m8, long8 x)
{
    return abs(x);
}

ulong8 __attribute__((overloadable)) abs(ulong8 x)
{
    return x;
}
ulong8 __attribute__((overloadable)) mask_abs(uchar m8, ulong8 x)
{
    return abs(x);
}

// abs_diff
uint16 __attribute__((overloadable)) abs_diff(int16 x, int16 y)
{
    return abs(x - y);
}
uint16 __attribute__((overloadable)) mask_abs_diff(ushort m16, int16 x, int16 y)
{
    return abs_diff(x, y);
}

uint16 __attribute__((overloadable)) abs_diff(uint16 x, uint16 y)
{
    return abs(as_int16(x - y));
}
uint16 __attribute__((overloadable)) mask_abs_diff(ushort m16, uint16 x, uint16 y)
{
    return abs_diff(x, y);
}

ulong8 __attribute__((overloadable)) abs_diff(long8 x, long8 y)
{
    return abs(x - y);
}
ulong8 __attribute__((overloadable)) mask_abs_diff(uchar m8, long8 x, long8 y)
{
    return abs_diff(x, y);
}

ulong8 __attribute__((overloadable)) abs_diff(ulong8 x, ulong8 y)
{
    return as_ulong8(abs(as_long8(x - y)));
}
ulong8 __attribute__((overloadable)) mask_abs_diff(uchar m8, ulong8 x, ulong8 y)
{
    return abs_diff(x, y);
}

// add_sat
int16 __attribute__((overloadable)) add_sat(int16 x, int16 y)
{
    ushort s16;
    int16 r = _mm512_addsets_epi32(x, y, &s16);
    ushort p16 = _mm512_cmplt_pi((int16)0, x | y) & s16; // both x and y > 0 but sign bit is set
    ushort n16 = _mm512_cmplt_pi(x & y, (int16)0) & (~s16); // both x and y < 0 but sign bit is not set
    return _mm512_mask_mov_epi32(_mm512_mask_mov_epi32(r, p16, (int16)INT_MAX), n16, (int16)INT_MIN);
}
int16 __attribute__((overloadable)) mask_add_sat(ushort m16, int16 x, int16 y)
{
    return add_sat(x, y);
}

uint16 __attribute__((overloadable)) add_sat(uint16 x, uint16 y)
{
    ushort c16;
    uint16 r = _mm512_addsetc_epi32(x, y, &c16);
    return _mm512_mask_mov_epi32(r, c16, (uint16)UINT_MAX);
}
uint16 __attribute__((overloadable)) mask_add_sat(ushort m16, uint16 x, uint16 y)
{
    return add_sat(x, y);
}

long8 __attribute__((overloadable)) add_sat(long8 x, long8 y)
{
    long8 r = x + y;
    uchar p8 = _cmpgt_zero(x | y) & _cmplt_zero(r);
    uchar n8 = _cmplt_zero(x & y) & _cmpgt_zero(r);
    return _mm512_mask_mov_epi64(_mm512_mask_mov_epi64(r, p8, (long8)LONG_MAX), n8, (long8)LONG_MIN);
}
long8 __attribute__((overloadable)) mask_add_sat(uchar m8, long8 x, long8 y)
{
    return add_sat(x, y);
}

ulong8 __attribute__((overloadable)) add_sat(ulong8 x, ulong8 y)
{
    uchar c8;
    ulong8 r = _addsetc(x, y, &c8);
    return _mm512_mask_mov_epi64(r, c8, (ulong8)ULONG_MAX);
}
ulong8 __attribute__((overloadable)) mask_add_sat(uchar m8, ulong8 x, ulong8 y)
{
    return add_sat(x, y);
}

// hadd
int16 __attribute__((overloadable)) hadd(int16 x, int16 y)
{
    int16 s, c;
    s = (x ^ y) >> 1;
    c = x & y;
    return s + c;
}
int16 __attribute__((overloadable)) mask_hadd(ushort c16, int16 x, int16 y)
{
    return hadd(x, y);
}

uint16 __attribute__((overloadable)) hadd(uint16 x, uint16 y)
{
    uint16 s, c;
    s = (x ^ y) >> 1;
    c = x & y;
    return s + c;
}
uint16 __attribute__((overloadable)) mask_hadd(ushort m16, uint16 x, uint16 y)
{
    return hadd(x, y);
}

long8 __attribute__((overloadable)) hadd(long8 x, long8 y)
{
    long8 s, c;
    s = (x ^ y) >> 1;
    c = x & y;
    return s + c;
}
long8 __attribute__((overloadable)) mask_hadd(uchar m8, long8 x, long8 y)
{
    return hadd(x, y);
}

ulong8 __attribute__((overloadable)) hadd(ulong8 x, ulong8 y)
{
    ulong8 s, c;
    s = (x ^ y) >> 1;
    c = x & y;
    return s + c;
}
ulong8 __attribute__((overloadable)) mask_hadd(uchar m8, ulong8 x, ulong8 y)
{
    return hadd(x, y);
}

// rhadd
int16 __attribute__((overloadable)) rhadd(int16 x, int16 y)
{
    ushort c16;
    int16 r = _mm512_adc_epi32(x, _MM_K0_REG, y, &c16);
    r = _mm512_srl_pi(r, (int16)1);
    int16 c = _mm512_mask_mov_epi32((int16)0, c16, (int16)0x80000000);
    return _mm512_or_epi32(r, c);
}
int16 __attribute__((overloadable)) mask_rhadd(ushort m16, int16 x, int16 y)
{
    return rhadd(x, y);
}

uint16 __attribute__((overloadable)) rhadd(uint16 x, uint16 y)
{
    return as_uint16(rhadd(as_int16(x), as_int16(y)));
}
uint16 __attribute__((overloadable)) mask_rhadd(ushort m16, uint16 x, uint16 y)
{
    return rhadd(x, y);
}

long8 __attribute__((overloadable)) rhadd(long8 x, long8 y)
{
    long8 s, c;
    s = x ^ y;
    c = (x & y) | (s & 1);
    return (s >> 1) + c;
}
long8 __attribute__((overloadable)) mask_rhadd(uchar m8, long8 x, long8 y)
{
    return rhadd(x, y);
}

ulong8 __attribute__((overloadable)) rhadd(ulong8 x, ulong8 y)
{
    ulong8 s, c;
    s = x ^ y;
    c = (x & y) | (s & 1);
    return (s >> 1) + c;
}
ulong8 __attribute__((overloadable)) mask_rhadd(uchar m8, ulong8 x, ulong8 y)
{
    return rhadd(x, y);
}

// clamp
int16 __attribute__((overloadable)) clamp(int16 x, int16 minval, int16 maxval)
{
    return min(max(x, minval), maxval);
}
int16 __attribute__((overloadable)) mask_clamp(ushort m16, int16 x, int16 minval, int16 maxval)
{
    return clamp(x, minval, maxval);
}

uint16 __attribute__((overloadable)) clamp(uint16 x, uint16 minval, uint16 maxval)
{
    return min(max(x, minval), maxval);
}
uint16 __attribute__((overloadable)) mask_clamp(ushort m16, uint16 x, uint16 minval, uint16 maxval)
{
    return clamp(x, minval, maxval);
}

long8 __attribute__((overloadable)) clamp(long8 x, long8 minval, long8 maxval)
{
    return min(max(x, minval), maxval);
}
long8 __attribute__((overloadable)) mask_clamp(uchar m8, long8 x, long8 minval, long8 maxval)
{
    return clamp(x, minval, maxval);
}

ulong8 __attribute__((overloadable)) clamp(ulong8 x, ulong8 minval, ulong8 maxval)
{
    return min(max(x, minval), maxval);
}
ulong8 __attribute__((overloadable)) mask_clamp(uchar m8, ulong8 x, ulong8 minval, ulong8 maxval)
{
    return clamp(x, minval, maxval);
}

// clz
int16 __attribute__((overloadable)) clz(int16 x)
{
    return as_int16(clz(as_uint16(x)));
}
int16 __attribute__((overloadable)) mask_clz(ushort m16, int16 x)
{
    return clz(x);
}

uint16 __attribute__((overloadable)) clz(uint16 x)
{
    x = _mm512_andnot_epi32(x, x >> 1);
    float16 f = _mm512_cvt_pi2ps(x, _MM_EXPADJ_NONE);
    x = 158 - (as_uint16(f) >> 23);
    return _mm512_min_epi32(x, (uint16)32);
}
uint16 __attribute__((overloadable)) mask_clz(ushort m16, uint16 x)
{
    return clz(x);
}

long8 __attribute__((overloadable)) clz(long8 x)
{
    return as_long8(clz(as_ulong8(x)));
}
long8 __attribute__((overloadable)) mask_clz(uchar m8, long8 x)
{
    return clz(x);
}

ulong8 __attribute__((overloadable)) clz(ulong8 x)
{
    uint16 lz = clz(as_uint16(x));
    ushort m16 = _mm512_mask_cmplt_pi(0xAA, lz, (uint16)32);
    uint16 r = _mm512_mask_add_epi32((uint16)0, 0x55, lz, _mm512_swizzle_epi32(lz, _MM_SWIZ_REG_CDAB));
    return _mm512_mask_mov_epi32(r, m16 >> 1, _mm512_swizzle_epi32(lz, _MM_SWIZ_REG_CDAB));
}
ulong8 __attribute__((overloadable)) mask_clz(uchar m8, ulong8 x)
{
    return clz(x);
}

// mad_hi
int16 __attribute__((overloadable)) mad_hi(int16 x, int16 y, int16 z)
{
    return mul_hi(x, y) + z;
}
int16 __attribute__((overloadable)) mask_mad_hi(ushort m16, int16 x, int16 y, int16 z)
{
    return mad_hi(x, y, z);
}

uint16 __attribute__((overloadable)) mad_hi(uint16 x, uint16 y, uint16 z)
{
    return mul_hi(x, y) + z;
}
uint16 __attribute__((overloadable)) mask_mad_hi(ushort m16, uint16 x, uint16 y, uint16 z)
{
    return mad_hi(x, y, z);
}

long8 __attribute__((overloadable)) mad_hi(long8 x, long8 y, long8 z)
{
    return mul_hi(x, y) + z;
}
long8 __attribute__((overloadable)) mask_mad_hi(uchar m8, long8 x, long8 y, long8 z)
{
    return mad_hi(x, y, z);
}

ulong8 __attribute__((overloadable)) mad_hi(ulong8 x, ulong8 y, ulong8 z)
{
    return mul_hi(x, y) + z;
}
ulong8 __attribute__((overloadable)) mask_mad_hi(uchar m8, ulong8 x, ulong8 y, ulong8 z)
{
    return mad_hi(x, y, z);
}

// mad_sat
int16 __attribute__((overloadable)) mad_sat(int16 x, int16 y, int16 z)
{
    int16 pl = _mm512_mullo_epi32(x, y);
    int16 ph = _mm512_mulhi_epi32(x, y);
    ushort c16;
    int16 s = _mm512_addsetc_epi32(z, pl, &c16);
    ph = _mm512_adc_epi32(ph, c16, (int16)0, &c16);
    ushort p16 = _mm512_cmplt_pi((int16)0, ph) |
                 (_mm512_cmpeq_pi(ph, (int16)0) & _mm512_cmplt_pi(s, (int16)0));
    ushort n16 = _mm512_cmplt_pi(ph, (int16)0) |
                 (_mm512_cmpeq_pi(ph, (int16)-1) & _mm512_cmplt_pi((int16)0, s));
    return _mm512_mask_mov_epi32(_mm512_mask_mov_epi32(s, p16, (int16)INT_MAX), n16, (int16)INT_MIN);
}
int16 __attribute__((overloadable)) mask_mad_sat(ushort m16, int16 x, int16 y, int16 z)
{
    return mad_sat(x, y, z);
}

uint16 __attribute__((overloadable)) mad_sat(uint16 x, uint16 y, uint16 z)
{
    uint16 pl = _mm512_mullo_epi32(x, y);
    uint16 ph = _mm512_mulhi_epu32(x, y);
    ushort c16;
    uint16 s = _mm512_addsetc_epi32(z, pl, &c16);
    ph = _mm512_adc_epi32(ph, c16, (uint16)0, &c16);
    c16 = _mm512_cmplt_pu((uint16)0, ph);
    return _mm512_mask_mov_epi32(s, c16, (uint16)UINT_MAX);
}
uint16 __attribute__((overloadable)) mask_mad_sat(ushort m16, uint16 x, uint16 y, uint16 z)
{
    return mad_sat(x, y, z);
}

long8 __attribute__((overloadable)) mad_sat(long8 x, long8 y, long8 z)
{
    long8 pl = _mm512_mullo_epi64(x, y);
    long8 ph = mul_hi(x, y);
    uchar c8;
    long8 s = _addsetc(z, pl, &c8);
    ph = _adc(ph, c8, (long8)0, &c8);
    uchar p8 = _mm512_cmplt_epi64((long8)0, ph) |
               (_mm512_cmpeq_epi64(ph, (long8)0) & _mm512_cmplt_epi64(s, (long8)0));
    uchar n8 = _mm512_cmplt_epi64(ph, (long8)0) |
               (_mm512_cmpeq_epi64(ph, (long8)-1) & _mm512_cmplt_epi64((long8)0, s));
    return _mm512_mask_mov_epi64(_mm512_mask_mov_epi64(s, p8, (long8)LONG_MAX), n8, (long8)LONG_MIN);
}
long8 __attribute__((overloadable)) mask_mad_sat(uchar m8, long8 x, long8 y, long8 z)
{
    return mad_sat(x, y, z);
}

ulong8 __attribute__((overloadable)) mad_sat(ulong8 x, ulong8 y, ulong8 z)
{
    ulong8 pl = _mm512_mullo_epi64(x, y);
    ulong8 ph = mul_hi(x, y);
    uchar c8;
    ulong8 s = _addsetc(z, pl, &c8);
    ph = _adc(ph, c8, (ulong8)0, &c8);
    c8 = _mm512_cmplt_epu64((ulong8)0, ph);
    return _mm512_mask_mov_epi64(s, c8, (ulong8)ULONG_MAX);
}
ulong8 __attribute__((overloadable)) mask_mad_sat(uchar m8, ulong8 x, ulong8 y, ulong8 z)
{
    return mad_sat(x, y, z);
}

// max
int16 __attribute__((overloadable)) max(int16 x, int16 y)
{
    return _mm512_max_epi32(x, y);
}
int16 __attribute__((overloadable)) mask_max(ushort m16, int16 x, int16 y)
{
    return max(x, y);
}

uint16 __attribute__((overloadable)) max(uint16 x, uint16 y)
{
    return _mm512_max_epu32(x, y);
}
uint16 __attribute__((overloadable)) mask_max(ushort m16, uint16 x, uint16 y)
{
    return max(x, y);
}

long8 __attribute__((overloadable)) max(long8 x, long8 y)
{
    return _mm512_max_epi64(x, y);
}
long8 __attribute__((overloadable)) mask_max(uchar m8, long8 x, long8 y)
{
    return max(x, y);
}

ulong8 __attribute__((overloadable)) max(ulong8 x, ulong8 y)
{
    return _mm512_max_epu64(x, y);
}
ulong8 __attribute__((overloadable)) mask_max(uchar m8, ulong8 x, ulong8 y)
{
    return max(x, y);
}

// min
int16 __attribute__((overloadable)) min(int16 x, int16 y)
{
    return _mm512_min_epi32(x, y);
}
int16 __attribute__((overloadable)) mask_min(ushort m16, int16 x, int16 y)
{
    return min(x, y);
}

uint16 __attribute__((overloadable)) min(uint16 x, uint16 y)
{
    return _mm512_min_epu32(x, y);
}
uint16 __attribute__((overloadable)) mask_min(ushort m16, uint16 x, uint16 y)
{
    return min(x, y);
}

long8 __attribute__((overloadable)) min(long8 x, long8 y)
{
    return _mm512_min_epi64(x, y);
}
long8 __attribute__((overloadable)) mask_min(uchar m8, long8 x, long8 y)
{
    return min(x, y);
}

ulong8 __attribute__((overloadable)) min(ulong8 x, ulong8 y)
{
    return _mm512_min_epu64(x, y);
}
ulong8 __attribute__((overloadable)) mask_min(uchar m8, ulong8 x, ulong8 y)
{
    return min(x, y);
}

// mul_hi
int16 __attribute__((overloadable)) mul_hi(int16 x, int16 y)
{
    return _mm512_mulhi_epi32(x, y);
}
int16 __attribute__((overloadable)) mask_mul_hi(ushort m16, int16 x, int16 y)
{
    return mul_hi(x, y);
}

uint16 __attribute__((overloadable)) mul_hi(uint16 x, uint16 y)
{
    return _mm512_mulhi_epu32(x, y);
}
uint16 __attribute__((overloadable)) mask_mul_hi(ushort m16, uint16 x, uint16 y)
{
    return mul_hi(x, y);
}

long8 __attribute__((overloadable)) mul_hi(long8 x, long8 y)
{
    // x = x_h, x_l; y = y_h, y_l
    // p0_h = H(x_l * y_l); p0_l = L(x_l * y_l)
    // p3_h = H(x_h * y_h); p3_l = L(x_h * y_h)
    // p1_h = H(x_h * y_l); p1_l = L(x_h * y_l)
    // p2_h = H(x_l * y_h); p2_l = L(x_l * y_h)
    //
    //             p0_h, p0_l
    //       p1_h, p1_l
    //       p2_h, p2_l
    // p3_h, p3_l
    // ----------------------
    // hi = ((p0_h + p1_l + p2_l) + ((p1_h + p2_h + p3_l) << 32) + (p3_h << 64)) >> 32
    int16 p30l = _mm512_mullo_epi32(x, y);
    int16 p30h = _mm512_mulhi_epi32(x, y);
    int16 p21l = _mm512_mullo_epi32(x, _mm512_swizzle_epi32(y, _MM_SWIZ_REG_CDAB));
    int16 p21h = _mm512_mulhi_epi32(x, _mm512_swizzle_epi32(y, _MM_SWIZ_REG_CDAB));
    ushort c0, c1;
    int16 ah, al;
    c0 = c1 = 0;
    // = p1_l + p0_h
    al = _mm512_mask_addsetc_epi32(p30h, 0x55, c0, p21l, &c0);
    // = p2_l + a0
    ah = _mm512_mask_addsetc_epi32(al, 0x55, c1, _mm512_swizzle_epi32(p21l, _MM_SWIZ_REG_CDAB), &c1);
    // al = p2_h + p3_l + c0
    al = _mm512_mask_adc_epi32(p30l, 0xAA, c0 << 1, p21h, &c0);
    // al = p1_h + al + c1
    al = _mm512_mask_adc_epi32(al, 0xAA, c1 << 1, _mm512_swizzle_epi32(p21h, _MM_SWIZ_REG_CDAB), &c1);
    // ah = p3_h + c0 + c1
    ah = _mm512_mask_adc_epi32(p30h, 0xAA, c0, (int16)0, &c0);
    ah = _mm512_mask_adc_epi32(ah, 0xAA, c1, (int16)0, &c1);
    // (ah, al)
    return _mm512_mask_mov_epi32(ah, 0x55, _mm512_swizzle_epi32(al, _MM_SWIZ_REG_CDAB));
}
long8 __attribute__((overloadable)) mask_mul_hi(uchar m8, long8 x, long8 y)
{
    return mul_hi(x, y);
}

ulong8 __attribute__((overloadable)) mul_hi(ulong8 x, ulong8 y)
{
    // x = x_h, x_l; y = y_h, y_l
    // p0_h = H(x_l * y_l); p0_l = L(x_l * y_l)
    // p3_h = H(x_h * y_h); p3_l = L(x_h * y_h)
    // p1_h = H(x_h * y_l); p1_l = L(x_h * y_l)
    // p2_h = H(x_l * y_h); p2_l = L(x_l * y_h)
    //
    //             p0_h, p0_l
    //       p1_h, p1_l
    //       p2_h, p2_l
    // p3_h, p3_l
    // ----------------------
    // hi = ((p0_h + p1_l + p2_l) + ((p1_h + p2_h + p3_l) << 32) + (p3_h << 64)) >> 32
    uint16 p30l = _mm512_mullo_epi32(x, y);
    uint16 p30h = _mm512_mulhi_epu32(x, y);
    uint16 p21l = _mm512_mullo_epi32(x, _mm512_swizzle_epi32(y, _MM_SWIZ_REG_CDAB));
    uint16 p21h = _mm512_mulhi_epu32(x, _mm512_swizzle_epi32(y, _MM_SWIZ_REG_CDAB));
    ushort c0, c1;
    uint16 ah, al;
    c0 = c1 = 0;
    // = p1_l + p0_h
    al = _mm512_mask_addsetc_epi32(p30h, 0x55, c0, p21l, &c0);
    // = p2_l + a0
    ah = _mm512_mask_addsetc_epi32(al, 0x55, c1, _mm512_swizzle_epi32(p21l, _MM_SWIZ_REG_CDAB), &c1);
    // al = p2_h + p3_l + c0
    al = _mm512_mask_adc_epi32(p30l, 0xAA, c0 << 1, p21h, &c0);
    // al = p1_h + al + c1
    al = _mm512_mask_adc_epi32(al, 0xAA, c1 << 1, _mm512_swizzle_epi32(p21h, _MM_SWIZ_REG_CDAB), &c1);
    // ah = p3_h + c0 + c1
    ah = _mm512_mask_adc_epi32(p30h, 0xAA, c0, (uint16)0, &c0);
    ah = _mm512_mask_adc_epi32(ah, 0xAA, c1, (uint16)0, &c1);
    // (ah, al)
    return _mm512_mask_mov_epi32(ah, 0x55, _mm512_swizzle_epi32(al, _MM_SWIZ_REG_CDAB));
}
ulong8 __attribute__((overloadable)) mask_mul_hi(uchar m8, ulong8 x, ulong8 y)
{
    return mul_hi(x, y);
}

// rotate
int16 __attribute__((overloadable)) rotate(int16 x, int16 y)
{
    return _mm512_sll_pi(x, y) | _mm512_srl_pi(x, (int16)32 - y);
}
int16 __attribute__((overloadable)) mask_rotate(ushort m16, int16 x, int16 y)
{
    return rotate(x, y);
}

uint16 __attribute__((overloadable)) rotate(uint16 x, uint16 y)
{
    return as_uint16(rotate(as_int16(x), as_int16(y)));
}
uint16 __attribute__((overloadable)) mask_rotate(ushort m16, uint16 x, uint16 y)
{
    return rotate(x, y);
}

long8 __attribute__((overloadable)) rotate(long8 x, long8 y)
{
    return _mm512_sll_epi64(x, y) | _mm512_srl_epi64(x, (int16)64 - y);
}
long8 __attribute__((overloadable)) mask_rotate(uchar m8, long8 x, long8 y)
{
    return rotate(x, y);
}

ulong8 __attribute__((overloadable)) rotate(ulong8 x, ulong8 y)
{
    return as_ulong8(rotate(as_long8(x), as_long8(y)));
}
ulong8 __attribute__((overloadable)) mask_rotate(uchar m8, ulong8 x, ulong8 y)
{
    return rotate(x, y);
}

// sub_sat
int16 __attribute__((overloadable)) sub_sat(int16 x, int16 y)
{
    return add_sat(x, -y);
}
int16 __attribute__((overloadable)) mask_sub_sat(ushort m16, int16 x, int16 y)
{
    return sub_sat(x, y);
}

uint16 __attribute__((overloadable)) sub_sat(uint16 x, uint16 y)
{
    ushort b16;
    uint16 r = _mm512_subsetb_epi32(x, y, &b16);
    return _mm512_mask_mov_epi32(r, b16, (uint16)0);
}
uint16 __attribute__((overloadable)) mask_sub_sat(ushort m16, uint16 x, uint16 y)
{
    return sub_sat(x, y);
}

long8 __attribute__((overloadable)) sub_sat(long8 x, long8 y)
{
    return add_sat(x, -y);
}
long8 __attribute__((overloadable)) mask_sub_sat(uchar m8, long8 x, long8 y)
{
    return sub_sat(x, y);
}

ulong8 __attribute__((overloadable)) sub_sat(ulong8 x, ulong8 y)
{
    ushort b16;
    uint16 r = _mm512_mask_subsetb_epi32(x, 0x55, b16, y, &b16);
    r = _mm512_mask_sbb_epi32(r, 0xAA, b16 << 1, y, &b16);
    uchar b8 = _mask16z8(b16);
    return _mm512_mask_mov_epi64(r, b8, (ulong8)0);
}
ulong8 __attribute__((overloadable)) mask_sub_sat(uchar m8, ulong8 x, ulong8 y)
{
    return sub_sat(x, y);
}

// upsample
long8 __attribute__((overloadable)) upsample(int8 hi, int8 lo)
{
    int16 val;
    val.hi = hi;
    val.lo = lo;
    // Note that both KNF & KNC are in-order core without register renaming.
    // The following instructions are independent as their mask values have no
    // overlap and hence has the shorter latency.
    long8 r = _mm512_undefined_epi64();
    r = _mm512_mask_shuf128x32(r, 0x0A0A, val, _MM_PERM_DDCC, _MM_PERM_BDAC);
    r = _mm512_mask_shuf128x32(r, 0xA0A0, val, _MM_PERM_DDCC, _MM_PERM_DBCA);
    r = _mm512_mask_shuf128x32(r, 0x0505, val, _MM_PERM_BBAA, _MM_PERM_DBCA);
    r = _mm512_mask_shuf128x32(r, 0x5050, val, _MM_PERM_BBAA, _MM_PERM_BDAC);
    return r;
}
long8 __attribute__((overloadable)) mask_upsample(uchar m8, int8 hi, int8 lo)
{
    return upsample(hi, lo);
}

ulong8 __attribute__((overloadable)) upsample(uint8 hi, uint8 lo)
{
    return as_ulong8(upsample(as_int8(hi), as_int8(lo)));
}
ulong8 __attribute__((overloadable)) mask_upsample(uchar m8, uint8 hi, uint8 lo)
{
    return upsample(hi, lo);
}

// popcount
int16 __attribute__((overloadable)) popcount(int16 x)
{
    return mask_popcount(0xFFFF, x);
}
int16 __attribute__((overloadable)) mask_popcount(ushort m16, int16 x)
{
    return as_int16(mask_popcount(m16, as_uint16(x)));
}

uint16 __attribute__((overloadable)) popcount(uint16 x)
{
    return mask_popcount(0xFFFF, x);
}
uint16 __attribute__((overloadable)) mask_popcount(ushort m16, uint16 x)
{
    uint t[16] __attribute__((aligned(64)));

    // The following 'switch' will be removed during incluing this function
    // with constant mask. As a result, the final builtin on narrow vector
    // won't have branches.
    _mm512_mask_stored(&t[0], m16, x, _MM_DOWNC_NONE, _MM_SUBSET32_16, _MM_HINT_NONE);
    switch (m16) {
        case 0xFFFF:
            t[15] = _mm_countbits_32(t[15]);
            t[14] = _mm_countbits_32(t[14]);
            t[13] = _mm_countbits_32(t[13]);
            t[12] = _mm_countbits_32(t[12]);
            t[11] = _mm_countbits_32(t[11]);
            t[10] = _mm_countbits_32(t[10]);
            t[ 9] = _mm_countbits_32(t[ 9]);
            t[ 8] = _mm_countbits_32(t[ 8]);
        case 0x00FF:
            t[ 7] = _mm_countbits_32(t[ 7]);
            t[ 6] = _mm_countbits_32(t[ 6]);
            t[ 5] = _mm_countbits_32(t[ 5]);
            t[ 4] = _mm_countbits_32(t[ 4]);
        case 0x000F:
            t[ 3] = _mm_countbits_32(t[ 3]);
        case 0x0007:
            t[ 2] = _mm_countbits_32(t[ 2]);
        case 0x0003:
            t[ 1] = _mm_countbits_32(t[ 1]);
        case 0x0001:
            t[ 0] = _mm_countbits_32(t[ 0]);
        default:
            break;
    }
    return _mm512_loadd(&t[0], _MM_FULLUPC_NONE, _MM_BROADCAST32_NONE, _MM_HINT_NONE);
}

long8 __attribute__((overloadable)) popcount(long8 x)
{
    return mask_popcount(0xFF, x);
}
long8 __attribute__((overloadable)) mask_popcount(uchar m8, long8 x)
{
    return as_long8(mask_popcount(m8, as_ulong8(x)));
}

ulong8 __attribute__((overloadable)) popcount(ulong8 x)
{
    return mask_popcount(0xFF, x);
}
ulong8 __attribute__((overloadable)) mask_popcount(uchar m8, ulong8 x)
{
    ulong t[8] __attribute__((aligned(64)));

    // The following 'switch' will be removed during incluing this function
    // with constant mask. As a result, the final builtin on narrow vector
    // won't have branches.
    _mm512_mask_storeq(&t[0], m8, x, _MM_DOWNC64_NONE, _MM_SUBSET64_8, _MM_HINT_NONE);
    switch (m8) {
        case 0xFF:
            t[7] = _mm_countbits_64(t[7]);
            t[6] = _mm_countbits_64(t[6]);
            t[5] = _mm_countbits_64(t[5]);
            t[4] = _mm_countbits_64(t[4]);
        case 0x0F:
            t[3] = _mm_countbits_64(t[3]);
        case 0x07:
            t[2] = _mm_countbits_64(t[2]);
        case 0x03:
            t[1] = _mm_countbits_64(t[1]);
        case 0x01:
            t[0] = _mm_countbits_64(t[0]);
        default:
            break;
    }
    return _mm512_loadq(&t[0], _MM_FULLUPC64_NONE, _MM_BROADCAST64_NONE, _MM_HINT_NONE);
}

// mad24
int16 __attribute__((overloadable)) mad24(int16 x, int16 y, int16 z)
{
    return _mm512_madd231_pi(z, x, y);
}
int16 __attribute__((overloadable)) mask_mad24(ushort m16, int16 x, int16 y, int16 z)
{
    return mad24(x, y, z);
}

uint16 __attribute__((overloadable)) mad24(uint16 x, uint16 y, uint16 z)
{
    return _mm512_madd231_pi(z, x, y);
}
uint16 __attribute__((overloadable)) mask_mad24(ushort m16, uint16 x, uint16 y, uint16 z)
{
    return mad24(x, y, z);
}

// mul24
int16 __attribute__((overloadable)) mul24(int16 x, int16 y)
{
    return _mm512_mullo_epi32(x, y);
}
int16 __attribute__((overloadable)) mask_mul24(ushort m16, int16 x, int16 y)
{
    return mul24(x, y);
}

uint16 __attribute__((overloadable)) mul24(uint16 x, uint16 y)
{
    return _mm512_mullo_epi32(x, y);
}
uint16 __attribute__((overloadable)) mask_mul24(ushort m16, uint16 x, uint16 y)
{
    return mul24(x, y);
}

#ifdef __cplusplus
}
#endif

#endif // defined (__MIC__) || defined(__MIC2__)
