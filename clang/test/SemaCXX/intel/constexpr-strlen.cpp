// RUN: %clang_cc1 %s -std=c++11 -fsyntax-only -verify -pedantic -fintel-compatibility

# 1 "/usr/include/string.h" 1 3 4
extern "C" {
  typedef decltype(sizeof(int)) size_t;
  extern size_t strlen(const char *p);
}

# 10 "SemaCXX/constexpr-strlen.cpp" 2
constexpr int n = __builtin_strlen("hello"); // ok // INTEL
constexpr int m = strlen("hello"); // ok

// Make sure we can evaluate a call to strlen.
int arr[3]; // expected-note {{here}}
int k = arr[strlen("hello")]; // expected-warning {{array index 5}}
