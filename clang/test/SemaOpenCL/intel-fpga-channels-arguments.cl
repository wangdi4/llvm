// RUN: %clang_cc1 %s -triple spir64-unknown-unknown-intelfpga -fsyntax-only
// expected-no-diagnostics
//
// Check that Intel FPGA Channels work for function arguments if no global
// channel is declared yet.

#pragma OPENCL EXTENSION cl_intel_channels : enable

void f1(channel float ch) {
  float f = 0.42f;
  write_channel_intel(ch, f);
}

typedef long i64;
void f2(channel i64 ch) {
  long l = 42;
  write_channel_intel(ch, l);
}
