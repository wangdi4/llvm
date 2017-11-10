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
void par_do_master(int8 *y) {
  int8 x = *y;
  #pragma omp parallel firstprivate(x) num_threads(N)
  {
     x = x + omp_get_thread_num();

     #pragma omp master 
     {   
       *y = x + 100;
     }
  }

}

// Main kernel for the test
__kernel void par_master(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.

    int8 x = 888, x_expected = 988;

    par_do_master(&x);

    if(x.s6 != x_expected.s6) {
      printf ("x = %d \nFAILED\n", x.s6);
      return;
    }
    printf ("PASSED\n");
    *error = 0;
}
