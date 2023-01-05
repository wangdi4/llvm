#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  char2 cc2 = (char2)('a', 'b');
  uchar16 ucc16 = (uchar16)(250); // set all to 250
  ucc16.s0 = 240;
  ucc16.sF = 240; // set first ant last elements to 240

  short4 ss4 = (short4)(-30000, 32000, -99, 444);
  ushort3 uss3 = (ushort3)(52222, 300, 9);

  int3 ii3 = (int3)(120, 240, -20);
  uint8 uii8 = (uint8)(1, 2, 3, 4, 5, 6, 7, 8);

  long2 ll2 = (long2)(-9000000000, 100);
  ulong2 ull2 = (ulong2)(345999999999, 42);

  float4 ff4 = (float4)(4.5f, 6.0f, 3.25, -7.75);

  double8 dd8 = (double8)(1.875); // set all to 1.875
  dd8.s1 = -9000000000.0;         // [1] is different

  int2 iiarr[4];
  iiarr[0] = (int2)(4, 6);
  iiarr[1] = (int2)(-9, -20);
  iiarr[2] = (int2)(40, 60);
  iiarr[3] = (int2)(234, -456);

  int marr[2][3][4] = {{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}},
                       {{21, 22, 23, 24}, {25, 26, 27, 28}, {29, 30, 31, 32}}};

  buf_out[0] = 0;
}
