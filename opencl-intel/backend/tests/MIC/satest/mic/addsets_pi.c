
__kernel void test_addsets_pi(__global int *out, __global const int *in1,
                              __global const int *in2) {
  int index = get_global_id(0);
  out[index] = add_sat(in1[index], in2[index]);
}
