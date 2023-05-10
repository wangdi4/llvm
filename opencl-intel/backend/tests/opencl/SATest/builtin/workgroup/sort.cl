// Check results for work group sort builtin

// char
void __devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8(
    __global char *, uint, __local char *);
void __devicelib_default_work_group_private_sort_close_descending_p1u8_u32_p1i8(
    __global uchar *, uint, __local char *);
// short
void __devicelib_default_work_group_private_sort_close_ascending_p1i16_u32_p1i8(
    __global short *, uint, __local char *);
void __devicelib_default_work_group_private_sort_close_descending_p1u16_u32_p1i8(
    __global ushort *, uint, __local char *);
// int
void __devicelib_default_work_group_private_sort_close_ascending_p1i32_u32_p3i8(
    __global int *, uint, __local char *);
void __devicelib_default_work_group_private_sort_close_descending_p1u32_u32_p3i8(
    __global uint *, uint, __local char *);
// long
void __devicelib_default_work_group_private_sort_close_ascending_p1i64_u32_p1i8(
    __global long *, uint, __local char *);
void __devicelib_default_work_group_private_sort_close_descending_p1u64_u32_p1i8(
    __global ulong *, uint, __local char *);
// float
void __devicelib_default_work_group_private_sort_close_ascending_p1f32_u32_p1i8(
    __global float *, uint, __local char *);
// double
void __devicelib_default_work_group_private_sort_close_descending_p1f64_u32_p3i8(
    __global double *, uint, __local char *);
// half
void __devicelib_default_work_group_private_sort_close_ascending_p1f16_u32_p3i8(
    __global half *, uint, __local char *);

const uint pre_wg_item_data_size = 4;

__attribute__((intel_reqd_sub_group_size(1))) __kernel void
WG_private_close_sort(__global char *data1, __global uchar *data2,
                      __global short *data3, __global ushort *data4,
                      __global int *data5, __global uint *data6,
                      __global long *data7, __global ulong *data8,
                      __global float *data9, __global double *data10,
                      __global half *data11) {
  __local char scratch1[1024];
  __local char scratch2[1024];
  __local char scratch3[1024];
  __local char scratch4[1024];
  __local char scratch5[1024];
  __local char scratch6[1024];
  __local char scratch7[1024];
  __local char scratch8[1024];
  __local char scratch9[1024];
  __local char scratch10[1024];
  __local char scratch11[1024];

  int lidx = get_local_linear_id();
  int data_offset = lidx * pre_wg_item_data_size;

  // char
  __devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8(
      data1 + data_offset, pre_wg_item_data_size, scratch1);

  // uchar
  __devicelib_default_work_group_private_sort_close_descending_p1u8_u32_p1i8(
      data2 + data_offset, pre_wg_item_data_size, scratch2);

  // short
  __devicelib_default_work_group_private_sort_close_ascending_p1i16_u32_p1i8(
      data3 + data_offset, pre_wg_item_data_size, scratch3);

  // ushort
  __devicelib_default_work_group_private_sort_close_descending_p1u16_u32_p1i8(
      data4 + data_offset, pre_wg_item_data_size, scratch4);

  // int
  __devicelib_default_work_group_private_sort_close_ascending_p1i32_u32_p3i8(
      data5 + data_offset, pre_wg_item_data_size, scratch5);

  // uint
  __devicelib_default_work_group_private_sort_close_descending_p1u32_u32_p3i8(
      data6 + data_offset, pre_wg_item_data_size, scratch6);

  // long
  __devicelib_default_work_group_private_sort_close_ascending_p1i64_u32_p1i8(
      data7 + data_offset, pre_wg_item_data_size, scratch7);

  // ulong
  __devicelib_default_work_group_private_sort_close_descending_p1u64_u32_p1i8(
      data8 + data_offset, pre_wg_item_data_size, scratch8);

  // float
  __devicelib_default_work_group_private_sort_close_ascending_p1f32_u32_p1i8(
      data9 + data_offset, pre_wg_item_data_size, scratch9);

  // double
  __devicelib_default_work_group_private_sort_close_descending_p1f64_u32_p3i8(
      data10 + data_offset, pre_wg_item_data_size, scratch10);

  // half
  __devicelib_default_work_group_private_sort_close_ascending_p1f16_u32_p3i8(
      data11 + data_offset, pre_wg_item_data_size, scratch11);
}

// char
void __devicelib_default_work_group_private_sort_spread_ascending_p1i8_u32_p3i8(
    __global char *, uint, __local char *);
void __devicelib_default_work_group_private_sort_spread_descending_p1u8_u32_p3i8(
    __global uchar *, uint, __local char *);
// short
void __devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p1i8(
    __global short *, uint, __local char *);
void __devicelib_default_work_group_private_sort_spread_descending_p1u16_u32_p1i8(
    __global ushort *, uint, __local char *);
// int
void __devicelib_default_work_group_private_sort_spread_ascending_p1i32_u32_p1i8(
    __global int *, uint, __local char *);
void __devicelib_default_work_group_private_sort_spread_descending_p1u32_u32_p1i8(
    __global uint *, uint, __local char *);
// long
void __devicelib_default_work_group_private_sort_spread_ascending_p1i64_u32_p3i8(
    __global long *, uint, __local char *);
void __devicelib_default_work_group_private_sort_spread_descending_p1u64_u32_p3i8(
    __global ulong *, uint, __local char *);
// float
void __devicelib_default_work_group_private_sort_spread_ascending_p1f32_u32_p1i8(
    __global float *, uint, __local char *);
// double
void __devicelib_default_work_group_private_sort_spread_descending_p1f64_u32_p1i8(
    __global double *, uint, __local char *);
// half
void __devicelib_default_work_group_private_sort_spread_ascending_p1f16_u32_p1i8(
    __global half *, uint, __local char *);

__attribute__((intel_reqd_sub_group_size(1))) __kernel void
WG_private_spread_sort(__global char *data1, __global uchar *data2,
                       __global short *data3, __global ushort *data4,
                       __global int *data5, __global uint *data6,
                       __global long *data7, __global ulong *data8,
                       __global float *data9, __global double *data10,
                       __global half *data11) {
  __local char scratch1[1024];
  __local char scratch2[1024];
  __local char scratch3[1024];
  __local char scratch4[1024];
  __local char scratch5[1024];
  __local char scratch6[1024];
  __local char scratch7[1024];
  __local char scratch8[1024];
  __local char scratch9[1024];
  __local char scratch10[1024];
  __local char scratch11[1024];

  int lidx = get_local_linear_id();
  int data_offset = lidx * pre_wg_item_data_size;

  // char
  __devicelib_default_work_group_private_sort_spread_ascending_p1i8_u32_p3i8(
      data1 + data_offset, pre_wg_item_data_size, scratch1);

  // uchar
  __devicelib_default_work_group_private_sort_spread_descending_p1u8_u32_p3i8(
      data2 + data_offset, pre_wg_item_data_size, scratch2);

  // short
  __devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p1i8(
      data3 + data_offset, pre_wg_item_data_size, scratch3);

  // ushort
  __devicelib_default_work_group_private_sort_spread_descending_p1u16_u32_p1i8(
      data4 + data_offset, pre_wg_item_data_size, scratch4);

  // int
  __devicelib_default_work_group_private_sort_spread_ascending_p1i32_u32_p1i8(
      data5 + data_offset, pre_wg_item_data_size, scratch5);

  // uint
  __devicelib_default_work_group_private_sort_spread_descending_p1u32_u32_p1i8(
      data6 + data_offset, pre_wg_item_data_size, scratch6);

  // long
  __devicelib_default_work_group_private_sort_spread_ascending_p1i64_u32_p3i8(
      data7 + data_offset, pre_wg_item_data_size, scratch7);

  // ulong
  __devicelib_default_work_group_private_sort_spread_descending_p1u64_u32_p3i8(
      data8 + data_offset, pre_wg_item_data_size, scratch8);

  // float
  __devicelib_default_work_group_private_sort_spread_ascending_p1f32_u32_p1i8(
      data9 + data_offset, pre_wg_item_data_size, scratch9);

  // double
  __devicelib_default_work_group_private_sort_spread_descending_p1f64_u32_p1i8(
      data10 + data_offset, pre_wg_item_data_size, scratch10);

  // half
  __devicelib_default_work_group_private_sort_spread_ascending_p1f16_u32_p1i8(
      data11 + data_offset, pre_wg_item_data_size, scratch11);
}

// char
void __devicelib_default_work_group_joint_sort_ascending_p1i8_u32_p1i8(
    __global char *, uint, __local char *);
void __devicelib_default_work_group_joint_sort_descending_p1u8_u32_p1i8(
    __global uchar *, uint, __local char *);
// short
void __devicelib_default_work_group_joint_sort_ascending_p1i16_u32_p3i8(
    __global short *, uint, __local char *);
void __devicelib_default_work_group_joint_sort_descending_p1u16_u32_p3i8(
    __global ushort *, uint, __local char *);
// int
void __devicelib_default_work_group_joint_sort_ascending_p1i32_u32_p1i8(
    __global int *, uint, __local char *);
void __devicelib_default_work_group_joint_sort_descending_p1u32_u32_p1i8(
    __global uint *, uint, __local char *);
// long
void __devicelib_default_work_group_joint_sort_ascending_p1i64_u32_p1i8(
    __global long *, uint, __local char *);
void __devicelib_default_work_group_joint_sort_descending_p1u64_u32_p1i8(
    __global ulong *, uint, __local char *);
// float
void __devicelib_default_work_group_joint_sort_ascending_p1f32_u32_p1i8(
    __global float *, uint, __local char *);
// double
void __devicelib_default_work_group_joint_sort_descending_p1f64_u32_p3i8(
    __global double *, uint, __local char *);
// half
void __devicelib_default_work_group_joint_sort_ascending_p1f16_u32_p3i8(
    __global half *, uint, __local char *);

__attribute__((intel_reqd_sub_group_size(1))) __kernel void
WG_joint_sort(__global char *data1, __global uchar *data2,
              __global short *data3, __global ushort *data4,
              __global int *data5, __global uint *data6, __global long *data7,
              __global ulong *data8, __global float *data9,
              __global double *data10, __global half *data11) {
  __local char scratch1[1024];
  __local char scratch2[1024];
  __local char scratch3[1024];
  __local char scratch4[1024];
  __local char scratch5[1024];
  __local char scratch6[1024];
  __local char scratch7[1024];
  __local char scratch8[1024];
  __local char scratch9[1024];
  __local char scratch10[1024];
  __local char scratch11[1024];

  int lidx = get_local_linear_id();
  int local_size = get_local_size(0);
  int data_size = local_size * pre_wg_item_data_size;

  // char
  __devicelib_default_work_group_joint_sort_ascending_p1i8_u32_p1i8(
      data1, data_size, scratch1);

  // uchar
  __devicelib_default_work_group_joint_sort_descending_p1u8_u32_p1i8(
      data2, data_size, scratch2);

  // short
  __devicelib_default_work_group_joint_sort_ascending_p1i16_u32_p3i8(
      data3, data_size, scratch3);

  // ushort
  __devicelib_default_work_group_joint_sort_descending_p1u16_u32_p3i8(
      data4, data_size, scratch4);

  // int
  __devicelib_default_work_group_joint_sort_ascending_p1i32_u32_p1i8(
      data5, data_size, scratch5);

  // uint
  __devicelib_default_work_group_joint_sort_descending_p1u32_u32_p1i8(
      data6, data_size, scratch6);

  // long
  __devicelib_default_work_group_joint_sort_ascending_p1i64_u32_p1i8(
      data7, data_size, scratch7);

  // ulong
  __devicelib_default_work_group_joint_sort_descending_p1u64_u32_p1i8(
      data8, data_size, scratch8);

  // float
  __devicelib_default_work_group_joint_sort_ascending_p1f32_u32_p1i8(
      data9, data_size, scratch9);

  // double
  __devicelib_default_work_group_joint_sort_descending_p1f64_u32_p3i8(
      data10, data_size, scratch10);

  // half
  __devicelib_default_work_group_joint_sort_ascending_p1f16_u32_p3i8(
      data11, data_size, scratch5);
}

__attribute__((intel_reqd_sub_group_size(4))) __kernel void
WG_private_close_sort_v4(__global char *data1, __global uchar *data2,
                         __global short *data3, __global ushort *data4,
                         __global int *data5, __global uint *data6,
                         __global long *data7, __global ulong *data8,
                         __global float *data9, __global double *data10,
                         __global half *data11) {
  __local char scratch1[1024];
  __local char scratch2[1024];
  __local char scratch3[1024];
  __local char scratch4[1024];
  __local char scratch5[1024];
  __local char scratch6[1024];
  __local char scratch7[1024];
  __local char scratch8[1024];
  __local char scratch9[1024];
  __local char scratch10[1024];
  __local char scratch11[1024];

  int lidx = get_local_linear_id();
  int data_offset = lidx * pre_wg_item_data_size;

  // char
  __devicelib_default_work_group_private_sort_close_ascending_p1i8_u32_p1i8(
      data1 + data_offset, pre_wg_item_data_size, scratch1);

  // uchar
  __devicelib_default_work_group_private_sort_close_descending_p1u8_u32_p1i8(
      data2 + data_offset, pre_wg_item_data_size, scratch2);

  // short
  __devicelib_default_work_group_private_sort_close_ascending_p1i16_u32_p1i8(
      data3 + data_offset, pre_wg_item_data_size, scratch3);

  // ushort
  __devicelib_default_work_group_private_sort_close_descending_p1u16_u32_p1i8(
      data4 + data_offset, pre_wg_item_data_size, scratch4);

  // int
  __devicelib_default_work_group_private_sort_close_ascending_p1i32_u32_p3i8(
      data5 + data_offset, pre_wg_item_data_size, scratch5);

  // uint
  __devicelib_default_work_group_private_sort_close_descending_p1u32_u32_p3i8(
      data6 + data_offset, pre_wg_item_data_size, scratch6);

  // long
  __devicelib_default_work_group_private_sort_close_ascending_p1i64_u32_p1i8(
      data7 + data_offset, pre_wg_item_data_size, scratch7);

  // ulong
  __devicelib_default_work_group_private_sort_close_descending_p1u64_u32_p1i8(
      data8 + data_offset, pre_wg_item_data_size, scratch8);

  // float
  __devicelib_default_work_group_private_sort_close_ascending_p1f32_u32_p1i8(
      data9 + data_offset, pre_wg_item_data_size, scratch9);

  // double
  __devicelib_default_work_group_private_sort_close_descending_p1f64_u32_p3i8(
      data10 + data_offset, pre_wg_item_data_size, scratch10);

  // half
  __devicelib_default_work_group_private_sort_close_ascending_p1f16_u32_p3i8(
      data11 + data_offset, pre_wg_item_data_size, scratch11);
}

__attribute__((intel_reqd_sub_group_size(8))) __kernel void
WG_private_spread_sort_v8(__global char *data1, __global uchar *data2,
                          __global short *data3, __global ushort *data4,
                          __global int *data5, __global uint *data6,
                          __global long *data7, __global ulong *data8,
                          __global float *data9, __global double *data10,
                          __global half *data11) {
  __local char scratch1[1024];
  __local char scratch2[1024];
  __local char scratch3[1024];
  __local char scratch4[1024];
  __local char scratch5[1024];
  __local char scratch6[1024];
  __local char scratch7[1024];
  __local char scratch8[1024];
  __local char scratch9[1024];
  __local char scratch10[1024];
  __local char scratch11[1024];

  int lidx = get_local_linear_id();
  int data_offset = lidx * pre_wg_item_data_size;

  // char
  __devicelib_default_work_group_private_sort_spread_ascending_p1i8_u32_p3i8(
      data1 + data_offset, pre_wg_item_data_size, scratch1);

  // uchar
  __devicelib_default_work_group_private_sort_spread_descending_p1u8_u32_p3i8(
      data2 + data_offset, pre_wg_item_data_size, scratch2);

  // short
  __devicelib_default_work_group_private_sort_spread_ascending_p1i16_u32_p1i8(
      data3 + data_offset, pre_wg_item_data_size, scratch3);

  // ushort
  __devicelib_default_work_group_private_sort_spread_descending_p1u16_u32_p1i8(
      data4 + data_offset, pre_wg_item_data_size, scratch4);

  // int
  __devicelib_default_work_group_private_sort_spread_ascending_p1i32_u32_p1i8(
      data5 + data_offset, pre_wg_item_data_size, scratch5);

  // uint
  __devicelib_default_work_group_private_sort_spread_descending_p1u32_u32_p1i8(
      data6 + data_offset, pre_wg_item_data_size, scratch6);

  // long
  __devicelib_default_work_group_private_sort_spread_ascending_p1i64_u32_p3i8(
      data7 + data_offset, pre_wg_item_data_size, scratch7);

  // ulong
  __devicelib_default_work_group_private_sort_spread_descending_p1u64_u32_p3i8(
      data8 + data_offset, pre_wg_item_data_size, scratch8);

  // float
  __devicelib_default_work_group_private_sort_spread_ascending_p1f32_u32_p1i8(
      data9 + data_offset, pre_wg_item_data_size, scratch9);

  // double
  __devicelib_default_work_group_private_sort_spread_descending_p1f64_u32_p1i8(
      data10 + data_offset, pre_wg_item_data_size, scratch10);

  // half
  __devicelib_default_work_group_private_sort_spread_ascending_p1f16_u32_p1i8(
      data11 + data_offset, pre_wg_item_data_size, scratch11);
}

__attribute__((intel_reqd_sub_group_size(16))) __kernel void WG_joint_sort_v16(
    __global char *data1, __global uchar *data2, __global short *data3,
    __global ushort *data4, __global int *data5, __global uint *data6,
    __global long *data7, __global ulong *data8, __global float *data9,
    __global double *data10, __global half *data11) {
  __local char scratch1[1024];
  __local char scratch2[1024];
  __local char scratch3[1024];
  __local char scratch4[1024];
  __local char scratch5[1024];
  __local char scratch6[1024];
  __local char scratch7[1024];
  __local char scratch8[1024];
  __local char scratch9[1024];
  __local char scratch10[1024];
  __local char scratch11[1024];

  int lidx = get_local_linear_id();
  int local_size = get_local_size(0);
  int data_size = local_size * pre_wg_item_data_size;

  // char
  __devicelib_default_work_group_joint_sort_ascending_p1i8_u32_p1i8(
      data1, data_size, scratch1);

  // uchar
  __devicelib_default_work_group_joint_sort_descending_p1u8_u32_p1i8(
      data2, data_size, scratch2);

  // short
  __devicelib_default_work_group_joint_sort_ascending_p1i16_u32_p3i8(
      data3, data_size, scratch3);

  // ushort
  __devicelib_default_work_group_joint_sort_descending_p1u16_u32_p3i8(
      data4, data_size, scratch4);

  // int
  __devicelib_default_work_group_joint_sort_ascending_p1i32_u32_p1i8(
      data5, data_size, scratch5);

  // uint
  __devicelib_default_work_group_joint_sort_descending_p1u32_u32_p1i8(
      data6, data_size, scratch6);

  // long
  __devicelib_default_work_group_joint_sort_ascending_p1i64_u32_p1i8(
      data7, data_size, scratch7);

  // ulong
  __devicelib_default_work_group_joint_sort_descending_p1u64_u32_p1i8(
      data8, data_size, scratch8);

  // float
  __devicelib_default_work_group_joint_sort_ascending_p1f32_u32_p1i8(
      data9, data_size, scratch9);

  // double
  __devicelib_default_work_group_joint_sort_descending_p1f64_u32_p3i8(
      data10, data_size, scratch10);

  // half
  __devicelib_default_work_group_joint_sort_ascending_p1f16_u32_p3i8(
      data11, data_size, scratch11);
}
