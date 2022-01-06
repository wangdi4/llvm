// RUN: %clang_cc1 %s -fsyntax-only -fintel-compatibility -verify -DWARN
// RUN: %clang_cc1 %s -fsyntax-only -fms-compatibility -verify -DERROR
// RUN: %clang_cc1 %s -fsyntax-only -fintel-compatibility -fms-compatibility -verify -DWARN
// RUN: %clang_cc1 %s -fsyntax-only -verify -DERROR

// In "-fintel-compatiblity" and "-fms-compatibility" modes intel-clang emits a
// warning, not an error, in case of non-void function not returning a value.
// CQ#364256.

#ifdef WARN
int foo() {
  return; // expected-warning {{non-void function 'foo' should return a value}}
}
#endif

#ifdef ERROR
int foo() {
  return; // expected-error {{non-void function 'foo' should return a value}}
}
#endif

