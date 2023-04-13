#define SUB_GROUP_SIZE 4

__attribute__((intel_reqd_sub_group_size(SUB_GROUP_SIZE))) __kernel void
broadcast(__global int *out, __global int *in) {
  int gid = get_global_id(0);
  int sglid = get_sub_group_local_id();

  // Algorithm:
  // For each subgroup,
  // broadcast gid of in[gid] for the first item;
  // broadcast -gid of in[gid] for other items.
  if (sglid < 1)
    out[gid] = sub_group_non_uniform_broadcast(gid, in[gid]);
  else
    out[gid] = sub_group_non_uniform_broadcast(-gid, in[gid]);
}
