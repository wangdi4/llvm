// CQ#366562/CQ#415958
// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s
// REQUIRES: llvm-backend
#include <stdint.h>

void simple_test() {
  int s = 0;
#pragma unroll(0)
  for (int i = 0; i < 10; ++i){s = s + i;} 
#pragma clang loop unroll_count(0) // expected-error {{invalid value '0'; must be positive}}
  for (int i = 0; i < 10; ++i){s = s + i;}
#pragma unroll(-3) // expected-warning {{unrolling factor '-3' is negative and will be ignored}}
  for (int i = 0; i < 10; ++i){s = s + i;} 
#pragma clang loop unroll_count(-3) // expected-error {{invalid value '-3'; must be positive}}
  for (int i = 0; i < 10; ++i){s = s + i;}
#pragma unroll(1844674407370955148)// expected-warning {{unrolling factor '1844674407370955148' is too large and will be ignored}}
  for (int i = 0; i < 10; ++i){s = s + i;} 
#pragma clang loop unroll_count(1844674407370955148)// expected-error {{value '1844674407370955148' is too large}}
  for (int i = 0; i < 10; ++i){s = s + i;}
}
template<int64_t L>
void size_test() {
  int s = 0;
#pragma unroll(L) //expected-warning {{unrolling factor '-2' is negative and will be ignored}} expected-warning {{unrolling factor '1844674407370955161' is too large and will be ignored}}
  for (int i = 0; i < 10; ++i){s = s + i;} 
#pragma clang loop unroll_count(L) // expected-error {{invalid value '0'; must be positive}} expected-error {{invalid value '-2'; must be positive}}// expected-error {{value '1844674407370955161' is too large}}
  for (int i = 0; i < 10; ++i){s = s + i;}
}


template <typename T> // expected-note {{declared here}}
void illegal_pragma() {
  int s = 0;
#pragma unroll(T) // expected-error {{'T' does not refer to a value}}
  for (int i = 0; i < 10; ++i){s = s + i;}
}

int main() {
  illegal_pragma<int>();
  simple_test();

  size_test<0>(); //expected-note {{in instantiation of function template specialization 'size_test<0>' requested here}}
  size_test<-2>(); //expected-note {{in instantiation of function template specialization 'size_test<-2>' requested here}}
  size_test<1844674407370955161>();//expected-note {{in instantiation of function template specialization 'size_test<1844674407370955161>' requested here}}
}
