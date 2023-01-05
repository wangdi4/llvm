#pragma OPENCL EXTENSION cl_intel_subgroups : enable
kernel void test(__global int *a, __global int *b) { *a += *b; }
