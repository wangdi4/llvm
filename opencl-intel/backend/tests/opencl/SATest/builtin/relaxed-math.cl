kernel void test(global float *a) {
  size_t gid = get_global_id(0);
  a[gid] = sin(a[gid]);
}
