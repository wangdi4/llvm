/* INTEL_FEATURE_ISA_AMX_MEMADVISE */
/*===-------- Intel_amxMEMADVISEintrin.h - AMX intrinsics -*- C++ -*---------===
*
* Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxmemadviseintrin.h> directly; include <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMXMEMADVISEINTRIN_H
#define __AMXMEMADVISEINTRIN_H
#ifdef __x86_64__

#define _tile_movadvise_load __builtin_ia32_tmovadvise_load
#define _tile_movadvise_store __builtin_ia32_tmovadvise_store

#endif /* __x86_64__ */
#endif /* __AMXMEMADVISEINTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_MEMADVISE */
