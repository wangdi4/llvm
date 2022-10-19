// RUN: %clang_cc1 -x c++ -verify -triple powerpc64le-unknown-unknown %s \
// RUN: -menable-no-infs -menable-no-nans -fintel-compatibility -DFAST=1

// RUN: %clang_cc1 -x c++ -verify -triple powerpc64le-unknown-unknown %s \
// RUN: -menable-no-infs -menable-no-nans -fintel-compatibility \
// RUN: -fintel-compatibility-enable=ExplicitComparisonToNaNOrINF -DFAST=1

// RUN: %clang_cc1 -x c++ -verify -triple powerpc64le-unknown-unknown %s \
// RUN: -fintel-compatibility -DNOFAST=1

// RUN: %clang_cc1 -x c++ -verify -triple powerpc64le-unknown-unknown %s \
// RUN: -fintel-compatibility \
// RUN: -fintel-compatibility-disable=ExplicitComparisonToNaNOrINF -DNOFAST=1

int isunorderedf (float x, float y);
#if NOFAST
// expected-no-diagnostics
#endif
extern "C++" {
namespace std __attribute__((__visibility__("default"))) {
  bool
  isinf(float __x);
  bool
  isinf(double __x);
  bool
  isinf(long double __x);
  bool
  isnan(float __x);
  bool
  isnan(double __x);
  bool
  isnan(long double __x);
} // namespace )
}
#define NAN (__builtin_nanf(""))
#define INFINITY (__builtin_inff())

int compareit(float a, float b) {
  volatile int i, j, k, l, m, n;
#if FAST
// expected-warning@+2 {{explicit comparison with infinity in fast floating point mode}}
#endif
  i = a == INFINITY;
#if FAST
// expected-warning@+2 {{explicit comparison with infinity in fast floating point mode}}
#endif
  j = INFINITY == a;
#if FAST
// expected-warning@+2 {{explicit comparison with NaN in fast floating point mode}}
#endif
  i = a == NAN;
#if FAST
// expected-warning@+2 {{explicit comparison with NaN in fast floating point mode}}
#endif
  j = NAN == a;
#if FAST
// expected-warning@+2 {{explicit comparison with infinity in fast floating point mode}}
#endif
  j = INFINITY <= a;
#if FAST
// expected-warning@+2 {{explicit comparison with infinity in fast floating point mode}}
#endif
  j = INFINITY < a;
#if FAST
// expected-warning@+2 {{explicit comparison with NaN in fast floating point mode}}
#endif
  j = a > NAN;
#if FAST
// expected-warning@+2 {{explicit comparison with NaN in fast floating point mode}}
#endif
  j = a >= NAN;
#if FAST
// expected-warning@+2 {{explicit comparison with infinity in fast floating point mode}}
#endif
  k = std::isinf(a);
#if FAST
// expected-warning@+2 {{explicit comparison with NaN in fast floating point mode}}
#endif
  l = std::isnan(a);
#if FAST
// expected-warning@+2 {{explicit comparison with infinity in fast floating point mode}}
#endif
  m = __builtin_isinf(a);
#if FAST
// expected-warning@+2 {{explicit comparison with NaN in fast floating point mode}}
#endif
  n = __builtin_isnan(a);

  // These should NOT warn, since they are not comparing with NaN or infinity.
  j = a > 1.1;
  j = b < 1.1;
  j = a >= 1.1;
  j = b <= 1.1;
  // These should NOT warn, since they are not comparing with isnan or isinf.
  j = isunorderedf(a, NAN);
  j = isunorderedf(a, INFINITY);
  return 0;
}
