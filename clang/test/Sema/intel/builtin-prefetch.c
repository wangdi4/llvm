// CQ#371990
// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s

int main(void) {
  int *p;
  __builtin_prefetch(p, -1, 0); // expected-warning{{expected a value between 0 and 1, defaults to 0}}
  __builtin_prefetch(p, 1, 4); // expected-warning{{expected a value between 0 and 3, defaults to 0}}
}
