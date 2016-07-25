// RUN: %clang_cc1 -std=c++11 -fintel-compatibility -fcxx-exceptions -fsyntax-only -verify %s

struct A {
  A() noexcept(false) = default; // expected-warning {{exception specification of explicitly defaulted default constructor does not match the calculated one}}
  A(const A&) noexcept(false) = default; // expected-warning {{exception specification of explicitly defaulted copy constructor does not match the calculated one}}
  A(A&&) noexcept(false) = default; // expected-warning {{exception specification of explicitly defaulted move constructor does not match the calculated one}}
  A &operator=(const A&) noexcept(false) = default; // expected-warning {{exception specification of explicitly defaulted copy assignment operator does not match the calculated one}}
  A &operator=(A&&) noexcept(false) = default; // expected-warning {{exception specification of explicitly defaulted move assignment operator does not match the calculated one}}
  ~A() noexcept(false) = default; // expected-warning {{exception specification of explicitly defaulted destructor does not match the calculated one}}
};

struct B {
  B() {}
};

struct Test {
  Test() noexcept = default; // expected-warning {{exception specification of explicitly defaulted default constructor does not match the calculated one}}
  B b;
};
