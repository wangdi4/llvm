/*===------------- avxbf16intrin.h - AVXBF16 intrinsics ------------------=== */
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
#error "Never use <avxbf16intrin.h> directly; include <immintrin.h> instead."
#endif

#ifdef __SSE2__

#ifndef __AVXBF16INTRIN_H
#define __AVXBF16INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128 __attribute__((__always_inline__, __nodebug__, __target__("avxbf16"), __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256 __attribute__((__always_inline__, __nodebug__, __target__("avxbf16"), __min_vector_width__(256)))

// must vex-encoding
static __inline__ __m128bh __DEFAULT_FN_ATTRS128
_mm_cvtne2ps_avx_pbh(__m128 __A, __m128 __B) {
  return (__m128bh)__builtin_ia32_cvtne2ps2bf16vex128((__v4sf) __A,
                                                    (__v4sf) __B);
}

static __inline__ __m256bh __DEFAULT_FN_ATTRS256
_mm256_cvtne2ps_avx_pbh(__m256 __A, __m256 __B) {
  return (__m256bh)__builtin_ia32_cvtne2ps2bf16vex256((__v8sf) __A,
                                                    (__v8sf) __B);
}

static __inline__ __m128 __DEFAULT_FN_ATTRS128
_mm_dpbf16_avx_ps(__m128 __D, __m128bh __A, __m128bh __B) {
  return (__m128)__builtin_ia32_dpbf16psvex128((__v4sf)__D,
                                             (__v4si)__A,
                                             (__v4si)__B);
}

static __inline__ __m256 __DEFAULT_FN_ATTRS256
_mm256_dpbf16_avx_ps(__m256 __D, __m256bh __A, __m256bh __B) {
  return (__m256)__builtin_ia32_dpbf16psvex256((__v8sf)__D,
                                             (__v8si)__A,
                                             (__v8si)__B);
}
#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVXBF16INTRIN_H
#endif
