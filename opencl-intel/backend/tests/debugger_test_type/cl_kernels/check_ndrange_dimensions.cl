__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  size_t workdim = get_work_dim();

  size_t globalsize0 = get_global_size(0);
  size_t globalsize1 = get_global_size(1);
  size_t globalsize2 = get_global_size(2);

  size_t localsize0 = get_local_size(0);
  size_t localsize1 = get_local_size(1);
  size_t localsize2 = get_local_size(2);

  size_t numgroups0 = get_num_groups(0);
  size_t numgroups1 = get_num_groups(1);
  size_t numgroups2 = get_num_groups(2);

  size_t gid0 = get_global_id(0);
  size_t gid1 = get_global_id(1);
  size_t gid2 = get_global_id(2);

  size_t lid0 = get_local_id(0);
  size_t lid1 = get_local_id(1);
  size_t lid2 = get_local_id(2);

  size_t groupid0 = get_group_id(0);
  size_t groupid1 = get_group_id(1);
  size_t groupid2 = get_group_id(2);

  buf_out[0] = 0;
}
