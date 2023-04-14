__kernel void test_right_bound(__global int *a, int n) {
  size_t gid = get_group_id(0) * get_local_size(0) + get_local_id(0);
  if (gid < n)
    a[gid] = gid;
}
