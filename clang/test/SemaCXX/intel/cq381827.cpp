// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility -std=c++11 %s -isystem %S
// expected-no-diagnostics

#ifndef __CQ381827__
#define __CQ381827__
#include <cq381827.cpp>
#else

struct A {
    static const int a;
};
const int A::a = 1;

template <class T = A>
struct B {
  B() noexcept(T::a);
};

template <class T>
B<T>::B() {
}

B<> x;

#endif
