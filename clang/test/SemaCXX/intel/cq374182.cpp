// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

typedef int t;
t __restrict__ a; // expected-warning {{restrict requires a pointer or reference ('t' (aka 'int') is invalid)}} 

