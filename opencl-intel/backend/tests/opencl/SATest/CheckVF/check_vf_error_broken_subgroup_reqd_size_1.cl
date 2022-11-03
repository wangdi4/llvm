#pragma OPENCL EXTENSION cl_intel_reqd_sub_group_size : enable
kernel void __attribute__((intel_reqd_sub_group_size(1))) test() {
  sub_group_barrier(CLK_LOCAL_MEM_FENCE);
}
