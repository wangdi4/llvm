// CQ#368318
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DERROR
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DOK

#ifdef ERROR

int __builtin_exp = 1;
// expected-error@-1{{redefinition of '__builtin_exp' as different kind of symbol}}
// expected-note@-2{{previous definition is here}}
int __builtin_index = 1;
// expected-error@-1{{redefinition of '__builtin_index' as different kind of symbol}}
// expected-note@-2{{previous definition is here}}

#elif OK

// expected-no-diagnostics

int exp = 2;
int index = 2;

void check() {
  exp = 42;
  index = 42;
}

#else

#error Unknown test mode

#endif
