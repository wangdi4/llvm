/* INTEL_FEATURE_ISA_AMX_TILE_EVEX */
/*===----- Intel_amxtileevexintrin.h - AMX_TILE_EVEX intrinsics -*- C++ -*---===
*
* Copyright (C) 2019 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxtileevexintrin.h> directly; use <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMX_TILE_EVEXINTRIN_H
#define __AMX_TILE_EVEXINTRIN_H
#ifdef __x86_64__

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

#endif /* __x86_64__ */
#endif /* __AMX_TILE_EVEXINTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_TILE_EVEX */
