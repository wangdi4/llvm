#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void test_shuffle_2_2f(__global float2 *aIn, __global uint2 *bIn,
                                __global float2 *newData) {
  float2 a = aIn[0];
  uint2 b = bIn[0];
  float2 res = shuffle(a, b);
  newData[0] = res;
}

__kernel void test_shuffle_4_2f(__global float4 *aIn, __global uint2 *bIn,
                                __global float2 *newData) {
  float4 a = aIn[0];
  uint2 b = bIn[0];
  float2 res = shuffle(a, b);
  newData[0] = res;
}
__kernel void test_shuffle_2_4f(__global float2 *aIn, __global uint4 *bIn,
                                __global float4 *newData) {
  float2 a = aIn[0];
  uint4 b = bIn[0];
  float4 res = shuffle(a, b);
  newData[0] = res;
}

__kernel void test_shuffle_2_2d(__global double2 *aIn, __global ulong2 *bIn,
                                __global double2 *newData) {
  double2 a = aIn[0];
  ulong2 b = bIn[0];
  double2 res = shuffle(a, b);
  newData[0] = res;
}

__kernel void test_shuffle_4_2d(__global double4 *aIn, __global ulong2 *bIn,
                                __global double2 *newData) {
  double4 a = aIn[0];
  ulong2 b = bIn[0];
  double2 res = shuffle(a, b);
  newData[0] = res;
}

__kernel void test_shuffle_2_4d(__global double2 *aIn, __global ulong4 *bIn,
                                __global double4 *newData) {
  double2 a = aIn[0];
  ulong4 b = bIn[0];
  double4 res = shuffle(a, b);
  newData[0] = res;
}