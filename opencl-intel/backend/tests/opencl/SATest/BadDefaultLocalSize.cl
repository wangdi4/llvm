__kernel void test(__global int *out) {
  size_t i = get_global_id(0);
  out[i] = 77;
}
