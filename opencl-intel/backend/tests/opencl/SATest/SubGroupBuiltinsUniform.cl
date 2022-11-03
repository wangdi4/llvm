#pragma OPENCL EXTENSION cl_intel_subgroups : enable
#define SUB_GROUP_SIZE 16

#define __ovld __attribute__((overloadable))
float __ovld sub_group_reduce_mul(float);
int __ovld sub_group_reduce_logical_and(int);
int __ovld sub_group_reduce_logical_or(int);
int __ovld sub_group_reduce_logical_xor(int);
int __ovld sub_group_reduce_bitwise_and(int);
int __ovld sub_group_reduce_bitwise_or(int);
int __ovld sub_group_reduce_bitwise_xor(int);

__attribute__((intel_reqd_sub_group_size(SUB_GROUP_SIZE))) __kernel void
SubGroupBuiltin(__global float *sg_reduce_mul,
                __global int *sg_reduce_logical_and,
                __global int *sg_reduce_logical_or,
                __global int *sg_reduce_logical_xor,
                __global int *sg_reduce_bitwise_and,
                __global int *sg_reduce_bitwise_or,
                __global int *sg_reduce_bitwise_xor) {
  int lid = get_local_id(0);

  float temp_f = 2;

  // reduce builtin
  sg_reduce_mul[lid] = sub_group_reduce_mul(temp_f);
  sg_reduce_logical_and[lid] = sub_group_reduce_logical_and(lid);
  sg_reduce_logical_or[lid] = sub_group_reduce_logical_or(lid);
  sg_reduce_logical_xor[lid] = sub_group_reduce_logical_xor(lid);
  sg_reduce_bitwise_and[lid] = sub_group_reduce_bitwise_and(lid);
  sg_reduce_bitwise_or[lid] = sub_group_reduce_bitwise_or(lid);
  sg_reduce_bitwise_xor[lid] = sub_group_reduce_bitwise_xor(lid);
}
