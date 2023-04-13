#pragma OPENCL EXTENSION cl_intel_subgroups : enable
kernel void test() { sub_group_barrier(CLK_LOCAL_MEM_FENCE); }
