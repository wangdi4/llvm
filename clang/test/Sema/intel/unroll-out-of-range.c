// CQ#366562
// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -DERROR -verify %s
// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -DOK -verify %s
// REQUIRES: llvm-backend

int main(void) {
  int i = 0, s = 0;

#if ERROR

#pragma clang loop unroll_count(0) // expected-error{{invalid value '0'; must be positive}}
  for (i = 0; i < 10; ++i)
    s = s + i;

#elif OK

#pragma unroll(0) // expected-no-diagnostics
  for (i = 0; i < 10; ++i)
    s = s + i;

#pragma unroll(2) // expected-no-diagnostics
  for (i = 0; i < 10; ++i)
    s = s + i;

#else

#error Unknown test mode.

#endif

  return s;
}
