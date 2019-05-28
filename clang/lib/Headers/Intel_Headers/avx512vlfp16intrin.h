/* INTEL_FEATURE_ISA_FP16 */
/*===--------------- avx512fp16intrin.h - FP16 intrinsics -----------------===
 *
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <avx512vlfp16intrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512VLFP16INTRIN_H
#define __AVX512VLFP16INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512fp16, avx512vl"), __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512fp16, avx512vl"), __min_vector_width__(128)))

static __inline__ _Float16 __DEFAULT_FN_ATTRS128
_mm_cvtsh_h(__m128h __a)
{
  return __a[0];
}

static __inline__ __m128h __DEFAULT_FN_ATTRS128
_mm_set_sh(_Float16 __h)
{
  return __extension__ (__m128h){ __h, 0, 0, 0, 0, 0, 0, 0 };
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif

/* end INTEL_FEATURE_ISA_FP16 */
