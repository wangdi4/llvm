// RUN: %clang_cc1 -fsyntax-only -verify %s -DERROR
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DERROR

void foo() {
  return 1; // expected-error{{void function 'foo' should not return a value}}
}
