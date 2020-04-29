#pragma OPENCL EXTENSION cl_intel_subgroups: enable
kernel void test(int a, __global int *b) {
*b = sub_group_all(a);
}

kernel void test_1(int a, __global size_t *b) {
*b = get_local_id(a);
sub_group_all(a);
}

void
__attribute__((noinline))
func(int a) {
sub_group_all(a);
}
kernel void test_2(int a) {
func(a);
}

