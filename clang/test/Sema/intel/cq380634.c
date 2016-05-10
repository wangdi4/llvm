// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -fsyntax-only -verify -Wall %s -DTEST1
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -fsyntax-only -verify -Wextra %s -DTEST2

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

