// CQ#366963
// RUN: %clang_cc1 -fintel-compatibility -restrict -verify %s -DTESTOK
// RUN: %clang_cc1 -fintel-compatibility -no-restrict -verify  %s -DTESTERR
// RUN: %clang_cc1 -fintel-compatibility -std=c89 -verify %s -DTESTERR
// RUN: %clang_cc1 -fintel-compatibility -std=c99 -verify %s -DTESTOK
// RUN: %clang_cc1 -fintel-compatibility -restrict -std=c89 -verify %s -DTESTOK
// RUN: %clang_cc1 -fintel-compatibility -restrict -no-restrict -verify %s -DTESTERR
// RUN: %clang_cc1 -fintel-compatibility -restrict -no-restrict -restrict -verify %s -DTESTOK
// RUN: %clang_cc1 -fintel-compatibility -restrict -x c++ -std=gnu++98 -verify %s -DTESTOK
// RUN: %clang_cc1 -fintel-compatibility -no-restrict -x c++ -std=gnu++98 -verify %s -DTESTERR
// RUN: %clang_cc1 -fintel-compatibility -x c++ -std=gnu++98 -verify %s -DTESTERR
// RUN: %clang_cc1 -restrict -std=c89 -verify %s -DTESTOK
// RUN: %clang_cc1 -restrict -std=c99 -verify %s -DTESTOK
// RUN: %clang_cc1 -restrict -x c++ -std=gnu++98 -verify %s -DTESTOK
// RUN: %clang_cc1 -no-restrict -std=c99 -verify %s -DTESTERR


#if TESTOK

int * restrict a; // expected-no-diagnostics

#elif TESTERR

int * restrict a; // expected-error {{expected ';' after top level declarator}}

#else

#error Unknown test mode

#endif
