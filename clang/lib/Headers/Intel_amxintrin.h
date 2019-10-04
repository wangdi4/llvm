/* INTEL_FEATURE_ISA_AMX */
/*===------------ Intel_amxintrin.h - AMX intrinsics -*- C++ -*--------------===
*
* Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxintrin.h> directly; include <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMXINTRIN_H
#define __AMXINTRIN_H
#ifdef __x86_64__

#define __DEFAULT_FN_ATTRS \
  __attribute__((__always_inline__, __nodebug__,  __target__("amx-tile")))

static __inline__ void __DEFAULT_FN_ATTRS
_tile_loadconfig(const void *__config)
{
  __builtin_ia32_tile_loadconfig(__config);
}

static __inline__ void __DEFAULT_FN_ATTRS
_tile_storeconfig(void *__config)
{
  __builtin_ia32_tile_storeconfig(__config);
}

static __inline__ void __DEFAULT_FN_ATTRS
_tile_release(void)
{
  __builtin_ia32_tilerelease();
}

#define _tile_loadd(dst, base, stride) \
  __builtin_ia32_tileloadd64((dst), ((const void *)(base)), (__SIZE_TYPE__)(stride))
#define _tile_stream_loadd(dst, base, stride) \
  __builtin_ia32_tileloaddt164((dst), ((const void *)(base)), (__SIZE_TYPE__)(stride))
#define _tile_stored(dst, base, stride) \
  __builtin_ia32_tilestored64((dst), ((void *)(base)), (__SIZE_TYPE__)(stride))

#define _tile_zero(tile) \
  __builtin_ia32_tilezero((tile))

#define _tile_dpbssd __builtin_ia32_tdpbssd
#define _tile_dpbsud __builtin_ia32_tdpbsud
#define _tile_dpbusd __builtin_ia32_tdpbusd
#define _tile_dpbuud __builtin_ia32_tdpbuud
#define _tile_dpbf16ps __builtin_ia32_tdpbf16ps

#undef __DEFAULT_FN_ATTRS

#endif /* __x86_64__ */
#endif /* __AMXINTRIN_H */
/* end INTEL_FEATURE_ISA_AMX */
