/*===-------- avx512memadviseintrin.h - AVXMEMADVISE intrinsics -------------===
 *
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
#error "Never use <avx512memadviseintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512MEMADVISEINTRIN_H
#define __AVX512MEMADVISEINTRIN_H
#ifdef __x86_64__

#define _mm128_vmovadvisew_loade_epi8(A, I) \
   (__m128i)__builtin_ia32_vmovadvisew_loade_128((const __v4si *)(A), (I))

#define _mm128_vmovadvisew_storee_epi8(A, B, I) \
   __builtin_ia32_vmovadvisew_storee_128((__v4si *)(A), (__v4si)(B), (I))

#define _mm256_vmovadvisew_loade_epi8(A, I) \
   (__m256i)__builtin_ia32_vmovadvisew_loade_256((const __v8si *)(A), (I))

#define _mm256_vmovadvisew_storee_epi8(A, B, I) \
   __builtin_ia32_vmovadvisew_storee_256((__v8si *)(A), (__v8si)(B), (I))

#define _mm512_vmovadvisew_loade_epi8(A, I) \
   (__m512i)__builtin_ia32_vmovadvisew_loade_512((const __v16si *)(A), (I))

#define _mm512_vmovadvisew_storee_epi8(A, B, I) \
   __builtin_ia32_vmovadvisew_storee_512((__v16si *)(A), (__v16si)(B), (I))

#endif /* __x86_64__ */
#endif /* __AVX512MEMADVISEINTRIN_H */
