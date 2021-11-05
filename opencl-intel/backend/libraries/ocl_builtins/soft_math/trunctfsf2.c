// This file is modified from trunctfsf2.c in LLVM compiler-rt project.
//
//===-- lib/trunctfsf2.c - quad -> single conversion --------------*- C -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define QUAD_PRECISION
#include "fp_lib.h"

#define SRC_QUAD
#define DST_SINGLE
#include "fp_trunc_impl.inc"

__attribute__((weak))
float __trunctfsf2(long double a) { return __truncXfYf2__(a); }
