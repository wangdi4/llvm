__attribute__((intel_reqd_sub_group_size(8))) kernel void
test(global uint4 *out1, global uint *out2, global uint *out3,
     global uint4 *in) {
  int sglid = get_sub_group_local_id();
  out1[sglid] = sub_group_ballot(sglid % 2);
  out2[sglid] = sub_group_ballot_find_lsb(in[sglid]);
  out3[sglid] = sub_group_ballot_find_msb(in[sglid]);
}
