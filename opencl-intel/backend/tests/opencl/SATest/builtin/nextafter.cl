#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#define SUB_GROUP_SIZE 4
#define LOCAL_WORK_SIZE 4

__attribute__((intel_reqd_sub_group_size(SUB_GROUP_SIZE)))
__kernel void nextafter_half_type(__global half* data1, __global ushort* data2,
                                __global ushort* data3, __global half* data4,
                                __global half* data5, __global half* data6,
                                __global half* data7, __global half* data8,
                                __global half* data9) {
  int sglid = get_sub_group_local_id();
  
  // normal num
  half from1 = data1[sglid];
  half to1 = data4[sglid];
  data1[sglid] = nextafter(from1, to1);

  // from is nan
  ushort temp_nan = 0x7c0f;
  half temp_nan_half = *(half*)(&temp_nan);
  half from2 = temp_nan_half;
  half data2_temp = nextafter(from2, to1);
  data2[sglid] = *(ushort*)(&data2_temp);

  // to is nan
  half from3 = from1;
  half to3 = temp_nan_half;
  half data3_temp = nextafter(from3, to3);
  data3[sglid] = *(ushort*)(&data3_temp);

  // to is infinite
  ushort temp_inf = 0x7c00;
  half temp_inf_half = *(half*)(&temp_inf);
  half to4 = temp_inf_half;
  data4[sglid] = nextafter(from3, to4);

  // same num
  data5[sglid] = nextafter(from1, from1);
  
  // +0 to -1
  half from5 = 0.f;
  half to5 = -1.f;
  data6[sglid] = nextafter(from5, to5);

  // -0 to +1
  half from6 = -0.f;
  half to6 = 1.f;
  data7[sglid] = nextafter(from6, to6);

  // -0 to +0
  half from7 = -0.f;
  half to7 = 0.f;
  data8[sglid] = nextafter(from7, to7);

  // +0 to -0
  half from8 = 0.f;
  half to8 = -0.f;
  data9[sglid] = nextafter(from8, to8);
}
