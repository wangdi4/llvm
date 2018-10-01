// RUN: %clang_cc1 %s -triple spir-unknown-unknown-intelfpga -cl-std=CL2.0 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -triple spir-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -triple x86_64-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -triple x86_64-unknown-unknown -cl-std=CL1.2 -verify -pedantic -fsyntax-only -DWITHOUTTRIPLE
// RUN: %clang_cc1 %s -triple spir-unknown-unknown -cl-std=CL1.2 -verify -pedantic -fsyntax-only -DWITHOUTTRIPLE

struct st {
  int a;
  float b;
};

void foo() {
#ifndef WITHOUTTRIPLE
  int a=123;
  int intArr[10] = {0};
  int b = __builtin_fpga_reg(a);
  int c = __builtin_fpga_reg(2.0f);
  int d = __builtin_fpga_reg( __builtin_fpga_reg( b+12 ));
  int e = __builtin_fpga_reg( __builtin_fpga_reg( a+b ));
  float f = 3.4f;
  int g = __builtin_fpga_reg((int)f);
  int *h = __builtin_fpga_reg(intArr); // expected-error{{non-POD and array types cannot be passed to __builtin_fpga_reg}}
  struct st i = {1, 5.0f};
  struct st ii = __builtin_fpga_reg(i);
  int *ap = &a;
  int *bp = __builtin_fpga_reg(ap);

  void * vp = __builtin_fpga_reg();
  // expected-error@-1{{too few arguments to function call, expected 1, have 0}}
  int tmp = __builtin_fpga_reg(1, 2);
  // expected-error@-1{{too many arguments to function call, expected 1, have 2}}

  // __fpga_reg should be an alias to __builtin_fpga_reg
  int orig_name = __fpga_reg(a);
#else
  int a=123;
  int b = __builtin_fpga_reg(a);
  // expected-error@-1{{'__builtin_fpga_reg' is only available in OpenCL FPGA}}
  int c = __fpga_reg(a);
  // expected-error@-1{{implicit declaration of function '__fpga_reg' is invalid in OpenCL}}
#endif
}
