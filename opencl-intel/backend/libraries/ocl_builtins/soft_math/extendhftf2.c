// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// This file is modified from extendsftf2.c in LLVM compiler-rt project.
//
//===-- lib/extendhftf2.c - half -> quad conversion ---------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define QUAD_PRECISION
#include "fp_lib.h"

#define SRC_HALF
#define DST_QUAD
#ifdef __INTEL_LLVM_COMPILER
#define COMPILER_RT_HAS_FLOAT16
#endif
#include "fp_extend_impl.inc"

COMPILER_RT_ABI long double __extendhftf2(src_t a) {
  return __extendXfYf2__(a);
}
