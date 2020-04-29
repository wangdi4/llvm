// This file is modified from truncsfhf2.c in LLVM compiler-rt project.
//
//===-- lib/truncsfhf2.c - single -> half conversion --------------*- C -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define SRC_SINGLE
#define DST_HALF
#include "fp_trunc_impl.inc"

uint16_t __truncsfhf2(float a) {
  return __truncXfYf2__(a);
}

#ifndef _WIN32
__attribute__((weak))
#endif
uint16_t __gnu_f2h_ieee(float a) { return __truncsfhf2(a); }
