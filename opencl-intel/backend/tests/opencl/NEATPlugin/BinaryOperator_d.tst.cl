#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void BinaryKernel(__global double *input, __global double *output,
                           const uint buffer_size) {
  uint tid = get_global_id(0);
  output[tid] = input[tid] + 1.0;
  output[tid] = output[tid] / 5.0;
  output[tid] = 2.0 - output[tid];
  output[tid] = output[tid] * input[tid];
}
