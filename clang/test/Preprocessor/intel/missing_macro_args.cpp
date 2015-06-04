// CQ#365448
// RUN: %clang_cc1 -fintel-ms-compatibility -verify %s -DWARNING1
// RUN: %clang_cc1 -fintel-ms-compatibility -verify %s -DWARNING2
// RUN: %clang_cc1 -verify %s -DERROR
// RUN: %clang_cc1 -fintel-ms-compatibility -DPREPROC -E %s -o - | FileCheck %s

#define RETERROR(x, y, z) return x + y##3 + z##4 // expected-note {{macro 'RETERROR' defined here}}

int main() {

#if WARNING1

  RETERROR(2,3); /* expected-warning {{too few arguments provided to function-like macro invocation}} \
                    expected-warning {{1 missing macro argument treated as empty string}} */

#elif WARNING2

  RETERROR(2); /* expected-warning {{too few arguments provided to function-like macro invocation}} \
                  expected-warning {{2 missing macro arguments treated as empty strings}} */

#elif ERROR

  RETERROR(2); /* expected-error {{too few arguments provided to function-like macro invocation}} \
                  expected-error {{use of undeclared identifier 'RETERROR'}} */

#elif PREPROC

  // CHECK: return 2 + 3 + 4;
  RETERROR(2);

#else

#error Unknown test mode

#endif
}
