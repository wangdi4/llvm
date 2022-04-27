// RUN: %clang_cc1 -fintel-compatibility -Wno-deprecated-non-prototype -verify %s

void f(a, b)
  int; // expected-warning{{declaration does not declare a parameter}}
{}

void g(a, b)
  int; // expected-warning{{declaration does not declare a parameter}}
  double; // expected-warning{{declaration does not declare a parameter}}
{}

void h(a, b)
  int a;
  double; // expected-warning{{declaration does not declare a parameter}}
{}

void z(a, b, c)
  double; // expected-warning{{declaration does not declare a parameter}}
  int; // expected-warning{{declaration does not declare a parameter}}
  double a;
{}
