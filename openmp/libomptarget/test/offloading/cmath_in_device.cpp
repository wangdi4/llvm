// UNSUPPORTED: system-windows
// RUN: %libomptarget-compilexx-generic -fno-fast-math -mlong-double-64 && %libomptarget-run-generic
// RUN: %libomptarget-compilexx-generic -O3 -fno-fast-math -mlong-double-64 && %libomptarget-run-generic

#include <cassert>
#include <cmath>
using namespace std;
template <typename T> void test_isnan() {
  T val = 1.0;
  int was_nan;
#pragma omp target map(to : val) map(tofrom : was_nan)
  { was_nan = isnan(val); }
  assert(was_nan == 0);

  val = INFINITY;
#pragma omp target map(to : val) map(tofrom : was_nan)
  { was_nan = isnan(val); }
  assert(was_nan == 0);

  val = NAN;
#pragma omp target map(to : val) map(tofrom : was_nan)
  { was_nan = isnan(val); }
  assert(was_nan != 0);
}

template <typename T> void test_isinf() {
  T val = 1.0;
  int was_inf;
#pragma omp target map(to : val) map(tofrom : was_inf)
  { was_inf = isinf(val); }
  assert(was_inf == 0);

  val = INFINITY;
#pragma omp target map(to : val) map(tofrom : was_inf)
  { was_inf = isinf(val); }
  assert(was_inf != 0);

  val = NAN;
#pragma omp target map(to : val) map(tofrom : was_inf)
  { was_inf = isinf(val); }
  assert(was_inf == 0);
}

template <typename T> void test_isnormal() {
  T val = 1.0;
  int was_normal;
#pragma omp target map(to : val) map(tofrom : was_normal)
  { was_normal = isnormal(val); }
  assert(was_normal != 0);

  val = 0.;
#pragma omp target map(to : val) map(tofrom : was_normal)
  { was_normal = isnormal(val); }
  assert(was_normal == 0);

  val = NAN;
#pragma omp target map(to : val) map(tofrom : was_normal)
  { was_normal = isnormal(val); }
  assert(was_normal == 0);

  val = INFINITY;
#pragma omp target map(to : val) map(tofrom : was_normal)
  { was_normal = isnormal(val); }
  assert(was_normal == 0);
}

template <typename T> void test_signbit() {
  T val = 0.;
  int sbt = 0;
#pragma omp target map(to : val) map(tofrom : sbt)
  { sbt = signbit(val); }
  assert(sbt == 0);

  val = -0.;
#pragma omp target map(to : val) map(tofrom : sbt)
  { sbt = signbit(val); }
  assert(sbt != 0);
}

template <typename T> void test_isfinite() {
  T val = NAN;
  int was_finite;
#pragma omp target map(to : val) map(tofrom : was_finite)
  { was_finite = isfinite(val); }
  assert(was_finite == 0);

  val = 1.0;
#pragma omp target map(to : val) map(tofrom : was_finite)
  { was_finite = isfinite(val); }
  assert(was_finite != 0);

  val = -INFINITY;
#pragma omp target map(to : val) map(tofrom : was_finite)
  { was_finite = isfinite(val); }
  assert(was_finite == 0);
}

int main() {
  test_isnan<float>();
  test_isnan<double>();
  test_isinf<float>();
  test_isinf<double>();
  test_isnormal<float>();
  test_isnormal<double>();
  test_signbit<float>();
  test_signbit<double>();
  test_isfinite<float>();
  test_isfinite<double>();
  return 0;
}
