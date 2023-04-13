#pragma OPENCL EXTENSION cl_intel_vec_len_hint : enable
#pragma OPENCL EXTENSION cl_intel_subgroups : enable
kernel void __attribute__((intel_vec_len_hint(1)))
test(int a, __global int *b) {
  *b = sub_group_all(a);
}
