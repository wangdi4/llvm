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

#define N 10

// This is the main OpenMP test kernel.
void work(TYPE* j_last) {
#pragma omp for
    for (unsigned j = 0; j < N ; j+=2) {
        // No need for critical/lastprivate here, since there is no parallel.
#if defined(DEBUG) && DEBUG > 4
        printf("j = %d\n", j);
#endif
        *j_last = j;
    }
}

// Main kernel for the test
__kernel void loop_trip_test_lt_stride2(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.
    TYPE liter_original = 0;
    TYPE liter = liter_original;
    TYPE liter_expected = 8;

    work(&liter);

    if(!check_result(liter_original, liter, liter_expected)) {
        printf("FAILED\n");
        return;
    }

    printf("PASSED\n");
    *error = 0;
}
