/* INTEL_FEATURE_ISA_KEYLOCKER */
/*===----------------- keylockerintrin.h - KL Intrinsics -------------------===
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
#ifndef _KEYLOCKERINTRIN_H
#define _KEYLOCKERINTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS \
  __attribute__((__always_inline__, __nodebug__, __target__("keylocker"),\
                 __min_vector_width__(128)))

static __inline__ void __DEFAULT_FN_ATTRS
_mm_loadiwkey (unsigned int __ctl, __m128i __intkey,
               __m128i __enkey_lo, __m128i __enkey_hi) {
  __builtin_ia32_loadiwkey (__ctl, __intkey, __enkey_lo, __enkey_hi);
}

static __inline__ unsigned int __DEFAULT_FN_ATTRS
_mm_encodekey128_u32(unsigned int __htype, __m128i __key, void *__h) {
  __m128i *__results = (__m128i*)__h;

  return __builtin_ia32_encodekey128(__htype, __key,
                                     __results,
                                     __results + 1,
                                     __results + 2,
                                     __results + 3,
                                     __results + 4,
                                     __results + 5);
}

static __inline__ unsigned int __DEFAULT_FN_ATTRS
_mm_encodekey256_u32(unsigned int __htype, __m128i __key_lo, __m128i __key_hi,
                     void *__h) {
  __m128i *__results = (__m128i*)__h;

  return __builtin_ia32_encodekey256(__htype, __key_lo, __key_hi,
                                     __results,
                                     __results + 1,
                                     __results + 2,
                                     __results + 3,
                                     __results + 4,
                                     __results + 5,
                                     __results + 6);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_aesenc128kl_si128(__m128i __data, const void *__h) {
  return __builtin_ia32_aesenc128kl(__data, __h);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_aesenc256kl_si128(__m128i __data, const void *__h) {
  return __builtin_ia32_aesenc256kl(__data, __h);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_aesdec128kl_si128(__m128i __data, const void *__h) {
  return __builtin_ia32_aesdec128kl(__data, __h);
}

static __inline__ __m128i __DEFAULT_FN_ATTRS
_mm_aesdec256kl_si128(__m128i __data, const void *__h) {
  return __builtin_ia32_aesdec256kl(__data, __h);
}

static __inline__ unsigned char __DEFAULT_FN_ATTRS
_mm_aesencwide128kl(__m128i __odata[8], const __m128i __idata[8], const void* __h) {
  return __builtin_ia32_aesencwide128kl(__h,
                                        __odata,
                                        __odata + 1,
                                        __odata + 2,
                                        __odata + 3,
                                        __odata + 4,
                                        __odata + 5,
                                        __odata + 6,
                                        __odata + 7,
                                        __idata[0],
                                        __idata[1],
                                        __idata[2],
                                        __idata[3],
                                        __idata[4],
                                        __idata[5],
                                        __idata[6],
                                        __idata[7]);
}

static __inline__ unsigned char __DEFAULT_FN_ATTRS
_mm_aesencwide256kl(__m128i __odata[8], const __m128i __idata[8], const void* __h) {
  return __builtin_ia32_aesencwide256kl(__h,
                                        __odata,
                                        __odata + 1,
                                        __odata + 2,
                                        __odata + 3,
                                        __odata + 4,
                                        __odata + 5,
                                        __odata + 6,
                                        __odata + 7,
                                        __idata[0],
                                        __idata[1],
                                        __idata[2],
                                        __idata[3],
                                        __idata[4],
                                        __idata[5],
                                        __idata[6],
                                        __idata[7]);
}

static __inline__ unsigned char __DEFAULT_FN_ATTRS
_mm_aesdecwide128kl(__m128i __odata[8], const __m128i __idata[8], const void* __h) {
  return __builtin_ia32_aesdecwide128kl(__h,
                                        __odata,
                                        __odata + 1,
                                        __odata + 2,
                                        __odata + 3,
                                        __odata + 4,
                                        __odata + 5,
                                        __odata + 6,
                                        __odata + 7,
                                        __idata[0],
                                        __idata[1],
                                        __idata[2],
                                        __idata[3],
                                        __idata[4],
                                        __idata[5],
                                        __idata[6],
                                        __idata[7]);
}

static __inline__ unsigned char __DEFAULT_FN_ATTRS
_mm_aesdecwide256kl(__m128i __odata[8], const __m128i __idata[8], const void* __h) {
  return __builtin_ia32_aesdecwide256kl(__h,
                                        __odata,
                                        __odata + 1,
                                        __odata + 2,
                                        __odata + 3,
                                        __odata + 4,
                                        __odata + 5,
                                        __odata + 6,
                                        __odata + 7,
                                        __idata[0],
                                        __idata[1],
                                        __idata[2],
                                        __idata[3],
                                        __idata[4],
                                        __idata[5],
                                        __idata[6],
                                        __idata[7]);
}


#undef __DEFAULT_FN_ATTRS

#endif /* _KEYLOCKERINTRIN_H */
/* end INTEL_FEATURE_ISA_KEYLOCKER */
