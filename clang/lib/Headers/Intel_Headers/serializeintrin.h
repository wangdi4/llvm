/* INTEL_FEATURE_ISA_SERIALIZE */
//==------------- serializeintrin.h - serialzie intrinsic -*- C -*------------==//
////
//// Copyright (C) 2019 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive property
//// of Intel Corporation and may not be disclosed, examined or reproduced in
//// whole or in part without explicit written authorization from the company.
////
//// ===--------------------------------------------------------------------=== //

#ifndef __IMMINTRIN_H
#error "Never use <serializeintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __INTEL_SERIALIZEINTRIN_H
#define __INTEL_SERIALIZEINTRIN_H

static __inline__ void
__attribute__((__always_inline__, __nodebug__, __target__("serialize")))
_serialize (void)
{
  __builtin_ia32_serialize ();
}

#endif /* __INTEL_SERIALIZEINTRIN_H */
/* end INTEL_FEATURE_ISA_SERIALIZE */
