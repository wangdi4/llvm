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
__constant unsigned NT       = 10;

extern int omp_get_thread_num(void);

// Initialize y, y_original and y_expected
void init(TYPE *y_original, TYPE *y, TYPE *y_expected, TYPE *x) {
    *y_original = 10;
    *x = *y_original;
    *y = *y_original;
    *y_expected = *y_original;

    TYPE x_temp = 10;
    for (unsigned i = 0; i < NT; i++) {
        *y_expected *= x_temp;
    }
}

// Main kernel for the test
__kernel void atomic_update_par_scalar_mul(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.

    TYPE y_original, y, y_expected, x;

    for (unsigned i = 0; i < NUM_REPS; i++) {

        init(&y_original, &y, &y_expected, &x);

        #pragma omp parallel shared(y) firstprivate (x) num_threads(NT)
        {
            #pragma omp atomic
            y *= x++;
        }

        if(!check_result(y_original, y, y_expected)) {
            printf ("Rep[%u]:\nFAILED\n", i);
            return;
        }
    }

    printf ("PASSED\n");
    *error = 0;
}
