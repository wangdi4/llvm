// RUN: SATest -OCL -VAL -config=%s.cfg -neat=0 --force_ref | FileCheck %s
// CHECK: Test Passed.

__kernel void test_shuffle_2_2_char_uchar( __global char2* aIn,
                               __global uchar2* bIn,
                               __global char2* newData)
{
    char2 a = aIn[0];
    uchar2 b = bIn[0];
    char2 res = shuffle(a, b);
    newData[0] = res;
}

__kernel void test_shuffle_2_2_uchar_uchar( __global uchar2* aIn,
                               __global uchar2* bIn,
                               __global uchar2* newData)
{
    uchar2 a = aIn[0];
    uchar2 b = bIn[0];
    uchar2 res = shuffle(a, b);
    newData[0] = res;
}

__kernel void test_shuffle_2_2_short_ushort( __global short2* aIn,
                               __global ushort2* bIn,
                               __global short2* newData)
{
    short2 a = aIn[0];
    ushort2 b = bIn[0];
    short2 res = shuffle(a, b);
    newData[0] = res;
}

__kernel void test_shuffle_2_2_ushort_ushort( __global ushort2* aIn,
                               __global ushort2* bIn,
                               __global ushort2* newData)
{
    ushort2 a = aIn[0];
    ushort2 b = bIn[0];
    ushort2 res = shuffle(a, b);
    newData[0] = res;
}

__kernel void test_shuffle_2_2_int_uint( __global int2* aIn,
                               __global uint2* bIn,
                               __global int2* newData)
{
    int2 a = aIn[0];
    uint2 b = bIn[0];
    int2 res = shuffle(a, b);
    newData[0] = res;
}

__kernel void test_shuffle_2_2_uint_uint( __global uint2* aIn,
                               __global uint2* bIn,
                               __global uint2* newData)
{
    uint2 a = aIn[0];
    uint2 b = bIn[0];
    uint2 res = shuffle(a, b);
    newData[0] = res;
}

__kernel void test_shuffle_2_2_long_ulong( __global long2* aIn,
                               __global ulong2* bIn,
                               __global long2* newData)
{
    long2 a = aIn[0];
    ulong2 b = bIn[0];
    long2 res = shuffle(a, b);
    newData[0] = res;
}

__kernel void test_shuffle_2_2_ulong_ulong( __global ulong2* aIn,
                               __global ulong2* bIn,
                               __global ulong2* newData)
{
    ulong2 a = aIn[0];
    ulong2 b = bIn[0];
    ulong2 res = shuffle(a, b);
    newData[0] = res;
}
