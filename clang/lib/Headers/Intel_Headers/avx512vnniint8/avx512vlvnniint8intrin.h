/*===------ avx512vlvnniint8intrin.h - AVX512VNNIINT8 intrinsics ------=== */
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
/*
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
#error "Never use <avx512vlvnniint8intrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512VLVNNIINT8INTRIN_H
#define __AVX512VLVNNIINT8INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, \
  __target__("avx512vnniint8, avx512vl"), __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, \
 __target__("avx512vnniint8, avx512vl"), __min_vector_width__(128)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_vpdpbssd_epi32(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_dpbssd_epi32(__W, __A, __B),
                  (__v4si)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_vpdpbssd_epi32(__mmask8 __U, __m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_dpbssd_epi32(__W, __A, __B),
                  (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_vpdpbssd_epi32(__m256i __W, __mmask8 __U, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_dpbssd_epi32(__W, __A, __B),
                                     (__v8si)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_vpdpbssd_epi32(__mmask8 __U, __m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_dpbssd_epi32(__W, __A, __B),
                                     (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_vpdpbssds_epi32(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_dpbssds_epi32(__W, __A, __B),
                  (__v4si)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_vpdpbssds_epi32(__mmask8 __U, __m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_dpbssds_epi32(__W, __A, __B),
                  (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_vpdpbssds_epi32(__m256i __W, __mmask8 __U, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_dpbssds_epi32(__W, __A, __B),
                                     (__v8si)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_vpdpbssds_epi32(__mmask8 __U, __m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_dpbssds_epi32(__W, __A, __B),
                                     (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_vpdpbsud_epi32(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_dpbsud_epi32(__W, __A, __B),
                  (__v4si)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_vpdpbsud_epi32(__mmask8 __U, __m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_dpbsud_epi32(__W, __A, __B),
                  (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_vpdpbsud_epi32(__m256i __W, __mmask8 __U, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_dpbsud_epi32(__W, __A, __B),
                                     (__v8si)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_vpdpbsud_epi32(__mmask8 __U, __m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_dpbsud_epi32(__W, __A, __B),
                                     (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_vpdpbsuds_epi32(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_dpbsuds_epi32(__W, __A, __B),
                  (__v4si)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_vpdpbsuds_epi32(__mmask8 __U, __m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_dpbsuds_epi32(__W, __A, __B),
                  (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_vpdpbsuds_epi32(__m256i __W, __mmask8 __U, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_dpbsuds_epi32(__W, __A, __B),
                                     (__v8si)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_vpdpbsuds_epi32(__mmask8 __U, __m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_dpbsuds_epi32(__W, __A, __B),
                                     (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_vpdpbuud_epi32(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_dpbuud_epi32(__W, __A, __B),
                  (__v4si)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_vpdpbuud_epi32(__mmask8 __U, __m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_dpbuud_epi32(__W, __A, __B),
                  (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_vpdpbuud_epi32(__m256i __W, __mmask8 __U, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_dpbuud_epi32(__W, __A, __B),
                                     (__v8si)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_vpdpbuud_epi32(__mmask8 __U, __m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_dpbuud_epi32(__W, __A, __B),
                                     (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_vpdpbuuds_epi32(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_dpbuuds_epi32(__W, __A, __B),
                  (__v4si)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_vpdpbuuds_epi32(__mmask8 __U, __m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_dpbuuds_epi32(__W, __A, __B),
                  (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_vpdpbuuds_epi32(__m256i __W, __mmask8 __U, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_dpbuuds_epi32(__W, __A, __B),
                                     (__v8si)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_vpdpbuuds_epi32(__mmask8 __U, __m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_dpbuuds_epi32(__W, __A, __B),
                                     (__v8si)_mm256_setzero_si256());
}
#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLVNNIINT8INTRIN_H
