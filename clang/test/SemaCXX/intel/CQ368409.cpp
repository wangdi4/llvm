// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -fintel-ms-compatibility -DMS -verify %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -pedantic -DPEDANTIC -verify %s

class C {
// expected-note@+1 {{implicitly declared private here}}
  static int foo();
// expected-note@+1 {{implicitly declared private here}}
  int i;
};
template <int (*pf)()>
struct C2 {};

#ifdef MS
// expected-error@+4 {{'foo' is a private member of 'C'}}
#else
// expected-warning@+2 {{'foo' is a private member of 'C'}}
#endif
void f() { new C2<C::foo>(); }

// expected-note@+1{{member is declared here}}
class B {
  // expected-note@+1 2 {{implicitly declared private here}}
  enum E { E1,
           E2 };

public:
  static const int m01;
};
const int B::m01 = 5;

// expected-note@+1 {{constrained by private inheritance here}}
struct C3 : private B, public C {
  // expected-error@+1 {{'E' is a private member of 'B'}}
  using B::E;
  // expected-error@+1 {{'i' is a private member of 'C'}}
  C3() { i = m01; }
};

template <int d>
struct C4 {
public:
  int foo() { return d; }
};
int main(void) {
#ifdef PEDANTIC
  // expected-warning@+6 {{'B' is a private member of 'B'}}
  // expected-error@+5 {{'E' is a private member of 'B'}}
#else
  // expected-warning@+3 {{type 'B' is a private member of 'B' (allowed for cfront compatibility)}}
  // expected-warning@+2 {{type 'E' is a private member of 'B' (allowed for cfront compatibility)}}
#endif
  return (new C4<C3::B::m01>())->foo() + (int)(B::E)(0);
}
