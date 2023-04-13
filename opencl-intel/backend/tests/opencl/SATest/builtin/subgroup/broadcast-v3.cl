#define SUB_GROUP_SIZE 4

__attribute__((intel_reqd_sub_group_size(SUB_GROUP_SIZE))) kernel void
broadcast(global int3 *out1, global half3 *out2, const global int3 *in) {
  int gid = get_global_id(0);
  int sgid = get_sub_group_id();

  out1[gid] =
      sub_group_broadcast(in[gid], sgid == 0 ? 0 : (SUB_GROUP_SIZE - 1));
  out1[gid] += (int3)intel_sub_group_broadcast(
      (short)gid, sgid == 0 ? 0 : (SUB_GROUP_SIZE - 1));

  out2[gid] = sub_group_broadcast(convert_half3(in[gid]),
                                  sgid == 0 ? 0 : (SUB_GROUP_SIZE - 1));
}
