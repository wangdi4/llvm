#pragma OPENCL EXTENSION cl_khr_fp16 : enable
__attribute__((vec_type_hint(half))) __kernel void test_kernel_0() {}

__attribute__((vec_type_hint(half2))) __kernel void test_kernel_1() {}
