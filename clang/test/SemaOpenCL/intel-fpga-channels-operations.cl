// RUN: %clang_cc1 -x cl -cl-std=CL1.2 -triple spir-unknown-unknown-intelfpga -fsyntax-only %s -verify
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown-intelfpga -fsyntax-only %s -verify

#pragma OPENCL EXTENSION cl_intel_channels : enable

channel int a, b;

__kernel void test() {
  a = b; // expected-error{{invalid operands to binary expression ('__global channel int' and '__global channel int')}}

  if (a < b) { // expected-error{{invalid operands to binary expression ('__global channel int' and '__global channel int')}}
    a *= 3; // expected-error{{invalid operands to binary expression ('__global channel int' and 'int')}}
  }
}
