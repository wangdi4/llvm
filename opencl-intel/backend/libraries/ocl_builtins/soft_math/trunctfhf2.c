// INTEL CONFIDENTIAL
//
// Copyright 2022 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// This file is modified from trunctfhf2.c in LLVM compiler-rt project.
//
//===-- lib/trunctfhf2.c - quad -> half conversion ----------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define QUAD_PRECISION
#include "fp_lib.h"

#define SRC_QUAD
#define DST_HALF
#ifdef __INTEL_LLVM_COMPILER
#define COMPILER_RT_HAS_FLOAT16
#endif
#include "fp_trunc_impl.inc"

__attribute__((weak)) dst_t __trunctfhf2(long double a) {
  return __truncXfYf2__(a);
}
