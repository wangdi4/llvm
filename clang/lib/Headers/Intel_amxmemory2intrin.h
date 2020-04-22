/* INTEL_FEATURE_ISA_AMX_MEMORY2 */
/*===----- Intel_amxmemory2intrin.h - AMX_MEMORY2 intrinsics -*- C++ -*------===
*
* Copyright (C) 2019 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxmemory2intrin.h> directly;"                        \
       " include <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMX_MEMORY2INTRIN_H
#define __AMX_MEMORY2INTRIN_H
#ifdef __x86_64__

// Memory
#define _tile_broadcastrowd(tdst, mem_src)                                     \
  __builtin_ia32_tbroadcastrowd(tdst, mem_src)
#define _tile_storehd(base, stride, tsrc)                                      \
  __builtin_ia32_tstorehd(base, stride, tsrc)
#define _tile_storehdt1(base, stride, tsrc)                                    \
  __builtin_ia32_tstorehdt1(base, stride, tsrc)
#define _tile_storentd(base, stride, tsrc)                                     \
  __builtin_ia32_tstorentd(base, stride, tsrc)
#define _tile_storeqd(base, stride, tsrc)                                      \
  __builtin_ia32_tstoreqd(base, stride, tsrc)
#define _tile_storeqdt1(base, stride, tsrc)                                    \
  __builtin_ia32_tstoreqdt1(base, stride, tsrc)
#define _tile_storerowd(mem_dst, tsrc) __builtin_ia32_tstorerowd(mem_dst, tsrc)

#endif /* __x86_64__ */
#endif /* __AMX_MEMORY2INTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_MEMORY2 */
