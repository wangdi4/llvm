// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s -isystem %S

#ifdef __CQ369248__
template<typename T>
T foo() {
  const char *arr[] = "www";
  return T();
}
#else
#define __CQ369248__
#include <cq369248.cpp>
template<typename T>
T foo1() {
  const char *arr[] = "www"; // expected-error {{array initializer must be an initializer list}}
  return T();
}

template<typename T>
T foo2() {
  const char *arr[] = "www"; // expected-error {{array initializer must be an initializer list}}
  return T();
}

int bar() {
  int arr[] = 2; // expected-error {{array initializer must be an initializer list}}
  return foo1<int>();
}
#endif
