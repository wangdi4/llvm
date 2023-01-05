// RUN: SATest -VAL -config=%s.cfg -neat=1 --force_ref
// CHECK: Test Passed.
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
__kernel void test_def_f(char a, short b, int c, long d, float e,
                         double f) { // from float to int(def rounding)
  a = convert_char(e);
  b = convert_short(e);
  c = convert_int(e);
  d = convert_long(e);
}
__kernel void test_def_d(char a, short b, int c, long d, float e,
                         double f) { // from double to int(def rounding)
  a = convert_char(f);
  b = convert_short(f);
  c = convert_int(f);
  d = convert_long(f);
}
__kernel void test_rte_f(char a, short b, int c, long d, float e,
                         double f) { // from float to int(rte rounding)
  a = convert_char_rte(e);
  b = convert_short_rte(e);
  c = convert_int_rte(e);
  d = convert_long_rte(e);
}
__kernel void test_rte_d(char a, short b, int c, long d, float e,
                         double f) { // from double to int(rte rounding)
  a = convert_char_rte(f);
  b = convert_short_rte(f);
  c = convert_int_rte(f);
  d = convert_long_rte(f);
}
__kernel void test_rtz_f(char a, short b, int c, long d, float e,
                         double f) { // from float to int(rtz rounding)
  a = convert_char_rtz(e);
  b = convert_short_rtz(e);
  c = convert_int_rtz(e);
  d = convert_long_rtz(e);
}
__kernel void test_rtz_d(char a, short b, int c, long d, float e,
                         double f) { // from double to int(rtz rounding)
  a = convert_char_rtz(f);
  b = convert_short_rtz(f);
  c = convert_int_rtz(f);
  d = convert_long_rtz(f);
}
__kernel void test_rtp_f(char a, short b, int c, long d, float e,
                         double f) { // from float to int(rtp rounding)
  a = convert_char_rtp(e);
  b = convert_short_rtp(e);
  c = convert_int_rtp(e);
  d = convert_long_rtp(e);
}
__kernel void test_rtp_d(char a, short b, int c, long d, float e,
                         double f) { // from double to int(rtp rounding)
  a = convert_char_rtp(f);
  b = convert_short_rtp(f);
  c = convert_int_rtp(f);
  d = convert_long_rtp(f);
}
__kernel void test_rtn_f(char a, short b, int c, long d, float e,
                         double f) { // from float to int(rtn rounding)
  a = convert_char_rtn(e);
  b = convert_short_rtn(e);
  c = convert_int_rtn(e);
  d = convert_long_rtn(e);
}
__kernel void test_rtn_d(char a, short b, int c, long d, float e,
                         double f) { // from double to int(rtn rounding)
  a = convert_char_rtn(f);
  b = convert_short_rtn(f);
  c = convert_int_rtn(f);
  d = convert_long_rtn(f);
}