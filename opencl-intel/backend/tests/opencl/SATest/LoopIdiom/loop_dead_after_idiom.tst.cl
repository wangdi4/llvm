kernel void test(global int *restrict dst, const global int *restrict src) {
  size_t gid = get_global_id(0);
  dst[gid] = src[gid];
}
