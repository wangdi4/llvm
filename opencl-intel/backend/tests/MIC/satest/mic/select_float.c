// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=0
// TODO: Enable test_select_float function when conditional move instruction is fixed.
#if 0
__kernel void test_select_float(__global float *out, __global const float *a, __global const float *b) {
    int index = get_global_id(0);
    if (a[index] < b[index])
    {
        out[index] = a[index];
    }
    else
    {
        out[index] = b[index];
    }
}
#endif

__kernel void test_select_float2(__global float2 *out, __global const float2 *a, __global const float2 *b) {
    int index = get_global_id(0);
    if (a[index].x < b[index].x)
    {
        out[index] = a[index];
    }
    else
    {
        out[index] = b[index];
    }
}

__kernel void test_select_float3(__global float3 *out, __global const float3 *a, __global const float3 *b) {
    int index = get_global_id(0);
    if (a[index].x < b[index].x)
    {
        out[index] = a[index];
    }
    else
    {
        out[index] = b[index];
    }
}

__kernel void test_select_float4(__global float4 *out, __global const float4 *a, __global const float4 *b) {
    int index = get_global_id(0);
    if (a[index].x < b[index].x)
    {
        out[index] = a[index];
    }
    else
    {
        out[index] = b[index];
    }
}

__kernel void test_select_float8(__global float8 *out, __global const float8 *a, __global const float8 *b) {
    int index = get_global_id(0);
    if (a[index].x < b[index].x)
    {
        out[index] = a[index];
    }
    else
    {
        out[index] = b[index];
    }
}

__kernel void test_select_float16(__global float16 *out, __global const float16 *a, __global const float16 *b) {
    int index = get_global_id(0);
    if (a[index].x < b[index].x)
    {
        out[index] = a[index];
    }
    else
    {
        out[index] = b[index];
    }
}

