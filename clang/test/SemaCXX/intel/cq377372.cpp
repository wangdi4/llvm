// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s

__attribute__((gnu_inline)) extern inline void getline(int a, double b) { // expected-note {{previous definition is here}}
  b = a;
}

void getline(int a, double b) { b = a; } // expected-warning {{redefinition of a 'extern inline' function 'getline' in C++ mode is an Intel extension}}
