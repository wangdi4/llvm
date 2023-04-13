/// b is not equal to a

kernel void test_ge(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (10 >= b - gid)
    dst[gid] = gid;
}

kernel void test_gt(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (10 > b - gid)
    dst[gid] = gid;
}

kernel void test_le(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (2 <= b - gid)
    dst[gid] = gid;
}

kernel void test_lt(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (2 < b - gid)
    dst[gid] = gid;
}

/// b equals a, which is 4

kernel void test_ge_aEQb(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (4 >= b - gid)
    dst[gid] = gid;
}

kernel void test_gt_aEQb(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (4 > b - gid)
    dst[gid] = gid;
}

kernel void test_le_aEQb(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (4 <= b - gid)
    dst[gid] = gid;
}

kernel void test_lt_aEQb(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (4 < b - gid)
    dst[gid] = gid;
}
