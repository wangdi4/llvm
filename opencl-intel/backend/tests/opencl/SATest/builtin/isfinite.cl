kernel void test(global float *a) {
  size_t gid = get_global_id(0);
  a[gid] = get_sub_group_size();
  if (gid < 4)
    a[gid] += isfinite(as_float(0x7F800000));
  else
    a[gid] += isfinite(as_float(0x00800000));
}
