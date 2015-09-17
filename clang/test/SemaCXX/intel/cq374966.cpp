// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -x c++ -std=c++11 -fsyntax-only -verify %s -DFOR_X64
// RUN: %clang_cc1 -triple=i386-unknown-linux-gnu -fintel-compatibility -x c++ -std=c++11 -fsyntax-only -verify %s -DFOR_X86

template<class T> struct CheckInt64Type {
  static const bool passed = false;
};

#if FOR_X64
extern int (*cmIML_ABI___INT64_IS_LONG__VERIFY__)(__int64*);
extern int (*cmIML_ABI___INT64_IS_LONG__VERIFY__)(long*);
extern int (*cmIML_ABI___INT64_IS_LONG__VERIFY__)(long long*);
// expected-error@-1{{redeclaration of 'cmIML_ABI___INT64_IS_LONG__VERIFY__' with a different type}}
// expected-note@-3{{previous declaration is here}}

template<> struct CheckInt64Type<long> {
  static const bool passed = true;
};

static_assert(CheckInt64Type<__int64>::passed, "__int64 is not long!");

#elif FOR_X86
extern int (*cmIML_ABI___INT64_IS_LONG__VERIFY__)(__int64*);
extern int (*cmIML_ABI___INT64_IS_LONG__VERIFY__)(long long*);
extern int (*cmIML_ABI___INT64_IS_LONG__VERIFY__)(long*);
// expected-error@-1{{redeclaration of 'cmIML_ABI___INT64_IS_LONG__VERIFY__' with a different type}}
// expected-note@-3{{previous declaration is here}}

template<> struct CheckInt64Type<long long> {
  static const bool passed = true;
};

static_assert(CheckInt64Type<__int64>::passed, "__int64 is not long long!");

#else

#error Unknown test mode

#endif //FOR_X86
