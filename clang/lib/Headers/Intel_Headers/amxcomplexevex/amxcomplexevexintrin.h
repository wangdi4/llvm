/*===------------- amxcomplexevexintrin.h - AMXCOMPLEXEVEX ------------------===
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
#error "Never use <amxcomplexevexintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AMX_COMPLEX_EVEXINTRIN_H
#define __AMX_COMPLEX_EVEXINTRIN_H
#ifdef __x86_64__

#define _tile_tcvtrowps2phiee(A, tsrc1, B)                                     \
  __builtin_ia32_tcvtrowps2phiee(A, tsrc1, B)
#define _tile_tcvtrowps2phiei(A, tsrc1, Imm)                                   \
  __builtin_ia32_tcvtrowps2phiei(A, tsrc1, Imm)

#endif // __x86_64__
#endif // __AMX_COMPLEX_EVEXINTRIN_H
