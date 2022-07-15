// INTEL_COLLAB
// RUN: %clang_cc1 -fopenmp %s -verify

int foobar() {
  return 1;
}

int main(int argc, char *argv[]) {
  int a;
  // expected-note@+1 {{declared here}}
  int b;
  int foo1[10];
  int foo2[10];

  // expected-error@+1 {{directive '#pragma ompx prefetch' requires the 'data' clause}}
  #pragma ompx prefetch

  // expected-error@+1 {{invalid expression in '#pragma ompx prefetch data'}}
  #pragma ompx prefetch data(foobar():1:1)

  // expected-error@+1 {{invalid expression in '#pragma ompx prefetch data'}}
  #pragma ompx prefetch data(&foo1:a:100)

  // expected-error@+2 {{expression is not an integral constant expression}}
  // expected-note@+1 {{read of non-const variable 'b' is not allowed in a constant expression}}
  #pragma ompx prefetch data(&foo1:4:b)

  // expected-error@+1 {{hint value, -1, must be between 1 and 4, inclusive}}
  #pragma ompx prefetch data(&foo1:-1:10)

  // expected-error@+1 {{argument to 'data' clause must be a strictly positive integer value}}
  #pragma ompx prefetch data(&foo1:4:0)

  // expected-error@+1 {{directive '#pragma ompx prefetch' cannot contain more than one 'if' clause}}
  #pragma ompx prefetch data(&foo1:1:2, &foo2:3:20) if (a < b) if (a == 0)

  // expected-error@+1 {{expected '(' after 'data'}}
  #pragma ompx prefetch data

  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(

  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(&foo1

  // expected-error@+4 {{expected expression}}
  // expected-error@+3 {{expected ':}}
  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(&foo1:

  // expected-error@+3 {{expected expression}}
  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(&foo1:1:

  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(&foo1:1:10

  // expected-error@+3 {{Expected comma in 'data' clause}}
  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(&foo1:1:10:

  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(&foo1:1:10,2

  // expected-error@+4 {{expected expression}}
  // expected-error@+3 {{expected ':'}}
  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(&foo1:1:10,2:

  // expected-error@+3 {{expected expression}}
  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(&foo1:1:10,2:3:

  // expected-error@+3 {{invalid expression in '#pragma ompx prefetch data'}}
  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(&foo1:1:10,2:3:10

  // expected-error@+1 {{expected ':'}}
  #pragma ompx prefetch data(&foo1,1,10)

  // expected-error@+3 {{expected ':'}}
  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma ompx prefetch data(&foo1:1
}

// Verify appropriate errors when using templates.
template <typename T, unsigned hint, unsigned size>
T run() {
  T foo[size];
  int d;
  // expected-error@+1 {{invalid expression in '#pragma ompx prefetch data'}}
  #pragma ompx prefetch data(foo:hint:size)
  // expected-error@+1 {{hint value, 0, must be between 1 and 4, inclusive}}
  #pragma ompx prefetch data(&foo:hint:size)
  // expected-error@+1 {{argument to 'data' clause must be a strictly positive integer value}}
  #pragma ompx prefetch data(&foo:1:size)
  return foo[0];
}

int template_test() {
  double d;
  // expected-note@+1 {{in instantiation of function template specialization 'run<double, 0U, 0U>' requested here}}
  d = run<double,0,0>();
  return 0;
}
// end INTEL_COLLAB
