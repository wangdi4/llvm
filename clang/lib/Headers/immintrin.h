//
// Copyright (C) 2015 Intel Corporation.  All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
/*===---- immintrin.h - Intel intrinsics -----------------------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __IMMINTRIN_H
#define __IMMINTRIN_H

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__MMX__)
#include <mmintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SSE__)
#include <xmmintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SSE2__)
#include <emmintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SSE3__)
#include <pmmintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SSSE3__)
#include <tmmintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__SSE4_2__) || defined(__SSE4_1__))
#include <smmintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AES__) || defined(__PCLMUL__))
#include <wmmintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__CLFLUSHOPT__)
#include <clflushoptintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX__)
#include <avxintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX2__)
#include <avx2intrin.h>

/* The 256-bit versions of functions in f16cintrin.h.
   Intel documents these as being in immintrin.h, and
   they depend on typedefs from avxintrin.h. */

/// \brief Converts a 256-bit vector of [8 x float] into a 128-bit vector
///    containing 16-bit half-precision float values.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m128i _mm256_cvtps_ph(__m256 a, const int imm);
/// \endcode
///
/// This intrinsic corresponds to the <c> VCVTPS2PH </c> instruction.
///
/// \param a
///    A 256-bit vector containing 32-bit single-precision float values to be
///    converted to 16-bit half-precision float values.
/// \param imm
///    An immediate value controlling rounding using bits [2:0]: \n
///    000: Nearest \n
///    001: Down \n
///    010: Up \n
///    011: Truncate \n
///    1XX: Use MXCSR.RC for rounding
/// \returns A 128-bit vector containing the converted 16-bit half-precision
///    float values.
#define _mm256_cvtps_ph(a, imm) __extension__ ({ \
 (__m128i)__builtin_ia32_vcvtps2ph256((__v8sf)(__m256)(a), (imm)); })

/// \brief Converts a 128-bit vector containing 16-bit half-precision float
///    values into a 256-bit vector of [8 x float].
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the <c> VCVTPH2PS </c> instruction.
///
/// \param __a
///    A 128-bit vector containing 16-bit half-precision float values to be
///    converted to 32-bit single-precision float values.
/// \returns A vector of [8 x float] containing the converted 32-bit
///    single-precision float values.
static __inline __m256 __attribute__((__always_inline__, __nodebug__, __target__("f16c")))
_mm256_cvtph_ps(__m128i __a)
{
  return (__m256)__builtin_ia32_vcvtph2ps256((__v8hi)__a);
}
#endif /* __AVX2__ */

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__BMI__)
#include <bmiintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__BMI2__)
#include <bmi2intrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__LZCNT__)
#include <lzcntintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__FMA__)
#include <fmaintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512F__)
#include <avx512fintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512VL__)
#include <avx512vlintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512BW__)
#include <avx512bwintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512CD__)
#include <avx512cdintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512VPOPCNTDQ__)
#include <avx512vpopcntdqintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512DQ__)
#include <avx512dqintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VL__) && defined(__AVX512BW__))
#include <avx512vlbwintrin.h>
#endif

#ifndef __INTEL_COMPILER
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VL__) && defined(__AVX512CD__))
#include <avx512vlcdintrin.h>
#endif
#endif /* __INTEL_COMPILER */

#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VL__) && defined(__AVX512DQ__))
#include <avx512vldqintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512ER__)
#include <avx512erintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512IFMA__)
#include <avx512ifmaintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512IFMA__) && defined(__AVX512VL__))
#include <avx512ifmavlintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512VBMI__)
#include <avx512vbmiintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VBMI__) && defined(__AVX512VL__))
#include <avx512vbmivlintrin.h>
#endif

#ifndef __INTEL_COMPILER
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512PF__)
#include <avx512pfintrin.h>
#endif
#endif /* __INTEL_COMPILER */

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__PKU__)
#include <pkuintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__RDRND__)
static __inline__ int __attribute__((__always_inline__, __nodebug__, __target__("rdrnd")))
_rdrand16_step(unsigned short *__p)
{
  return __builtin_ia32_rdrand16_step(__p);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__, __target__("rdrnd")))
_rdrand32_step(unsigned int *__p)
{
  return __builtin_ia32_rdrand32_step(__p);
}

/* __bit_scan_forward */
static __inline__ int __attribute__((__always_inline__, __nodebug__))
_bit_scan_forward(int __A) {
  return __builtin_ctz(__A);
}

/* __bit_scan_reverse */
static __inline__ int __attribute__((__always_inline__, __nodebug__))
_bit_scan_reverse(int __A) {
  return 31 - __builtin_clz(__A);
}

#ifdef __x86_64__
static __inline__ int __attribute__((__always_inline__, __nodebug__, __target__("rdrnd")))
_rdrand64_step(unsigned long long *__p)
{
  return __builtin_ia32_rdrand64_step(__p);
}
#endif
#endif /* __RDRND__ */

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__FSGSBASE__)
#ifdef __x86_64__
static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_readfsbase_u32(void)
{
  return __builtin_ia32_rdfsbase32();
}

static __inline__ unsigned long long __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_readfsbase_u64(void)
{
  return __builtin_ia32_rdfsbase64();
}

static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_readgsbase_u32(void)
{
  return __builtin_ia32_rdgsbase32();
}

static __inline__ unsigned long long __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_readgsbase_u64(void)
{
  return __builtin_ia32_rdgsbase64();
}

static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_writefsbase_u32(unsigned int __V)
{
  return __builtin_ia32_wrfsbase32(__V);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_writefsbase_u64(unsigned long long __V)
{
  return __builtin_ia32_wrfsbase64(__V);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_writegsbase_u32(unsigned int __V)
{
  return __builtin_ia32_wrgsbase32(__V);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_writegsbase_u64(unsigned long long __V)
{
  return __builtin_ia32_wrgsbase64(__V);
}

#endif
#endif /* __FSGSBASE__ */

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__RTM__)
#include <rtmintrin.h>
#include <xtestintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SHA__)
#include <shaintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__FXSR__)
#include <fxsrintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__XSAVE__)
#include <xsaveintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__XSAVEOPT__)
#include <xsaveoptintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__XSAVEC__)
#include <xsavecintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__XSAVES__)
#include <xsavesintrin.h>
#endif

/* Some intrinsics inside adxintrin.h are available only on processors with ADX,
 * whereas others are also available at all times. */
#include <adxintrin.h>

/* Definitions of feature list to be used by feature select intrinsics */
#define _FEATURE_GENERIC_IA32        0x00000001ULL
#define _FEATURE_FPU                 0x00000002ULL
#define _FEATURE_CMOV                0x00000004ULL
#define _FEATURE_MMX                 0x00000008ULL
#define _FEATURE_FXSAVE              0x00000010ULL
#define _FEATURE_SSE                 0x00000020ULL
#define _FEATURE_SSE2                0x00000040ULL
#define _FEATURE_SSE3                0x00000080ULL
#define _FEATURE_SSSE3               0x00000100ULL
#define _FEATURE_SSE4_1              0x00000200ULL
#define _FEATURE_SSE4_2              0x00000400ULL
#define _FEATURE_MOVBE               0x00000800ULL
#define _FEATURE_POPCNT              0x00001000ULL
#define _FEATURE_PCLMULQDQ           0x00002000ULL
#define _FEATURE_AES                 0x00004000ULL
#define _FEATURE_F16C                0x00008000ULL
#define _FEATURE_AVX                 0x00010000ULL
#define _FEATURE_RDRND               0x00020000ULL
#define _FEATURE_FMA                 0x00040000ULL
#define _FEATURE_BMI                 0x00080000ULL
#define _FEATURE_LZCNT               0x00100000ULL
#define _FEATURE_HLE                 0x00200000ULL
#define _FEATURE_RTM                 0x00400000ULL
#define _FEATURE_AVX2                0x00800000ULL
#define _FEATURE_AVX512DQ            0x01000000ULL
#define _FEATURE_KNCNI               0x04000000ULL
#define _FEATURE_AVX512F             0x08000000ULL
#define _FEATURE_ADX                 0x10000000ULL
#define _FEATURE_RDSEED              0x20000000ULL
#define _FEATURE_AVX512IFMA52        0x40000000ULL
#define _FEATURE_AVX512ER            0x100000000ULL
#define _FEATURE_AVX512PF            0x200000000ULL
#define _FEATURE_AVX512CD            0x400000000ULL
#define _FEATURE_SHA                 0x800000000ULL
#define _FEATURE_MPX                 0x1000000000ULL
#define _FEATURE_AVX512BW            0x2000000000ULL
#define _FEATURE_AVX512VL            0x4000000000ULL
#define _FEATURE_AVX512VBMI          0x8000000000ULL
#define _FEATURE_CLFLUSHOPT          0x10000000000ULL

#endif /* __IMMINTRIN_H */
