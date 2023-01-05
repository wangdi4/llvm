#pragma OPENCL EXTENSION cl_intel_vec_len_hint : enable
__attribute__((vec_type_hint(int))) __attribute__((intel_vec_len_hint(4)))
kernel void
test(int a, __global int *b) {
  b[get_global_id(0)] = a;
}

__attribute__((vec_type_hint(int2))) __attribute__((intel_vec_len_hint(4)))
kernel void
test_1(int a, __global int *b) {
  b[get_global_id(0)] = a;
}

__attribute__((vec_type_hint(int2))) kernel void test_2(int a,
                                                        __global int *b) {
  b[get_global_id(0)] = a;
}
