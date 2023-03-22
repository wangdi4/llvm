// TODO declare them in clang.
long __attribute__((overloadable)) __attribute__((convergent))
work_group_scan_inclusive_mul(long);
long __attribute__((overloadable)) __attribute__((convergent))
work_group_scan_exclusive_mul(long);

kernel void scan_integer(global int *inclusive_add, global int *inclusive_mul,
                         global int *inclusive_min, global int *inclusive_max,
                         global int *exclusive_add, global int *exclusive_mul,
                         global int *exclusive_min, global int *exclusive_max,
                         const global int *in) {
  int gid = get_global_id(0);

  inclusive_add[gid] = work_group_scan_inclusive_add(in[gid]);
  inclusive_mul[gid] = work_group_scan_inclusive_mul(in[gid]);
  inclusive_min[gid] = work_group_scan_inclusive_min(in[gid]);
  inclusive_max[gid] = work_group_scan_inclusive_max(in[gid]);

  exclusive_add[gid] = work_group_scan_exclusive_add(in[gid]);
  exclusive_mul[gid] = work_group_scan_exclusive_mul(in[gid]);
  exclusive_min[gid] = work_group_scan_exclusive_min(in[gid]);
  exclusive_max[gid] = work_group_scan_exclusive_max(in[gid]);
}
