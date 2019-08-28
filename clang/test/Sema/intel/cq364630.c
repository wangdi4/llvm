// RUN: %clang_cc1 -fsyntax-only -verify %s -DERROR
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -fintel-compatibility-disable=AllowExtraArgument -verify %s -DERROR
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DERROR
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility-enable=AllowExtraArgument -verify %s -DOK

void foo(int *arg1, const int *arg2); // expected-note {{'foo' declared here}}

void check() {
  int a1[42];
  int a2[42];
  int offset1, offset2;
  int *arg3;

#ifdef ERROR

  foo(a1 + offset1, a2 + offset2, &arg3);
  // expected-error@-1 {{too many arguments to function call, expected 2, have 3}}

#elif OK

  foo(a1 + offset1, a2 + offset2, &arg3);
  // expected-warning@-1 {{too many arguments to function call, expected 2, have 3}}

#else

#error Unknown test mode

#endif

}
