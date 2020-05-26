/* INTEL_FEATURE_ISA_AMX_TILE2 */
/*===-- Intel_amxtile2intrin.h - AMX_TILE2 intrinsics -*- C++ -*---===
*
* Copyright (C) 2020 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxtile2intrin.h> directly; use <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMX_TILE2INTRIN_H
#define __AMX_TILE2INTRIN_H
#ifdef __x86_64__

// TILE2
#define _tile_tilemov(dst, tsrc)      \
  __builtin_ia32_tilemov(dst, tsrc)

#endif /* __x86_64__ */
#endif /* __AMX_TILE2INTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_TILE2 */
