/*===--------- avxmemadviseintrin.h - AVXMEMADVISE intrinsics ----------------=== */
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
#error "Never use <avxmemadviseintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVXMEMADVISEINTRIN_H
#define __AVXMEMADVISEINTRIN_H
#ifdef __x86_64__

/* Below intrinsics defined in avx512vlvnniintrin.h can be used for AVXMEMADVIS */
/// \fn __m128i _mm_vmovadvisew_load_epi8(const __m128i* __A,  I)
/// \fn __m256i _mm256_vmovadvisew_load_epi8(const __m256i* __A,  I)
/// \fn void _mm_vmovadvisew_store_epi8(__m128i* A, __m128i B,  I)
/// \fn void _mm256_vmovadvisew_store_epi8(__m256i* A, __m256i B,  I)
/// \fn void _mm_vmemadvise_epi8(const __m128i* __A,  I)
/// \fn void _mm256_vmemadvise_epi8(const __m256i* __A,  I)

/* Intrinsics with _avx_ prefix are for compatibility with msvc. */

#define _mm_vmovadvisew_load_avx_epi8(A, I)                                    \
  (__m128i)__builtin_ia32_vmovadvisew_load_128((const __v4si *)(A), (I))

#define _mm256_vmovadvisew_load_avx_epi8(A, I)                                 \
   (__m256i)__builtin_ia32_vmovadvisew_load_256((const __v8si *)(A), (I))

#define _mm_vmovadvisew_store_avx_epi8(A, B, I)                                \
  __builtin_ia32_vmovadvisew_store_128((__v4si *)(A), (__v4si)(B), (I))

#define _mm256_vmovadvisew_store_avx_epi8(A, B, I)                             \
  __builtin_ia32_vmovadvisew_store_256((__v8si *)(A), (__v8si)(B), (I))

#define _mm_vmemadvise_avx_epi8(A, I)                                          \
  __builtin_ia32_vmemadvise_128((const __v16qi *)(A), (I))

#define _mm256_vmemadvise_avx_epi8(A, I)                                       \
  __builtin_ia32_vmemadvise_256((const __v32qi *)(A), (I))
#endif /* __x86_64__ */
#endif /* __AVXMEMADVISEINTRIN_H */
