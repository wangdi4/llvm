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
__constant unsigned NT       = 8;
__constant unsigned N        = 10;



// This is the main OpenMP test kernel.
void for_red_array(TYPE *y) {
  TYPE x[4];
  #pragma omp parallel for 
  for (unsigned i = 0; i < 4; ++i) {
    x[i] = y[i];
  }

  #pragma omp parallel num_threads(NT)
  {
    #pragma omp for reduction(+:x)
    for (unsigned j = 1; j <= N; ++j)
    {
      for (unsigned i = 0; i < 4; ++i)
      {
        x[i] += i+j;
      }
    }
  }

  #pragma omp parallel for 
  for (unsigned i = 0; i < 4; ++i) {
    y[i] = x[i];
  }
}


// Main kernel for the test
__kernel void for_reduction_array(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.

    TYPE y[4];
    TYPE sum;

    for (unsigned i = 0; i < NUM_REPS; i++) {

        sum=y[0]=y[1]=y[2]=y[3]=0;

        for_red_array(y);

        sum=y[0]+y[1]+y[2]+y[3];

        // printf ("y: %d %d %d %d  sum=%d", y[0],y[1],y[2],y[3], sum);
        // expect:  y: 55 65 75 85  sum=280

        if(!check_result(0, sum, 280)) {
            printf ("Rep[%u]:\nFAILED\n", i);
            return;
        }
    }

    printf ("PASSED\n");
    *error = 0;
}
