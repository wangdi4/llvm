__kernel void test(__global int *res, long y, long x) {
  if (get_global_id(0) - y < x) {
    return;
  }
  atomic_add(res, 1 << get_global_id(0));
}
