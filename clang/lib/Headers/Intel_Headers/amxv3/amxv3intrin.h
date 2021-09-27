/*===----------------- amxv3intrin.h - AMXV3 intrinsics --------------------===
 *
 * Copyright (C) 2021 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===-----------------------------------------------------------------------===
 */
#ifndef __IMMINTRIN_H
#error "Never use <amxv3intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AMX_V3INTRIN_H
#define __AMX_V3INTRIN_H
#ifdef __x86_64__

#define _tile_loadtransposed(tdst, base, stride)                               \
  __builtin_ia32_tloadtransposed(tdst, base, stride)
#define _tile_loadtransposedt1(tdst, base, stride)                             \
  __builtin_ia32_tloadtransposedt1(tdst, base, stride)
#define _tile_rpntlvwz0(tdst, base, stride)                                    \
  __builtin_ia32_trpntlvwz0(tdst, base, stride)
#define _tile_rpntlvwz0t1(tdst, base, stride)                                  \
  __builtin_ia32_trpntlvwz0t1(tdst, base, stride)
#define _tile_rpntlvwz1(tdst, base, stride)                                    \
  __builtin_ia32_trpntlvwz1(tdst, base, stride)
#define _tile_rpntlvwz1t1(tdst, base, stride)                                  \
  __builtin_ia32_trpntlvwz1t1(tdst, base, stride)
#define _tile_storetransposed(base, stride, tsrc1)                             \
  __builtin_ia32_tstoretransposed(base, stride, tsrc1)

#endif // __x86_64__
#endif // __AMX_V3INTRIN_H
