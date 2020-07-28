#pragma OPENCL EXTENSION cl_intel_subgroups: enable
#pragma OPENCL EXTENSION cl_intel_reqd_sub_group_size: enable

__attribute__((intel_reqd_sub_group_size(32)))
kernel void test(int a, __global int *b) {
*b = sub_group_all(a);
}

__attribute__((intel_reqd_sub_group_size(32)))
kernel void test_1() {
sub_group_barrier(CLK_LOCAL_MEM_FENCE);
}

__attribute__((intel_reqd_sub_group_size(16)))
kernel void test_2(int a, __global int *b) {
*b = work_group_all(a);
}

__attribute__((intel_reqd_sub_group_size(8)))
kernel void test_3(int a, __global int *b) {
*b = sub_group_any(a);
}
