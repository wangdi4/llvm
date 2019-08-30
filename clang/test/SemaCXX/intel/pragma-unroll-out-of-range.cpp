// CQ#366562/CQ#415958

// RUN: %clang_cc1 -fintel-compatibility -verify %s -DZERO_OK
// RUN: %clang_cc1 -fintel-compatibility -fintel-compatibility-disable=UnrollZero -verify %s
// RUN: %clang_cc1 -fintel-compatibility -fintel-compatibility-enable=UnrollZero -verify %s -DZERO_OK

#include <stdint.h>

void simple_test() {
  int s = 0;
#ifdef ZERO_OK
#pragma unroll(-3) // expected-error{{invalid value '-3'; must be non-negative}}
#else
#pragma unroll(-3) // expected-error{{invalid value '-3'; must be positive}}
#endif
  for (int i = 0; i < 10; ++i){s = s + i;}
#pragma unroll(1844674407370955148) // expected-error{{value '1844674407370955148' is too large}}
  for (int i = 0; i < 10; ++i){s = s + i;}
}
template<int64_t L>
void size_test() {
  int s = 0;

#ifdef ZERO_OK
#pragma unroll(-3) // expected-error{{invalid value '-3'; must be non-negative}}
#else
#pragma unroll(-3) // expected-error{{invalid value '-3'; must be positive}}
#endif
  for (int i = 0; i < 10; ++i){s = s + i;}

#pragma unroll(1844674407370955148) // expected-error{{value '1844674407370955148' is too large}}
  for (int i = 0; i < 10; ++i){s = s + i;}
}
int main() {
  simple_test();

  size_test<-2>();
  size_test<1844674407370955161>();
}
