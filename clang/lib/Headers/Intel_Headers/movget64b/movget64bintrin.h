/*===---------------- movget64bintrin.h - MOVGET64B -------------------------===
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
#error "Never use <movget64bintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __MOVGET64BINTRIN_H
#define __MOVGET64BINTRIN_H

#define __DEFAULT_FN_ATTRS512 __attribute__((__always_inline__, __nodebug__, __target__("movget64b"), __min_vector_width__(512)))

#ifdef __x86_64__
static __inline__ unsigned long long __DEFAULT_FN_ATTRS512
_movget64b_u64(const __m512i *__A) {
  return (unsigned long long)__builtin_ia32_movget64b64((const __v16su *)__A);
}
#endif

static __inline__ unsigned int __DEFAULT_FN_ATTRS512
_movget64b_u32(const __m512i *__A) {
  return (unsigned int)__builtin_ia32_movget64b32((const __v16su *)__A);
}

#undef __DEFAULT_FN_ATTRS512

#endif // __MOVGET64BINTRIN_H
