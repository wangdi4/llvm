// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown -cl-ext=+cl_intel_channels -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown -cl-ext=+cl_intel_channels -fsyntax-only -verify -D__IHC_USE_DEPRECATED_NAMES %s
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown -cl-ext=+cl_intel_channels -fsyntax-only -verify -DUSE_LEGACY_EXT %s
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown -cl-ext=+cl_intel_channels -fsyntax-only -verify -DUSE_LEGACY_EXT -D__IHC_USE_DEPRECATED_NAMES %s

// expected-no-diagnostics

#ifdef USE_LEGACY_EXT
#pragma OPENCL EXTENSION cl_altera_channels : enable
#else
#pragma OPENCL EXTENSION cl_intel_channels : enable
#endif

channel float fch;
channel int ich;

__kernel void f() {
#ifdef __IHC_USE_DEPRECATED_NAMES
  int i = read_channel_altera(ich);
  float f = read_channel_altera(fch);
#else
  int i = read_channel_intel(ich);
  float f = read_channel_intel(fch);
#endif
}
