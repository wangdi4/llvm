#pragma OPENCL EXTENSION cl_khr_fp16 : enable

// Check the builtin result for edge cases (NaN, 0, Inf, ...)
// We don't care about execution parallelism so the kernel is executed by 1
// single item.
__kernel void test(__global int *out) {
  half h_nan = nan((ushort)1);
  half h_inf = __builtin_astype((ushort)0x7c00, half);
  float f_nan = nan((uint)1);
  float f_inf = __builtin_astype((uint)0x7f800000, float);
  double d_nan = nan((ulong)1);
  double d_inf = __builtin_astype((ulong)0x7FF0000000000000L, double);

  // atan2
  // expects NaN if either arg is NaN
  // half
  out[0] = isnan(atan2(h_nan, (half)1));
  out[1] = isnan(atan2((half)1, h_nan));
  // float
  out[2] = isnan(atan2(f_nan, (float)1));
  out[3] = isnan(atan2((float)1, f_nan));
  // double
  out[4] = isnan(atan2(d_nan, (double)1));
  out[5] = isnan(atan2((double)1, d_nan));

  // hypot
  // expects +Inf if either arg is +/-Inf, even when the other arg is NaN
  // half
  out[6] = isinf(hypot(h_inf, (half)1));
  out[7] = isinf(hypot(-h_inf, (half)1));
  out[8] = isinf(hypot(h_inf, h_nan));
  out[9] = isinf(hypot(-h_inf, h_nan));
  out[10] = isinf(hypot((half)1, h_inf));
  out[11] = isinf(hypot((half)1, -h_inf));
  out[12] = isinf(hypot(h_nan, h_inf));
  out[13] = isinf(hypot(h_nan, -h_inf));
  // float
  out[14] = isinf(hypot(f_inf, (float)1));
  out[15] = isinf(hypot(-f_inf, (float)1));
  out[16] = isinf(hypot(f_inf, f_nan));
  out[17] = isinf(hypot(-f_inf, f_nan));
  out[18] = isinf(hypot((float)1, f_inf));
  out[19] = isinf(hypot((float)1, -f_inf));
  out[20] = isinf(hypot(f_nan, f_inf));
  out[21] = isinf(hypot(f_nan, -f_inf));
  // double
  out[22] = isinf(hypot(d_inf, (double)1));
  out[23] = isinf(hypot(-d_inf, (double)1));
  out[24] = isinf(hypot(d_inf, d_nan));
  out[25] = isinf(hypot(-d_inf, d_nan));
  out[26] = isinf(hypot((double)1, d_inf));
  out[27] = isinf(hypot((double)1, -d_inf));
  out[28] = isinf(hypot(d_nan, d_inf));
  out[29] = isinf(hypot(d_nan, -d_inf));
  // expects NaN if either arg is NaN
  // half
  out[30] = isnan(hypot((half)1, h_nan));
  out[31] = isnan(hypot(h_nan, (half)1));
  // float
  out[32] = isnan(hypot((float)1, f_nan));
  out[33] = isnan(hypot(f_nan, (float)1));
  // double
  out[34] = isnan(hypot((double)1, d_nan));
  out[35] = isnan(hypot(d_nan, (double)1));

  // sub_group_broadcast
  // This is a SNaN (Signaling NaN):
  // exponent bits all set + MSB of mantissa unset + non-zero mantissa
  // Make sure we don't touch data bits when broadcasting SNaN inputs
  ushort2 h_snan = 0x7CBD;
  half2 broadcasted_nan =
      sub_group_broadcast(__builtin_astype(h_snan, half2), 0);
  short2 res = __builtin_astype(broadcasted_nan, ushort2) == h_snan;
  out[36] = res[0] & res[1];
}
