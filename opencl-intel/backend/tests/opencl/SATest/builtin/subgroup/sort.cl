#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#define SUB_GROUP_SIZE 4

char __devicelib_default_sub_group_private_sort_ascending_i8(char);
uchar __devicelib_default_sub_group_private_sort_descending_u8(uchar);
short __devicelib_default_sub_group_private_sort_ascending_i16(short);
ushort __devicelib_default_sub_group_private_sort_descending_u16(ushort);
int __devicelib_default_sub_group_private_sort_ascending_i32(int);
uint __devicelib_default_sub_group_private_sort_descending_u32(uint);
long __devicelib_default_sub_group_private_sort_ascending_i64(long);
ulong __devicelib_default_sub_group_private_sort_descending_u64(ulong);
float __devicelib_default_sub_group_private_sort_ascending_f32(float);
double __devicelib_default_sub_group_private_sort_descending_f64(double);
half __devicelib_default_sub_group_private_sort_ascending_f16(half);

__attribute__((intel_reqd_sub_group_size(SUB_GROUP_SIZE))) __kernel void
SubGroupSort(__global char *data1, __global uchar *data2, __global short *data3,
             __global ushort *data4, __global int *data5, __global uint *data6,
             __global long *data7, __global ulong *data8, __global float *data9,
             __global double *data10, __global half *data11) {
  int lid = get_local_id(0);

  data1[lid] =
      __devicelib_default_sub_group_private_sort_ascending_i8(data1[lid]);

  data2[lid] =
      __devicelib_default_sub_group_private_sort_descending_u8(data2[lid]);

  data3[lid] =
      __devicelib_default_sub_group_private_sort_ascending_i16(data3[lid]);

  data4[lid] =
      __devicelib_default_sub_group_private_sort_descending_u16(data4[lid]);

  data5[lid] =
      __devicelib_default_sub_group_private_sort_ascending_i32(data5[lid]);

  data6[lid] =
      __devicelib_default_sub_group_private_sort_descending_u32(data6[lid]);

  data7[lid] =
      __devicelib_default_sub_group_private_sort_ascending_i64(data7[lid]);

  data8[lid] =
      __devicelib_default_sub_group_private_sort_descending_u64(data8[lid]);

  data11[lid] =
      __devicelib_default_sub_group_private_sort_ascending_f16(data11[lid]);

  data9[lid] =
      __devicelib_default_sub_group_private_sort_ascending_f32(data9[lid]);

  data10[lid] =
      __devicelib_default_sub_group_private_sort_descending_f64(data10[lid]);
}
