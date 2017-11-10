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

// Initialize y, y_original and y_expected
void init(TYPE *y_original, TYPE *y, TYPE *y_expected) {
    for (unsigned i = 0; i < TC; i++) {
      y_original[i] = 0;
      y[i] = y_original[i];
      y_expected[i] = i + i;
    }
}

#pragma omp declare simd
__attribute__((noinline)) int vec_sum(int i, int j) {
  return i + j;
}

// Main kernel for the test
__kernel void simd_func_vector(__global int* error) {
  *error = -1;

  // unsigned thread_id = get_global_id(0);
  // Note: there is only one OpenCL thread created by the host program.

  TYPE a[TC], b[TC];
  unsigned i;

  TYPE c_original[TC], c[TC], c_expected[TC];
  init(&c_original, &c, &c_expected);

  for (i = 0; i < TC; i++) {
    a[i] = i;
    b[i] = i;
  }

#pragma omp simd
  for (i = 0; i < TC; i++) {
    c[i] = vec_sum(a[i], b[i]);
  }

  if(!check_result_1d((TYPE*) c_original, (TYPE*) c, (TYPE*) c_expected, TC)) {
    printf ("\nFAILED\n");
    return;
  }

  printf ("PASSED\n");
  *error = 0;
}
