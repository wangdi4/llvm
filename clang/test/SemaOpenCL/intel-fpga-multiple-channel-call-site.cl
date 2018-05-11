// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -cl-std=CL2.0 -verify -fsyntax-only %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -fsyntax-only %s
// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -cl-std=CL2.0 -verify -fsyntax-only %s
// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -fsyntax-only %s

// expected-no-diagnostics

#pragma OPENCL EXTENSION cl_intel_channels : enable

channel int a;
channel int b[2];

__kernel void k1(__global int *data) {
  data[0] = read_channel_intel(a);
  data[1] = read_channel_intel(b[0]);

  bool valid = false;
  data[2] = read_channel_nb_intel(a, &valid);
  data[3] = read_channel_nb_intel(b[0], &valid);

  data[4] = read_channel_intel(a);
  data[5] = read_channel_intel(b[0]);
}

__kernel void k2(__global int *data) {
  write_channel_intel(a, data[0]);
  write_channel_intel(b[0], data[1]);

  bool valid = false;
  valid = write_channel_nb_intel(a, data[2]);
  valid = write_channel_nb_intel(b[0], data[3]);

  write_channel_intel(a, data[4]);
  write_channel_intel(b[0], data[5]);
}
