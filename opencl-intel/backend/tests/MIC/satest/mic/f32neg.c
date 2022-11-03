
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void test_fxor32(__global float *out, __global const float *in) {
  int index = get_global_id(0);
  out[index] = -0.000000e+00 - in[index];
}
