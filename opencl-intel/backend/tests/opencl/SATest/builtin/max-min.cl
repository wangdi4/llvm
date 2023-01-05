kernel void test(global int *a, global int *b, global int *dst) {
  size_t i = get_global_id(0);
  dst[i] = max(a[i], b[i]) + min(a[i], b[i]);
}
