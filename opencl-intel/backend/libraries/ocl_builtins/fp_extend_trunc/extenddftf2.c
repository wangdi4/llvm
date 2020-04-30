// This file is modified from extenddftf2.c in LLVM compiler-rt project.
//
//===-- lib/extenddftf2.c - double -> quad conversion -------------*- C -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define QUAD_PRECISION
#include "fp_lib.h"

#define SRC_DOUBLE
#define DST_QUAD
#include "fp_extend_impl.inc"

__attribute__((weak))
long double __extenddftf2(double a) { return __extendXfYf2__(a); }
