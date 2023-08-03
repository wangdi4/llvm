// TODO declare them in clang.
long __attribute__((overloadable)) __attribute__((convergent))
work_group_scan_inclusive_mul(long);
long __attribute__((overloadable)) __attribute__((convergent))
work_group_scan_exclusive_mul(long);
int __attribute__((overloadable)) __attribute__((convergent))
work_group_scan_inclusive_bitwise_and(int);
int __attribute__((overloadable)) __attribute__((convergent))
work_group_scan_inclusive_bitwise_or(int);
int __attribute__((overloadable)) __attribute__((convergent))
work_group_scan_inclusive_bitwise_xor(int);
int __attribute__((overloadable)) __attribute__((convergent))
work_group_scan_exclusive_bitwise_and(int);
int __attribute__((overloadable)) __attribute__((convergent))
work_group_scan_exclusive_bitwise_or(int);
int __attribute__((overloadable)) __attribute__((convergent))
work_group_scan_exclusive_bitwise_xor(int);

kernel void scan_integer(global int *inclusive_add, global int *inclusive_mul,
                         global int *inclusive_min, global int *inclusive_max,
                         global int *inclusive_bitwise_and,
                         global int *inclusive_bitwise_or,
                         global int *inclusive_bitwise_xor,
                         global int *exclusive_add, global int *exclusive_mul,
                         global int *exclusive_min, global int *exclusive_max,
                         global int *exclusive_bitwise_and,
                         global int *exclusive_bitwise_or,
                         global int *exclusive_bitwise_xor,
                         const global int *in) {
  int gid = get_global_id(0);

  inclusive_add[gid] = work_group_scan_inclusive_add(in[gid]);
  inclusive_mul[gid] = work_group_scan_inclusive_mul(in[gid]);
  inclusive_min[gid] = work_group_scan_inclusive_min(in[gid]);
  inclusive_max[gid] = work_group_scan_inclusive_max(in[gid]);
  inclusive_bitwise_and[gid] = work_group_scan_inclusive_bitwise_and(in[gid]);
  inclusive_bitwise_or[gid] = work_group_scan_inclusive_bitwise_or(in[gid]);
  inclusive_bitwise_xor[gid] = work_group_scan_inclusive_bitwise_xor(in[gid]);

  exclusive_add[gid] = work_group_scan_exclusive_add(in[gid]);
  exclusive_mul[gid] = work_group_scan_exclusive_mul(in[gid]);
  exclusive_min[gid] = work_group_scan_exclusive_min(in[gid]);
  exclusive_max[gid] = work_group_scan_exclusive_max(in[gid]);
  exclusive_bitwise_and[gid] = work_group_scan_exclusive_bitwise_and(in[gid]);
  exclusive_bitwise_or[gid] = work_group_scan_exclusive_bitwise_or(in[gid]);
  exclusive_bitwise_xor[gid] = work_group_scan_exclusive_bitwise_xor(in[gid]);
}
