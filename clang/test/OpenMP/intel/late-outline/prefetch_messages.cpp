// INTEL_COLLAB
// RUN: %clang_cc1 -fopenmp %s -verify

int foobar();

int main(int argc, char *argv[]) {
  int a;
  int b;
  int foo1[10];
  int foo2[10];

  // expected-error@+1 {{directive '#pragma ompx prefetch' requires the 'data' clause}}
  #pragma ompx prefetch

  // expected-error@+1 {{expected array section or array subscript in 'data' clause}}
  #pragma ompx prefetch data(1:foobar())

  // expected-error@+1 {{hint value must be a constant integer expression between 0 and 6, inclusive}}
  #pragma ompx prefetch data(a:foo1[0:100])

  // expected-error@+1 {{hint value, -1, must be between 0 and 6, inclusive}}
  #pragma ompx prefetch data(-1:foo1[:10])

  // expected-error@+1 {{hint value must be a constant integer expression between 0 and 6, inclusive}}
  #pragma ompx prefetch data(&foo1:foo1[:])

  // expected-error@+1 {{directive '#pragma ompx prefetch' cannot contain more than one 'if' clause}}
  #pragma ompx prefetch data(1:foo1[:]) if (a < b) if (a == 0)

  // expected-error@+1 {{expected '(' after 'data'}}
  #pragma ompx prefetch data

  // expected-error@+3 2 {{expected expression}}
  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(

  // expected-error@+3 {{expected array section or array subscript in 'data' clause}}
  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(2

  // expected-error@+3 {{expected expression}}
  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(2:
}

// Verify appropriate errors when using templates.
template <typename T, unsigned hint, unsigned size>
T run() {
  T foo[size];
  int d;
  // expected-error@+1 {{hint value, 7, must be between 0 and 6, inclusive}}
  #pragma ompx prefetch data(hint:foo[:size])
  return foo[0];
}

int template_test() {
  double d;
  // expected-note@+1 {{in instantiation of function template specialization 'run<double, 7U, 0U>' requested here}}
  d = run<double,7,0>();
  return 0;
}
// end INTEL_COLLAB
