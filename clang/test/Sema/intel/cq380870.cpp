// RUN: %clang_cc1 -fintel-compatibility -verify %s

struct I {
  unsigned n;
  int c[3];
  int s;
};

struct S {
  I v;
  void f(const I&);
};

void foo(S &s) {
  s.f({ 0, {}, 0 });  // expected-warning{{generalized initializer lists are a C++11 extension}}
}
