// CQ#364426
// RUN: %clang_cc1 -fsyntax-only -verify -DTEST1 %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify -DTEST2 %s


#ifdef TEST1

enum someenum {};  // expected-error {{use of empty enum}}

#elif TEST2

enum someenum {};  // expected-warning {{use of empty enum}}

#else

#error Unknown test mode

#endif
