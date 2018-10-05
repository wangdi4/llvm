// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown-intelfpga -fsyntax-only -verify -DINTELFPGA %s
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir64-unknown-unknown-intelfpga -fsyntax-only -verify -DINTELFPGA %s
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir64 -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -fsyntax-only -verify -DINTELFPGA %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -fsyntax-only -verify -DINTELFPGA %s
// RUN: %clang_cc1 -x cl -triple spir -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -triple spir64 -fsyntax-only -verify %s

__attribute__((max_global_work_dim(0)))
__attribute__((num_compute_units(2, 2)))
__attribute__((autorun))
__kernel void foo() {
  int x = get_compute_id(0);
}

#ifdef INTELFPGA
// expected-no-diagnostics
#else
// expected-warning@-10 {{unknown attribute 'max_global_work_dim' ignored}}
// expected-warning@-10 {{unknown attribute 'num_compute_units' ignored}}
// expected-warning@-10 {{unknown attribute 'autorun' ignored}}
// expected-error@-9 {{implicit declaration of function 'get_compute_id' is invalid in OpenCL}}
#endif
