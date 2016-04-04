// RUN: %clang_cc1 -fintel-compatibility -verify %s
// expected-no-diagnostics

__unaligned int *b;

struct UnalignedS {
  void foo(double) __unaligned { ; }
};

void bar(void (UnalignedS::*pf)(double)__unaligned);

