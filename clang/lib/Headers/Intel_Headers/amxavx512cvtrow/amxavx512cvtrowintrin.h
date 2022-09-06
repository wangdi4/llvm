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

/// Moves a row from a tile register to a zmm destination register, converting
///    the int32 source elements to fp32. The row of the tile is selected by an
///    32b GPR.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _tile_cvtrowd2pse(__tile tsrc, unsigned int row);
/// \endcode
///
/// \code{.operation}
/// VL := 512
/// VL_bytes := VL >> 3
/// row_index := row & 0xffff
/// row_chunk := ((row >> 16) & 0xffff) * VL_bytes
/// FOR i := 0 TO (VL_bytes / 4) - 1
///     IF i + row_chunk / 4 >= tsrc.colsb / 4
///         dst.dword[i] := 0
///     ELSE
///         dst.f32[i] := CONVERT_INT32_TO_FP32(tsrc.row[row_index].dword[row_chunk/4+i], RNE)
///     FI
/// ENDFOR
/// dst[MAX_VL-1:VL] := 0
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCVTROWD2PS instruction.
///
/// \param tsrc
///    The 1st source tile. Max size is 1024 Bytes.
/// \param row
///    The row of the source tile
#define _tile_cvtrowd2pse(tsrc, row) __builtin_ia32_tcvtrowd2pse(tsrc, row)

/// Moves a row from a tile register to a zmm destination register, converting
///    the int32 source elements to fp32. The row of the tile is selected by an
///    imm8.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _tile_cvtrowd2psi(__tile tsrc, const unsigned int Imm);
/// \endcode
///
/// \code{.operation}
/// VL := 512
/// VL_bytes := VL >> 3
/// row := Imm
/// row_index := imm8 & 0x3f
/// row_chunk := (imm8 >> 6) * VL_bytes
/// FOR i := 0 TO (VL_bytes / 4) - 1
///     IF i + row_chunk / 4 >= tsrc.colsb / 4
///         dst.dword[i] := 0
///     ELSE
///         dst.f32[i] := CONVERT_INT32_TO_FP32(tsrc.row[row_index].dword[row_chunk/4+i], RNE)
///     FI
/// ENDFOR
/// dst[MAX_VL-1:VL] := 0
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCVTROWD2PS instruction.
///
/// \param tsrc
///    The source tile. Max size is 1024 Bytes.
/// \param Imm
///    The row of the source tile
#define _tile_cvtrowd2psi(tsrc, Imm) __builtin_ia32_tcvtrowd2psi(tsrc, Imm)

/// Moves a row from a tile register to a zmm destination register, converting
///    the fp32 source elements to bf16. It places the resulting bf16 elements
///    in the high 16 bits within each dword. The row of the tile is selected
///    by an 32b GPR.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _tile_cvtrowps2pbf16he(__tile tsrc, unsigned int row);
/// \endcode
///
/// \code{.operation}
/// VL := 512
/// VL_bytes := VL >> 3
/// row_index := row & 0xffff
/// row_chunk := ((row >> 16) & 0xffff) * VL_bytes
/// FOR i := 0 TO (VL_bytes / 4) - 1
///     IF i + row_chunk / 4 >= tsrc.colsb / 4
///         dst.dword[i] := 0
///     ELSE
///         dst.word[2*i+0] := 0
///         dst.bf16[2*i+1] := CONVERT_FP32_TO_BF16(tsrc.row[row_index].fp32[row_chunk/4+i], RNE)
///     FI
/// ENDFOR
/// dst[MAX_VL-1:VL] := 0
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCVTROWPS2PBF16H instruction.
///
/// \param tsrc
///    The source tile. Max size is 1024 Bytes.
/// \param row
///    The the row of the source tile.
#define _tile_cvtrowps2pbf16he(tsrc, row)                                       \
  __builtin_ia32_tcvtrowps2pbf16he(tsrc, row)

/// Moves a row from a tile register to a zmm destination register, converting
///    the fp32 source elements to bf16. It places the resulting bf16 elements
///    in the high 16 bits within each dword. The row of the tile is selected
///    by an immediate.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _tile_cvtrowps2pbf16hi(__tile tsrc, const unsigned int Imm);
/// \endcode
///
/// \code{.operation}
/// VL := 512
/// VL_bytes := VL >> 3
/// row := Imm
/// row_index := imm8 & 0x3f
/// row_chunk := (imm8 >> 6) * VL_bytes
/// FOR i := 0 TO (VL_bytes / 4) - 1
///     IF i + row_chunk / 4 >= tsrc.colsb / 4
///         dst.dword[i] := 0
///     ELSE
///         dst.word[2*i+0] := 0
///         dst.bf16[2*i+1] := CONVERT_FP32_TO_BF16(tsrc.row[row_index].fp32[row_chunk/4+i], RNE)
///     FI
/// ENDFOR
/// dst[MAX_VL-1:VL] := 0
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCVTROWPS2PBF16H instruction.
///
/// \param tsrc
///    The source tile. Max size is 1024 Bytes.
/// \param Imm
///    The the row of the source tile.
#define _tile_cvtrowps2pbf16hi(tsrc, Imm)                                     \
  __builtin_ia32_tcvtrowps2pbf16hi(tsrc, Imm)

/// Moves a row from a tile register to a zmm destination register, converting
///    the fp32 source elements to bf16. It places the resulting bf16 elements
///    in the low 16 bits within each dword. The row of the tile is selected
///    by an 32b GPR.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _tile_cvtrowps2pbf16le(__tile tsrc, unsigned int row);
/// \endcode
///
/// \code{.operation}
/// VL := 512
/// VL_bytes := VL >> 3
/// row_index := row & 0xffff
/// row_chunk := ((row >> 16) & 0xffff) * VL_bytes
/// FOR i := 0 TO (VL_bytes / 4) - 1
///     IF i + row_chunk / 4 >= tsrc.colsb / 4
///         dst.dword[i] := 0
///     ELSE
///         dst.word[2*i+1] := 0
///         dst.bf16[2*i+0] := CONVERT_FP32_TO_BF16(tsrc.row[row_index].fp32[row_chunk/4+i], RNE)
///     FI
/// ENDFOR
/// dst[MAX_VL-1:VL] := 0
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCVTROWPS2PBF16L instruction.
///
/// \param tsrc
///    The source tile. Max size is 1024 Bytes.
/// \param row
///    The the row of the source tile.
#define _tile_cvtrowps2pbf16le(tsrc, row)                                       \
  __builtin_ia32_tcvtrowps2pbf16le(tsrc, row)

/// Moves a row from a tile register to a zmm destination register, converting
///    the fp32 source elements to bf16. It places the resulting bf16 elements
///    in the low 16 bits within each dword. The row of the tile is selected
///    by an immediate.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _tile_cvtrowps2pbf16hi(__tile tsrc, const unsigned int Imm);
/// \endcode
///
/// \code{.operation}
/// VL := 512
/// VL_bytes := VL >> 3
/// row := Imm
/// row_index := imm8 & 0x3f
/// row_chunk := (imm8 >> 6) * VL_bytes
/// FOR i := 0 TO (VL_bytes / 4) - 1
///     IF i + row_chunk / 4 >= tsrc.colsb / 4
///         dst.dword[i] := 0
///     ELSE
///         dst.word[2*i+1] := 0
///         dst.bf16[2*i+0] := CONVERT_FP32_TO_BF16(tsrc.row[row_index].fp32[row_chunk/4+i], RNE)
///     FI
/// ENDFOR
/// dst[MAX_VL-1:VL] := 0
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCVTROWPS2PBF16H instruction.
///
/// \param tsrc
///    The source tile. Max size is 1024 Bytes.
/// \param Imm
///    The the row of the source tile.
#define _tile_cvtrowps2pbf16li(tsrc, Imm)                                     \
  __builtin_ia32_tcvtrowps2pbf16li(tsrc, Imm)

/// Moves a row from a tile register to a zmm destination register, converting
///    the fp32 source elements to fp16. It places the resulting fp16 elements
///    in the high 16 bits within each dword. The row of the tile is selected
///    by an 32b GPR.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _tile_cvtrowps2phhe(__tile tsrc, unsigned int row);
/// \endcode
///
/// \code{.operation}
/// VL := 512
/// VL_bytes := VL >> 3
/// row_index := row & 0xffff
/// row_chunk := ((row >> 16) & 0xffff) * VL_bytes
/// FOR i := 0 TO (VL_bytes / 4) - 1
///     IF i + row_chunk / 4 >= tsrc.colsb / 4
///         dst.dword[i] := 0
///     ELSE
///         dst.word[2*i+0] := 0
///         dst.fp16[2*i+1] := CONVERT_FP32_TO_FP16(tsrc.row[row_index].fp32[row_chunk/4+i], RNE)
///     FI
/// ENDFOR
/// dst[MAX_VL-1:VL] := 0
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCVTROWPS2PHH instruction.
///
/// \param tsrc
///    The source tile. Max size is 1024 Bytes.
/// \param row
///    The the row of the source tile.
#define _tile_cvtrowps2phhe(tsrc, row) __builtin_ia32_tcvtrowps2phhe(tsrc, row)

/// Moves a row from a tile register to a zmm destination register, converting
///    the fp32 source elements to fp16. It places the resulting fp16 elements
///    in the high 16 bits within each dword. The row of the tile is selected
///    by an immediate.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _tile_cvtrowps2phhi(__tile tsrc, const unsigned int Imm);
/// \endcode
///
/// \code{.operation}
/// VL := 512
/// VL_bytes := VL >> 3
/// row := Imm
/// row_index := imm8 & 0x3f
/// row_chunk := (imm8 >> 6) * VL_bytes
/// FOR i := 0 TO (VL_bytes / 4) - 1
///     IF i + row_chunk / 4 >= tsrc.colsb / 4
///         dst.dword[i] := 0
///     ELSE
///         dst.word[2*i+0] := 0
///         dst.fp16[2*i+1] := CONVERT_FP32_TO_FP16(tsrc.row[row_index].fp32[row_chunk/4+i], RNE)
///     FI
/// ENDFOR
/// dst[MAX_VL-1:VL] := 0
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCVTROWPS2PHH instruction.
///
/// \param tsrc
///    The source tile. Max size is 1024 Bytes.
/// \param Imm
///    The the row of the source tile.
#define _tile_cvtrowps2phhi(tsrc, Imm)                                        \
  __builtin_ia32_tcvtrowps2phhi(tsrc, Imm)

/// Moves a row from a tile register to a zmm destination register, converting
///    the fp32 source elements to fp16. It places the resulting fp16 elements
///    in the low 16 bits within each dword. The row of the tile is selected
///    by an 32b GPR.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _tile_cvtrowps2phle(__tile tsrc, unsigned int row);
/// \endcode
///
/// \code{.operation}
/// VL := 512
/// VL_bytes := VL >> 3
/// row_index := row & 0xffff
/// row_chunk := ((row >> 16) & 0xffff) * VL_bytes
/// FOR i := 0 TO (VL_bytes / 4) - 1
///     IF i + row_chunk / 4 >= tsrc.colsb / 4
///         dst.dword[i] := 0
///     ELSE
///         dst.word[2*i+1] := 0
///         dst.fp16[2*i+0] := CONVERT_FP32_TO_FP16(tsrc.row[row_index].fp32[row_chunk/4+i], RNE)
///     FI
/// ENDFOR
/// dst[MAX_VL-1:VL] := 0
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCVTROWPS2PHL instruction.
///
/// \param tsrc
///    The source tile. Max size is 1024 Bytes.
/// \param row
///    The the row of the source tile.
#define _tile_cvtrowps2phle(tsrc, row) __builtin_ia32_tcvtrowps2phle(tsrc, row)

/// Moves a row from a tile register to a zmm destination register, converting
///    the fp32 source elements to fp16. It places the resulting fp16 elements
///    in the low 16 bits within each dword. The row of the tile is selected
///    by an immediate.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _tile_cvtrowps2phli(__tile tsrc, const unsigned int Imm);
/// \endcode
///
/// \code{.operation}
/// VL := 512
/// VL_bytes := VL >> 3
/// row := Imm
/// row_index := imm8 & 0x3f
/// row_chunk := (imm8 >> 6) * VL_bytes
/// FOR i := 0 TO (VL_bytes / 4) - 1
///     IF i + row_chunk / 4 >= tsrc.colsb / 4
///         dst.dword[i] := 0
///     ELSE
///         dst.word[2*i+1] := 0
///         dst.fp16[2*i+0] := CONVERT_FP32_TO_FP16(tsrc.row[row_index].fp32[row_chunk/4+i], RNE)
///     FI
/// ENDFOR
/// dst[MAX_VL-1:VL] := 0
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCVTROWPS2PHL instruction.
///
/// \param tsrc
///    The source tile. Max size is 1024 Bytes.
/// \param Imm
///    The the row of the source tile.
#define _tile_cvtrowps2phli(tsrc, Imm)                                        \
  __builtin_ia32_tcvtrowps2phli(tsrc, Imm)

/// This is internal intrinsic. C/C++ user should avoid calling it directly.

static __inline__ __m512 __DEFAULT_FN_ATTRS_AVX512
_tile_cvtrowd2pse_internal(unsigned short m, unsigned short n,
                           _tile1024i src, unsigned u) {
  return __builtin_ia32_tcvtrowd2pse_internal(m, n, src, u);
}

#define _tile_cvtrowd2psi_internal(m, n, tsrc, i)                            \
  __builtin_ia32_tcvtrowd2psi_internal(m, n, tsrc, i)

static __inline__ __m512bh __DEFAULT_FN_ATTRS_AVX512
_tile_cvtrowps2pbf16he_internal(unsigned short m, unsigned short n,
                                _tile1024i src, unsigned u) {
  return __builtin_ia32_tcvtrowps2pbf16he_internal(m, n, src, u);
}

#define _tile_cvtrowps2pbf16hi_internal(m, n, tsrc, i)                       \
  __builtin_ia32_tcvtrowps2pbf16hi_internal(m, n, tsrc, i)

static __inline__ __m512bh __DEFAULT_FN_ATTRS_AVX512
_tile_cvtrowps2pbf16le_internal(unsigned short m, unsigned short n,
                                _tile1024i src, unsigned u) {
  return __builtin_ia32_tcvtrowps2pbf16le_internal(m, n, src, u);
}

#define _tile_cvtrowps2pbf16li_internal(m, n, tsrc, i)                       \
  __builtin_ia32_tcvtrowps2pbf16li_internal(m, n, tsrc, i)

static __inline__ __m512h __DEFAULT_FN_ATTRS_AVX512
_tile_cvtrowps2phhe_internal(unsigned short m, unsigned short n,
                             _tile1024i src, unsigned u) {
  return __builtin_ia32_tcvtrowps2phhe_internal(m, n, src, u);
}

#define _tile_cvtrowps2phhi_internal(m, n, tsrc, i)                       \
  __builtin_ia32_tcvtrowps2phhi_internal(m, n, tsrc, i)

static __inline__ __m512h __DEFAULT_FN_ATTRS_AVX512
_tile_cvtrowps2phle_internal(unsigned short m, unsigned short n,
                             _tile1024i src, unsigned u) {
  return __builtin_ia32_tcvtrowps2phle_internal(m, n, src, u);
}

#define _tile_cvtrowps2phli_internal(m, n, tsrc, i)                       \
  __builtin_ia32_tcvtrowps2phli_internal(m, n, tsrc, i)

/// Move a row from a tile (src0) to a v16f32 dst, converting the int32 source
/// elements to fp32. No SIMD exceptions are generated. Rounding is done as if
/// MXCSR.RC=RNE. Embedded rounding is not supported.
/// The row and chunk elements of tile is fetched from 32bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWD2PS </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 1st source r32. Size is 4 Bytes.
/// \ret
///    The destination v16f32 data. Size is 64 Bytes.
__DEFAULT_FN_ATTRS_AVX512
static __m512 __tile_cvtrowd2pse(__tile1024i src0, unsigned src1) {
  return _tile_cvtrowd2pse_internal(src0.row, src0.col, src0.tile, src1);
}

/// Move a row from a tile (src0) to a v16f32 dst, converting the int32 source
/// elements to fp32. No SIMD exceptions are generated. Rounding is done as if
/// MXCSR.RC=RNE. Embedded rounding is not supported.
/// The row and chunk elements of tile is fetched from 8bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWD2PS </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source imm8. Size is 4 Bytes.
/// \ret
///    The destination v16f32 data. Size is 64 Bytes.
#define __tile_cvtrowd2psi(src0, src1)                                     \
  _tile_cvtrowd2psi_internal(src0.row, src0.col, src0.tile, src1);

/// Move a row from a tile (src0) to a v32bf16 dst, converting the fp32 source
/// elements to bf16 at high 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 32bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PBF16H </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 1st source r32. Size is 4 Bytes.
/// \ret
///    The destination v32bf16 data. Size is 64 Bytes.
__DEFAULT_FN_ATTRS_AVX512
static __m512bh __tile_cvtrowps2pbf16he(__tile1024i src0, unsigned src1) {
  return _tile_cvtrowps2pbf16he_internal(src0.row, src0.col, src0.tile, src1);
}

/// Move a row from a tile (src0) to a v32bf16 dst, converting the f32 source
/// elements to bf16 at high 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 8bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PBF16H </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source imm8. Size is 4 Bytes.
/// \ret
///    The destination v32bf16 data. Size is 64 Bytes.
#define __tile_cvtrowps2pbf16hi(src0, src1)                                     \
  _tile_cvtrowps2pbf16hi_internal(src0.row, src0.col, src0.tile, src1);

/// Move a row from a tile (src0) to a v32bf16 dst, converting the fp32 source
/// elements to bf16 at low 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 32bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PBF16L </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 1st source r32. Size is 4 Bytes.
/// \ret
///    The destination v32bf16 data. Size is 64 Bytes.
__DEFAULT_FN_ATTRS_AVX512
static __m512bh __tile_cvtrowps2pbf16le(__tile1024i src0, unsigned src1) {
  return _tile_cvtrowps2pbf16le_internal(src0.row, src0.col, src0.tile, src1);
}

/// Move a row from a tile (src0) to a v32bf16 dst, converting the f3232 source
/// elements to bf16 at low 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 8bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PBF16L </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source imm8. Size is 4 Bytes.
/// \ret
///    The destination v32bf16 data. Size is 64 Bytes.
#define __tile_cvtrowps2pbf16li(src0, src1)                                     \
  _tile_cvtrowps2pbf16li_internal(src0.row, src0.col, src0.tile, src1);

/// Move a row from a tile (src0) to a v32fp16 dst, converting the fp32 source
/// elements to fp16 at high 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 32bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PHH </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 1st source r32. Size is 4 Bytes.
/// \ret
///    The destination v32fp16 data. Size is 64 Bytes.
__DEFAULT_FN_ATTRS_AVX512
static __m512h __tile_cvtrowps2phhe(__tile1024i src0, unsigned src1) {
  return _tile_cvtrowps2phhe_internal(src0.row, src0.col, src0.tile, src1);
}

/// Move a row from a tile (src0) to a v32fp16 dst, converting the f3232 source
/// elements to fp16 at high 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 8bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PHH </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source imm8. Size is 4 Bytes.
/// \ret
///    The destination v32fp16 data. Size is 64 Bytes.
#define __tile_cvtrowps2phhi(src0, src1)                                     \
  _tile_cvtrowps2phhi_internal(src0.row, src0.col, src0.tile, src1);

/// Move a row from a tile (src0) to a v32fp16 dst, converting the fp32 source
/// elements to fp16 at low 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 32bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PHL </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 1st source r32. Size is 4 Bytes.
/// \ret
///    The destination v32fp16 data. Size is 64 Bytes.
__DEFAULT_FN_ATTRS_AVX512
static __m512h __tile_cvtrowps2phle(__tile1024i src0, unsigned src1) {
  return _tile_cvtrowps2phle_internal(src0.row, src0.col, src0.tile, src1);
}

/// Move a row from a tile (src0) to a v32fp16 dst, converting the f3232 source
/// elements to fp16 at low 16-bits of each dword.
/// The row and chunk elements of tile is fetched from 8bit src1.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCVTROWPS2PHL </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source imm8. Size is 4 Bytes.
/// \ret
///    The destination v32fp16 data. Size is 64 Bytes.
#define __tile_cvtrowps2phli(src0, src1)                                     \
  _tile_cvtrowps2phli_internal(src0.row, src0.col, src0.tile, src1);

#endif // __x86_64__
#endif // __AMX_AVX512_CVTROWINTRIN_H
