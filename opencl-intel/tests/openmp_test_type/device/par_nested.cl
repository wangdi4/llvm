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
void par_do_nested(int *x, int *y) {

  omp_set_nested(1);

  #pragma omp parallel num_threads(N)
  {
     #pragma omp atomic  
     *x = *x + 1;

     #pragma omp parallel num_threads(N/2)
     {
        #pragma omp atomic 
        *y = *y + 1;
     } 
  }

}

// Main kernel for the test
__kernel void par_nested(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.
    int x = 0;
    int y = 0;

    par_do_nested(&x, &y);

    if(x != 16 || y != 128) {
      printf ("x = %d y = %d \nFAILED\n", x, y);
      return;
    }
    printf ("PASSED\n");
    *error = 0;
}
