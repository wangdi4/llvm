#pragma OPENCL EXTENSION cl_intel_subgroups : enable
#pragma OPENCL EXTENSION cl_intel_reqd_sub_group_size : enable

__attribute__((intel_reqd_sub_group_size(128))) kernel void
test(int a, __global int *b) {
  *b = sub_group_all(a);
  *b = work_group_all(a);
}
