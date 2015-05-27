// CQ#366612
// RUN: %clang_cc1 -fsyntax-only -verify %s -DTEST0
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST1
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST2
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST3
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST4
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST5
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST6
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DTEST7

#if TEST0

// Declarations with incompatible types.
void foo(long *a); // expected-note{{previous declaration is here}}
int foo(const long *a); // expected-error{{conflicting types for 'foo'}}

#elif TEST1

// Declarations with different return types.
void foo(int a, int b); // expected-note{{previous declaration is here}}
int foo(const char *a); // expected-warning{{declaration of 'foo' is incompatible with previous declaration}}

#elif TEST2

// Declarations with different number of arguments.
void foo(int a); // expected-note{{previous declaration is here}}
void foo(double a, ...); // expected-warning{{declaration of 'foo' is incompatible with previous declaration}}

#elif TEST3

// Declarations with incompatible types.
void foo(short a, char b); // expected-note{{previous declaration is here}}
int foo(long double a, long long b, const char *c, ...); // expected-warning{{declaration of 'foo' is incompatible with previous declaration}}

#elif TEST4

// Definition and declaration.
void foo(long double a, unsigned short b) {} // expected-note{{previous definition is here}}
int foo(short b); // expected-warning{{declaration of 'foo' is incompatible with previous definition}}

#elif TEST5

// Declaration and definition.
int foo(const int a, ...); // expected-note{{previous declaration is here}}
void foo(float a, float b) {} // expected-warning{{declaration of 'foo' is incompatible with previous declaration}}

#elif TEST6

// Definition and redefinition.
void foo(long *a) {} // expected-note 2 {{previous definition is here}}
void foo(const long *a) {} // expected-warning{{declaration of 'foo' is incompatible with previous definition}} expected-error{{redefinition of 'foo'}}

#elif TEST7

typedef struct S1 {
  char a;
  short b;
} S1;

void foo(int a); // expected-note{{previous declaration is here}}
const char *foo(S1 a, long double *b, const char c); // expected-warning{{declaration of 'foo' is incompatible with previous declaration}}

#else

#error Unknown test mode

#endif


int main() {
#if TEST0
  long *lp = 0;
  foo(lp);
#elif TEST1
  char *cp = 0;
  foo(cp);
#elif TEST2
  double d = 0;
  long long ll = 0;
  char *cp = 0;
  foo(d, ll, cp);
#elif TEST3
  long double ld = 0;
  long long ll = 0;
  char *cp = 0;
  int i = 0;
  foo(ld, ll, cp, i);
#elif TEST4
  long double ld = 0;
  unsigned short us;
  foo(ld, us);
#elif TEST5
  float f = 0;
  foo(f, f);
#elif TEST6
  long *lp = 0;
  foo(lp);
#elif TEST7
  S1 s;
  s.a = 0;
  s.b = 0;
  long double *ldp = 0;
  char c = 0;
  foo(s, ldp, c);
#else
#error Unknown test mode
#endif
  return 0;
}
