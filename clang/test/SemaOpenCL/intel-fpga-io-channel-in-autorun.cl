// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -fsyntax-only -verify %s

#pragma OPENCL EXTENSION cl_intel_channels : enable

channel int io_channel_a __attribute__((io("test"))); // expected-error{{usage of I/O channel 'io_channel_a' found in autorun kernel 'test_autorun'. See call stack below:}}

void write_helper(int data) {
  write_channel_intel(io_channel_a, data); // expected-note{{channel 'io_channel_a' is used for writing here}}
}

__attribute__((max_global_work_dim(0))) __attribute__((autorun)) // expected-note{{channel 'io_channel_a' is used for writing here}}
__kernel void test_autorun() {
  write_helper(10); // expected-note{{channel 'io_channel_a' is used for writing here}}
}

