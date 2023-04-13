#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void test_100019884(__global double4 *aIn, __global double4 *newData) {
  double4 a = aIn[0] + 2.0;
  newData[0] = a;
}
