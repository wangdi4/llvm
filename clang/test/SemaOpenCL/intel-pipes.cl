// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -cl-std=CL1.2 -triple spir-unknown-unknown-intelfpga -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple x86_64-unknown-unknown -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -cl-std=CL1.2 -triple x86_64-unknown-unknown-intelfpga -fsyntax-only -verify %s

struct st {
  int i1;
  int i2;
};

pipe int multiarr[2][7] __attribute__((depth(0))); // expected-error{{can only be used as a function parameter in OpenCL}}

__kernel void k1() __attribute__((depth(100))) { // expected-warning{{'depth' attribute only applies to parameters and global variables}}
}

__kernel void k2(__global int *in_data,
                 __global float __attribute__((depth(1))) *fp_data, // expected-warning{{'depth' attribute only applies to OpenCL channels or pipes}}
                 read_only pipe float __attribute__((depth(-1))) pf, // expected-warning{{'depth' attribute parameter 0 is negative and will be ignored}}
                 write_only pipe int __attribute__((depth(10))) pi,
                 read_only pipe struct st __attribute__((depth(0))) ps) {
}

__kernel void k3() __attribute__((io("tmp"))) { // expected-warning{{'io' attribute only applies to variables}}
}

__kernel void k4(__global float __attribute__((io("tmp"))) *fp_data, // expected-warning{{'io' attribute only applies to OpenCL channels or pipes}}
                 read_only pipe float __attribute__((io(1))) pf, // expected-error{{expected string literal as argument of 'io' attribute}}
                 write_only pipe int __attribute__((io("tmp"))) pi) {
}
