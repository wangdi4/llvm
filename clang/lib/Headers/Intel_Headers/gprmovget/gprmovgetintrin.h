/*===-------- gprmovgetintrin.h - GPRMOVGET -------------===
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
#ifndef __X86GPRINTRIN_H
#error "Never use <gprmovgetintrin.h> directly; include <x86gprintrin.h> instead."
#endif // __X86GPRINTRIN_H

#ifndef __GPRMOVGETINTRIN_H
#define __GPRMOVGETINTRIN_H

static __inline__ unsigned int
__attribute__((__always_inline__, __nodebug__, __target__("gprmovget")))
_movget_u32(const unsigned int *__A) {
  return (unsigned int)__builtin_ia32_movget32((const unsigned int *)__A);
}

#ifdef __x86_64__
static __inline__ unsigned long long
__attribute__((__always_inline__, __nodebug__, __target__("gprmovget")))
_movget_u64(const unsigned long long *__A) {
  return (unsigned long long)__builtin_ia32_movget64(
      (const unsigned long long *)__A);
}
#endif

#endif // __GPRMOVGETINTRIN_H
