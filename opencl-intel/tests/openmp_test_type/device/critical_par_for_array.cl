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

__constant unsigned N        = 10;
__constant unsigned M        = 10000;
__constant unsigned NUM_REPS = 10;
__constant unsigned NT       = 31;

// Initialize y, y_original and y_expected
void init(TYPE *y_original, TYPE *y, TYPE *y_expected, const unsigned n) {
    for (unsigned i = 0; i < n; i++) {
          y_original[i] = i;
          y[i] = y_original[i];
          y_expected[i] = y_original[i] + M;
    }
}

// This is the main OpenMP test kernel.
void work(TYPE *y, const unsigned n) {
    #pragma omp parallel for shared(y) num_threads(NT)
    for (unsigned i = 0; i < M * N; i++) {
        #pragma omp critical
        y[i/M] += 1;
    }
}

// Main kernel for the test
__kernel void critical_par_for_array(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.

    TYPE y_original[N], y[N], y_expected[N];

    for (unsigned i = 0; i < NUM_REPS; i++) {

        init(y_original, y, y_expected, N);
        work(y, N);

        if(!check_result_1d(y_original, y, y_expected, N)) {
            printf ("Rep[%u]:\nFAILED\n", i);
            return;
        }
    }

    printf ("PASSED\n");
    *error = 0;
}
