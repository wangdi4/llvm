// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s
int a;
// expected-warning@+1{{variable attributes appearing after a parenthesized initializer are ignored}}
int f(a) __attribute__((__unused__));

