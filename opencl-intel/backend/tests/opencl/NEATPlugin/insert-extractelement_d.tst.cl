#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void InsertExtractElement(__global double4 *input,
                                   __global double4 *output,
                                   const uint buffer_size) {
  uint tid = get_global_id(0);
  output[tid].x = input[tid].x + 5.0;
  output[tid].y = input[tid].x + 5.0;
  output[tid].z = input[tid].x + 5.0;
  output[tid].w = input[tid].x + 5.0;
}
