/* CQ#374317
  RUN: %clang_cc1 -fsyntax-only -verify %s -DERROR
  RUN: %clang_cc1 -fsyntax-only -verify %s -DERROR -fintel-compatibility -std=gnu99
  RUN: %clang_cc1 -fsyntax-only -verify %s -fintel-compatibility -std=c99
*/

#if ERROR
void foo(int _Decimal64); /* expected-error{{cannot combine with previous 'int' declaration specifier}} */
#else
void foo(int _Decimal64); /* expected-no-diagnostics */
#endif
