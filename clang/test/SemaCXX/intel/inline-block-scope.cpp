// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s 

void f0a(void) {
   inline void f1(); // expected-warning {{inline declaration of 'f1' not allowed in block scope}}
}

void x() {
  inline int f(int); // expected-warning {{inline declaration of 'f' not allowed in block scope}}
  inline int n; // expected-warning {{inline declaration of 'n' not allowed in block scope}}
  static inline int m; // expected-warning {{inline declaration of 'm' not allowed in block scope}}
}
