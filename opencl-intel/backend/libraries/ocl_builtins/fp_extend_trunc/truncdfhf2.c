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
#include "fp_trunc_impl.inc"

#ifndef _WIN32
__attribute__((weak))
#endif
uint16_t __truncdfhf2(double a) { return __truncXfYf2__(a); }
