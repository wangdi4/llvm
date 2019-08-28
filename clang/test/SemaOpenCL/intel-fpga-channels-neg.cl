// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -fsyntax-only -verify -cl-ext=-cl_intel_channels %s
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -fsyntax-only -verify -cl-ext=-cl_intel_channels -D __KERNEL %s
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -fsyntax-only -verify -cl-ext=-cl_intel_channels -D __FUNCTION %s
// RUN: %clang_cc1 -x cl -fsyntax-only -verify -cl-ext=-cl_intel_channels %s
// RUN: %clang_cc1 -x cl -fsyntax-only -verify -cl-ext=-cl_intel_channels -D __KERNEL %s
// RUN: %clang_cc1 -x cl -fsyntax-only -verify -cl-ext=-cl_intel_channels -D __FUNCTION %s

// 'channel' should be treated as an identifier if cl_intel_channels extension
// is not supported

#ifdef __KERNEL

__kernel void channel() { // expected-no-diagnostics

}

#elif __FUNCTION

int channel() { // expected-no-diagnostics
    return 42;
}

__kernel void t(__global int* out) {
    out[0] = channel();
}

#else

__constant int channel = 1;
channel int foo; // expected-error{{unknown type name 'channel'}}

__kernel void k1() {
  int channel = 5;
  if (channel == 6) {
  }
}

__kernel void k2() {
  int *channel;
}

__kernel void k3(__global int *channel) {
}

__kernel void k4() {
  typedef int channel;
  channel bar;
}

#endif
