/// b is greater than a, which is -10 or -2

kernel void test_ge(global ulong *dst, int b) {
  int gid = get_global_id(0);
  if (b - gid >= -10)
    dst[gid] = gid;
}

kernel void test_gt(global ulong *dst, int b) {
  int gid = get_global_id(0);
  if (b - gid > -10)
    dst[gid] = gid;
}

kernel void test_le(global ulong *dst, int b) {
  int gid = get_global_id(0);
  if (b - gid <= -2)
    dst[gid] = gid;
}

kernel void test_lt(global ulong *dst, int b) {
  int gid = get_global_id(0);
  if (b - gid < -2)
    dst[gid] = gid;
}

/// b equals a, which is 4

kernel void test_ge_aEQb(global ulong *dst, int b) {
  int gid = get_global_id(0);
  if (b - gid >= 4)
    dst[gid] = gid;
}

kernel void test_gt_aEQb(global ulong *dst, int b) {
  int gid = get_global_id(0);
  if (b - gid > 4)
    dst[gid] = gid;
}

kernel void test_le_aEQb(global ulong *dst, int b) {
  int gid = get_global_id(0);
  if (b - gid <= 4)
    dst[gid] = gid;
}

kernel void test_lt_aEQb(global ulong *dst, int b) {
  int gid = get_global_id(0);
  if (b - gid < 4)
    dst[gid] = gid;
}
