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

#define SIZE 1023

void dummy_noop(void *);
void dummy_noop2(void);

// Test for simd
void simd_loop(int *arr) {
  int index;

#pragma omp simd
  for (index = 0; index < SIZE; index++) {
    arr[index] = index;
  }
}

// Test for simd simdlen + sum reduction
int sum_loop(int *arr) {
  int index, sum = 0;

#pragma omp simd simdlen(8) reduction(+ : sum)
  for (index = 0; index < SIZE; index++) {
    dummy_noop2();
    sum += arr[index];
  }

  return sum;
}

// Main kernel for the test
__kernel void simd_sum_reduction2(__global int* error) {
  *error = -1;

  // unsigned thread_id = get_global_id(0);
  // Note: there is only one OpenCL thread created by the host program.

  int arr[SIZE], sum;
  int expected = ((SIZE - 1) * SIZE) / 2;

  simd_loop(arr);
  dummy_noop(arr);
  sum = sum_loop(arr);
  if (sum != expected) {
    printf("sum: %d != %d \nFAILED\n", sum, expected);
    return;
  }

  printf("PASSED\n");
  *error = 0;
}

void dummy_noop(void *vp) {}
void dummy_noop2(void) {}
