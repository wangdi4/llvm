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

// Transpose
#define _tile_2rpntlvw(tdst, base, stride, src)                                \
  __builtin_ia32_t2rpntlvw(tdst, base, stride, src)
#define _tile_2rpntlvwt1(tdst, base, stride, src)                              \
  __builtin_ia32_t2rpntlvwt1(tdst, base, stride, src)
#define _tile_2transposew(tdst, base, stride, src)                             \
  __builtin_ia32_t2transposew(tdst, base, stride, src)
#define _tile_2transposewt1(tdst, base, stride, src)                           \
  __builtin_ia32_t2transposewt1(tdst, base, stride, src)

// AMX_LNC AVX512
#define _tile_tile16move(tdst, tsrc1, tsrc2, tsrc3, tsrc4, tsrc5, tsrc6, tsrc7,\
  tsrc8, tsrc9, tsrc10, tsrc11, tsrc12, tsrc13, tsrc14, tsrc15, tsrc16)        \
  __builtin_ia32_tile16move(tdst, tsrc1, tsrc2, tsrc3, tsrc4, tsrc5, tsrc6,    \
  tsrc7, tsrc8, tsrc9, tsrc10, tsrc11, tsrc12, tsrc13, tsrc14, tsrc15, tsrc16)

#define _tile_tilemovrowei(tsrc1, tsrc2)  __builtin_ia32_tilemovei(tsrc1, tsrc2)
#define _tile_tilemovrowee(tsrc1, tsrc2)  __builtin_ia32_tilemovee(tsrc1, tsrc2)
#define _tile_tilemovrowex(tsrc1, tsrc2)  __builtin_ia32_tilemovex(tsrc1, tsrc2)
#define _tile_cvtrowd2psei(tsrc1, src2)                                        \
  __builtin_ia32_tcvtrowd2psei(tsrc1, src2)
#define _tile_cvtrowd2psee(tsrc1, src2)                                        \
  __builtin_ia32_tcvtrowd2psee(tsrc1, src2)

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

#endif /* __x86_64__ */
#endif /* __AMX_LNCINTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_LNC */
