#pragma OPENCL FP_CONTRACT OFF

__kernel void test_FP_CONTRACT(__global float *a, __global float *b,
                               __global float *c, __global float *newData) {
  int tid = get_global_id(0);
  float sum = a[tid] * b[tid] + c[tid];
  newData[tid] = sum;
}