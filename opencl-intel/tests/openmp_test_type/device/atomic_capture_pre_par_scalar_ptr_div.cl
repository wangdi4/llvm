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
__constant unsigned NT       = 8;

extern int omp_get_thread_num(void);

// Initialize y, y_original and y_expected
void init(TYPE *y_original, TYPE *y, TYPE *y_expected,
          TYPE *ya_original, TYPE *ya, TYPE *ya_expected) {

    for (unsigned i = 0; i < NT; i++) {
        ya_original[i] = 0;
        ya[i] = ya_original[i];
        ya_expected[i] = 1 << (NT - i + 1);
    }

    *y_original = 1 << (NT + 1);
    *y = *y_original;
    *y_expected = 2;
}

// This is the main OpenMP test kernel.
void work(TYPE *y, TYPE *ya) {
    #pragma omp parallel shared(y) num_threads(NT)
    {
        TYPE v;
        #pragma omp atomic capture
        {v = *y; *y = *y / 2;};

        ya[omp_get_thread_num()] = v;
    }
}

// Main kernel for the test
__kernel void atomic_capture_pre_par_scalar_ptr_div(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.

    TYPE y_original, y, y_expected;
    TYPE ya_original[NT], ya[NT], ya_expected[NT];

    for (unsigned i = 0; i < NUM_REPS; i++) {

        init(&y_original, &y, &y_expected,
             (TYPE*) &ya_original, (TYPE*) &ya, (TYPE*) &ya_expected);
        work(&y, ya);

        if(!check_result(y_original, y, y_expected)) {
            printf ("Rep[%u]: Final value is incorrect.\nFAILED\n", i);
            return;
        }

        if(!compare_checksums_1d((TYPE*) ya_original, (TYPE*) ya, (TYPE*) ya_expected, NT)) {
            printf ("Rep[%u]: Captured values are incorrect.\nFAILED\n", i);
            return;
        }
    }

    printf ("PASSED\n");
    *error = 0;
}
