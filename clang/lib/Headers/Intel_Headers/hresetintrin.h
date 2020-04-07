/* INTEL_FEATURE_ISA_HRESET */
// ==------------- hresetintrin.h - serialzie intrinsic -*- C -*------------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __IMMINTRIN_H
#error "Never use <hresetintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __INTEL_HRESETINTRIN_H
#define __INTEL_HRESETINTRIN_H

#define _hreset(IMM, EAX) \
  __asm__ __volatile__ ("hreset %0" :: "i"(IMM), "a"(EAX))

#endif /* __INTEL_HRESETINTRIN_H */
/* end INTEL_FEATURE_ISA_HRESET */
