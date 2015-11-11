/* CQ#376559
  RUN: %clang_cc1 -triple i386-unknown-unknown -fsyntax-only -verify %s -DERROR
  RUN: %clang_cc1 -triple i386-unknown-unknown -fsyntax-only -verify %s -fintel-compatibility
*/

#if ERROR
void foo(unsigned __int128); /* expected-error{{__int128 is not supported on this target}} */
#else
void foo(unsigned __int128); /* expected-no-diagnostics */
#endif
