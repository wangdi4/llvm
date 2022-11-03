
__kernel void test_bitselect(__global int *out, __global const int *in1,
                             __global const int *in2, __global const int *in3) {
  int index = get_global_id(0);
  out[index] = bitselect(in1[index], in2[index], in3[index]);
}
