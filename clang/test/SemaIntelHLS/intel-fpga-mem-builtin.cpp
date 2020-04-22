//RUN: %clang_cc1 -fhls -fsyntax-only -verify -pedantic %s
//RUN: %clang_cc1 -fsyntax-only -verify -pedantic %s -DWITHOUTFLAG

#define PARAM_1 1U << 7
#define PARAM_2 1U << 8

#ifndef WITHOUTFLAG
struct State {
  int x;
  float y;
};

struct inner {
  void (*fp)(); // expected-note {{Field with illegal type declared here}}
};

struct outer {
  inner A;
};

void foo(float *A, int *B, State *C) {
  float *x;
  int *y;
  State *z;
  int i;
  void *U;

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
  z = __builtin_intel_fpga_mem(U, PARAM_1 | PARAM_2, 0);
  // expected-error@-1{{illegal pointer argument of type 'void *'  to __builtin_intel_fpga_mem}}

  int intArr[10] = {0};
  int *k1 = __builtin_intel_fpga_mem(intArr, 0, 0);
  // expected-error@-1{{builtin parameter must be a pointer}}

  int **k2 = __builtin_intel_fpga_mem(&intArr, 0, 0);
  // expected-error@-1{{illegal pointer argument of type 'int (*)[10]'  to __builtin_intel_fpga_mem}}

  void (*fp1)();
  void (*fp2)() = __builtin_intel_fpga_mem(fp1, 0, 0);
  // expected-error@-1{{illegal pointer argument of type 'void (*)()'  to __builtin_intel_fpga_mem}}

  struct outer *iii;
  struct outer *iv = __builtin_intel_fpga_mem(iii, 0, 0);
  // expected-error@-1{{illegal field in type pointed to by pointer argument to __builtin_intel_fpga_mem}}
}
#else
void bar(float *A) {
  float *x = __builtin_intel_fpga_mem(A, PARAM_1 | PARAM_2, 127);
  // expected-error@-1{{'__builtin_intel_fpga_mem' is only available in OpenCL FPGA or SYCL device or HLS}}
}
#endif
