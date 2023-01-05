kernel void foo(global float *a) {
  size_t gid = get_global_id(0);
  a[gid] = sin(a[gid]);
}

__attribute__((intel_reqd_sub_group_size(1))) kernel void bar(global float *a) {
  size_t gid = get_global_id(0);
  a[gid] = sin(a[gid]);
}
