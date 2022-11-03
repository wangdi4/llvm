// TODO: uncomment vector select!

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void SelectTest(__global double4 *input, __global double4 *output,
                         const uint buffer_size) {
  uint tid = get_global_id(0);
  output[tid].xy = (input[tid].x < 15.0) ? input[tid].xx : input[tid].yy;
  //  output[tid].zw = (input[tid].zw >= (41.0, 41.0)) ? input[tid].xy :
  // input[tid].zw;
}
