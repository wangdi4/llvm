// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility -gnu-permissive %s
// expected-no-diagnostics

template <typename T>
T foo(T a = T());

template <typename T>
T foo(T a = T());

int bar(int a = 1);
int bar(int a = 1) {
  return 0;
}
