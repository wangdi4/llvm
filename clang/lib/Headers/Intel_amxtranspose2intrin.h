/* INTEL_FEATURE_ISA_AMX_TRANSPOSE2 */
/*===-- Intel_amxtranspose2intrin.h - AMX_TRANSPOSE2 intrinsics -*- C++ -*---===
*
* Copyright (C) 2019 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxtranspose2intrin.h> directly; use <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMX_TRANSPOSE2INTRIN_H
#define __AMX_TRANSPOSE2INTRIN_H
#ifdef __x86_64__

// Transpose2
#define _tile_2transposew(tdst, base, stride, src)                             \
  __builtin_ia32_t2transposew(tdst, base, stride, src)
#define _tile_2transposewt1(tdst, base, stride, src)                           \
  __builtin_ia32_t2transposewt1(tdst, base, stride, src)
#define _tile_4rqntlvbz0(tdst, base, stride)                                   \
  __builtin_ia32_t4rqntlvbz0(tdst, base, stride)
#define _tile_4rqntlvbz0t1(tdst, base, stride)                                 \
  __builtin_ia32_t4rqntlvbz0t1(tdst, base, stride)
#define _tile_4rqntlvbz1(tdst, base, stride)                                   \
  __builtin_ia32_t4rqntlvbz1(tdst, base, stride)
#define _tile_4rqntlvbz1t1(tdst, base, stride)                                 \
  __builtin_ia32_t4rqntlvbz1t1(tdst, base, stride)
#define _tile_4rqntlvbz2(tdst, base, stride)                                   \
  __builtin_ia32_t4rqntlvbz2(tdst, base, stride)
#define _tile_4rqntlvbz2t1(tdst, base, stride)                                 \
  __builtin_ia32_t4rqntlvbz2t1(tdst, base, stride)
#define _tile_4rqntlvbz3(tdst, base, stride)                                   \
  __builtin_ia32_t4rqntlvbz3(tdst, base, stride)
#define _tile_4rqntlvbz3t1(tdst, base, stride)                                 \
  __builtin_ia32_t4rqntlvbz3t1(tdst, base, stride)
#define _tile_tdpbssd(tdst, tsrc1, tsrc2)                                      \
  __builtin_ia32_ttdpbssd(tdst, tsrc1, tsrc2)
#define _tile_tdpbsud(tdst, tsrc1, tsrc2)                                      \
  __builtin_ia32_ttdpbsud(tdst, tsrc1, tsrc2)
#define _tile_tdpbusd(tdst, tsrc1, tsrc2)                                      \
  __builtin_ia32_ttdpbusd(tdst, tsrc1, tsrc2)
#define _tile_tdpbuud(tdst, tsrc1, tsrc2)                                      \
  __builtin_ia32_ttdpbuud(tdst, tsrc1, tsrc2)

#endif /* __x86_64__ */
#endif /* __AMX_TRANSPOSE2INTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_TRANSPOSE2 */
