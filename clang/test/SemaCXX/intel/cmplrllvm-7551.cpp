// RUN: %clang_cc1 -triple spir64 -fsyntax-only -fintel-compatibility -verify %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fsyntax-only \
// RUN:  -fintel-compatibility -verify %s
// RUN: %clang_cc1 -triple x86_64-windows -fsyntax-only -fintel-compatibility \
// RUN: -verify %s
// RUN: %clang_cc1 -triple spir64 -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-windows -fsyntax-only -verify %s
// expected-no-diagnostics

template<typename T>
class Foo {
  typedef Foo<T> __type;
  template<typename V> __type& bar(V v);
};

typedef Foo<int> SFoo;
extern template SFoo& SFoo::bar(long);
