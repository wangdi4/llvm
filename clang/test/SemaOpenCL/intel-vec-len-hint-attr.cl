// RUN: %clang_cc1 -cl-std=CL1.2 -fsyntax-only -verify %s

#if defined(cl_intel_vec_len_hint)
#pragma OPENCL EXTENSION cl_intel_vec_len_hint: enable
#endif

#if defined(cl_intel_vec_len_hint)
__attribute__((intel_vec_len_hint(31)))
#endif
// expected-error@-2 {{invalid attribute value, possible values are: 0, 1, 4, 8, 16}}
kernel void kernel1(int a) {}
