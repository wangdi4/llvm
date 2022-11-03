
__kernel void test_i64trunc(__global int *out, __global const long *in1) {
  int index = get_global_id(0);
  out[index] = in1[index];
}
