#pragma OPENCL EXTENSION cl_intel_reqd_sub_group_size: enable
#pragma OPENCL EXTENSION cl_intel_vec_len_hint: enable
kernel void  __attribute__((intel_reqd_sub_group_size(8)))
test() {
}

kernel void
__attribute__((intel_reqd_sub_group_size(8)))
__attribute__((intel_vec_len_hint(4)))
test_1() {
}

kernel void  __attribute__((intel_vec_len_hint(8)))
test_2() {
}

