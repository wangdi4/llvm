#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#define SUB_GROUP_SIZE 8

__attribute__((intel_reqd_sub_group_size(SUB_GROUP_SIZE))) __kernel void
broadcast(__global half *data1, __global half4 *data4) {
  int sglid = get_sub_group_local_id();

  half temp = sglid - 0.1;
  data1[sglid] = sub_group_broadcast(temp, 5);

  half4 temp4 = {sglid - 0.1, sglid - 0.2, sglid - 0.3, sglid - 0.4};
  data4[sglid] = sub_group_broadcast(temp4, 5);
}
