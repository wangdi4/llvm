__attribute__((intel_reqd_sub_group_size(16))) kernel void
test(global uint *OUT) {
  int sglid = get_sub_group_local_id();
  uint4 ballot = sub_group_ballot(sglid % 2);
  *(global uint4 *)OUT = ballot;
}
