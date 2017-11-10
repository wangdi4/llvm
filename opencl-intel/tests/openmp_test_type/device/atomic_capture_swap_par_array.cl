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
__constant unsigned NT       = 21;

extern int omp_get_thread_num(void);

// Initialize y, y_original and y_expected
void init(TYPE *ya_original, TYPE *ya, TYPE *ya_expected,
          TYPE *za_original, TYPE *za, TYPE *za_expected) {

    for (unsigned i = 0; i < NT; i++) {
        ya_original[i] = i;
        za_original[i] = 256;
        za[i] = za_original[i];
        ya[i] = ya_original[i];
        ya_expected[i] = za[i];
        za_expected[i] = 512;
    }
}

// Main kernel for the test
__kernel void atomic_capture_swap_par_array(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.

    TYPE ya_original[NT], ya[NT], ya_expected[NT];
    TYPE za_original[NT], za[NT], za_expected[NT];

    for (unsigned i = 0; i < NUM_REPS; i++) {

        init((TYPE*) &ya_original, (TYPE*) &ya, (TYPE*) &ya_expected,
             (TYPE*) &za_original, (TYPE*) &za, (TYPE*) &za_expected);

#pragma omp parallel shared(ya, za) num_threads(NT)
        {
            unsigned index = omp_get_thread_num();

            #pragma omp atomic capture
            {ya[index] = za[index]; za[index] = 512;};
        }

        if(!check_result_1d(ya_original, ya, ya_expected, NT)) {
            printf ("Rep[%u]: Captured values are incorrect.\nFAILED\n", i);
            return;
        }

        if(!check_result_1d((TYPE*) za_original, (TYPE*) za, (TYPE*) za_expected, NT)) {
            printf ("Rep[%u]: Atomic Operand's values are incorrect.\nFAILED\n", i);
            return;
        }
    }

    printf ("PASSED\n");
    *error = 0;
}
