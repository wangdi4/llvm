/*===--------------- sm4intrin.h - SM4 intrinsics -----------------===
 *
 * Copyright (C) 2021-2023 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <sm4intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __SM4INTRIN_H
#define __SM4INTRIN_H

#define _mm_sm4key4_epi32(A, B)                                                \
  (__m128i) __builtin_ia32_vsm4key4128((__v4su)A, (__v4su)B)

#define _mm256_sm4key4_epi32(A, B)                                             \
  (__m256i) __builtin_ia32_vsm4key4256((__v8su)A, (__v8su)B)

#define _mm_sm4rnds4_epi32(A, B)                                               \
  (__m128i) __builtin_ia32_vsm4rnds4128((__v4su)A, (__v4su)B)

#define _mm256_sm4rnds4_epi32(A, B)                                            \
  (__m256i) __builtin_ia32_vsm4rnds4256((__v8su)A, (__v8su)B)

#endif // __SM4INTRIN_H
