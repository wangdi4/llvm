// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
#if INTEL_COLLAB
//===--- abi-atomic-fp.cpp - Atomic FP operation support ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains entry points for supporting FP atomic operations.
//
//===----------------------------------------------------------------------===//

#if __SPIR__

#include <stdint.h>
#define EXTERN extern "C"

/// Native float atomic extension
extern float __spirv_AtomicFAddEXT(
    float *ptr, uint32_t scope, uint32_t order, float val);
extern float __spirv_AtomicFMinEXT(
    float *ptr, uint32_t scope, uint32_t order, float val);
extern float __spirv_AtomicFMaxEXT(
    float *ptr, uint32_t scope, uint32_t order, float val);
#ifdef ENABLE_FP64_ATOMIC_EXT
#if INTEL_CUSTOMIZATION
// This is disabled for now due to issues (hang) on ATS.
// Existing emulation in device RTL works fine.
#endif // INTEL_CUSTOMIZATION
extern double __spirv_AtomicFAddEXT(
    double *ptr, uint32_t scope, uint32_t order, double val);
extern double __spirv_AtomicFMinEXT(
    double *ptr, uint32_t scope, uint32_t order, double val);
extern double __spirv_AtomicFMaxEXT(
    double *ptr, uint32_t scope, uint32_t order, double val);
#endif // ENABLE_FP64_ATOMIC_EXT

/// From sycl's header
#define scope_device 1
#define order_acq_rel 0x8

#define OP_MIN(X, Y, DT) ((X) < (Y) ? (X) : (Y))
#define OP_MAX(X, Y, DT) ((X) > (Y) ? (X) : (Y))
#define OP_ADD(X, Y, DT) ((X) + (Y))
#define OP_SUB(X, Y, DT) ((X) - (Y))

/// Signature for plain atomics
#define KMPC_ATOMIC_FN(DATANAME, OPTYPE, DATATYPE)                             \
EXTERN void __kmpc_atomic_##DATANAME##_##OPTYPE                                \
  (DATATYPE *lhs, DATATYPE rhs)

/// Signature for capture atomics
#define KMPC_ATOMIC_FN_CPT(DATANAME, OPTYPE, DATATYPE)                         \
EXTERN DATATYPE __kmpc_atomic_##DATANAME##_##OPTYPE##_cpt                      \
  (DATATYPE *lhs, DATATYPE rhs, int flag)

/// Use float atomic extension function for supported operations
#define KMPC_ATOMIC_IMPL_FLOAT_EXT_BASE(                                       \
    DATANAME, DATATYPE, OPNAME, OPEXT, SIGN)                                   \
  KMPC_ATOMIC_FN(DATANAME, OPNAME, DATATYPE) {                                 \
    __spirv_AtomicF##OPEXT##EXT(lhs, scope_device, order_acq_rel, SIGN rhs);   \
  }
#define KMPC_ATOMIC_IMPL_FLOAT_EXT(X, Y, Z, S)                                 \
  KMPC_ATOMIC_IMPL_FLOAT_EXT_BASE(X, Y, Z, S,)

/// Use float atomic extension function for supported captured operations
#define KMPC_ATOMIC_IMPL_FLOAT_EXT_CPT_BASE(                                   \
    DATANAME, DATATYPE, OPNAME, OPEXT, OP, SIGN)                               \
  KMPC_ATOMIC_FN_CPT(DATANAME, OPNAME, DATATYPE) {                             \
    DATATYPE captured =                                                        \
        __spirv_AtomicF##OPEXT##EXT(                                           \
            lhs, scope_device, order_acq_rel, SIGN rhs);                       \
    if (flag)                                                                  \
      captured = OP(captured, rhs, DATATYPE);                                  \
    return captured;                                                           \
  }
#define KMPC_ATOMIC_IMPL_FLOAT_EXT_CPT(X, Y, Z, S, T)                          \
  KMPC_ATOMIC_IMPL_FLOAT_EXT_CPT_BASE(X, Y, Z, S, T,)

#pragma omp declare target
/// 4-byte float atomics
#ifdef ENABLE_FP32_ATOMIC_ADD_EXT
#if INTEL_CUSTOMIZATION
// Disabling Add extension since it is unstable on ATS. MinMax is OK.
#endif // INTEL_CUSTOMIZATION
KMPC_ATOMIC_IMPL_FLOAT_EXT(float4, float, add, Add)
KMPC_ATOMIC_IMPL_FLOAT_EXT_BASE(float4, float, sub, Add, -)
KMPC_ATOMIC_IMPL_FLOAT_EXT_CPT(float4, float, add, Add, OP_ADD)
KMPC_ATOMIC_IMPL_FLOAT_EXT_CPT_BASE(float4, float, sub, Add, OP_SUB, -)
#endif
KMPC_ATOMIC_IMPL_FLOAT_EXT(float4, float, min, Min)
KMPC_ATOMIC_IMPL_FLOAT_EXT(float4, float, max, Max)
KMPC_ATOMIC_IMPL_FLOAT_EXT_CPT(float4, float, min, Min, OP_MIN)
KMPC_ATOMIC_IMPL_FLOAT_EXT_CPT(float4, float, max, Max, OP_MAX)

#ifdef ENABLE_FP64_ATOMIC_EXT
/// 8-byte float atomics
KMPC_ATOMIC_IMPL_FLOAT_EXT(float8, double, add, Add)
KMPC_ATOMIC_IMPL_FLOAT_EXT_BASE(float8, double, sub, Add, -)
KMPC_ATOMIC_IMPL_FLOAT_EXT(float8, double, min, Min)
KMPC_ATOMIC_IMPL_FLOAT_EXT(float8, double, max, Max)
KMPC_ATOMIC_IMPL_FLOAT_EXT_CPT(float8, double, add, Add, OP_ADD)
KMPC_ATOMIC_IMPL_FLOAT_EXT_CPT_BASE(float8, double, sub, Add, OP_SUB, -)
KMPC_ATOMIC_IMPL_FLOAT_EXT_CPT(float8, double, min, Min, OP_MIN)
KMPC_ATOMIC_IMPL_FLOAT_EXT_CPT(float8, double, max, Max, OP_MAX)
#endif // ENABLE_FP64_ATOMIC_EXT
#pragma omp end declare target

#endif // __SPIR__

#endif // INTEL_COLLAB
