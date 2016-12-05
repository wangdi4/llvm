// RUN: %clang_cc1 -fsyntax-only -std=c++11 -fintel-compatibility -verify %s
// RUN: %clang_cc1 -fsyntax-only -std=c++14 -fintel-compatibility -verify %s
// expected-no-diagnostics
struct base
{
    constexpr base() { }
      ~base() { }
};

struct derived : base {
    constexpr derived() { }
};
