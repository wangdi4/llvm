//==-------------  Tests for OpenMP support -*- OpenCL -*-------------------==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

void do_par_loop_reduction(
  int *x,
  int *y
)
{
  const int N1 = 100;
  const int N2 = 100;
  int xR = 0, yR = 100;

#pragma omp parallel for reduction(+:xR) reduction(-:yR)
  for (int i = 0; i < N1; ++i) {
    xR += 1;
    yR -= 1;
  }
  *x = xR;
  *y = yR;
}

__kernel void loop_trip_test_br(__global int* error) {
  *error = -1;

  int x = 0;
  int y = 0;

  do_par_loop_reduction(&x, &y);

  if (x != 100 || y != 0) {
    printf ("x = %d y = %d \nFAILED\n", x, y);
      return;
  }
  printf ("PASSED\n");
  *error = 0;
}

