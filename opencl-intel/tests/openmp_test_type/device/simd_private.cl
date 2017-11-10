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
void dummy_noop2(void);

// Test for omp simd
void simd_loop(int *arr) {
  int index;

#pragma omp simd
  for (index = 0; index < SIZE; index++) {
    arr[index] = index;
  }
}

// Test for omp simd private
void simd_loopprivate(int *arr) {
  int index;
  int lpi;

#pragma omp simd private(lpi)
  for (index = 0; index < SIZE; index++) {
    lpi = arr[index];
    dummy_noop2(arr);
    arr[index] = lpi + 2;
  }
}

// Main kernel for the test
__kernel void simd_private(__global int* error) {
  *error = -1;

  // unsigned thread_id = get_global_id(0);
  // Note: there is only one OpenCL thread created by the host program.

  int arr[SIZE];
  int index;

  simd_loop(arr);
  dummy_noop(arr);
  simd_loopprivate(arr);
  dummy_noop(arr);

  for (index = 0; index < SIZE; index++) {
    if (arr[index] != index + 2) {
      printf("arr[%d] != %d \nFAILED\n", index, index + 2);
      return;
    }
  }
  printf("PASSED\n");
  *error = 0;
}

void dummy_noop(void *vp) {}
void dummy_noop2(void) {}
