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

#define SIZE 19

void dummy_noop(void *);
void dummy_noop2(void);

// Test for simd simdlen, min reduction
int min_loop(int *arr) {
  int index, minval = arr[0];

#pragma omp simd simdlen(4) reduction(min : minval)
  for (index = 0; index < SIZE; index++) {
    dummy_noop2();
    if (arr[index] < minval)
      minval = arr[index];
  }

  return minval;
}

// Main kernel for the test
__kernel void simd_min_reduction(__global int* error) {
  *error = -1;

  // unsigned thread_id = get_global_id(0);
  // Note: there is only one OpenCL thread created by the host program.

  int arr[SIZE] = {1199, 101, 23, 108,  65, 2,  100, 199, 100, 200,
                   2300, 345, 76, 8400, 24, 34, 22,  78,  18};
  int minval;
  int expected = 2;

  dummy_noop(arr);
  minval = min_loop(arr);
  if (minval != expected) {
    printf("minval: %d != %d \nFAILED\n", minval, expected);
    return;
  }

  printf("PASSED\n");
  *error = 0;
}

void dummy_noop(void *vp) {}
void dummy_noop2(void) {}
