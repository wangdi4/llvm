// CQ#366562
// RUN: %clang_cc1 -fintel-compatibility -verify %s -DZERO_OK
// RUN: %clang_cc1 -fintel-compatibility -fintel-compatibility-disable=UnrollZero -verify %s
// RUN: %clang_cc1 -fintel-compatibility -fintel-compatibility-enable=UnrollZero -verify %s -DZERO_OK

int main(void) {
  int i = 0, s = 0;

#ifdef ZERO_OK
#pragma unroll(0)
#else
#pragma unroll(0) // expected-error{{invalid value '0'; must be positive}}
#endif
  for (i = 0; i < 10; ++i)
    s = s + i;

#ifdef ZERO_OK
#pragma unroll(-2) // expected-error{{invalid value '-2'; must be non-negative}}
#else
#pragma unroll(-2) // expected-error{{invalid value '-2'; must be positive}}
#endif
  for (i = 0; i < 10; ++i)
    s = s + i;

#pragma unroll(1844674407370955161) // expected-error{{value '1844674407370955161' is too large}}
  for (i = 0; i < 20; ++i)
    s = s + i;

#pragma clang loop unroll_count(0) // expected-error{{invalid value '0'; must be positive}}
  for (i = 0; i < 10; ++i)
    s = s + i;

#pragma clang loop unroll_count(-1) // expected-error{{invalid value '-1'; must be positive}}
  for (i = 0; i < 10; ++i)
    s = s + i;

#pragma clang loop unroll_count(1844674407370955161) // expected-error{{value '1844674407370955161' is too large}}
  for (i = 0; i < 10; ++i)
    s = s + i;

  return s;
}
