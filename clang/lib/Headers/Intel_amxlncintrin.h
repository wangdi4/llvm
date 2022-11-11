/* INTEL_FEATURE_ISA_AMX_LNC */
/*===---------- Intel_amxlncintrin.h - AMX_LNC intrinsics -*- C++ -*---------===
*
* Copyright (C) 2019 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxlncintrin.h> directly; use <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMX_LNCINTRIN_H
#define __AMX_LNCINTRIN_H
#ifdef __x86_64__

#define __DEFAULT_FN_ATTRS_TRANSPOSE                                           \
  __attribute__((__always_inline__, __nodebug__, __target__("amx-transpose")))

#define __DEFAULT_FN_ATTRS_AVX512                                              \
  __attribute__((__always_inline__, __nodebug__, __target__("amx-avx512")))

// Transpose
#define _tile_2rpntlvw(tdst, base, stride, src)                                \
  __builtin_ia32_t2rpntlvw(tdst, base, stride, src)
#define _tile_2rpntlvwt1(tdst, base, stride, src)                              \
  __builtin_ia32_t2rpntlvwt1(tdst, base, stride, src)
#define _tile_2rpntlvwz0(tdst, base, stride)                                   \
  __builtin_ia32_t2rpntlvwz0(tdst, base, stride)
#define _tile_2rpntlvwz0t1(tdst, base, stride)                                 \
  __builtin_ia32_t2rpntlvwz0t1(tdst, base, stride)
#define _tile_2rpntlvwz1(tdst, base, stride)                                   \
  __builtin_ia32_t2rpntlvwz1(tdst, base, stride)
#define _tile_2rpntlvwz1t1(tdst, base, stride)                                 \
  __builtin_ia32_t2rpntlvwz1t1(tdst, base, stride)

/// Compute transpose and dot product of BF16 pairs, accumulating to single
/// precision. This function has 3 tile operands, one source/dest accumulator
/// operand \a srcdst and two source operands, \a a and \a b. \a a is
/// transposed and matrix multiplied with \a b. The transpose operation is
/// done in BF16 pair granularity, transforming columns of BF16 pairs into rows.
/// The tile registers specified must be distinct from one another, no repeats.
/// Any attempt to execute an AMX instruction inside a TSX transaction will
/// result in a transaction abort.
///
/// \headerfile <immintrin.h>
///
/// \code
/// void __tile_tdpbf16ps(__tile srcdst, __tile a, __tile b);
/// \endcode
///
/// This intrinsic corresponds to the <c> TTDPBF16PS </c> instruction.
///
/// \param srcdst
/// 	The destination tile. Max size is 1024 Bytes.
/// \param a
/// 	The 1st source tile. Max size is 1024 Bytes.
/// \param b
/// 	The 2nd source tile. Max size is 1024 Bytes.
///
/// \code{.operation}
/// elements_dest:= srcdst.colsb/4
///
/// FOR m := 0 TO (srcdst.rows-1)
/// 	tmp[511:0] := 0
/// 	FOR k := 0 TO (a.rows-1)
/// 		FOR n := 0 TO (elements_dest-1)
/// 			tmp.fp32[2*n+0] += FP32(a.row[k].bfloat16[2*m+0]) * FP32(b.row[k].bfloat16[2*n+0])
/// 			tmp.fp32[2*n+1] += FP32(a.row[k].bfloat16[2*m+1]) * FP32(b.row[k].bfloat16[2*n+1])
/// 		ENDFOR
/// 	ENDFOR
/// 	
/// 	FOR n := 0 TO (elements_dest-1)
/// 		tmp2.f32[n] := tmp.fp32[2*n+0] + tmp.fp32[2*n+1]
/// 		tmp2.f32[n] += srcdst.row[m].fp32[n]
/// 	ENDFOR
/// 	write_row_and_zero(srcdst, m, tmp2, srcdst.colsb)
///
/// ENDFOR
///
/// zero_upper_rows(srcdst, srcdst.rows)
/// zero_tileconfig_start()
/// \endcode
#define _tile_tdpbf16ps(srcdst, a, b)                               \
  __builtin_ia32_ttdpbf16ps(srcdst, a, b)

/// Compute transpose and dot product of FP16 pairs, accumulating to single
/// precision. This function has 3 tile operands, one source/dest accumulator
/// operand srcdst and two source operands, a and b. a is transposed
/// and matrix multiplied with b. The transpose operation is done in FP16
/// pair granularity, transforming columns of FP16 pairs into rows. The tile
/// registers specified must be distinct from one another, no repeats. Any
/// attempt to execute an AMX instruction inside a TSX transaction will result
/// in a transaction abort.
///
/// \headerfile <immintrin.h>
///
/// \code
/// void __tile_tdpfp16ps(__tile srcdst, __tile a, __tile b);
/// \endcode
///
/// This intrinsic corresponds to the <c> TTDPFP16PS </c> instruction.
///
/// \param srcdst
/// 	The destination tile. Max size is 1024 Bytes.
/// \param a
/// 	The 1st source tile. Max size is 1024 Bytes.
/// \param b
/// 	The 2nd source tile. Max size is 1024 Bytes.
///
/// \code{.operation}
/// elements_dest:= srcdst.colsb/4
///
/// FOR m := 0 TO (srcdst.rows-1)
/// 	tmp[511:0] := 0
/// 	FOR k := 0 TO (a.rows-1)
/// 		FOR n := 0 TO (elements_dest-1)
/// 			tmp.fp32[2*n+0] += FP32(a.row[k].fp16[2*m+0]) * FP32(b.row[k].fp16[2*n+0])
/// 			tmp.fp32[2*n+1] += FP32(a.row[k].fp16[2*m+1]) * FP32(b.row[k].fp16[2*n+1])
/// 		ENDFOR
/// 	ENDFOR
/// 	
/// 	FOR n := 0 TO (elements_dest-1)
/// 		tmp2.f32[n] := tmp.fp32[2*n+0] + tmp.fp32[2*n+1]
/// 		tmp2.f32[n] += srcdst.row[m].fp32[n]
/// 	ENDFOR
/// 	write_row_and_zero(srcdst, m, tmp2, srcdst.colsb)
/// 	
/// ENDFOR
///
/// zero_upper_rows(srcdst, srcdst.rows)
/// zero_tileconfig_start()
/// \endcode
#define _tile_tdpfp16ps(srcdst, a, b)                                    \
  __builtin_ia32_ttdpfp16ps(srcdst, a, b)

/// Transpose 32-bit elements from \a src and write the result to \a dst.
///
/// \headerfile <immintrin.h>
///
/// \code
/// void __tile_transposed(__tile dst, __tile src);
/// \endcode
///
/// This intrinsic corresponds to the <c> TTRANSPOSED </c> instruction.
///
/// \param dst
/// 	The destination tile. Max size is 1024 Bytes.
/// \param src
/// 	The 1st source tile. Max size is 1024 Bytes.
///
/// \code{.operation}
///
/// FOR i := 0 TO (dst.rows-1)
/// 	tmp[511:0] := 0
/// 	FOR j := 0 TO (dst.colsb/4-1)
/// 		tmp.dword[j] := src.row[j].dword[i]
/// 	ENDFOR
/// 	dst.row[i] := tmp
/// ENDFOR
///
/// zero_upper_rows(dst, dst.rows)
/// zero_tileconfig_start()
/// \endcode
#define _tile_transposed(dst, src)                                          \
  __builtin_ia32_ttransposed(dst, src)

// AMX_LNC AVX512
#define _tile_tile16move(tdst, tsrc1, tsrc2, tsrc3, tsrc4, tsrc5, tsrc6, tsrc7,\
  tsrc8, tsrc9, tsrc10, tsrc11, tsrc12, tsrc13, tsrc14, tsrc15, tsrc16)        \
  __builtin_ia32_tile16move(tdst, tsrc1, tsrc2, tsrc3, tsrc4, tsrc5, tsrc6,    \
  tsrc7, tsrc8, tsrc9, tsrc10, tsrc11, tsrc12, tsrc13, tsrc14, tsrc15, tsrc16)

/// Move one row of a tile data to a v16f32 data.
/// The row of the tile is selected by an imm8.
///
/// \headerfile <immintrin.h>
///
/// \code
/// __m512 _tile_movrowi(__tile a, unsigned b);
/// \endcode
///
/// This intrinsic corresponds to the <c> TILEMOVROW </c> instruction.
///
/// \param a
/// 	The 1st source tile. Max size is 1024 Bytes.
/// \param b
/// 	The 2nd source imm8. Size is 4 Bytes.
/// \returns
/// 	The destination v16f32 data. Size is 64 Bytes.
///
/// \code{.operation}
/// VL := 512
/// VL_bytes := VL>>3
/// row_index := b&0x3f
/// row_chunk := (b>>6) * VL_bytes
/// FOR i := 0 TO (VL_bytes-1)
/// 	IF (row_chunk + i >= a.colsb)
/// 		dst.byte[i] := 0
/// 	ELSE
/// 		dst.byte[i] := a.row[row_index].byte[row_chunk+i]
/// ENDFOR
/// \endcode
#define _tile_movrowi(a, b)  __builtin_ia32_tilemovei(a, b)

/// Move one row of a tile data to a v16f32 data.
/// The row of the tile is selected by a 32b GPR.
///
/// \headerfile <immintrin.h>
///
/// \code
/// __m512 _tile_movrow(__tile a, unsigned b);
/// \endcode
///
/// This intrinsic corresponds to the <c> TILEMOVROW </c> instruction.
///
/// \param a
/// 	The 1st source tile. Max size is 1024 Bytes.
/// \param b
/// 	The 2nd source r32. Size is 4 Bytes.
/// \returns
/// 	The destination v16f32 data. Size is 64 Bytes.
///
/// \code{.operation}
/// VL := 512
/// VL_bytes := VL>>3
/// row_index := b&0xffff
/// row_chunk := ((b>>16)&0xffff) * VL_bytes
/// FOR i := 0 TO (VL_bytes-1)
/// 	IF (row_chunk + i >= a.colsb)
/// 		dst.byte[i] := 0
/// 	ELSE
/// 		dst.byte[i] := a.row[row_index].byte[row_chunk+i]
/// ENDFOR
/// \endcode
#define _tile_movrow(a, b)  __builtin_ia32_tilemovee(a, b)
#define _tile_movrowx(a, b)  __builtin_ia32_tilemovex(a, b)
// BF16EVEX
#define _tile_dpbf16pse(tsrc1_dst, tsrc2, tsrc3)                               \
  __builtin_ia32_tdpbf16pse(tsrc1_dst, tsrc2, tsrc3)

// INT8EVEX
#define _tile_dpbssde(tsrc1_dst, tsrc2, tsrc3)                                 \
  __builtin_ia32_tdpbssde(tsrc1_dst, tsrc2, tsrc3)
#define _tile_dpbsude(tsrc1_dst, tsrc2, tsrc3)                                 \
  __builtin_ia32_tdpbsude(tsrc1_dst, tsrc2, tsrc3)
#define _tile_dpbusde(tsrc1_dst, tsrc2, tsrc3)                                 \
  __builtin_ia32_tdpbusde(tsrc1_dst, tsrc2, tsrc3)
#define _tile_dpbuude(tsrc1_dst, tsrc2, tsrc3)                                 \
  __builtin_ia32_tdpbuude(tsrc1_dst, tsrc2, tsrc3)

// TILEEVEX
#define _tile_loadde(dst, base, stride)  __builtin_ia32_tileloadde64((dst),    \
  ((const void *)(base)), (__SIZE_TYPE__)(stride))
#define _tile_stream_loadde(dst, base, stride)                                 \
  __builtin_ia32_tileloaddt1e64((dst), ((const void *)(base)),                 \
  (__SIZE_TYPE__)(stride))
#define _tile_storede(src, base, stride)                                       \
  __builtin_ia32_tilestorede64((src), ((void *)(base)),                        \
  (__SIZE_TYPE__)(stride))
#define _tile_tilemove(tdst, tsrc)  __builtin_ia32_tilemove(tdst, tsrc)
#define _tile_zeroe(tile)       __builtin_ia32_tilezeroe(tile)

//AMX_LNC ELEMENTEVEX
#define _tile_cvtd2pse(base, stride, tsrc)                                    \
  __builtin_ia32_tcvtd2pse(base, stride, tsrc)

// This is internal intrinsic. C/C++ user should avoid calling it directly.
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_transposed_internal(unsigned short m, unsigned short n, _tile1024i src) {
  return __builtin_ia32_ttransposed_internal(m, n, src);
}

// dst = m x n (srcdest), src1 = k x m, src2 = k x n
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_tdpbf16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                         _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_ttdpbf16ps_internal(m, n, k, dst, src1, src2);
}

// dst = m x n (srcdest), src1 = k x m, src2 = k x n
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_tdpfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                         _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_ttdpfp16ps_internal(m, n, k, dst, src1, src2);
}

static __inline__ __m512 __DEFAULT_FN_ATTRS_AVX512
_tile_movrow_internal(unsigned short m, unsigned short n,
                      _tile1024i src, unsigned u) {
  return __builtin_ia32_tilemovrowe_internal(m, n, src, u);
}

#define _tile_movrowi_internal(m, n, tsrc, i)                            \
  __builtin_ia32_tilemovrowi_internal(m, n, tsrc, i)

/// Transpose 32-bit elements from src and write the result to dst.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TTRANSPOSED </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src
///    The 1st source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_TRANSPOSE
static void __tile_transposed(__tile1024i *dst, __tile1024i src) {
  dst->tile = _tile_transposed_internal(src.row, src.col, src.tile);
}

/// Compute transpose and dot product of BF16 pairs, accumulating to single
/// precision. This function has 3 tile operands, one source/dest accumulator
/// operand dst and two source operands, src0 and src1. Src0 is transposed and
/// matrix multiplied with src1. The transpose operation is done in BF16 pair
/// granularity, transforming columns of BF16 pairs into rows. The tile
/// registers specified must be distinct from one another, no repeats. Any
/// attempt to execute an AMX instruction inside a TSX transaction will result
/// in a transaction abort.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TTDPBF16PS </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_TRANSPOSE
static void __tile_tdpbf16ps(__tile1024i *dst, __tile1024i src0,
                             __tile1024i src1) {
  dst->tile = _tile_tdpbf16ps_internal(src0.col / 4, src1.col,
                           src1.row * 4, dst->tile, src0.tile, src1.tile);
}

/// Compute transpose and dot product of FP16 pairs, accumulating to single
/// precision. This function has 3 tile operands, one source/dest accumulator
/// operand dst and two source operands, src0 and src1. Src0 is transposed and
/// matrix multiplied with src1. The transpose operation is done in FP16 pair
/// granularity, transforming columns of FP16 pairs into rows. The tile
/// registers specified must be distinct from one another, no repeats. Any
/// attempt to execute an AMX instruction inside a TSX transaction will result
/// in a transaction abort.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TTDPFP16PS </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_TRANSPOSE
static void __tile_tdpfp16ps(__tile1024i *dst, __tile1024i src0,
                             __tile1024i src1) {
  dst->tile = _tile_tdpfp16ps_internal(src0.col / 4, src1.col,
                           src1.row * 4, dst->tile, src0.tile, src1.tile);
}

/// Move one row of a tile data to a v16f32 data.
/// The row of the tile is selected by a 32b GPR.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TILEMOVROW </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 1st source r32. Size is 4 Bytes.
/// \returns
///    The destination v16f32 data. Size is 64 Bytes.
__DEFAULT_FN_ATTRS_AVX512
static __m512 __tile_movrow(__tile1024i src0, unsigned src1) {
  return _tile_movrow_internal(src0.row, src0.col, src0.tile, src1);
}

/// Move one row of a tile data to a v16f32 data.
/// The row of the tile is selected by an imm8.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TILEMOVROW </c> instruction.
///
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 1st source imm8. Size is 4 Bytes.
/// \returns
///    The destination v16f32 data. Size is 64 Bytes.
#define __tile_movrowi(src0, src1)                                     \
  _tile_movrowi_internal(src0.row, src0.col, src0.tile, src1);

#endif /* __x86_64__ */
#endif /* __AMX_LNCINTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_LNC */
