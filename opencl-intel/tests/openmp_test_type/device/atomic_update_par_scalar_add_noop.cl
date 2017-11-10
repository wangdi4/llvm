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
__constant unsigned NUM_REPS = 5;
__constant unsigned NT       = 16;

// Initialize y, y_original and y_expected
void init(TYPE *y_original, TYPE *y, TYPE *y_expected) {
    *y_original = 10;
    *y = *y_original;
    *y_expected = *y_original;
}

// Main kernel for the test
__kernel void atomic_update_par_scalar_add_noop(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.

    TYPE y_original, y, y_expected;

    for (unsigned i = 0; i < NUM_REPS; i++) {

        init(&y_original, &y, &y_expected);

        #pragma omp parallel shared(y) num_threads(NT)
        {
            #pragma omp atomic
            y += 0;
        }

        if(!check_result(y_original, y, y_expected)) {
            printf ("Rep[%u]:\nFAILED\n", i);
            return;
        }
    }

    printf ("PASSED\n");
    *error = 0;
}
