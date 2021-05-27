/*===------------------- dspintrin.h - DSP intrinsics ----------------------===
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
#error "Never use <dspintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __DSPINTRIN_H
#define __DSPINTRIN_H

/* Define the default attributes for the functions in this file. */
#define _mm_dsp_pcr2bfrsw_epi16(A, B, C, D)                                    \
  (__m128i)                                                                    \
      __builtin_ia32_dvpcr2bfrsw128((__v8hi)A, (__v8hi)B, (__v8hi)C, (int)D)

#define _mm_dsp_plutsincosw_epi16(A, B)                                        \
  (__m128i) __builtin_ia32_dvplutsincosw128((__v8hi)A, (int)B)

#endif // __DSPINTRIN_H
