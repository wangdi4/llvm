#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void test_shuffle2_4_2f(__global float4 *aIn, __global float4 *bIn,
                                 __global uint2 *cIn,
                                 __global float2 *newData) {
  float4 a = aIn[0];
  float4 b = bIn[0];
  uint2 c = cIn[0];
  float2 res = shuffle2(a, b, c);
  newData[0] = res;
}

__kernel void test_shuffle2_2_2f(__global float2 *aIn, __global float2 *bIn,
                                 __global uint2 *cIn,
                                 __global float2 *newData) {
  float2 a = aIn[0];
  float2 b = bIn[0];
  uint2 c = cIn[0];
  float2 res = shuffle2(a, b, c);
  newData[0] = res;
}

__kernel void test_shuffle2_2_4f(__global float2 *aIn, __global float2 *bIn,
                                 __global uint4 *cIn,
                                 __global float4 *newData) {
  float2 a = aIn[0];
  float2 b = bIn[0];
  uint4 c = cIn[0];
  float4 res = shuffle2(a, b, c);
  newData[0] = res;
}

__kernel void test_shuffle2_4_2d(__global double4 *aIn, __global double4 *bIn,
                                 __global ulong2 *cIn,
                                 __global double2 *newData) {
  double4 a = aIn[0];
  double4 b = bIn[0];
  ulong2 c = cIn[0];
  double2 res = shuffle2(a, b, c);
  newData[0] = res;
}

__kernel void test_shuffle2_2_2d(__global double2 *aIn, __global double2 *bIn,
                                 __global ulong2 *cIn,
                                 __global double2 *newData) {
  double2 a = aIn[0];
  double2 b = bIn[0];
  ulong2 c = cIn[0];
  double2 res = shuffle2(a, b, c);
  newData[0] = res;
}

__kernel void test_shuffle2_2_4d(__global double2 *aIn, __global double2 *bIn,
                                 __global ulong4 *cIn,
                                 __global double4 *newData) {
  double2 a = aIn[0];
  double2 b = bIn[0];
  ulong4 c = cIn[0];
  double4 res = shuffle2(a, b, c);
  newData[0] = res;
}