// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify --friend_injection %s -DNODIAG
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify --no_friend_injection -std=c++98 %s -DDIAGFUN -DDIAGCLASS
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify --gnu_version=40100 %s -DDIAGFUN -DDIAGCLASS
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify --gnu_version=40099 %s -DDIAGCLASS
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify --gnu_version=40000 %s -DNODIAG
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify --gnu_version=40000 -std=c++11 %s -DDIAGFUN -DDIAGCLASS

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

