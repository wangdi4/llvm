// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -std=c++98 --friend_injection %s -DNODIAG
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -std=c++11 --friend_injection %s -DNODIAG

// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -std=c++98 %s -DDIAGFUN -DDIAGCLASS
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -std=c++11 %s -DDIAGFUN -DDIAGCLASS

#if NODIAG
// expected-no-diagnostics
#endif // NODIAG

struct S {
  friend class C;
  friend void *foo();
};
#if DIAGFUN
// expected-error@+2 {{use of undeclared identifier 'foo'}}
#endif // DIAGFUN
void *a = foo();
#if DIAGCLASS
// expected-error@+2 {{unknown type name 'C'}}
#endif // DIAGCLASS
C *c;

