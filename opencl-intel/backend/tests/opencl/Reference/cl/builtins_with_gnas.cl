// Test for builtins with generic address space in SATest OpenCL Reference

__kernel void test_buitlins_with_gnas(__global float *in) {
  float a = 4.2f, b, res;
  int c;

  res = fract(a, &b);
  res = frexp(a, &c);
  res = lgamma_r(a, &c);
  res = modf(a, &b);
  res = remquo(a, b, &c);
  res = sincos(a, &b);
}
