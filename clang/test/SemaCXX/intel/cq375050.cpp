// RUN: %clang_cc1 -std=c++11 -fintel-compatibility -triple x86_64-unknown-linux-gnu -fsyntax-only -verify %s
// expected-no-diagnostics

struct Base {
  Base();
};

template <class Accessors> struct Derived : public Base {
  constexpr Derived() : Base() {}
};

struct D : public Derived<int> {
  typedef Derived Parent;
  constexpr D() : Parent() {}
};

