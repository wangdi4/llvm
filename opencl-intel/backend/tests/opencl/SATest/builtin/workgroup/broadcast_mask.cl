int __attribute__((overloadable)) __attribute__((convergent))
work_group_broadcast(int, size_t);

kernel void test(global int *a, const global int *in) {
  int gid = get_global_id(0);
  if (gid < 5)
    a[gid] = work_group_broadcast(in[gid], 1);
}
