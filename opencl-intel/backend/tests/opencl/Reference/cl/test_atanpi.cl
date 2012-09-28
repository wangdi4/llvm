// RUN: SATest -OCL -VAL -config=%s.cfg -neat=1 --force_ref | FileCheck %s
// CHECK: Test Passed.
#pragma OPENCL EXTENSION cl_khr_fp64: enable

__kernel void test_atanpi( __global double3* aIn,
                           __global double3* newData)
{
    double3 a = aIn[0];
    double3 res = atanpi(a);
    newData[0] = res;
}