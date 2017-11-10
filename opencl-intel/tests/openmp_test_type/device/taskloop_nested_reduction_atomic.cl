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

__constant unsigned N = 16;

extern void omp_set_nested(bool);

// This is the main OpenMP test kernel.
void task_loop_nested_reduction_atomic(int *x, int *y) {
  TYPE x1=*x;
  TYPE y1=*y;

  #pragma omp taskloop reduction(+:x1) shared(y1)
  for (unsigned i = 1; i <= N; ++i)
  {
     x1 = x1 + 1;

     #pragma omp taskloop shared(y1)
     for (unsigned j = 1; j <= N; ++j)
     {
        #pragma omp atomic
        y1 = y1 + 1;
     }
  }
  *x = x1;
  *y = y1;
}

// Main kernel for the test
__kernel void taskloop_nested_reduction_atomic(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.
    int x = 0;
    int y = 0;

    task_loop_nested_reduction_atomic(&x, &y);

    if(x != 16 || y != 256) {
      printf ("x = %d y = %d \nFAILED\n", x, y);
      return;
    }
    printf ("PASSED\n");
    *error = 0;
}
