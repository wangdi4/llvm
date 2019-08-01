// RUN: %clang_cc1 -cl-std=CL1.2 -cl-ext=-cl_intel_vec_len_hint -fsyntax-only -verify %s

#pragma OPENCL EXTENSION cl_intel_vec_len_hint: enable // expected-warning {{unsupported OpenCL extension 'cl_intel_vec_len_hint' - ignoring}}
