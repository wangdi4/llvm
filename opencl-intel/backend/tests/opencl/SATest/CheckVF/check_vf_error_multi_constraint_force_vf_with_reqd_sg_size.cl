#pragma OPENCL EXTENSION cl_intel_reqd_sub_group_size : enable
kernel void __attribute__((intel_reqd_sub_group_size(8))) test() {}
