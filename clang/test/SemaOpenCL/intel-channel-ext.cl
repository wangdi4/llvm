// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir -fsyntax-only -verify %s

channel int rgb;    // expected-error{{use of type '__global channel int' requires cl_altera_channels extension to be enabled}}
channel int arr[5]; // expected-error{{use of type 'channel int __global[5]' requires cl_altera_channels extension to be enabled}}
channel int rgba  __attribute__((depth(3)));   // expected-error{{use of type '__global channel int' requires cl_altera_channels extension to be enabled}}
