/*===----------- amxsparseintrin.h - AMXSPARSE intrinsics -----------------===
 *
 * Copyright (C) 2021 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <amxsparseintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AMX_SPARSEINTRIN_H
#define __AMX_SPARSEINTRIN_H
#ifdef __x86_64__

#define _tile_dpsbssd(tdst, tsrc1, tsrc2)                                      \
  __builtin_ia32_tdpsbssd(tdst, tsrc1, tsrc2)
#define _tile_dpsbsud(tdst, tsrc1, tsrc2)                                      \
  __builtin_ia32_tdpsbsud(tdst, tsrc1, tsrc2)
#define _tile_dpsbusd(tdst, tsrc1, tsrc2)                                      \
  __builtin_ia32_tdpsbusd(tdst, tsrc1, tsrc2)
#define _tile_dpsbuud(tdst, tsrc1, tsrc2)                                      \
  __builtin_ia32_tdpsbuud(tdst, tsrc1, tsrc2)
#define _tile_dpsbf16ps(tdst, tsrc1, tsrc2)                                    \
  __builtin_ia32_tdpsbf16ps(tdst, tsrc1, tsrc2)
#define _tile_dpsfp16ps(tdst, tsrc1, tsrc2)                                    \
  __builtin_ia32_tdpsfp16ps(tdst, tsrc1, tsrc2)
#define _tile_ldexpandb(tdst, base, stride, A)                                 \
  __builtin_ia32_tldexpandb(tdst, base, stride, A)
#define _tile_ldexpandbt1(tdst, base, stride, A)                               \
  __builtin_ia32_tldexpandbt1(tdst, base, stride, A)
#define _tile_ldexpandw(tdst, base, stride, A)                                 \
  __builtin_ia32_tldexpandw(tdst, base, stride, A)
#define _tile_ldexpandwt1(tdst, base, stride, A)                               \
  __builtin_ia32_tldexpandwt1(tdst, base, stride, A)

#endif // __x86_64__
#endif // __AMX_SPARSEINTRIN_H
