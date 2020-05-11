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
#include "fp_trunc_impl.inc"

__attribute__((weak))
uint16_t __trunctfhf2(long double a) { return __truncXfYf2__(a); }
