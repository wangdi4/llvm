#pragma OPENCL EXTENSION cl_intel_vec_len_hint : enable
kernel void __attribute__((intel_vec_len_hint(8))) test() {}
