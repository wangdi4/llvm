#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#define SUB_GROUP_SIZE 8

#define __ovld __attribute__((overloadable))
half __ovld sub_group_reduce_mul(half);
half __ovld sub_group_scan_exclusive_mul(half);

__attribute__((intel_reqd_sub_group_size(SUB_GROUP_SIZE))) __kernel void
reduce_half_type(__global half *data1, __global half *data2,
                 __global half *data3, __global half *data4,
                 __global half *ex_data1, __global half *ex_data2,
                 __global half *in_data1, __global half *in_data2,
                 __global half *in_data3) {
  int sglid = get_sub_group_local_id();
  half temp = sglid - 0.5;
  half temp_add = sglid + 789;
  data1[sglid] = sub_group_reduce_add(temp_add);
  data2[sglid] = sub_group_reduce_min(temp);
  data3[sglid] = sub_group_reduce_max(temp);
  data4[sglid] = sub_group_reduce_mul(temp);

  ex_data1[sglid] = sub_group_scan_exclusive_add(temp_add);
  ex_data2[sglid] = sub_group_scan_exclusive_mul(temp);
  in_data1[sglid] = sub_group_scan_inclusive_min(temp);
  in_data2[sglid] = sub_group_scan_inclusive_max(temp);
  in_data3[sglid] = sub_group_scan_inclusive_add(temp_add);
}
