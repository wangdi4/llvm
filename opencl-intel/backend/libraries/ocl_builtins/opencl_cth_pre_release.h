//==--- opencl_cth_pre_release.h - Pre-release extensions -*- C++ -*----------==//
////
//// Copyright (C) 2019 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive property
//// of Intel Corporation and may not be disclosed, examined or reproduced in
//// whole or in part without explicit written authorization from the company.
////
//// ===--------------------------------------------------------------------=== //

//
// Float atomics
//
#if defined (float_atomics_enable)

// atom_min
float __attribute__((overloadable)) atomic_min(volatile __global float *p, float val);
float __attribute__((overloadable)) atomic_min(volatile __local float *p, float val);

// atom_max
float __attribute__((overloadable)) atomic_max(volatile __global float *p, float val);
float __attribute__((overloadable)) atomic_max(volatile __local float *p, float val);

// atom_cmpxchg
float __attribute__((overloadable)) atomic_cmpxchg(volatile __global float *p, float cmp, float val);
float __attribute__((overloadable)) atomic_cmpxchg(volatile __local float *p, float cmp, float val);

#endif
