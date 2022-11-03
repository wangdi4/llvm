
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void test_fxor64(__global double *out, __global const double *in) {
  int index = get_global_id(0);
  out[index] = -0.000000e+00 - in[index];
}
