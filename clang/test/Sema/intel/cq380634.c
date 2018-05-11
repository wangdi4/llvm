// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -fsyntax-only -verify -Wall %s -DTEST1
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -fsyntax-only -verify -Wtautological-constant-in-range-compare %s -DTEST2
// Note: This test was written to ensure that this warning doesn't happen with
// -Wall.  Since the warning was moved out of extra, it was altered to have its
// second run command just call the warning flag.

#if TEST1

// expected-no-diagnostics

int foo(unsigned int arg) {
  return (arg < 0);
}

#elif TEST2

int foo(unsigned int arg) {
  return (arg < 0); // expected-warning {{comparison of unsigned expression < 0 is always false}}
}

#else

#error Unknown test mode

#endif

