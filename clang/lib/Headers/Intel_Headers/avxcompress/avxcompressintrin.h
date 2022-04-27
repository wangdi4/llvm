/*===--------- avxcompressintrin.h - AVXCOMPRESS intrinsics ----------------=== */
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
#error "Never use <avxcompressintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVXCOMPRESSINTRIN_H
#define __AVXCOMPRESSINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS128 __attribute__((__always_inline__, __nodebug__, __target__("avxcompress"), __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS256 __attribute__((__always_inline__, __nodebug__, __target__("avxcompress"), __min_vector_width__(256)))
static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskload_epi8(char const *__X, __m256i __M)
{
  return (__m256i)__builtin_ia32_maskloadb256((const __v32qi *)__X, (__v32qi)__M);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_maskload_epi16(short const *__X, __m256i __M)
{
  return (__m256i)__builtin_ia32_maskloadw256((const __v16hi *)__X, (__v16hi)__M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskload_epi8(char const *__X, __m128i __M)
{
  return (__m128i)__builtin_ia32_maskloadb((const __v16qi *)__X, (__v16qi)__M);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_maskload_epi16(short const *__X, __m128i __M)
{
  return (__m128i)__builtin_ia32_maskloadw((const __v8hi *)__X, (__v8hi)__M);
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_maskstore_epi8(char *__X, __m256i __M, __m256i __Y)
{
  __builtin_ia32_maskstoreb256((__v32qi *)__X, (__v32qi)__M, (__v32qi)__Y);
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_maskstore_epi16(short *__X, __m256i __M, __m256i __Y)
{
  __builtin_ia32_maskstorew256((__v16hi *)__X, (__v16hi)__M, (__v16hi)__Y);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_maskstore_epi8(char *__X, __m128i __M, __m128i __Y)
{
  __builtin_ia32_maskstoreb((__v16qi *)__X, (__v16qi)__M, (__v16qi)__Y);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_maskstore_epi16(short *__X, __m128i __M, __m128i __Y)
{
  __builtin_ia32_maskstorew((__v8hi *)__X, (__v8hi)__M, (__v8hi)__Y);
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_compress_store_epi8(char *__X, __m256i __M, __m256i __Y)
{
  __builtin_ia32_vpcompressb_store_256((__v32qi *)__X, (__v32qi)__M, (__v32qi)__Y);
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_compress_store_epi16(short *__X, __m256i __M, __m256i __Y)
{
  __builtin_ia32_vpcompressw_store_256((__v16hi *)__X, (__v16hi)__M, (__v16hi)__Y);
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_compress_store_epi32(int *__X, __m256i __M, __m256i __Y)
{
  __builtin_ia32_vpcompressd_store_256((__v8si *)__X, (__v8si)__M, (__v8si)__Y);
}

static __inline__ void __DEFAULT_FN_ATTRS256
_mm256_compress_store_epi64(long long *__X, __m256i __M, __m256i __Y)
{
  __builtin_ia32_vpcompressq_store_256((__v4di *)__X, (__v4di)__M, (__v4di)__Y);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_compress_store_epi8(char *__X, __m128i __M, __m128i __Y)
{
  __builtin_ia32_vpcompressb_store_128((__v16qi *)__X, (__v16qi)__M, (__v16qi)__Y);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_compress_store_epi16(short *__X, __m128i __M, __m128i __Y)
{
  __builtin_ia32_vpcompressw_store_128((__v8hi *)__X, (__v8hi)__M, (__v8hi)__Y);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_compress_store_epi32(int *__X, __m128i __M, __m128i __Y)
{
  __builtin_ia32_vpcompressd_store_128((__v4si *)__X, (__v4si)__M, (__v4si)__Y);
}

static __inline__ void __DEFAULT_FN_ATTRS128
_mm_compress_store_epi64(long long *__X, __m128i __M, __m128i __Y)
{
  __builtin_ia32_vpcompressq_store_128((__v2di *)__X, (__v2di)__M, (__v2di)__Y);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_compress_epi8(__m256i __X, __m256i __Y)
{
  return (__m256i)__builtin_ia32_vpcompressb_256((__v32qi )__X, (__v32qi)__Y);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_compress_epi16(__m256i __X, __m256i __Y)
{
  return (__m256i)__builtin_ia32_vpcompressw_256((__v16hi )__X, (__v16hi)__Y);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_compress_epi32(__m256i __X, __m256i __Y)
{
  return (__m256i)__builtin_ia32_vpcompressd_256((__v8si )__X, (__v8si)__Y);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_compress_epi64(__m256i __X, __m256i __Y)
{
  return (__m256i)__builtin_ia32_vpcompressq_256((__v4di )__X, (__v4di)__Y);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_compress_epi8(__m128i __X, __m128i __Y)
{
  return (__m128i)__builtin_ia32_vpcompressb_128((__v16qi )__X, (__v16qi)__Y);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_compress_epi16(__m128i __X, __m128i __Y)
{
  return (__m128i)__builtin_ia32_vpcompressw_128((__v8hi )__X, (__v8hi)__Y);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_compress_epi32(__m128i __X, __m128i __Y)
{
  return (__m128i)__builtin_ia32_vpcompressd_128((__v4si )__X, (__v4si)__Y);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_compress_epi64(__m128i __X, __m128i __Y)
{
  return (__m128i)__builtin_ia32_vpcompressq_128((__v2di )__X, (__v2di)__Y);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_popcnt_avx_epi64(__m128i __A) {
  return (__m128i)__builtin_ia32_avxcompress_vpopcntq_128((__v2di)__A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_popcnt_avx_epi32(__m128i __A) {
  return (__m128i)__builtin_ia32_avxcompress_vpopcntd_128((__v4si)__A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_popcnt_avx_epi64(__m256i __A) {
  return (__m256i)__builtin_ia32_avxcompress_vpopcntq_256((__v4di)__A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_popcnt_avx_epi32(__m256i __A) {
  return (__m256i)__builtin_ia32_avxcompress_vpopcntd_256((__v8si)__A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_popcnt_avx_epi16(__m256i __A)
{
  return (__m256i) __builtin_ia32_avxcompress_vpopcntw_256((__v16hi) __A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_popcnt_avx_epi16(__m128i __A)
{
  return (__m128i) __builtin_ia32_avxcompress_vpopcntw_128((__v8hi) __A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_popcnt_avx_epi8(__m256i __A)
{
  return (__m256i) __builtin_ia32_avxcompress_vpopcntb_256((__v32qi) __A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_popcnt_avx_epi8(__m128i __A)
{
  return (__m128i) __builtin_ia32_avxcompress_vpopcntb_128((__v16qi) __A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_lzcnt_avx_epi32 (__m256i __A)
{
  return (__m256i) __builtin_ia32_avxcompress_vplzcntd_256 ((__v8si) __A);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_lzcnt_avx_epi32 (__m128i __A)
{
  return (__m128i) __builtin_ia32_avxcompress_vplzcntd_128 ((__v4si) __A);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_lzcnt_avx_epi64 (__m256i __A)
{
  return (__m256i) __builtin_ia32_avxcompress_vplzcntq_256 ((__v4di) __A);
}


static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_lzcnt_avx_epi64 (__m128i __A)
{
  return (__m128i) __builtin_ia32_avxcompress_vplzcntq_128 ((__v2di) __A);
}

#define _mm256_shldi_avx_epi64(A, B, I) \
  (__m256i)__builtin_ia32_avxcompress_vpshldq256((__v4di)(__m256i)(A), \
                                     (__v4di)(__m256i)(B), (int)(I))

#define _mm_shldi_avx_epi64(A, B, I) \
  (__m128i)__builtin_ia32_avxcompress_vpshldq128((__v2di)(__m128i)(A), \
                                     (__v2di)(__m128i)(B), (int)(I))

#define _mm256_shldi_avx_epi32(A, B, I) \
  (__m256i)__builtin_ia32_avxcompress_vpshldd256((__v8si)(__m256i)(A), \
                                     (__v8si)(__m256i)(B), (int)(I))

#define _mm_shldi_avx_epi32(A, B, I) \
  (__m128i)__builtin_ia32_avxcompress_vpshldd128((__v4si)(__m128i)(A), \
                                     (__v4si)(__m128i)(B), (int)(I))

#define _mm256_shldi_avx_epi16(A, B, I) \
  (__m256i)__builtin_ia32_avxcompress_vpshldw256((__v16hi)(__m256i)(A), \
                                     (__v16hi)(__m256i)(B), (int)(I))

#define _mm_shldi_avx_epi16(A, B, I) \
  (__m128i)__builtin_ia32_avxcompress_vpshldw128((__v8hi)(__m128i)(A), \
                                     (__v8hi)(__m128i)(B), (int)(I))

#define _mm256_shrdi_avx_epi64(A, B, I) \
  (__m256i)__builtin_ia32_avxcompress_vpshrdq256((__v4di)(__m256i)(A), \
                                     (__v4di)(__m256i)(B), (int)(I))

#define _mm_shrdi_avx_epi64(A, B, I) \
  (__m128i)__builtin_ia32_avxcompress_vpshrdq128((__v2di)(__m128i)(A), \
                                     (__v2di)(__m128i)(B), (int)(I))

#define _mm256_shrdi_avx_epi32(A, B, I) \
  (__m256i)__builtin_ia32_avxcompress_vpshrdd256((__v8si)(__m256i)(A), \
                                     (__v8si)(__m256i)(B), (int)(I))

#define _mm_shrdi_avx_epi32(A, B, I) \
  (__m128i)__builtin_ia32_avxcompress_vpshrdd128((__v4si)(__m128i)(A), \
                                     (__v4si)(__m128i)(B), (int)(I))

#define _mm256_shrdi_avx_epi16(A, B, I) \
  (__m256i)__builtin_ia32_avxcompress_vpshrdw256((__v16hi)(__m256i)(A), \
                                     (__v16hi)(__m256i)(B), (int)(I))

#define _mm_shrdi_avx_epi16(A, B, I) \
  (__m128i)__builtin_ia32_avxcompress_vpshrdw128((__v8hi)(__m128i)(A), \
                                     (__v8hi)(__m128i)(B), (int)(I))

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_shldv_avx_epi64(__m256i __A, __m256i __B, __m256i __C)
{
  return (__m256i)__builtin_ia32_avxcompress_vpshldvq256((__v4di)__A, (__v4di)__B,
                                             (__v4di)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_shldv_avx_epi64(__m128i __A, __m128i __B, __m128i __C)
{
  return (__m128i)__builtin_ia32_avxcompress_vpshldvq128((__v2di)__A, (__v2di)__B,
                                             (__v2di)__C);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_shldv_avx_epi32(__m256i __A, __m256i __B, __m256i __C)
{
  return (__m256i)__builtin_ia32_avxcompress_vpshldvd256((__v8si)__A, (__v8si)__B,
                                             (__v8si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_shldv_avx_epi32(__m128i __A, __m128i __B, __m128i __C)
{
  return (__m128i)__builtin_ia32_avxcompress_vpshldvd128((__v4si)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_shldv_avx_epi16(__m256i __A, __m256i __B, __m256i __C)
{
  return (__m256i)__builtin_ia32_avxcompress_vpshldvw256((__v16hi)__A, (__v16hi)__B,
                                             (__v16hi)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_shldv_avx_epi16(__m128i __A, __m128i __B, __m128i __C)
{
  return (__m128i)__builtin_ia32_avxcompress_vpshldvw128((__v8hi)__A, (__v8hi)__B,
                                             (__v8hi)__C);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_shrdv_avx_epi64(__m256i __A, __m256i __B, __m256i __C)
{
  return (__m256i)__builtin_ia32_avxcompress_vpshrdvq256((__v4di)__A, (__v4di)__B,
                                             (__v4di)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_shrdv_avx_epi64(__m128i __A, __m128i __B, __m128i __C)
{
  return (__m128i)__builtin_ia32_avxcompress_vpshrdvq128((__v2di)__A, (__v2di)__B,
                                             (__v2di)__C);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_shrdv_avx_epi32(__m256i __A, __m256i __B, __m256i __C)
{
  return (__m256i)__builtin_ia32_avxcompress_vpshrdvd256((__v8si)__A, (__v8si)__B,
                                             (__v8si)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_shrdv_avx_epi32(__m128i __A, __m128i __B, __m128i __C)
{
  return (__m128i)__builtin_ia32_avxcompress_vpshrdvd128((__v4si)__A, (__v4si)__B,
                                             (__v4si)__C);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_shrdv_avx_epi16(__m256i __A, __m256i __B, __m256i __C)
{
  return (__m256i)__builtin_ia32_avxcompress_vpshrdvw256((__v16hi)__A, (__v16hi)__B,
                                             (__v16hi)__C);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_shrdv_avx_epi16(__m128i __A, __m128i __B, __m128i __C)
{
  return (__m128i)__builtin_ia32_avxcompress_vpshrdvw128((__v8hi)__A, (__v8hi)__B,
                                             (__v8hi)__C);
}

#define _mm_rol_avx_epi32(a, b) \
  (__m128i)__builtin_ia32_avxcompress_vprold128((__v4si)(__m128i)(a), (int)(b))

#define _mm256_rol_avx_epi32(a, b) \
  (__m256i)__builtin_ia32_avxcompress_vprold256((__v8si)(__m256i)(a), (int)(b))

#define _mm_rol_avx_epi64(a, b) \
  (__m128i)__builtin_ia32_avxcompress_vprolq128((__v2di)(__m128i)(a), (int)(b))

#define _mm256_rol_avx_epi64(a, b) \
  (__m256i)__builtin_ia32_avxcompress_vprolq256((__v4di)(__m256i)(a), (int)(b))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_rolv_avx_epi32 (__m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_avxcompress_vprolvd128((__v4si)__A, (__v4si)__B);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_rolv_avx_epi32 (__m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_avxcompress_vprolvd256((__v8si)__A, (__v8si)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_rolv_avx_epi64 (__m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_avxcompress_vprolvq128((__v2di)__A, (__v2di)__B);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_rolv_avx_epi64 (__m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_avxcompress_vprolvq256((__v4di)__A, (__v4di)__B);
}

#define _mm_ror_avx_epi32(a, b) \
  (__m128i)__builtin_ia32_avxcompress_vprord128((__v4si)(__m128i)(a), (int)(b))

#define _mm256_ror_avx_epi32(a, b) \
  (__m256i)__builtin_ia32_avxcompress_vprord256((__v8si)(__m256i)(a), (int)(b))

#define _mm_ror_avx_epi64(a, b) \
  (__m128i)__builtin_ia32_avxcompress_vprorq128((__v2di)(__m128i)(a), (int)(b))

#define _mm256_ror_avx_epi64(a, b) \
  (__m256i)__builtin_ia32_avxcompress_vprorq256((__v4di)(__m256i)(a), (int)(b))

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_rorv_avx_epi32 (__m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_avxcompress_vprorvd128((__v4si)__A, (__v4si)__B);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_rorv_avx_epi32 (__m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_avxcompress_vprorvd256((__v8si)__A, (__v8si)__B);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_rorv_avx_epi64 (__m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_avxcompress_vprorvq128((__v2di)__A, (__v2di)__B);
}

static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_rorv_avx_epi64 (__m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_avxcompress_vprorvq256((__v4di)__A, (__v4di)__B);
}

#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVXCOMPRESSINTRIN_H
