//==-------------  Tests for OpenMP support -*- OpenCL -*-------------------==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "type_defs.h"
#include "check_results_c.h"

__constant unsigned N = 1;
__constant unsigned TC = 200;

#pragma omp declare simd linear(i) uniform(x)
__attribute__((noinline)) int foo(int i, int x) {
  return (i + x);
}

// Initialize y, y_original and y_expected
void init(TYPE *y_original, TYPE *y, TYPE *y_expected) {

  for (unsigned i = 0; i < TC; i++) {
    y_original[i] = i;
    y[i] = y_original[i];
    y_expected[i] = y_original[i] + 3;
  }
}

// Main kernel for the test
__kernel void simd_func_linear(__global int* error) {
  *error = -1;

  // unsigned thread_id = get_global_id(0);
  // Note: there is only one OpenCL thread created by the host program.

  TYPE c_original[TC], c[TC], c_expected[TC];
  init((TYPE*) c_original, (TYPE*) c, (TYPE*) c_expected);

  unsigned i;
  unsigned x = 3;

#pragma omp simd
  for (i = 0; i < TC; i++) {
    c[i] = foo(i, x);
  }

  if(!check_result_1d((TYPE*) c_original, (TYPE*) c, (TYPE*) c_expected, TC)) {
    printf ("\nFAILED\n");
    return;
  }

  printf ("PASSED\n");
  *error = 0;
}
