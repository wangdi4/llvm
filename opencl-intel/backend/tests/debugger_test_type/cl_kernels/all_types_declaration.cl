#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void main_kernel(__global uchar *buf_in, __global uchar *buf_out) {
  bool var_bool = true;
  bool *p_bool = (bool *)34;
  char var_char = 2;
  char *p_char = (char *)34;
  uchar var_uchar = 2;
  uchar *p_uchar = (uchar *)34;
  short var_short = 28;
  short *p_short = (short *)34;
  ushort var_ushort = 28;
  ushort *p_ushort = (ushort *)34;
  int var_int = 28;
  int *p_int = (int *)34;
  uint var_uint = 28;
  uint *p_uint = (uint *)34;
  long var_long = 28;
  long *p_long = (long *)34;
  ulong var_ulong = 28;
  ulong *p_ulong = (ulong *)34;
  float var_float = 28;
  float *p_float = (float *)34;
  double var_double = 28;
  double *p_double = (double *)34;
  size_t var_size_t = 28;
  size_t *p_size_t = (size_t *)34;
  ptrdiff_t var_ptrdiff_t = 28;
  ptrdiff_t *p_ptrdiff_t = (ptrdiff_t *)34;
  intptr_t var_intptr_t = 28;
  intptr_t *p_intptr_t = (intptr_t *)34;
  uintptr_t var_uintptr_t = 28;
  uintptr_t *p_uintptr_t = (uintptr_t *)34;
  char2 var_char2 = (char2)(2, 3);
  char2 *p_char2 = (char2 *)34;
  char3 var_char3 = (char3)(2, 3, 4);
  char3 *p_char3 = (char3 *)34;
  char4 var_char4 = (char4)(2, 3, 4, 5);
  char4 *p_char4 = (char4 *)34;
  char8 var_char8 = (char8)(2, 3, 4, 5, 6, 7, 8, 9);
  char8 *p_char8 = (char8 *)34;
  char16 var_char16 =
      (char16)(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
  char16 *p_char16 = (char16 *)34;
  uchar2 var_uchar2 = (uchar2)(2, 3);
  uchar2 *p_uchar2 = (uchar2 *)34;
  uchar3 var_uchar3 = (uchar3)(2, 3, 4);
  uchar3 *p_uchar3 = (uchar3 *)34;
  uchar4 var_uchar4 = (uchar4)(2, 3, 4, 5);
  uchar4 *p_uchar4 = (uchar4 *)34;
  uchar8 var_uchar8 = (uchar8)(2, 3, 4, 5, 6, 7, 8, 9);
  uchar8 *p_uchar8 = (uchar8 *)34;
  uchar16 var_uchar16 =
      (uchar16)(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
  uchar16 *p_uchar16 = (uchar16 *)34;
  short2 var_short2 = (short2)(2, 3);
  short2 *p_short2 = (short2 *)34;
  short3 var_short3 = (short3)(2, 3, 4);
  short3 *p_short3 = (short3 *)34;
  short4 var_short4 = (short4)(2, 3, 4, 5);
  short4 *p_short4 = (short4 *)34;
  short8 var_short8 = (short8)(2, 3, 4, 5, 6, 7, 8, 9);
  short8 *p_short8 = (short8 *)34;
  short16 var_short16 =
      (short16)(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
  short16 *p_short16 = (short16 *)34;
  ushort2 var_ushort2 = (ushort2)(2, 3);
  ushort2 *p_ushort2 = (ushort2 *)34;
  ushort3 var_ushort3 = (ushort3)(2, 3, 4);
  ushort3 *p_ushort3 = (ushort3 *)34;
  ushort4 var_ushort4 = (ushort4)(2, 3, 4, 5);
  ushort4 *p_ushort4 = (ushort4 *)34;
  ushort8 var_ushort8 = (ushort8)(2, 3, 4, 5, 6, 7, 8, 9);
  ushort8 *p_ushort8 = (ushort8 *)34;
  ushort16 var_ushort16 =
      (ushort16)(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
  ushort16 *p_ushort16 = (ushort16 *)34;
  int2 var_int2 = (int2)(2, 3);
  int2 *p_int2 = (int2 *)34;
  int3 var_int3 = (int3)(2, 3, 4);
  int3 *p_int3 = (int3 *)34;
  int4 var_int4 = (int4)(2, 3, 4, 5);
  int4 *p_int4 = (int4 *)34;
  int8 var_int8 = (int8)(2, 3, 4, 5, 6, 7, 8, 9);
  int8 *p_int8 = (int8 *)34;
  int16 var_int16 =
      (int16)(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
  int16 *p_int16 = (int16 *)34;
  uint2 var_uint2 = (uint2)(2, 3);
  uint2 *p_uint2 = (uint2 *)34;
  uint3 var_uint3 = (uint3)(2, 3, 4);
  uint3 *p_uint3 = (uint3 *)34;
  uint4 var_uint4 = (uint4)(2, 3, 4, 5);
  uint4 *p_uint4 = (uint4 *)34;
  uint8 var_uint8 = (uint8)(2, 3, 4, 5, 6, 7, 8, 9);
  uint8 *p_uint8 = (uint8 *)34;
  uint16 var_uint16 =
      (uint16)(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
  uint16 *p_uint16 = (uint16 *)34;
  long2 var_long2 = (long2)(2, 3);
  long2 *p_long2 = (long2 *)34;
  long3 var_long3 = (long3)(2, 3, 4);
  long3 *p_long3 = (long3 *)34;
  long4 var_long4 = (long4)(2, 3, 4, 5);
  long4 *p_long4 = (long4 *)34;
  long8 var_long8 = (long8)(2, 3, 4, 5, 6, 7, 8, 9);
  long8 *p_long8 = (long8 *)34;
  long16 var_long16 =
      (long16)(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
  long16 *p_long16 = (long16 *)34;
  ulong2 var_ulong2 = (ulong2)(2, 3);
  ulong2 *p_ulong2 = (ulong2 *)34;
  ulong3 var_ulong3 = (ulong3)(2, 3, 4);
  ulong3 *p_ulong3 = (ulong3 *)34;
  ulong4 var_ulong4 = (ulong4)(2, 3, 4, 5);
  ulong4 *p_ulong4 = (ulong4 *)34;
  ulong8 var_ulong8 = (ulong8)(2, 3, 4, 5, 6, 7, 8, 9);
  ulong8 *p_ulong8 = (ulong8 *)34;
  ulong16 var_ulong16 =
      (ulong16)(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
  ulong16 *p_ulong16 = (ulong16 *)34;
  float2 var_float2 = (float2)(2.0, 3.0);
  float2 *p_float2 = (float2 *)34;
  float3 var_float3 = (float3)(2.0, 3.0, 4.0);
  float3 *p_float3 = (float3 *)34;
  float4 var_float4 = (float4)(2.0, 3.0, 4.0, 5.0);
  float4 *p_float4 = (float4 *)34;
  float8 var_float8 = (float8)(2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
  float8 *p_float8 = (float8 *)34;
  float16 var_float16 = (float16)(2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 2.0,
                                  3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
  float16 *p_float16 = (float16 *)34;
  double2 var_double2 = (double2)(2.0, 3.0);
  double2 *p_double2 = (double2 *)34;
  double3 var_double3 = (double3)(2.0, 3.0, 4.0);
  double3 *p_double3 = (double3 *)34;
  double4 var_double4 = (double4)(2.0, 3.0, 4.0, 5.0);
  double4 *p_double4 = (double4 *)34;
  double8 var_double8 = (double8)(2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
  double8 *p_double8 = (double8 *)34;
  double16 var_double16 = (double16)(2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0,
                                     2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
  double16 *p_double16 = (double16 *)34;
  sampler_t var_sampler_t = CLK_NORMALIZED_COORDS_TRUE;
  event_t *p_event_t = (event_t *)34;
  half *p_half = (half *)34;
  int a = 1;
  return;
}
