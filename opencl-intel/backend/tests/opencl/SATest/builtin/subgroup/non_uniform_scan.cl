__attribute__((intel_reqd_sub_group_size(4))) kernel void
test_max_4(global ulong *out, const global ulong *in) {
  size_t gid = get_global_id(0);
  uint subgroup_local_id = get_sub_group_local_id();
  if (subgroup_local_id & 1)
    out[gid] = sub_group_non_uniform_scan_exclusive_max(in[gid]);
}

__attribute__((intel_reqd_sub_group_size(8))) kernel void
test_max_8(global uint *out, const global uint *in) {
  size_t gid = get_global_id(0);
  uint subgroup_local_id = get_sub_group_local_id();
  if (subgroup_local_id & 1)
    out[gid] = sub_group_non_uniform_scan_exclusive_max(in[gid]);
}
