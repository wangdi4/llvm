// CQ#364717
// RUN: %clang -cc1 -fintel-compatibility -Wintel-compat -verify -DTEST1 %s
// RUN: %clang -cc1 -fintel-compatibility -Wintel-compat -Werror -verify -DTEST2 %s
// RUN: %clang -cc1 -fintel-compatibility -Wno-intel-compat -verify -DTEST3 %s

// A known Intel compatibility feature is used for testing purpose - CQ#364426.
#ifdef TEST1

enum someenum {}; // expected-warning {{use of empty enum}}

#elif TEST2

enum someenum {}; // expected-error {{use of empty enum}}

#elif TEST3

enum someenum {}; // expected-no-diagnostics

#else

#error Unknown test mode

#endif

