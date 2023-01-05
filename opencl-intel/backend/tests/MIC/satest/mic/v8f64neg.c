
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void test_v8fxor64(__global double8 *out, __global const double8 *in) {
  int index = get_global_id(0);
  out[index] = -0.000000e+00 - in[index];
}
