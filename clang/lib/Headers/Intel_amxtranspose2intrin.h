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

#endif /* __x86_64__ */
#endif /* __AMX_TRANSPOSE2INTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_TRANSPOSE2 */
