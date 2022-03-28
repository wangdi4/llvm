/*===-------------- avx512raofpintrin.h - AVX512RAOFP -----------------------=== */
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
#error "Never use <avx512raofpintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVX512RAOFPINTRIN_H
#define __AVX512RAOFPINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512vl,avx512raofp"), __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS512 \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512raofp"), __min_vector_width__(512)))
#define __DEFAULT_FN_ATTRS    \
  __attribute__((__always_inline__, __nodebug__, __target__("avx512raofp"), __min_vector_width__(128)))

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vaadd_pbh(__m512bh *__A, __m512bh __B) {
  __builtin_ia32_vaaddpbf16512((__v32hi *)__A, (__v32hi)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_mask_vaadd_pbh(__m512bh *__A, __mmask32 __U, __m512bh __B) {
  __builtin_ia32_vaaddpbf16512_mask((__v32hi *)__A, (__v32hi)__B, (__mmask32)__U);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vaadd_pd(__m512d *__A, __m512d __B) {
  __builtin_ia32_vaaddpd512((__v8df *)__A, (__v8df)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_mask_vaadd_pd(__m512d *__A, __mmask8 __U, __m512d __B) {
  __builtin_ia32_vaaddpd512_mask((__v8df *)__A, (__v8df)__B, (__mmask8)__U);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vaadd_ph(__m512h *__A, __m512h __B) {
  __builtin_ia32_vaaddph512((__v32hf *)__A, (__v32hf)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_mask_vaadd_ph(__m512h *__A, __mmask32 __U, __m512h __B) {
  __builtin_ia32_vaaddph512_mask((__v32hf *)__A, (__v32hf)__B, (__mmask32)__U);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_vaadd_ps(__m512 *__A, __m512 __B) {
  __builtin_ia32_vaaddps512((__v16sf *)__A, (__v16sf)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS512
_mm512_mask_vaadd_ps(__m512 *__A, __mmask16 __U, __m512 __B) {
  __builtin_ia32_vaaddps512_mask((__v16sf *)__A, (__v16sf)__B, (__mmask16)__U);
}

#define _mm_vaadd_sbh(A, B) \
  __builtin_ia32_vaaddsbf16((void *)A, (__v8hi)B)

static __inline__ void __DEFAULT_FN_ATTRS
_mm_mask_vaadd_sbh(void *__A, __mmask8 __U, __m128bh __B) {
  __builtin_ia32_vaaddsbf16_mask((void *)__A, (__v8hi)__B, (__mmask8)__U);
}

#define _mm_vaadd_sd(A, B) \
  __builtin_ia32_vaaddsd((void *)A, (__v2df)B)

static __inline__ void __DEFAULT_FN_ATTRS
_mm_mask_vaadd_sd(void *__A, __mmask8 __U, __m128d __B) {
  __builtin_ia32_vaaddsd_mask((void *)__A, (__v2df)__B, (__mmask8)__U);
}

#define _mm_vaadd_sh(A, B) \
  __builtin_ia32_vaaddsh((void *)A, (__v8hf)B)

static __inline__ void __DEFAULT_FN_ATTRS
_mm_mask_vaadd_sh(void *__A, __mmask8 __U, __m128h __B) {
  __builtin_ia32_vaaddsh_mask((void *)__A, (__v8hf)__B, (__mmask8)__U);
}

#define _mm_vaadd_ss(A, B) \
  __builtin_ia32_vaaddss((void *)A, (__v4sf)B)

static __inline__ void __DEFAULT_FN_ATTRS
_mm_mask_vaadd_ss(void *__A, __mmask8 __U, __m128 __B) {
  __builtin_ia32_vaaddss_mask((void *)__A, (__v4sf)__B, (__mmask8)__U);
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS512

#endif // __AVX512RAOFPINTRIN_H
