#define __ovld __attribute__((overloadable))
ushort __ovld __spirv_ConvertFToBF16INTEL(float);
ushort4 __ovld __spirv_ConvertFToBF16INTEL(float4);

__kernel void convert_float_bfloat16(__global uint *data1) {
  float input_f = 2.18f;
  ushort result_u = __spirv_ConvertFToBF16INTEL(input_f);
  data1[0] = result_u;

  float4 input_f_v4 = {2.18f, 2.17f, 2.19f, 2.2f};
  ushort4 result_u_v4 = __spirv_ConvertFToBF16INTEL(input_f_v4);
  data1[1] = result_u_v4[0];
  data1[2] = result_u_v4[1];
  data1[3] = result_u_v4[2];
  data1[4] = result_u_v4[3];

  unsigned int input_nan_u = 0xffd9ac74; // NaN
  float input_nan_f = *(float *)(&input_nan_u);
  ushort result_nan_u = __spirv_ConvertFToBF16INTEL(input_nan_f);
  data1[5] = convert_uint(result_nan_u);
}
