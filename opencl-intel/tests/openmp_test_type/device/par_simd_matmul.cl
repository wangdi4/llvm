//==-------------  Tests for OpenMP support -*- OpenCL -*-------------------==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#define N 128

void do_par_matmul(
    int x[][N],
    int y[][N],
    int z[][N]
    )
{
#pragma omp parallel for num_threads(4)
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int sum = 0;
#pragma omp simd reduction(+: sum)
            for (int k = 0; k < N; ++k) {
                sum = sum + x[i][k] * y[k][j];
            }
            z[i][j] = sum;
        }
    }
}

// Main kernel for the test
__kernel void par_simd_matmul(__global int* error) {
    *error = -1;

    // unsigned thread_id = get_global_id(0);
    // Note: there is only one OpenCL thread created by the host program.
    int x[N][N];
    int y[N][N];
    int z[N][N];

#pragma omp parallel for num_threads(4)
    for (int i = 0; i < N; ++i) {
#pragma omp simd
        for (int j = 0; j < N; ++j) {
            x[i][j] = 1;
            y[i][j] = 2;
            z[i][j] = 0;
        }
    }

    do_par_matmul(x, y, z);

    if (z[2][2] != 256) {
        printf ("z = %d \nFAILED\n", z[2][2]);
        return;
    }
    printf ("PASSED\n");
    *error = 0;
}

