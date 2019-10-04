/*===---- immintrin.h - Intel intrinsics -----------------------------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __IMMINTRIN_H
#define __IMMINTRIN_H

/* INTEL_CUSTOMIZATION */
#include <ia32intrin.h>
/* end INTEL_CUSTOMIZATION */

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__MMX__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <mmintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SSE__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <xmmintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SSE2__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <emmintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SSE3__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <pmmintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SSSE3__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <tmmintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__SSE4_2__) || defined(__SSE4_1__)) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <smmintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AES__) || defined(__PCLMUL__)) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <wmmintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__CLFLUSHOPT__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <clflushoptintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__CLWB__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <clwbintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avxintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX2__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx2intrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__F16C__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <f16cintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__VPCLMULQDQ__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <vpclmulqdqintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__BMI__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <bmiintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__BMI2__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <bmi2intrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__LZCNT__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <lzcntintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__POPCNT__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <popcntintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__FMA__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <fmaintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512F__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512fintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512VL__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vlintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512BW__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512bwintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512BITALG__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512bitalgintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512CD__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512cdintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512VPOPCNTDQ__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vpopcntdqintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VL__) && defined(__AVX512VPOPCNTDQ__)) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vpopcntdqvlintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512VNNI__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vnniintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ISA_AVX_VNNI */
#if defined(__AVXVNNI__)
#include <avxvnni/avxvnniintrin.h>
#else
/* end INTEL_FEATURE_ISA_AVX_VNNI */
/* end INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VL__) && defined(__AVX512VNNI__)) || defined(__M_INTRINSIC_PROMOTE__)
#include <avx512vlvnniintrin.h>
#endif
/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ISA_AVX_VNNI */
#endif
/* end INTEL_FEATURE_ISA_AVX_VNNI */
/* end INTEL_CUSTOMIZATION */

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512DQ__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512dqintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VL__) && defined(__AVX512BITALG__)) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vlbitalgintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VL__) && defined(__AVX512BW__)) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vlbwintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VL__) && defined(__AVX512CD__)) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vlcdintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VL__) && defined(__AVX512DQ__)) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vldqintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512ER__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512erintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512IFMA__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512ifmaintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512IFMA__) && defined(__AVX512VL__)) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512ifmavlintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512VBMI__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vbmiintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VBMI__) && defined(__AVX512VL__)) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vbmivlintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512VBMI2__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vbmi2intrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VBMI2__) && defined(__AVX512VL__)) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vlvbmi2intrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512PF__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512pfintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ISA_FP16 */
/*
 * FIXME: _Float16 type is legal only when HW support float16 operation.
 * We use __AVX512FP16__ to identify if float16 is supported or not, so
 * when float16 is not supported, the related header is not included.
 *
 */
#if defined(__AVX512FP16__)
#include <avx512fp16intrin.h>
#endif

#if defined(__AVX512FP16__) && defined(__AVX512VL__)
#include <avx512vlfp16intrin.h>
#endif
/* end INTEL_FEATURE_ISA_FP16 */
/* end INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AVX512BF16__) || defined(__M_INTRINSIC_PROMOTE__)
#include <avx512bf16intrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
    (defined(__AVX512VL__) && defined(__AVX512BF16__)) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vlbf16intrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__PKU__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <pkuintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__VAES__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <vaesintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__GFNI__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <gfniintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__RDPID__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
/// Returns the value of the IA32_TSC_AUX MSR (0xc0000103).
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> RDPID </c> instruction.
static __inline__ unsigned int __attribute__((__always_inline__, __nodebug__, __target__("rdpid")))
_rdpid_u32(void) {
  return __builtin_ia32_rdpid();
}
#endif // __RDPID__

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__RDRND__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
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

#ifdef __x86_64__
static __inline__ int __attribute__((__always_inline__, __nodebug__, __target__("rdrnd")))
_rdrand64_step(unsigned long long *__p)
{
  return __builtin_ia32_rdrand64_step(__p);
}
#endif
#endif /* __RDRND__ */

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__FSGSBASE__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
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
  __builtin_ia32_wrfsbase32(__V);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_writefsbase_u64(unsigned long long __V)
{
  __builtin_ia32_wrfsbase64(__V);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_writegsbase_u32(unsigned int __V)
{
  __builtin_ia32_wrgsbase32(__V);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("fsgsbase")))
_writegsbase_u64(unsigned long long __V)
{
  __builtin_ia32_wrgsbase64(__V);
}

#endif
#endif /* __FSGSBASE__ */

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__MOVBE__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */

/* The structs used below are to force the load/store to be unaligned. This
 * is accomplished with the __packed__ attribute. The __may_alias__ prevents
 * tbaa metadata from being generated based on the struct and the type of the
 * field inside of it.
 */

static __inline__ short __attribute__((__always_inline__, __nodebug__, __target__("movbe")))
_loadbe_i16(void const * __P) {
  struct __loadu_i16 {
    short __v;
  } __attribute__((__packed__, __may_alias__));
  return __builtin_bswap16(((struct __loadu_i16*)__P)->__v);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("movbe")))
_storebe_i16(void * __P, short __D) {
  struct __storeu_i16 {
    short __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __storeu_i16*)__P)->__v = __builtin_bswap16(__D);
}

static __inline__ int __attribute__((__always_inline__, __nodebug__, __target__("movbe")))
_loadbe_i32(void const * __P) {
  struct __loadu_i32 {
    int __v;
  } __attribute__((__packed__, __may_alias__));
  return __builtin_bswap32(((struct __loadu_i32*)__P)->__v);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("movbe")))
_storebe_i32(void * __P, int __D) {
  struct __storeu_i32 {
    int __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __storeu_i32*)__P)->__v = __builtin_bswap32(__D);
}

#ifdef __x86_64__
static __inline__ long long __attribute__((__always_inline__, __nodebug__, __target__("movbe")))
_loadbe_i64(void const * __P) {
  struct __loadu_i64 {
    long long __v;
  } __attribute__((__packed__, __may_alias__));
  return __builtin_bswap64(((struct __loadu_i64*)__P)->__v);
}

static __inline__ void __attribute__((__always_inline__, __nodebug__, __target__("movbe")))
_storebe_i64(void * __P, long long __D) {
  struct __storeu_i64 {
    long long __v;
  } __attribute__((__packed__, __may_alias__));
  ((struct __storeu_i64*)__P)->__v = __builtin_bswap64(__D);
}
#endif
#endif /* __MOVBE */

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__RTM__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <rtmintrin.h>
#include <xtestintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SHA__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <shaintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__FXSR__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <fxsrintrin.h>
#endif

/* No feature check desired due to internal MSC_VER checks */
#include <xsaveintrin.h>

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__XSAVEOPT__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <xsaveoptintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__XSAVEC__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <xsavecintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__XSAVES__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <xsavesintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SHSTK__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <cetintrin.h>
#endif

/* Some intrinsics inside adxintrin.h are available only on processors with ADX,
 * whereas others are also available at all times. */
#include <adxintrin.h>

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__RDSEED__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <rdseedintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__WBNOINVD__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <wbnoinvdintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__CLDEMOTE__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <cldemoteintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__WAITPKG__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <waitpkgintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
  defined(__MOVDIRI__) || defined(__MOVDIR64B__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <movdirintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__PCONFIG__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <pconfigintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SGX__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <sgxintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__PTWRITE__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <ptwriteintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__INVPCID__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <invpcidintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ICECODE */
#if defined(__ICECODE__)
#include <icecode/ceintrin.h>
#endif
/* end INTEL_FEATURE_ICECODE */
/* end INTEL_CUSTOMIZATION */

/* INTEL_CUSTOMIZATION */

/* INTEL_FEATURE_ISA_ULI */
#if defined(__ULI_SUPPORTED__)
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__ULI__) || defined(__M_INTRINSIC_PROMOTE__)
#include <uliintrin.h>
#endif
#endif
/* end INTEL_FEATURE_ISA_ULI */

/* INTEL_FEATURE_ISA_SERIALIZE */
#if defined(__SERIALIZE_SUPPORTED__)
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SERIALIZE__) || defined(__M_INTRINSIC_PROMOTE__)
#include <serializeintrin.h>
#endif
#endif
/* end INTEL_FEATURE_ISA_SERIALIZE */

/* INTEL_FEATURE_ISA_TSXLDTRK */
#if defined(__TSXLDTRK_SUPPORTED__)
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__TSXLDTRK__) || defined(__M_INTRINSIC_PROMOTE__)
#include <tsxldtrkintrin.h>
#endif
#endif
/* end INTEL_FEATURE_ISA_TSXLDTRK */

/* INTEL_FEATURE_ISA_AMX */
#if defined(__AMXTILE_SUPPORTED__) || defined(__AMXINT8_SUPPORTED__) || defined(__AMXBF16_SUPPORTED__)
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AMXTILE__) || defined(__AMXINT8__) || defined(__AMXBF16__) || defined(__M_INTRINSIC_PROMOTE__)
#include <Intel_amxintrin.h>
#endif
#endif
/* end INTEL_FEATURE_ISA_AMX */

/* INTEL_FEATURE_ISA_AMX2 */
#if defined(__AMX2TILE_SUPPORTED__) || defined(__AMXINT8_SUPPORTED__) || defined(__AMXBF16_SUPPORTED__)
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__AMX2TILE__) || defined(__AMXINT8__) || defined(__AMXBF16__) || defined(__M_INTRINSIC_PROMOTE__)
// TODO: when AMX2 clang part is finished, fix here.
#include <Intel_amx2intrin.h>
#endif
#endif
/* end INTEL_FEATURE_ISA_AMX2 */

/* INTEL_FEATURE_ISA_KEYLOCKER */
#if defined(__KEYLOCKER_SUPPORTED__)
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__KEYLOCKER__) || defined(__M_INTRINSIC_PROMOTE__)
#include <keylockerintrin.h>
#endif
#endif
/* end INTEL_FEATURE_ISA_KEYLOCKER */
/* end INTEL_CUSTOMIZATION */

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
  defined(__AVX512VP2INTERSECT__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vp2intersectintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || \
  (defined(__AVX512VL__) && defined(__AVX512VP2INTERSECT__)) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <avx512vlvp2intersectintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__ENQCMD__)
#include <enqcmdintrin.h>
#endif

#if defined(_MSC_VER) && __has_extension(gnu_asm)
/* Define the default attributes for these intrinsics */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__))
#ifdef __cplusplus
extern "C" {
#endif
/*----------------------------------------------------------------------------*\
|* Interlocked Exchange HLE
\*----------------------------------------------------------------------------*/
#if defined(__i386__) || defined(__x86_64__)
static __inline__ long __DEFAULT_FN_ATTRS
_InterlockedExchange_HLEAcquire(long volatile *_Target, long _Value) {
  __asm__ __volatile__(".byte 0xf2 ; lock ; xchg %0, %1"
                       : "+r" (_Value), "+m" (*_Target) :: "memory");
  return _Value;
}
static __inline__ long __DEFAULT_FN_ATTRS
_InterlockedExchange_HLERelease(long volatile *_Target, long _Value) {
  __asm__ __volatile__(".byte 0xf3 ; lock ; xchg %0, %1"
                       : "+r" (_Value), "+m" (*_Target) :: "memory");
  return _Value;
}
#endif
#if defined(__x86_64__)
static __inline__ __int64 __DEFAULT_FN_ATTRS
_InterlockedExchange64_HLEAcquire(__int64 volatile *_Target, __int64 _Value) {
  __asm__ __volatile__(".byte 0xf2 ; lock ; xchg %0, %1"
                       : "+r" (_Value), "+m" (*_Target) :: "memory");
  return _Value;
}
static __inline__ __int64 __DEFAULT_FN_ATTRS
_InterlockedExchange64_HLERelease(__int64 volatile *_Target, __int64 _Value) {
  __asm__ __volatile__(".byte 0xf3 ; lock ; xchg %0, %1"
                       : "+r" (_Value), "+m" (*_Target) :: "memory");
  return _Value;
}
#endif
/*----------------------------------------------------------------------------*\
|* Interlocked Compare Exchange HLE
\*----------------------------------------------------------------------------*/
#if defined(__i386__) || defined(__x86_64__)
static __inline__ long __DEFAULT_FN_ATTRS
_InterlockedCompareExchange_HLEAcquire(long volatile *_Destination,
                              long _Exchange, long _Comparand) {
  __asm__ __volatile__(".byte 0xf2 ; lock ; cmpxchg %2, %1"
                       : "+a" (_Comparand), "+m" (*_Destination)
                       : "r" (_Exchange) : "memory");
  return _Comparand;
}
static __inline__ long __DEFAULT_FN_ATTRS
_InterlockedCompareExchange_HLERelease(long volatile *_Destination,
                              long _Exchange, long _Comparand) {
  __asm__ __volatile__(".byte 0xf3 ; lock ; cmpxchg %2, %1"
                       : "+a" (_Comparand), "+m" (*_Destination)
                       : "r" (_Exchange) : "memory");
  return _Comparand;
}
#endif
#if defined(__x86_64__)
static __inline__ __int64 __DEFAULT_FN_ATTRS
_InterlockedCompareExchange64_HLEAcquire(__int64 volatile *_Destination,
                              __int64 _Exchange, __int64 _Comparand) {
  __asm__ __volatile__(".byte 0xf2 ; lock ; cmpxchg %2, %1"
                       : "+a" (_Comparand), "+m" (*_Destination)
                       : "r" (_Exchange) : "memory");
  return _Comparand;
}
static __inline__ __int64 __DEFAULT_FN_ATTRS
_InterlockedCompareExchange64_HLERelease(__int64 volatile *_Destination,
                              __int64 _Exchange, __int64 _Comparand) {
  __asm__ __volatile__(".byte 0xf3 ; lock ; cmpxchg %2, %1"
                       : "+a" (_Comparand), "+m" (*_Destination)
                       : "r" (_Exchange) : "memory");
  return _Comparand;
}
#endif

extern int _may_i_use_cpu_feature(unsigned __int64);

#ifdef __cplusplus
}
#endif

#undef __DEFAULT_FN_ATTRS

#endif /* defined(_MSC_VER) && __has_extension(gnu_asm) */

#include <svmlintrin.h>// INTEL

/* Definitions of feature list to be used by feature select intrinsics */
#define _FEATURE_GENERIC_IA32        (1ULL     )
#define _FEATURE_FPU                 (1ULL << 1)
#define _FEATURE_CMOV                (1ULL << 2)
#define _FEATURE_MMX                 (1ULL << 3)
#define _FEATURE_FXSAVE              (1ULL << 4)
#define _FEATURE_SSE                 (1ULL << 5)
#define _FEATURE_SSE2                (1ULL << 6)
#define _FEATURE_SSE3                (1ULL << 7)
#define _FEATURE_SSSE3               (1ULL << 8)
#define _FEATURE_SSE4_1              (1ULL << 9)
#define _FEATURE_SSE4_2              (1ULL << 10)
#define _FEATURE_MOVBE               (1ULL << 11)
#define _FEATURE_POPCNT              (1ULL << 12)
#define _FEATURE_PCLMULQDQ           (1ULL << 13)
#define _FEATURE_AES                 (1ULL << 14)
#define _FEATURE_F16C                (1ULL << 15)
#define _FEATURE_AVX                 (1ULL << 16)
#define _FEATURE_RDRND               (1ULL << 17)
#define _FEATURE_FMA                 (1ULL << 18)
#define _FEATURE_BMI                 (1ULL << 19)
#define _FEATURE_LZCNT               (1ULL << 20)
#define _FEATURE_HLE                 (1ULL << 21)
#define _FEATURE_RTM                 (1ULL << 22)
#define _FEATURE_AVX2                (1ULL << 23)
#define _FEATURE_AVX512DQ            (1ULL << 24)
#define _FEATURE_PTWRITE             (1ULL << 25)
#define _FEATURE_AVX512F             (1ULL << 27)
#define _FEATURE_ADX                 (1ULL << 28)
#define _FEATURE_RDSEED              (1ULL << 29)
#define _FEATURE_AVX512IFMA52        (1ULL << 30)
#define _FEATURE_AVX512ER            (1ULL << 32)
#define _FEATURE_AVX512PF            (1ULL << 33)
#define _FEATURE_AVX512CD            (1ULL << 34)
#define _FEATURE_SHA                 (1ULL << 35)
#define _FEATURE_MPX                 (1ULL << 36)
#define _FEATURE_AVX512BW            (1ULL << 37)
#define _FEATURE_AVX512VL            (1ULL << 38)
#define _FEATURE_AVX512VBMI          (1ULL << 39)
#define _FEATURE_AVX512_4FMAPS       (1ULL << 40)
#define _FEATURE_AVX512_4VNNIW       (1ULL << 41)
#define _FEATURE_AVX512_VPOPCNTDQ    (1ULL << 42)
#define _FEATURE_AVX512_BITALG       (1ULL << 43)
#define _FEATURE_AVX512_VBMI2        (1ULL << 44)
#define _FEATURE_GFNI                (1ULL << 45)
#define _FEATURE_VAES                (1ULL << 46)
#define _FEATURE_VPCLMULQDQ          (1ULL << 47)
#define _FEATURE_AVX512_VNNI         (1ULL << 48)
#define _FEATURE_CLWB                (1ULL << 49)
#define _FEATURE_RDPID               (1ULL << 50)
#define _FEATURE_IBT                 (1ULL << 51)
#define _FEATURE_SHSTK               (1ULL << 52)
#define _FEATURE_SGX                 (1ULL << 53)
#define _FEATURE_WBNOINVD            (1ULL << 54)
#define _FEATURE_PCONFIG             (1ULL << 55)

#endif /* __IMMINTRIN_H */
