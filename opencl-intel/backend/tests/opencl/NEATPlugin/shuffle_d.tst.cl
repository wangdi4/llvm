#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void ShuffleTest(__global double4 *input, __global double4 *output,
                          const uint buffer_size) {
  uint tid = get_global_id(0);
  output[tid] = input[tid].xxxx;
}
