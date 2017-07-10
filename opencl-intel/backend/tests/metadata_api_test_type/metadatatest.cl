__attribute__((vec_type_hint(float)))
__attribute__((work_group_size_hint(8,16,32)))
__attribute__((reqd_work_group_size(1,2,4)))
__attribute__((intel_reqd_sub_group_size(1)))
__kernel void metatest_kernel(float argFloat, __global int * argIntBuffer, __read_only image2d_t argImg) {
    return;
}
