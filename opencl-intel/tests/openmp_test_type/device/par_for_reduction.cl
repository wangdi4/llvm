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
__constant unsigned N        = 10;



// This is the main OpenMP test kernel.
void par_for_red(TYPE *y) {
  TYPE x=*y;
  #pragma omp parallel for reduction(+:x) num_threads(NT)
  for (unsigned j = 1; j <= N; ++j)
  {
     x += j;
  }
  *y = x;
}


// Main kernel for the test
__kernel void par_for_reduction(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.

    TYPE y;

    for (unsigned i = 0; i < NUM_REPS; i++) {

        y = 10;
        par_for_red(&y);

        if(!check_result(10, y, 65)) {
            printf ("Rep[%u]:\nFAILED\n", i);
            return;
        }
    }

    printf ("PASSED\n");
    *error = 0;
}
