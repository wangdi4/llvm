#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#define SUB_GROUP_SIZE 8
#define LOCAL_WORK_SIZE 8

__attribute__((intel_reqd_sub_group_size(SUB_GROUP_SIZE))) __kernel void
shuffle_half_type(__global half *data1) {
  int sglid = get_sub_group_local_id();
  half temp = sglid - 0.1;
  uint idx;
  if (sglid == 0)
    idx = SUB_GROUP_SIZE - 1;
  else
    idx = sglid - 1;

  data1[sglid] = intel_sub_group_shuffle(temp, idx);
}
