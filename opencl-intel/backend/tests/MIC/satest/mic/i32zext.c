
__kernel void test_i32zext(__global unsigned long *out,
                           __global const unsigned int *in1) {
  int index = get_global_id(0);
  out[index] = in1[index];
}
