// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s
// expected-no-diagnostics

template <class T, int I> struct A { void h(); };

struct A<char, 2> {
};

template <class T, int I> struct B { void h(); };

void B<char, 2>::h(){};

