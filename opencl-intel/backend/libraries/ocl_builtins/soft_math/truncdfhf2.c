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
// This file is modified from truncdfhf2.c in LLVM compiler-rt project.
//
//===-- lib/truncdfhf2.c - double -> half conversion --------------*- C -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define SRC_DOUBLE
#define DST_HALF
#ifdef __INTEL_LLVM_COMPILER
#define COMPILER_RT_HAS_FLOAT16
#endif
#include "fp_trunc_impl.inc"

#ifndef _WIN32
__attribute__((weak))
#endif
dst_t __truncdfhf2(double a) {
  return __truncXfYf2__(a);
}
