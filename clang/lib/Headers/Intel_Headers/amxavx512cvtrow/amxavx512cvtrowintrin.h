/*===--------------- amxavx512cvtrowintrin.h - AMXAVX512CVTROW -------------===
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
#error "Never use <amxavx512cvtrowintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AMX_AVX512_CVTROWINTRIN_H
#define __AMX_AVX512_CVTROWINTRIN_H
#ifdef __x86_64__

#define _tile_tcvtrowd2psee(tsrc, A) __builtin_ia32_tcvtrowd2psee(tsrc, A)
#define _tile_tcvtrowd2psei(tsrc, Imm) __builtin_ia32_tcvtrowd2psei(tsrc, Imm)
#define _tile_tcvtrowps2pbf16hee(tsrc, A)                                      \
  __builtin_ia32_tcvtrowps2pbf16hee(tsrc, A)
#define _tile_tcvtrowps2pbf16hei(tsrc, Imm)                                    \
  __builtin_ia32_tcvtrowps2pbf16hei(tsrc, Imm)
#define _tile_tcvtrowps2pbf16lee(tsrc, A)                                      \
  __builtin_ia32_tcvtrowps2pbf16lee(tsrc, A)
#define _tile_tcvtrowps2pbf16lei(tsrc, Imm)                                    \
  __builtin_ia32_tcvtrowps2pbf16lei(tsrc, Imm)
#define _tile_tcvtrowps2phhee(tsrc, A) __builtin_ia32_tcvtrowps2phhee(tsrc, A)
#define _tile_tcvtrowps2phhei(tsrc, Imm)                                       \
  __builtin_ia32_tcvtrowps2phhei(tsrc, Imm)
#define _tile_tcvtrowps2phlee(tsrc, A) __builtin_ia32_tcvtrowps2phlee(tsrc, A)
#define _tile_tcvtrowps2phlei(tsrc, Imm)                                       \
  __builtin_ia32_tcvtrowps2phlei(tsrc, Imm)

#endif // __x86_64__
#endif // __AMX_AVX512_CVTROWINTRIN_H
