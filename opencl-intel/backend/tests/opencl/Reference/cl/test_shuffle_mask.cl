// RUN: SATest -OCL -VAL -config=%s.cfg -neat=1 --force_ref | FileCheck %s
// CHECK: Test Passed.

__kernel void test_shuffle_4( __global float4* aIn,
                              __global uint4* bIn,
                              __global float4* newData)
{
    float4 a = aIn[0];
    uint4 b = bIn[0];
    float4 res = shuffle(a, b);
    newData[0] = res;
}