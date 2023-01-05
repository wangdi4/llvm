__kernel void test_shuffle2_4(__global float4 *aIn, __global float4 *bIn,
                              __global uint4 *cIn, __global float4 *newData) {
  float4 a = aIn[0];
  float4 b = bIn[0];
  uint4 c = cIn[0];
  float4 res = shuffle2(a, b, c);
  newData[0] = res;
}