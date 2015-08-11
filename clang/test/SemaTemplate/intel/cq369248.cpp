// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s

template<typename T>
T foo() {
  T arr[] = T(); // expected-error {{array initializer must be an initializer list}}
  return arr[0];
}

int bar() {
  int arr[] = 2; // expected-error {{array initializer must be an initializer list}}
  return foo<int>(); // expected-note {{in instantiation of function template specialization 'foo<int>' requested here}}
}
