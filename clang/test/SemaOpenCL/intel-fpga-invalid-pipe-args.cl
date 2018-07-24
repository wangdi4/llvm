// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -cl-std=CL1.2 -triple spir-unknown-unknown-intelfpga -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple x86_64-unknown-unknown -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -cl-std=CL1.2 -triple x86_64-unknown-unknown-intelfpga -fsyntax-only -verify %s

kernel void k1( read_only pipe int p ) {
  int x = 0;
  write_pipe(p, &x); // expected-error{{invalid pipe access modifier (expecting write_only)}}
}

kernel void k2( write_only pipe int p ) {
  int x = 0;
  read_pipe(p, &x); // expected-error{{invalid pipe access modifier (expecting read_only)}}
}
