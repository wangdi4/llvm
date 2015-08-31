// RUN: %clang_cc1 -fintel-compatibility -pedantic -fsyntax-only -verify %s

// Check that we have allowed incomplete element type.
struct A (*x)[3]; // expected-warning{{array with incomplete element type 'struct A' is an Intel extension}} \
                  // expected-note{{forward declaration of 'struct A'}}

// Check that we have *NOT* accidently allowed arrays with 'void' element type.
void y[5]; // expected-error{{array has incomplete element type 'void'}}

struct A {
  int a;
  int b;
};

// Check that we have *NOT* allowed arrays with elements of incomplete type at
// the moment of being used.
struct B (*z)[5]; // expected-warning{{array with incomplete element type 'struct B' is an Intel extension}} \
                  // expected-note{{forward declaration of 'struct B'}}
int main(void) {
  return (*z)[0].a;  // expected-error{{subscript of pointer to incomplete type 'struct B'}} \
                     // expected-note@-3{{forward declaration of 'struct B'}} 
}

struct B {
  int a;
};
