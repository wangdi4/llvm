/*===------ avx512vldotprodint8intrin.h - AVX512DOTPRODINT8 intrinsics ------===
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
#error "Never use <avx512vldotprodint8intrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512VLDOTPRODINT8INTRIN_H
#define __AVX512VLDOTPRODINT8INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, \
  __target__("avx512dotprodint8, avx512vl"), __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, \
 __target__("avx512dotprodint8, avx512vl"), __min_vector_width__(128)))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_vpdpbssd_epi32(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_vpdpbssd_epi32(__W, __A, __B),
                  (__v4si)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_vpdpbssd_epi32(__mmask8 __U, __m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_vpdpbssd_epi32(__W, __A, __B),
                  (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_vpdpbssd_epi32(__m256i __W, __mmask8 __U, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_vpdpbssd_epi32(__W, __A, __B),
                                     (__v8si)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_vpdpbssd_epi32(__mmask8 __U, __m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_vpdpbssd_epi32(__W, __A, __B),
                                     (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_vpdpbssds_epi32(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_vpdpbssds_epi32(__W, __A, __B),
                  (__v4si)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_vpdpbssds_epi32(__mmask8 __U, __m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_vpdpbssds_epi32(__W, __A, __B),
                  (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_vpdpbssds_epi32(__m256i __W, __mmask8 __U, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_vpdpbssds_epi32(__W, __A, __B),
                                     (__v8si)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_vpdpbssds_epi32(__mmask8 __U, __m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_vpdpbssds_epi32(__W, __A, __B),
                                     (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_vpdpbsud_epi32(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_vpdpbsud_epi32(__W, __A, __B),
                  (__v4si)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_vpdpbsud_epi32(__mmask8 __U, __m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_vpdpbsud_epi32(__W, __A, __B),
                  (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_vpdpbsud_epi32(__m256i __W, __mmask8 __U, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_vpdpbsud_epi32(__W, __A, __B),
                                     (__v8si)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_vpdpbsud_epi32(__mmask8 __U, __m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_vpdpbsud_epi32(__W, __A, __B),
                                     (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_vpdpbsuds_epi32(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_vpdpbsuds_epi32(__W, __A, __B),
                  (__v4si)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_vpdpbsuds_epi32(__mmask8 __U, __m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_vpdpbsuds_epi32(__W, __A, __B),
                  (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_vpdpbsuds_epi32(__m256i __W, __mmask8 __U, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_vpdpbsuds_epi32(__W, __A, __B),
                                     (__v8si)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_vpdpbsuds_epi32(__mmask8 __U, __m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_vpdpbsuds_epi32(__W, __A, __B),
                                     (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_vpdpbuud_epi32(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_vpdpbuud_epi32(__W, __A, __B),
                  (__v4si)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_vpdpbuud_epi32(__mmask8 __U, __m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_vpdpbuud_epi32(__W, __A, __B),
                  (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_vpdpbuud_epi32(__m256i __W, __mmask8 __U, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_vpdpbuud_epi32(__W, __A, __B),
                                     (__v8si)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_vpdpbuud_epi32(__mmask8 __U, __m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_vpdpbuud_epi32(__W, __A, __B),
                                     (__v8si)_mm256_setzero_si256());
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_mask_vpdpbuuds_epi32(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_vpdpbuuds_epi32(__W, __A, __B),
                  (__v4si)__W);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskz_vpdpbuuds_epi32(__mmask8 __U, __m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_selectd_128(__U,
                  (__v4si)_mm_vpdpbuuds_epi32(__W, __A, __B),
                  (__v4si)_mm_setzero_si128());
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_mask_vpdpbuuds_epi32(__m256i __W, __mmask8 __U, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_vpdpbuuds_epi32(__W, __A, __B),
                                     (__v8si)__W);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskz_vpdpbuuds_epi32(__mmask8 __U, __m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_selectd_256(__U,
                                     (__v8si)_mm256_vpdpbuuds_epi32(__W, __A, __B),
                                     (__v8si)_mm256_setzero_si256());
}
#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVX512VLDOTPRODINT8INTRIN_H
