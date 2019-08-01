// RUN: %clang_cc1 -verify -fsyntax-only %s
// expected-no-diagnostics

void f(void) __attribute__((leaf));
void f2(void) __attribute__((__leaf__));
