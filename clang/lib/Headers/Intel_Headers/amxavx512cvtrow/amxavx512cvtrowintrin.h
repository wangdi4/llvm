/*===--------------- amxavx512cvtrowintrin.h - AMXAVX512CVTROW -------------=== */
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
#error "Never use <amxavx512cvtrowintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AMX_AVX512_CVTROWINTRIN_H
#define __AMX_AVX512_CVTROWINTRIN_H
#ifdef __x86_64__

#define __DEFAULT_FN_ATTRS_AVX512                                              \
__attribute__((__always_inline__, __nodebug__, __target__("amx-avx512-cvtrow")))

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

/// This is internal intrinsic. C/C++ user should avoid calling it directly.

static __inline__ __m512 __DEFAULT_FN_ATTRS_AVX512
_tile_tcvtrowd2psee_internal(unsigned short m, unsigned short n,
                              _tile1024i src, unsigned u) {
  return __builtin_ia32_tcvtrowd2psee_internal(m, n, src, u);
}

#define _tile_tcvtrowd2psei_internal(m, n, tsrc, i)                            \
  __builtin_ia32_tcvtrowd2psei_internal(m, n, tsrc, i)

static __inline__ __m512bh __DEFAULT_FN_ATTRS_AVX512
_tile_tcvtrowps2pbf16hee_internal(unsigned short m, unsigned short n,
                              _tile1024i src, unsigned u) {
  return __builtin_ia32_tcvtrowps2pbf16hee_internal(m, n, src, u);
}

#define _tile_tcvtrowps2pbf16hei_internal(m, n, tsrc, i)                       \
  __builtin_ia32_tcvtrowps2pbf16hei_internal(m, n, tsrc, i)

static __inline__ __m512bh __DEFAULT_FN_ATTRS_AVX512
_tile_tcvtrowps2pbf16lee_internal(unsigned short m, unsigned short n,
                              _tile1024i src, unsigned u) {
  return __builtin_ia32_tcvtrowps2pbf16lee_internal(m, n, src, u);
}

#define _tile_tcvtrowps2pbf16lei_internal(m, n, tsrc, i)                       \
  __builtin_ia32_tcvtrowps2pbf16lei_internal(m, n, tsrc, i)

static __inline__ __m512h __DEFAULT_FN_ATTRS_AVX512
_tile_tcvtrowps2phhee_internal(unsigned short m, unsigned short n,
                              _tile1024i src, unsigned u) {
  return __builtin_ia32_tcvtrowps2phhee_internal(m, n, src, u);
}

#define _tile_tcvtrowps2phhei_internal(m, n, tsrc, i)                       \
  __builtin_ia32_tcvtrowps2phhei_internal(m, n, tsrc, i)

static __inline__ __m512h __DEFAULT_FN_ATTRS_AVX512
_tile_tcvtrowps2phlee_internal(unsigned short m, unsigned short n,
                              _tile1024i src, unsigned u) {
  return __builtin_ia32_tcvtrowps2phlee_internal(m, n, src, u);
}

#define _tile_tcvtrowps2phlei_internal(m, n, tsrc, i)                       \
  __builtin_ia32_tcvtrowps2phlei_internal(m, n, tsrc, i)

/// Move a row from a tile (src0) to a v16f32 dst, converting the int32 source
/// elements to fp32. No SIMD exceptions are generated. Rounding is done as if
/// MXCSR.RC=RNE. Embedded rounding is not supported.
/// The row and chunk elements of tile is fetched from 32bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWD2PSE </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 1st source r32. Size is 4 Bytes.
/// \ret
///    The destination v16f32 data. Size is 64 Bytes.
__DEFAULT_FN_ATTRS_AVX512
static __m512 __tile_tcvtrowd2psee(__tile1024i src0, unsigned src1) {
  return _tile_tcvtrowd2psee_internal(src0.row, src0.col, src0.tile, src1);
}

/// Move a row from a tile (src0) to a v16f32 dst, converting the int32 source
/// elements to fp32. No SIMD exceptions are generated. Rounding is done as if
/// MXCSR.RC=RNE. Embedded rounding is not supported.
/// The row and chunk elements of tile is fetched from 8bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWD2PSE </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source imm8. Size is 4 Bytes.
/// \ret
///    The destination v16f32 data. Size is 64 Bytes.
#define __tile_tcvtrowd2psei(src0, src1)                                     \
  _tile_tcvtrowd2psei_internal(src0.row, src0.col, src0.tile, src1);

/// Move a row from a tile (src0) to a v32bf16 dst, converting the fp32 source
/// elements to bf16 at high 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 32bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PBF16HE </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 1st source r32. Size is 4 Bytes.
/// \ret
///    The destination v32bf16 data. Size is 64 Bytes.
__DEFAULT_FN_ATTRS_AVX512
static __m512bh __tile_tcvtrowps2pbf16hee(__tile1024i src0, unsigned src1) {
  return _tile_tcvtrowps2pbf16hee_internal(src0.row, src0.col, src0.tile, src1);
}

/// Move a row from a tile (src0) to a v32bf16 dst, converting the f32 source
/// elements to bf16 at high 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 8bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PBF16HE </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source imm8. Size is 4 Bytes.
/// \ret
///    The destination v32bf16 data. Size is 64 Bytes.
#define __tile_tcvtrowps2pbf16hei(src0, src1)                                     \
  _tile_tcvtrowps2pbf16hei_internal(src0.row, src0.col, src0.tile, src1);

/// Move a row from a tile (src0) to a v32bf16 dst, converting the fp32 source
/// elements to bf16 at low 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 32bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PBF16LE </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 1st source r32. Size is 4 Bytes.
/// \ret
///    The destination v32bf16 data. Size is 64 Bytes.
__DEFAULT_FN_ATTRS_AVX512
static __m512bh __tile_tcvtrowps2pbf16lee(__tile1024i src0, unsigned src1) {
  return _tile_tcvtrowps2pbf16lee_internal(src0.row, src0.col, src0.tile, src1);
}

/// Move a row from a tile (src0) to a v32bf16 dst, converting the f3232 source
/// elements to bf16 at low 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 8bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PBF16LE </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source imm8. Size is 4 Bytes.
/// \ret
///    The destination v32bf16 data. Size is 64 Bytes.
#define __tile_tcvtrowps2pbf16lei(src0, src1)                                     \
  _tile_tcvtrowps2pbf16lei_internal(src0.row, src0.col, src0.tile, src1);

/// Move a row from a tile (src0) to a v32fp16 dst, converting the fp32 source
/// elements to fp16 at high 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 32bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PHHE </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 1st source r32. Size is 4 Bytes.
/// \ret
///    The destination v32fp16 data. Size is 64 Bytes.
__DEFAULT_FN_ATTRS_AVX512
static __m512h __tile_tcvtrowps2phhee(__tile1024i src0, unsigned src1) {
  return _tile_tcvtrowps2phhee_internal(src0.row, src0.col, src0.tile, src1);
}

/// Move a row from a tile (src0) to a v32fp16 dst, converting the f3232 source
/// elements to fp16 at high 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 8bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PHHE </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source imm8. Size is 4 Bytes.
/// \ret
///    The destination v32fp16 data. Size is 64 Bytes.
#define __tile_tcvtrowps2phhei(src0, src1)                                     \
  _tile_tcvtrowps2phhei_internal(src0.row, src0.col, src0.tile, src1);

/// Move a row from a tile (src0) to a v32fp16 dst, converting the fp32 source
/// elements to fp16 at low 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 32bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PHLE </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 1st source r32. Size is 4 Bytes.
/// \ret
///    The destination v32fp16 data. Size is 64 Bytes.
__DEFAULT_FN_ATTRS_AVX512
static __m512h __tile_tcvtrowps2phlee(__tile1024i src0, unsigned src1) {
  return _tile_tcvtrowps2phlee_internal(src0.row, src0.col, src0.tile, src1);
}

/// Move a row from a tile (src0) to a v32fp16 dst, converting the f3232 source
/// elements to fp16 at low 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 8bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PHLE </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source imm8. Size is 4 Bytes.
/// \ret
///    The destination v32fp16 data. Size is 64 Bytes.
#define __tile_tcvtrowps2phlei(src0, src1)                                     \
  _tile_tcvtrowps2phlei_internal(src0.row, src0.col, src0.tile, src1);

#endif // __x86_64__
#endif // __AMX_AVX512_CVTROWINTRIN_H
