
__kernel void test_outofbounds_add_before(__global long8 *out,
                                          __global const long8 *a,
                                          __global const long8 *b) {
  int index = get_global_id(0);
  out[index - 1] = a[index] + b[index];
}
