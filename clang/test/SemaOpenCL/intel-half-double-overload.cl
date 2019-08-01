// RUN: %clang_cc1 %s -verify -pedantic -fsyntax-only -cl-std=CL1.2

float __attribute__((overloadable)) foo(float in1, float in2);
int __attribute__((overloadable)) goo(float in1, float in2);

#pragma OPENCL EXTENSION cl_khr_fp16 : enable
float __attribute__((overloadable)) foo(half in1, half in2);
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
half __attribute__((overloadable)) goo(double in1, double in2);
#pragma OPENCL EXTENSION cl_khr_fp64 : disable
#pragma OPENCL EXTENSION cl_khr_fp16 : disable

__kernel void vi(int x, int y) {
  foo(x, y);
  goo(x, y);
}

#pragma OPENCL EXTENSION cl_khr_fp16 : enable
float __attribute__((overloadable)) foo_err(half in1, half in2);
// expected-note@-1 {{candidate unavailable as it requires OpenCL extension '' to be enabled}}
float __attribute__((overloadable)) foo_err(half in1, int in2);
// expected-note@-1 {{candidate unavailable as it requires OpenCL extension '' to be enabled}}
#pragma OPENCL EXTENSION cl_khr_fp16 : disable

__kernel void vi_err(int x, int y) {
  foo_err(x, y);
  // expected-error@-1 {{no matching function for call to 'foo_err'}}
}
