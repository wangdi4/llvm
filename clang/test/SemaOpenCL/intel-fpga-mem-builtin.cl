// RUN: %clang_cc1 %s -triple spir-unknown-unknown-intelfpga -cl-std=CL2.0 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -triple spir-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -triple x86_64-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -triple x86_64-unknown-unknown -cl-std=CL1.2 -verify -pedantic -fsyntax-only -DWITHOUTTRIPLE
// RUN: %clang_cc1 %s -triple spir-unknown-unknown -cl-std=CL1.2 -verify -pedantic -fsyntax-only -DWITHOUTTRIPLE

#define PARAM_1 1U << 7
#define PARAM_2 1U << 8

struct State {
  int x;
  float y;
};

#ifndef WITHOUTTRIPLE
void foo(global float *A, global int *B, global struct State *C) {
  global float *x;
  global int *y;
  global struct State *z;
  int i = 0;
  global void *U;

  x = __builtin_intel_fpga_mem(A, PARAM_1 | PARAM_2, -1);
  // expected-error@-1{{builtin parameter must be a non-negative integer constant}}
  x = __builtin_intel_fpga_mem(A, PARAM_1 | PARAM_2);
  // expected-error@-1{{too few arguments to function call, expected 3, have 2}}
  x = __builtin_intel_fpga_mem(A, PARAM_1 | PARAM_2, 1, 1);
  // expected-error@-1{{too many arguments to function call, expected 3, have 4}}
  y = __builtin_intel_fpga_mem(B, 0, i);
  // expected-error@-1{{argument to '__builtin_intel_fpga_mem' must be a constant integer}}
  z = __builtin_intel_fpga_mem(C, i, 0);
  // expected-error@-1{{argument to '__builtin_intel_fpga_mem' must be a constant integer}}
  z = __builtin_intel_fpga_mem(U, 0, 0);
  // expected-error@-1{{illegal pointer argument of type '__global void *__private'  to __builtin_intel_fpga_mem}}

  int intArr[10] = {0};
  int *k1 = __builtin_intel_fpga_mem(intArr, 0, 0);
  // expected-error@-1{{builtin parameter must be a pointer}}

  int **k2 = __builtin_intel_fpga_mem(&intArr, 0, 0);
  // expected-error@-1{{illegal pointer argument of type '__private int (*)[10]'  to __builtin_intel_fpga_mem}}
}
#else
void bar(global float *A) {
  global float *x = __builtin_intel_fpga_mem(A, 0, 0, 0, 1, -1, 0);
  // expected-error@-1{{'__builtin_intel_fpga_mem' is only available in OpenCL FPGA}}
}
#endif
