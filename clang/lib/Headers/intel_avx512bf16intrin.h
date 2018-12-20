/* INTEL_FEATURE_ISA_BF16 */
/*===--------- intel_avx512bf16intrin.h - BF16 intrinsics -----------------===
 *
 * Copyright (C) 2018 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <intel_avx512bf16intrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512BF16INTRIN_H
#define __AVX512BF16INTRIN_H

typedef short __m512bh __attribute__((__vector_size__(64)));
typedef short __m256bh __attribute__((__vector_size__(32)));

#define __DEFAULT_FN_ATTRS512 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512bf16"), \
                 __min_vector_width__(512)))
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512bf16"), \
                 __min_vector_width__(256)))

static __inline__ __m512bh __DEFAULT_FN_ATTRS512
_mm512_cvtne2ps_pbh(__m512 __A, __m512 __B) {
  return (__m512bh)__builtin_ia32_cvtne2ps2bf16_512((__v16sf) __A,
                                                    (__v16sf) __B);
}

static __inline__ __m512bh __DEFAULT_FN_ATTRS512
_mm512_mask_cvtne2ps_pbh(__m512bh __W, __mmask32 __U, __m512 __A, __m512 __B) {
  return (__m512bh)__builtin_ia32_selectw_512((__mmask32)__U,
                                        (__v32hi)_mm512_cvtne2ps_pbh(__A, __B),
                                        (__v32hi)__W);
}

static __inline__ __m512bh __DEFAULT_FN_ATTRS512
_mm512_maskz_cvtne2ps_pbh(__mmask32 __U, __m512 __A, __m512 __B) {
  return (__m512bh)__builtin_ia32_selectw_512((__mmask32)__U,
                                        (__v32hi)_mm512_cvtne2ps_pbh(__A, __B),
                                        (__v32hi)_mm512_setzero_si512());
}

static __inline__ __m256bh __DEFAULT_FN_ATTRS256
_mm512_cvtneps_pbh(__m512 __A) {
  return (__m256bh)__builtin_ia32_cvtneps2bf16_512((__v16sf) __A);
}
static __inline__ __m256bh __DEFAULT_FN_ATTRS256
_mm512_mask_cvtneps_pbh(__m256bh __W, __mmask16 __U, __m512 __A) {
  return (__m256bh)__builtin_ia32_selectw_256((__mmask16)__U,
                                              (__v16hi)_mm512_cvtneps_pbh(__A),
                                              (__v16hi)__W);
}
static __inline__ __m256bh __DEFAULT_FN_ATTRS256
_mm512_maskz_cvtneps_pbh(__mmask16 __U, __m512 __A) {
  return (__m256bh)__builtin_ia32_selectw_256((__mmask16)__U,
                                              (__v16hi)_mm512_cvtneps_pbh(__A),
                                              (__v16hi)_mm256_setzero_si256());
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_dpbf16_ps(__m512 __D, __m512bh __A, __m512bh __B) {
  return (__m512)__builtin_ia32_dpbf16ps_512((__v16sf) __D,
                                             (__v16si) __A,
                                             (__v16si) __B);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_mask_dpbf16_ps(__m512 __D, __mmask16 __U, __m512bh __A, __m512bh __B) {
  return (__m512)__builtin_ia32_selectps_512((__mmask16)__U,
                                       (__v16sf)_mm512_dpbf16_ps(__D, __A, __B),
                                       (__v16sf)__D);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS512
_mm512_maskz_dpbf16_ps(__mmask16 __U, __m512 __D, __m512bh __A, __m512bh __B) {
  return (__m512)__builtin_ia32_selectps_512((__mmask16)__U,
                                       (__v16sf)_mm512_dpbf16_ps(__D, __A, __B),
                                       (__v16sf)_mm512_setzero_si512());
}

#undef __DEFAULT_FN_ATTRS512
#undef __DEFAULT_FN_ATTRS256

#endif
/* end INTEL_FEATURE_ISA_BF16 */
