// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -triple spir -fsyntax-only -verify %s

channel int rgb;    // expected-error{{use of type '__global channel int' requires cl_intel_channels extension to be enabled}}
channel int arr[5]; // expected-error{{use of type '__global channel int [5]' requires cl_intel_channels extension to be enabled}}
channel int rgba  __attribute__((depth(3)));   // expected-error{{use of type '__global channel int' requires cl_intel_channels extension to be enabled}}

__kernel void channel() {} // expected-error{{cannot combine with previous 'void' declaration specifier}} expected-error{{expected identifier or '('}}

__kernel void k1() {
  int channel; // expected-error{{cannot combine with previous 'int' declaration specifier}} expected-warning{{declaration does not declare anything}}
}

__kernel void k2() {
  typedef int channel; // expected-error{{cannot combine with previous 'int' declaration specifier}} expected-warning{{typedef requires a name}}
  channel bar; // expected-error{{expected expression}}
}

