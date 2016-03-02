// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

class B {
  int a; // expected-note {{implicitly declared private here}}
};

template <typename>
class C {
  int foo() {
    B a;
    return a.a; // expected-warning {{'a' is a private member of 'B'}}
  }
};

