#pragma OPENCL EXTENSION cl_khr_fp64 : enable

typedef struct _str1 {
  double4 f4;
  int i;
} str1;

__kernel void InsertExtractValue(__global double4 *input,
                                 __global double4 *output,
                                 const uint buffer_size) {
  uint tid = get_global_id(0);
  str1 s = {(double4)(1.0, 2.0, 3.0, 4.0), 0};
  output[tid] = s.f4;
}
