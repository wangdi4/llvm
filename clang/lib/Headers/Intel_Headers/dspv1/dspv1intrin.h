/*===------------------- dspintrin.h - DSP intrinsics ----------------------===
 *
 * Copyright (C) 2021 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <dspintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __DSPINTRIN_H
#define __DSPINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("dspv1"),          \
                 __min_vector_width__(128)))

#define _mm_dsp_pcr2bfrsw_epi16(A, B, C, D)                                    \
  (__m128i) __builtin_ia32_dvpcr2bfrsw((__v8hi)A, (__v8hi)B, (__v8hi)C, (int)D)

#define _mm_dsp_plutsincosw_epi16(A, B)                                        \
  (__m128i) __builtin_ia32_dvplutsincosw((__v8hi)A, (int)B)

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmuludhhq_epi64(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmuludhhq((__v4su)__A, (__v4su)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmuldhhq_epi64(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmuldhhq((__v4si)__A, (__v4si)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmuldfrs_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmuldfrs((__v4si)__A, (__v4si)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmulds_epi32(__m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_dvpmulds((__v4si)__A, (__v4si)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacudllsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacudllsq((__v2du)__A, (__v4su)__B,
                                              (__v4su)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacudhhsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacudhhsq((__v2du)__A, (__v4su)__B,
                                              (__v4su)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacudllq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacudllq((__v2du)__A, (__v4su)__B,
                                             (__v4su)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacudhhq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacudhhq((__v2du)__A, (__v4su)__B,
                                             (__v4su)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacdllsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacdllsq((__v2di)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacdhhsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacdhhsq((__v2di)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacdllq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacdllq((__v2di)__A, (__v4si)__B,
                                            (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmacdhhq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmacdhhq((__v2di)__A, (__v4si)__B,
                                            (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pnmacdllsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpnmacdllsq((__v2di)__A, (__v4si)__B,
                                              (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pnmacdhhsq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpnmacdhhsq((__v2di)__A, (__v4si)__B,
                                              (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pnmacdllq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpnmacdllq((__v2di)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pnmacdhhq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpnmacdhhq((__v2di)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_dsp_pmsubadddllq_epi64(__m128i __A, __m128i __B, __m128i __C) {
  return (__m128i)__builtin_ia32_dvpmsubadddllq((__v2di)__A, (__v4si)__B,
                                                (__v4si)__C);
}

#undef __DEFAULT_FN_ATTRS128
#endif // __DSPINTRIN_H
