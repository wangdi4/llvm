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

#define SIZE 1024

void dummy_noop(void *);

// Test for omp simd
void simd_loop(int *arr) {
  int index;

#pragma omp simd
  for (index = 0; index < SIZE; index++) {
    arr[index] = index;
  }
}

// Main kernel for the test
__kernel void simd_test(__global int* error) {
  *error = -1;

  // unsigned thread_id = get_global_id(0);
  // Note: there is only one OpenCL thread created by the host program.

  int arr[SIZE], index;

  simd_loop(arr);
  dummy_noop(arr);
  for (index = 0; index < SIZE; index++) {
    if (arr[index] != index) {
      printf("arr[%d] != %d \nFAILED\n", index, index);
      return;
    }
  }

  printf("PASSED\n");
  *error = 0;
}

void dummy_noop(void *vp) {}
