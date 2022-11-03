#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void fcmp(__global double4 *input, __global double4 *output,
                   const uint buffer_size) {
  uint tid = get_global_id(0);
  output[tid].w = input[tid].x;
  if (input[tid].x < 20.0) {
    if (input[tid].y <= 10.0) {
      output[tid].y = input[tid].x + 5.0; // x < 20; y <= 10; z, w - any
    } else {
      output[tid].x = 100.0; // x < 20; y > 10; z, w - any
    }
  } else {
    if (input[tid].z == 41.0) {
      output[tid].z = 1.0; // x >= 20; z == 41; y, w - any
    } else {
      if (input[tid].w > 20.0) {
        if (input[tid].x != 30.0) {
          output[tid].x = 30.0; // x >= 20 && x != 30; z != 41; w > 20
        } else {
          output[tid].y = 40.0; // x == 30; z != 41; w > 20
        }
      } else {
        if (input[tid].x >= 41.0) {
          output[tid].x = 45.0; //
        } else {
          output[tid].x = 44.0;
        }
      }
    }
  }
}
