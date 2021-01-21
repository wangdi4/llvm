/*===---------------- avxraointintrin.h - AVXRAOINT -------------------------===
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
#error "Never use <avxraointintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AVXRAOINTINTRIN_H
#define __AVXRAOINTINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("avxraoint"), __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256                                                  \
  __attribute__((__always_inline__, __nodebug__, __target__("avxraoint"), __min_vector_width__(256)))

static __inline__ void __DEFAULT_FN_ATTRS128 _mm_vpaaddd_epi32(__m128i *__A,
                                                               __m128i __B) {
  __builtin_ia32_vpaaddd128((__v4si *)__A, (__v4si)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS256 _mm256_vpaaddd_epi32(__m256i *__A,
                                                                  __m256i __B) {
  __builtin_ia32_vpaaddd256((__v8si *)__A, (__v8si)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128 _mm_vpaaddq_epi64(__m128i *__A,
                                                               __m128i __B) {
  __builtin_ia32_vpaaddq128((__v2di *)__A, (__v2di)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS256 _mm256_vpaaddq_epi64(__m256i *__A,
                                                                  __m256i __B) {
  __builtin_ia32_vpaaddq256((__v4di *)__A, (__v4di)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128 _mm_vpaandd_epi32(__m128i *__A,
                                                               __m128i __B) {
  __builtin_ia32_vpaandd128((__v4si *)__A, (__v4si)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS256 _mm256_vpaandd_epi32(__m256i *__A,
                                                                  __m256i __B) {
  __builtin_ia32_vpaandd256((__v8si *)__A, (__v8si)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128 _mm_vpaandq_epi64(__m128i *__A,
                                                               __m128i __B) {
  __builtin_ia32_vpaandq128((__v2di *)__A, (__v2di)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS256 _mm256_vpaandq_epi64(__m256i *__A,
                                                                  __m256i __B) {
  __builtin_ia32_vpaandq256((__v4di *)__A, (__v4di)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128 _mm_vpaord_epi32(__m128i *__A,
                                                              __m128i __B) {
  __builtin_ia32_vpaord128((__v4si *)__A, (__v4si)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS256 _mm256_vpaord_epi32(__m256i *__A,
                                                                 __m256i __B) {
  __builtin_ia32_vpaord256((__v8si *)__A, (__v8si)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128 _mm_vpaorq_epi64(__m128i *__A,
                                                              __m128i __B) {
  __builtin_ia32_vpaorq128((__v2di *)__A, (__v2di)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS256 _mm256_vpaorq_epi64(__m256i *__A,
                                                                 __m256i __B) {
  __builtin_ia32_vpaorq256((__v4di *)__A, (__v4di)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128 _mm_vpaxord_epi32(__m128i *__A,
                                                               __m128i __B) {
  __builtin_ia32_vpaxord128((__v4si *)__A, (__v4si)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS256 _mm256_vpaxord_epi32(__m256i *__A,
                                                                  __m256i __B) {
  __builtin_ia32_vpaxord256((__v8si *)__A, (__v8si)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS128 _mm_vpaxorq_epi64(__m128i *__A,
                                                               __m128i __B) {
  __builtin_ia32_vpaxorq128((__v2di *)__A, (__v2di)__B);
}

static __inline__ void __DEFAULT_FN_ATTRS256 _mm256_vpaxorq_epi64(__m256i *__A,
                                                                  __m256i __B) {
  __builtin_ia32_vpaxorq256((__v4di *)__A, (__v4di)__B);
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVXRAOINTINTRIN_H
