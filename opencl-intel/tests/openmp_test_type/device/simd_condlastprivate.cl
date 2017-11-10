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
    if (index % 3)
      arr[index] = index;
    else
      arr[index] = 0;
  }
}

// Test for omp simd conditional last private
int simd_loopcondlastprivate(int *arr) {
  int index;
  int lpi = 0;

#pragma omp simd lastprivate(conditional : lpi)
  for (index = 0; index < SIZE; index++) {
    if (arr[index])
      lpi = arr[index];
    dummy_noop2(arr);
  }

  return lpi;
}

// Main kernel for the test
__kernel void simd_condlastprivate(__global int* error) {
  *error = -1;

  // unsigned thread_id = get_global_id(0);
  // Note: there is only one OpenCL thread created by the host program.

  int arr[SIZE];
  int index, lpi;
  int expected = 1022;

  simd_loop(arr);
  dummy_noop(arr);
  lpi = simd_loopcondlastprivate(arr);
  dummy_noop(arr);

  if (lpi != expected) {
    printf("lpi: %d != %d \nFAILED\n", lpi, expected);
    return;
  }

  for (index = 0; index < SIZE; index++) {
    if (index % 3) {
      if (arr[index] != index) {
        printf("arr[%d] != %d \nFAILED\n", index, index);
        return;
      }
    } else {
      if (arr[index] != 0) {
        printf("arr[%d] != 0 \nFAILED\n", index);
        return;
      }
    }
  }
  printf("PASSED\n");
  *error = 0;
}

void dummy_noop(void *vp) {}
void dummy_noop2(void) {}
