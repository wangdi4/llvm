// Check results for scalar version work group sort builtin
#pragma OPENCL EXTENSION cl_khr_fp16 : enable

// double - char
void __devicelib_default_work_group_private_sort_close_ascending_p3f64_p3i8_u32_p1i8(
    __local double *, __local char *, uint, __local char *);
// half - uchar
void __devicelib_default_work_group_private_sort_close_descending_p3f16_p3u8_u32_p1i8(
    __local half *, __local uchar *, uint, __local char *);
// ulong - short
void __devicelib_default_work_group_private_sort_close_ascending_p3u64_p3i16_u32_p1i8(
    __local ulong *, __local short *, uint, __local char *);
// float - ushort
void __devicelib_default_work_group_private_sort_close_descending_p3f32_p3u16_u32_p1i8(
    __local float *, __local ushort *, uint, __local char *);
// uint - int
void __devicelib_default_work_group_private_sort_close_ascending_p3u32_p3i32_u32_p1i8(
    __local uint *, __local int *, uint, __local char *);
// long - uint
void __devicelib_default_work_group_private_sort_close_descending_p3i64_p3u32_u32_p1i8(
    __local long *, __local uint *, uint, __local char *);
// ushort - long
void __devicelib_default_work_group_private_sort_close_ascending_p3u16_p3i64_u32_p1i8(
    __local ushort *, __local long *, uint, __local char *);
// int - ulong
void __devicelib_default_work_group_private_sort_close_descending_p3i32_p3u64_u32_p1i8(
    __local int *, __local ulong *, uint, __local char *);
// uchar - float
void __devicelib_default_work_group_private_sort_close_ascending_p3u8_p3f32_u32_p3i8(
    __local uchar *, __local float *, uint, __local char *);
// char - double
void __devicelib_default_work_group_private_sort_close_descending_p3i8_p3f64_u32_p3i8(
    __local char *, __local double *, uint, __local char *);
// short - half
void __devicelib_default_work_group_private_sort_close_ascending_p3i16_p3f16_u32_p3i8(
    __local short *, __local half *, uint, __local char *);

const uint pre_wg_item_data_size = 4;

__attribute__((intel_reqd_sub_group_size(1))) __kernel void
WG_private_close_sort(__global char *data1, __global uchar *data2,
                      __global short *data3, __global ushort *data4,
                      __global int *data5, __global uint *data6,
                      __global long *data7, __global ulong *data8,
                      __global float *data9, __global double *data10,
                      __global half *data11) {
  __local char scratch1[2048];
  __local char scratch2[2048];
  __local char scratch3[2048];
  __local char scratch4[2048];
  __local char scratch5[2048];
  __local char scratch6[2048];
  __local char scratch7[2048];
  __local char scratch8[2048];
  __local char scratch9[2048];
  __local char scratch10[2048];
  __local char scratch11[2048];

  __local double key1[64];
  __local half key2[64];
  __local ulong key3[64];
  __local float key4[64];
  __local uint key5[64];
  __local long key6[64];
  __local ushort key7[64];
  __local int key8[64];
  __local uchar key9[64];
  __local char key10[64];
  __local short key11[64];

  __local char value1[64];
  __local uchar value2[64];
  __local short value3[64];
  __local ushort value4[64];
  __local int value5[64];
  __local uint value6[64];
  __local long value7[64];
  __local ulong value8[64];
  __local float value9[64];
  __local double value10[64];
  __local half value11[64];

  int data_size = 64;

  // 1,3...11 keys fill decrease data, and will run ascending sort
  //          key and value wiil be increasing
  // 2,4...10 keys fill increase data, and will run descending sort
  //          key and value wiil be decreasing
  for (uint i = 0; i < data_size; i++) {
    key1[i] = data_size - i;
    key2[i] = i;
    key3[i] = data_size - i;
    key4[i] = i;
    key5[i] = data_size - i;
    key6[i] = i;
    key7[i] = data_size - i;
    key8[i] = i;
    key9[i] = data_size - i;
    key10[i] = i;
    key11[i] = data_size - i;

    value1[i] = data1[i];
    value2[i] = data2[i];
    value3[i] = data3[i];
    value4[i] = data4[i];
    value5[i] = data5[i];
    value6[i] = data6[i];
    value7[i] = data7[i];
    value8[i] = data8[i];
    value9[i] = data9[i];
    value10[i] = data10[i];
    value11[i] = data11[i];
  }
  work_group_barrier(CLK_LOCAL_MEM_FENCE);

  int lidx = get_local_linear_id();
  int data_offset = lidx * pre_wg_item_data_size;

  // double - char
  __devicelib_default_work_group_private_sort_close_ascending_p3f64_p3i8_u32_p1i8(
      &key1[0] + data_offset, &value1[0] + data_offset, pre_wg_item_data_size,
      scratch1);

  // half - uchar
  __devicelib_default_work_group_private_sort_close_descending_p3f16_p3u8_u32_p1i8(
      &key2[0] + data_offset, &value2[0] + data_offset, pre_wg_item_data_size,
      scratch2);

  // ulong - short
  __devicelib_default_work_group_private_sort_close_ascending_p3u64_p3i16_u32_p1i8(
      &key3[0] + data_offset, &value3[0] + data_offset, pre_wg_item_data_size,
      scratch3);

  // float - ushort
  __devicelib_default_work_group_private_sort_close_descending_p3f32_p3u16_u32_p1i8(
      &key4[0] + data_offset, &value4[0] + data_offset, pre_wg_item_data_size,
      scratch4);

  // uint - int
  __devicelib_default_work_group_private_sort_close_ascending_p3u32_p3i32_u32_p1i8(
      &key5[0] + data_offset, &value5[0] + data_offset, pre_wg_item_data_size,
      scratch5);

  // long - uint
  __devicelib_default_work_group_private_sort_close_descending_p3i64_p3u32_u32_p1i8(
      &key6[0] + data_offset, &value6[0] + data_offset, pre_wg_item_data_size,
      scratch6);

  // ushort - long
  __devicelib_default_work_group_private_sort_close_ascending_p3u16_p3i64_u32_p1i8(
      &key7[0] + data_offset, &value7[0] + data_offset, pre_wg_item_data_size,
      scratch7);

  // int - ulong
  __devicelib_default_work_group_private_sort_close_descending_p3i32_p3u64_u32_p1i8(
      &key8[0] + data_offset, &value8[0] + data_offset, pre_wg_item_data_size,
      scratch8);

  // uchar - float
  __devicelib_default_work_group_private_sort_close_ascending_p3u8_p3f32_u32_p3i8(
      &key9[0] + data_offset, &value9[0] + data_offset, pre_wg_item_data_size,
      scratch9);

  // char - double
  __devicelib_default_work_group_private_sort_close_descending_p3i8_p3f64_u32_p3i8(
      &key10[0] + data_offset, &value10[0] + data_offset, pre_wg_item_data_size,
      scratch10);

  // short - half
  __devicelib_default_work_group_private_sort_close_ascending_p3i16_p3f16_u32_p3i8(
      &key11[0] + data_offset, &value11[0] + data_offset, pre_wg_item_data_size,
      scratch11);

  work_group_barrier(CLK_GLOBAL_MEM_FENCE);

  for (uint i = 0; i < data_size; i++) {
    data1[i] = value1[i];
    data2[i] = value2[i];
    data3[i] = value3[i];
    data4[i] = value4[i];
    data5[i] = value5[i];
    data6[i] = value6[i];
    data7[i] = value7[i];
    data8[i] = value8[i];
    data9[i] = value9[i];
    data10[i] = value10[i];
    data11[i] = value11[i];
  }
}

// double - char
void __devicelib_default_work_group_private_sort_spread_ascending_p3f64_p3i8_u32_p1i8(
    __local double *, __local char *, uint, __local char *);
// half - uchar
void __devicelib_default_work_group_private_sort_spread_descending_p3f16_p3u8_u32_p1i8(
    __local half *, __local uchar *, uint, __local char *);
// ulong - short
void __devicelib_default_work_group_private_sort_spread_ascending_p3u64_p3i16_u32_p1i8(
    __local ulong *, __local short *, uint, __local char *);
// float - ushort
void __devicelib_default_work_group_private_sort_spread_descending_p3f32_p3u16_u32_p1i8(
    __local float *, __local ushort *, uint, __local char *);
// uint - int
void __devicelib_default_work_group_private_sort_spread_ascending_p3u32_p3i32_u32_p1i8(
    __local uint *, __local int *, uint, __local char *);
// long - uint
void __devicelib_default_work_group_private_sort_spread_descending_p3i64_p3u32_u32_p3i8(
    __local long *, __local uint *, uint, __local char *);
// ushort - long
void __devicelib_default_work_group_private_sort_spread_ascending_p3u16_p3i64_u32_p3i8(
    __local ushort *, __local long *, uint, __local char *);
// int - ulong
void __devicelib_default_work_group_private_sort_spread_descending_p3i32_p3u64_u32_p3i8(
    __local int *, __local ulong *, uint, __local char *);
// uchar - float
void __devicelib_default_work_group_private_sort_spread_ascending_p3u8_p3f32_u32_p1i8(
    __local uchar *, __local float *, uint, __local char *);
// char - double
void __devicelib_default_work_group_private_sort_spread_descending_p3i8_p3f64_u32_p1i8(
    __local char *, __local double *, uint, __local char *);
// short - half
void __devicelib_default_work_group_private_sort_spread_ascending_p3i16_p3f16_u32_p1i8(
    __local short *, __local half *, uint, __local char *);

__attribute__((intel_reqd_sub_group_size(1))) __kernel void
WG_private_spread_sort(__global char *data1, __global uchar *data2,
                       __global short *data3, __global ushort *data4,
                       __global int *data5, __global uint *data6,
                       __global long *data7, __global ulong *data8,
                       __global float *data9, __global double *data10,
                       __global half *data11) {
  __local char scratch1[2048];
  __local char scratch2[2048];
  __local char scratch3[2048];
  __local char scratch4[2048];
  __local char scratch5[2048];
  __local char scratch6[2048];
  __local char scratch7[2048];
  __local char scratch8[2048];
  __local char scratch9[2048];
  __local char scratch10[2048];
  __local char scratch11[2048];

  __local double key1[64];
  __local half key2[64];
  __local ulong key3[64];
  __local float key4[64];
  __local uint key5[64];
  __local long key6[64];
  __local ushort key7[64];
  __local int key8[64];
  __local uchar key9[64];
  __local char key10[64];
  __local short key11[64];

  __local char value1[64];
  __local uchar value2[64];
  __local short value3[64];
  __local ushort value4[64];
  __local int value5[64];
  __local uint value6[64];
  __local long value7[64];
  __local ulong value8[64];
  __local float value9[64];
  __local double value10[64];
  __local half value11[64];

  int data_size = 64;

  // 1,3...11 keys fill decrease data, and will run ascending sort
  //          key and value wiil be increasing
  // 2,4...10 keys fill increase data, and will run descending sort
  //          key and value wiil be decreasing
  for (uint i = 0; i < data_size; i++) {
    key1[i] = data_size - i;
    key2[i] = i;
    key3[i] = data_size - i;
    key4[i] = i;
    key5[i] = data_size - i;
    key6[i] = i;
    key7[i] = data_size - i;
    key8[i] = i;
    key9[i] = data_size - i;
    key10[i] = i;
    key11[i] = data_size - i;

    value1[i] = data1[i];
    value2[i] = data2[i];
    value3[i] = data3[i];
    value4[i] = data4[i];
    value5[i] = data5[i];
    value6[i] = data6[i];
    value7[i] = data7[i];
    value8[i] = data8[i];
    value9[i] = data9[i];
    value10[i] = data10[i];
    value11[i] = data11[i];
  }
  work_group_barrier(CLK_LOCAL_MEM_FENCE);

  int lidx = get_local_linear_id();
  int data_offset = lidx * pre_wg_item_data_size;

  // double - char
  __devicelib_default_work_group_private_sort_spread_ascending_p3f64_p3i8_u32_p1i8(
      &key1[0] + data_offset, &value1[0] + data_offset, pre_wg_item_data_size,
      scratch1);

  // half - uchar
  __devicelib_default_work_group_private_sort_spread_descending_p3f16_p3u8_u32_p1i8(
      &key2[0] + data_offset, &value2[0] + data_offset, pre_wg_item_data_size,
      scratch2);

  // ulong - short
  __devicelib_default_work_group_private_sort_spread_ascending_p3u64_p3i16_u32_p1i8(
      &key3[0] + data_offset, &value3[0] + data_offset, pre_wg_item_data_size,
      scratch3);

  // float - ushort
  __devicelib_default_work_group_private_sort_spread_descending_p3f32_p3u16_u32_p1i8(
      &key4[0] + data_offset, &value4[0] + data_offset, pre_wg_item_data_size,
      scratch4);

  // uint - int
  __devicelib_default_work_group_private_sort_spread_ascending_p3u32_p3i32_u32_p1i8(
      &key5[0] + data_offset, &value5[0] + data_offset, pre_wg_item_data_size,
      scratch5);

  // long - uint
  __devicelib_default_work_group_private_sort_spread_descending_p3i64_p3u32_u32_p3i8(
      &key6[0] + data_offset, &value6[0] + data_offset, pre_wg_item_data_size,
      scratch6);

  // ushort - long
  __devicelib_default_work_group_private_sort_spread_ascending_p3u16_p3i64_u32_p3i8(
      &key7[0] + data_offset, &value7[0] + data_offset, pre_wg_item_data_size,
      scratch7);

  // int - ulong
  __devicelib_default_work_group_private_sort_spread_descending_p3i32_p3u64_u32_p3i8(
      &key8[0] + data_offset, &value8[0] + data_offset, pre_wg_item_data_size,
      scratch8);

  // uchar - float
  __devicelib_default_work_group_private_sort_spread_ascending_p3u8_p3f32_u32_p1i8(
      &key9[0] + data_offset, &value9[0] + data_offset, pre_wg_item_data_size,
      scratch9);

  // char - double
  __devicelib_default_work_group_private_sort_spread_descending_p3i8_p3f64_u32_p1i8(
      &key10[0] + data_offset, &value10[0] + data_offset, pre_wg_item_data_size,
      scratch10);

  // short - half
  __devicelib_default_work_group_private_sort_spread_ascending_p3i16_p3f16_u32_p1i8(
      &key11[0] + data_offset, &value11[0] + data_offset, pre_wg_item_data_size,
      scratch11);

  work_group_barrier(CLK_GLOBAL_MEM_FENCE);

  for (uint i = 0; i < data_size; i++) {
    data1[i] = value1[i];
    data2[i] = value2[i];
    data3[i] = value3[i];
    data4[i] = value4[i];
    data5[i] = value5[i];
    data6[i] = value6[i];
    data7[i] = value7[i];
    data8[i] = value8[i];
    data9[i] = value9[i];
    data10[i] = value10[i];
    data11[i] = value11[i];
  }
}

// double - char
void __devicelib_default_work_group_joint_sort_ascending_p3f64_p3i8_u32_p1i8(
    __local double *, __local char *, uint, __local char *);
// half - uchar
void __devicelib_default_work_group_joint_sort_descending_p3f16_p3u8_u32_p1i8(
    __local half *, __local uchar *, uint, __local char *);
// ulong - short
void __devicelib_default_work_group_joint_sort_ascending_p3u64_p3i16_u32_p3i8(
    __local ulong *, __local short *, uint, __local char *);
// float - ushort
void __devicelib_default_work_group_joint_sort_descending_p3f32_p3u16_u32_p3i8(
    __local float *, __local ushort *, uint, __local char *);
// uint - int
void __devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p3i8(
    __local uint *, __local int *, uint, __local char *);
// long - uint
void __devicelib_default_work_group_joint_sort_descending_p3i64_p3u32_u32_p1i8(
    __local long *, __local uint *, uint, __local char *);
// ushort - long
void __devicelib_default_work_group_joint_sort_ascending_p3u16_p3i64_u32_p1i8(
    __local ushort *, __local long *, uint, __local char *);
// int - ulong
void __devicelib_default_work_group_joint_sort_descending_p3i32_p3u64_u32_p1i8(
    __local int *, __local ulong *, uint, __local char *);
// uchar - float
void __devicelib_default_work_group_joint_sort_ascending_p3u8_p3f32_u32_p1i8(
    __local uchar *, __local float *, uint, __local char *);
// char - double
void __devicelib_default_work_group_joint_sort_descending_p3i8_p3f64_u32_p1i8(
    __local char *, __local double *, uint, __local char *);
// short - half
void __devicelib_default_work_group_joint_sort_ascending_p3i16_p3f16_u32_p1i8(
    __local short *, __local half *, uint, __local char *);

__attribute__((intel_reqd_sub_group_size(1))) __kernel void
WG_joint_sort(__global char *data1, __global uchar *data2,
              __global short *data3, __global ushort *data4,
              __global int *data5, __global uint *data6, __global long *data7,
              __global ulong *data8, __global float *data9,
              __global double *data10, __global half *data11) {
  __local char scratch1[2048];
  __local char scratch2[2048];
  __local char scratch3[2048];
  __local char scratch4[2048];
  __local char scratch5[2048];
  __local char scratch6[2048];
  __local char scratch7[2048];
  __local char scratch8[2048];
  __local char scratch9[2048];
  __local char scratch10[2048];
  __local char scratch11[2048];

  __local double key1[64];
  __local half key2[64];
  __local ulong key3[64];
  __local float key4[64];
  __local uint key5[64];
  __local long key6[64];
  __local ushort key7[64];
  __local int key8[64];
  __local uchar key9[64];
  __local char key10[64];
  __local short key11[64];

  __local char value1[64];
  __local uchar value2[64];
  __local short value3[64];
  __local ushort value4[64];
  __local int value5[64];
  __local uint value6[64];
  __local long value7[64];
  __local ulong value8[64];
  __local float value9[64];
  __local double value10[64];
  __local half value11[64];

  int data_size = 64;

  // 1,3...11 keys fill decrease data, and will run ascending sort
  //          key and value wiil be increasing
  // 2,4...10 keys fill increase data, and will run descending sort
  //          key and value wiil be decreasing
  for (uint i = 0; i < data_size; i++) {
    key1[i] = data_size - i;
    key2[i] = i;
    key3[i] = data_size - i;
    key4[i] = i;
    key5[i] = data_size - i;
    key6[i] = i;
    key7[i] = data_size - i;
    key8[i] = i;
    key9[i] = data_size - i;
    key10[i] = i;
    key11[i] = data_size - i;

    value1[i] = data1[i];
    value2[i] = data2[i];
    value3[i] = data3[i];
    value4[i] = data4[i];
    value5[i] = data5[i];
    value6[i] = data6[i];
    value7[i] = data7[i];
    value8[i] = data8[i];
    value9[i] = data9[i];
    value10[i] = data10[i];
    value11[i] = data11[i];
  }
  work_group_barrier(CLK_LOCAL_MEM_FENCE);

  int lidx = get_local_linear_id();

  // double - char
  __devicelib_default_work_group_joint_sort_ascending_p3f64_p3i8_u32_p1i8(
      &key1[0], &value1[0], data_size, scratch1);

  // half - uchar
  __devicelib_default_work_group_joint_sort_descending_p3f16_p3u8_u32_p1i8(
      &key2[0], &value2[0], data_size, scratch2);

  // ulong - short
  __devicelib_default_work_group_joint_sort_ascending_p3u64_p3i16_u32_p3i8(
      &key3[0], &value3[0], data_size, scratch3);

  // float - ushort
  __devicelib_default_work_group_joint_sort_descending_p3f32_p3u16_u32_p3i8(
      &key4[0], &value4[0], data_size, scratch4);

  // uint - int
  __devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p3i8(
      &key5[0], &value5[0], data_size, scratch5);

  // long - uint
  __devicelib_default_work_group_joint_sort_descending_p3i64_p3u32_u32_p1i8(
      &key6[0], &value6[0], data_size, scratch6);

  // ushort - long
  __devicelib_default_work_group_joint_sort_ascending_p3u16_p3i64_u32_p1i8(
      &key7[0], &value7[0], data_size, scratch7);

  // int - ulong
  __devicelib_default_work_group_joint_sort_descending_p3i32_p3u64_u32_p1i8(
      &key8[0], &value8[0], data_size, scratch8);

  // uchar - float
  __devicelib_default_work_group_joint_sort_ascending_p3u8_p3f32_u32_p1i8(
      &key9[0], &value9[0], data_size, scratch9);

  // char - double
  __devicelib_default_work_group_joint_sort_descending_p3i8_p3f64_u32_p1i8(
      &key10[0], &value10[0], data_size, scratch10);

  // short - half
  __devicelib_default_work_group_joint_sort_ascending_p3i16_p3f16_u32_p1i8(
      &key11[0], &value11[0], data_size, scratch11);

  work_group_barrier(CLK_GLOBAL_MEM_FENCE);

  for (uint i = 0; i < data_size; i++) {
    data1[i] = value1[i];
    data2[i] = value2[i];
    data3[i] = value3[i];
    data4[i] = value4[i];
    data5[i] = value5[i];
    data6[i] = value6[i];
    data7[i] = value7[i];
    data8[i] = value8[i];
    data9[i] = value9[i];
    data10[i] = value10[i];
    data11[i] = value11[i];
  }
}

__attribute__((intel_reqd_sub_group_size(4))) __kernel void
WG_private_close_sort_v4(__global char *data1, __global uchar *data2,
                         __global short *data3, __global ushort *data4,
                         __global int *data5, __global uint *data6,
                         __global long *data7, __global ulong *data8,
                         __global float *data9, __global double *data10,
                         __global half *data11) {
  __local char scratch1[2048];
  __local char scratch2[2048];
  __local char scratch3[2048];
  __local char scratch4[2048];
  __local char scratch5[2048];
  __local char scratch6[2048];
  __local char scratch7[2048];
  __local char scratch8[2048];
  __local char scratch9[2048];
  __local char scratch10[2048];
  __local char scratch11[2048];

  __local double key1[64];
  __local half key2[64];
  __local ulong key3[64];
  __local float key4[64];
  __local uint key5[64];
  __local long key6[64];
  __local ushort key7[64];
  __local int key8[64];
  __local uchar key9[64];
  __local char key10[64];
  __local short key11[64];

  __local char value1[64];
  __local uchar value2[64];
  __local short value3[64];
  __local ushort value4[64];
  __local int value5[64];
  __local uint value6[64];
  __local long value7[64];
  __local ulong value8[64];
  __local float value9[64];
  __local double value10[64];
  __local half value11[64];

  int data_size = 64;

  // 1,3...11 keys fill decrease data, and will run ascending sort
  //          key and value wiil be increasing
  // 2,4...10 keys fill increase data, and will run descending sort
  //          key and value wiil be decreasing
  for (uint i = 0; i < data_size; i++) {
    key1[i] = data_size - i;
    key2[i] = i;
    key3[i] = data_size - i;
    key4[i] = i;
    key5[i] = data_size - i;
    key6[i] = i;
    key7[i] = data_size - i;
    key8[i] = i;
    key9[i] = data_size - i;
    key10[i] = i;
    key11[i] = data_size - i;

    value1[i] = data1[i];
    value2[i] = data2[i];
    value3[i] = data3[i];
    value4[i] = data4[i];
    value5[i] = data5[i];
    value6[i] = data6[i];
    value7[i] = data7[i];
    value8[i] = data8[i];
    value9[i] = data9[i];
    value10[i] = data10[i];
    value11[i] = data11[i];
  }
  work_group_barrier(CLK_LOCAL_MEM_FENCE);

  int lidx = get_local_linear_id();
  int data_offset = lidx * pre_wg_item_data_size;

  // double - char
  __devicelib_default_work_group_private_sort_close_ascending_p3f64_p3i8_u32_p1i8(
      &key1[0] + data_offset, &value1[0] + data_offset, pre_wg_item_data_size,
      scratch1);

  // half - uchar
  __devicelib_default_work_group_private_sort_close_descending_p3f16_p3u8_u32_p1i8(
      &key2[0] + data_offset, &value2[0] + data_offset, pre_wg_item_data_size,
      scratch2);

  // ulong - short
  __devicelib_default_work_group_private_sort_close_ascending_p3u64_p3i16_u32_p1i8(
      &key3[0] + data_offset, &value3[0] + data_offset, pre_wg_item_data_size,
      scratch3);

  // float - ushort
  __devicelib_default_work_group_private_sort_close_descending_p3f32_p3u16_u32_p1i8(
      &key4[0] + data_offset, &value4[0] + data_offset, pre_wg_item_data_size,
      scratch4);

  // uint - int
  __devicelib_default_work_group_private_sort_close_ascending_p3u32_p3i32_u32_p1i8(
      &key5[0] + data_offset, &value5[0] + data_offset, pre_wg_item_data_size,
      scratch5);

  // long - uint
  __devicelib_default_work_group_private_sort_close_descending_p3i64_p3u32_u32_p1i8(
      &key6[0] + data_offset, &value6[0] + data_offset, pre_wg_item_data_size,
      scratch6);

  // ushort - long
  __devicelib_default_work_group_private_sort_close_ascending_p3u16_p3i64_u32_p1i8(
      &key7[0] + data_offset, &value7[0] + data_offset, pre_wg_item_data_size,
      scratch7);

  // int - ulong
  __devicelib_default_work_group_private_sort_close_descending_p3i32_p3u64_u32_p1i8(
      &key8[0] + data_offset, &value8[0] + data_offset, pre_wg_item_data_size,
      scratch8);

  // uchar - float
  __devicelib_default_work_group_private_sort_close_ascending_p3u8_p3f32_u32_p3i8(
      &key9[0] + data_offset, &value9[0] + data_offset, pre_wg_item_data_size,
      scratch9);

  // char - double
  __devicelib_default_work_group_private_sort_close_descending_p3i8_p3f64_u32_p3i8(
      &key10[0] + data_offset, &value10[0] + data_offset, pre_wg_item_data_size,
      scratch10);

  // short - half
  __devicelib_default_work_group_private_sort_close_ascending_p3i16_p3f16_u32_p3i8(
      &key11[0] + data_offset, &value11[0] + data_offset, pre_wg_item_data_size,
      scratch11);

  work_group_barrier(CLK_GLOBAL_MEM_FENCE);

  for (uint i = 0; i < data_size; i++) {
    data1[i] = value1[i];
    data2[i] = value2[i];
    data3[i] = value3[i];
    data4[i] = value4[i];
    data5[i] = value5[i];
    data6[i] = value6[i];
    data7[i] = value7[i];
    data8[i] = value8[i];
    data9[i] = value9[i];
    data10[i] = value10[i];
    data11[i] = value11[i];
  }
}

__attribute__((intel_reqd_sub_group_size(8))) __kernel void
WG_private_spread_sort_v8(__global char *data1, __global uchar *data2,
                          __global short *data3, __global ushort *data4,
                          __global int *data5, __global uint *data6,
                          __global long *data7, __global ulong *data8,
                          __global float *data9, __global double *data10,
                          __global half *data11) {
  __local char scratch1[2048];
  __local char scratch2[2048];
  __local char scratch3[2048];
  __local char scratch4[2048];
  __local char scratch5[2048];
  __local char scratch6[2048];
  __local char scratch7[2048];
  __local char scratch8[2048];
  __local char scratch9[2048];
  __local char scratch10[2048];
  __local char scratch11[2048];

  __local double key1[64];
  __local half key2[64];
  __local ulong key3[64];
  __local float key4[64];
  __local uint key5[64];
  __local long key6[64];
  __local ushort key7[64];
  __local int key8[64];
  __local uchar key9[64];
  __local char key10[64];
  __local short key11[64];

  __local char value1[64];
  __local uchar value2[64];
  __local short value3[64];
  __local ushort value4[64];
  __local int value5[64];
  __local uint value6[64];
  __local long value7[64];
  __local ulong value8[64];
  __local float value9[64];
  __local double value10[64];
  __local half value11[64];

  int data_size = 64;

  // 1,3...11 keys fill decrease data, and will run ascending sort
  //          key and value wiil be increasing
  // 2,4...10 keys fill increase data, and will run descending sort
  //          key and value wiil be decreasing
  for (uint i = 0; i < data_size; i++) {
    key1[i] = data_size - i;
    key2[i] = i;
    key3[i] = data_size - i;
    key4[i] = i;
    key5[i] = data_size - i;
    key6[i] = i;
    key7[i] = data_size - i;
    key8[i] = i;
    key9[i] = data_size - i;
    key10[i] = i;
    key11[i] = data_size - i;

    value1[i] = data1[i];
    value2[i] = data2[i];
    value3[i] = data3[i];
    value4[i] = data4[i];
    value5[i] = data5[i];
    value6[i] = data6[i];
    value7[i] = data7[i];
    value8[i] = data8[i];
    value9[i] = data9[i];
    value10[i] = data10[i];
    value11[i] = data11[i];
  }
  work_group_barrier(CLK_LOCAL_MEM_FENCE);

  int lidx = get_local_linear_id();
  int data_offset = lidx * pre_wg_item_data_size;

  // double - char
  __devicelib_default_work_group_private_sort_spread_ascending_p3f64_p3i8_u32_p1i8(
      &key1[0] + data_offset, &value1[0] + data_offset, pre_wg_item_data_size,
      scratch1);

  // half - uchar
  __devicelib_default_work_group_private_sort_spread_descending_p3f16_p3u8_u32_p1i8(
      &key2[0] + data_offset, &value2[0] + data_offset, pre_wg_item_data_size,
      scratch2);

  // ulong - short
  __devicelib_default_work_group_private_sort_spread_ascending_p3u64_p3i16_u32_p1i8(
      &key3[0] + data_offset, &value3[0] + data_offset, pre_wg_item_data_size,
      scratch3);

  // float - ushort
  __devicelib_default_work_group_private_sort_spread_descending_p3f32_p3u16_u32_p1i8(
      &key4[0] + data_offset, &value4[0] + data_offset, pre_wg_item_data_size,
      scratch4);

  // uint - int
  __devicelib_default_work_group_private_sort_spread_ascending_p3u32_p3i32_u32_p1i8(
      &key5[0] + data_offset, &value5[0] + data_offset, pre_wg_item_data_size,
      scratch5);

  // long - uint
  __devicelib_default_work_group_private_sort_spread_descending_p3i64_p3u32_u32_p3i8(
      &key6[0] + data_offset, &value6[0] + data_offset, pre_wg_item_data_size,
      scratch6);

  // ushort - long
  __devicelib_default_work_group_private_sort_spread_ascending_p3u16_p3i64_u32_p3i8(
      &key7[0] + data_offset, &value7[0] + data_offset, pre_wg_item_data_size,
      scratch7);

  // int - ulong
  __devicelib_default_work_group_private_sort_spread_descending_p3i32_p3u64_u32_p3i8(
      &key8[0] + data_offset, &value8[0] + data_offset, pre_wg_item_data_size,
      scratch8);

  // uchar - float
  __devicelib_default_work_group_private_sort_spread_ascending_p3u8_p3f32_u32_p1i8(
      &key9[0] + data_offset, &value9[0] + data_offset, pre_wg_item_data_size,
      scratch9);

  // char - double
  __devicelib_default_work_group_private_sort_spread_descending_p3i8_p3f64_u32_p1i8(
      &key10[0] + data_offset, &value10[0] + data_offset, pre_wg_item_data_size,
      scratch10);

  // short - half
  __devicelib_default_work_group_private_sort_spread_ascending_p3i16_p3f16_u32_p1i8(
      &key11[0] + data_offset, &value11[0] + data_offset, pre_wg_item_data_size,
      scratch11);

  work_group_barrier(CLK_GLOBAL_MEM_FENCE);

  for (uint i = 0; i < data_size; i++) {
    data1[i] = value1[i];
    data2[i] = value2[i];
    data3[i] = value3[i];
    data4[i] = value4[i];
    data5[i] = value5[i];
    data6[i] = value6[i];
    data7[i] = value7[i];
    data8[i] = value8[i];
    data9[i] = value9[i];
    data10[i] = value10[i];
    data11[i] = value11[i];
  }
}

__attribute__((intel_reqd_sub_group_size(16))) __kernel void WG_joint_sort_v16(
    __global char *data1, __global uchar *data2, __global short *data3,
    __global ushort *data4, __global int *data5, __global uint *data6,
    __global long *data7, __global ulong *data8, __global float *data9,
    __global double *data10, __global half *data11) {
  __local char scratch1[2048];
  __local char scratch2[2048];
  __local char scratch3[2048];
  __local char scratch4[2048];
  __local char scratch5[2048];
  __local char scratch6[2048];
  __local char scratch7[2048];
  __local char scratch8[2048];
  __local char scratch9[2048];
  __local char scratch10[2048];
  __local char scratch11[2048];

  __local double key1[64];
  __local half key2[64];
  __local ulong key3[64];
  __local float key4[64];
  __local uint key5[64];
  __local long key6[64];
  __local ushort key7[64];
  __local int key8[64];
  __local uchar key9[64];
  __local char key10[64];
  __local short key11[64];

  __local char value1[64];
  __local uchar value2[64];
  __local short value3[64];
  __local ushort value4[64];
  __local int value5[64];
  __local uint value6[64];
  __local long value7[64];
  __local ulong value8[64];
  __local float value9[64];
  __local double value10[64];
  __local half value11[64];

  int data_size = 64;

  // 1,3...11 keys fill decrease data, and will run ascending sort
  //          key and value wiil be increasing
  // 2,4...10 keys fill increase data, and will run descending sort
  //          key and value wiil be decreasing
  for (uint i = 0; i < data_size; i++) {
    key1[i] = data_size - i;
    key2[i] = i;
    key3[i] = data_size - i;
    key4[i] = i;
    key5[i] = data_size - i;
    key6[i] = i;
    key7[i] = data_size - i;
    key8[i] = i;
    key9[i] = data_size - i;
    key10[i] = i;
    key11[i] = data_size - i;

    value1[i] = data1[i];
    value2[i] = data2[i];
    value3[i] = data3[i];
    value4[i] = data4[i];
    value5[i] = data5[i];
    value6[i] = data6[i];
    value7[i] = data7[i];
    value8[i] = data8[i];
    value9[i] = data9[i];
    value10[i] = data10[i];
    value11[i] = data11[i];
  }
  work_group_barrier(CLK_LOCAL_MEM_FENCE);

  int lidx = get_local_linear_id();

  // double - char
  __devicelib_default_work_group_joint_sort_ascending_p3f64_p3i8_u32_p1i8(
      &key1[0], &value1[0], data_size, scratch1);

  // half - uchar
  __devicelib_default_work_group_joint_sort_descending_p3f16_p3u8_u32_p1i8(
      &key2[0], &value2[0], data_size, scratch2);

  // ulong - short
  __devicelib_default_work_group_joint_sort_ascending_p3u64_p3i16_u32_p3i8(
      &key3[0], &value3[0], data_size, scratch3);

  // float - ushort
  __devicelib_default_work_group_joint_sort_descending_p3f32_p3u16_u32_p3i8(
      &key4[0], &value4[0], data_size, scratch4);

  // uint - int
  __devicelib_default_work_group_joint_sort_ascending_p3u32_p3i32_u32_p3i8(
      &key5[0], &value5[0], data_size, scratch5);

  // long - uint
  __devicelib_default_work_group_joint_sort_descending_p3i64_p3u32_u32_p1i8(
      &key6[0], &value6[0], data_size, scratch6);

  // ushort - long
  __devicelib_default_work_group_joint_sort_ascending_p3u16_p3i64_u32_p1i8(
      &key7[0], &value7[0], data_size, scratch7);

  // int - ulong
  __devicelib_default_work_group_joint_sort_descending_p3i32_p3u64_u32_p1i8(
      &key8[0], &value8[0], data_size, scratch8);

  // uchar - float
  __devicelib_default_work_group_joint_sort_ascending_p3u8_p3f32_u32_p1i8(
      &key9[0], &value9[0], data_size, scratch9);

  // char - double
  __devicelib_default_work_group_joint_sort_descending_p3i8_p3f64_u32_p1i8(
      &key10[0], &value10[0], data_size, scratch10);

  // short - half
  __devicelib_default_work_group_joint_sort_ascending_p3i16_p3f16_u32_p1i8(
      &key11[0], &value11[0], data_size, scratch11);

  work_group_barrier(CLK_GLOBAL_MEM_FENCE);

  for (uint i = 0; i < data_size; i++) {
    data1[i] = value1[i];
    data2[i] = value2[i];
    data3[i] = value3[i];
    data4[i] = value4[i];
    data5[i] = value5[i];
    data6[i] = value6[i];
    data7[i] = value7[i];
    data8[i] = value8[i];
    data9[i] = value9[i];
    data10[i] = value10[i];
    data11[i] = value11[i];
  }
}