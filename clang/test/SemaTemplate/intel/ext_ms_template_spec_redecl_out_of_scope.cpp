// CQ#365451
// RUN: %clang_cc1 -fsyntax-only -verify %s -DTEST1
// RUN: %clang_cc1 -fsyntax-only -fms-compatibility -verify %s -DTEST2
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST1
// RUN: %clang_cc1 -fsyntax-only -fms-compatibility -fintel-compatibility -verify %s -DTEST1

#if TEST1

namespace A {
template <typename T> class X; // expected-note {{explicitly specialized declaration is here}}
}

namespace B {
template <> class A::X<int>; // expected-error {{class template specialization of 'X' not in a namespace enclosing 'A'}}
}

#elif TEST2

namespace A {
template <typename T> class X; // expected-note {{explicitly specialized declaration is here}}
}

namespace B {
template <> class A::X<int>; // expected-warning {{class template specialization of 'X' not in a namespace enclosing 'A' is a Microsoft extension}}
}

#else

#error Unknown test mode

#endif
