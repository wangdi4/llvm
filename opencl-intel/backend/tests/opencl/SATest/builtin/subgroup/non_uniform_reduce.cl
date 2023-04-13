__attribute__((intel_reqd_sub_group_size(8))) kernel void
test_min_8(global long *out, const global long *in) {
  size_t gid = get_global_id(0);
  uint subgroup_local_id = get_sub_group_local_id();
  if (subgroup_local_id & (subgroup_local_id << 1))
    out[gid] = sub_group_non_uniform_reduce_min(in[gid]);
}
