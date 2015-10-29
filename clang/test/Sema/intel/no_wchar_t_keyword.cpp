// RUN: %clang_cc1 %s --no_wchar_t_keyword -fsyntax-only -verify -DTEST1
// RUN: %clang_cc1 -DTEST2 %s -fsyntax-only -verify

unsigned short *a = 0;

#ifdef TEST1

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif  /* _WCHAR_T_DEFINED */

wchar_t *b = a; //expected-no-diagnostics

#endif // TEST1



#ifdef TEST2

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t; //expected-error {{'short wchar_t' is invalid}} expected-warning{{typedef requires a name}}
#define _WCHAR_T_DEFINED
#endif  /* _WCHAR_T_DEFINED */

wchar_t *b = a; // expected-error {{cannot initialize a variable of type 'wchar_t *' with an lvalue of type 'unsigned short *'}}

#endif // TEST2
