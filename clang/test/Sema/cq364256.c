// RUN: %clang_cc1 %s -fsyntax-only -fintel-compatibility -verify -DTEST1
// RUN: %clang_cc1 %s -fsyntax-only -fms-compatibility -verify -DTEST2
// RUN: %clang_cc1 %s -fsyntax-only -verify -DTEST3
// RUN: %clang_cc1 %s -fsyntax-only -fintel-compatibility -fms-compatibility -verify -DTEST4

// In "-fintel-compatiblity" and "-fms-compatibility" modes intel-clang emits a
// warning, not an error, in case of non-void function not returning a value.
// CQ#364256.

#if defined(TEST1) || defined(TEST2) || defined(TEST4)
int foo() {
  return; // expected-warning {{non-void function 'foo' should return a value}}
}
#endif

#ifdef TEST3
int foo() {
  return; // expected-error {{non-void function 'foo' should return a value}}
}
#endif

