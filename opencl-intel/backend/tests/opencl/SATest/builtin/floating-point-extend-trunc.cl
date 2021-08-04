#pragma OPENCL EXTENSION cl_khr_fp16 : enable

kernel void test() {
  long double ld1 = 1.0;
  double d1 = ld1;
  float f1 = ld1;
  half h1 = ld1;

  float f2 = d1;
  half h2 = d1;

  half h3 = f2;

  float f4 = h3;
  double d4 = h3;
  long double ld4 = h3;

  double d5 = f4;
  long double ld5 = f4;

  long double ld6 = d5;
}
