/*===------------- vpinsrvpextr.h - VPINSRVPEXTR intrinsics -----------------===
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
#error "Never use <vpinsrvpextr.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __VPINSRVPEXTR_INTRIN_H
#define __VPINSRVPEXTR_INTRIN_H

#ifdef __x86_64__
#define _mm512_extract_epi64(X, N)                                             \
  ((long long)__builtin_ia32_vec_ext_v8di((__v8di)(__m512i)(X), (int)(N)))
#endif // __x86_64__

#define _mm512_extract_epi32(X, N)                                             \
  ((int)__builtin_ia32_vec_ext_v16si((__v16si)(__m512i)(X), (int)(N)))

#define _mm512_extract_epi16(X, N)                                             \
  ((int)(unsigned short)__builtin_ia32_vec_ext_v32hi((__v32hi)(__m512i)(X),    \
                                                     (int)(N)))

#define _mm512_extract_epi8(X, N)                                              \
  ((int)(unsigned char)__builtin_ia32_vec_ext_v64qi((__v64qi)(__m512i)(X),     \
                                                    (int)(N)))

#ifdef __x86_64__
#define _mm512_insert_epi64(X, I, N)                                           \
  ((__m512i)__builtin_ia32_vec_set_v8di((__v8di)(__m512i)(X), (long long)(I),  \
                                        (int)(N)))
#endif // __x86_64__

#define _mm512_insert_epi32(X, I, N)                                           \
  ((__m512i)__builtin_ia32_vec_set_v16si((__v16si)(__m512i)(X), (int)(I),      \
                                         (int)(N)))

#define _mm512_insert_epi16(X, I, N)                                           \
  ((__m512i)__builtin_ia32_vec_set_v32hi((__v32hi)(__m512i)(X), (int)(I),      \
                                         (int)(N)))

#define _mm512_insert_epi8(X, I, N)                                            \
  ((__m512i)__builtin_ia32_vec_set_v64qi((__v64qi)(__m512i)(X), (int)(I),      \
                                         (int)(N)))

#endif // __VPINSRVPEXTR_INTRIN_H
