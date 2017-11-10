//==-------------  Tests for OpenMP support -*- OpenCL -*-------------------==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "check_results_c.h"
#include "type_defs.h"

#define SIZE 20

void dummy_noop(void *);

// Test for simd simdlen, max reduction
int max_loop(int *arr) {
  int index, maxval = arr[0];

#pragma omp simd simdlen(4) reduction(max : maxval)
  for (index = 0; index < SIZE; index++) {
    if (arr[index] > maxval)
      maxval = arr[index];
  }

  return maxval;
}

// Main kernel for the test
__kernel void simd_max_reduction(__global int* error) {
  *error = -1;

  // unsigned thread_id = get_global_id(0);
  // Note: there is only one OpenCL thread created by the host program.

  int arr[SIZE] = {1199, 101, 23, 108,  65, 2,  100, 199, 100, 200,
                   2300, 345, 76, 8400, 24, 34, 22,  78,  18,  1100};
  int maxval;
  int expected = 8400;

  dummy_noop(arr);
  maxval = max_loop(arr);
  if (maxval != expected) {
    printf("maxval: %d != %d \nFAILED\n", maxval, expected);
    return;
  }

  printf("PASSED\n");
  *error = 0;
}

void dummy_noop(void *vp) {}
