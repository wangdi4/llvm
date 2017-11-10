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

__constant unsigned NUM_REPS = 10;
__constant unsigned NT       = 4;
__constant unsigned N        = 100;



// This is the main OpenMP test kernel.
void par_for_shr(TYPE *y) {
  TYPE x=*y;
  #pragma omp parallel for shared(x) num_threads(NT)
  for (unsigned j = 0; j < N; ++j)
  {
     if (j==7) {
       x = x + 100;
     }
  }
  *y = x;
}


// Main kernel for the test
__kernel void par_for_shared(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.

    TYPE y;

    for (unsigned i = 0; i < NUM_REPS; i++) {

        y = 123;
        par_for_shr(&y);

        if(!check_result(123, y, 223)) {
            printf ("Rep[%u]:\nFAILED\n", i);
            return;
        }
    }

    printf ("PASSED\n");
    *error = 0;
}
