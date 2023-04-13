/// b is not equal to a

kernel void test_ge(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (b - gid >= 2)
    dst[gid] = gid;
}

kernel void test_gt(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (b - gid > 2)
    dst[gid] = gid;
}

kernel void test_le(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (b - gid <= 2)
    dst[gid] = gid;
}

kernel void test_lt(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (b - gid < 2)
    dst[gid] = gid;
}

/// b equals a, which is 4

kernel void test_ge_aEQb(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (b - gid >= 4)
    dst[gid] = gid;
}

kernel void test_gt_aEQb(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (b - gid > 4)
    dst[gid] = gid;
}

kernel void test_le_aEQb(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (b - gid <= 4)
    dst[gid] = gid;
}

kernel void test_lt_aEQb(global ulong *dst, uint b) {
  size_t gid = get_global_id(0);
  if (b - gid < 4)
    dst[gid] = gid;
}
