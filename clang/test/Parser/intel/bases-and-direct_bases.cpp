// CQ#369185
// RUN: %clang_cc1 -fsyntax-only -std=c++11 -DDISABLED -DCXX11 -verify %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -std=c++03 -DDISABLED -verify %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -std=c++98 -DDISABLED -verify %s
// RUN: %clang_cc1 -fsyntax-only -std=c++11 -fintel-compatibility -DENABLED -DCXX11 -verify %s

#ifdef DISABLED

// Simple typelist. Compile-time list of types.
template <typename... _Elements>
struct __reflection_typelist;
#ifndef CXX11
// expected-warning@-3{{variadic templates are a C++11 extension}}
#endif

template <typename _Tp>
// expected-note@-1{{declared here}}
// expected-note@-2{{declared here}}
struct obsolete {
  typedef __reflection_typelist<__bases(_Tp)...> type1;
  // expected-error@-1{{'_Tp' does not refer to a value}}
  typedef __reflection_typelist<__direct_bases(_Tp)...> type2;
  // expected-error@-1{{'_Tp' does not refer to a value}}
};

#elif ENABLED

// Simple typelist. Compile-time list of types.
template <typename... _Elements>
struct __reflection_typelist;

template <typename _Tp>
struct test {
  // __bases and __direct_bases can be used only with dependent types.
  typedef __reflection_typelist<__bases(int)...> type1;
  // expected-error@-1{{'__bases' specifier must be used with dependent type}}
  typedef __reflection_typelist<__direct_bases(short)...> type2;
  // expected-error@-1{{'__direct_bases' specifier must be used with dependent type}}

  // __bases and __direct_bases are pack expansions, so an ellipsis must follow.
  typedef __reflection_typelist<__bases(_Tp)> type3;
  // expected-error@-1{{expected '...' after '__bases' specifier}}
  typedef __reflection_typelist<__direct_bases(_Tp)> type4;
  // expected-error@-1{{expected '...' after '__direct_bases' specifier}}

  // Good case.
  typedef __reflection_typelist<__bases(_Tp)...> type5;
  typedef __reflection_typelist<__direct_bases(_Tp)...> type6;
};

#else

#error Unknown test mode

#endif
