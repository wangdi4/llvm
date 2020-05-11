// This file is modified from extendhfsf2.c in LLVM compiler-rt project.
//
//===-- lib/extendhfsf2.c - half -> single conversion -------------*- C -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define SRC_HALF
#define DST_SINGLE
#include "fp_extend_impl.inc"

float __extendhfsf2(uint16_t a) {
  return __extendXfYf2__(a);
}

#ifndef _WIN32
__attribute__((weak))
#endif
float __gnu_h2f_ieee(uint16_t a) { return __extendhfsf2(a); }
