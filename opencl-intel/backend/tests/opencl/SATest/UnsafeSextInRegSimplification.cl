/*
  The following function contains two select instructions of different
  scalar types that depend on the same mask (the two assignments under
  the if). This combination was being lowered incorrectly to X86, causing
  the 'double' select to use a bad mask (all zeros).
  The bug was fixed in LLVM. If the bug persists, this test will produce
  incorrect results due to the invalid select implementation.
*/
#define W 2

#pragma OPENCL EXTENSION cl_khr_fp64 : enable // for double

int findOptimalK(float value, int index) {
  int optK = -W;
  double WEIGHT0 = 0.73;
  double WEIGHT1 = -1.22;
  double interpolVal =
      WEIGHT0 * ((index - W) * 10) + WEIGHT1 * ((index + W) * 10);
  double minDiff = value - interpolVal;
  for (int k = -W + 1; k <= W; k++) {
    double div_by = k != 0 ? k : 1;
    interpolVal = WEIGHT0 * ((-index - W) * 10 / div_by) +
                  WEIGHT1 * ((-index + W) * 10 / div_by);
    double diff = value - interpolVal;
    if (diff < minDiff) {
      optK = k;
      minDiff = diff;
    }
  }
  return optK;
}

__kernel void test(__global float *a, __global int *out) {
  int gid = get_global_id(0);
  int optK = findOptimalK(a[gid], gid);
  out[gid] = optK;
}
