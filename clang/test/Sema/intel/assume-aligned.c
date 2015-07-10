// CQ#373129
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

void correct_cases(int *a) {
  __assume_aligned(a, 32);
  __assume_aligned(a, 16);
  __assume_aligned(a, 1);
}

void incorrect_cases(int *a, int j) {
  __assume_aligned(a); // expected-error {{too few arguments to function call, expected 2, have 1}}
  __assume_aligned(a, 32, 0); // expected-error {{too many arguments to function call, expected 2, have 3}}
  __assume_aligned(j, 32); // expected-error {{first argument to '__assume_aligned' must be a pointer}}
  __assume_aligned(a, j); // expected-error {{argument to '__assume_aligned' must be a constant integer}}
  __assume_aligned(a, 31); // expected-error {{requested alignment is not a power of 2}}
}
