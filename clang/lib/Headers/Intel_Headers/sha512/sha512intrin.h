/*===--------------- sha512intrin.h - SHA512 intrinsics -----------------===
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
#error "Never use <sha512intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __SHA512INTRIN_H
#define __SHA512INTRIN_H

#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("sha512"),         \
                 __min_vector_width__(256)))

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_sha512msg1_epi64(__m256i __A, __m128i __B) {
  return (__m256i)__builtin_ia32_vsha512msg1((__v4du)__A, (__v2du)__B);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_sha512msg2_epi64(__m256i __A, __m256i __B) {
  return (__m256i)__builtin_ia32_vsha512msg2((__v4du)__A, (__v4du)__B);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_sha512rnds2_epi64(__m256i __A, __m256i __B, __m128i __C) {
  return (__m256i)__builtin_ia32_vsha512rnds2((__v4du)__A, (__v4du)__B,
                                              (__v2du)__C);
}

#undef __DEFAULT_FN_ATTRS256

#endif // __SHA512INTRIN_H
