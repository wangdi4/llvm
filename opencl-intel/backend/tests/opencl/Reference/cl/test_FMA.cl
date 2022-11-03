#pragma OPENCL FP_CONTRACT ON
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void testFMA_f(__global float *a, __global float *b, __global float *c,
                        __global float *newData) {
  int tid = get_global_id(0);
  float sum = a[tid] * b[tid] + c[tid];
  newData[tid] = sum;
}

__kernel void testFMA_d(__global double *a, __global double *b,
                        __global double *c, __global double *newData) {
  int tid = get_global_id(0);
  double sum = a[tid] * b[tid] + c[tid];
  newData[tid] = sum;
}
