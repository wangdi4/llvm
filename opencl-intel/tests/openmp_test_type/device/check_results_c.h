    //==-------------  Tests for OpenMP support -*- OpenCL -*-------------------==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef _CHECK_RESULTS_C_H_
#define _CHECK_RESULTS_C_H_

#include "type_defs.h"

TYPE abs_val(TYPE x) { if ( x < 0 ) { return x * -1; }; return x; }

// Print the array a
void print_array(TYPE* a, unsigned n)
{
    for (unsigned i = 0; i < n; i++) {
        printf ( FORMAT"\t", a[i]);
    }
    printf("\n");
}

// Check if scalars "actual" and "expected" are same
bool check_result(TYPE original, TYPE actual, TYPE expected) {

    TYPE difference = abs_val(actual - expected);
    if (EPSILON < difference) {
#if defined(DEBUG) && DEBUG > 0
        printf("Value mismatch. Original: " FORMAT ", Expected: " FORMAT ", "
               "Actual: " FORMAT ".\n", original, expected, actual);
#endif
        return false;
    }
    return true;
}

// Check if arrays "actual" and "expected" are same.
bool check_result_1d(TYPE *original, TYPE *actual, TYPE *expected, unsigned n) {
    for (unsigned i = 0; i < n; i++) {
        TYPE difference = abs_val(actual[i] - expected[i]);
        if (EPSILON < difference) {

#if defined(DEBUG) && DEBUG > 0
            printf("Value mismatch at i = %u. Expected: "FORMAT", "
                   "Actual: "FORMAT".\n", i, expected[i], actual[i]);
            printf("\nOriginal Array:\n");
            print_array(original, n);
            printf("\nExpected Result:\n");
            print_array(expected, n);
            printf("\nActual Result:\n");
            print_array(actual, n);
            printf("\n");
#endif
            return false;
        }
    }
    return true;
}

bool compare_checksums_1d(TYPE *original, TYPE *actual, TYPE *expected, unsigned n) {
    TYPE checksum_actual = 0;
    TYPE checksum_expected = 0;
    for (unsigned i = 0; i < n; i++) {
        checksum_actual = checksum_actual + actual[i];
        checksum_expected = checksum_expected + expected[i];
    }

    TYPE difference = abs_val(checksum_expected - checksum_actual);
    if (EPSILON < difference) {
#if defined(DEBUG) && DEBUG > 0
            printf("Checksum mismatch. Expected: "FORMAT", "
                   ",Actual: "FORMAT".\n", checksum_expected, checksum_actual);
            printf("\nOriginal Array:\n");
            print_array(original, n);
            printf("\nExpected Result:\n");
            print_array(expected, n);
            printf("\nActual Result:\n");
            print_array(actual, n);
            printf("\n");
#endif
        return false;
    }
    return true;
}

#endif // _CHECK_RESULTS_C_H_
