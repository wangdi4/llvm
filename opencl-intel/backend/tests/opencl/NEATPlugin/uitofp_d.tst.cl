#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void UIToFPTest(__global ulong4 *input, __global double4 *output,
                         const uint buffer_size) {
  uint tid = get_global_id(0);
  output[tid] = (double4)(10.0, 11.0, 11.0, 12.0) + (double4)input[tid].y;
  ulong z = 4u;
  output[tid].w = (double)z;
}
