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

extern int omp_get_thread_num(void);

// This is the main OpenMP test kernel.
void par_atomic(int *y) {
  #pragma omp parallel num_threads(N)
  {
    #pragma omp atomic 
     *y = *y + omp_get_thread_num();
  }
}

// Main kernel for the test
__kernel void par_region_atomic(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.

    int x = 0, x_expected = 120;

    par_atomic(&x);

    if(x != x_expected) {
      printf ("x = %d \nFAILED\n", x);
      return;
    }
    printf ("PASSED\n");
    *error = 0;
}
