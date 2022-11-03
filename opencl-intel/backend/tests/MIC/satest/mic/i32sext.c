
__kernel void test_i32sext(__global long *out, __global const int *in1) {
  int index = get_global_id(0);
  out[index] = in1[index];
}
