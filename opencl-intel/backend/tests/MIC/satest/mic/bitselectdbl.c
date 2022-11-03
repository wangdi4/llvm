
__kernel void test_bitselectdbl(__global double *out,
                                __global const double *in1,
                                __global const double *in2,
                                __global const double *in3) {
  int index = get_global_id(0);
  out[index] = bitselect(in1[index], in2[index], in3[index]);
}
