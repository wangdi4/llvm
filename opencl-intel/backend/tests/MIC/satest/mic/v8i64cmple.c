
__kernel void test_v8i64cmple(__global long8 *out, __global const long8 *a,
                              __global const long8 *b) {
  int index = get_global_id(0);
  out[index] = a[index] <= b[index];
}
