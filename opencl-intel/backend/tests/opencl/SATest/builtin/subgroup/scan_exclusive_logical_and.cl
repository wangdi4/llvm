#define SUB_GROUP_SIZE 4

__attribute__((intel_reqd_sub_group_size(SUB_GROUP_SIZE))) kernel void
scan_exclusive_logical_and(global long *out, const global long *in) {
  int gid = get_global_id(0);
  out[gid] = sub_group_non_uniform_scan_exclusive_logical_and(in[gid]);
}
