// RUN: %clang_cc1 %s -Eonly -fintel-compatibility -verify

#pragma poison rindex
rindex(some_string, 'h');   // expected-error {{attempt to use a poisoned identifier}}

#define BAR _Pragma ("poison XYZW")  XYZW /*NO ERROR*/
  XYZW      // ok
BAR
  XYZW      // expected-error {{attempt to use a poisoned identifier}}

// Pragma poison shouldn't warn from macro expansions defined before the token
// is poisoned.

#define strrchr rindex2
#pragma poison rindex2

// Can poison multiple times.
#pragma poison rindex2

strrchr(some_string, 'h');   // ok.
