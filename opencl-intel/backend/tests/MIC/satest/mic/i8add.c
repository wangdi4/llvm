
__kernel void test_i8add(__global unsigned char *out,
                         __global const unsigned char *in1,
                         __global const unsigned char *in2) {
  int index = get_global_id(0);
  out[index] = in1[index] + in2[index];
}
