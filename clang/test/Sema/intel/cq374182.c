// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -std=c89 -verify %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -std=c99 -DC99 -verify %s

typedef int *ipa[2];
restrict ipa y;
#ifdef C99
// expected-warning@-2 {{restrict requires a pointer or reference ('ipa' (aka 'int *[2]') is invalid)}}
#else
// expected-error@-4 {{unknown type name 'restrict'}}
// expected-error@-5 {{expected ';' after top level declarator}}
#endif
