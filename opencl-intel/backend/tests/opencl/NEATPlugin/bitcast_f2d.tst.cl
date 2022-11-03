#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void BitcastTest_f2d(__global float2 *input, __global double *output,
                              const uint buffer_size) {
  uint tid = get_global_id(0);
  double tmp =
      as_double(input[tid]) * (double)(0.0f, 0.0f) + (double)(0.0f, 0.2f);
  input[tid] = as_float2(tmp);
  output[tid] = tmp;
}
