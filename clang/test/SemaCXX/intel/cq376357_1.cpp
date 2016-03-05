// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility -gnu-permissive -triple x86_64-unknown-linux-gnu %s

struct A {
  static void foo();
};

static void A::foo() {
  // expected-warning@-1{{'static' can only be specified inside the class definition}}
}

void A::foo();
// expected-warning@-1{{out-of-line declaration of a member must be a definition}}

typedef unsigned char bool;
// expected-warning@-1{{redeclaration of C++ built-in type 'bool'}}
int a1[0] = { 0 };
// expected-warning@-1{{excess elements in array initializer}}
int a1test[sizeof(a1) == 0 ? 1 : -1];
