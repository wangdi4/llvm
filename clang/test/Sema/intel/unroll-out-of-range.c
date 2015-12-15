// CQ#366562
// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s
// REQUIRES: llvm-backend

int main(void) {
  int i = 0, s = 0;

  #pragma unroll(0)
  for (i = 0; i < 10; ++i)
    s = s + i;

  #pragma unroll(-2) // expected-warning{{unrolling factor '-2' is negative and will be ignored}}
  for (i = 0; i < 10; ++i)
    s = s + i;

  #pragma unroll(1844674407370955161) // expected-warning{{unrolling factor '1844674407370955161' is too large and will be ignored}}
  for (i = 0; i < 20; ++i)
    s = s + i;

  // #pragma clang loop is not affected.
  #pragma clang loop unroll_count(0) // expected-error{{invalid value '0'; must be positive}}
  for (i = 0; i < 10; ++i)
    s = s + i;

  #pragma clang loop unroll_count(-1) // expected-error{{invalid value '-1'; must be positive}}
  for (i = 0; i < 10; ++i)
    s = s + i;

  #pragma clang loop unroll_count(1844674407370955161) // expected-error{{value '1844674407370955161' is too large}}
  for (i = 0; i < 10; ++i)
    s = s + i;

  return s;
}
