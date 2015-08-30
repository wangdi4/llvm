// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -pedantic -verify %s

extern int i1; // expected-note {{previous declaration is here}}
static int i1; // expected-warning {{redeclaring non-static 'i1' as static is an Intel extension}}

static int i2; // expected-note {{previous definition is here}}
int i2;        // expected-warning {{redeclaring static 'i2' as non-static is an Intel extension}}

int f1();        // expected-note {{previous declaration is here}}
static int f1(); // expected-warning {{redeclaring non-static 'f1' as static is an Intel extension}}

int f2();          // expected-note {{previous declaration is here}}
static int f2() {  // expected-warning {{redeclaring non-static 'f2' as static is an Intel extension}}
  return 1;
}

int main(void) {
  static int f3(); // expected-warning {{static functions inside blocks are Intel extension}}
}
