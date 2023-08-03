__kernel void test(__global int *res, unsigned long y, unsigned long x) {
  if (get_global_id(0) - y < x) {
    return;
  }
  atomic_add(res, 1 << get_global_id(0));
}
