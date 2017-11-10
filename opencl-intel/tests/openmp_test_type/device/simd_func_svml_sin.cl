//==-------------  Tests for OpenMP support -*- OpenCL -*-------------------==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#define DTYPE_FLOAT 1

#include "type_defs.h"
#include "check_results_c.h"

__constant unsigned TC = 180;

#pragma omp declare simd
__attribute__((noinline)) float foo(float x) {
  return (native_sin(x));
}

// Initialize y, y_original and y_expected
void init(TYPE *y_original, TYPE *y, TYPE *y_expected, unsigned n) {
  for (unsigned i = 0; i < n; i++) {
    y_original[i] = i;
    y[i] = y_original[i];
    y_expected[i] = native_sin(y_original[i]);
  }
}

// Main kernel for the test
__kernel void simd_func_svml_sin(__global int* error) {
  *error = -1;

  // unsigned thread_id = get_global_id(0);
  // Note: there is only one OpenCL thread created by the host program.

  TYPE y_original[TC], y[TC], y_expected[TC];
  init((TYPE*) y_original, (TYPE*) y, (TYPE*) y_expected, TC);

  TYPE c[TC];
  unsigned i;

#pragma omp simd
  for (i = 0; i < TC; i++) {
    y[i] = foo(i);
  }

  if(!check_result_1d((TYPE*) y_original, (TYPE*) y, (TYPE*) y_expected, TC)) {
    printf ("Rep[%u]:\nFAILED\n", 0);
    return;
  }

  printf ("PASSED\n");
  *error = 0;
}
